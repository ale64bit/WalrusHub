#pragma once

#include <gtk/gtk.h>

#include <functional>
#include <random>
#include <string>
#include <vector>

#include "wq.h"

namespace ui {

enum class AnnotationType {
  kNone,
  kTerritory,
  kTopLeftTriangle,
  kBottomRightTriangle,
  kTriangle,
};

struct AnnotationState {
  AnnotationType type;
  GdkRGBA color;
};

class GtkBoard {
  using PointCallback = std::function<void(int, int)>;

 public:
  GtkBoard(std::string id, int board_size, int r1, int c1, int r2, int c2,
           std::mt19937 &rand_gen);
  GtkWidget *widget() const;

  void set_board_texture(GdkTexture *tex);
  void set_black_stone_textures(GdkTexture *const *tex, int tex_count);
  void set_white_stone_textures(GdkTexture *const *tex, int tex_count);
  void set_on_point_click(PointCallback cb);
  void set_on_point_enter(PointCallback cb);
  void set_on_point_leave(PointCallback cb);
  void set_point(int r, int c, wq::Color col);
  void set_annotation(int r, int c, AnnotationType a);
  void set_annotation_color(int r, int c, GdkRGBA color);
  void set_text(int r, int c, std::string text);
  void set_text_color(int r, int c, GdkRGBA color);
  void resize(int board_size, int r1, int c1, int r2, int c2);
  void clear();

 private:
  std::mt19937 &rand_gen_;
  const std::string id_;
  int board_size_;
  int row_offset_;
  int col_offset_;
  int board_border_mask_;
  std::vector<std::vector<AnnotationState>> annotations_;
  std::vector<std::vector<std::tuple<GtkBoard *, int, int>>> point_coords_;
  int black_stone_tex_count_ = 0;
  GdkTexture *const *black_stone_tex_ = nullptr;
  int white_stone_tex_count_ = 0;
  GdkTexture *const *white_stone_tex_ = nullptr;
  GtkGrid *grid_ = nullptr;
  GtkDrawingArea *board_lines_ = nullptr;
  GtkOverlay *overlay_ = nullptr;
  GtkAspectFrame *frame_ = nullptr;
  PointCallback on_point_click_;
  PointCallback on_point_enter_;
  PointCallback on_point_leave_;

  static void board_lines_draw_function(GtkDrawingArea *area, cairo_t *cr,
                                        int w, int h, gpointer data);
  static void annotation_draw_function(GtkDrawingArea *area, cairo_t *cr, int w,
                                       int h, gpointer data);
  static void on_board_resized(GtkDrawingArea *self, gint width, gint height,
                               gpointer user_data);
  static void on_point_pressed(GtkGestureClick *self, gint n_press, gdouble x,
                               gdouble y, gpointer user_data);
  static void on_point_enter(GtkGestureClick *self, gdouble x, gdouble y,
                             gpointer user_data);
  static void on_point_leave(GtkGestureClick *self, gpointer user_data);
};

}  // namespace ui
