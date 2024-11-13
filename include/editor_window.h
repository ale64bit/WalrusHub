#pragma once

#include <gtk/gtk.h>

#include <vector>

#include "app_context.h"
#include "board.h"
#include "gtkgoban.h"
#include "window.h"

namespace ui {

class EditorWindow : public Window {
 public:
  EditorWindow(AppContext& ctx);

 private:
  // State
  wq::Color turn_ = wq::Color::kBlack;
  std::unique_ptr<wq::Board> board_;
  size_t cur_move_ = 0;
  std::vector<wq::Move> moves_;

  // Widgets
  std::unique_ptr<GtkGoban> goban_;
  GtkWidget* board_size_dropdown_;

  void toggle_turn();
  bool goto_prev_move();
  bool goto_next_move();

  static void on_board_size_changed(GObject* self, GParamSpec* pspec,
                                    gpointer user_data);

  static void on_first_move_clicked(GtkWidget* self, gpointer user_data);
  static void on_prev_n_moves_clicked(GtkWidget* self, gpointer user_data);
  static void on_prev_move_clicked(GtkWidget* self, gpointer user_data);
  static void on_next_move_clicked(GtkWidget* self, gpointer user_data);
  static void on_next_n_moves_clicked(GtkWidget* self, gpointer user_data);
  static void on_last_move_clicked(GtkWidget* self, gpointer user_data);
};

}  // namespace ui