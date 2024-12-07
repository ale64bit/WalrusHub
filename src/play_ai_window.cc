#include "play_ai_window.h"

#include <cctype>
#include <random>
#include <sstream>

#include "board.h"
#include "color.h"
#include "log.h"

namespace ui {

static const char* style_string(PlayAIWindow::PlayStyle style) {
  switch (style) {
    case PlayAIWindow::PlayStyle::kPreAlphaZero:
      return "Pre-AlphaZero";
    case PlayAIWindow::PlayStyle::kModern:
      return "Modern";
  }
  return "?";
}

static int max_visits_for_rank(Rank rank) {
  if (rank < Rank::k5D) return 1;
  return 3 * ((int)rank - int(Rank::k4D));
}

PlayAIWindow::PlayAIWindow(AppContext& ctx, PlayStyle play_style, Rank rank)
    : GameWindow(ctx, 19), play_style_(play_style), rank_(rank) {
  std::ostringstream title;
  title << "Play vs AI (" << style_string(play_style_) << ' '
        << rank_string(rank_) << ")";

  gtk_window_set_title(GTK_WINDOW(window_), title.str().c_str());

  GtkWidget* box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);

  eval_bar_.update(0.5, 0);
  top_center_box_ = gtk_center_box_new();
  GtkWidget* game_action_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
  GtkWidget* pass_button = gtk_button_new_with_label("Pass");
  GtkWidget* resign_button = gtk_button_new_with_label("Resign");
  GtkWidget* autocount_button = gtk_button_new_with_label("Auto-Count");
  gtk_box_append(GTK_BOX(game_action_box), pass_button);
  gtk_box_append(GTK_BOX(game_action_box), resign_button);
  gtk_box_append(GTK_BOX(game_action_box), autocount_button);
  gtk_center_box_set_center_widget(GTK_CENTER_BOX(top_center_box_),
                                   game_action_box);
  g_signal_connect(pass_button, "clicked", G_CALLBACK(on_pass_clicked), this);
  g_signal_connect(resign_button, "clicked", G_CALLBACK(on_resign_clicked),
                   this);
  g_signal_connect(autocount_button, "clicked",
                   G_CALLBACK(on_autocount_clicked), this);
  gtk_box_append(GTK_BOX(box), top_center_box_);
  gtk_box_append(GTK_BOX(box), gtk_separator_new(GTK_ORIENTATION_HORIZONTAL));

  show_ai_variation_button_ = gtk_button_new_with_label("Show AI Variation");
  g_signal_connect(show_ai_variation_button_, "clicked",
                   G_CALLBACK(on_show_ai_variation_clicked), this);

  gtk_box_append(GTK_BOX(box), board_widget().widget());

  navigation_bar_ = gtk_action_bar_new();
  gtk_action_bar_set_center_widget(GTK_ACTION_BAR(navigation_bar_),
                                   navigation_bar_widget());
  gtk_box_append(GTK_BOX(box), navigation_bar_);
  gtk_widget_set_visible(GTK_WIDGET(navigation_bar_), false);

  gtk_window_set_child(GTK_WINDOW(window_), box);
  gtk_window_present(GTK_WINDOW(window_));

  std::string human_profile;
  switch (play_style_) {
    case PlayStyle::kPreAlphaZero:
      human_profile += "preaz_";
      break;
    case PlayStyle::kModern:
      human_profile += "rank_";
      break;
  }
  human_profile += rank_string(rank);
  human_profile.back() = std::tolower(human_profile.back());

  ctx_.katago()->run();
  katago_query_.rules = "chinese";
  katago_query_.komi = 7.5;
  katago_query_.board_size_rows = 19;
  katago_query_.board_size_cols = 19;
  katago_query_.max_visits = max_visits_for_rank(rank);
  katago_query_.include_policy = true;
  katago_query_.override_settings = {
      {"humanSLProfile", human_profile},
      {"ignorePreRootHistory", false},
      {"rootNumSymmetriesToSample", 2},
  };

  std::bernoulli_distribution dist;
  my_color_ = dist(ctx_.rand()) ? wq::Color::kBlack : wq::Color::kWhite;
  if (my_color_ == wq::Color::kWhite) gen_move();
}

