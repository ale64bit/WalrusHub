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
  bool first_ = true;

  static void on_row_selected(GtkListBox* self, GtkListBoxRow* row,
                              gpointer user_data);
};

}  // namespace ui