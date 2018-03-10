#ifndef __OCT_TASK_VIEW_H__
#define __OCT_TASK_VIEW_H__

#include <gtk/gtk.h>
#include <sqlite3.h>

G_BEGIN_DECLS

#define OCT_TYPE_TASK_VIEW oct_task_view_get_type()
G_DECLARE_FINAL_TYPE(OctTaskView, oct_task_view, OCT, TASK_VIEW, GtkTreeView)

GtkWidget*
oct_task_view_new(void);

G_END_DECLS

#endif
