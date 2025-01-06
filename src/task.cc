#include "task.h"

#include <algorithm>
#include <sstream>

#include "log.h"

constexpr const char *kTaskDBSchema = R"(
  PRAGMA foreign_keys=on;

  CREATE TABLE IF NOT EXISTS tasks (
    id             INTEGER PRIMARY KEY,
    source         TEXT,
    description    TEXT,
    type           INTEGER NOT NULL,
    rank           INTEGER NOT NULL,
    rating         REAL,
    first_to_play  INTEGER NOT NULL,
    board_size     INTEGER NOT NULL,
    top_left_r     INTEGER NOT NULL,
    top_left_c     INTEGER NOT NULL,
    bottom_right_r INTEGER NOT NULL,
    bottom_right_c INTEGER NOT NULL,
    initial_stones TEXT NOT NULL,
    answer_points  TEXT,
    labels         TEXT,
    vtree          TEXT NOT NULL,
    metadata       TEXT
  );

  CREATE TABLE IF NOT EXISTS tags (
    id          INTEGER PRIMARY KEY,
    name        TEXT NOT NULL,
    description TEXT,
    url         TEXT
  );

  CREATE TABLE IF NOT EXISTS tasks_tags(
    tag_id  INTEGER,
    task_id INTEGER,
    FOREIGN KEY(tag_id) REFERENCES tags(id),
    FOREIGN KEY(task_id) REFERENCES tasks(id),
    PRIMARY KEY(tag_id, task_id)
  );

  CREATE TABLE IF NOT EXISTS books(
    id          INTEGER PRIMARY KEY,
    title       TEXT NOT NULL,
    title_en    TEXT NOT NULL,
    description TEXT NOT NULL,
    url         TEXT NOT NULL,
    min_rank    INTEGER NOT NULL,
    max_rank    INTEGER NOT NULL
  );

  CREATE TABLE IF NOT EXISTS book_chapters(
    book_id INTEGER,
    id      INTEGER,
    title   TEXT NOT NULL,
    FOREIGN KEY(book_id) REFERENCES books(id),
    PRIMARY KEY(book_id, id)
  );

  CREATE TABLE IF NOT EXISTS book_tasks(
    book_id    INTEGER,
    chapter_id INTEGER,
    task_id    INTEGER,
    FOREIGN KEY(book_id) REFERENCES books(id),
    FOREIGN KEY(book_id, chapter_id) REFERENCES book_chapters(book_id, id),
    FOREIGN KEY(task_id) REFERENCES tasks(id),
    PRIMARY KEY(book_id, chapter_id, task_id)
  );
)";

TaskDB::TaskDB(const char *path) {
  if (sqlite3_open(path, &db_)) {
    LOG(ERROR) << "task db: failed to open task database: "
               << sqlite3_errmsg(db_);
    sqlite3_close(db_);
    std::exit(1);
  }
  if (sqlite3_exec(db_, kTaskDBSchema, nullptr, nullptr, nullptr)) {
    LOG(INFO) << "task db: applying schema: code=" << sqlite3_errcode(db_)
              << " msg='" << sqlite3_errmsg(db_) << "'";
    std::exit(1);
  }
}

TaskDB::~TaskDB() { sqlite3_close(db_); }

int64_t TaskDB::get_tag_id(std::string_view tag_name) const {
  int64_t id = -1;
  std::ostringstream q;
  q << "SELECT id FROM tags WHERE name = '" << tag_name << "';";
  if (sqlite3_exec(db_, q.str().c_str(), get_tag_id_cb, &id, nullptr)) {
    LOG(ERROR) << "get_tag_id: code=" << sqlite3_errcode(db_) << ": "
               << sqlite3_errmsg(db_);
    return -1;
  }
  return id;
}

int TaskDB::get_tag_id_cb(void *out, int /*column_count*/, char **column_value,
                          char ** /*column_name*/) {
  int64_t &id = *(int64_t *)out;
  id = std::stoll(column_value[0]);
  return 0;
}

