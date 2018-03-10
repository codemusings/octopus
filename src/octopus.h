#ifndef __OCTOPUS_H__
#define __OCTOPUS_H__

#include <gtk/gtk.h>
#include <stdbool.h>
#include <sqlite3.h>


#define OCT_SCHEMA "de.codemusings.Octopus"


/** T A S K S *****************************************************************/
typedef struct {
    sqlite3_int64 rowid;
    char *description;
    bool  completed;
} OctTask;

sqlite3_int64
oct_task_add(sqlite3*, const char*, sqlite3_int64);

void
oct_task_delete(sqlite3*, sqlite3_int64);

void
oct_task_delete_by_topic(sqlite3*, sqlite3_int64);

OctTask**
oct_task_fetch_by_topic(sqlite3*, sqlite3_int64);

void
oct_task_list_free(OctTask**);

void
oct_task_update(sqlite3*, sqlite3_int64, const char*, bool);


/** T O P I C S ***************************************************************/
typedef struct {
    sqlite3_int64  rowid;
    char          *title;
} OctTopic;

sqlite3_int64
oct_topic_add(sqlite3*, const char*);

void
oct_topic_delete(sqlite3*, sqlite3_int64);

bool
oct_topic_exists(sqlite3*, const char*);

OctTopic**
oct_topic_fetch_all(sqlite3*);

void
oct_topic_list_free(OctTopic **);

void
oct_topic_update(sqlite3*, sqlite3_int64, const char*);

#endif
