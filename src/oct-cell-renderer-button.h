#ifndef __OCT_CELL_RENDERER_BUTTON_H__
#define __OCT_CELL_RENDERER_BUTTON_H__

#include <gtk/gtk.h>

#define OCT_TYPE_CELL_RENDERER_BUTTON oct_cell_renderer_button_get_type()
G_DECLARE_FINAL_TYPE(OctCellRendererButton, oct_cell_renderer_button,
    OCT, CELL_RENDERER_BUTTON, GtkCellRenderer)

GtkCellRenderer* oct_cell_renderer_button_new(void);

#endif
