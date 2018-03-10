#include <string.h>
#include <stdlib.h>

#include <sqlite3.h>

#include "octopus.h"
#include "oct-application.h"
#include "oct-cell-renderer-button.h"
#include "oct-task-view.h"

#define NEW_TASK_PLACEHOLDER "New task..."


struct _OctTaskView {
    GtkTreeView   parent_instance;
    sqlite3_int64 selected_topic_id;
};
G_DEFINE_TYPE(OctTaskView, oct_task_view, GTK_TYPE_TREE_VIEW)

enum {
    COLUMN_ROWID,
    COLUMN_CHECKED,
    COLUMN_CONTROLS_VISIBLE,
    COLUMN_CHECKBOX_EDITABLE,
    COLUMN_TITLE,
    COLUMN_TITLE_EDITABLE,
    COLUMN_TITLE_COLOR,
    NUM_COLUMNS
};


static void
oct_task_view_checkbox_toggled(GtkCellRendererToggle*, gchar*, OctTaskView*);

static void
oct_task_view_delete_button_clicked(OctCellRendererButton*,
                                    gchar*,
                                    OctTaskView*);
static void
oct_task_view_title_edited(GtkCellRendererText*, gchar*, gchar*, OctTaskView*);

static void
oct_task_view_update(OctTaskView*);

static void
on_topic_selected(OctApplication*, sqlite3_int64, OctTaskView*);


static void
oct_task_view_checkbox_toggled(GtkCellRendererToggle *renderer,
                               gchar                 *path_string,
                               OctTaskView           *self)
{
    GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(self));
    GtkTreeIter iter;
    gtk_tree_model_get_iter_from_string(model, &iter, path_string);

    sqlite3_int64 rowid;
    char *description;
    gboolean checked;
    gtk_tree_model_get(model, &iter,
        COLUMN_ROWID, &rowid,
        COLUMN_TITLE, &description,
        COLUMN_CHECKED, &checked,
        -1);

    oct_task_update(
        oct_application_get_database(), rowid, description, !checked);

    gtk_list_store_set(GTK_LIST_STORE(model), &iter,
        COLUMN_CHECKED, !checked,
        COLUMN_TITLE_COLOR, checked ? "#000000" : "#aaaaaa",
        COLUMN_TITLE_EDITABLE, checked,
        -1);
}

static void
oct_task_view_class_init(OctTaskViewClass *class)
{
}

static void
oct_task_view_delete_button_clicked(OctCellRendererButton *renderer,
                                    gchar                 *path_string,
                                    OctTaskView           *self)
{
    GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(self));
    GtkTreeIter iter;
    gtk_tree_model_get_iter_from_string(model, &iter, path_string);

    sqlite3_int64 rowid;
    gtk_tree_model_get(model, &iter, COLUMN_ROWID, &rowid, -1);
    oct_task_delete(oct_application_get_database(), rowid);

    gtk_list_store_remove(GTK_LIST_STORE(model), &iter);
}

static void
oct_task_view_init(OctTaskView *self)
{
    GtkCellRenderer *checked_renderer;
    GtkStyleContext *context;
    GtkCellRenderer *delete_renderer;
    GtkCssProvider *provider;
    GtkListStore *store;
    GtkCellRenderer *title_renderer;

    /* task view table */
    gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(self), FALSE);
    provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider,
        "treeview { background: #fffae1; }"
        "treeview:selected { background: #4a90d9; }",
    -1, NULL);
    context = gtk_widget_get_style_context(GTK_WIDGET(self));
    gtk_style_context_add_provider(context,
        GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    store = gtk_list_store_new(NUM_COLUMNS,
        G_TYPE_INT64,   /* COLUMN_ROWID */
        G_TYPE_BOOLEAN, /* COLUMN_CHECKED */
        G_TYPE_BOOLEAN, /* COLUMN_CONTROLS_VISIBLE */
        G_TYPE_BOOLEAN, /* COLUMN_CHECKBOX_EDITABLE */
        G_TYPE_STRING,  /* COLUMN_TITLE */
        G_TYPE_BOOLEAN, /* COLUMN_TITLE_EDITABLE */
        G_TYPE_STRING   /* COLUMN_TITLE_COLOR */
    );
    gtk_tree_view_set_model(GTK_TREE_VIEW(self), GTK_TREE_MODEL(store));
    g_object_unref(store);
    self->selected_topic_id = 0;
    oct_task_view_update(self);

    delete_renderer = oct_cell_renderer_button_new();
    g_signal_connect(delete_renderer, "clicked",
        G_CALLBACK(oct_task_view_delete_button_clicked), self);
    GtkTreeViewColumn *delete_column;
    delete_column = gtk_tree_view_column_new_with_attributes(
        "", delete_renderer, "visible", COLUMN_CONTROLS_VISIBLE, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(self), delete_column);

    checked_renderer = gtk_cell_renderer_toggle_new();
    g_signal_connect(checked_renderer, "toggled",
        G_CALLBACK(oct_task_view_checkbox_toggled), self);
    GtkTreeViewColumn *checked_column;
    checked_column = gtk_tree_view_column_new_with_attributes(
        "", checked_renderer,
        "activatable", COLUMN_CHECKBOX_EDITABLE,
        "active", COLUMN_CHECKED,
        "visible", COLUMN_CONTROLS_VISIBLE, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(self), checked_column);

    title_renderer = gtk_cell_renderer_text_new();
    g_signal_connect(title_renderer, "edited",
        G_CALLBACK(oct_task_view_title_edited), self);
    GtkTreeViewColumn *title_column;
    title_column = gtk_tree_view_column_new_with_attributes(
        "", title_renderer,
        "editable", COLUMN_TITLE_EDITABLE,
        "foreground", COLUMN_TITLE_COLOR,
        "strikethrough", COLUMN_CHECKED,
        "text", COLUMN_TITLE, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(self), title_column);

    gtk_widget_show_all(GTK_WIDGET(self));

    g_signal_connect(oct_application_shared(), "topic-selected",
        G_CALLBACK(on_topic_selected), self);
}

