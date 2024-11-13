#include "window.h"

#include "log.h"

namespace ui {

Window::Window(AppContext& ctx)
    : ctx_(ctx), window_(gtk_application_window_new(ctx.gtk_app())) {
  g_signal_connect(GTK_WINDOW(window_), "destroy", G_CALLBACK(on_destroy),
                   this);
}

Window::~Window() { window_ = nullptr; }

void Window::on_destroy(GtkWidget* /*self*/, gpointer user_data) {
  Window* win = (Window*)user_data;
  delete win;  // suicide
}

}  // namespace ui