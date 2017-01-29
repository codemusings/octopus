#include <math.h>

#include "oct-cell-renderer-button.h"

#define M_PI 3.14159
#define SIZE 16
#define PADDING 4

static gboolean
oct_cell_renderer_button_activate(GtkCellRenderer*,
                                  GdkEvent*,
                                  GtkWidget*,
                                  const gchar*,
                                  const GdkRectangle*,
                                  const GdkRectangle*,
                                  GtkCellRendererState);
static void
oct_cell_renderer_button_get_preferred_height(GtkCellRenderer*,
                                              GtkWidget*,
                                              gint*,
                                              gint*);
static void
oct_cell_renderer_button_get_preferred_width(GtkCellRenderer*,
                                             GtkWidget*,
                                             gint*,
                                             gint*);
static void
oct_cell_renderer_button_render(GtkCellRenderer*,
                                cairo_t*,
                                GtkWidget*,
                                const GdkRectangle*,
                                const GdkRectangle*,
                                GtkCellRendererState);
static void
oct_cell_renderer_set_property(GObject*, guint, const GValue*, GParamSpec*);

struct _OctCellRendererButton {
    GtkCellRenderer parent_instance;
    gboolean        visible;
};
G_DEFINE_TYPE(
    OctCellRendererButton, oct_cell_renderer_button, GTK_TYPE_CELL_RENDERER)

enum {
    PROP_VISIBLE = 1,
    NUM_PROPS
};
static GParamSpec* properties[NUM_PROPS];

static gboolean
oct_cell_renderer_button_activate(GtkCellRenderer*     renderer,
                                  GdkEvent*            event,
                                  GtkWidget*           widget,
                                  const gchar*         path_string,
                                  const GdkRectangle*  background_area,
                                  const GdkRectangle*  cell_area,
                                  GtkCellRendererState flags)
{
    OctCellRendererButton* self = OCT_CELL_RENDERER_BUTTON(renderer);
    if (!self->visible) {
        return FALSE;
    }
    if (event->type != GDK_BUTTON_PRESS) {
        return FALSE;
    }
    if (((GdkEventButton*)event)->x >= cell_area->x &&
        ((GdkEventButton*)event)->x <= cell_area->x + cell_area->width &&
        ((GdkEventButton*)event)->y >= cell_area->y &&
        ((GdkEventButton*)event)->y <= cell_area->y + cell_area->height)
    {
        g_signal_emit_by_name(renderer, "clicked", path_string);
        return TRUE;
    }
    return FALSE;
}

static void
oct_cell_renderer_button_class_init(OctCellRendererButtonClass* class)
{
    GObjectClass* object_class = G_OBJECT_CLASS(class);
    object_class->set_property = oct_cell_renderer_set_property;

    properties[PROP_VISIBLE] = g_param_spec_boolean("visible",
        "visible", "visible", TRUE, G_PARAM_WRITABLE);
    g_object_class_install_properties(object_class, NUM_PROPS, properties);

    GtkCellRendererClass* r_class = GTK_CELL_RENDERER_CLASS(class);
    r_class->activate = oct_cell_renderer_button_activate;
    r_class->get_preferred_height =
        oct_cell_renderer_button_get_preferred_height;
    r_class->get_preferred_width = oct_cell_renderer_button_get_preferred_width;
    r_class->render = oct_cell_renderer_button_render;

    g_signal_new("clicked",
        G_OBJECT_CLASS_TYPE(class),
        G_SIGNAL_RUN_FIRST,
        0, NULL, NULL, NULL, G_TYPE_NONE, 1, G_TYPE_STRING);
}

static void
oct_cell_renderer_button_get_preferred_height(GtkCellRenderer* renderer,
                                              GtkWidget*       widget,
                                              gint*            minimum_height,
                                              gint*            natural_height)
{
    if (minimum_height) {
        *minimum_height = SIZE;
    }
    if (natural_height) {
        *natural_height = SIZE;
    }
}

static void
oct_cell_renderer_button_get_preferred_width(GtkCellRenderer* renderer,
                                             GtkWidget*       widget,
                                             gint*            minimum_width,
                                             gint*            natural_width)
{
    if (minimum_width) {
        *minimum_width = SIZE + PADDING;
    }
    if (natural_width) {
        *natural_width = SIZE + PADDING;
    }
}

static void
oct_cell_renderer_button_init(OctCellRendererButton* self)
{
    g_object_set(self, "mode", GTK_CELL_RENDERER_MODE_ACTIVATABLE, NULL);
}

static void
oct_cell_renderer_button_render(GtkCellRenderer*     renderer,
                                cairo_t*             cr,
                                GtkWidget*           widget,
                                const GdkRectangle*  background_area,
                                const GdkRectangle*  cell_area,
                                GtkCellRendererState flags)
{
    OctCellRendererButton* self = OCT_CELL_RENDERER_BUTTON(renderer);
    if (!self->visible) {
        return;
    }

    double x = cell_area->width / 2.0 + PADDING / 2.0;
    double y = cell_area->height / 2.0;
    double r = SIZE / 2.0;

    cairo_save(cr);
    cairo_translate(cr, cell_area->x, cell_area->y);

    cairo_arc(cr, x, y, r, 0, 2 * M_PI);
    cairo_set_source_rgb(cr, 0.85, 0.85, 0.85);
    cairo_fill(cr);

    double offset = cos(M_PI / 4.0) * r - 3;
    cairo_move_to(cr, x - offset, y - offset);
    cairo_line_to(cr, x + offset, y + offset);
    cairo_set_source_rgb(cr, 0.5, 0.5, 0.5);
    cairo_set_line_width(cr, 1.5);
    cairo_stroke(cr);

    cairo_move_to(cr, x - offset, y + offset);
    cairo_line_to(cr, x + offset, y - offset);
    cairo_stroke(cr);

    cairo_restore(cr);
}

static void
oct_cell_renderer_set_property(GObject*      object,
                               guint         property_id,
                               const GValue* value,
                               GParamSpec*   spec)
{
    OctCellRendererButton* self = OCT_CELL_RENDERER_BUTTON(object);
    switch (property_id) {
        case PROP_VISIBLE:
            self->visible = g_value_get_boolean(value);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, spec);
    }
}

GtkCellRenderer*
oct_cell_renderer_button_new(void)
{
    return g_object_new(OCT_TYPE_CELL_RENDERER_BUTTON, NULL);
}
