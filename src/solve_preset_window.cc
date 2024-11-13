#include "solve_preset_window.h"

#include <gtk/gtk.h>

#include <cmath>

#include "log.h"
#include "solve_window.h"

namespace ui {

SolvePresetWindow::SolvePresetWindow(AppContext& ctx) : Window(ctx) {
  gtk_window_set_title(GTK_WINDOW(window_), "Solve Presets");
  gtk_window_set_default_size(GTK_WINDOW(window_), 400, 550);

  GtkWidget* scrolled = gtk_scrolled_window_new();

  GtkWidget* preset_list_box = gtk_list_box_new();

  gtk_list_box_set_selection_mode(GTK_LIST_BOX(preset_list_box),
                                  GTK_SELECTION_SINGLE);
  for (int rank = (int)Rank::k15K; rank <= (int)Rank::k6D; ++rank) {
    SolvePreset preset;
    preset.tags_.push_back("life_and_death");
    preset.tags_.push_back("tesuji");
    preset.tags_.push_back("capture_race");
    preset.tags_.push_back("capture");
    preset.min_rank_ = Rank(rank - 1);
    preset.max_rank_ = Rank(rank);
    preset.time_limit_sec_ = 45;
    preset.max_tasks_ = 10;

    presets_.push_back(preset);

    std::ostringstream ss;
    ss << "Timed Challenge, 45s, " << rank_string(Rank(rank));
    GtkWidget* preset_label = gtk_label_new(ss.str().c_str());
    gtk_list_box_append(GTK_LIST_BOX(preset_list_box), preset_label);
  }
  g_signal_connect(preset_list_box, "row-selected", G_CALLBACK(on_row_selected),
                   this);

  gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled), preset_list_box);
  gtk_window_set_child(GTK_WINDOW(window_), scrolled);
  gtk_window_present(GTK_WINDOW(window_));
}

void SolvePresetWindow::on_row_selected(GtkListBox* /*self*/,
                                        GtkListBoxRow* row,
                                        gpointer user_data) {
  if (!row) return;
  SolvePresetWindow* win = (SolvePresetWindow*)user_data;
  const int index = gtk_list_box_row_get_index(row);
  if (index == 0 && win->first_) {
    win->first_ = false;
    return;
  }
  LOG(INFO) << "row selected: " << index;
  new SolveWindow(win->ctx_, win->presets_[index]);
}

}  // namespace ui