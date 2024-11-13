#include "gtkgoban.h"

#include <cassert>
#include <map>

#include "board.h"
#include "log.h"

static const std::map<int, std::vector<std::pair<int, int>>>
    kStarPointPositions = {
        {
            9,
            {
                {2, 2},
                {2, 6},
                {6, 2},
                {6, 6},
                {4, 4},
            },
        },
        {
            19,
            {
                {3, 3},
                {3, 9},
                {3, 15},
                {9, 3},
                {9, 9},
                {9, 15},
                {15, 3},
                {15, 9},
                {15, 15},
            },
        },
};

GtkGoban::GtkGoban(std::string id, int board_size, int r1, int c1, int r2,
                   int c2, std::mt19937 &rand_gen)
    : rand_gen_(rand_gen), id_(id), board_size_(0) {
  grid_ = (GtkGrid *)gtk_grid_new();
  gtk_grid_set_row_spacing(grid_, 0);
  gtk_grid_set_column_spacing(grid_, 0);
  gtk_grid_set_row_homogeneous(grid_, true);
  gtk_grid_set_column_homogeneous(grid_, true);

  board_lines_ = GTK_DRAWING_AREA(gtk_drawing_area_new());
  gtk_widget_set_hexpand(GTK_WIDGET(board_lines_), true);
  gtk_widget_set_vexpand(GTK_WIDGET(board_lines_), true);
  gtk_drawing_area_set_draw_func(board_lines_, board_lines_draw_function, this,
                                 nullptr);

  g_signal_connect(GTK_WIDGET(board_lines_), "resize",
                   G_CALLBACK(on_board_resized), this);

  resize(board_size, r1, c1, r2, c2);

  overlay_ = (GtkOverlay *)gtk_overlay_new();
  gtk_overlay_add_overlay(overlay_, GTK_WIDGET(board_lines_));
  gtk_overlay_add_overlay(overlay_, GTK_WIDGET(grid_));

  frame_ = (GtkAspectFrame *)gtk_aspect_frame_new(0.5, 0.5, 1.0, false);
  gtk_widget_set_halign((GtkWidget *)frame_, GTK_ALIGN_FILL);
  gtk_widget_set_valign((GtkWidget *)frame_, GTK_ALIGN_FILL);
  gtk_widget_set_hexpand((GtkWidget *)frame_, true);
  gtk_widget_set_vexpand((GtkWidget *)frame_, true);
  gtk_aspect_frame_set_child(frame_, (GtkWidget *)overlay_);
}

GtkWidget *GtkGoban::widget() const { return (GtkWidget *)frame_; }

void GtkGoban::set_board_texture(GdkTexture *tex) {
  auto img = gtk_image_new_from_paintable((GdkPaintable *)tex);
  gtk_overlay_set_child(overlay_, GTK_WIDGET(img));
  gtk_overlay_set_clip_overlay(overlay_, GTK_WIDGET(img), true);
}

void GtkGoban::set_black_stone_textures(GdkTexture *const *tex, int tex_count) {
  black_stone_tex_count_ = tex_count;
  black_stone_tex_ = tex;
}

void GtkGoban::set_white_stone_textures(GdkTexture *const *tex, int tex_count) {
  white_stone_tex_count_ = tex_count;
  white_stone_tex_ = tex;
}

void GtkGoban::set_on_point_click(PointCallback cb) { on_point_click_ = cb; }

void GtkGoban::set_on_point_enter(PointCallback cb) { on_point_enter_ = cb; }

void GtkGoban::set_on_point_leave(PointCallback cb) { on_point_leave_ = cb; }

void GtkGoban::set_point(int r, int c, wq::Color col) {
  r -= row_offset_;
  c -= col_offset_;
  auto overlay = (GtkOverlay *)gtk_grid_get_child_at(grid_, c, r);
  auto img = (GtkImage *)gtk_overlay_get_child(overlay);
  switch (col) {
    case wq::Color::kEmpty:
      gtk_image_clear(img);
      break;
    case wq::Color::kBlack: {
      std::uniform_int_distribution<> dist(0, black_stone_tex_count_ - 1);
      gtk_image_set_from_paintable(
          img, (GdkPaintable *)black_stone_tex_[dist(rand_gen_)]);
      break;
    }
    case wq::Color::kWhite: {
      std::uniform_int_distribution<> dist(0, white_stone_tex_count_ - 1);
      gtk_image_set_from_paintable(
          img, (GdkPaintable *)white_stone_tex_[dist(rand_gen_)]);
      break;
    }
  }
}

