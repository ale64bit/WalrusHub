#pragma once

#include <gtk/gtk.h>

#include "app_context.h"

namespace ui {

class Window {
 public:
  Window(AppContext& ctx);
  virtual ~Window();

 protected:
  AppContext& ctx_;
  const GtkWidget* window_;

 private:
  static void on_destroy(GtkWidget* self, gpointer user_data);
};

}  // namespace ui