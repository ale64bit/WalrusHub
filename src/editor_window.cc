#include "editor_window.h"

#include <array>
#include <cassert>
#include <cstdlib>

#include "color.h"
#include "log.h"

namespace ui {

static constexpr std::array<const char*, 18> kBoardSizes = {
    "3",  "4",  "5",  "6",  "7",  "8",  "9",  "10", "11",
    "12", "13", "14", "15", "16", "17", "18", "19", nullptr,
};

EditorWindow::EditorWindow(AppContext& ctx) : Window(ctx) {
  gtk_window_set_title(GTK_WINDOW(window_), "Editor");
  gtk_window_set_default_size(GTK_WINDOW(window_), 800, 800);

  GtkWidget* box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);

  GtkWidget* top_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);
  board_size_dropdown_ = gtk_drop_down_new_from_strings(kBoardSizes.data());
  gtk_drop_down_set_selected(GTK_DROP_DOWN(board_size_dropdown_),
                             kBoardSizes.size() - 2);
  g_signal_connect(GTK_DROP_DOWN(board_size_dropdown_), "notify::selected",
                   G_CALLBACK(on_board_size_changed), this);

  gtk_box_append(GTK_BOX(top_box), gtk_label_new("Board size:"));
  gtk_box_append(GTK_BOX(top_box), board_size_dropdown_);
  gtk_box_append(GTK_BOX(box), top_box);
  gtk_box_append(GTK_BOX(box), gtk_separator_new(GTK_ORIENTATION_HORIZONTAL));

  board_ = std::make_unique<wq::Board>(19, 19);
  goban_ = std::make_unique<GtkGoban>("editor", 19, 0, 0, 18, 18, ctx_.rand());
  goban_->set_board_texture(ctx.board_texture());
  goban_->set_black_stone_textures(ctx.black_stone_textures().data(),
                                   ctx.black_stone_textures().size());
  goban_->set_white_stone_textures(ctx.white_stone_textures().data(),
                                   ctx.white_stone_textures().size());
  gtk_box_append(GTK_BOX(box), goban_->widget());

  GtkWidget* action_bar = gtk_action_bar_new();

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

  gtk_action_bar_set_center_widget(GTK_ACTION_BAR(action_bar), tool_box);

  gtk_box_append(GTK_BOX(box), action_bar);

  gtk_window_set_child(GTK_WINDOW(window_), box);
  gtk_window_present(GTK_WINDOW(window_));

  goban_->set_on_point_click([this](int r, int c) {
    auto prev_move = board_->last_move();

    wq::PointList removed;
    if (!board_->move(turn_, r, c, removed)) return;

    if (cur_move_ < moves_.size()) moves_.resize(cur_move_);
    cur_move_++;
    moves_.push_back(wq::Move(turn_, wq::Point(r, c)));

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
      goban_->set_point(rr, cc, wq::Color::kNone);
    goban_->set_annotation(r, c, AnnotationType::kBottomRightTriangle);
    goban_->set_annotation_color(
        r, c, turn_ == wq::Color::kBlack ? color_white : color_black);

    toggle_turn();
  });
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
}

void EditorWindow::toggle_turn() {
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

bool EditorWindow::goto_prev_move() {
  int r, c;
  wq::PointList added;
  if (!board_->undo(r, c, added)) return false;

  cur_move_--;

  goban_->set_annotation(r, c, AnnotationType::kNone);
  goban_->set_point(r, c, wq::Color::kNone);
  for (const auto& [rr, cc] : added) {
    goban_->set_point(rr, cc, turn_);
  }

  if (auto prev_move = board_->last_move()) {
    const auto& [col, pt] = prev_move.value();
    const auto& [rr, cc] = pt;
    goban_->set_annotation(rr, cc, AnnotationType::kBottomRightTriangle);
    goban_->set_annotation_color(
        rr, cc, turn_ == wq::Color::kBlack ? color_white : color_black);
  }

  toggle_turn();
  return true;
}

bool EditorWindow::goto_next_move() {
  if (cur_move_ >= moves_.size()) return false;

  auto prev_move = board_->last_move();

  const auto& [col, pnt] = moves_[cur_move_];
  const auto& [r, c] = pnt;
  wq::PointList removed;
  board_->move(turn_, r, c, removed);

  cur_move_++;

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
    goban_->set_point(rr, cc, wq::Color::kNone);
  goban_->set_annotation(r, c, AnnotationType::kBottomRightTriangle);
  goban_->set_annotation_color(
      r, c, turn_ == wq::Color::kBlack ? color_white : color_black);

  toggle_turn();
  return true;
}

void EditorWindow::on_board_size_changed(GObject* /*self*/,
                                         GParamSpec* /*pspec*/,
                                         gpointer user_data) {
  EditorWindow* win = (EditorWindow*)user_data;
  guint selected =
      gtk_drop_down_get_selected(GTK_DROP_DOWN(win->board_size_dropdown_));
  if (selected == GTK_INVALID_LIST_POSITION) return;

  const int board_size = std::stoi(kBoardSizes[selected]);
  assert(1 <= board_size && board_size <= 19);
  win->turn_ = wq::Color::kBlack;
  win->board_ = std::make_unique<wq::Board>(board_size, board_size);
  win->goban_->resize(board_size, 0, 0, board_size - 1, board_size - 1);
}

void EditorWindow::on_first_move_clicked(GtkWidget* /*self*/,
                                         gpointer user_data) {
  EditorWindow* win = (EditorWindow*)user_data;
  while (win->goto_prev_move());
}

void EditorWindow::on_prev_n_moves_clicked(GtkWidget* /*self*/,
                                           gpointer user_data) {
  EditorWindow* win = (EditorWindow*)user_data;
  for (int i = 0; i < 10; ++i)
    if (!win->goto_prev_move()) break;
}

void EditorWindow::on_prev_move_clicked(GtkWidget* /*self*/,
                                        gpointer user_data) {
  EditorWindow* win = (EditorWindow*)user_data;
  win->goto_prev_move();
}

void EditorWindow::on_next_move_clicked(GtkWidget* /*self*/,
                                        gpointer user_data) {
  EditorWindow* win = (EditorWindow*)user_data;
  win->goto_next_move();
}

void EditorWindow::on_next_n_moves_clicked(GtkWidget* /*self*/,
                                           gpointer user_data) {
  EditorWindow* win = (EditorWindow*)user_data;
  for (int i = 0; i < 10; ++i)
    if (!win->goto_next_move()) break;
}

void EditorWindow::on_last_move_clicked(GtkWidget* /*self*/,
                                        gpointer user_data) {
  EditorWindow* win = (EditorWindow*)user_data;
  while (win->goto_next_move());
}

}  // namespace ui
