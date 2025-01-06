#pragma once

#include <gtk/gtk.h>

#include <string>
#include <vector>

#include "app_context.h"
#include "task.h"
#include "window.h"

namespace ui {

class BooksWindow : public Window {
 public:
  BooksWindow(AppContext &ctx);

 private:
  // State
  std::vector<Book> books_;
  std::vector<BookChapter> chapters_;
  std::vector<Task> tasks_;
  int selected_book_;
  int selected_chapter_;

  // Widgets
  GtkWidget *grid_;
  GtkWidget *book_listbox_;
  GtkWidget *chapter_listbox_;
  GtkWidget *book_info_link_;
  std::vector<GtkWidget *> task_previews_;

  void setup_tasks();

  static void draw_task_preview(GtkDrawingArea *area, cairo_t *cr, int width,
                                int height, gpointer data);
  static void on_book_selected(GtkListBox *self, GtkListBoxRow *row,
                               gpointer user_data);
  static void on_chapter_selected(GtkListBox *self, GtkListBoxRow *row,
                                  gpointer user_data);
  static void on_solve_clicked(GtkWidget *widget, gpointer data);
  static void on_book_info_clicked(GtkWidget *widget, gpointer data);
};

}  // namespace ui