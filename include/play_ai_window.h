#pragma once

#include <gtk/gtk.h>

#include <memory>

#include "board.h"
#include "gtk_eval_bar.h"
#include "gtkgoban.h"
#include "katago_client.h"
#include "task.h"
#include "window.h"

namespace ui {

class PlayAIWindow : public Window {
  enum class State {
    kPaused,
    kPlaying,
    kCounting,
    kReviewing,
  };

 public:
  enum PlayStyle {
    kPreAlphaZero,
    kModern,
  };

  PlayAIWindow(AppContext& ctx, PlayStyle play_style, Rank rank);

 private:
  // State
  State state_ = State::kPlaying;
  wq::Color my_color_ = wq::Color::kEmpty;
  wq::Color turn_ = wq::Color::kBlack;
  std::unique_ptr<wq::Board> board_;
  size_t cur_move_ = 0;
  wq::MoveList moves_;
  wq::MoveList variation_moves_;
  std::string last_query_id_;
  int consecutive_pass_ = 0;

  // Widgets
  GtkWidget* top_center_box_;
  GtkWidget* navigation_bar_;
  std::unique_ptr<GtkGoban> goban_;
  GtkEvalBar eval_bar_;

  // Analysis
  KataGoClient::Query katago_query_;

  void on_point_click(int r, int c);
  void on_point_enter(int r, int c);
  void on_point_leave(int r, int c);
  void on_pass();
  void gen_move();
  void finish_game(wq::Color winner, double score_lead);
  void evaluate_current_position();
  void toggle_turn();
  bool goto_prev_move();
  bool goto_next_move();

  static void on_pass_clicked(GtkWidget* self, gpointer user_data);
  static void on_resign_clicked(GtkWidget* self, gpointer user_data);
  static void on_autocount_clicked(GtkWidget* self, gpointer user_data);
  static void on_first_move_clicked(GtkWidget* self, gpointer user_data);
  static void on_prev_n_moves_clicked(GtkWidget* self, gpointer user_data);
  static void on_prev_move_clicked(GtkWidget* self, gpointer user_data);
  static void on_next_move_clicked(GtkWidget* self, gpointer user_data);
  static void on_next_n_moves_clicked(GtkWidget* self, gpointer user_data);
  static void on_last_move_clicked(GtkWidget* self, gpointer user_data);
  static void on_show_game_result(GObject* src, GAsyncResult* res,
                                  gpointer user_data);
};

}  // namespace ui