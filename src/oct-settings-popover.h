#ifndef __OCT_SETTINGS_POPOVER_H__
#define __OCT_SETTINGS_POPOVER_H__

#include <gtk/gtk.h>

#include "octopus.h"

G_BEGIN_DECLS

#define OCT_TYPE_SETTINGS_POPOVER oct_settings_popover_get_type()
G_DECLARE_FINAL_TYPE(
    OctSettingsPopover, oct_settings_popover, OCT, SETTINGS_POPOVER, GtkPopover)

GtkWidget*
oct_settings_popover_new(GtkWidget*, OctSync*);

G_END_DECLS

#endif
