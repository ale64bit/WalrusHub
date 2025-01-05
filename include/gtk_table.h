#pragma once

#include <glib-object.h>
#include <gtk/gtk.h>

#include <functional>
#include <tuple>
#include <utility>
#include <vector>

#include "log.h"

G_BEGIN_DECLS

#define TABLE_ROW_TYPE table_row_get_type()
G_DECLARE_FINAL_TYPE(TableRow, table_row, TABLE, ROW, GObject)

G_END_DECLS

namespace ui {

template <class T>
class GtkTable {
  static constexpr size_t kMaxColumns = 16;

 public:
  using SetupFunc = std::function<GtkWidget*(const T& t)>;

  GtkTable() {
    model_ = g_list_store_new(TABLE_ROW_TYPE);
    selection_ = gtk_single_selection_new(G_LIST_MODEL(model_));
    view_ =
        GTK_COLUMN_VIEW(gtk_column_view_new(GTK_SELECTION_MODEL(selection_)));
    column_callbacks_.reserve(kMaxColumns);

    gtk_single_selection_set_autoselect(GTK_SINGLE_SELECTION(selection_),
                                        false);
    g_signal_connect(GTK_SINGLE_SELECTION(selection_), "notify::selected",
                     G_CALLBACK(on_selected), this);
  }

  operator GtkWidget*() const { return GTK_WIDGET(view_); }

  void set_selection_func(std::function<void(int)> f) { selection_func_ = f; }

  void add_column(const char* title, SetupFunc setup_func) {
    if (column_callbacks_.size() == kMaxColumns) {
      LOG(ERROR) << "GtkTable cannot have more than " << kMaxColumns
                 << " columns";
      std::exit(1);
    }
    GtkListItemFactory* factory = gtk_signal_list_item_factory_new();
    GtkColumnViewColumn* column = gtk_column_view_column_new(title, factory);
    column_callbacks_.emplace_back(this, column, setup_func);

    gtk_column_view_append_column(view_, column);

    g_signal_connect(factory, "setup", G_CALLBACK(on_setup),
                     &column_callbacks_.back());
    g_signal_connect(factory, "bind", G_CALLBACK(on_bind),
                     &column_callbacks_.back());
    g_signal_connect(factory, "unbind", G_CALLBACK(on_unbind),
                     &column_callbacks_.back());
    g_signal_connect(factory, "teardown", G_CALLBACK(on_teardown),
                     &column_callbacks_.back());
  }

  void add_row(T t) {
    GObject* obj = G_OBJECT(g_object_new(TABLE_ROW_TYPE, nullptr));
    g_object_set_data(obj, "index", GSIZE_TO_POINTER(items_.size()));
    items_.push_back(t);
    g_list_store_append(model_, obj);
  }

  void update_row(size_t i, T t) {
    g_list_store_remove(model_, i);
    GObject* obj = G_OBJECT(g_object_new(TABLE_ROW_TYPE, nullptr));
    g_object_set_data(obj, "index", GSIZE_TO_POINTER(i));
    items_[i] = t;
    g_list_store_insert(model_, i, obj);
  }

  size_t size() const { return items_.size(); }

 private:
  GListStore* model_;
  GtkSingleSelection* selection_;
  std::function<void(int)> selection_func_;
  GtkColumnView* view_;
  std::vector<std::tuple<GtkTable<T>*, GtkColumnViewColumn*, SetupFunc>>
      column_callbacks_;
  std::vector<T> items_;

  static void on_setup(GtkSignalListItemFactory* /*self*/, GObject* /*obj*/,
                       gpointer /*user_data*/) {}

  static void on_bind(GtkSignalListItemFactory* /*self*/, GObject* obj,
                      gpointer user_data) {
    const auto& [table, _, setup_func] = *(
        std::tuple<GtkTable<T>*, GtkColumnViewColumn*, SetupFunc>*)(user_data);
    GObject* item = G_OBJECT(gtk_list_item_get_item(GTK_LIST_ITEM(obj)));
    const size_t index = GPOINTER_TO_SIZE(g_object_get_data(item, "index"));
    GtkWidget* widget = setup_func(table->items_[index]);
    gtk_list_item_set_child(GTK_LIST_ITEM(obj), widget);
  }

  static void on_unbind(GtkSignalListItemFactory* /*self*/, GObject* /*object*/,
                        gpointer /*user_data*/) {}

  static void on_teardown(GtkSignalListItemFactory* /*self*/,
                          GObject* /*object*/, gpointer /*user_data*/) {}

  static void on_selected(GObject* /*self*/, GParamSpec* /*pspec*/,
                          gpointer user_data) {
    GtkTable<T>* table = (GtkTable<T>*)user_data;
    if (table->selection_func_) {
      const guint selected =
          gtk_single_selection_get_selected(table->selection_);
      table->selection_func_(selected + 1);
    }
  }
};

}  // namespace ui