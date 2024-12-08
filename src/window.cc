#include "window.h"

#include "log.h"

namespace ui {

std::unordered_map<std::string, std::set<Window*>> Window::window_groups_;

Window::Window(AppContext& ctx) : Window(ctx, "") {}

Window::Window(AppContext& ctx, std::string group_name)
    : ctx_(ctx),
      group_(group_name),
      window_(gtk_application_window_new(ctx.gtk_app())) {
  g_signal_connect(GTK_WINDOW(window_), "destroy", G_CALLBACK(on_destroy),
                   this);
  if (!group_.empty()) window_groups_[group_].insert(this);
}

Window::~Window() {
  if (!group_.empty()) window_groups_[group_].erase(this);
  window_ = nullptr;
}

void Window::update() {}

void Window::update_group(std::string group_name) {
  auto it = window_groups_.find(group_name);
  if (it != window_groups_.end()) {
    for (Window* w : it->second) w->update();
  }
}

void Window::on_destroy(GtkWidget* /*self*/, gpointer user_data) {
  Window* win = (Window*)user_data;
  delete win;  // suicide
}

}  // namespace ui