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
  Window(AppContext& ctx, std::string group_name);
  virtual ~Window();

  virtual void update();
  static void update_group(std::string name);

 protected:
  AppContext& ctx_;
  const std::string group_;
  const GtkWidget* window_;

 private:
  static std::unordered_map<std::string, std::set<Window*>> window_groups_;
  static void on_destroy(GtkWidget* self, gpointer user_data);
};

}  // namespace ui