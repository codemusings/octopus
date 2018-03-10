#ifndef __OCT_APPLICATION_H__
#define __OCT_APPLICATION_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define OCT_TYPE_APPLICATION oct_application_get_type()
G_DECLARE_FINAL_TYPE(
    OctApplication, oct_application, OCT, APPLICATION, GtkApplication)

sqlite3*
oct_application_get_database(void);

OctApplication*
oct_application_new(const char*, GApplicationFlags, sqlite3*);

OctApplication*
oct_application_shared(void);

G_END_DECLS

#endif
