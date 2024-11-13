#include "app_context.h"

#include <cstdlib>
#include <iomanip>
#include <sstream>

#include "log.h"

static GdkTexture *load_texture_from_file(const char *path) {
  auto tex_file = g_file_new_for_path(path);
  auto tex = gdk_texture_new_from_file(tex_file, nullptr);
  g_object_unref(tex_file);
  return tex;
}

AppContext::AppContext(const char *task_db_path, RunFunc run_func)
    : rand_gen_(rand_dev_()), run_func_(run_func), task_db_(task_db_path) {
  app_ = gtk_application_new("ru.walruswq.hub", G_APPLICATION_DEFAULT_FLAGS);
  g_signal_connect(app_, "activate", G_CALLBACK(activate), this);
}

AppContext::~AppContext() { g_object_unref(app_); }

int AppContext::run(int argc, char **argv) {
  return g_application_run(G_APPLICATION(app_), argc, argv);
}

void AppContext::activate(GtkApplication * /*gtkApp*/, gpointer user_data) {
  AppContext *app_ctx = (AppContext *)user_data;

  // Load textures
  app_ctx->res.board_tex = load_texture_from_file("assets/board.png");
  app_ctx->res.black_stone_tex.push_back(
      load_texture_from_file("assets/stone_black.png"));
  app_ctx->res.white_stone_tex.push_back(
      load_texture_from_file("assets/stone_white.png"));

  // Load sounds
  app_ctx->sound.play_stone =
      gtk_media_file_new_for_filename("assets/play_stone.wav");
  app_ctx->sound.capture_one =
      gtk_media_file_new_for_filename("assets/capture_one.wav");
  app_ctx->sound.capture_few =
      gtk_media_file_new_for_filename("assets/capture_few.wav");
  app_ctx->sound.capture_many =
      gtk_media_file_new_for_filename("assets/capture_many.wav");

  app_ctx->run_func_(*app_ctx);
}