#pragma once

#include <gtk/gtk.h>

#include <functional>
#include <memory>
#include <random>
#include <vector>

#include "http.h"
#include "task.h"

class AppContext {
  using RunFunc = std::function<void(AppContext &)>;

 public:
  AppContext(const char *task_db_path, RunFunc run_func);
  ~AppContext();

  int run(int argc, char **argv);
  GdkTexture *board_texture() const { return res.board_tex; }
  const std::vector<GdkTexture *> &black_stone_textures() const {
    return res.black_stone_tex;
  }
  const std::vector<GdkTexture *> &white_stone_textures() const {
    return res.white_stone_tex;
  }
  GtkMediaStream *play_stone_sound() const { return sound.play_stone; }
  GtkMediaStream *capture_one_sound() const { return sound.capture_one; }
  GtkMediaStream *capture_few_sound() const { return sound.capture_few; }
  GtkMediaStream *capture_many_sound() const { return sound.capture_many; }
  TaskDB &tasks() { return task_db_; }
  Http &http() { return http_; }
  GtkApplication *gtk_app() { return app_; }
  std::mt19937 &rand() { return rand_gen_; }

 private:
  std::random_device rand_dev_;
  std::mt19937 rand_gen_;
  RunFunc run_func_;
  GtkApplication *app_;
  Http http_;
  TaskDB task_db_;
  struct {
    GdkTexture *board_tex;
    std::vector<GdkTexture *> black_stone_tex;
    std::vector<GdkTexture *> white_stone_tex;
  } res;
  struct {
    GtkMediaStream *play_stone;
    GtkMediaStream *capture_one;
    GtkMediaStream *capture_few;
    GtkMediaStream *capture_many;
  } sound;

  static void activate(GtkApplication * /*gtkApp*/, gpointer user_data);
};