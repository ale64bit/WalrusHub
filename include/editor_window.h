#pragma once

#include <gtk/gtk.h>

#include <memory>
#include <optional>

#include "app_context.h"
#include "game_window.h"
#include "wq.h"

namespace ui {

class EditorWindow : public GameWindow {
 public:
  EditorWindow(AppContext& ctx);

 protected:
  void on_point_click(int r, int c) override;
  void on_board_position_changed() override;

 private:
  GtkWidget* board_size_dropdown_;
  GtkWidget* variation_button_;

  static void on_board_size_changed(GObject* self, GParamSpec* pspec,
                                    gpointer user_data);
  static void on_variation_toggled(GtkToggleButton* self, gpointer user_data);
};

}  // namespace ui