#include "stats.h"

#include <sstream>

#include "log.h"
#include "task.h"

constexpr const char *kStatsDBSchema = R"(
  CREATE TABLE IF NOT EXISTS rank_stats (
    rank   INTEGER NOT NULL PRIMARY KEY,
    total  INTEGER NOT NULL,
    errors INTEGER NOT NULL
  );

  CREATE TABLE IF NOT EXISTS tag_stats (
    tag    INTEGER NOT NULL,
    rank   INTEGER NOT NULL,
    total  INTEGER NOT NULL,
    errors INTEGER NOT NULL,
    PRIMARY KEY(tag, rank)
  );

  CREATE TABLE IF NOT EXISTS play_ai_stats (
    style INTEGER NOT NULL,
    rank  INTEGER NOT NULL,
    wins  INTEGER NOT NULL,
    losses INTEGER NOT NULL,
    PRIMARY KEY(style, rank)
  );
)";

StatsDB::StatsDB(const char *path) {
  if (sqlite3_open(path, &db_)) {
    LOG(ERROR) << "stats db: failed to open database: " << sqlite3_errmsg(db_);
    sqlite3_close(db_);
    std::exit(1);
  }
  if (sqlite3_exec(db_, kStatsDBSchema, nullptr, nullptr, nullptr)) {
    LOG(INFO) << "stats db: applying schema: code=" << sqlite3_errcode(db_)
              << " msg='" << sqlite3_errmsg(db_) << "'";
    std::exit(1);
  }
  init_rank_stats();
  init_tag_stats();
  init_play_ai_stats();
}

StatsDB::~StatsDB() { sqlite3_close(db_); }

void StatsDB::init_rank_stats() {
  std::stringstream q;
  q << "INSERT OR IGNORE INTO rank_stats VALUES ";
  for (int rank = (int)Rank::k30K; rank <= (int)Rank::k15D; ++rank) {
    if (rank > (int)Rank::k30K) q << ", ";
    q << "(" << rank << ", " << "0, 0)";
  }
  q << ";";
  if (sqlite3_exec(db_, q.str().c_str(), nullptr, nullptr, nullptr)) {
    LOG(INFO) << "stats db: initializing rank stats: code="
              << sqlite3_errcode(db_) << " msg='" << sqlite3_errmsg(db_) << "'";
    std::exit(1);
  }
}

void StatsDB::init_tag_stats() {
  std::stringstream q;
  q << "INSERT OR IGNORE INTO tag_stats VALUES ";
  for (int tag = 1; tag <= 999; ++tag) {
    for (int rank = (int)Rank::k30K; rank <= (int)Rank::k15D; ++rank) {
      if (tag > 1 || rank > (int)Rank::k30K) q << ", ";
      q << "(" << tag << ", " << rank << ", " << "0, 0)";
    }
  }
  q << ";";
  if (sqlite3_exec(db_, q.str().c_str(), nullptr, nullptr, nullptr)) {
    LOG(INFO) << "stats db: initializing tag stats: code="
              << sqlite3_errcode(db_) << " msg='" << sqlite3_errmsg(db_) << "'";
    std::exit(1);
  }
}

void StatsDB::init_play_ai_stats() {
  std::stringstream q;
  q << "INSERT OR IGNORE INTO play_ai_stats VALUES ";
  for (int style = 0; style < 2; ++style) {
    for (int rank = (int)Rank::k20K; rank <= (int)Rank::k9D; ++rank) {
      if (style > 0 || rank > (int)Rank::k20K) q << ", ";
      q << "(" << style << ", " << rank << ", " << "0, 0)";
    }
  }
  q << ";";
  if (sqlite3_exec(db_, q.str().c_str(), nullptr, nullptr, nullptr)) {
    LOG(INFO) << "stats db: initializing play ai stats: code="
              << sqlite3_errcode(db_) << " msg='" << sqlite3_errmsg(db_) << "'";
    std::exit(1);
  }
}

void StatsDB::update_rank_stats(Rank rank, int total_inc, int err_inc) {
  std::stringstream q;
  q << "UPDATE rank_stats SET total = total + " << total_inc
    << ", errors = errors + " << err_inc << " WHERE rank = " << (int)rank
    << ";";
  if (sqlite3_exec(db_, q.str().c_str(), nullptr, nullptr, nullptr)) {
    LOG(ERROR) << "stats db: updating rank stats: code=" << sqlite3_errcode(db_)
               << " msg='" << sqlite3_errmsg(db_) << "'";
  }
}

