#include "books_window.h"

#include <sstream>

#include "color.h"
#include "log.h"
#include "solve_window.h"

namespace ui {

BooksWindow::BooksWindow(AppContext& ctx)
    : Window(ctx), books_(ctx.tasks().list_books()) {
  gtk_window_set_title(GTK_WINDOW(window_), "Books");
  gtk_window_set_default_size(GTK_WINDOW(window_), 112 * 13, 1000);

  GtkWidget* box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 4);

  GtkWidget* sidebar_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
  book_listbox_ = gtk_list_box_new();
  gtk_list_box_set_selection_mode(GTK_LIST_BOX(book_listbox_),
                                  GTK_SELECTION_SINGLE);
  chapter_listbox_ = gtk_list_box_new();
  gtk_list_box_set_selection_mode(GTK_LIST_BOX(chapter_listbox_),
                                  GTK_SELECTION_SINGLE);
  GtkWidget* book_list_frame = gtk_frame_new("Books");
  gtk_frame_set_child(GTK_FRAME(book_list_frame), book_listbox_);
  GtkWidget* chapter_list_frame = gtk_frame_new("Chapters");
  gtk_frame_set_child(GTK_FRAME(chapter_list_frame), chapter_listbox_);
  gtk_widget_set_vexpand(chapter_list_frame, true);
  gtk_widget_set_valign(chapter_list_frame, GTK_ALIGN_FILL);

  for (const auto& book : books_) {
    gtk_list_box_append(GTK_LIST_BOX(book_listbox_),
                        gtk_label_new(book.title_en.c_str()));
  }

  g_signal_connect(book_listbox_, "row-selected", G_CALLBACK(on_book_selected),
                   this);
  g_signal_connect(chapter_listbox_, "row-selected",
                   G_CALLBACK(on_chapter_selected), this);

  gtk_box_append(GTK_BOX(sidebar_box), book_list_frame);
  gtk_box_append(GTK_BOX(sidebar_box), chapter_list_frame);
  gtk_box_append(GTK_BOX(box), sidebar_box);
  gtk_box_append(GTK_BOX(box), gtk_separator_new(GTK_ORIENTATION_VERTICAL));

  GtkWidget* preview_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
  GtkWidget* scrolled = gtk_scrolled_window_new();
  gtk_box_append(GTK_BOX(preview_box), scrolled);
  GtkWidget* action_bar = gtk_action_bar_new();
  gtk_box_append(GTK_BOX(preview_box), action_bar);
  GtkWidget* solve_button = gtk_button_new_with_label("Solve");
  book_info_link_ = gtk_link_button_new("Book Info");
  gtk_action_bar_pack_start(GTK_ACTION_BAR(action_bar), book_info_link_);
  gtk_action_bar_pack_end(GTK_ACTION_BAR(action_bar), solve_button);
  g_signal_connect(solve_button, "clicked", G_CALLBACK(on_solve_clicked), this);

  gtk_box_append(GTK_BOX(box), preview_box);

  grid_ = gtk_grid_new();
  gtk_widget_set_hexpand(grid_, true);
  gtk_widget_set_halign(grid_, GTK_ALIGN_CENTER);
  gtk_widget_set_vexpand(grid_, true);
  gtk_widget_set_valign(grid_, GTK_ALIGN_CENTER);
  gtk_grid_set_row_homogeneous(GTK_GRID(grid_), true);
  gtk_grid_set_column_homogeneous(GTK_GRID(grid_), true);
  gtk_grid_set_column_spacing(GTK_GRID(grid_), 24);

  setup_tasks();

  gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled), grid_);

  gtk_window_set_child(GTK_WINDOW(window_), box);
  gtk_window_present(GTK_WINDOW(window_));
}

