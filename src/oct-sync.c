#include <curl/curl.h>
#include <json-glib/json-glib.h>

#include <stdio.h>

#include "octopus.h"

static size_t
oct_sync_auth_callback(char*, size_t, size_t, void*);

void
oct_sync_auth(GTask*        task,
              gpointer      source_object,
              gpointer      task_data,
              GCancellable* cancellable)
{
    OctSync* sync = (OctSync*)task_data;

    JsonBuilder* builder = json_builder_new();
    json_builder_begin_object(builder);
    json_builder_set_member_name(builder, "username");
    json_builder_add_string_value(builder, sync->username);
    json_builder_set_member_name(builder, "password");
    json_builder_add_string_value(builder, sync->password);
    json_builder_end_object(builder);

    char* json = json_to_string(json_builder_get_root(builder), false);
    printf("%s\n", json);

    curl_global_init(CURL_GLOBAL_DEFAULT);
    CURL* curl = curl_easy_init();
    CURLcode result = CURLE_OK;
    struct curl_slist* header_list = NULL;

    if (curl) {

        curl_easy_setopt(curl, CURLOPT_URL, sync->host);
        curl_easy_setopt(curl, CURLOPT_PORT, sync->port);
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, task);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, oct_sync_auth_callback);
        result = curl_easy_setopt(curl, CURLOPT_CAPATH, ".");
        if (result != CURLE_OK) {
            fprintf(stderr, "Unable to find or load certificate file: %s\n",
                curl_easy_strerror(result));
            goto cleanup;
        }

        header_list = curl_slist_append(header_list,
            "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header_list);

        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json);
        result = curl_easy_perform(curl);
        if (result != CURLE_OK) {
            fprintf(stderr,
                "Pushing data failed: %s\n", curl_easy_strerror(result));
        }
    }

cleanup:
    if (header_list) {
        curl_slist_free_all(header_list);
    }
    if (curl) {
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();
    if (result != CURLE_OK) {
        g_task_return_boolean(task, FALSE);
    }
}

static size_t
oct_sync_auth_callback(char* data, size_t size, size_t num, void* user_data)
{
    GTask* task = G_TASK(user_data);
    g_task_return_boolean(task, TRUE);
    return num * size;
}
