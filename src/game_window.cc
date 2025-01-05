#include "game_window.h"

#include "color.h"
#include "log.h"

namespace ui {

GameWindow::GameWindow(AppContext& ctx, int board_size) : Window(ctx) {
  gtk_window_set_default_size(GTK_WINDOW(window_), 800, 800);

  // Board
  board_ = std::make_unique<wq::Board>(board_size, board_size);
  goban_ =
      std::make_unique<GtkBoard>("main_board", board_size, 0, 0, board_size - 1,
                                 board_size - 1, ctx_.rand());
  goban_->set_board_texture(ctx.board_texture());
  goban_->set_black_stone_textures(ctx.black_stone_textures().data(),
                                   ctx.black_stone_textures().size());
  goban_->set_white_stone_textures(ctx.white_stone_textures().data(),
                                   ctx.white_stone_textures().size());

  using namespace std::placeholders;
  goban_->set_on_point_click(
      std::bind(&GameWindow::on_point_click, this, _1, _2));
  goban_->set_on_point_enter(
      std::bind(&GameWindow::on_point_enter, this, _1, _2));
  goban_->set_on_point_leave(
      std::bind(&GameWindow::on_point_leave, this, _1, _2));

  // Navigation bar
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
  navigation_bar_ = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
  gtk_box_append(GTK_BOX(navigation_bar_), first_move_button);
  gtk_box_append(GTK_BOX(navigation_bar_), prev_n_moves_button);
  gtk_box_append(GTK_BOX(navigation_bar_), prev_move_button);
  gtk_box_append(GTK_BOX(navigation_bar_), next_move_button);
  gtk_box_append(GTK_BOX(navigation_bar_), next_n_moves_button);
  gtk_box_append(GTK_BOX(navigation_bar_), last_move_button);
}

GtkBoard& GameWindow::board_widget() { return *goban_; }

GtkWidget* GameWindow::navigation_bar_widget() { return navigation_bar_; }

wq::Board& GameWindow::board() { return *board_; }

wq::Color GameWindow::turn() const { return turn_; }

wq::MoveList GameWindow::moves() const {
  auto moves = wq::MoveList(moves_.begin(), moves_.begin() + cur_move_);
  moves.insert(moves.end(), variation_moves_.begin(), variation_moves_.end());
  return moves;
}

void GameWindow::set_turn(wq::Color turn) { turn_ = turn; }

void GameWindow::toggle_turn() {
  switch (turn_) {
    case wq::Color::kNone:
      break;
    case wq::Color::kBlack:
      turn_ = wq::Color::kWhite;
      break;
    case wq::Color::kWhite:
      turn_ = wq::Color::kBlack;
      break;
  }
}

void GameWindow::set_board_size(int board_size) {
  assert(1 <= board_size && board_size <= 19);
  turn_ = wq::Color::kBlack;
  board_ = std::make_unique<wq::Board>(board_size, board_size);
  goban_->resize(board_size, 0, 0, board_size - 1, board_size - 1);
  cur_move_ = 0;
  moves_.clear();
  variation_moves_.clear();
}

bool GameWindow::move(int r, int c, MoveFlag flags) {
  auto prev_move = board_->last_move();

  wq::PointList removed;
  if (!board_->move(turn_, r, c, removed)) return false;

  if (flags & kMoveFlagVariation) {
    variation_moves_.push_back(wq::Move(turn_, wq::Point(r, c)));
  } else {
    if (cur_move_ < moves_.size()) moves_.resize(cur_move_);
    cur_move_++;
    moves_.push_back(wq::Move(turn_, wq::Point(r, c)));
  }

  if (flags & kMoveFlagSound) play_move_sound(removed.size());

  if (prev_move) {
    const auto& [pcol, pnt] = prev_move.value();
    const auto& [pr, pc] = pnt;
    goban_->set_annotation(pr, pc, AnnotationType::kNone);
  }

  goban_->set_point(r, c, turn_);
  for (const auto& [rr, cc] : removed)
    goban_->set_point(rr, cc, wq::Color::kNone);
  if (flags & kMoveFlagVariation) {
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
  return true;
}

void GameWindow::pass() {
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
}

void GameWindow::goto_move(int move_number) {
  while (cur_move_ < move_number && goto_next_move());
  while (cur_move_ > move_number && goto_prev_move());
}

bool GameWindow::goto_prev_move() {
  if (variation_moves_.empty() && cur_move_ == 0) return false;

  // Find the last played point
  wq::Point last;
  if (!variation_moves_.empty()) {
    last = variation_moves_.back().second;
    variation_moves_.pop_back();
  } else {
    cur_move_--;
    last = moves_[cur_move_].second;
  }

  // If it was not pass, remove it from board state
  if (last != wq::kPass) {
    int r, c;
    wq::PointList added;
    if (!board_->undo(r, c, added)) {
      LOG(ERROR) << "undo move did not match";
      std::exit(1);
    }
    if (wq::Point(r, c) != last) {
      LOG(ERROR) << "wtf: goto_prev_move: want (" << last.first << ","
                 << last.second << "), got (" << r << "," << c << ")";
    }
    assert(wq::Point(r, c) == last);
    goban_->set_text(r, c, "");
    goban_->set_annotation(r, c, AnnotationType::kNone);
    goban_->set_point(r, c, wq::Color::kNone);
    for (const auto& [rr, cc] : added) {
      goban_->set_point(rr, cc, turn_);
    }
  }

  // Find the new last played point again and annotate it
  last = wq::kPass;
  if (!variation_moves_.empty()) {
    last = variation_moves_.back().second;
  } else if (cur_move_ > 0 && cur_move_ <= moves_.size()) {
    last = moves_[cur_move_ - 1].second;
  }
  if (last != wq::kPass) {
    const auto& [rr, cc] = last;
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

bool GameWindow::goto_next_move() {
  if (cur_move_ >= moves_.size() || !variation_moves_.empty()) return false;

  // Find last played point to remove annotation
  wq::Point last = wq::kPass;
  if (!variation_moves_.empty()) {
    last = variation_moves_.back().second;
  } else if (cur_move_ > 0) {
    last = moves_[cur_move_ - 1].second;
  }

  // Play the next point
  const auto& [col, pnt] = moves_[cur_move_];
  const auto& [r, c] = pnt;
  wq::PointList removed;
  if (pnt != wq::kPass) {
    if (!board_->move(turn_, r, c, removed)) {
      LOG(ERROR) << "next move did not match";
      std::exit(1);
    }
  }
  cur_move_++;

  // Remove annotation from previous point
  if (last != wq::kPass) {
    goban_->set_annotation(last.first, last.second, AnnotationType::kNone);
  }

  // Annotate current point
  if (pnt != wq::kPass) {
    goban_->set_point(r, c, turn_);
    for (const auto& [rr, cc] : removed)
      goban_->set_point(rr, cc, wq::Color::kNone);
    goban_->set_annotation(r, c, AnnotationType::kBottomRightTriangle);
    goban_->set_annotation_color(
        r, c, turn_ == wq::Color::kBlack ? color_white : color_black);
  }

  toggle_turn();
  return true;
}

bool GameWindow::goto_mainline() {
  bool changed = false;
  while (!variation_moves_.empty()) changed |= goto_prev_move();
  if (changed) on_board_position_changed();
  return changed;
}

void GameWindow::on_point_enter(int r, int c) {
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
}

void GameWindow::on_point_leave(int r, int c) {
  if (board_->at(r, c) != wq::Color::kNone) return;
  goban_->set_annotation(r, c, AnnotationType::kNone);
}

void GameWindow::play_move_sound(size_t capture_count) {
  GtkMediaStream* snd = ctx_.play_stone_sound();
  if (capture_count > 5)
    snd = ctx_.capture_many_sound();
  else if (capture_count > 1)
    snd = ctx_.capture_few_sound();
  else if (capture_count == 1)
    snd = ctx_.capture_one_sound();
  gtk_media_stream_set_volume(snd, 1.0);
  gtk_media_stream_play(snd);
}

void GameWindow::on_first_move_clicked(GtkWidget* /*self*/,
                                       gpointer user_data) {
  GameWindow* win = (GameWindow*)user_data;
  bool changed = false;
  while (win->goto_prev_move()) changed = true;
  if (changed) win->on_board_position_changed();
}

void GameWindow::on_prev_n_moves_clicked(GtkWidget* /*self*/,
                                         gpointer user_data) {
  GameWindow* win = (GameWindow*)user_data;
  bool changed = false;
  for (int i = 0; i < 10; ++i) {
    if (!win->goto_prev_move()) break;
    changed = true;
  }
  if (changed) win->on_board_position_changed();
}

void GameWindow::on_prev_move_clicked(GtkWidget* /*self*/, gpointer user_data) {
  GameWindow* win = (GameWindow*)user_data;
  if (win->goto_prev_move()) win->on_board_position_changed();
}

void GameWindow::on_next_move_clicked(GtkWidget* /*self*/, gpointer user_data) {
  GameWindow* win = (GameWindow*)user_data;
  if (win->goto_next_move()) win->on_board_position_changed();
}

void GameWindow::on_next_n_moves_clicked(GtkWidget* /*self*/,
                                         gpointer user_data) {
  GameWindow* win = (GameWindow*)user_data;
  bool changed = false;
  for (int i = 0; i < 10; ++i) {
    if (!win->goto_next_move()) break;
    changed = true;
  }
  if (changed) win->on_board_position_changed();
}

void GameWindow::on_last_move_clicked(GtkWidget* /*self*/, gpointer user_data) {
  GameWindow* win = (GameWindow*)user_data;
  bool changed = false;
  while (win->goto_next_move()) changed = true;
  if (changed) win->on_board_position_changed();
}

}  // namespace ui