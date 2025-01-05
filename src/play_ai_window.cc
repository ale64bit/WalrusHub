#include "play_ai_window.h"

#include <cctype>
#include <iomanip>
#include <numeric>
#include <random>
#include <sstream>

#include "color.h"
#include "log.h"
#include "play_ai_preset_window.h"

namespace ui {

static int max_visits_for_rank(Rank rank) {
  if (rank < Rank::k5D) return 1;
  return 3 * ((int)rank - int(Rank::k4D));
}

PlayAIWindow::PlayAIWindow(AppContext& ctx, PlayStyle play_style, Rank rank,
                           bool ranked)
    : GameWindow(ctx, 19),
      play_style_(play_style),
      rank_(rank),
      ranked_(ranked) {
  std::ostringstream title;
  title << "Play vs AI (" << play_style_string(play_style_) << ' '
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

  move_table_.set_selection_func([this](int i) {
    goto_move(i);
    evaluate_current_position();
  });

  move_table_.add_column("#", &PlayAIWindow::move_table_column_number);
  move_table_.add_column("Move", &PlayAIWindow::move_table_column_move);
  move_table_.add_column("Loss", &PlayAIWindow::move_table_column_loss);
  gtk_widget_set_visible(move_table_, false);

  GtkWidget* center_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);

  GtkWidget* board_frame = gtk_frame_new(nullptr);
  gtk_frame_set_child(GTK_FRAME(board_frame), board_widget().widget());

  GtkWidget* move_table_frame = gtk_scrolled_window_new();
  gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(move_table_frame),
                                move_table_);
  gtk_scrolled_window_set_has_frame(GTK_SCROLLED_WINDOW(move_table_frame),
                                    true);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(move_table_frame),
                                 GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

  gtk_box_append(GTK_BOX(center_box), board_frame);
  gtk_box_append(GTK_BOX(center_box), move_table_frame);
  gtk_widget_set_hexpand(move_table_, false);
  gtk_widget_set_halign(move_table_, GTK_ALIGN_END);

  gtk_box_append(GTK_BOX(box), center_box);

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

        if (should_resign(resp)) {
          finish_game(my_color_, 0);
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

bool PlayAIWindow::should_resign(const KataGoClient::Response& resp) {
  constexpr double kScoreLeadThreshold = 40.0;
  constexpr double kWinrateThreshold = 0.99;
  constexpr int kConsecutiveTurns = 20;
  constexpr int kMinMoveCount = 90;

  const bool score_lead_cond =
      (my_color_ == wq::Color::kBlack &&
       resp.root_info.score_lead >= kScoreLeadThreshold) ||
      (my_color_ == wq::Color::kWhite &&
       -resp.root_info.score_lead >= kScoreLeadThreshold);
  const bool winrate_cond = (my_color_ == wq::Color::kBlack &&
                             resp.root_info.winrate >= kWinrateThreshold) ||
                            (my_color_ == wq::Color::kWhite &&
                             resp.root_info.winrate <= (1 - kWinrateThreshold));
  if (score_lead_cond && winrate_cond) {
    ++consecutive_resign_checks_;
    if ((int)katago_query_.moves.size() >= kMinMoveCount &&
        consecutive_resign_checks_ > kConsecutiveTurns) {
      return true;
    }
  } else {
    consecutive_resign_checks_ = 0;
  }
  return false;
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
      game_res << wq::color_short_string(winner) << "+";
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

  if (ranked_) {
    ctx_.stats().update_play_ai_stats(
        play_style_, rank_, winner == my_color_ ? 1 : 0,
        (winner != wq::Color::kNone && winner != my_color_) ? 1 : 0);
    Window::update_group(PlayAIPresetWindow::kPlayAIPresetWindowGroup);
  }

  // Evaluate full game
  gtk_widget_set_visible(move_table_, true);

  KataGoClient::Query full_game_query = katago_query_;
  full_game_query.include_ownership = false;
  full_game_query.override_settings = json::object();
  full_game_query.moves = moves();

  int move_num = 1;
  turn_score_lead_.resize(1, std::numeric_limits<float>::infinity());
  full_game_query.analyze_turns.resize(1, 0);
  for (const auto& mv : moves()) {
    turn_score_lead_.push_back(std::numeric_limits<float>::infinity());
    full_game_query.analyze_turns.push_back(move_num);
    move_table_.add_row(
        MoveTableEntry(move_num, mv, std::numeric_limits<float>::infinity()));
    move_num++;
  }

  ctx_.katago()->query(
      full_game_query,
      [this](KataGoClient::Response resp, std::optional<std::string> error) {
        if (error) {
          LOG(ERROR) << "katago: " << *error;
          return;
        }
        const int i = resp.turn_number;
        turn_score_lead_[i] = resp.root_info.score_lead;
        if (i > 0 &&
            turn_score_lead_[i - 1] != std::numeric_limits<float>::infinity()) {
          compute_point_loss(i);
        }
        if (i + 1 < (int)turn_score_lead_.size() &&
            turn_score_lead_[i + 1] != std::numeric_limits<float>::infinity()) {
          compute_point_loss(i + 1);
        }
      });
}

void PlayAIWindow::compute_point_loss(int i) {
  const auto mvs = moves();
  float point_loss = .0f;
  const auto& [col, _] = mvs[i - 1];
  const float sl0 = turn_score_lead_[i - 1];
  const float sl1 = turn_score_lead_[i];
  if (col == wq::Color::kBlack && sl1 < sl0) point_loss = std::abs(sl1 - sl0);
  if (col == wq::Color::kWhite && sl1 > sl0) point_loss = std::abs(sl1 - sl0);
  move_table_.update_row(i - 1, MoveTableEntry(i, mvs[i - 1], -point_loss));
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

GtkWidget* PlayAIWindow::move_table_column_number(const MoveTableEntry& entry) {
  return gtk_label_new(std::to_string(std::get<0>(entry)).c_str());
}

GtkWidget* PlayAIWindow::move_table_column_move(const MoveTableEntry& entry) {
  const auto& [col, pnt] = std::get<1>(entry);
  const auto& [r, c] = pnt;
  std::ostringstream ss;
  ss << "<span color='" << (col == wq::Color::kWhite ? "white" : "black")
     << u8"'>⬤</span><span> " << (char)('a' + c) << (char)('a' + r)
     << "</span>";

  GtkWidget* label = gtk_label_new("");
  gtk_label_set_markup(GTK_LABEL(label), ss.str().c_str());
  return label;
}

GtkWidget* PlayAIWindow::move_table_column_loss(const MoveTableEntry& entry) {
  const float point_loss = std::get<2>(entry);
  if (point_loss == std::numeric_limits<float>::infinity())
    return gtk_label_new("-");

  std::ostringstream ss;
  ss << "<span color='";
  if (point_loss <= -12)
    ss << "darkorchid";
  else if (point_loss <= -8)
    ss << "crimson";
  else if (point_loss <= -5)
    ss << "orange";
  else if (point_loss <= -3)
    ss << "gold";
  else if (point_loss < -1.0f)
    ss << "greenyellow";
  else
    ss << "limegreen";
  ss << "'>" << std::fixed << std::setprecision(2) << point_loss << "</span>";

  GtkWidget* label = gtk_label_new("");
  gtk_label_set_markup(GTK_LABEL(label), ss.str().c_str());
  return label;
}

}  // namespace ui