int64_t TaskDB::add_tag(std::string_view tag_name) {
  int64_t id = get_tag_id(tag_name);
  if (id != -1) return id;

  std::ostringstream q;
  q << "INSERT INTO tags (name) VALUES ('" << tag_name << "');";
  if (sqlite3_exec(db_, q.str().c_str(), nullptr, nullptr, nullptr)) {
    LOG(ERROR) << "add_tag: code=" << sqlite3_errcode(db_) << ": "
               << sqlite3_errmsg(db_);
    return -1;
  }
  return sqlite3_last_insert_rowid(db_);
}

void TaskDB::add_tag(int64_t tag_id, std::string_view tag_name) {
  std::ostringstream q;
  q << "INSERT INTO tags (id, name) VALUES (" << tag_id << ", '" << tag_name
    << "');";
  if (sqlite3_exec(db_, q.str().c_str(), nullptr, nullptr, nullptr)) {
    LOG(ERROR) << "add_tag: code=" << sqlite3_errcode(db_) << ": "
               << sqlite3_errmsg(db_);
  }
}

std::optional<TaskTag> TaskDB::get_tag(int64_t tag_id) const {
  std::optional<TaskTag> tag;
  std::ostringstream q;
  q << "SELECT id, name, description, url FROM tags WHERE id = " << tag_id
    << ";";
  if (sqlite3_exec(db_, q.str().c_str(), get_tag_cb, &tag, nullptr)) {
    LOG(ERROR) << "get_tag(" << tag_id << "): code=" << sqlite3_errcode(db_)
               << ": " << sqlite3_errmsg(db_) << "\nquery: " << q.str();
    return {};
  }
  return tag;
}

int TaskDB::get_tag_cb(void *out, int /*column_count*/, char **column_value,
                       char ** /*column_name*/) {
  TaskTag tag;

  tag.id_ = std::stoll(column_value[0]);
  tag.name_ = column_value[1];
  if (column_value[2]) tag.description_ = column_value[2];
  if (column_value[3]) tag.url_ = column_value[3];

  *((std::optional<TaskTag> *)out) = std::move(tag);

  return 0;
}

int64_t TaskDB::add_task(const Task &task) {
  std::ostringstream q;
  q << "INSERT INTO tasks (source, description, type, rank, rating, "
       "first_to_play, "
       "board_size, top_left_r, top_left_c, bottom_right_r, bottom_right_c, "
       "initial_stones, answer_points, labels, vtree, metadata) VALUES ("
    << "'" << task.source_ << "', "
    << "'" << task.description_ << "', " << (int)task.type_ << ", "
    << (int)task.rank_ << ", " << task.rating_ << ", "
    << (int)task.first_to_play_ << ", " << task.board_size_ << ", "
    << task.top_left_.first << ", " << task.top_left_.second << ", "
    << task.bottom_right_.first << ", " << task.bottom_right_.second << ", "
    << "'" << encode_task_initial_stones(task).dump() << "', "
    << "'" << encode_task_answer_points(task).dump() << "', "
    << "'" << encode_task_labels(task).dump() << "', "
    << "'" << encode_task_vtree(task).dump() << "', "
    << "'" << encode_metadata(task) << "');";

  if (sqlite3_exec(db_, q.str().c_str(), nullptr, nullptr, nullptr)) {
    LOG(ERROR) << "add_task: code=" << sqlite3_errcode(db_) << ": "
               << sqlite3_errmsg(db_) << "\nquery: `" << q.str() << "`";
    return -1;
  }

  const int64_t task_id = sqlite3_last_insert_rowid(db_);

  if (!task.tags_.empty()) {
    q.str("");
    q << "INSERT INTO tasks_tags (tag_id, task_id) VALUES ";
    bool first = true;
    for (const auto &tag_id : task.tags_) {
      if (!first) {
        q << ", ";
      } else {
        first = false;
      }
      q << "(" << tag_id << ", " << task_id << ")";
    }
    q << ";";

    if (sqlite3_exec(db_, q.str().c_str(), nullptr, nullptr, nullptr)) {
      LOG(ERROR) << "add_task (tag assoc): code=" << sqlite3_errcode(db_)
                 << ": " << sqlite3_errmsg(db_) << "\nquery: `" << q.str()
                 << "`";
      return -1;
    }
  }

  return task_id;
}

