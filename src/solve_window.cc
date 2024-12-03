#include "solve_window.h"

#include <algorithm>
#include <cassert>
#include <nlohmann/json.hpp>
#include <set>

#include "color.h"
#include "log.h"
#include "solve_preset_window.h"

namespace ui {

SolveWindow::SolveWindow(AppContext& ctx, SolvePreset preset,
                         std::optional<std::pair<int, Rank>> tag_ref)
    : Window(ctx),
      preset_(preset),
      tag_ref_(tag_ref),
      task_ids_(ctx.tasks().get_tasks(preset)) {
  std::shuffle(task_ids_.begin(), task_ids_.end(), ctx_.rand());
  if (preset_.max_tasks_ > 0 && preset_.max_tasks_ < (int)task_ids_.size())
    task_ids_.resize(preset_.max_tasks_);
  LOG(INFO) << "solve session started: task_count=" << task_ids_.size();

  gtk_window_set_title(GTK_WINDOW(window_), preset_.description_.c_str());
  gtk_window_set_default_size(GTK_WINDOW(window_), 800, 800);

  GtkWidget* box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);

  //================================================================================
  GtkWidget* top_box = gtk_center_box_new();

  rank_label_ = gtk_label_new("Rank: ?");
  gtk_widget_set_margin_start(rank_label_, 8);
  gtk_widget_set_margin_end(rank_label_, 8);
  gtk_center_box_set_start_widget(GTK_CENTER_BOX(top_box), rank_label_);

  turn_label_ = gtk_label_new("");
  gtk_center_box_set_center_widget(GTK_CENTER_BOX(top_box), turn_label_);

  task_type_label_ = gtk_label_new("");
  gtk_widget_set_margin_start(task_type_label_, 8);
  gtk_widget_set_margin_end(task_type_label_, 8);
  gtk_center_box_set_end_widget(GTK_CENTER_BOX(top_box), task_type_label_);

  gtk_box_append(GTK_BOX(box), top_box);
  gtk_box_append(GTK_BOX(box), gtk_separator_new(GTK_ORIENTATION_HORIZONTAL));

  //================================================================================

  board_ = std::make_unique<wq::Board>(19, 19);
  goban_ = std::make_unique<GtkGoban>("testpan", 19, 0, 0, 18, 18, ctx_.rand());
  goban_->set_board_texture(ctx.board_texture());
  goban_->set_black_stone_textures(ctx.black_stone_textures().data(),
                                   ctx.black_stone_textures().size());
  goban_->set_white_stone_textures(ctx.white_stone_textures().data(),
                                   ctx.white_stone_textures().size());
  gtk_box_append(GTK_BOX(box), goban_->widget());

  //================================================================================
  GtkWidget* action_bar = gtk_action_bar_new();

  solve_stats_label_ = gtk_label_new("- / - (-)");
  gtk_action_bar_pack_start(GTK_ACTION_BAR(action_bar), solve_stats_label_);

  GtkWidget* action_bar_center_widget =
      gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);

  time_result_label_ = gtk_label_new("-");
  gtk_box_append(GTK_BOX(action_bar_center_widget), time_result_label_);

  gtk_action_bar_set_center_widget(GTK_ACTION_BAR(action_bar),
                                   action_bar_center_widget);

  reset_button_ = gtk_button_new_with_label("Reset");
  g_signal_connect(reset_button_, "clicked", G_CALLBACK(on_reset_clicked),
                   this);

  next_button_ = gtk_button_new_with_label("Next");
  g_signal_connect(next_button_, "clicked", G_CALLBACK(on_next_clicked), this);

  GtkWidget* button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
  gtk_box_append(GTK_BOX(button_box), reset_button_);
  gtk_box_append(GTK_BOX(button_box), next_button_);

  gtk_action_bar_pack_end(GTK_ACTION_BAR(action_bar), button_box);

  gtk_box_append(GTK_BOX(box), action_bar);
  //================================================================================

  gtk_window_set_child(GTK_WINDOW(window_), box);
  gtk_window_present(GTK_WINDOW(window_));

  goban_->set_on_point_click([this](int r, int c) { on_point_click(r, c); });
  goban_->set_on_point_enter([this](int r, int c) {
    if (board_->at(r, c) != wq::Color::kNone) return;
    switch (turn_) {
      case wq::Color::kBlack:
        goban_->set_annotation_color(r, c, color_black);
        break;
      case wq::Color::kWhite:
        goban_->set_annotation_color(r, c, color_white);
        break;
      case wq::Color::kNone:
        break;
    }
    goban_->set_annotation(r, c, AnnotationType::kTerritory);
  });
  goban_->set_on_point_leave([this](int r, int c) {
    if (board_->at(r, c) != wq::Color::kNone) return;
    goban_->set_annotation(r, c, AnnotationType::kNone);
  });

  load_task(task_ids_[cur_task_index_]);

  if (preset.time_limit_sec_ > 0) {
    timer_source_ = g_timeout_add(1000, on_timer_tick, this);
  } else {
    timer_source_ = 0;
  }
}

