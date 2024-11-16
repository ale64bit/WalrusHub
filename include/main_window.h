#pragma once

#include "app_context.h"
#include "window.h"

namespace ui {

class MainWindow : public Window {
 public:
  MainWindow(AppContext&);

 private:
  static void on_editor_clicked(GtkWidget* widget, gpointer data);
  static void on_solve_clicked(GtkWidget* widget, gpointer data);
};

}  // namespace ui