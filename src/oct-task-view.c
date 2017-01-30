#include <string.h>
#include <stdlib.h>

#include <sqlite3.h>

#include "octopus.h"
#include "oct-cell-renderer-button.h"
#include "oct-task-view.h"

#define NEW_TASK_PLACEHOLDER "New task..."

static void
oct_task_view_checkbox_toggled(GtkCellRendererToggle*, gchar*, OctTaskView*);

static void
oct_task_view_constructed(GObject*);

static void
oct_task_view_delete_button_clicked(OctCellRendererButton*,
                                    gchar*,
                                    OctTaskView*);
static void
oct_task_view_set_property(GObject*, guint, const GValue*, GParamSpec*);

static void
oct_task_view_title_edited(GtkCellRendererText*, gchar*, gchar*, OctTaskView*);

struct _OctTaskView {
    GtkTreeView parent_instance;
    sqlite3*    database;
};
G_DEFINE_TYPE(OctTaskView, oct_task_view, GTK_TYPE_TREE_VIEW)

enum {
    PROP_DATABASE = 1,
    NUM_PROPS
};
static GParamSpec* properties[NUM_PROPS];

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
oct_task_view_checkbox_toggled(GtkCellRendererToggle* renderer,
                               gchar*                 path_string,
                               OctTaskView*           self)
{
    GtkTreeModel* model = gtk_tree_view_get_model(GTK_TREE_VIEW(self));
    GtkTreeIter iter;
    gtk_tree_model_get_iter_from_string(model, &iter, path_string);

    sqlite3_int64 rowid;
    char* description;
    gboolean checked;
    gtk_tree_model_get(model, &iter,
        COLUMN_ROWID, &rowid,
        COLUMN_TITLE, &description,
        COLUMN_CHECKED, &checked,
        -1);

    oct_task_update(self->database, rowid, description, !checked);

    gtk_list_store_set(GTK_LIST_STORE(model), &iter,
        COLUMN_CHECKED, !checked,
        COLUMN_TITLE_COLOR, checked ? "#000000" : "#aaaaaa",
        COLUMN_TITLE_EDITABLE, checked,
        -1);
}

static void
oct_task_view_constructed(GObject* object)
{
    OctTaskView* self = OCT_TASK_VIEW(object);
    gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(self), FALSE);

    GtkListStore* store = gtk_list_store_new(NUM_COLUMNS,
        G_TYPE_INT64,   /* COLUMN_ROWID */
        G_TYPE_BOOLEAN, /* COLUMN_CHECKED */
        G_TYPE_BOOLEAN, /* COLUMN_CONTROLS_VISIBLE */
        G_TYPE_BOOLEAN, /* COLUMN_CHECKBOX_EDITABLE */
        G_TYPE_STRING,  /* COLUMN_TITLE */
        G_TYPE_BOOLEAN, /* COLUMN_TITLE_EDITABLE */
        G_TYPE_STRING   /* COLUMN_TITLE_COLOR */
    );

    GtkTreeIter iter;
    OctTask** tasks = oct_task_fetch_all(self->database);
    int i = 0;
    for (OctTask* task = tasks[i]; task != NULL; task = tasks[++i]) {
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter,
            COLUMN_ROWID, task->rowid,
            COLUMN_CHECKED, task->completed,
            COLUMN_CONTROLS_VISIBLE, TRUE,
            COLUMN_CHECKBOX_EDITABLE, TRUE,
            COLUMN_TITLE, task->description,
            COLUMN_TITLE_EDITABLE, TRUE,
            COLUMN_TITLE_COLOR, "#000000",
            -1);
        free(task);
    }
    free(tasks);

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

    gtk_tree_view_set_model(GTK_TREE_VIEW(self), GTK_TREE_MODEL(store));
    g_object_unref(store);

    G_OBJECT_CLASS(oct_task_view_parent_class)->constructed(object);
}

