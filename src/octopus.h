#ifndef __OCTOPUS_H__
#define __OCTOPUS_H__

#include <gtk/gtk.h>
#include <stdbool.h>
#include <sqlite3.h>


/** S Y N C I N G *************************************************************/
typedef struct {
    char* host;
    long port;
    char* username;
    char* password;
    char* token;
} OctSync;

void
oct_sync_auth(GTask*, gpointer, gpointer, GCancellable*);


/** T A S K S *****************************************************************/
typedef struct {
    sqlite3_int64 rowid;
    char* id;
    char* rev;
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