void PlayAIWindow::on_point_click(int r, int c) {
  if (state_ != State::kPlaying && state_ != State::kReviewing) return;
  if (state_ == State::kPlaying && turn() != my_color_) return;

  int flags = kMoveFlagSound;
  if (state_ != State::kPlaying) flags |= kMoveFlagVariation;
  if (!move(r, c, MoveFlag(flags))) return;

  consecutive_pass_ = 0;

  switch (state_) {
    case State::kPlaying:
      gen_move();
      break;
    case State::kReviewing:
      evaluate_current_position();
      break;
    default:
      break;
  }
}

void PlayAIWindow::on_point_enter(int r, int c) {
  if (state_ == State::kCounting) return;
  GameWindow::on_point_enter(r, c);
}

void PlayAIWindow::on_point_leave(int r, int c) {
  if (state_ == State::kCounting) return;
  GameWindow::on_point_leave(r, c);
}

void PlayAIWindow::on_board_position_changed() {
  if (state_ != State::kPlaying) evaluate_current_position();
}

void PlayAIWindow::on_pass() {
  if (state_ != State::kPlaying && state_ != State::kReviewing) return;
  if (state_ == State::kPlaying && turn() != my_color_) return;

  pass();
  consecutive_pass_++;
  if (state_ == State::kPlaying && consecutive_pass_ == 2)
    finish_game(wq::Color::kNone, 0);

  switch (state_) {
    case State::kPlaying:
      gen_move();
      break;
    case State::kReviewing:
      evaluate_current_position();
      break;
    default:
      break;
  }
}

void PlayAIWindow::gen_move() {
  if (!last_query_id_.empty()) ctx_.katago()->cancel_query(last_query_id_);
  katago_query_.moves = moves();
  last_query_id_ = ctx_.katago()->query(
      katago_query_,
      [this](KataGoClient::Response resp, std::optional<std::string> error) {
        if (error) {
          LOG(ERROR) << "katago: " << *error;
          return;
        }

        for (double& p : resp.human_policy) p = std::clamp(p, 0., 1.);
        std::discrete_distribution<> dist(resp.human_policy.begin(),
                                          resp.human_policy.end());
        const int move_index = dist(ctx_.rand());
        if (move_index == (int)resp.human_policy.size() - 1) {
          pass();
          consecutive_pass_++;
        } else {
          const int r = move_index / 19;
          const int c = move_index % 19;
          move(r, c, kMoveFlagNone);
        }
        if (consecutive_pass_ == 2) finish_game(wq::Color::kNone, 0);
      });
}

void PlayAIWindow::evaluate_current_position() {
  if (!last_query_id_.empty()) ctx_.katago()->cancel_query(last_query_id_);
  katago_query_.moves = moves();
  last_query_id_ = ctx_.katago()->query(
      katago_query_,
      [this](KataGoClient::Response resp, std::optional<std::string> error) {
        if (error) {
          LOG(ERROR) << "katago: " << *error;
          return;
        }
        katago_last_resp_ = resp;
        eval_bar_.update(resp.root_info.winrate, resp.root_info.score_lead);
      });
}

