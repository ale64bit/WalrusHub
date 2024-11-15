#pragma once

#include <vector>

#include "app_context.h"
#include "task.h"
#include "window.h"

namespace ui {

class SolvePresetWindow : public Window {
 public:
  SolvePresetWindow(AppContext& ctx);

 private:
  std::vector<SolvePreset> presets_;

  static void on_preset_clicked(GtkWidget* self, gpointer data);
};

}  // namespace ui