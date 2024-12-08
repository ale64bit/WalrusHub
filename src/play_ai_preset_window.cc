#include "play_ai_preset_window.h"

#include "play_ai_window.h"

namespace ui {

constexpr Rank kMinRank = Rank::k18K;
constexpr Rank kMaxRank = Rank::k9D;

static GtkWidget* new_preset_button(PlayStyle style, Rank rank) {
  GtkWidget* label = gtk_label_new("");
  gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_CENTER);
  GtkWidget* button = gtk_button_new();
  g_object_set_data(G_OBJECT(button), "preset_style", (gpointer)(style));
  g_object_set_data(G_OBJECT(button), "preset_rank", (gpointer)(rank));
  gtk_button_set_child(GTK_BUTTON(button), label);
  return button;
}

PlayAIPresetWindow::PlayAIPresetWindow(AppContext& ctx)
    : Window(ctx, kPlayAIPresetWindowGroup) {
  gtk_window_set_title(GTK_WINDOW(window_), "Play vs AI");
  gtk_window_set_resizable(GTK_WINDOW(window_), false);

  GtkWidget* notebook = gtk_notebook_new();

  for (auto style : {
           PlayStyle::kPreAlphaZero,
           PlayStyle::kModern,
       }) {
    GtkWidget* grid = gtk_grid_new();
    gtk_widget_set_hexpand(GTK_WIDGET(grid), true);
    gtk_widget_set_vexpand(GTK_WIDGET(grid), true);

    // DDK
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("DDK"), 0, 0, 1, 1);
    for (int rank = (int)kMinRank; rank <= (int)Rank::k10K; ++rank) {
      GtkWidget* button = new_preset_button(style, Rank(rank));
      g_signal_connect(button, "clicked", G_CALLBACK(on_preset_clicked), this);
      gtk_grid_attach(GTK_GRID(grid), button, rank - (int)kMinRank + 1, 0, 1,
                      1);
      preset_buttons_[{style, Rank(rank)}] = button;
    }

    // SDK
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("SDK"), 0, 1, 1, 1);
    for (int rank = (int)Rank::k9K; rank <= (int)Rank::k1K; ++rank) {
      GtkWidget* button = new_preset_button(style, Rank(rank));
      g_signal_connect(button, "clicked", G_CALLBACK(on_preset_clicked), this);
      gtk_grid_attach(GTK_GRID(grid), button, rank - (int)Rank::k9K + 1, 1, 1,
                      1);
      preset_buttons_[{style, Rank(rank)}] = button;
    }

    // DAN
    gtk_grid_attach(GTK_GRID(grid), gtk_label_new("DAN"), 0, 2, 1, 1);
    for (int rank = (int)Rank::k1D; rank <= (int)Rank::k9D; ++rank) {
      GtkWidget* button = new_preset_button(style, Rank(rank));
      g_signal_connect(button, "clicked", G_CALLBACK(on_preset_clicked), this);
      gtk_grid_attach(GTK_GRID(grid), button, rank - (int)Rank::k1D + 1, 2, 1,
                      1);
      preset_buttons_[{style, Rank(rank)}] = button;
    }

    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), grid,
                             gtk_label_new(play_style_string(style)));
  }
  update_preset_buttons();
  gtk_window_set_child(GTK_WINDOW(window_), notebook);
  gtk_window_present(GTK_WINDOW(window_));
}

void PlayAIPresetWindow::update() { update_preset_buttons(); }

void PlayAIPresetWindow::update_preset_buttons() {
  const auto stats = ctx_.stats().get_play_ai_stats();
  for (auto style : {
           PlayStyle::kPreAlphaZero,
           PlayStyle::kModern,
       }) {
    for (int rank = (int)kMinRank; rank <= (int)kMaxRank; ++rank) {
      const auto k = std::make_pair(style, Rank(rank));
      GtkWidget* button = preset_buttons_.at(k);
      GtkWidget* label = gtk_button_get_child(GTK_BUTTON(button));
      const auto& [wins, losses] = stats.at(k);

      const bool unlocked = (Rank(rank) == kMinRank ||
                             stats.at({style, Rank(rank - 1)}).first > 0);

      std::ostringstream ss;
      if (unlocked) {
        ss << "<span size=\"x-large\">" << rank_string(Rank(rank))
           << u8"</span>\n✓" << wins << "\tx" << losses << "\n"
           << 100 * wins / std::max(wins + losses, 1) << "%";
      } else {
        ss << "<span size=\"x-large\">" << rank_string(Rank(rank))
           << u8"</span>\n✓-\tx-\n-%";
      }
      gtk_label_set_markup(GTK_LABEL(label), ss.str().c_str());
      gtk_widget_set_sensitive(GTK_WIDGET(button), unlocked);
    }
  }
}

void PlayAIPresetWindow::on_preset_clicked(GtkWidget* self, gpointer data) {
  PlayAIPresetWindow* win = (PlayAIPresetWindow*)data;
  const PlayStyle style =
      (PlayStyle)((size_t)g_object_get_data(G_OBJECT(self), "preset_style"));
  const Rank rank =
      (Rank)((size_t)g_object_get_data(G_OBJECT(self), "preset_rank"));
  new PlayAIWindow(win->ctx_, style, rank, true);
}

}  // namespace ui