std::vector<int64_t> TaskDB::get_tasks(SolvePreset preset) const {
  std::ostringstream q;
  q << "SELECT id FROM tasks ";

  if (!preset.tags_.empty()) {
    q << "INNER JOIN tasks_tags ON tasks.id = tasks_tags.task_id AND "
         "tasks_tags.tag_id IN (";
    for (size_t i = 0; i < preset.tags_.size(); ++i) {
      if (i > 0) q << ", ";
      q << get_tag_id(preset.tags_[i]);
    }
    q << ")";
  }

  q << "WHERE TRUE ";

  if (!preset.sources_.empty()) {
    q << " AND (source IN (";
    for (size_t i = 0; i < preset.sources_.size(); ++i) {
      if (i > 0) q << ", ";
      q << "'" << preset.sources_[i] << "'";
    }
    q << "))";
  }

  if (!preset.types_.empty()) {
    q << " AND (type IN (";
    for (size_t i = 0; i < preset.types_.size(); ++i) {
      if (i > 0) q << ", ";
      q << (int)preset.types_[i];
    }
    q << "))";
  }

  if (preset.min_rank_ != Rank::kUnknown) {
    q << " AND (" << (int)preset.min_rank_ << " <= rank)";
  }

  if (preset.max_rank_ != Rank::kUnknown) {
    q << " AND (rank <= " << (int)preset.max_rank_ << ")";
  }

  if (preset.min_rating_ > 0) {
    q << " AND (" << preset.min_rating_ << " <= rating)";
  }

  if (preset.min_board_size_ > 0) {
    q << " AND (" << preset.min_board_size_ << " <= board_size)";
  }

  if (preset.max_board_size_ > 0) {
    q << " AND (board_size <= " << preset.max_board_size_ << ")";
  }

  q << ";";

  std::vector<int64_t> ids;

  if (sqlite3_exec(db_, q.str().c_str(), get_tasks_cb, &ids, nullptr)) {
    LOG(ERROR) << "get_tasks: code=" << sqlite3_errcode(db_) << ": "
               << sqlite3_errmsg(db_) << "\nquery: " << q.str();
    return {};
  }

  return ids;
}

int TaskDB::get_tasks_cb(void *out, int /*column_count*/, char **column_value,
                         char ** /*column_name*/) {
  int64_t id = std::stoll(column_value[0]);
  if (id > 0) ((std::vector<int64_t> *)out)->push_back(id);
  return 0;
}

std::optional<Task> TaskDB::get_task(int64_t id) const {
  std::optional<Task> task;
  std::ostringstream q;
  q << "SELECT * FROM tasks WHERE id = " << id << ";";
  if (sqlite3_exec(db_, q.str().c_str(), get_task_cb, &task, nullptr)) {
    LOG(ERROR) << "get_task(" << id << "): code=" << sqlite3_errcode(db_)
               << ": " << sqlite3_errmsg(db_) << "\nquery: " << q.str();
    return {};
  }
  if (!task) return {};

  return task;
}

static inline bool valid_sgf_point(int board_size, std::string_view p) {
  return p.size() == 2 && 'a' <= p[0] && p[0] <= ('a' + board_size - 1) &&
         'a' <= p[1] && p[1] <= ('a' + board_size - 1);
}

int TaskDB::get_task_cb(void *out, int /*column_count*/, char **column_value,
                        char ** /*column_name*/) {
  Task task;

  task.id_ = std::stoll(column_value[0]);
  if (column_value[1]) task.source_ = column_value[1];
  if (column_value[2]) task.description_ = column_value[2];
  task.type_ = TaskType(std::atoi(column_value[3]));
  task.rank_ = Rank(std::atoi(column_value[4]));
  if (column_value[4]) task.rating_ = std::atof(column_value[5]);
  task.first_to_play_ = wq::Color(std::atoi(column_value[6]));
  task.board_size_ = std::atoi(column_value[7]);
  task.top_left_ =
      wq::Point(std::atoi(column_value[8]), std::atoi(column_value[9]));
  task.bottom_right_ =
      wq::Point(std::atoi(column_value[10]), std::atoi(column_value[11]));

  // Initial stones
  {
    json j = json::parse(column_value[12]);
    task.initial_[0] = decode_point_list(j[0]);
    task.initial_[1] = decode_point_list(j[1]);
  }

  // Answer points
  if (column_value[13]) {
    json j = json::parse(column_value[13]);
    task.answer_points_ = decode_point_list(j);
  }

  // Labels
  if (column_value[14]) {
    json j = json::parse(column_value[14]);
    for (const auto &[p, label] : j.items()) {
      if (valid_sgf_point(task.board_size_, p))
        task.labels_[decode_point(p)] = label;
    }
  }

  // Vtree
  task.vtree_ = decode_task_vtree(json::parse(column_value[15]));
  task.metadata_ = json::parse(column_value[16]);

  *((std::optional<Task> *)out) = std::move(task);

  return 0;
}