void BooksWindow::setup_tasks() {
  while (!task_previews_.empty()) {
    gtk_grid_remove(GTK_GRID(grid_), task_previews_.back());
    task_previews_.pop_back();
  }
  int i = 1;
  int r = 0;
  int c = 0;
  for (auto& task : tasks_) {
    GtkWidget* box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    GtkWidget* preview_frame = gtk_frame_new(nullptr);
    GtkWidget* preview = gtk_drawing_area_new();
    gtk_frame_set_child(GTK_FRAME(preview_frame), preview);
    gtk_box_append(GTK_BOX(box), preview_frame);

    GtkWidget* center_box = gtk_center_box_new();
    gtk_center_box_set_start_widget(GTK_CENTER_BOX(center_box),
                                    gtk_label_new(std::to_string(i).c_str()));
    gtk_center_box_set_end_widget(GTK_CENTER_BOX(center_box),
                                  gtk_label_new(rank_string(task.rank_)));
    gtk_box_append(GTK_BOX(box), center_box);

    gtk_drawing_area_set_content_width(GTK_DRAWING_AREA(preview), 112);
    gtk_drawing_area_set_content_height(GTK_DRAWING_AREA(preview), 112);
    gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(preview), draw_task_preview,
                                   &task, nullptr);
    gtk_grid_attach(GTK_GRID(grid_), box, c, r, 1, 1);
    task_previews_.push_back(box);
    if (++c == 8) {
      ++r;
      c = 0;
    }
    ++i;
  }
}

void BooksWindow::draw_task_preview(GtkDrawingArea* /*area*/, cairo_t* cr,
                                    int width, int height, gpointer data) {
  Task* task = (Task*)data;

  auto [r1, c1] = task->top_left_;
  auto [r2, c2] = task->bottom_right_;

  // Make subboard square
  while (r2 - r1 < c2 - c1 && r1 > 0) r1--;
  while (r2 - r1 < c2 - c1 && r2 + 1 < task->board_size_) r2++;
  while (c2 - c1 < r2 - r1 && c1 > 0) c1--;
  while (c2 - c1 < r2 - r1 && c2 + 1 < task->board_size_) c2++;
  assert(r2 - r1 == c2 - c1);

  // Compute border mask
  int border_mask = 0;
  if (r1 == 0) border_mask ^= (1 << 0);                      // top
  if (c1 == 0) border_mask ^= (1 << 1);                      // left
  if (r2 == task->board_size_ - 1) border_mask ^= (1 << 2);  // bottom
  if (c2 == task->board_size_ - 1) border_mask ^= (1 << 3);  // right

  const int board_size = r2 - r1 + 1;
  const int row_offset = r1;
  const int col_offset = c1;

  // Compute point dimensions
  const double psize = width / double(board_size);
  const double hpsize = psize / 2;
  const double line_width = std::max(psize / 30, 1.0);

  // Preview background color
  cairo_rectangle(cr, 0, 0, width, height);
  gdk_cairo_set_source_rgba(cr, &color_white);
  cairo_fill(cr);

  gdk_cairo_set_source_rgba(cr, &color_black);

  // Outer rectangle
  cairo_set_line_width(cr, line_width * 1.8);

  if (border_mask == 0xF) {
    cairo_rectangle(cr, hpsize, hpsize, width - psize, height - psize);
  } else {
    if (border_mask & (1 << 0)) {  // top
      cairo_move_to(cr, hpsize, hpsize + 0 * psize);
      cairo_line_to(cr, width - hpsize, hpsize + 0 * psize);
    }
    if (border_mask & (1 << 1)) {  // left
      cairo_move_to(cr, hpsize + 0 * psize, hpsize);
      cairo_line_to(cr, hpsize + 0 * psize, height - hpsize);
    }
    if (border_mask & (1 << 2)) {  // bottom
      cairo_move_to(cr, hpsize, hpsize + (board_size - 1) * psize);
      cairo_line_to(cr, width - hpsize, hpsize + (board_size - 1) * psize);
    }
    if (border_mask & (1 << 3)) {  // right
      cairo_move_to(cr, hpsize + (board_size - 1) * psize, hpsize);
      cairo_line_to(cr, hpsize + (board_size - 1) * psize, height - hpsize);
    }
  }
  cairo_stroke(cr);

  // Board lines
  cairo_set_line_width(cr, line_width);
  for (int i = 1; i < board_size - 1; ++i) {
    // Horizontal lines
    cairo_move_to(cr, hpsize, hpsize + i * psize);
    cairo_line_to(cr, width - hpsize, hpsize + i * psize);
    // Vertical lines
    cairo_move_to(cr, hpsize + i * psize, hpsize);
    cairo_line_to(cr, hpsize + i * psize, height - hpsize);
  }
  cairo_stroke(cr);

  // Star points
  if (border_mask == 0xF) {
    for (const auto& [r, c] : wq::Board::star_points(task->board_size_)) {
      const double x = hpsize + (c - col_offset) * psize;
      const double y = hpsize + (r - row_offset) * psize;
      cairo_arc(cr, x, y, psize / 10, 0, 2 * M_PI);
      cairo_fill(cr);
    }
  }

  // Stones
  for (const auto& [r, c] : task->initial_[0]) {
    const double x = hpsize + (c - col_offset) * psize;
    const double y = hpsize + (r - row_offset) * psize;
    cairo_arc(cr, x, y, psize / 2.1, 0, 2 * M_PI);
    cairo_fill(cr);
  }
  for (const auto& [r, c] : task->initial_[1]) {
    const double x = hpsize + (c - col_offset) * psize;
    const double y = hpsize + (r - row_offset) * psize;
    gdk_cairo_set_source_rgba(cr, &color_white);
    cairo_arc(cr, x, y, psize / 2.2, 0, 2 * M_PI);
    cairo_fill(cr);
    gdk_cairo_set_source_rgba(cr, &color_black);
    cairo_arc(cr, x, y, psize / 2.2, 0, 2 * M_PI);
    cairo_stroke(cr);
  }
}