void GtkGoban::set_annotation(int r, int c, AnnotationType a) {
  r -= row_offset_;
  c -= col_offset_;
  annotations_[r][c].type = a;
  auto overlay = gtk_grid_get_child_at(grid_, c, r);
  auto annotation_area = gtk_widget_get_last_child(overlay);
  gtk_widget_queue_draw(annotation_area);
}

void GtkGoban::set_annotation_color(int r, int c, GdkRGBA color) {
  r -= row_offset_;
  c -= col_offset_;
  annotations_[r][c].color = color;
  auto overlay = gtk_grid_get_child_at(grid_, c, r);
  auto annotation_area = gtk_widget_get_last_child(overlay);
  gtk_widget_queue_draw(annotation_area);
}

void GtkGoban::set_text(int r, int c, std::string text) {
  r -= row_offset_;
  c -= col_offset_;
  auto overlay = (GtkOverlay *)gtk_grid_get_child_at(grid_, c, r);
  auto text_area =
      (GtkLabel *)gtk_widget_get_next_sibling(gtk_overlay_get_child(overlay));
  gtk_label_set_text(text_area, text.c_str());
}

void GtkGoban::set_text_color(int r, int c, GdkRGBA color) {
  r -= row_offset_;
  c -= col_offset_;
  auto overlay = (GtkOverlay *)gtk_grid_get_child_at(grid_, c, r);
  auto text_area =
      (GtkLabel *)gtk_widget_get_next_sibling(gtk_overlay_get_child(overlay));

  if (auto attrs = gtk_label_get_attributes(text_area)) {
    auto attr_fg = pango_attr_foreground_new(
        color.red * 0xFFFF, color.green * 0xFFFF, color.blue * 0xFFFF);
    pango_attr_list_change(attrs, attr_fg);
    gtk_label_set_attributes(text_area, attrs);
  }
}

void GtkGoban::board_lines_draw_function(GtkDrawingArea *, cairo_t *cr, int w,
                                         int h, gpointer data) {
  GtkGoban *goban = (GtkGoban *)data;

  // Dimensions
  const double psize = w / double(goban->board_size_);
  const double hpsize = psize / 2;
  const double line_width = std::max(psize / 30, 1.0);

  // Outer rectangle
  GdkRGBA black{0, 0, 0, 1};
  gdk_cairo_set_source_rgba(cr, &black);
  cairo_set_line_width(cr, line_width * 1.8);

  if (goban->board_border_mask_ == 0xF) {
    cairo_rectangle(cr, hpsize, hpsize, w - psize, h - psize);
  } else {
    if (goban->board_border_mask_ & (1 << 0)) {  // top
      cairo_move_to(cr, hpsize, hpsize + 0 * psize);
      cairo_line_to(cr, w - hpsize, hpsize + 0 * psize);
    }
    if (goban->board_border_mask_ & (1 << 1)) {  // left
      cairo_move_to(cr, hpsize + 0 * psize, hpsize);
      cairo_line_to(cr, hpsize + 0 * psize, h - hpsize);
    }
    if (goban->board_border_mask_ & (1 << 2)) {  // bottom
      cairo_move_to(cr, hpsize, hpsize + (goban->board_size_ - 1) * psize);
      cairo_line_to(cr, w - hpsize, hpsize + (goban->board_size_ - 1) * psize);
    }
    if (goban->board_border_mask_ & (1 << 3)) {  // right
      cairo_move_to(cr, hpsize + (goban->board_size_ - 1) * psize, hpsize);
      cairo_line_to(cr, hpsize + (goban->board_size_ - 1) * psize, h - hpsize);
    }
  }
  cairo_stroke(cr);

  cairo_set_line_width(cr, line_width);
  for (int i = 1; i < goban->board_size_ - 1; ++i) {
    // Horizontal lines
    cairo_move_to(cr, hpsize, hpsize + i * psize);
    cairo_line_to(cr, w - hpsize, hpsize + i * psize);
    // Vertical lines
    cairo_move_to(cr, hpsize + i * psize, hpsize);
    cairo_line_to(cr, hpsize + i * psize, h - hpsize);
  }
  cairo_stroke(cr);

  // Star points
  if (goban->board_border_mask_ == 0xF) {
    if (auto positions = kStarPointPositions.find(goban->board_size_);
        positions != kStarPointPositions.end()) {
      for (const auto &[r, c] : positions->second) {
        const double x = hpsize + c * psize;
        const double y = hpsize + r * psize;
        cairo_arc(cr, x, y, psize / 10, 0, 2 * M_PI);
        cairo_fill(cr);
      }
    }
  }
}

