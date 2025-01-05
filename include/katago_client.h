#pragma once

#include <gtk/gtk.h>

#include <array>
#include <functional>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "wq.h"

using json = nlohmann::json;

enum class PlayStyle {
  kPreAlphaZero,
  kModern,
};

const char *play_style_string(PlayStyle style);

class KataGoClient {
 public:
  struct Query {
    wq::MoveList initial_stones;
    wq::MoveList moves;
    std::string rules = "chinese";
    float komi = 7.5;
    int board_size_rows = 19;
    int board_size_cols = 19;
    std::vector<int> analyze_turns;
    std::optional<int> max_visits;
    bool include_policy = false;
    bool include_ownership = false;
    json override_settings;
  };

  struct RootInfo {
    wq::Color current_player;
    int visits;
    double winrate;
    double score_lead;
  };

  struct MoveInfo {
    int order;
    wq::Point move;
    wq::PointList pv;
    int visits;
    double winrate;
    double score_lead;
  };

  struct Response {
    bool is_during_search;
    int turn_number;
    RootInfo root_info;
    std::vector<MoveInfo> move_infos;
    std::vector<double> policy;
    std::vector<double> human_policy;
    std::vector<double> ownership;
  };

  using QueryCallback =
      std::function<void(Response, std::optional<std::string>)>;

  KataGoClient(const char *katago_path, const char *model_path,
               const char *human_model_path, const char *config_path);
  ~KataGoClient();

  void run();
  void stop();
  std::string query(Query q, QueryCallback);
  void cancel_query(std::string id);

 private:
  const char *katago_path_;
  const char *model_path_;
  const char *human_model_path_;
  const char *config_path_;
  GSubprocess *proc_;
  GOutputStream *proc_in_;
  std::string cur_out_line_;
  std::array<uint8_t, 8 << 10> out_buf_;
  int query_id_ = 0;
  std::unordered_map<std::string, QueryCallback> queries_;
  std::unordered_map<std::string, int> query_pending_responses_;

  void process_cur_line();

  static void on_read(GObject *src, GAsyncResult *res, gpointer data);
};