static void
oct_task_view_class_init(OctTaskViewClass* class)
{
    GObjectClass* object_class = G_OBJECT_CLASS(class);
    object_class->constructed = oct_task_view_constructed;
    object_class->set_property = oct_task_view_set_property;

    properties[PROP_DATABASE] = g_param_spec_pointer("database",
        "database", "database", G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY);
    g_object_class_install_properties(object_class, NUM_PROPS, properties);
}

static void
oct_task_view_delete_button_clicked(OctCellRendererButton* renderer,
                                    gchar*                 path_string,
                                    OctTaskView*           self)
{
    GtkTreeModel* model = gtk_tree_view_get_model(GTK_TREE_VIEW(self));
    GtkTreeIter iter;
    gtk_tree_model_get_iter_from_string(model, &iter, path_string);

    sqlite3_int64 rowid;
    gtk_tree_model_get(model, &iter, COLUMN_ROWID, &rowid, -1);
    oct_task_delete(self->database, rowid);

    gtk_list_store_remove(GTK_LIST_STORE(model), &iter);
}

static void
oct_task_view_init(OctTaskView* self)
{

    GtkCellRenderer* delete_renderer = oct_cell_renderer_button_new();
    g_signal_connect(delete_renderer, "clicked",
        G_CALLBACK(oct_task_view_delete_button_clicked), self);
    GtkTreeViewColumn* delete_column;
    delete_column = gtk_tree_view_column_new_with_attributes(
        "", delete_renderer, "visible", COLUMN_CONTROLS_VISIBLE, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(self), delete_column);

    GtkCellRenderer* checked_renderer = gtk_cell_renderer_toggle_new();
    g_signal_connect(checked_renderer, "toggled",
        G_CALLBACK(oct_task_view_checkbox_toggled), self);
    GtkTreeViewColumn* checked_column;
    checked_column = gtk_tree_view_column_new_with_attributes(
        "", checked_renderer,
        "activatable", COLUMN_CHECKBOX_EDITABLE,
        "active", COLUMN_CHECKED,
        "visible", COLUMN_CONTROLS_VISIBLE, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(self), checked_column);

    GtkCellRenderer* title_renderer = gtk_cell_renderer_text_new();
    g_signal_connect(title_renderer, "edited",
        G_CALLBACK(oct_task_view_title_edited), self);
    GtkTreeViewColumn* title_column;
    title_column = gtk_tree_view_column_new_with_attributes(
        "", title_renderer,
        "editable", COLUMN_TITLE_EDITABLE,
        "foreground", COLUMN_TITLE_COLOR,
        "strikethrough", COLUMN_CHECKED,
        "text", COLUMN_TITLE, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(self), title_column);
}

static void
oct_task_view_set_property(GObject*      object,
                           guint         property_id,
                           const GValue* value,
                           GParamSpec*   spec)
{
    OctTaskView* self = OCT_TASK_VIEW(object);
    switch (property_id) {
        case PROP_DATABASE:
            self->database = g_value_get_pointer(value);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, spec);
    }
}

static void
oct_task_view_title_edited(GtkCellRendererText* renderer,
                           gchar*               path_string,
                           gchar*               new_text,
                           OctTaskView*         self)
{
    GtkTreeModel* model = gtk_tree_view_get_model(GTK_TREE_VIEW(self));
    GtkTreeIter iter, new_iter;
    gtk_tree_model_get_iter_from_string(model, &iter, path_string);
    gint num_tasks = gtk_tree_model_iter_n_children(model, NULL);

    if (strncmp(NEW_TASK_PLACEHOLDER, new_text, strlen(new_text)) == 0) {
        return;
    }

    if (atoi(path_string) == num_tasks - 1) {
        sqlite3_int64 rowid = oct_task_add(self->database, new_text);
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
        oct_task_update(self->database, rowid, new_text, checked);
        gtk_list_store_set(GTK_LIST_STORE(model), &iter,
            COLUMN_TITLE, new_text, -1);
    }
}

GtkWidget*
oct_task_view_new(sqlite3* database)
{
    return g_object_new(OCT_TYPE_TASK_VIEW, "database", database, NULL);
}
