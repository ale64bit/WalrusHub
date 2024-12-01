#include "app_context.h"

#include <cstdlib>
#include <iomanip>
#include <sstream>

#include "log.h"

constexpr const gchar *kKataGoConfigGroup = "katago";

static GdkTexture *load_texture_from_file(const char *path) {
  auto tex_file = g_file_new_for_path(path);
  auto tex = gdk_texture_new_from_file(tex_file, nullptr);
  g_object_unref(tex_file);
  return tex;
}

AppContext::AppContext(const char *task_db_path, const char *stats_db_path,
                       RunFunc run_func)
    : rand_gen_(rand_dev_()),
      run_func_(run_func),
      task_db_(task_db_path),
      stats_db_(stats_db_path) {
  app_ = gtk_application_new("ru.walruswq.hub", G_APPLICATION_DEFAULT_FLAGS);
  g_signal_connect(app_, "activate", G_CALLBACK(activate), this);
}

AppContext::~AppContext() {
  if (katago_client_) katago_client_->stop();
  g_object_unref(app_);
}

int AppContext::run(int argc, char **argv) {
  return g_application_run(G_APPLICATION(app_), argc, argv);
}

void AppContext::activate(GtkApplication * /*gtkApp*/, gpointer user_data) {
  AppContext *app_ctx = (AppContext *)user_data;

  // Load textures
  app_ctx->res.logo = load_texture_from_file("assets/walruswq_logo.png");
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

  // Load styling
  GtkCssProvider *css_provider = gtk_css_provider_new();
  gtk_css_provider_load_from_file(css_provider,
                                  g_file_new_for_path("assets/styles.css"));
  gtk_style_context_add_provider_for_display(
      gdk_display_get_default(), GTK_STYLE_PROVIDER(css_provider),
      GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
  g_object_unref(css_provider);

  // Load config
  app_ctx->config_filename_ =
      fs::path(g_get_user_config_dir()) / "WalrusHub.ini";
  app_ctx->config_key_file_ = g_key_file_new();
  if (fs::exists(app_ctx->config_filename_)) {
    if (!g_key_file_load_from_file(app_ctx->config_key_file_,
                                   app_ctx->config_filename_.c_str(),
                                   G_KEY_FILE_NONE, nullptr)) {
      LOG(ERROR) << "error loading config file: " << app_ctx->config_filename_;
      std::exit(1);
    }
    LOG(INFO) << "config file loaded: " << app_ctx->config_filename_;
    app_ctx->reload_katago();
  }

  app_ctx->run_func_(*app_ctx);
}

void AppContext::reload_katago() {
  auto bin_path = get_katago_path();
  auto config_path = get_katago_config_path();
  auto model_path = get_katago_model_path();
  auto human_model_path = get_katago_human_model_path();
  if (!bin_path || !config_path || !model_path || !human_model_path) return;

  katago_client_ = std::make_unique<KataGoClient>(
      bin_path, model_path, human_model_path, config_path);
}

void AppContext::set_katago_path(fs::path p) {
  g_key_file_set_string(config_key_file_, kKataGoConfigGroup, "path",
                        p.c_str());
  flush_config();
  reload_katago();
}

gchar *AppContext::get_katago_path() const {
  return g_key_file_get_string(config_key_file_, kKataGoConfigGroup, "path",
                               nullptr);
}

void AppContext::set_katago_config_path(fs::path p) {
  g_key_file_set_string(config_key_file_, kKataGoConfigGroup, "config_path",
                        p.c_str());
  flush_config();
  reload_katago();
}

gchar *AppContext::get_katago_config_path() const {
  return g_key_file_get_string(config_key_file_, kKataGoConfigGroup,
                               "config_path", nullptr);
}

void AppContext::set_katago_model_path(fs::path p) {
  g_key_file_set_string(config_key_file_, kKataGoConfigGroup, "model_path",
                        p.c_str());
  flush_config();
  reload_katago();
}

gchar *AppContext::get_katago_model_path() const {
  return g_key_file_get_string(config_key_file_, kKataGoConfigGroup,
                               "model_path", nullptr);
}

void AppContext::set_katago_human_model_path(fs::path p) {
  g_key_file_set_string(config_key_file_, kKataGoConfigGroup,
                        "human_model_path", p.c_str());
  flush_config();
  reload_katago();
}

gchar *AppContext::get_katago_human_model_path() const {
  return g_key_file_get_string(config_key_file_, kKataGoConfigGroup,
                               "human_model_path", nullptr);
}

void AppContext::flush_config() {
  if (!g_key_file_save_to_file(config_key_file_, config_filename_.c_str(),
                               nullptr)) {
    LOG(ERROR) << "error writing config file";
    std::exit(1);
  }
}