void PlayAIWindow::finish_game(wq::Color winner, double score_lead) {
  // Reset engine settings for analysis
  katago_query_.max_visits.reset();
  katago_query_.include_policy = false;
  katago_query_.override_settings = json::object();

  // If there's no winner, we need to count first.
  if (winner == wq::Color::kNone) {
    state_ = State::kCounting;
    if (!last_query_id_.empty()) ctx_.katago()->cancel_query(last_query_id_);
    katago_query_.include_ownership = true;
    katago_query_.moves = moves();
    last_query_id_ = ctx_.katago()->query(
        katago_query_,
        [this](KataGoClient::Response resp, std::optional<std::string> error) {
          if (error) {
            LOG(ERROR) << "katago: counting: " << *error;
            return;
          }

          const double score_lead =
              std::round(resp.root_info.score_lead + 0.5) - 0.5;
          const wq::Color winner =
              (score_lead > 0) ? wq::Color::kBlack : wq::Color::kWhite;
          for (int i = 0; i < 19; ++i) {
            for (int j = 0; j < 19; ++j) {
              const double t = resp.ownership[i * 19 + j];
              if (t > 0.9) {
                switch (board().at(i, j)) {
                  case wq::Color::kNone:
                    board_widget().set_annotation(i, j,
                                                  AnnotationType::kTerritory);
                    board_widget().set_annotation_color(i, j, color_black);
                    break;
                  case wq::Color::kBlack:
                    board_widget().set_annotation(i, j, AnnotationType::kNone);
                    break;
                  case wq::Color::kWhite:
                    board_widget().set_annotation(i, j,
                                                  AnnotationType::kTerritory);
                    board_widget().set_annotation_color(i, j, color_black);
                    break;
                }
              } else if (t < -0.9) {
                switch (board().at(i, j)) {
                  case wq::Color::kNone:
                    board_widget().set_annotation(i, j,
                                                  AnnotationType::kTerritory);
                    board_widget().set_annotation_color(i, j, color_white);
                    break;
                  case wq::Color::kBlack:
                    board_widget().set_annotation(i, j,
                                                  AnnotationType::kTerritory);
                    board_widget().set_annotation_color(i, j, color_white);
                    break;
                  case wq::Color::kWhite:
                    board_widget().set_annotation(i, j, AnnotationType::kNone);
                    break;
                }
              } else {
                board_widget().set_annotation(i, j, AnnotationType::kNone);
              }
            }
          }

          finish_game(winner, std::abs(score_lead));
        });
    return;
  }

  std::ostringstream game_res;
  switch (winner) {
    case wq::Color::kNone:
      game_res << "Draw";
      break;
    default:
      game_res << wq::ColorShortString(winner) << "+";
      if (score_lead == 0)
        game_res << "R";
      else
        game_res << std::fixed << std::setprecision(1) << score_lead;
      break;
  }
  GtkAlertDialog* dialog = gtk_alert_dialog_new("%s", game_res.str().c_str());
  gtk_alert_dialog_set_modal(GTK_ALERT_DIALOG(dialog), true);
  gtk_alert_dialog_choose(dialog, GTK_WINDOW(window_), nullptr,
                          on_show_game_result, this);
  g_object_unref(dialog);
}

void PlayAIWindow::on_show_game_result(GObject* src, GAsyncResult* res,
                                       gpointer user_data) {
  PlayAIWindow* win = (PlayAIWindow*)user_data;
  win->state_ = State::kReviewing;
  gtk_center_box_set_center_widget(GTK_CENTER_BOX(win->top_center_box_),
                                   win->eval_bar_);
  gtk_center_box_set_end_widget(GTK_CENTER_BOX(win->top_center_box_),
                                win->show_ai_variation_button_);
  gtk_widget_set_visible(GTK_WIDGET(win->navigation_bar_), true);

  for (int i = 0; i < 19; ++i) {
    for (int j = 0; j < 19; ++j) {
      win->board_widget().set_annotation(i, j, AnnotationType::kNone);
    }
  }

  // Evaluate current position
  win->katago_query_.include_ownership = false;
  win->katago_query_.override_settings = json::object();
  win->evaluate_current_position();

  gtk_alert_dialog_choose_finish(GTK_ALERT_DIALOG(src), res, nullptr);
}

void PlayAIWindow::on_pass_clicked(GtkWidget* /*self*/, gpointer user_data) {
  PlayAIWindow* win = (PlayAIWindow*)user_data;
  win->on_pass();
}

void PlayAIWindow::on_resign_clicked(GtkWidget* /*self*/, gpointer user_data) {
  PlayAIWindow* win = (PlayAIWindow*)user_data;
  win->finish_game(win->my_color_ == wq::Color::kBlack ? wq::Color::kWhite
                                                       : wq::Color::kBlack,
                   0);
}

void PlayAIWindow::on_autocount_clicked(GtkWidget* /*self*/,
                                        gpointer user_data) {
  PlayAIWindow* win = (PlayAIWindow*)user_data;
  win->finish_game(wq::Color::kNone, 0);
}

void PlayAIWindow::on_show_ai_variation_clicked(GtkWidget* /*self*/,
                                                gpointer user_data) {
  PlayAIWindow* win = (PlayAIWindow*)user_data;
  for (const auto& [r, c] : win->katago_last_resp_.move_infos[0].pv) {
    win->move(r, c, kMoveFlagVariation);
  }
  win->evaluate_current_position();
}

}  // namespace ui