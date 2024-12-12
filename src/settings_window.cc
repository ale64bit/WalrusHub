#include "settings_window.h"

#include <array>
#include <functional>

#include "log.h"

namespace ui {

constexpr std::array<const char*, 3> kThemes = {"Light", "Dark", nullptr};

SettingsWindow::SettingsWindow(AppContext& ctx) : Window(ctx) {
  gtk_window_set_title(GTK_WINDOW(window_), "Settings");
  gtk_window_set_default_size(GTK_WINDOW(window_), 600, 300);

  GtkWidget* box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);

  GtkWidget* stack_sidebar = gtk_stack_sidebar_new();
  GtkWidget* stack = gtk_stack_new();
  gtk_box_append(GTK_BOX(box), stack_sidebar);
  gtk_box_append(GTK_BOX(box), stack);
  gtk_stack_sidebar_set_stack(GTK_STACK_SIDEBAR(stack_sidebar),
                              GTK_STACK(stack));

  {
    GtkWidget* appearance_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    gtk_stack_add_titled(GTK_STACK(stack), appearance_box, nullptr,
                         "Appearance");

    GtkWidget* theme_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    theme_dropdown_ = gtk_drop_down_new_from_strings(kThemes.data());
    g_signal_connect(GTK_DROP_DOWN(theme_dropdown_), "notify::selected",
                     G_CALLBACK(on_theme_selected), this);
    gtk_box_append(GTK_BOX(theme_box), gtk_label_new("Theme"));

    gtk_drop_down_set_selected(GTK_DROP_DOWN(theme_dropdown_),
                               ctx_.get_appearance_theme_dark() ? 1 : 0);
    gtk_box_append(GTK_BOX(theme_box), theme_dropdown_);

    gtk_box_append(GTK_BOX(appearance_box), theme_box);
  }
  {
    GtkWidget* katago_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    gtk_stack_add_titled(GTK_STACK(stack), katago_box, nullptr, "KataGo");

    {
      GtkWidget* katago_path_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
      katago_path_entry_ = gtk_entry_new();
      GtkWidget* katago_path_button = gtk_button_new_with_label("Change");
      gtk_editable_set_editable(GTK_EDITABLE(katago_path_entry_), false);
      gtk_widget_set_can_focus(GTK_WIDGET(katago_path_entry_), false);
      gtk_widget_set_hexpand(GTK_WIDGET(katago_path_entry_), true);
      gtk_box_append(GTK_BOX(katago_path_box),
                     gtk_label_new("KataGo executable: "));
      gtk_box_append(GTK_BOX(katago_path_box), katago_path_entry_);
      gtk_box_append(GTK_BOX(katago_path_box), katago_path_button);
      gtk_box_append(GTK_BOX(katago_box), katago_path_box);
      if (auto p = ctx_.get_katago_path()) {
        auto buf = gtk_entry_get_buffer(GTK_ENTRY(katago_path_entry_));
        gtk_entry_buffer_set_text(GTK_ENTRY_BUFFER(buf), p, -1);
      }
      g_signal_connect(katago_path_button, "clicked",
                       G_CALLBACK(on_change_katago_path_clicked), this);
    }

    {
      GtkWidget* katago_config_path_box =
          gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
      katago_config_path_entry_ = gtk_entry_new();
      GtkWidget* katago_config_path_button =
          gtk_button_new_with_label("Change");
      gtk_editable_set_editable(GTK_EDITABLE(katago_config_path_entry_), false);
      gtk_widget_set_can_focus(GTK_WIDGET(katago_config_path_entry_), false);
      gtk_widget_set_hexpand(GTK_WIDGET(katago_config_path_entry_), true);
      gtk_box_append(GTK_BOX(katago_config_path_box),
                     gtk_label_new("Config file: "));
      gtk_box_append(GTK_BOX(katago_config_path_box),
                     katago_config_path_entry_);
      gtk_box_append(GTK_BOX(katago_config_path_box),
                     katago_config_path_button);
      gtk_box_append(GTK_BOX(katago_box), katago_config_path_box);
      if (auto p = ctx_.get_katago_config_path()) {
        auto buf = gtk_entry_get_buffer(GTK_ENTRY(katago_config_path_entry_));
        gtk_entry_buffer_set_text(GTK_ENTRY_BUFFER(buf), p, -1);
      }
      g_signal_connect(katago_config_path_button, "clicked",
                       G_CALLBACK(on_change_katago_config_path_clicked), this);
    }

    {
      GtkWidget* katago_model_path_box =
          gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
      katago_model_path_entry_ = gtk_entry_new();
      GtkWidget* katago_model_path_button = gtk_button_new_with_label("Change");
      gtk_editable_set_editable(GTK_EDITABLE(katago_model_path_entry_), false);
      gtk_widget_set_can_focus(GTK_WIDGET(katago_model_path_entry_), false);
      gtk_widget_set_hexpand(GTK_WIDGET(katago_model_path_entry_), true);
      gtk_box_append(GTK_BOX(katago_model_path_box),
                     gtk_label_new("Model file: "));
      gtk_box_append(GTK_BOX(katago_model_path_box), katago_model_path_entry_);
      gtk_box_append(GTK_BOX(katago_model_path_box), katago_model_path_button);
      gtk_box_append(GTK_BOX(katago_box), katago_model_path_box);
      if (auto p = ctx_.get_katago_model_path()) {
        auto buf = gtk_entry_get_buffer(GTK_ENTRY(katago_model_path_entry_));
        gtk_entry_buffer_set_text(GTK_ENTRY_BUFFER(buf), p, -1);
      }
      g_signal_connect(katago_model_path_button, "clicked",
                       G_CALLBACK(on_change_katago_model_path_clicked), this);
    }

    {
      GtkWidget* katago_human_model_path_box =
          gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
      katago_human_model_path_entry_ = gtk_entry_new();
      GtkWidget* katago_human_model_path_button =
          gtk_button_new_with_label("Change");
      gtk_editable_set_editable(GTK_EDITABLE(katago_human_model_path_entry_),
                                false);
      gtk_widget_set_can_focus(GTK_WIDGET(katago_human_model_path_entry_),
                               false);
      gtk_widget_set_hexpand(GTK_WIDGET(katago_human_model_path_entry_), true);
      gtk_box_append(GTK_BOX(katago_human_model_path_box),
                     gtk_label_new("Human model file: "));
      gtk_box_append(GTK_BOX(katago_human_model_path_box),
                     katago_human_model_path_entry_);
      gtk_box_append(GTK_BOX(katago_human_model_path_box),
                     katago_human_model_path_button);
      gtk_box_append(GTK_BOX(katago_box), katago_human_model_path_box);
      if (auto p = ctx_.get_katago_human_model_path()) {
        auto buf =
            gtk_entry_get_buffer(GTK_ENTRY(katago_human_model_path_entry_));
        gtk_entry_buffer_set_text(GTK_ENTRY_BUFFER(buf), p, -1);
      }
      g_signal_connect(katago_human_model_path_button, "clicked",
                       G_CALLBACK(on_change_katago_human_model_path_clicked),
                       this);
    }
  }

