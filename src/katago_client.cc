#include "katago_client.h"

#include <cstdlib>

#include "log.h"

const char *play_style_string(PlayStyle style) {
  switch (style) {
    case PlayStyle::kPreAlphaZero:
      return "Pre-AlphaZero";
    case PlayStyle::kModern:
      return "Modern";
  }
  return "?";
}

KataGoClient::KataGoClient(const char *katago_path, const char *model_path,
                           const char *human_model_path,
                           const char *config_path)
    : katago_path_(katago_path),
      model_path_(model_path),
      human_model_path_(human_model_path),
      config_path_(config_path),
      proc_(nullptr) {}

KataGoClient::~KataGoClient() { stop(); }

void KataGoClient::run() {
  if (proc_) return;
  proc_ = g_subprocess_new(
      GSubprocessFlags(G_SUBPROCESS_FLAGS_STDIN_PIPE |
                       G_SUBPROCESS_FLAGS_STDOUT_PIPE |
                       G_SUBPROCESS_FLAGS_STDERR_PIPE |
                       G_SUBPROCESS_FLAGS_SEARCH_PATH_FROM_ENVP),
      nullptr, katago_path_, "analysis", "-config", config_path_, "-model",
      model_path_, "-human-model", human_model_path_, nullptr);

  if (proc_) {
    proc_in_ = g_subprocess_get_stdin_pipe(proc_);

    GInputStream *out = g_subprocess_get_stdout_pipe(proc_);
    cur_out_line_.clear();
    g_input_stream_read_async(out, out_buf_.data(), out_buf_.size(),
                              G_PRIORITY_DEFAULT, nullptr, on_read, this);
  }
}

void KataGoClient::stop() {
  if (proc_) {
    g_subprocess_force_exit(proc_);
    proc_ = nullptr;
  }
}

std::string KataGoClient::query(Query q, QueryCallback cb) {
  std::string id = std::to_string(query_id_++);
  json j = {
      {"id", id},
      {"rules", q.rules},
      {"komi", q.komi},
      {"boardXSize", q.board_size_cols},
      {"boardYSize", q.board_size_rows},
      {"includePolicy", q.include_policy},
      {"includeOwnership", q.include_ownership},
      {"overrideSettings", q.override_settings},
      {"moves", json::array()},
  };
  if (q.max_visits) j["maxVisits"] = *q.max_visits;
  if (!q.initial_stones.empty()) {
    j["initialStones"] = json::array();
    for (const auto &[col, p] : q.initial_stones) {
      const auto &[r, c] = p;
      j["initialStones"].push_back(json::array(
          {wq::color_short_string(col),
           "(" + std::to_string(c) + "," + std::to_string(r) + ")"}));
    }
  }
  for (const auto &[col, p] : q.moves) {
    const auto &[r, c] = p;
    const std::string move_str =
        (p == wq::kPass)
            ? "pass"
            : ("(" + std::to_string(c) + "," + std::to_string(r) + ")");
    j["moves"].push_back(json::array({wq::color_short_string(col), move_str}));
  }
  if (!q.analyze_turns.empty()) j["analyze_turns"] = q.analyze_turns;

  std::string payload = j.dump() + "\n";
  if (!g_output_stream_write_all(proc_in_, payload.data(), payload.size(),
                                 nullptr, nullptr, nullptr)) {
    LOG(ERROR) << "katago: query write failed";
    return "";
  }
  queries_[id] = cb;
  return id;
}

void KataGoClient::cancel_query(std::string id) {
  auto it = queries_.find(id);
  if (it != queries_.end()) {
    json j = {
        {"id", id + "_term"},
        {"action", "terminate"},
        {"terminateId", id},
    };
    std::string payload = j.dump() + "\n";
    if (!g_output_stream_write_all(proc_in_, payload.data(), payload.size(),
                                   nullptr, nullptr, nullptr)) {
      LOG(ERROR) << "katago: terminate write failed";
    }
    queries_.erase(it);
  }
}

static wq::Point parse_gtp_point(const std::string &s) {
  if (s == "pass") return wq::kPass;
  int c = s[0] - 'A';
  if (s[0] > 'I') c--;
  return wq::Point(19 - std::stoi(s.substr(1)),
                   c);  // TODO: handle other board sizes
}

void KataGoClient::process_cur_line() {
  json j = json::parse(cur_out_line_);
  if (j.contains("error")) {
    if (j.contains("id")) {
      auto it = queries_.find(j["id"]);
      if (it != queries_.end()) {
        it->second(Response{}, j["error"]);
        queries_.erase(it);
      }
    } else {
      LOG(ERROR) << "katago: " << j.dump();
    }
  } else if (j.contains("warning")) {
    LOG(WARN) << "katago: " << j.dump();
  } else if (j.contains("id")) {
    auto it = queries_.find(j["id"]);
    if (it != queries_.end()) {
      Response resp;
      resp.is_during_search = j["isDuringSearch"];
      resp.turn_number = j["turnNumber"];
      for (const auto &mi : j["moveInfos"]) {
        MoveInfo move_info;
        move_info.order = mi["order"];
        move_info.move = parse_gtp_point(mi["move"]);
        for (std::string pvi : mi["pv"])
          move_info.pv.push_back(parse_gtp_point(pvi));
        move_info.visits = mi["visits"];
        move_info.winrate = mi["winrate"];
        move_info.score_lead = mi["scoreLead"];
        resp.move_infos.emplace_back(move_info);
      }
      resp.root_info.winrate = j["rootInfo"]["winrate"];
      resp.root_info.score_lead = j["rootInfo"]["scoreLead"];
      resp.root_info.visits = j["rootInfo"]["visits"];
      if (j.contains("policy")) {
        resp.policy.clear();
        for (double v : j["policy"]) resp.policy.push_back(v);
      }
      if (j.contains("humanPolicy")) {
        resp.human_policy.clear();
        for (double v : j["humanPolicy"]) resp.human_policy.push_back(v);
      }
      if (j.contains("ownership")) {
        resp.ownership.clear();
        for (double v : j["ownership"]) resp.ownership.push_back(v);
      }
      it->second(resp, {});
      if (!resp.is_during_search) queries_.erase(it);
    }
  }
  cur_out_line_.clear();
}

void KataGoClient::on_read(GObject *src, GAsyncResult *res, gpointer data) {
  KataGoClient *client = (KataGoClient *)data;
  gssize size = g_input_stream_read_finish(G_INPUT_STREAM(src), res, nullptr);
  if (size > 0) {
    for (gssize i = 0; i < size; ++i) {
      if (client->out_buf_[i] == '\n') {
        client->process_cur_line();
      } else {
        client->cur_out_line_.push_back(client->out_buf_[i]);
      }
    }
    g_input_stream_read_async(G_INPUT_STREAM(src), client->out_buf_.data(),
                              client->out_buf_.size(), G_PRIORITY_DEFAULT,
                              nullptr, on_read, data);
  }
}