void GtkGoban::annotation_draw_function(GtkDrawingArea *, cairo_t *cr, int w,
                                        int h, gpointer data) {
  const AnnotationState annotation = *reinterpret_cast<AnnotationState *>(data);

  switch (annotation.type) {
    case AnnotationType::kNone:
      break;
    case AnnotationType::kTerritory:
      gdk_cairo_set_source_rgba(cr, &annotation.color);
      cairo_rectangle(cr, w / 4.0, h / 4.0, w / 2.0, h / 2.0);
      cairo_fill(cr);
      break;
    case AnnotationType::kTopLeftTriangle:
      gdk_cairo_set_source_rgba(cr, &annotation.color);
      cairo_move_to(cr, 0, 0);
      cairo_line_to(cr, 0, w / 4.0);
      cairo_line_to(cr, h / 4.0, 0);
      cairo_close_path(cr);
      cairo_fill(cr);
      break;
    case AnnotationType::kBottomRightTriangle:
      gdk_cairo_set_source_rgba(cr, &annotation.color);
      cairo_move_to(cr, w / 2.0, h / 2.0);
      cairo_line_to(cr, w, h / 2.0);
      cairo_line_to(cr, w / 2.0, h);
      cairo_fill(cr);
      break;
  }
}

void GtkGoban::on_board_resized(GtkDrawingArea *, gint width, gint,
                                gpointer user_data) {
  GtkGoban *goban = (GtkGoban *)user_data;
  const auto font_size = width * 512 / goban->board_size_;
  for (int i = 0; i < goban->board_size_; ++i) {
    for (int j = 0; j < goban->board_size_; ++j) {
      auto overlay = (GtkOverlay *)gtk_grid_get_child_at(goban->grid_, j, i);
      auto text_area = (GtkLabel *)gtk_widget_get_next_sibling(
          gtk_overlay_get_child(overlay));
      if (auto attrs = gtk_label_get_attributes(text_area)) {
        auto attr_size = pango_attr_size_new(font_size);
        pango_attr_list_change(attrs, attr_size);
        gtk_label_set_attributes(text_area, attrs);
      }
    }
  }
}

void GtkGoban::on_point_pressed(GtkGestureClick *, gint /*n_press*/,
                                gdouble /*x*/, gdouble /*y*/,
                                gpointer user_data) {
  const auto &[ptr, r, c] = *(std::tuple<GtkGoban *, int, int> *)user_data;
  if (ptr->on_point_click_) ptr->on_point_click_(r, c);
}

void GtkGoban::on_point_enter(GtkGestureClick *, gint /*n_press*/,
                              gdouble /*x*/, gdouble /*y*/,
                              gpointer user_data) {
  const auto &[ptr, r, c] = *(std::tuple<GtkGoban *, int, int> *)user_data;
  if (ptr->on_point_enter_) ptr->on_point_enter_(r, c);
}

void GtkGoban::on_point_leave(GtkGestureClick *, gint /*n_press*/,
                              gdouble /*x*/, gdouble /*y*/,
                              gpointer user_data) {
  const auto &[ptr, r, c] = *(std::tuple<GtkGoban *, int, int> *)user_data;
  if (ptr->on_point_leave_) ptr->on_point_leave_(r, c);
}