  gtk_window_set_child(GTK_WINDOW(window_), box);
  gtk_window_present(GTK_WINDOW(window_));
}

void SettingsWindow::on_theme_selected(GObject* /*self*/, GParamSpec* /*pspec*/,
                                       gpointer user_data) {
  SettingsWindow* win = (SettingsWindow*)user_data;
  guint selected =
      gtk_drop_down_get_selected(GTK_DROP_DOWN(win->theme_dropdown_));
  win->ctx_.set_appearance_theme_dark(selected);
  g_object_set(gtk_settings_get_default(), "gtk-application-prefer-dark-theme",
               selected, nullptr);
}

void SettingsWindow::on_change_katago_path(GFile* f) {
  auto p = g_file_get_path(f);
  ctx_.set_katago_path(p);
  auto buf = gtk_entry_get_buffer(GTK_ENTRY(katago_path_entry_));
  gtk_entry_buffer_set_text(GTK_ENTRY_BUFFER(buf), p, -1);
}

void SettingsWindow::on_change_katago_config_path(GFile* f) {
  auto p = g_file_get_path(f);
  ctx_.set_katago_config_path(p);
  auto buf = gtk_entry_get_buffer(GTK_ENTRY(katago_config_path_entry_));
  gtk_entry_buffer_set_text(GTK_ENTRY_BUFFER(buf), p, -1);
}

