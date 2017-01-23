#include <string.h>
#include <stdlib.h>

#include "oct-task-view.h"

static void
oct_task_view_new_entry_edited(GtkCellRendererText*, gchar*, gchar*, gpointer);

struct _OctTaskView {
    GtkTreeView parent_instance;
};
G_DEFINE_TYPE(OctTaskView, oct_task_view, GTK_TYPE_TREE_VIEW)

enum {
    COLUMN_ROWID,
    COLUMN_CHECKED,
    COLUMN_TITLE,
    COLUMN_CHECKBOX_VISIBLE,
    COLUMN_EDITABLE,
    NUM_COLUMNS
};

static void
oct_task_view_class_init(OctTaskViewClass* class)
{
}

static void
oct_task_view_init(OctTaskView* self)
{
    GtkListStore* store = gtk_list_store_new(NUM_COLUMNS, G_TYPE_INT64,
        G_TYPE_BOOLEAN, G_TYPE_STRING, G_TYPE_BOOLEAN, G_TYPE_BOOLEAN);

    GtkTreeIter iter;
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter,
        COLUMN_ROWID, 0,
        COLUMN_CHECKED, FALSE,
        COLUMN_TITLE, "New task...",
        COLUMN_CHECKBOX_VISIBLE, FALSE,
        COLUMN_EDITABLE, TRUE,
        -1);

    gtk_tree_view_set_model(GTK_TREE_VIEW(self), GTK_TREE_MODEL(store));
    g_object_unref(store);

    GtkTreeViewColumn* checked_column;
    checked_column = gtk_tree_view_column_new_with_attributes(
        "", gtk_cell_renderer_toggle_new(),
        "active", COLUMN_CHECKED,
        "visible", COLUMN_CHECKBOX_VISIBLE, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(self), checked_column);

    GtkCellRenderer* title_renderer = gtk_cell_renderer_text_new();
    g_signal_connect(title_renderer, "edited",
        G_CALLBACK(oct_task_view_new_entry_edited), self);
    GtkTreeViewColumn* title_column;
    title_column = gtk_tree_view_column_new_with_attributes(
        "", title_renderer,
        "editable", COLUMN_EDITABLE,
        "text", COLUMN_TITLE, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(self), title_column);
}

static void
oct_task_view_new_entry_edited(GtkCellRendererText* renderer,
                               gchar*               path_string,
                               gchar*               new_text,
                               gpointer             user_data)
{
    GtkListStore* store = GTK_LIST_STORE(
        gtk_tree_view_get_model(GTK_TREE_VIEW(user_data)));
    gint num_tasks = gtk_tree_model_iter_n_children(
        GTK_TREE_MODEL(store), NULL);

    if (atoi(path_string) == num_tasks - 1) {
        GtkTreeIter iter;
        gtk_list_store_prepend(store, &iter);
        gtk_list_store_set(store, &iter,
            COLUMN_ROWID, num_tasks,
            COLUMN_CHECKED, FALSE,
            COLUMN_TITLE, new_text,
            COLUMN_CHECKBOX_VISIBLE, TRUE,
            COLUMN_EDITABLE, TRUE,
            -1);
    } else {
        GtkTreeModel* model = gtk_tree_view_get_model(GTK_TREE_VIEW(user_data));
        GtkTreeIter iter;
        gtk_tree_model_get_iter_from_string(model, &iter, path_string);
        gtk_list_store_set(GTK_LIST_STORE(model), &iter,
            COLUMN_TITLE, g_strdup(new_text), -1);
    }
}

GtkWidget*
oct_task_view_new(void)
{
    return g_object_new(OCT_TYPE_TASK_VIEW, NULL);
}
