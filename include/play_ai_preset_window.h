#pragma once

#include "window.h"

namespace ui {

class PlayAIPresetWindow : public Window {
 public:
  PlayAIPresetWindow(AppContext& ctx);

 private:
  GtkWidget* style_dropdown_;
  GtkWidget* rank_dropdown_;

  static void on_play_clicked(GtkWidget* self, gpointer user_data);
};

}  // namespace ui