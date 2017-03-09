#define _XOPEN_SOURCE 500
#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <string.h>

#include "octopus.h"

sqlite3_int64
oct_task_add(sqlite3* db, const char* description)
{
    const char* query = "INSERT INTO tasks(description) VALUES(?)";
    sqlite3_stmt* statement;
    sqlite3_prepare_v2(db, query, -1, &statement, NULL);
    sqlite3_bind_text(statement, 1, description, -1, SQLITE_STATIC);
    sqlite3_step(statement);
    sqlite3_int64 rowid = sqlite3_last_insert_rowid(db);
    sqlite3_finalize(statement);
    return rowid;
}

void
oct_task_delete(sqlite3* db, sqlite3_int64 rowid)
{
    const char* query = "DELETE FROM tasks WHERE rowid = ?";
    sqlite3_stmt* statement;
    sqlite3_prepare_v2(db, query, -1, &statement, NULL);
    sqlite3_bind_int64(statement, 1, rowid);
    sqlite3_step(statement);
    sqlite3_finalize(statement);
}

OctTask**
oct_task_fetch_all(sqlite3* db)
{
    const char* query = "SELECT rowid, description, completed FROM tasks";
    sqlite3_stmt* statement;
    sqlite3_prepare_v2(db, query, -1, &statement, NULL);

    OctTask** tasks = malloc(sizeof(*tasks));
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
oct_task_update(sqlite3*      db,
                sqlite3_int64 rowid,
                const char*   description,
                bool          completed)
{
    const char* query = "UPDATE tasks SET description = ?, completed = ? "
                  "WHERE rowid = ?";
    sqlite3_stmt* statement;
    sqlite3_prepare_v2(db, query, -1, &statement, NULL);
    sqlite3_bind_text(statement, 1, description, -1, SQLITE_STATIC);
    sqlite3_bind_int(statement, 2, completed);
    sqlite3_bind_int64(statement, 3, rowid);
    sqlite3_step(statement);
    sqlite3_finalize(statement);
}