wq::Point TaskDB::decode_point(std::string_view s) {
  return wq::Point(s[1] - 'a', s[0] - 'a');
}

wq::PointList TaskDB::decode_point_list(const json &j) {
  wq::PointList pl;
  for (std::string p : j) pl.emplace_back(decode_point(p));
  return pl;
}

std::unique_ptr<TreeNode> TaskDB::decode_task_vtree(const json &j) {
  auto node = std::make_unique<TreeNode>();
  if (j.contains("c")) node->comment_ = j["c"];
  if (j.contains("a")) node->answer_ = AnswerType((int)j["a"]);
  if (j.contains("n")) {
    for (const auto &[p, next] : j["n"].items()) {
      node->children_[decode_point(p)] = decode_task_vtree(next);
    }
  }
  return node;
}

std::string TaskDB::encode_point(const wq::Point &p) {
  const auto &[r, c] = p;
  std::ostringstream out;
  out << (char)('a' + c) << (char)('a' + r);
  return out.str();
}

json TaskDB::encode_point_list(const wq::PointList &pl) {
  json ret = json::array();
  for (const auto &p : pl) ret.push_back(encode_point(p));
  return ret;
}

json TaskDB::encode_task_initial_stones(const Task &task) {
  return json::array({
      encode_point_list(task.initial_[0]),
      encode_point_list(task.initial_[1]),
  });
}

json TaskDB::encode_task_answer_points(const Task &task) {
  return encode_point_list(task.answer_points_);
}

json TaskDB::encode_task_labels(const Task &task) {
  json ret = {};
  for (const auto &[p, label] : task.labels_) ret[encode_point(p)] = label;
  return ret;
}

json TaskDB::encode_task_vtree_node(const TreeNode *node) {
  json ret;
  if (!node->comment_.empty()) ret["c"] = node->comment_;
  if (node->answer_) ret["a"] = int(node->answer_.value());
  if (!node->children_.empty()) {
    json children;
    for (const auto &[p, next] : node->children_) {
      children[encode_point(p)] = encode_task_vtree_node(next.get());
    }
    ret["n"] = children;
  }
  return ret;
}

json TaskDB::encode_task_vtree(const Task &task) {
  return encode_task_vtree_node(task.vtree_.get());
}

json TaskDB::encode_metadata(const Task &task) {
  json ret = json::object();
  for (const auto &[k, v] : task.metadata_) ret[k] = v;
  return ret;
}

int64_t TaskDB::add_book(const Book &book) {
  std::ostringstream q;
  q << "INSERT INTO books (title, title_en, description, url, min_rank, "
       "max_rank) VALUES ("
    << "'" << book.title << "', "
    << "'" << book.title_en << "', "
    << "'" << book.description << "', "
    << "'" << book.url << "', " << (int)book.min_rank << ", "
    << (int)book.max_rank << ");";

  if (sqlite3_exec(db_, q.str().c_str(), nullptr, nullptr, nullptr)) {
    LOG(ERROR) << "add_book: code=" << sqlite3_errcode(db_) << ": "
               << sqlite3_errmsg(db_) << "\nquery: `" << q.str() << "`";
    return -1;
  }

  const int64_t book_id = sqlite3_last_insert_rowid(db_);
  return book_id;
}

int TaskDB::list_books_cb(void *out, int /*column_count*/, char **column_value,
                          char ** /*column_name*/) {
  Book book;
  book.id = std::stoll(column_value[0]);
  book.title = column_value[1];
  book.title_en = column_value[2];
  book.description = column_value[3];
  book.url = column_value[4];
  book.min_rank = Rank(std::stoi(column_value[5]));
  book.max_rank = Rank(std::stoi(column_value[6]));
  ((std::vector<Book> *)out)->emplace_back(book);
  return 0;
}

