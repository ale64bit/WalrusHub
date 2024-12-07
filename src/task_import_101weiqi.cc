#include <gtk/gtk.h>

#include <nlohmann/json.hpp>

#include "log.h"
#include "task.h"

using json = nlohmann::json;

static Rank parse_101weiqi_rank(std::string_view s) {
  if (!s.empty() && s.back() == '+') s.remove_suffix(1);
  for (int i = 30; i >= 1; --i) {
    const auto n = std::to_string(i);
    if (n + "k" == s || n + "K" == s) {
      return Rank(31 - i);
    }
  }
  for (int i = 1; i <= 15; ++i) {
    const auto n = std::to_string(i);
    if (n + "d" == s || n + "D" == s) {
      return Rank(int(Rank::k1D) + i - 1);
    }
  }
  return Rank::kUnknown;
}

std::optional<Task> import_101weiqi_task(std::string_view data) {
  auto from = data.find("var g_qq = {");
  if (from == data.npos) return {};
  auto to = data.find("\"};", from);
  if (to == data.npos) return {};

  std::string s(data.substr(from + 11, to - from - 9));
  std::replace(s.begin(), s.end(), '\n', ' ');

  json e = json::parse(s);

  if (e["status"] != 2) return {};

  Task task;
  task.source_ = "101weiqi";
  task.description_ = e["title"];
  if (task.description_.empty() && e.contains("name") && !e["name"].is_null()) {
    task.description_ = e["name"];
  }
  std::replace(task.description_.begin(), task.description_.end(), '\'', ';');
  task.rating_ = e["vote"];
  task.type_ = TaskType((int)e["qtype"]);
  task.rank_ = parse_101weiqi_rank(std::string(e["levelname"]));
  task.first_to_play_ = e["blackfirst"] ? wq::Color::kBlack : wq::Color::kWhite;
  task.board_size_ = e["lu"];
  task.top_left_ = wq::Point(e["pos_y1"], e["pos_x1"]);
  task.bottom_right_ = wq::Point(e["pos_y2"], e["pos_x2"]);

  task.metadata_["public_id"] = std::to_string((int)e["publicid"]);

  task.vtree_ = std::make_unique<TreeNode>();
  for (const auto& ans : e["answers"]) {
    if (ans["st"] != 2) continue;
    if (ans["pts"].empty()) continue;

    TreeNode* cur = task.vtree_.get();
    for (const auto& cp : ans["pts"]) {
      const std::string& cps = cp["p"];

      wq::Point p;
      if (cps.empty())
        p = wq::Point(-1, -1);
      else
        p = wq::Point(cps[1] - 'a', cps[0] - 'a');

      auto it = cur->children_.find(p);
      if (it == cur->children_.end()) {
        auto next = std::make_unique<TreeNode>();
        if (auto c = cp["c"]; c.is_string()) {
          next->comment_ = c;
          if (next->comment_ == " ") next->comment_.clear();
          std::replace(next->comment_.begin(), next->comment_.end(), '\n', ' ');
          std::replace(next->comment_.begin(), next->comment_.end(), '\'', ';');
        }
        cur->children_[p] = std::move(next);
      }
      cur = cur->children_[p].get();
    }
    if (cur->answer_.value_or(AnswerType::kWrong) == AnswerType::kWrong) {
      switch ((int)ans["ty"]) {
        case 1:
          cur->answer_ = AnswerType::kCorrect;
          break;
        case 2:
          cur->answer_ = AnswerType::kVariation;
          break;
        case 3:
          cur->answer_ = AnswerType::kWrong;
          break;
      }
    }
  }

  const int r = (int)e["r"];

  // Decode obfuscated initial points
  if (r != 1 && r != 2) {
    for (int i = 0; i < 2; ++i) {
      for (std::string ps : e["prepos"][i]) {
        task.initial_[i].emplace_back(ps[1] - 'a', ps[0] - 'a');
      }
    }
  } else {
    const std::string key = "101" + std::string(3, '0' + r + 1);
    const std::string input = e["c"];
    gsize decoded_len = 0;
    auto decoded = g_base64_decode(input.c_str(), &decoded_len);

    std::string output;
    for (gsize i = 0, j = 0; i < decoded_len; ++i) {
      output += (char)(decoded[i] ^ key[j]);
      j = (j + 1) % key.size();
    }
    g_free(decoded);

    json init_pos = json::parse(output);
    for (int i = 0; i < 2; ++i) {
      for (std::string ps : init_pos[i]) {
        task.initial_[i].emplace_back(ps[1] - 'a', ps[0] - 'a');
      }
    }
  }

  // Invert coordinates if needed
  const bool inv_coord = ((int)e["xv"]) % 3 != 0;
  if (inv_coord) {
    for (int i = 0; i < 2; ++i) {
      for (auto& p : task.initial_[i]) {
        std::swap(p.first, p.second);
      }
    }
  }

  // Answer points (for choose tasks)
  for (std::string ps : e["luozis"]) {
    task.answer_points_.emplace_back(ps[1] - 'a', ps[0] - 'a');
  }

  // Labels
  for (const auto& l : e["xuandians"]) {
    std::string ps = l["pt"];
    task.labels_[wq::Point(ps[1] - 'a', ps[0] - 'a')] = l["name"];
  }
  for (const auto& l : e["signs"]) {
    std::string ps = l;
    task.labels_[wq::Point(ps[1] - 'a', ps[0] - 'a')] = "$triangle";
  }

  return task;
}