SolveWindow::~SolveWindow() {
  if (timer_source_) g_source_remove(timer_source_);
}

void SolveWindow::on_point_click(int r, int c) {
  if (task_.type_ == TaskType::kLuoZi)
    on_point_click_choose_task(r, c);
  else
    on_point_click_normal_task(r, c);
}

void SolveWindow::on_point_click_normal_task(int r, int c) {
  if (turn_ == wq::Color::kNone || turn_ != task_.first_to_play_) return;

  auto prev_move = board_->last_move();
  wq::PointList removed;
  if (!board_->move(turn_, r, c, removed)) return;

  GtkMediaStream* snd = ctx_.play_stone_sound();
  if (removed.size() > 5)
    snd = ctx_.capture_many_sound();
  else if (removed.size() > 1)
    snd = ctx_.capture_few_sound();
  else if (removed.size() == 1)
    snd = ctx_.capture_one_sound();
  gtk_media_stream_set_volume(snd, 1.0);
  gtk_media_stream_play(snd);

  move_num_++;

  if (prev_move) {
    const auto& [pcol, pnt] = prev_move.value();
    const auto& [pr, pc] = pnt;
    goban_->set_annotation(pr, pc, AnnotationType::kNone);
  }

  goban_->set_point(r, c, turn_);
  for (const auto& [rr, cc] : removed) {
    goban_->set_point(rr, cc, wq::Color::kNone);
    goban_->set_text_color(rr, cc, color_black);
  }
  goban_->set_text(r, c, std::to_string(move_num_));
  goban_->set_text_color(
      r, c, turn_ == wq::Color::kBlack ? color_white : color_black);
  goban_->set_annotation(r, c, AnnotationType::kTopLeftTriangle);
  goban_->set_annotation_color(
      r, c, turn_ == wq::Color::kBlack ? color_red : color_blue);

  switch (turn_) {
    case wq::Color::kBlack:
      turn_ = wq::Color::kWhite;
      break;
    case wq::Color::kWhite:
      turn_ = wq::Color::kBlack;
      break;
    case wq::Color::kNone:
      break;
  }

  std::string comment;
  if (auto ans_opt = solve_state_->move(wq::Point(r, c), comment)) {
    set_solve_result(*ans_opt);
    return;
  }

  opponent_move_source_ =
      g_timeout_add_once(20, &SolveWindow::on_opponent_move, this);
}

void SolveWindow::on_point_click_choose_task(int r, int c) {
  if (pending_answer_points_.empty()) return;
  const wq::Point p(r, c);
  auto it = pending_answer_points_.find(p);
  if (it == pending_answer_points_.end()) {
    goban_->set_text(r, c, u8"⬤");
    goban_->set_text_color(r, c, color_red);
    for (const auto& [rr, cc] : pending_answer_points_) {
      goban_->set_text(rr, cc, u8"⬤");
      goban_->set_text_color(rr, cc, color_green);
    }
    pending_answer_points_.clear();
    set_solve_result(AnswerType::kWrong);
  } else {
    goban_->set_text(r, c, u8"⬤");
    goban_->set_text_color(r, c, color_blue);
    pending_answer_points_.erase(it);
    if (pending_answer_points_.empty()) set_solve_result(AnswerType::kCorrect);
  }
}

