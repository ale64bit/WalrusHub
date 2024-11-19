#pragma once

#include <gtk/gtk.h>

#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include "app_context.h"

namespace ui {

class Window {
 public:
  Window(AppContext& ctx);
  virtual ~Window();

 protected:
  static std::unordered_map<std::string, std::set<Window*>> window_groups_;
  AppContext& ctx_;
  const GtkWidget* window_;

 private:
  static void on_destroy(GtkWidget* self, gpointer user_data);
};

}  // namespace ui