#include "gtk_table.h"

struct _TableRow {
  GObject parent_instance;
};

G_DEFINE_TYPE(TableRow, table_row, G_TYPE_OBJECT)

static void table_row_class_init(TableRowClass *) {}

static void table_row_init(TableRow *) {}