void SolveWindow::on_reset_clicked(GtkWidget* /*self*/, gpointer data) {
  SolveWindow* win = (SolveWindow*)data;
  win->reset_task(true);
}

void SolveWindow::on_next_clicked(GtkWidget* /*self*/, gpointer data) {
  SolveWindow* win = (SolveWindow*)data;
  win->set_solve_result(AnswerType::kWrong);
  if (win->session_complete_) return;

  win->cur_task_index_++;
  if (win->cur_task_index_ >= win->task_ids_.size()) win->cur_task_index_ = 0;
  win->load_task(win->task_ids_[win->cur_task_index_]);
}

void SolveWindow::on_opponent_move(gpointer data) {
  SolveWindow* win = (SolveWindow*)data;

  win->opponent_move_source_ = 0;
  win->move_num_++;

  const auto& [r, c] = win->solve_state_->gen_move(win->ctx_.rand());

  auto prev_move = win->board_->last_move();
  wq::PointList removed;
  win->board_->move(win->turn_, r, c, removed);

  if (prev_move) {
    const auto& [pcol, pnt] = prev_move.value();
    const auto& [pr, pc] = pnt;
    win->goban_->set_annotation(pr, pc, AnnotationType::kNone);
  }

  win->goban_->set_point(r, c, win->turn_);
  for (const auto& [rr, cc] : removed) {
    win->goban_->set_point(rr, cc, wq::Color::kNone);
    win->goban_->set_text_color(rr, cc, color_black);
  }
  win->goban_->set_text(r, c, std::to_string(win->move_num_));
  win->goban_->set_text_color(
      r, c, win->turn_ == wq::Color::kBlack ? color_white : color_black);
  win->goban_->set_annotation(r, c, AnnotationType::kTopLeftTriangle);
  win->goban_->set_annotation_color(
      r, c, win->turn_ == wq::Color::kBlack ? color_red : color_blue);

  std::string comment;
  if (auto ans_opt = win->solve_state_->move(wq::Point(r, c), comment)) {
    win->set_solve_result(*ans_opt);
    return;
  }

  switch (win->turn_) {
    case wq::Color::kBlack:
      win->turn_ = wq::Color::kWhite;
      break;
    case wq::Color::kWhite:
      win->turn_ = wq::Color::kBlack;
      break;
    case wq::Color::kNone:
      break;
  }
}

void SolveWindow::load_task(int64_t task_id) {
  if (auto task_opt = ctx_.tasks().get_task(task_id)) {
    LOG(INFO) << "task loaded: id=" << task_id
              << " public_id=" << task_opt->metadata_["public_id"]
              << " type=" << (int)task_opt->type_;
    task_ = std::move(task_opt.value());
    reset_task(false);
  }
}

void SolveWindow::reset_task(bool is_solved) {
  solve_state_ = std::make_unique<TaskVTreeIterator>(task_);
  pending_answer_points_ = std::set<wq::Point>(task_.answer_points_.begin(),
                                               task_.answer_points_.end());
  move_num_ = 0;
  turn_ = task_.first_to_play_;

  wq::PointList removed;
  goban_->resize(task_.board_size_, task_.top_left_.first,
                 task_.top_left_.second, task_.bottom_right_.first,
                 task_.bottom_right_.second);
  board_ = std::make_unique<wq::Board>(task_.board_size_, task_.board_size_);
  for (const auto& [r, c] : task_.initial_[0]) {
    board_->move(wq::Color::kBlack, r, c, removed);
    goban_->set_point(r, c, wq::Color::kBlack);
  }
  for (const auto& [r, c] : task_.initial_[1]) {
    board_->move(wq::Color::kWhite, r, c, removed);
    goban_->set_point(r, c, wq::Color::kWhite);
  }
  for (const auto& [p, label] : task_.labels_) {
    const auto& [r, c] = p;
    if (label == "$triangle") {
      goban_->set_text(r, c, u8"▲");
    } else {
      goban_->set_text(r, c, label);
    }
    goban_->set_text_color(r, c, color_blue);
  }

  if (is_solved) {
    gtk_label_set_text(GTK_LABEL(time_result_label_), "--");
    return;
  }

  time_left_ = preset_.time_limit_sec_;
  if (time_left_ > 0)
    set_time_left_label(time_left_);
  else
    gtk_label_set_text(GTK_LABEL(time_result_label_), "--");

  gtk_label_set_text(GTK_LABEL(rank_label_), "Rank: ?");
  std::string prompt;
  switch (task_.first_to_play_) {
    case wq::Color::kNone:
      break;
    case wq::Color::kBlack:
      prompt = "Black to play";
      break;
    case wq::Color::kWhite:
      prompt = "White to play";
      break;
  }
  if (!task_.description_.empty()) prompt += " - " + task_.description_;
  gtk_label_set_text(GTK_LABEL(turn_label_), prompt.c_str());
  gtk_label_set_text(GTK_LABEL(task_type_label_),
                     task_type_string(task_.type_));

  gtk_widget_set_sensitive(GTK_WIDGET(reset_button_), false);

  task_result_.reset();
}

