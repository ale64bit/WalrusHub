#include "editor_window.h"

#include <array>
#include <cassert>
#include <cstdlib>

#include "color.h"
#include "log.h"

namespace ui {

static constexpr std::array<const char*, 18> kBoardSizes = {
    "3",  "4",  "5",  "6",  "7",  "8",  "9",  "10", "11",
    "12", "13", "14", "15", "16", "17", "18", "19", nullptr,
};

EditorWindow::EditorWindow(AppContext& ctx) : GameWindow(ctx, 19) {
  gtk_window_set_title(GTK_WINDOW(window_), "Editor");
  gtk_window_set_default_size(GTK_WINDOW(window_), 800, 800);

  GtkWidget* box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);

  GtkWidget* top_left_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
  board_size_dropdown_ = gtk_drop_down_new_from_strings(kBoardSizes.data());
  gtk_drop_down_set_selected(GTK_DROP_DOWN(board_size_dropdown_),
                             kBoardSizes.size() - 2);
  g_signal_connect(GTK_DROP_DOWN(board_size_dropdown_), "notify::selected",
                   G_CALLBACK(on_board_size_changed), this);
  gtk_box_append(GTK_BOX(top_left_box), gtk_label_new("Board size:"));
  gtk_box_append(GTK_BOX(top_left_box), board_size_dropdown_);

  variation_button_ = gtk_toggle_button_new_with_label("Variation");
  g_signal_connect(GTK_TOGGLE_BUTTON(variation_button_), "toggled",
                   G_CALLBACK(on_variation_toggled), this);

  GtkWidget* center_box = gtk_center_box_new();
  gtk_center_box_set_start_widget(GTK_CENTER_BOX(center_box), top_left_box);
  gtk_center_box_set_end_widget(GTK_CENTER_BOX(center_box), variation_button_);

  gtk_box_append(GTK_BOX(box), center_box);
  gtk_box_append(GTK_BOX(box), gtk_separator_new(GTK_ORIENTATION_HORIZONTAL));

  gtk_box_append(GTK_BOX(box), board_widget().widget());

  GtkWidget* action_bar = gtk_action_bar_new();
  gtk_action_bar_set_center_widget(GTK_ACTION_BAR(action_bar),
                                   navigation_bar_widget());
  gtk_box_append(GTK_BOX(box), action_bar);

  gtk_window_set_child(GTK_WINDOW(window_), box);
  gtk_window_present(GTK_WINDOW(window_));
}

void EditorWindow::on_point_click(int r, int c) {
  int flags = kMoveFlagSound;
  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(variation_button_)))
    flags |= kMoveFlagVariation;
  move(r, c, MoveFlag(flags));
}

void EditorWindow::on_board_position_changed() {}

void EditorWindow::on_board_size_changed(GObject* /*self*/,
                                         GParamSpec* /*pspec*/,
                                         gpointer user_data) {
  EditorWindow* win = (EditorWindow*)user_data;
  guint selected =
      gtk_drop_down_get_selected(GTK_DROP_DOWN(win->board_size_dropdown_));
  if (selected == GTK_INVALID_LIST_POSITION) return;

  const int board_size = std::stoi(kBoardSizes[selected]);
  assert(1 <= board_size && board_size <= 19);
  win->set_board_size(board_size);
}

void EditorWindow::on_variation_toggled(GtkToggleButton* self,
                                        gpointer user_data) {
  EditorWindow* win = (EditorWindow*)user_data;
  if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(self)))
    win->goto_mainline();
}

}  // namespace ui
