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

PlayAIWindow::PlayAIWindow(AppContext& ctx, PlayStyle play_style, Rank rank)
    : Window(ctx) {
  std::ostringstream title;
  title << "Play vs AI (" << style_string(play_style) << ' '
        << rank_string(rank) << ")";

  gtk_window_set_title(GTK_WINDOW(window_), title.str().c_str());
  gtk_window_set_default_size(GTK_WINDOW(window_), 800, 800);

  GtkWidget* box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);

  // Top bar
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

  // Board area
  board_ = std::make_unique<wq::Board>(19, 19);
  goban_ =
      std::make_unique<GtkGoban>("main_board", 19, 0, 0, 18, 18, ctx_.rand());
  goban_->set_board_texture(ctx.board_texture());
  goban_->set_black_stone_textures(ctx.black_stone_textures().data(),
                                   ctx.black_stone_textures().size());
  goban_->set_white_stone_textures(ctx.white_stone_textures().data(),
                                   ctx.white_stone_textures().size());
  gtk_box_append(GTK_BOX(box), goban_->widget());

  // Navigation bar
  navigation_bar_ = gtk_action_bar_new();
  GtkWidget* tool_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
  GtkWidget* first_move_button = gtk_button_new_with_label("|<");
  g_signal_connect(first_move_button, "clicked",
                   G_CALLBACK(on_first_move_clicked), this);
  GtkWidget* prev_n_moves_button = gtk_button_new_with_label("<<");
  g_signal_connect(prev_n_moves_button, "clicked",
                   G_CALLBACK(on_prev_n_moves_clicked), this);
  GtkWidget* prev_move_button = gtk_button_new_with_label("<");
  g_signal_connect(prev_move_button, "clicked",
                   G_CALLBACK(on_prev_move_clicked), this);
  GtkWidget* next_move_button = gtk_button_new_with_label(">");
  g_signal_connect(next_move_button, "clicked",
                   G_CALLBACK(on_next_move_clicked), this);
  GtkWidget* next_n_moves_button = gtk_button_new_with_label(">>");
  g_signal_connect(next_n_moves_button, "clicked",
                   G_CALLBACK(on_next_n_moves_clicked), this);
  GtkWidget* last_move_button = gtk_button_new_with_label(">|");
  g_signal_connect(last_move_button, "clicked",
                   G_CALLBACK(on_last_move_clicked), this);
  gtk_box_append(GTK_BOX(tool_box), first_move_button);
  gtk_box_append(GTK_BOX(tool_box), prev_n_moves_button);
  gtk_box_append(GTK_BOX(tool_box), prev_move_button);
  gtk_box_append(GTK_BOX(tool_box), next_move_button);
  gtk_box_append(GTK_BOX(tool_box), next_n_moves_button);
  gtk_box_append(GTK_BOX(tool_box), last_move_button);
  gtk_action_bar_set_center_widget(GTK_ACTION_BAR(navigation_bar_), tool_box);
  gtk_box_append(GTK_BOX(box), navigation_bar_);
  gtk_widget_set_visible(GTK_WIDGET(navigation_bar_), false);

  gtk_window_set_child(GTK_WINDOW(window_), box);
  gtk_window_present(GTK_WINDOW(window_));

  using namespace std::placeholders;
  goban_->set_on_point_click(
      std::bind(&PlayAIWindow::on_point_click, this, _1, _2));
  goban_->set_on_point_enter(
      std::bind(&PlayAIWindow::on_point_enter, this, _1, _2));
  goban_->set_on_point_leave(
      std::bind(&PlayAIWindow::on_point_leave, this, _1, _2));

  std::string human_profile;
  switch (play_style) {
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
  katago_query_.max_visits = 40;
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
  if (state_ == State::kPlaying && turn_ != my_color_) return;

  auto prev_move = board_->last_move();

  wq::PointList removed;
  if (!board_->move(turn_, r, c, removed)) return;

  bool is_variation = false;
  if (cur_move_ < moves_.size() || state_ != State::kPlaying) {
    variation_moves_.push_back(wq::Move(turn_, wq::Point(r, c)));
    is_variation = true;
    // TODO do not append to variation if it matches next mainline move
  } else {
    cur_move_++;
    moves_.push_back(wq::Move(turn_, wq::Point(r, c)));
  }

  GtkMediaStream* snd = ctx_.play_stone_sound();
  if (removed.size() > 5)
    snd = ctx_.capture_many_sound();
  else if (removed.size() > 1)
    snd = ctx_.capture_few_sound();
  else if (removed.size() == 1)
    snd = ctx_.capture_one_sound();
  gtk_media_stream_set_volume(snd, 1.0);
  gtk_media_stream_play(snd);

  if (prev_move) {
    const auto& [pcol, pnt] = prev_move.value();
    const auto& [pr, pc] = pnt;
    goban_->set_annotation(pr, pc, AnnotationType::kNone);
  }

  goban_->set_point(r, c, turn_);
  for (const auto& [rr, cc] : removed)
    goban_->set_point(rr, cc, wq::Color::kEmpty);
  if (is_variation) {
    goban_->set_annotation(r, c, AnnotationType::kTopLeftTriangle);
    goban_->set_annotation_color(
        r, c, turn_ == wq::Color::kBlack ? color_red : color_blue);
    goban_->set_text(r, c, std::to_string(variation_moves_.size()));
    goban_->set_text_color(
        r, c, turn_ == wq::Color::kBlack ? color_white : color_black);
  } else {
    goban_->set_annotation(r, c, AnnotationType::kBottomRightTriangle);
    goban_->set_annotation_color(
        r, c, turn_ == wq::Color::kBlack ? color_white : color_black);
  }

  toggle_turn();

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
  if (board_->at(r, c) != wq::Color::kEmpty) return;
  switch (turn_) {
    case wq::Color::kBlack:
      goban_->set_annotation_color(r, c, color_black);
      break;
    case wq::Color::kWhite:
      goban_->set_annotation_color(r, c, color_white);
      break;
    case wq::Color::kEmpty:
      break;
  }
  goban_->set_annotation(r, c, AnnotationType::kTerritory);
}

