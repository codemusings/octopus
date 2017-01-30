#ifndef __OCTOPUS_H__
#define __OCTOPUS_H__

#include <stdbool.h>
#include <sqlite3.h>

typedef struct {
    sqlite3_int64 rowid;
    char* description;
    bool completed;
} OctTask;

sqlite3_int64
oct_task_add(sqlite3*, const char*);

void
oct_task_delete(sqlite3*, sqlite3_int64);

OctTask**
oct_task_fetch_all(sqlite3*);

void
oct_task_update(sqlite3*, sqlite3_int64, const char*, bool);

#endif