#pragma once

#include <map>
#include <unordered_map>
#include <utility>

#include "katago_client.h"
#include "sqlite3.h"
#include "task.h"

class StatsDB {
 public:
  StatsDB(const char* path);
  ~StatsDB();

  void update_rank_stats(Rank rank, int total_inc, int err_inc);
  void update_tag_stats(int tag_id, Rank rank, int total_inc, int err_inc);
  void update_play_ai_stats(PlayStyle style, Rank rank, int wins_inc,
                            int losses_inc);
  std::unordered_map<Rank, std::pair<int, int>> get_rank_stats() const;
  std::unordered_map<int, std::map<Rank, std::pair<int, int>>> get_tag_stats()
      const;
  std::map<std::pair<PlayStyle, Rank>, std::pair<int, int>> get_play_ai_stats()
      const;

 private:
  sqlite3* db_;

  void init_rank_stats();
  void init_tag_stats();
  void init_play_ai_stats();

  static int get_rank_stats_cb(void* out, int column_count, char** column_value,
                               char** column_name);
  static int get_tag_stats_cb(void* out, int column_count, char** column_value,
                              char** column_name);
  static int get_play_ai_stats_cb(void* out, int column_count,
                                  char** column_value, char** column_name);
};