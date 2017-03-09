#ifndef __OCT_TOPIC_VIEW_H__
#define __OCT_TOPIC_VIEW_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define OCT_TYPE_TOPIC_VIEW oct_topic_view_get_type()
G_DECLARE_FINAL_TYPE(OctTopicView, oct_topic_view, OCT, TOPIC_VIEW, GtkBox)

GtkWidget*
oct_topic_view_new(void);

G_END_DECLS

#endif
