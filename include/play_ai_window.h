#pragma once

#include <gtk/gtk.h>

#include <memory>

#include "app_context.h"
#include "board.h"
#include "game_window.h"
#include "gtk_eval_bar.h"
#include "gtkgoban.h"
#include "katago_client.h"
#include "task.h"

namespace ui {

class PlayAIWindow : public GameWindow {
  enum class State {
    kPaused,
    kPlaying,
    kCounting,
    kReviewing,
  };

 public:
  PlayAIWindow(AppContext& ctx, PlayStyle play_style, Rank rank, bool ranked);

 protected:
  void on_point_click(int r, int c) override;
  void on_point_enter(int r, int c) override;
  void on_point_leave(int r, int c) override;
  void on_board_position_changed() override;

 private:
  const PlayStyle play_style_;
  const Rank rank_;
  const bool ranked_;

  // State
  State state_ = State::kPlaying;
  wq::Color my_color_ = wq::Color::kNone;
  std::string last_query_id_;
  int consecutive_pass_ = 0;
  int consecutive_resign_checks_ = 0;

  // Widgets
  GtkWidget* top_center_box_;
  GtkWidget* navigation_bar_;
  GtkEvalBar eval_bar_;
  GtkWidget* show_ai_variation_button_;

  // Analysis
  KataGoClient::Query katago_query_;
  KataGoClient::Response katago_last_resp_;

  void on_pass();
  void gen_move();
  bool should_resign(const KataGoClient::Response& resp);
  void finish_game(wq::Color winner, double score_lead);
  void evaluate_current_position();

  static void on_pass_clicked(GtkWidget* self, gpointer user_data);
  static void on_resign_clicked(GtkWidget* self, gpointer user_data);
  static void on_autocount_clicked(GtkWidget* self, gpointer user_data);
  static void on_show_ai_variation_clicked(GtkWidget* self, gpointer user_data);
  static void on_show_game_result(GObject* src, GAsyncResult* res,
                                  gpointer user_data);
};

}  // namespace ui