void StatsDB::update_tag_stats(int tag_id, Rank rank, int total_inc,
                               int err_inc) {
  std::stringstream q;
  q << "UPDATE tag_stats SET total = total + " << total_inc
    << ", errors = errors + " << err_inc << " WHERE tag = " << tag_id
    << " AND rank = " << (int)rank << ";";
  if (sqlite3_exec(db_, q.str().c_str(), nullptr, nullptr, nullptr)) {
    LOG(ERROR) << "stats db: updating tag stats: code=" << sqlite3_errcode(db_)
               << " msg='" << sqlite3_errmsg(db_) << "'";
  }
}

void StatsDB::update_play_ai_stats(PlayStyle style, Rank rank, int wins_inc,
                                   int losses_inc) {
  std::stringstream q;
  q << "UPDATE play_ai_stats SET wins = wins + " << wins_inc
    << ", losses = losses + " << losses_inc << " WHERE style = " << (int)style
    << " AND rank = " << (int)rank << ";";
  if (sqlite3_exec(db_, q.str().c_str(), nullptr, nullptr, nullptr)) {
    LOG(ERROR) << "stats db: updating play AI stats: code="
               << sqlite3_errcode(db_) << " msg='" << sqlite3_errmsg(db_)
               << "'";
  }
}

std::unordered_map<Rank, std::pair<int, int>> StatsDB::get_rank_stats() const {
  std::unordered_map<Rank, std::pair<int, int>> rank_stats;
  if (sqlite3_exec(db_, "SELECT * FROM rank_stats;", get_rank_stats_cb,
                   &rank_stats, nullptr)) {
    LOG(ERROR) << "stats db: getting rank stats: code=" << sqlite3_errcode(db_)
               << " msg='" << sqlite3_errmsg(db_) << "'";
    return {};
  }
  return rank_stats;
}

int StatsDB::get_rank_stats_cb(void *out, int /*column_count*/,
                               char **column_value, char ** /*column_name*/) {
  const Rank id = (Rank)std::stoi(column_value[0]);
  (*(std::unordered_map<Rank, std::pair<int, int>> *)out)[id] =
      std::make_pair(std::stoi(column_value[1]), std::stoi(column_value[2]));
  return 0;
}

std::unordered_map<int, std::map<Rank, std::pair<int, int>>>
StatsDB::get_tag_stats() const {
  std::unordered_map<int, std::map<Rank, std::pair<int, int>>> tag_stats;
  if (sqlite3_exec(db_, "SELECT * FROM tag_stats;", get_tag_stats_cb,
                   &tag_stats, nullptr)) {
    LOG(ERROR) << "stats db: getting tag stats: code=" << sqlite3_errcode(db_)
               << " msg='" << sqlite3_errmsg(db_) << "'";
    return {};
  }
  return tag_stats;
}

int StatsDB::get_tag_stats_cb(void *out, int /*column_count*/,
                              char **column_value, char ** /*column_name*/) {
  const int tag_id = std::stoi(column_value[0]);
  const Rank rank = (Rank)std::stoi(column_value[1]);
  (*(std::unordered_map<int, std::map<Rank, std::pair<int, int>>> *)
       out)[tag_id][rank] =
      std::make_pair(std::stoi(column_value[2]), std::stoi(column_value[3]));
  return 0;
}

std::map<std::pair<PlayStyle, Rank>, std::pair<int, int>>
StatsDB::get_play_ai_stats() const {
  std::map<std::pair<PlayStyle, Rank>, std::pair<int, int>> stats;
  if (sqlite3_exec(db_, "SELECT * FROM play_ai_stats;", get_play_ai_stats_cb,
                   &stats, nullptr)) {
    LOG(ERROR) << "stats db: getting play AI stats: code="
               << sqlite3_errcode(db_) << " msg='" << sqlite3_errmsg(db_)
               << "'";
    return {};
  }
  return stats;
}

int StatsDB::get_play_ai_stats_cb(void *out, int /*column_count*/,
                                  char **column_value,
                                  char ** /*column_name*/) {
  const PlayStyle style = (PlayStyle)std::stoi(column_value[0]);
  const Rank rank = (Rank)std::stoi(column_value[1]);
  (*(std::map<std::pair<PlayStyle, Rank>, std::pair<int, int>> *)
       out)[{style, rank}] =
      std::make_pair(std::stoi(column_value[2]), std::stoi(column_value[3]));
  return 0;
}