void GtkGoban::resize(int board_size, int r1, int c1, int r2, int c2) {
  // Clear current board data
  clear();
  for (int i = board_size_ - 1; i >= 0; --i) {
    gtk_grid_remove_row(grid_, i);
  }

  // Make subboard square
  while (r2 - r1 < c2 - c1 && r1 > 0) r1--;
  while (r2 - r1 < c2 - c1 && r2 + 1 < board_size) r2++;
  while (c2 - c1 < r2 - r1 && c1 > 0) c1--;
  while (c2 - c1 < r2 - r1 && c2 + 1 < board_size) c2++;
  assert(r2 - r1 == c2 - c1);

  // Compute border mask
  board_border_mask_ = 0;
  if (r1 == 0) board_border_mask_ ^= (1 << 0);               // top
  if (c1 == 0) board_border_mask_ ^= (1 << 1);               // left
  if (r2 == board_size - 1) board_border_mask_ ^= (1 << 2);  // bottom
  if (c2 == board_size - 1) board_border_mask_ ^= (1 << 3);  // right

  board_size_ = r2 - r1 + 1;
  row_offset_ = r1;
  col_offset_ = c1;

  // Create new board data
  annotations_.resize(board_size_);
  point_coords_.resize(board_size_);
  for (int i = 0; i < board_size_; ++i) {
    annotations_[i].resize(board_size_);
    point_coords_[i].resize(board_size_);
  }

  for (int i = 0; i < board_size_; ++i) {
    for (int j = 0; j < board_size_; ++j) {
      point_coords_[i][j] = {this, row_offset_ + i, col_offset_ + j};

      auto click_ctrl = gtk_gesture_click_new();
      g_signal_connect(click_ctrl, "pressed", G_CALLBACK(on_point_pressed),
                       &point_coords_[i][j]);

      auto motion_ctrl = gtk_event_controller_motion_new();
      g_signal_connect(motion_ctrl, "enter", G_CALLBACK(on_point_enter),
                       &point_coords_[i][j]);
      g_signal_connect(motion_ctrl, "leave", G_CALLBACK(on_point_leave),
                       &point_coords_[i][j]);

      auto stone_img = gtk_image_new();
      gtk_widget_set_hexpand(stone_img, true);
      gtk_widget_set_vexpand(stone_img, true);

      auto annotation_area = gtk_drawing_area_new();
      gtk_widget_set_hexpand(annotation_area, true);
      gtk_widget_set_vexpand(annotation_area, true);
      gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(annotation_area),
                                     annotation_draw_function,
                                     (void *)&annotations_[i][j], nullptr);

      auto text_area = gtk_label_new("");
      gtk_widget_set_halign(text_area, GTK_ALIGN_CENTER);
      gtk_widget_set_valign(text_area, GTK_ALIGN_CENTER);

      auto text_attrs = pango_attr_list_new();
      auto font_desc = pango_font_description_new();
      pango_attr_list_insert(text_attrs, pango_attr_font_desc_new(font_desc));
      gtk_label_set_attributes((GtkLabel *)text_area, text_attrs);

      auto overlay = gtk_overlay_new();
      gtk_overlay_set_child((GtkOverlay *)overlay, stone_img);
      gtk_overlay_add_overlay((GtkOverlay *)overlay, text_area);
      gtk_overlay_add_overlay((GtkOverlay *)overlay, annotation_area);

      gtk_widget_add_controller(overlay, (GtkEventController *)click_ctrl);
      gtk_widget_add_controller(overlay, (GtkEventController *)motion_ctrl);

      gtk_grid_attach(grid_, overlay, j, i, 1, 1);
    }
  }

  // Rescale annotation text and queue drawing of the board lines
  on_board_resized(board_lines_, gtk_widget_get_width(GTK_WIDGET(board_lines_)),
                   0, this);
  gtk_widget_queue_draw(GTK_WIDGET(board_lines_));
}

void GtkGoban::clear() {
  for (int i = 0; i < board_size_; ++i) {
    for (int j = 0; j < board_size_; ++j) {
      const int ii = row_offset_ + i;
      const int jj = col_offset_ + j;
      set_point(ii, jj, wq::Color::kEmpty);
      set_text(ii, jj, "");
      set_annotation(ii, jj, AnnotationType::kNone);
    }
  }
}