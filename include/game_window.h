#pragma once

#include <memory>

#include "gtk_board.h"
#include "window.h"
#include "wq.h"

namespace ui {

class GameWindow : public Window {
 public:
  GameWindow(AppContext& ctx, int board_size);

 protected:
  enum MoveFlag {
    kMoveFlagNone = 0,
    kMoveFlagVariation = 1,
    kMoveFlagSound = 2,
  };

  GtkBoard& board_widget();
  GtkWidget* navigation_bar_widget();
  wq::Board& board();
  wq::Color turn() const;
  wq::MoveList moves() const;
  void set_turn(wq::Color turn);
  void toggle_turn();
  void set_board_size(int board_size);
  bool move(int r, int c, MoveFlag flags);
  void pass();
  void goto_move(int move_number);
  bool goto_prev_move();
  bool goto_next_move();
  bool goto_mainline();

  virtual void on_point_click(int r, int c) = 0;
  virtual void on_point_enter(int r, int c);
  virtual void on_point_leave(int r, int c);
  virtual void on_board_position_changed() = 0;

 private:
  // State
  wq::Color turn_ = wq::Color::kBlack;
  std::unique_ptr<wq::Board> board_;
  size_t cur_move_ = 0;
  wq::MoveList moves_;
  wq::MoveList variation_moves_;

  // Widgets
  std::unique_ptr<GtkBoard> goban_;
  GtkWidget* navigation_bar_;

  void play_move_sound(size_t capture_count);

  static void on_first_move_clicked(GtkWidget* self, gpointer user_data);
  static void on_prev_n_moves_clicked(GtkWidget* self, gpointer user_data);
  static void on_prev_move_clicked(GtkWidget* self, gpointer user_data);
  static void on_next_move_clicked(GtkWidget* self, gpointer user_data);
  static void on_next_n_moves_clicked(GtkWidget* self, gpointer user_data);
  static void on_last_move_clicked(GtkWidget* self, gpointer user_data);
};

}  // namespace ui