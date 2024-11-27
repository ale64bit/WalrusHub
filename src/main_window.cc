#include "main_window.h"

#include <gtk/gtk.h>

#include "editor_window.h"
#include "git_version.h"
#include "solve_preset_window.h"
#include "sputnik_debug_window.h"
#include "stats_window.h"
#include "task.h"

namespace ui {

MainWindow::MainWindow(AppContext& ctx) : Window(ctx) {
  gtk_window_set_title(GTK_WINDOW(window_), "Walrus Hub");
  gtk_window_set_default_size(GTK_WINDOW(window_), 200, 80);

  GtkWidget* grid = gtk_grid_new();

  GtkWidget* editor_button = gtk_button_new_with_label("Editor");
  g_signal_connect(editor_button, "clicked", G_CALLBACK(on_editor_clicked),
                   this);
  gtk_widget_set_hexpand(GTK_WIDGET(editor_button), true);
  gtk_widget_set_halign(GTK_WIDGET(editor_button), GTK_ALIGN_FILL);

  GtkWidget* solve_button = gtk_button_new_with_label("Training");
  g_signal_connect(solve_button, "clicked", G_CALLBACK(on_solve_clicked), this);
  gtk_widget_set_hexpand(GTK_WIDGET(solve_button), true);
  gtk_widget_set_halign(GTK_WIDGET(solve_button), GTK_ALIGN_FILL);

  GtkWidget* stats_button = gtk_button_new_with_label("Stats");
  g_signal_connect(stats_button, "clicked", G_CALLBACK(on_stats_clicked), this);
  gtk_widget_set_hexpand(GTK_WIDGET(stats_button), true);
  gtk_widget_set_halign(GTK_WIDGET(stats_button), GTK_ALIGN_FILL);

  GtkWidget* about_button = gtk_button_new_with_label("About");
  g_signal_connect(about_button, "clicked", G_CALLBACK(on_about_clicked), this);
  gtk_widget_set_hexpand(GTK_WIDGET(about_button), true);
  gtk_widget_set_halign(GTK_WIDGET(about_button), GTK_ALIGN_FILL);

  gtk_grid_attach(GTK_GRID(grid), editor_button, 0, 0, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), solve_button, 0, 1, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), stats_button, 0, 2, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), about_button, 0, 3, 1, 1);

  gtk_window_set_child(GTK_WINDOW(window_), grid);
  gtk_window_present(GTK_WINDOW(window_));
}

void MainWindow::on_editor_clicked(GtkWidget* /*self*/, gpointer data) {
  MainWindow* win = (MainWindow*)data;
  new EditorWindow(win->ctx_);
}

void MainWindow::on_solve_clicked(GtkWidget* /*self*/, gpointer data) {
  MainWindow* win = (MainWindow*)data;
  new SolvePresetWindow(win->ctx_);
}

void MainWindow::on_stats_clicked(GtkWidget* /*self*/, gpointer data) {
  MainWindow* win = (MainWindow*)data;
  new StatsWindow(win->ctx_);
}

void MainWindow::on_about_clicked(GtkWidget* /*self*/, gpointer data) {
  MainWindow* win = (MainWindow*)data;
  gtk_show_about_dialog(GTK_WINDOW(win->window_), "version", GIT_VERSION,
                        "logo", win->ctx_.logo(), "website-label",
                        "Contribute on GitHub", "website",
                        "https://github.com/ale64bit/WalrusHub", nullptr);
}

}  // namespace ui
