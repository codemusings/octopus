#include <string.h>
#include <stdlib.h>

#include "oct-task-view.h"

static void
oct_task_view_checkbox_toggled(GtkCellRendererToggle*, gchar*, gpointer);

static void
oct_task_view_title_edited(GtkCellRendererText*, gchar*, gchar*, gpointer);

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
    gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(self), FALSE);

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

    GtkCellRenderer* checked_renderer = gtk_cell_renderer_toggle_new();
    g_signal_connect(checked_renderer, "toggled",
        G_CALLBACK(oct_task_view_checkbox_toggled), self);
    GtkTreeViewColumn* checked_column;
    checked_column = gtk_tree_view_column_new_with_attributes(
        "", checked_renderer,
        "activatable", COLUMN_EDITABLE,
        "active", COLUMN_CHECKED,
        "visible", COLUMN_CHECKBOX_VISIBLE, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(self), checked_column);

    GtkCellRenderer* title_renderer = gtk_cell_renderer_text_new();
    g_signal_connect(title_renderer, "edited",
        G_CALLBACK(oct_task_view_title_edited), self);
    GtkTreeViewColumn* title_column;
    title_column = gtk_tree_view_column_new_with_attributes(
        "", title_renderer,
        "editable", COLUMN_EDITABLE,
        "text", COLUMN_TITLE, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(self), title_column);
}

static void
oct_task_view_title_edited(GtkCellRendererText* renderer,
                           gchar*               path_string,
                           gchar*               new_text,
                           gpointer             user_data)
{
    GtkTreeModel* model = gtk_tree_view_get_model(GTK_TREE_VIEW(user_data));
    GtkTreeIter iter, new_iter;
    gtk_tree_model_get_iter_from_string(model, &iter, path_string);
    gint num_tasks = gtk_tree_model_iter_n_children(model, NULL);

    if (atoi(path_string) == num_tasks - 1) {
        gtk_list_store_insert_before(GTK_LIST_STORE(model), &new_iter, &iter);
        gtk_list_store_set(GTK_LIST_STORE(model), &new_iter,
            COLUMN_ROWID, num_tasks,
            COLUMN_CHECKED, FALSE,
            COLUMN_TITLE, new_text,
            COLUMN_CHECKBOX_VISIBLE, TRUE,
            COLUMN_EDITABLE, TRUE,
            -1);
    } else {
        gtk_list_store_set(GTK_LIST_STORE(model), &iter,
            COLUMN_TITLE, g_strdup(new_text), -1);
    }
}

static void
oct_task_view_checkbox_toggled(GtkCellRendererToggle* renderer,
                               gchar*                 path_string,
                               gpointer               user_data)
{
    GtkTreeModel* model = gtk_tree_view_get_model(GTK_TREE_VIEW(user_data));
    GtkTreeIter iter;
    gtk_tree_model_get_iter_from_string(model, &iter, path_string);

    gboolean checked;
    gtk_tree_model_get(model, &iter, COLUMN_CHECKED, &checked, -1);

    gtk_list_store_set(GTK_LIST_STORE(model), &iter,
        COLUMN_CHECKED, !checked, -1);
}

GtkWidget*
oct_task_view_new(void)
{
    return g_object_new(OCT_TYPE_TASK_VIEW, NULL);
}