void SettingsWindow::on_change_katago_model_path(GFile* f) {
  auto p = g_file_get_path(f);
  ctx_.set_katago_model_path(p);
  auto buf = gtk_entry_get_buffer(GTK_ENTRY(katago_model_path_entry_));
  gtk_entry_buffer_set_text(GTK_ENTRY_BUFFER(buf), p, -1);
}

void SettingsWindow::on_change_katago_human_model_path(GFile* f) {
  auto p = g_file_get_path(f);
  ctx_.set_katago_human_model_path(p);
  auto buf = gtk_entry_get_buffer(GTK_ENTRY(katago_human_model_path_entry_));
  gtk_entry_buffer_set_text(GTK_ENTRY_BUFFER(buf), p, -1);
}

void SettingsWindow::on_file_selected(GObject* src, GAsyncResult* res,
                                      gpointer user_data) {
  SettingsWindow* win = (SettingsWindow*)user_data;
  GFile* f = gtk_file_dialog_open_finish(GTK_FILE_DIALOG(src), res, nullptr);
  if (!f) return;
  (win->*(win->cur_change_fun_))(f);
}

void SettingsWindow::on_change_katago_path_clicked(GtkWidget* /*self*/,
                                                   gpointer user_data) {
  SettingsWindow* win = (SettingsWindow*)user_data;
  win->cur_change_fun_ = &SettingsWindow::on_change_katago_path;
  GtkFileDialog* dialog = gtk_file_dialog_new();
  gtk_file_dialog_set_modal(GTK_FILE_DIALOG(dialog), true);
  gtk_file_dialog_open(dialog, GTK_WINDOW(win->window_), nullptr,
                       on_file_selected, user_data);
  g_object_unref(dialog);
}

void SettingsWindow::on_change_katago_config_path_clicked(GtkWidget* /*self*/,
                                                          gpointer user_data) {
  SettingsWindow* win = (SettingsWindow*)user_data;
  win->cur_change_fun_ = &SettingsWindow::on_change_katago_config_path;
  GtkFileDialog* dialog = gtk_file_dialog_new();
  gtk_file_dialog_set_modal(GTK_FILE_DIALOG(dialog), true);
  gtk_file_dialog_open(dialog, GTK_WINDOW(win->window_), nullptr,
                       on_file_selected, user_data);
  g_object_unref(dialog);
}

void SettingsWindow::on_change_katago_model_path_clicked(GtkWidget* /*self*/,
                                                         gpointer user_data) {
  SettingsWindow* win = (SettingsWindow*)user_data;
  win->cur_change_fun_ = &SettingsWindow::on_change_katago_model_path;
  GtkFileDialog* dialog = gtk_file_dialog_new();
  gtk_file_dialog_set_modal(GTK_FILE_DIALOG(dialog), true);
  gtk_file_dialog_open(dialog, GTK_WINDOW(win->window_), nullptr,
                       on_file_selected, user_data);
  g_object_unref(dialog);
}

void SettingsWindow::on_change_katago_human_model_path_clicked(
    GtkWidget* /*self*/, gpointer user_data) {
  SettingsWindow* win = (SettingsWindow*)user_data;
  win->cur_change_fun_ = &SettingsWindow::on_change_katago_human_model_path;
  GtkFileDialog* dialog = gtk_file_dialog_new();
  gtk_file_dialog_set_modal(GTK_FILE_DIALOG(dialog), true);
  gtk_file_dialog_open(dialog, GTK_WINDOW(win->window_), nullptr,
                       on_file_selected, user_data);
  g_object_unref(dialog);
}

}  // namespace ui