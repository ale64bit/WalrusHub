#pragma once

#include <sqlite3.h>

#include <cassert>
#include <map>
#include <memory>
#include <nlohmann/json.hpp>
#include <optional>
#include <random>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "wq.h"

using json = nlohmann::json;

enum class Rank {
  kUnknown,
  k30K,
  k29K,
  k28K,
  k27K,
  k26K,
  k25K,
  k24K,
  k23K,
  k22K,
  k21K,
  k20K,
  k19K,
  k18K,
  k17K,
  k16K,
  k15K,
  k14K,
  k13K,
  k12K,
  k11K,
  k10K,
  k9K,
  k8K,
  k7K,
  k6K,
  k5K,
  k4K,
  k3K,
  k2K,
  k1K,
  k1D,
  k2D,
  k3D,
  k4D,
  k5D,
  k6D,
  k7D,
  k8D,
  k9D,
  k10D,
  k11D,
  k12D,
  k13D,
  k14D,
  k15D,
};

enum class TaskType {
  kUnknown = 0,
  kLifeAndDeath = 1,
  kTesuji = 2,
  kJoseki = 3,
  kOpening = 4,
  kEndgame = 5,
  kAppreciation = 8,
  kTrick = 9,
  kMiddlegame = 10,
  kMirror = 11,
  kTheory = 12,
  kOpeningChoice = 13,
  kMiddlegameChoice = 14,
  kEndgameChoice = 15,
  kLuoZi = 16,
  kCapture = 17,
  kCaptureRace = 21,
};

enum class AnswerType {
  kCorrect,
  kVariation,
  kWrong,
};

struct TreeNode {
  std::map<wq::Point, std::unique_ptr<TreeNode>> children_;
  std::string comment_;
  std::optional<AnswerType> answer_;
};

struct Task {
  int64_t id_;
  std::string source_;
  std::string description_;
  TaskType type_;
  std::vector<int64_t> tags_;
  std::unordered_map<std::string, std::string> metadata_;
  float rating_;
  Rank rank_;
  wq::Color first_to_play_;
  int board_size_;
  wq::Point top_left_;
  wq::Point bottom_right_;
  wq::PointList initial_[2];
  wq::PointList answer_points_;
  std::map<wq::Point, std::string> labels_;
  std::unique_ptr<TreeNode> vtree_;
};

struct SolvePreset {
  std::string description_;
  std::vector<std::string> sources_;
  std::vector<std::string> tags_;
  std::vector<TaskType> types_;
  Rank min_rank_ = Rank::kUnknown;
  Rank max_rank_ = Rank::kUnknown;
  float min_rating_ = 0;
  int min_board_size_ = 0;
  int max_board_size_ = 0;
  int time_limit_sec_ = 0;
  int max_tasks_ = 0;
  int max_errors_ = -1;
};

struct TaskTag {
  int64_t id_;
  std::string name_;
  std::string description_;
  std::string url_;
};

struct Book {
  int64_t id;
  std::string title;
  std::string title_en;
  std::string description;
  std::string url;
  Rank min_rank;
  Rank max_rank;
};

struct BookChapter {
  int64_t book_id;
  int64_t id;
  std::string title;
};

class TaskDB {
 public:
  TaskDB(const char* path);
  ~TaskDB();

  int64_t get_tag_id(std::string_view tag_name) const;
  int64_t add_tag(std::string_view tag_name);
  void add_tag(int64_t tag_id, std::string_view tag_name);
  std::optional<TaskTag> get_tag(int64_t tag_id) const;
  int64_t add_task(const Task& task);
  std::optional<Task> get_task(int64_t id) const;
  std::vector<int64_t> get_tasks(SolvePreset preset) const;

  int64_t add_book(const Book& book);
  std::vector<Book> list_books() const;
  std::optional<Book> get_book(int64_t id) const;
  int64_t add_book_chapter(int64_t book_id, const BookChapter& chapter);
  std::vector<BookChapter> list_book_chapters(int64_t book_id) const;
  std::optional<BookChapter> get_book_chapter(int64_t book_id,
                                              int64_t chapter_id) const;
  void add_book_task(int64_t book_id, int64_t chapter_id, int64_t task_id);
  std::vector<Task> list_book_tasks(int64_t book_id,
                                    std::optional<int64_t> chapter_id) const;

 private:
  sqlite3* db_;

  // Serialization
  static std::string encode_point(const wq::Point& p);
  static json encode_point_list(const wq::PointList& pl);
  static json encode_task_initial_stones(const Task& task);
  static json encode_task_answer_points(const Task& task);
  static json encode_task_labels(const Task& task);
  static json encode_task_vtree_node(const TreeNode* node);
  static json encode_task_vtree(const Task& task);
  static json encode_metadata(const Task& task);

  // Deserialization
  static wq::Point decode_point(std::string_view s);
  static wq::PointList decode_point_list(const json& j);
  static std::unique_ptr<TreeNode> decode_task_vtree(const json& j);

  // Callbacks
  static int get_tag_id_cb(void* out, int column_count, char** column_value,
                           char** column_name);
  static int get_tag_cb(void* out, int column_count, char** column_value,
                        char** column_name);
  static int get_tasks_cb(void* out, int column_count, char** column_value,
                          char** column_name);
  static int get_task_cb(void* out, int column_count, char** column_value,
                         char** column_name);
  static int list_books_cb(void* out, int column_count, char** column_value,
                           char** column_name);
  static int list_book_chapters_cb(void* out, int column_count,
                                   char** column_value, char** column_name);
  static int add_book_chapter_cb(void* out, int column_count,
                                 char** column_value, char** column_name);
  static int list_book_tasks_cb(void* out, int column_count,
                                char** column_value, char** column_name);
};

class TaskVTreeIterator {
 public:
  TaskVTreeIterator(const Task& task);

  std::optional<AnswerType> move(wq::Point p, std::string& comment);

  void reset();

  template <class Generator>
  const wq::Point& gen_move(Generator& gen) const {
    assert(!cur_->children_.empty());
    std::uniform_int_distribution<> dist(0, cur_->children_.size() - 1);
    int index = dist(gen);
    auto it = cur_->children_.begin();
    for (int i = 0; i < index; ++i) ++it;
    return it->first;
  }

 private:
  const Task& task_;
  TreeNode* cur_;
};

const char* rank_string(Rank rank);
const char* task_type_string(TaskType type);
std::optional<Task> import_101weiqi_task(std::string_view data);