void SolveWindow::set_solve_result(AnswerType type) {
  switch (type) {
    case AnswerType::kCorrect:
    case AnswerType::kVariation:
      gtk_label_set_markup(
          GTK_LABEL(time_result_label_),
          R"(<span weight="bold" background="deepskyblue" foreground="white" size="x-large"> CORRECT </span>)");
      break;
    case AnswerType::kWrong:
      gtk_label_set_markup(
          GTK_LABEL(time_result_label_),
          R"(<span weight="bold" background="tomato" foreground="white" size="x-large"> WRONG </span>)");
      break;
  }

  if (!task_result_) {
    task_result_ = type;
    ++task_count_;
    if (type == AnswerType::kWrong) ++error_count_;
    ctx_.stats().update_rank_stats(task_.rank_, 1,
                                   (type == AnswerType::kWrong ? 1 : 0));

    std::ostringstream ss;

    ss << "Rank: " << rank_string(task_.rank_);
    gtk_label_set_text(GTK_LABEL(rank_label_), ss.str().c_str());

    ss.str("");
    ss << (task_count_ - error_count_) << " / " << task_count_ << " ("
       << 100 * (task_count_ - error_count_) / task_count_ << "%)";
    gtk_label_set_text(GTK_LABEL(solve_stats_label_), ss.str().c_str());

    gtk_widget_set_sensitive(GTK_WIDGET(reset_button_), true);

    if (task_count_ == preset_.max_tasks_) {
      session_complete_ = true;
      session_complete_dialog_ = gtk_alert_dialog_new("Session Complete");
      gtk_alert_dialog_set_modal(session_complete_dialog_, true);
      gtk_alert_dialog_choose(session_complete_dialog_, GTK_WINDOW(window_),
                              nullptr, on_session_complete, this);
      gtk_widget_set_visible(GTK_WIDGET(next_button_), false);
      if (tag_ref_) {
        const auto& [tag_id, rank] = *tag_ref_;
        ctx_.stats().update_tag_stats(
            tag_id, rank, 1, (error_count_ > preset_.max_errors_) ? 1 : 0);
        for (Window* w :
             window_groups_[SolvePresetWindow::kSolvePresetWindowGroup]) {
          reinterpret_cast<SolvePresetWindow*>(w)->update_preset_buttons();
        }
      }
    }
  }
}

void SolveWindow::on_session_complete(GObject* /*source_object*/,
                                      GAsyncResult* res, gpointer data) {
  SolveWindow* win = (SolveWindow*)data;
  LOG(INFO) << "session complete";
  gtk_alert_dialog_choose_finish(win->session_complete_dialog_, res, nullptr);
}

void SolveWindow::set_time_left_label(int t) {
  std::ostringstream ss;
  ss << R"(<span weight="bold" size="x-large">)" << t << "</span>";
  gtk_label_set_markup(GTK_LABEL(time_result_label_), ss.str().c_str());
}

gboolean SolveWindow::on_timer_tick(gpointer data) {
  SolveWindow* win = (SolveWindow*)data;
  if (win->task_result_) return G_SOURCE_CONTINUE;

  if (win->time_left_ > 0) {
    win->time_left_--;
    win->set_time_left_label(win->time_left_);
    if (win->time_left_ == 0) win->set_solve_result(AnswerType::kWrong);
  }

  return G_SOURCE_CONTINUE;
}

}  // namespace ui