void PlayAIWindow::on_point_leave(int r, int c) {
  if (state_ == State::kCounting) return;
  if (board_->at(r, c) != wq::Color::kEmpty) return;
  goban_->set_annotation(r, c, AnnotationType::kNone);
}

void PlayAIWindow::on_pass() {
  if (state_ != State::kPlaying && state_ != State::kReviewing) return;
  if (state_ == State::kPlaying && turn_ != my_color_) return;

  auto prev_move = board_->last_move();

  if (cur_move_ < moves_.size()) moves_.resize(cur_move_);
  cur_move_++;
  moves_.push_back(wq::Move(turn_, wq::kPass));

  if (prev_move) {
    const auto& [pcol, pnt] = prev_move.value();
    const auto& [pr, pc] = pnt;
    goban_->set_annotation(pr, pc, AnnotationType::kNone);
  }

  toggle_turn();

  consecutive_pass_++;
  if (state_ == State::kPlaying && consecutive_pass_ == 2)
    finish_game(wq::Color::kEmpty, 0);

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
  katago_query_.moves =
      wq::MoveList(moves_.begin(), moves_.begin() + cur_move_);
  katago_query_.moves.insert(katago_query_.moves.end(),
                             variation_moves_.begin(), variation_moves_.end());
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
        auto prev_move = board_->last_move();

        if (move_index == 19 * 19) {  // pass
          cur_move_++;
          moves_.push_back(wq::Move(turn_, wq::kPass));

          if (prev_move) {
            const auto& [pcol, pnt] = prev_move.value();
            const auto& [pr, pc] = pnt;
            goban_->set_annotation(pr, pc, AnnotationType::kNone);
          }
          consecutive_pass_++;
        } else {
          int r = move_index / 19;
          int c = move_index % 19;

          wq::PointList removed;
          if (!board_->move(turn_, r, c, removed)) return;

          cur_move_++;
          moves_.push_back(wq::Move(turn_, wq::Point(r, c)));

          if (prev_move) {
            const auto& [pcol, pnt] = prev_move.value();
            const auto& [pr, pc] = pnt;
            goban_->set_annotation(pr, pc, AnnotationType::kNone);
          }

          goban_->set_point(r, c, turn_);
          for (const auto& [rr, cc] : removed)
            goban_->set_point(rr, cc, wq::Color::kEmpty);
          goban_->set_annotation(r, c, AnnotationType::kBottomRightTriangle);
          goban_->set_annotation_color(
              r, c, turn_ == wq::Color::kBlack ? color_white : color_black);
        }

        toggle_turn();

        if (consecutive_pass_ == 2) finish_game(wq::Color::kEmpty, 0);
      });
}

