#pragma once

#include <gtk/gtk.h>

#include <filesystem>
#include <functional>
#include <memory>
#include <random>
#include <vector>

#include "http.h"
#include "katago_client.h"
#include "stats.h"
#include "task.h"

namespace fs = std::filesystem;

class AppContext {
  using RunFunc = std::function<void(AppContext &)>;

 public:
  AppContext(const char *task_db_path, const char *stats_db_path,
             RunFunc run_func);
  ~AppContext();

  int run(int argc, char **argv);
  GdkTexture *logo() const { return res.logo; }
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
  StatsDB &stats() { return stats_db_; }
  http::Client &http() { return http_; }
  GtkApplication *gtk_app() { return app_; }
  std::mt19937 &rand() { return rand_gen_; }
  void reload_katago();
  KataGoClient *katago() { return katago_client_.get(); }

  // Config
  void set_katago_path(fs::path p);
  gchar *get_katago_path() const;
  void set_katago_config_path(fs::path p);
  gchar *get_katago_config_path() const;
  void set_katago_model_path(fs::path p);
  gchar *get_katago_model_path() const;
  void set_katago_human_model_path(fs::path p);
  gchar *get_katago_human_model_path() const;
  void set_appearance_theme_dark(bool);
  bool get_appearance_theme_dark() const;
  void flush_config();

 private:
  std::random_device rand_dev_;
  std::mt19937 rand_gen_;
  RunFunc run_func_;
  GtkApplication *app_;
  fs::path config_filename_;
  GKeyFile *config_key_file_;
  http::Client http_;
  TaskDB task_db_;
  StatsDB stats_db_;
  std::unique_ptr<KataGoClient> katago_client_;
  struct {
    GdkTexture *logo;
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