void BooksWindow::on_book_selected(GtkListBox* /*self*/, GtkListBoxRow* row,
                                   gpointer user_data) {
  BooksWindow* w = (BooksWindow*)user_data;

  if (!row) {
    gtk_list_box_unselect_all(GTK_LIST_BOX(w->chapter_listbox_));
    gtk_list_box_remove_all(GTK_LIST_BOX(w->chapter_listbox_));
    return;
  }

  const int index = gtk_list_box_row_get_index(row);
  w->selected_book_ = index;
  w->selected_chapter_ = -1;
  gtk_link_button_set_uri(GTK_LINK_BUTTON(w->book_info_link_),
                          w->books_[index].url.c_str());
  gtk_list_box_unselect_all(GTK_LIST_BOX(w->chapter_listbox_));
  gtk_list_box_remove_all(GTK_LIST_BOX(w->chapter_listbox_));
  w->chapters_ = w->ctx_.tasks().list_book_chapters(w->books_[index].id);
  for (const auto& chapter : w->chapters_) {
    gtk_list_box_append(GTK_LIST_BOX(w->chapter_listbox_),
                        gtk_label_new(chapter.title.c_str()));
  }
  w->tasks_.clear();
  w->setup_tasks();
}

void BooksWindow::on_chapter_selected(GtkListBox* /*self*/, GtkListBoxRow* row,
                                      gpointer user_data) {
  if (!row) return;

  BooksWindow* w = (BooksWindow*)user_data;
  const int index = gtk_list_box_row_get_index(row);
  w->selected_chapter_ = index;
  w->tasks_ = w->ctx_.tasks().list_book_tasks(w->chapters_[index].book_id,
                                              w->chapters_[index].id);
  w->setup_tasks();
}

void BooksWindow::on_solve_clicked(GtkWidget* /*self*/, gpointer user_data) {
  BooksWindow* w = (BooksWindow*)user_data;
  if (w->tasks_.empty()) return;
  new SolveWindow(w->ctx_,
                  w->books_[w->selected_book_].title_en + " - " +
                      w->chapters_[w->selected_chapter_].title,
                  w->tasks_, 10 * 60);
}

}  // namespace ui