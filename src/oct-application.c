#include <stdlib.h>
#include <string.h>

#include <gtk/gtk.h>
#include <sqlite3.h>

#include "oct-application.h"
#include "oct-resources.h"
#include "oct-task-view.h"
#include "oct-topic-view.h"

#define OCT_SCHEMA "de.codemusings.Octopus"


struct _OctApplication {
    GtkApplication  parent_instance;
    sqlite3        *database;
};
G_DEFINE_TYPE(OctApplication, oct_application, GTK_TYPE_APPLICATION)

enum {
    PROP_DATABASE = 1,
    NUM_PROPS
};
static GParamSpec* properties[NUM_PROPS];


static void
oct_application_get_property(GObject*, guint, GValue*, GParamSpec*);

static void
oct_application_set_property(GObject*, guint, const GValue*, GParamSpec*);

static void
on_activate(OctApplication*, gpointer);

static void
on_shutdown(OctApplication*, gpointer);


static void
oct_application_class_init(OctApplicationClass* class)
{
    GObjectClass *object_class = G_OBJECT_CLASS(class);
    object_class->get_property = oct_application_get_property;
    object_class->set_property = oct_application_set_property;

    properties[PROP_DATABASE] = g_param_spec_pointer(
        "database", "database", "database",
        G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY);
    g_object_class_install_properties(object_class, NUM_PROPS, properties);

    g_signal_new("topic-selected",
                 OCT_TYPE_APPLICATION,
                 G_SIGNAL_RUN_FIRST,
                 0,
                 NULL,
                 NULL,
                 NULL,
                 G_TYPE_NONE,
                 1,
                 G_TYPE_INT64);
}

static void
oct_application_init(OctApplication* self)
{
    g_signal_connect(self, "activate", G_CALLBACK(on_activate), NULL);
    g_signal_connect(self, "shutdown", G_CALLBACK(on_shutdown), NULL);
}

static void
oct_application_get_property(GObject    *object,
                             guint       property_id,
                             GValue     *value,
                             GParamSpec *pspec)
{
    OctApplication *self = OCT_APPLICATION(object);
    switch (property_id) {
        case PROP_DATABASE:
            g_value_set_pointer(value, self->database);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    }
}

static void
oct_application_set_property(GObject      *object,
                             guint         property_id,
                             const GValue *value,
                             GParamSpec   *pspec)
{
    OctApplication *self = OCT_APPLICATION(object);
    switch (property_id) {
        case PROP_DATABASE:
            self->database = g_value_get_pointer(value);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
    }
}

static void
on_activate(OctApplication *self, gpointer user_data)
{
    GtkWidget *header_bar = gtk_header_bar_new();
    gtk_header_bar_set_show_close_button(GTK_HEADER_BAR(header_bar), TRUE);
    //gtk_header_bar_set_title(GTK_HEADER_BAR(header_bar), "Octopus");
    gtk_widget_show(header_bar);

    GtkWidget *topic_view = oct_topic_view_new();
    //gtk_header_bar_set_custom_title(GTK_HEADER_BAR(header_bar), topic_view);
    gtk_header_bar_pack_start(GTK_HEADER_BAR(header_bar), topic_view);

    GtkWidget *task_view = oct_task_view_new();

    GtkWidget *window = gtk_application_window_new(GTK_APPLICATION(self));
    gtk_window_set_default_size(GTK_WINDOW(window), 300, 400);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_titlebar(GTK_WINDOW(window), header_bar);
    gtk_container_add(GTK_CONTAINER(window), task_view);

    gtk_widget_show(window);
}

static void
on_shutdown(OctApplication *self, gpointer user_data)
{
    sqlite3_close(self->database);
}

sqlite3*
oct_application_get_database(void)
{
    return oct_application_shared()->database;
}

OctApplication*
oct_application_new(const char        *application_id,
                    GApplicationFlags  flags,
                    sqlite3           *database)
{
    return g_object_new(OCT_TYPE_APPLICATION,
        "application-id", application_id,
        "flags", flags,
        "database", database,
        NULL);
}

OctApplication*
oct_application_shared(void)
{
    return OCT_APPLICATION(g_application_get_default());
}
