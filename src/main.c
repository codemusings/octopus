#include <stdlib.h>
#include <string.h>

#include <sqlite3.h>

#include "octopus.h"
#include "oct-application.h"
#include "oct-resources.h"


static gboolean
bootstrap_db(sqlite3*);

static gboolean
bootstrap_db(sqlite3 *database)
{
    gboolean success = FALSE;
    GResource *resource = oct_get_resource();

    GError *res_error = NULL;
    GBytes *data = g_resource_lookup_data(resource,
        "/de/codemusings/Octopus/bootstrap.sql",
        G_RESOURCE_LOOKUP_FLAGS_NONE, &res_error);
    if (data == NULL) {
        fprintf(stderr,
            "Unable to load bootstrap SQL script resource: %s\n",
            res_error->message);
        goto exit;
    }

    char *script = (char*)g_bytes_get_data(data, NULL);
    if (script == NULL) {
        fprintf(stderr,
            "Unable to extract bootstrap SQL script from resource.\n");
        goto exit;
    }
    g_debug("%s\n", script);

    char *errmsg;
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

int main(int argc, char *argv[])
{
    const char *data_path = g_get_user_data_dir();
    if (access(data_path, F_OK) == -1) {
        fprintf(stderr, "Unable to fetch valid user data directory.\n");
        return EXIT_FAILURE;
    }

    const char *filename = "octopus.db";
    size_t length = strlen(data_path) + 1 + strlen(filename) + 1;
    char full_path[length];
    strcpy(full_path, data_path);
    strcat(full_path, "/");
    strcat(full_path, filename);
    g_debug("%s\n", full_path);

    gboolean file_existed_before = access(full_path, F_OK) != -1;
    sqlite3 *database;
    if (sqlite3_open(full_path, &database) != SQLITE_OK) {
        fprintf(stderr,
            "Unable to open database: %s\n", sqlite3_errmsg(database));
        return EXIT_FAILURE;
    }

    if (!file_existed_before && !bootstrap_db(database)) {
        sqlite3_close(database);
        return EXIT_FAILURE;
    }

    OctApplication *app = oct_application_new(
        OCT_SCHEMA, G_APPLICATION_FLAGS_NONE, database);
    int result = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return result;
}
