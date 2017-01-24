#include <gtk/gtk.h>

#include "oct-task-view.h"

static void
oct_application_activate(GApplication* app, gpointer user_data)
{
    GtkWidget* task_view = oct_task_view_new();

    GtkWidget* window = gtk_application_window_new(GTK_APPLICATION(app));
    gtk_window_set_default_size(GTK_WINDOW(window), 300, 400);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_title(GTK_WINDOW(window), "Octopus");
    gtk_container_add(GTK_CONTAINER(window), task_view);

    gtk_widget_show_all(window);
}

int main(int argc, char** argv)
{
    GtkApplication* app = gtk_application_new(
        "de.codemusings.Octopus", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate",
        G_CALLBACK(oct_application_activate), NULL);
    int result = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return result;
}
