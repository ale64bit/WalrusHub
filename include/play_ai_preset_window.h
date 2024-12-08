#pragma once

#include <map>

#include "katago_client.h"
#include "task.h"
#include "window.h"

namespace ui {

class PlayAIPresetWindow : public Window {
 public:
  static constexpr const char* kPlayAIPresetWindowGroup = "PlayAIPresetWindow";

  PlayAIPresetWindow(AppContext& ctx);
  void update() override;

 private:
  std::map<std::pair<PlayStyle, Rank>, GtkWidget*> preset_buttons_;

  void update_preset_buttons();

  static void on_preset_clicked(GtkWidget* self, gpointer data);
};

}  // namespace ui