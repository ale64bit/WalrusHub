#pragma once

#include <map>
#include <unordered_map>
#include <vector>

#include "app_context.h"
#include "task.h"
#include "window.h"

namespace ui {

class SolvePresetWindow : public Window {
 public:
  static constexpr const char* kSolvePresetWindowGroup = "solve_preset";

  SolvePresetWindow(AppContext& ctx);
  ~SolvePresetWindow();

  void update_preset_buttons();

 private:
  std::vector<std::pair<SolvePreset, std::pair<int, Rank>>> presets_;
  std::unordered_map<int, std::map<Rank, std::vector<GtkWidget*>>>
      preset_buttons_;

  void update_time_challenge_button_label(
      GtkWidget* button, int tag_id, Rank rank,
      const std::map<Rank, std::pair<int, int>>& stats);
  static void on_preset_clicked(GtkWidget* self, gpointer data);
};

}  // namespace ui