static void
oct_task_view_title_edited(GtkCellRendererText *renderer,
                           gchar               *path_string,
                           gchar               *new_text,
                           OctTaskView         *self)
{
    GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(self));
    GtkTreeIter iter, new_iter;
    gtk_tree_model_get_iter_from_string(model, &iter, path_string);
    gint num_tasks = gtk_tree_model_iter_n_children(model, NULL);

    if (strncmp(NEW_TASK_PLACEHOLDER, new_text, strlen(new_text)) == 0) {
        return;
    }

    if (atoi(path_string) == num_tasks - 1) {
        sqlite3_int64 rowid = oct_task_add(
            oct_application_get_database(), new_text, self->selected_topic_id);
        gtk_list_store_insert_before(GTK_LIST_STORE(model), &new_iter, &iter);
        gtk_list_store_set(GTK_LIST_STORE(model), &new_iter,
            COLUMN_ROWID, rowid,
            COLUMN_CHECKED, FALSE,
            COLUMN_CONTROLS_VISIBLE, TRUE,
            COLUMN_CHECKBOX_EDITABLE, TRUE,
            COLUMN_TITLE, new_text,
            COLUMN_TITLE_EDITABLE, TRUE,
            COLUMN_TITLE_COLOR, "#000000",
            -1);
    } else {
        sqlite3_int64 rowid;
        gboolean checked;
        gtk_tree_model_get(model, &iter,
            COLUMN_ROWID, &rowid,
            COLUMN_CHECKED, &checked,
            -1);
        oct_task_update(
            oct_application_get_database(), rowid, new_text, checked);
        gtk_list_store_set(GTK_LIST_STORE(model), &iter,
            COLUMN_TITLE, new_text, -1);
    }
}

static void
oct_task_view_update(OctTaskView *self)
{
    GtkTreeIter iter;
    GtkListStore *store;
    OctTask **task;
    OctTask **tasks;

    store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(self)));
    gtk_list_store_clear(store);

    tasks = oct_task_fetch_by_topic(
        oct_application_get_database(), self->selected_topic_id);
    for (task = tasks; *task != NULL; task++) {
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter,
            COLUMN_ROWID, (*task)->rowid,
            COLUMN_CHECKED, (*task)->completed,
            COLUMN_CONTROLS_VISIBLE, TRUE,
            COLUMN_CHECKBOX_EDITABLE, TRUE,
            COLUMN_TITLE, (*task)->description,
            COLUMN_TITLE_EDITABLE, TRUE,
            COLUMN_TITLE_COLOR, (*task)->completed ? "#aaaaaa" : "#000000",
            -1);
    }
    oct_task_list_free(tasks);

    /* new task entry */
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter,
        COLUMN_ROWID, 0,
        COLUMN_CHECKED, FALSE,
        COLUMN_CONTROLS_VISIBLE, FALSE,
        COLUMN_CHECKBOX_EDITABLE, TRUE,
        COLUMN_TITLE, NEW_TASK_PLACEHOLDER,
        COLUMN_TITLE_EDITABLE, TRUE,
        COLUMN_TITLE_COLOR, "#000000",
        -1);
}

static void
on_topic_selected(OctApplication *app,
                  sqlite3_int64   topic_id,
                  OctTaskView    *self)
{
    self->selected_topic_id = topic_id;
    oct_task_view_update(self);
}

GtkWidget*
oct_task_view_new(void)
{
    return g_object_new(OCT_TYPE_TASK_VIEW, NULL);
}
