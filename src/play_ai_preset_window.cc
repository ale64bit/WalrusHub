#include "play_ai_preset_window.h"

#include <array>

#include "play_ai_window.h"

namespace ui {

PlayAIPresetWindow::PlayAIPresetWindow(AppContext& ctx) : Window(ctx) {
  gtk_window_set_title(GTK_WINDOW(window_), "Play AI");
  gtk_window_set_default_size(GTK_WINDOW(window_), 200, 120);

  GtkWidget* box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);

  {
    constexpr std::array<const char*, 3> kStyleStrings = {
        "Pre-AlphaZero",
        "Modern",
        nullptr,
    };
    GtkWidget* style_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    style_dropdown_ = gtk_drop_down_new_from_strings(kStyleStrings.data());
    gtk_box_append(GTK_BOX(style_box), gtk_label_new("Style: "));
    gtk_box_append(GTK_BOX(style_box), style_dropdown_);

    gtk_box_append(GTK_BOX(box), style_box);
  }

  {
    constexpr std::array<const char*, 30> kRankStrings = {
        "20K", "19K", "18K", "17K", "16K", "15K", "14K", "13K", "12K", "11K",
        "10K", "9K",  "8K",  "7K",  "6K",  "5K",  "4K",  "3K",  "2K",  "1K",
        "1D",  "2D",  "3D",  "4D",  "5D",  "6D",  "7D",  "8D",  "9D",  nullptr,
    };
    GtkWidget* rank_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
    rank_dropdown_ = gtk_drop_down_new_from_strings(kRankStrings.data());
    gtk_box_append(GTK_BOX(rank_box), gtk_label_new("Rank: "));
    gtk_box_append(GTK_BOX(rank_box), rank_dropdown_);

    gtk_box_append(GTK_BOX(box), rank_box);
  }

  gtk_box_append(GTK_BOX(box), gtk_separator_new(GTK_ORIENTATION_HORIZONTAL));

  GtkWidget* play_button = gtk_button_new_with_label("Play");
  g_signal_connect(play_button, "clicked", G_CALLBACK(on_play_clicked), this);
  gtk_box_append(GTK_BOX(box), play_button);

  gtk_window_set_child(GTK_WINDOW(window_), box);
  gtk_window_present(GTK_WINDOW(window_));
}

void PlayAIPresetWindow::on_play_clicked(GtkWidget* /*self*/,
                                         gpointer user_data) {
  PlayAIPresetWindow* win = (PlayAIPresetWindow*)user_data;
  auto style = PlayAIWindow::PlayStyle(
      gtk_drop_down_get_selected(GTK_DROP_DOWN(win->style_dropdown_)));
  auto rank = Rank((int)Rank::k20K + gtk_drop_down_get_selected(
                                         GTK_DROP_DOWN(win->rank_dropdown_)));
  new PlayAIWindow(win->ctx_, style, rank);
}

}  // namespace ui