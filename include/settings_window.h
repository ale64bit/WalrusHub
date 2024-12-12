#pragma once

#include "window.h"

namespace ui {

class SettingsWindow : public Window {
 public:
  SettingsWindow(AppContext& ctx);

 private:
  GtkWidget* theme_dropdown_;
  GtkWidget* katago_path_entry_;
  GtkWidget* katago_config_path_entry_;
  GtkWidget* katago_model_path_entry_;
  GtkWidget* katago_human_model_path_entry_;
  void (SettingsWindow::*cur_change_fun_)(GFile*);

  void on_change_katago_path(GFile* f);
  void on_change_katago_config_path(GFile* f);
  void on_change_katago_model_path(GFile* f);
  void on_change_katago_human_model_path(GFile* f);

  static void on_theme_selected(GObject* self, GParamSpec* pspec,
                                gpointer user_data);
  static void on_file_selected(GObject* source_object, GAsyncResult* res,
                               gpointer user_data);
  static void on_change_katago_path_clicked(GtkWidget* self,
                                            gpointer user_data);
  static void on_change_katago_config_path_clicked(GtkWidget* self,
                                                   gpointer user_data);
  static void on_change_katago_model_path_clicked(GtkWidget* self,
                                                  gpointer user_data);
  static void on_change_katago_human_model_path_clicked(GtkWidget* self,
                                                        gpointer user_data);
};

}  // namespace ui