std::vector<Book> TaskDB::list_books() const {
  std::vector<Book> books;
  const char *q = "SELECT * FROM books;";
  if (sqlite3_exec(db_, q, list_books_cb, &books, nullptr)) {
    LOG(ERROR) << "list_books: code=" << sqlite3_errcode(db_) << ": "
               << sqlite3_errmsg(db_) << "\nquery: `" << q << "`";
    return {};
  }
  return books;
}

int TaskDB::add_book_chapter_cb(void *out, int /*column_count*/,
                                char **column_value, char ** /*column_name*/) {
  *((int64_t *)out) = std::stoll(column_value[0]);
  return 0;
}

int64_t TaskDB::add_book_chapter(int64_t book_id, const BookChapter &chapter) {
  std::ostringstream q;
  q << "INSERT INTO book_chapters (book_id, id, title) "
    << "SELECT " << book_id << ", 1+COALESCE(MAX(id), 0), '" << chapter.title
    << "' FROM book_chapters WHERE book_id = " << book_id << ";";

  if (sqlite3_exec(db_, q.str().c_str(), nullptr, nullptr, nullptr)) {
    LOG(ERROR) << "add_book_chapter: code=" << sqlite3_errcode(db_) << ": "
               << sqlite3_errmsg(db_) << "\nquery: `" << q.str() << "`";
    return -1;
  }

  int64_t chapter_id = -1;
  q.str("");
  q << "SELECT MAX(id) FROM book_chapters WHERE book_id = " << book_id << ";";
  if (sqlite3_exec(db_, q.str().c_str(), add_book_chapter_cb, &chapter_id,
                   nullptr)) {
    LOG(ERROR) << "add_book_chapter: code=" << sqlite3_errcode(db_) << ": "
               << sqlite3_errmsg(db_) << "\nquery: `" << q.str() << "`";
    return -1;
  }

  return chapter_id;
}

int TaskDB::list_book_chapters_cb(void *out, int /*column_count*/,
                                  char **column_value,
                                  char ** /*column_name*/) {
  BookChapter chapter;
  chapter.book_id = std::stoll(column_value[0]);
  chapter.id = std::stoll(column_value[1]);
  chapter.title = column_value[2];
  ((std::vector<BookChapter> *)out)->emplace_back(chapter);
  return 0;
}

std::vector<BookChapter> TaskDB::list_book_chapters(int64_t book_id) const {
  std::vector<BookChapter> chapters;
  std::ostringstream q;
  q << "SELECT * FROM book_chapters WHERE book_id = " << book_id << ";";

  if (sqlite3_exec(db_, q.str().c_str(), list_book_chapters_cb, &chapters,
                   nullptr)) {
    LOG(ERROR) << "list_book_chapters: code=" << sqlite3_errcode(db_) << ": "
               << sqlite3_errmsg(db_) << "\nquery: `" << q.str() << "`";
    return {};
  }

  return chapters;
}

void TaskDB::add_book_task(int64_t book_id, int64_t chapter_id,
                           int64_t task_id) {
  std::ostringstream q;
  q << "INSERT INTO book_tasks (book_id, chapter_id, task_id) VALUES ("
    << book_id << ", " << chapter_id << ", " << task_id << ");";

  if (sqlite3_exec(db_, q.str().c_str(), nullptr, nullptr, nullptr)) {
    LOG(ERROR) << "add_book_task: code=" << sqlite3_errcode(db_) << ": "
               << sqlite3_errmsg(db_) << "\nquery: `" << q.str() << "`";
  }
}

int TaskDB::list_book_tasks_cb(void *out, int column_count, char **column_value,
                               char **column_name) {
  std::optional<Task> task;
  if (auto ret = get_task_cb(&task, column_count, column_value, column_name)) {
    return ret;
  }
  ((std::vector<Task> *)out)->emplace_back(std::move(*task));
  return 0;
}

