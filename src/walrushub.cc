#include "app_context.h"
#include "main_window.h"

int main(int argc, char **argv) {
  AppContext app_ctx("assets/tasks.db", "stats.db",
                     [](AppContext &app_ctx) { new ui::MainWindow(app_ctx); });
  return app_ctx.run(argc, argv);
}
