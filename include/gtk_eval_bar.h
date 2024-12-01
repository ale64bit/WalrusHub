#pragma once

#include <gtk/gtk.h>

namespace ui {

class GtkEvalBar {
 public:
  GtkEvalBar();
  void update(double winrate, double score_lead);
  operator GtkWidget*() const { return overlay_; }

 private:
  GtkWidget* overlay_;
  GtkWidget* level_bar_;
  GtkWidget* b_score_lead_label_;
  GtkWidget* w_score_lead_label_;
};

}  // namespace ui