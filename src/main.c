#include <stdlib.h>
#include <string.h>

#include <gtk/gtk.h>
#include <sqlite3.h>

#include "oct-resources.h"
#include "oct-task-view.h"

static void
oct_application_activate(GApplication*, sqlite3*);

static gboolean
oct_application_bootstrap_db(sqlite3*);

static void
oct_application_shutdown(GApplication*, sqlite3*);

static void
oct_application_activate(GApplication* app, sqlite3* database)
{
    GtkWidget* task_view = oct_task_view_new(database);

    GtkWidget* window = gtk_application_window_new(GTK_APPLICATION(app));
    gtk_window_set_default_size(GTK_WINDOW(window), 300, 400);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_title(GTK_WINDOW(window), "Octopus");
    gtk_container_add(GTK_CONTAINER(window), task_view);

    gtk_widget_show_all(window);
}

static gboolean
oct_application_bootstrap_db(sqlite3* database)
{
    gboolean success = FALSE;
    GResource* resource = oct_get_resource();

    GError* res_error = NULL;
    GBytes* data = g_resource_lookup_data(resource,
        "/de/codemusings/Octopus/res/bootstrap.sql",
        G_RESOURCE_LOOKUP_FLAGS_NONE, &res_error);
    if (data == NULL) {
        fprintf(stderr,
            "Unable to load bootstrap SQL script resource: %s\n",
            res_error->message);
        goto exit;
    }

    char* script = (char*)g_bytes_get_data(data, NULL);
    if (script == NULL) {
        fprintf(stderr,
            "Unable to extract bootstrap SQL script from resource.\n");
        goto exit;
    }
    g_debug("%s\n", script);

    char* errmsg;
    if (sqlite3_exec(database, script, NULL, NULL, &errmsg) == SQLITE_OK) {
        success = TRUE;
    } else {
        fprintf(stderr, "Unable to initialize database: %s\n", errmsg);
        fprintf(stderr, "%s\n", script);
    }

exit:
    if (res_error != NULL) {
        g_error_free(res_error);
    }
    if (data != NULL) {
        g_bytes_unref(data);
    }

    return success;
}

static void
oct_application_shutdown(GApplication* app, sqlite3* database)
{
    sqlite3_close(database);
}

int main(int argc, char** argv)
{
    const char* data_path = g_get_user_data_dir();
    if (access(data_path, F_OK) == -1) {
        fprintf(stderr, "Unable to fetch valid user data directory.\n");
        return EXIT_FAILURE;
    }

    const char* filename = "octopus.db";
    size_t length = strlen(data_path) + 1 + strlen(filename) + 1;
    char full_path[length];
    strcpy(full_path, data_path);
    strcat(full_path, "/");
    strcat(full_path, filename);
    g_debug("%s\n", full_path);

    gboolean file_existed_before = access(full_path, F_OK) != -1;
    sqlite3* database;
    if (sqlite3_open(full_path, &database) != SQLITE_OK) {
        fprintf(stderr,
            "Unable to open database: %s\n", sqlite3_errmsg(database));
        return EXIT_FAILURE;
    }

    if (!file_existed_before && !oct_application_bootstrap_db(database)) {
        sqlite3_close(database);
        return EXIT_FAILURE;
    }

    GtkApplication* app = gtk_application_new(
        "de.codemusings.Octopus", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate",
        G_CALLBACK(oct_application_activate), database);
    g_signal_connect(app, "shutdown",
        G_CALLBACK(oct_application_shutdown), database);
    int result = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return result;
}