void PlayAIWindow::toggle_turn() {
  switch (turn_) {
    case wq::Color::kEmpty:
      break;
    case wq::Color::kBlack:
      turn_ = wq::Color::kWhite;
      break;
    case wq::Color::kWhite:
      turn_ = wq::Color::kBlack;
      break;
  }
}

bool PlayAIWindow::goto_prev_move() {
  int r, c;
  wq::PointList added;
  if (!board_->undo(r, c, added)) return false;

  if (variation_moves_.empty()) {
    cur_move_--;
  } else {
    variation_moves_.pop_back();
    goban_->set_text(r, c, "");
  }

  goban_->set_annotation(r, c, AnnotationType::kNone);
  goban_->set_point(r, c, wq::Color::kEmpty);
  for (const auto& [rr, cc] : added) {
    goban_->set_point(rr, cc, turn_);
  }

  if (auto prev_move = board_->last_move()) {
    const auto& [col, pt] = prev_move.value();
    const auto& [rr, cc] = pt;
    if (variation_moves_.empty()) {
      goban_->set_annotation(rr, cc, AnnotationType::kBottomRightTriangle);
      goban_->set_annotation_color(
          rr, cc, turn_ == wq::Color::kBlack ? color_white : color_black);
    } else {
      goban_->set_annotation(rr, cc, AnnotationType::kTopLeftTriangle);
      goban_->set_annotation_color(
          rr, cc, turn_ == wq::Color::kBlack ? color_red : color_blue);
      goban_->set_text(rr, cc, std::to_string(variation_moves_.size()));
      goban_->set_text_color(
          rr, cc, turn_ == wq::Color::kBlack ? color_white : color_black);
    }
  }

  toggle_turn();
  return true;
}

bool PlayAIWindow::goto_next_move() {
  if (cur_move_ >= moves_.size() || !variation_moves_.empty()) return false;

  auto prev_move = board_->last_move();

  const auto& [col, pnt] = moves_[cur_move_];
  const auto& [r, c] = pnt;
  wq::PointList removed;
  board_->move(turn_, r, c, removed);

  cur_move_++;

  if (prev_move) {
    const auto& [pcol, pnt] = prev_move.value();
    const auto& [pr, pc] = pnt;
    goban_->set_annotation(pr, pc, AnnotationType::kNone);
  }

  goban_->set_point(r, c, turn_);
  for (const auto& [rr, cc] : removed)
    goban_->set_point(rr, cc, wq::Color::kEmpty);
  goban_->set_annotation(r, c, AnnotationType::kBottomRightTriangle);
  goban_->set_annotation_color(
      r, c, turn_ == wq::Color::kBlack ? color_white : color_black);

  toggle_turn();
  return true;
}

void PlayAIWindow::evaluate_current_position() {
  if (!last_query_id_.empty()) ctx_.katago()->cancel_query(last_query_id_);
  katago_query_.moves =
      wq::MoveList(moves_.begin(), moves_.begin() + cur_move_);
  katago_query_.moves.insert(katago_query_.moves.end(),
                             variation_moves_.begin(), variation_moves_.end());
  last_query_id_ = ctx_.katago()->query(
      katago_query_,
      [this](KataGoClient::Response resp, std::optional<std::string> error) {
        if (error) {
          LOG(ERROR) << "katago: " << *error;
          return;
        }

        eval_bar_.update(resp.root_info.winrate, resp.root_info.score_lead);
      });
}

