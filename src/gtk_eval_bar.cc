#include "gtk_eval_bar.h"

#include <cmath>
#include <iomanip>
#include <sstream>
#include <string>

#include "log.h"

namespace ui {

static std::string score_lead_string(double score_lead) {
  std::stringstream ss;
  if (score_lead > 0) {
    ss << "B + " << std::fixed << std::setprecision(1) << score_lead;
  } else {
    ss << "W + " << std::fixed << std::setprecision(1) << std::abs(score_lead);
  }
  return ss.str();
}

GtkEvalBar::GtkEvalBar() {
  level_bar_ = gtk_level_bar_new();
  gtk_widget_add_css_class(GTK_WIDGET(level_bar_), "eval_bar");
  gtk_widget_set_size_request(GTK_WIDGET(level_bar_), 200, 20);
  gtk_level_bar_set_value(GTK_LEVEL_BAR(level_bar_), 0.5);
  gtk_level_bar_set_mode(GTK_LEVEL_BAR(level_bar_),
                         GTK_LEVEL_BAR_MODE_CONTINUOUS);
  b_score_lead_label_ = gtk_label_new("B + 0.5");
  w_score_lead_label_ = gtk_label_new("W + 0.5");
  gtk_widget_set_visible(GTK_WIDGET(b_score_lead_label_), false);
  GtkWidget* score_lead_label_center_box = gtk_center_box_new();
  gtk_center_box_set_start_widget(GTK_CENTER_BOX(score_lead_label_center_box),
                                  b_score_lead_label_);
  GtkWidget* separator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
  gtk_widget_add_css_class(GTK_WIDGET(separator), "eval_bar_separator");
  gtk_center_box_set_center_widget(GTK_CENTER_BOX(score_lead_label_center_box),
                                   separator);
  gtk_center_box_set_end_widget(GTK_CENTER_BOX(score_lead_label_center_box),
                                w_score_lead_label_);
  overlay_ = gtk_overlay_new();
  gtk_overlay_set_child(GTK_OVERLAY(overlay_), level_bar_);
  gtk_overlay_add_overlay(GTK_OVERLAY(overlay_), score_lead_label_center_box);
}

void GtkEvalBar::update(double winrate, double score_lead) {
  gtk_level_bar_set_value(GTK_LEVEL_BAR(level_bar_), winrate);
  if (score_lead > 0) {
    gtk_label_set_markup(
        GTK_LABEL(b_score_lead_label_),
        ("<span size=\"large\" weight=\"bold\" foreground=\"white\">" +
         score_lead_string(score_lead) + "</span>")
            .c_str());
    gtk_widget_set_visible(b_score_lead_label_, true);
    gtk_widget_set_visible(w_score_lead_label_, false);
  } else {
    gtk_label_set_markup(
        GTK_LABEL(w_score_lead_label_),
        ("<span size=\"large\" weight=\"bold\" foreground=\"black\">" +
         score_lead_string(score_lead) + "</span>")
            .c_str());
    gtk_widget_set_visible(b_score_lead_label_, false);
    gtk_widget_set_visible(w_score_lead_label_, true);
  }
}

}  // namespace ui