#include "stats_window.h"

#include <gtk/gtk.h>

#include "task.h"

namespace ui {

StatsWindow::StatsWindow(AppContext& ctx) : Window(ctx) {
  gtk_window_set_title(GTK_WINDOW(window_), "Stats");
  gtk_window_set_default_size(GTK_WINDOW(window_), 400, 550);

  GtkWidget* grid = gtk_grid_new();
  gtk_grid_set_row_spacing(GTK_GRID(grid), 8);
  gtk_grid_set_column_spacing(GTK_GRID(grid), 8);

  const auto rank_stats = ctx.stats().get_rank_stats();
  int row = 0;
  for (int rank = (int)Rank::k15K; rank <= (int)Rank::k7D; ++rank) {
    const auto& [total, errors] = rank_stats.at((Rank)rank);
    double pct;
    std::string summary;
    if (total > 0) {
      pct = double(total - errors) / total;
      summary = std::to_string((int)(100 * pct)) + "% (" +
                std::to_string(total - errors) + "/" + std::to_string(total) +
                ")";
    } else {
      pct = 0;
      summary = "-";
    }
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new(rank_string((Rank)rank)), 0,
                    row, 1, 1);
    GtkWidget* level_bar = gtk_level_bar_new();
    gtk_widget_set_hexpand(GTK_WIDGET(level_bar), true);
    gtk_widget_set_vexpand(GTK_WIDGET(level_bar), true);
    gtk_level_bar_set_value(GTK_LEVEL_BAR(level_bar), pct);
    gtk_grid_attach(GTK_GRID(grid), level_bar, 1, row, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new(summary.c_str()), 2, row, 1,
                    1);
    row++;
  }

  gtk_window_set_child(GTK_WINDOW(window_), grid);
  gtk_window_present(GTK_WINDOW(window_));
}

}  // namespace ui