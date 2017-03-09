#include "oct-topic-view.h"

struct _OctTopicView {
    GtkBox parent_instance;
};
G_DEFINE_TYPE(OctTopicView, oct_topic_view, GTK_TYPE_BOX)

static void
oct_topic_view_class_init(OctTopicViewClass* class)
{
}

static void
oct_topic_view_init(OctTopicView* self)
{
}

GtkWidget*
oct_topic_view_new(void)
{
    return g_object_new(OCT_TYPE_TOPIC_VIEW, NULL);
}
