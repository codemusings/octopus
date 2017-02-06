#include <stdlib.h>
#include <string.h>

#include <gtk/gtk.h>
#include <sqlite3.h>

#include "oct-resources.h"
#include "oct-settings-popover.h"
#include "oct-task-view.h"

#define OCT_SCHEMA "de.codemusings.Octopus"

static GSettings* settings = NULL;
static OctSync* sync = NULL;

static void
oct_application_activate(GApplication*, sqlite3*);

static gboolean
oct_application_bootstrap_db(sqlite3*);

static void
oct_application_shutdown(GApplication*, sqlite3*);

static void
oct_application_activate(GApplication* app, sqlite3* database)
{
    settings = g_settings_new(OCT_SCHEMA);
    sync = malloc(sizeof(*sync));
    sync->host = g_strdup(g_settings_get_string(settings, "sync-host"));
    sync->port = g_settings_get_int(settings, "sync-port");
    sync->username = g_strdup(g_settings_get_string(settings, "sync-username"));
    sync->password = g_strdup(g_settings_get_string(settings, "sync-password"));

    GtkWidget* settings_icon = gtk_image_new_from_icon_name(
        "emblem-system-symbolic", GTK_ICON_SIZE_BUTTON);
    GtkWidget* settings_button = gtk_menu_button_new();
    gtk_button_set_image(GTK_BUTTON(settings_button), settings_icon);

    GtkWidget* settings_popover;
    settings_popover = oct_settings_popover_new(settings_button, sync);
    gtk_menu_button_set_popover(
        GTK_MENU_BUTTON(settings_button), settings_popover);

    GtkWidget* header_bar = gtk_header_bar_new();
    gtk_header_bar_set_show_close_button(GTK_HEADER_BAR(header_bar), TRUE);
    gtk_header_bar_set_title(GTK_HEADER_BAR(header_bar), "Octopus");
    gtk_header_bar_set_subtitle(GTK_HEADER_BAR(header_bar),
        "Sync is not enabled");
    gtk_header_bar_pack_start(GTK_HEADER_BAR(header_bar), settings_button);

    GtkWidget* task_view = oct_task_view_new(database);

    GtkWidget* window = gtk_application_window_new(GTK_APPLICATION(app));
    gtk_window_set_default_size(GTK_WINDOW(window), 300, 400);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_titlebar(GTK_WINDOW(window), header_bar);
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
        "/de/codemusings/Octopus/bootstrap.sql",
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
    g_settings_set_string(settings, "sync-host", sync->host);
    g_settings_set_int(settings, "sync-port", sync->port);
    g_settings_set_string(settings, "sync-username", sync->username);
    g_settings_set_string(settings, "sync-password", sync->password);
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
        OCT_SCHEMA, G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate",
        G_CALLBACK(oct_application_activate), database);
    g_signal_connect(app, "shutdown",
        G_CALLBACK(oct_application_shutdown), database);
    int result = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return result;
}
