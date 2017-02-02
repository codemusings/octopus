#include "oct-settings-popover.h"

static void
oct_settings_popover_constructed(GObject*);

static void
oct_settings_popover_save_button_clicked(GtkButton*, gpointer);

static void
oct_settings_popover_set_property(GObject*, guint, const GValue*, GParamSpec*);

static void
oct_settings_popover_toggle_buttons(GtkEditable*, OctSettingsPopover*);

struct _OctSettingsPopover {
    GtkPopover parent_instance;
    OctSync*   sync;
    GtkEntry*  host_entry;
    GtkEntry*  username_entry;
    GtkEntry*  password_entry;
    GtkButton* test_button;
    GtkButton* save_button;
};
G_DEFINE_TYPE(OctSettingsPopover, oct_settings_popover, GTK_TYPE_POPOVER)

enum {
    PROP_SYNC = 1,
    NUM_PROPS
};
static GParamSpec* properties[NUM_PROPS];

static void
oct_settings_popover_class_init(OctSettingsPopoverClass* class)
{
    GObjectClass* object_class = G_OBJECT_CLASS(class);
    object_class->constructed = oct_settings_popover_constructed;
    object_class->set_property = oct_settings_popover_set_property;
    properties[PROP_SYNC] = g_param_spec_pointer("sync", "sync", "sync",
        G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY);
    g_object_class_install_properties(object_class, NUM_PROPS, properties);
}

static void
oct_settings_popover_constructed(GObject* object)
{
    OctSettingsPopover* self = OCT_SETTINGS_POPOVER(object);

    GtkWidget* host_label = gtk_label_new("Hostname:");
    gtk_label_set_xalign(GTK_LABEL(host_label), 1);
    self->host_entry = GTK_ENTRY(gtk_entry_new());
    gtk_entry_set_text(self->host_entry, self->sync->host);
    gtk_entry_set_placeholder_text(self->host_entry, "octopus.org");
    gtk_entry_set_activates_default(self->host_entry, TRUE);
    g_signal_connect(self->host_entry, "changed",
        G_CALLBACK(oct_settings_popover_toggle_buttons), self);

    GtkWidget* port_label = gtk_label_new("Port:");
    gtk_label_set_xalign(GTK_LABEL(port_label), 1);
    GtkWidget* port_spin_button;
    port_spin_button = gtk_spin_button_new_with_range(0, G_MAXDOUBLE, 1);
    gtk_spin_button_set_value(
        GTK_SPIN_BUTTON(port_spin_button), self->sync->port);

    GtkWidget* username_label = gtk_label_new("Username:");
    gtk_label_set_xalign(GTK_LABEL(username_label), 1);
    self->username_entry = GTK_ENTRY(gtk_entry_new());
    gtk_entry_set_text(self->username_entry, self->sync->username);
    gtk_entry_set_activates_default(self->username_entry, TRUE);
    g_signal_connect(self->username_entry, "changed",
        G_CALLBACK(oct_settings_popover_toggle_buttons), self);

    GtkWidget* password_label = gtk_label_new("Password:");
    gtk_label_set_xalign(GTK_LABEL(password_label), 1);
    self->password_entry = GTK_ENTRY(gtk_entry_new());
    gtk_entry_set_input_purpose(
        self->password_entry, GTK_INPUT_PURPOSE_PASSWORD);
    gtk_entry_set_visibility(self->password_entry, FALSE);
    gtk_entry_set_text(self->password_entry, self->sync->password);
    gtk_entry_set_activates_default(self->password_entry, TRUE);
    g_signal_connect(self->password_entry, "changed",
        G_CALLBACK(oct_settings_popover_toggle_buttons), self);

    self->test_button = GTK_BUTTON(gtk_button_new_with_label("Test"));

    self->save_button = GTK_BUTTON(gtk_button_new_with_label("Save"));
    gtk_widget_set_can_default(GTK_WIDGET(self->save_button), TRUE);
    GtkStyleContext* context;
    context = gtk_widget_get_style_context(GTK_WIDGET(self->save_button));
    gtk_style_context_add_class(context, "suggested-action");
    g_signal_connect(self->save_button, "activate",
        G_CALLBACK(oct_settings_popover_save_button_clicked), self);
    g_signal_connect(self->save_button, "clicked",
        G_CALLBACK(oct_settings_popover_save_button_clicked), self);

    /* layout */
    GtkWidget* button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_pack_start(GTK_BOX(button_box),
        GTK_WIDGET(self->test_button), TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(button_box),
        GTK_WIDGET(self->save_button), TRUE, TRUE, 0);

    GtkWidget* grid = gtk_grid_new();
    gtk_grid_attach(GTK_GRID(grid), host_label, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), GTK_WIDGET(self->host_entry), 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), port_label, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), port_spin_button, 1, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), username_label, 0, 2, 1, 1);
    gtk_grid_attach(
        GTK_GRID(grid), GTK_WIDGET(self->username_entry), 1, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), password_label, 0, 3, 1, 1);
    gtk_grid_attach(
        GTK_GRID(grid), GTK_WIDGET(self->password_entry), 1, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), button_box, 0, 4, 2, 1);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_widget_show_all(grid);

    /* popover */
    gtk_container_add(GTK_CONTAINER(self), grid);
    gtk_container_set_border_width(GTK_CONTAINER(self), 10);
    gtk_popover_set_default_widget(
        GTK_POPOVER(self), GTK_WIDGET(self->save_button));

    oct_settings_popover_toggle_buttons(NULL, self);

    G_OBJECT_CLASS(oct_settings_popover_parent_class)->constructed(object);
}

static void
oct_settings_popover_init(OctSettingsPopover* self)
{
}

static void
oct_settings_popover_save_button_clicked(GtkButton* button, gpointer user_data)
{
    gtk_widget_hide(GTK_WIDGET(user_data));
}

static void
oct_settings_popover_set_property(GObject*      object,
                                  guint         property_id,
                                  const GValue* value,
                                  GParamSpec*   spec)
{
    OctSettingsPopover* self = OCT_SETTINGS_POPOVER(object);
    switch (property_id) {
        case PROP_SYNC:
            self->sync = g_value_get_pointer(value);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, spec);
    }
}

static void
oct_settings_popover_toggle_buttons(GtkEditable*        editable,
                                    OctSettingsPopover* self)
{
    gboolean enabled = gtk_entry_get_text_length(self->host_entry) > 0;
    enabled &= gtk_entry_get_text_length(self->username_entry) > 0;
    enabled &= gtk_entry_get_text_length(self->password_entry) > 0;
    gtk_widget_set_sensitive(GTK_WIDGET(self->test_button), enabled);
    gtk_widget_set_sensitive(GTK_WIDGET(self->save_button), enabled);
}

GtkWidget*
oct_settings_popover_new(GtkWidget* relative_to, OctSync* sync)
{
    return g_object_new(OCT_TYPE_SETTINGS_POPOVER,
        "relative-to", relative_to,
        "sync", sync,
        NULL);
}