void PlayAIWindow::finish_game(wq::Color winner, double score_lead) {
  // Reset engine settings for analysis
  katago_query_.max_visits.reset();
  katago_query_.include_policy = false;
  katago_query_.override_settings = json::object();

  // If there's no winner, we need to count first.
  if (winner == wq::Color::kEmpty) {
    state_ = State::kCounting;
    if (!last_query_id_.empty()) ctx_.katago()->cancel_query(last_query_id_);
    katago_query_.include_ownership = true;
    katago_query_.moves =
        wq::MoveList(moves_.begin(), moves_.begin() + cur_move_);
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
                switch (board_->at(i, j)) {
                  case wq::Color::kEmpty:
                    goban_->set_annotation(i, j, AnnotationType::kTerritory);
                    goban_->set_annotation_color(i, j, color_black);
                    break;
                  case wq::Color::kBlack:
                    goban_->set_annotation(i, j, AnnotationType::kNone);
                    break;
                  case wq::Color::kWhite:
                    goban_->set_annotation(i, j, AnnotationType::kTerritory);
                    goban_->set_annotation_color(i, j, color_black);
                    break;
                }
              } else if (t < -0.9) {
                switch (board_->at(i, j)) {
                  case wq::Color::kEmpty:
                    goban_->set_annotation(i, j, AnnotationType::kTerritory);
                    goban_->set_annotation_color(i, j, color_white);
                    break;
                  case wq::Color::kBlack:
                    goban_->set_annotation(i, j, AnnotationType::kTerritory);
                    goban_->set_annotation_color(i, j, color_white);
                    break;
                  case wq::Color::kWhite:
                    goban_->set_annotation(i, j, AnnotationType::kNone);
                    break;
                }
              } else {
                goban_->set_annotation(i, j, AnnotationType::kNone);
              }
            }
          }

          finish_game(winner, std::abs(score_lead));
        });
    return;
  }

  std::ostringstream game_res;
  switch (winner) {
    case wq::Color::kEmpty:
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
  gtk_widget_set_visible(GTK_WIDGET(win->navigation_bar_), true);

  for (int i = 0; i < 19; ++i) {
    for (int j = 0; j < 19; ++j) {
      win->goban_->set_annotation(i, j, AnnotationType::kNone);
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
  win->finish_game(wq::Color::kEmpty, 0);
}

void PlayAIWindow::on_first_move_clicked(GtkWidget* /*self*/,
                                         gpointer user_data) {
  PlayAIWindow* win = (PlayAIWindow*)user_data;
  bool changed = false;
  while (win->goto_prev_move()) changed = true;
  if (changed) win->evaluate_current_position();
}

void PlayAIWindow::on_prev_n_moves_clicked(GtkWidget* /*self*/,
                                           gpointer user_data) {
  PlayAIWindow* win = (PlayAIWindow*)user_data;
  bool changed = false;
  for (int i = 0; i < 10; ++i) {
    if (!win->goto_prev_move()) break;
    changed = true;
  }
  if (changed) win->evaluate_current_position();
}

void PlayAIWindow::on_prev_move_clicked(GtkWidget* /*self*/,
                                        gpointer user_data) {
  PlayAIWindow* win = (PlayAIWindow*)user_data;
  if (win->goto_prev_move()) win->evaluate_current_position();
}

void PlayAIWindow::on_next_move_clicked(GtkWidget* /*self*/,
                                        gpointer user_data) {
  PlayAIWindow* win = (PlayAIWindow*)user_data;
  if (win->goto_next_move()) win->evaluate_current_position();
}

void PlayAIWindow::on_next_n_moves_clicked(GtkWidget* /*self*/,
                                           gpointer user_data) {
  PlayAIWindow* win = (PlayAIWindow*)user_data;
  bool changed = false;
  for (int i = 0; i < 10; ++i) {
    if (!win->goto_next_move()) break;
    changed = true;
  }
  if (changed) win->evaluate_current_position();
}

void PlayAIWindow::on_last_move_clicked(GtkWidget* /*self*/,
                                        gpointer user_data) {
  PlayAIWindow* win = (PlayAIWindow*)user_data;
  bool changed = false;
  while (win->goto_next_move()) changed = true;
  if (changed) win->evaluate_current_position();
}

}  // namespace ui