std::vector<Task> TaskDB::list_book_tasks(
    int64_t book_id, std::optional<int64_t> chapter_id) const {
  std::vector<Task> tasks;
  std::ostringstream q;
  q << "SELECT * FROM tasks "
    << "INNER JOIN book_tasks ON "
    << "tasks.id = book_tasks.task_id AND "
    << "book_tasks.book_id = " << book_id;
  if (chapter_id) q << " AND book_tasks.chapter_id = " << *chapter_id;
  q << ";";
  if (sqlite3_exec(db_, q.str().c_str(), list_book_tasks_cb, &tasks, nullptr)) {
    LOG(ERROR) << "list_book_tasks: code=" << sqlite3_errcode(db_) << ": "
               << sqlite3_errmsg(db_) << "\nquery: `" << q.str() << "`";
    return {};
  }
  return tasks;
}

TaskVTreeIterator::TaskVTreeIterator(const Task &task) : task_(task) {
  reset();
}

std::optional<AnswerType> TaskVTreeIterator::move(wq::Point p,
                                                  std::string &comment) {
  auto it = cur_->children_.find(p);
  if (it == cur_->children_.end()) return AnswerType::kWrong;

  cur_ = it->second.get();
  comment = cur_->comment_;
  return cur_->answer_;
}

void TaskVTreeIterator::reset() { cur_ = task_.vtree_.get(); }

const char *rank_string(Rank rank) {
  switch (rank) {
    case Rank::kUnknown:
      return "?";
    case Rank::k30K:
      return "30K";
    case Rank::k29K:
      return "29K";
    case Rank::k28K:
      return "28K";
    case Rank::k27K:
      return "27K";
    case Rank::k26K:
      return "26K";
    case Rank::k25K:
      return "25K";
    case Rank::k24K:
      return "24K";
    case Rank::k23K:
      return "23K";
    case Rank::k22K:
      return "22K";
    case Rank::k21K:
      return "21K";
    case Rank::k20K:
      return "20K";
    case Rank::k19K:
      return "19K";
    case Rank::k18K:
      return "18K";
    case Rank::k17K:
      return "17K";
    case Rank::k16K:
      return "16K";
    case Rank::k15K:
      return "15K";
    case Rank::k14K:
      return "14K";
    case Rank::k13K:
      return "13K";
    case Rank::k12K:
      return "12K";
    case Rank::k11K:
      return "11K";
    case Rank::k10K:
      return "10K";
    case Rank::k9K:
      return "9K";
    case Rank::k8K:
      return "8K";
    case Rank::k7K:
      return "7K";
    case Rank::k6K:
      return "6K";
    case Rank::k5K:
      return "5K";
    case Rank::k4K:
      return "4K";
    case Rank::k3K:
      return "3K";
    case Rank::k2K:
      return "2K";
    case Rank::k1K:
      return "1K";
    case Rank::k1D:
      return "1D";
    case Rank::k2D:
      return "2D";
    case Rank::k3D:
      return "3D";
    case Rank::k4D:
      return "4D";
    case Rank::k5D:
      return "5D";
    case Rank::k6D:
      return "6D";
    case Rank::k7D:
      return "7D";
    case Rank::k8D:
      return "8D";
    case Rank::k9D:
      return "9D";
    case Rank::k10D:
      return "10D";
    case Rank::k11D:
      return "11D";
    case Rank::k12D:
      return "12D";
    case Rank::k13D:
      return "13D";
    case Rank::k14D:
      return "14D";
    case Rank::k15D:
      return "15D";
  }
}

const char *task_type_string(TaskType type) {
  switch (type) {
    case TaskType::kUnknown:
      return "Unknown";
    case TaskType::kLifeAndDeath:
      return "Life & Death";
    case TaskType::kTesuji:
      return "Tesuji";
    case TaskType::kJoseki:
      return "Joseki";
    case TaskType::kOpening:
      return "Opening";
    case TaskType::kEndgame:
      return "Endgame";
    case TaskType::kAppreciation:
      return "Appreciation";
    case TaskType::kTrick:
      return "Trick";
    case TaskType::kMiddlegame:
      return "Middlegame";
    case TaskType::kMirror:
      return "Mirror";
    case TaskType::kTheory:
      return "Theory";
    case TaskType::kOpeningChoice:
      return "Opening Choice";
    case TaskType::kMiddlegameChoice:
      return "Middlegame Choice";
    case TaskType::kEndgameChoice:
      return "Endgame Choice";
    case TaskType::kLuoZi:
      return "Pick";
    case TaskType::kCapture:
      return "Capture";
    case TaskType::kCaptureRace:
      return "Capturing Race";
  }
  return "?";
}