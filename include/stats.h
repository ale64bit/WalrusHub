#pragma once

#include <map>
#include <unordered_map>
#include <utility>

#include "sqlite3.h"
#include "task.h"

class StatsDB {
 public:
  StatsDB(const char* path);
  ~StatsDB();

  void update_rank_stats(Rank rank, int total_inc, int err_inc);
  void update_tag_stats(int tag_id, Rank rank, int total_inc, int err_inc);
  std::unordered_map<Rank, std::pair<int, int>> get_rank_stats() const;
  std::unordered_map<int, std::map<Rank, std::pair<int, int>>> get_tag_stats()
      const;

 private:
  sqlite3* db_;

  static int get_rank_stats_cb(void* out, int column_count, char** column_value,
                               char** column_name);
  static int get_tag_stats_cb(void* out, int column_count, char** column_value,
                              char** column_name);
};