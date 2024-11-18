#pragma once

#include "app_context.h"
#include "window.h"

namespace ui {

class StatsWindow : public Window {
 public:
  StatsWindow(AppContext& ctx);
};

}  // namespace ui