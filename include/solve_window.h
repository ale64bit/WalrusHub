#pragma once

#include <gtk/gtk.h>
#include <sqlite3.h>

#include <memory>
#include <optional>
#include <random>
#include <vector>

#include "app_context.h"
#include "gtk_board.h"
#include "http.h"
#include "task.h"
#include "window.h"
#include "wq.h"

namespace ui {

class SolveWindow : public Window {
 public:
  SolveWindow(AppContext& ctx, SolvePreset preset,
              std::optional<std::pair<int, Rank>> tag_ref);
  SolveWindow(AppContext& ctx, std::string description,
              const std::vector<Task>& tasks, int time_limit_sec);
  ~SolveWindow();

 private:
  void init();

  SolvePreset preset_;
  std::optional<std::pair<int, Rank>> tag_ref_;
  std::vector<int64_t> task_ids_;

  // State
  size_t cur_task_index_ = 0;
  int task_count_ = 0;
  int error_count_ = 0;
  Task task_;
  int move_num_ = 0;
  int time_left_ = 0;
  wq::Color turn_ = wq::Color::kNone;
  std::unique_ptr<TaskVTreeIterator> solve_state_;
  std::set<wq::Point> pending_answer_points_;
  std::unique_ptr<wq::Board> board_;
  std::optional<AnswerType> task_result_;
  bool session_complete_ = false;
  int total_solve_time_ = 0;
  int max_solve_time_ = 0;

  // Widgets
  GtkWidget* rank_label_;
  GtkWidget* turn_label_;
  GtkWidget* comment_label_;
  GtkWidget* task_type_label_;
  std::unique_ptr<GtkBoard> goban_;
  GtkWidget* solve_stats_label_;
  GtkWidget* time_result_label_;
  GtkWidget* reset_button_;
  GtkWidget* next_button_;
  GtkAlertDialog* session_complete_dialog_;

  // Others
  guint timer_source_;
  guint opponent_move_source_ = 0;

  void load_task(int64_t task_id);
  void reset_task(bool is_solved);
  void set_solve_result(AnswerType type);
  void set_time_left_label(int t);
  void on_point_click(int r, int c);
  void on_point_click_normal_task(int r, int c);
  void on_point_click_choose_task(int r, int c);

  static void on_opponent_move(gpointer data);
  static void on_reset_clicked(GtkWidget* widget, gpointer data);
  static void on_next_clicked(GtkWidget* widget, gpointer data);
  static gboolean on_timer_tick(gpointer data);
  static void on_session_complete(GObject* source_object, GAsyncResult* res,
                                  gpointer data);
};

}  // namespace ui