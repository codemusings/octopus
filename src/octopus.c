#define _XOPEN_SOURCE 500
#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <string.h>

#include "octopus.h"

sqlite3_int64
oct_task_add(sqlite3 *db, const char *description, sqlite3_int64 topic_id)
{
    const char *query = "INSERT INTO tasks(description, topic) VALUES(?, ?)";
    sqlite3_stmt *statement;
    sqlite3_prepare_v2(db, query, -1, &statement, NULL);
    sqlite3_bind_text(statement, 1, description, -1, SQLITE_STATIC);
    sqlite3_bind_int64(statement, 2, topic_id);
    sqlite3_step(statement);
    sqlite3_int64 rowid = sqlite3_last_insert_rowid(db);
    sqlite3_finalize(statement);
    return rowid;
}

void
oct_task_delete(sqlite3 *db, sqlite3_int64 rowid)
{
    const char *query = "DELETE FROM tasks WHERE rowid = ?";
    sqlite3_stmt *statement;
    sqlite3_prepare_v2(db, query, -1, &statement, NULL);
    sqlite3_bind_int64(statement, 1, rowid);
    sqlite3_step(statement);
    sqlite3_finalize(statement);
}

void
oct_task_delete_by_topic(sqlite3 *db, sqlite3_int64 topic_id)
{
    const char *query = "DELETE FROM tasks WHERE topic = ?";
    sqlite3_stmt *statement;
    sqlite3_prepare_v2(db, query, -1, &statement, NULL);
    sqlite3_bind_int64(statement, 1, topic_id);
    sqlite3_step(statement);
    sqlite3_finalize(statement);
}

OctTask**
oct_task_fetch_by_topic(sqlite3 *db, sqlite3_int64 topic_id)
{
    const char *query =
        "SELECT rowid, description, completed "
          "FROM tasks "
         "WHERE topic = ?";
    sqlite3_stmt *statement;
    sqlite3_prepare_v2(db, query, -1, &statement, NULL);
    sqlite3_bind_int64(statement, 1, topic_id);

    OctTask **tasks = malloc(sizeof(*tasks));
    tasks[0] = NULL;
    sqlite3_int64 count = 0;

    while (sqlite3_step(statement) == SQLITE_ROW) {
        tasks = realloc(tasks, (count + 2) * sizeof(*tasks));
        tasks[count] = malloc(sizeof(*tasks[count]));
        tasks[count]->rowid = sqlite3_column_int64(statement, 0);
        tasks[count]->description = strdup(
            (const char*)sqlite3_column_text(statement, 1));
        tasks[count]->completed = sqlite3_column_int(statement, 2);
        tasks[count + 1] = NULL;
        count++;
    }
    sqlite3_finalize(statement);
    return tasks;
}

void
oct_task_list_free(OctTask** tasks)
{
    OctTask** task;
    for (task = tasks; *task != NULL; task++) {
        free((*task)->description);
        free(*task);
    }
    free(tasks);
}

void
oct_task_update(sqlite3       *db,
                sqlite3_int64  rowid,
                const char    *description,
                bool           completed)
{
    const char *query =
        "UPDATE tasks "
           "SET description = ?, completed = ? "
         "WHERE rowid = ?";
    sqlite3_stmt *statement;
    sqlite3_prepare_v2(db, query, -1, &statement, NULL);
    sqlite3_bind_text(statement, 1, description, -1, SQLITE_STATIC);
    sqlite3_bind_int(statement, 2, completed);
    sqlite3_bind_int64(statement, 3, rowid);
    sqlite3_step(statement);
    sqlite3_finalize(statement);
}

sqlite3_int64
oct_topic_add(sqlite3 *db, const char *title)
{
    const char *query = "INSERT INTO topics(title) VALUES(?)";
    sqlite3_stmt *statement;
    sqlite3_prepare_v2(db, query, -1, &statement, NULL);
    sqlite3_bind_text(statement, 1, title, -1, SQLITE_STATIC);
    sqlite3_step(statement);
    sqlite3_int64 rowid = sqlite3_last_insert_rowid(db);
    sqlite3_finalize(statement);
    return rowid;
}

void
oct_topic_delete(sqlite3 *db, sqlite3_int64 rowid)
{
    const char *query = "DELETE FROM topics WHERE rowid = ?";
    sqlite3_stmt *statement;
    sqlite3_prepare_v2(db, query, -1, &statement, NULL);
    sqlite3_bind_int64(statement, 1, rowid);
    sqlite3_step(statement);
    sqlite3_finalize(statement);
}

bool
oct_topic_exists(sqlite3 *db, const char *title)
{
    const char *query = "SELECT rowid FROM topics WHERE title = ?";
    sqlite3_stmt *statement;
    sqlite3_prepare_v2(db, query, -1, &statement, NULL);
    sqlite3_bind_text(statement, 1, title, -1, SQLITE_STATIC);
    bool exists = sqlite3_step(statement) == SQLITE_ROW;
    sqlite3_finalize(statement);
    return exists;
}

OctTopic**
oct_topic_fetch_all(sqlite3 *db)
{
    const char *query = "SELECT rowid, title FROM topics";
    sqlite3_stmt *statement;
    sqlite3_prepare_v2(db, query, -1, &statement, NULL);

    OctTopic **topics = malloc(sizeof(*topics));
    topics[0] = NULL;
    sqlite3_int64 count = 0;

    while (sqlite3_step(statement) == SQLITE_ROW) {
        topics = realloc(topics, (count + 2) * sizeof(*topics));
        topics[count] = malloc(sizeof(*topics[count]));
        topics[count]->rowid = sqlite3_column_int64(statement, 0);
        topics[count]->title = strdup(
            (const char*)sqlite3_column_text(statement, 1));
        topics[count + 1] = NULL;
        count++;
    }
    sqlite3_finalize(statement);
    return topics;
}

void
oct_topic_list_free(OctTopic **topics)
{
    int i = 0;
    for (OctTopic *topic = topics[i]; topic != NULL; topic = topics[++i]) {
        free(topic->title);
    }
    free(topics);
}

void
oct_topic_update(sqlite3       *db,
                 sqlite3_int64  rowid,
                 const char    *title)
{
    const char *query = "UPDATE topics SET title = ? WHERE rowid = ?";
    sqlite3_stmt *statement;
    sqlite3_prepare_v2(db, query, -1, &statement, NULL);
    sqlite3_bind_text(statement, 1, title, -1, SQLITE_STATIC);
    sqlite3_bind_int64(statement, 2, rowid);
    sqlite3_step(statement);
    sqlite3_finalize(statement);
}
