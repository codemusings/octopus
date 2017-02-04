#include "octopus.h"
#include "oct-settings-popover.h"

static void
oct_settings_popover_constructed(GObject*);

static void
oct_settings_popover_entry_changed(GtkEditable*, OctSettingsPopover*);

static void
oct_settings_popover_port_value_changed(GtkSpinButton*, OctSettingsPopover*);

static void
oct_settings_popover_save_button_clicked(GtkButton*, gpointer);

static void
oct_settings_popover_test_button_clicked(GtkButton*, OctSettingsPopover*);

static void
oct_settings_popover_test_complete(GObject*, GAsyncResult*, gpointer);

static void
oct_settings_popover_set_property(GObject*, guint, const GValue*, GParamSpec*);

struct _OctSettingsPopover {
    GtkPopover  parent_instance;
    OctSync*    sync;
    GtkEntry*   host_entry;
    GtkEntry*   username_entry;
    GtkEntry*   password_entry;
    GtkSpinner* spinner;
    GtkButton*  test_button;
    GtkButton*  save_button;
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
        G_CALLBACK(oct_settings_popover_entry_changed), self);

    GtkWidget* port_label = gtk_label_new("Port:");
    gtk_label_set_xalign(GTK_LABEL(port_label), 1);
    GtkWidget* port_spin_button;
    port_spin_button = gtk_spin_button_new_with_range(0, G_MAXDOUBLE, 1);
    gtk_spin_button_set_value(
        GTK_SPIN_BUTTON(port_spin_button), self->sync->port);
    g_signal_connect(port_spin_button, "value-changed",
        G_CALLBACK(oct_settings_popover_port_value_changed), self);

    GtkWidget* username_label = gtk_label_new("Username:");
    gtk_label_set_xalign(GTK_LABEL(username_label), 1);
    self->username_entry = GTK_ENTRY(gtk_entry_new());
    gtk_entry_set_text(self->username_entry, self->sync->username);
    gtk_entry_set_activates_default(self->username_entry, TRUE);
    g_signal_connect(self->username_entry, "changed",
        G_CALLBACK(oct_settings_popover_entry_changed), self);

    GtkWidget* password_label = gtk_label_new("Password:");
    gtk_label_set_xalign(GTK_LABEL(password_label), 1);
    self->password_entry = GTK_ENTRY(gtk_entry_new());
    gtk_entry_set_input_purpose(
        self->password_entry, GTK_INPUT_PURPOSE_PASSWORD);
    gtk_entry_set_visibility(self->password_entry, FALSE);
    gtk_entry_set_text(self->password_entry, self->sync->password);
    gtk_entry_set_activates_default(self->password_entry, TRUE);
    g_signal_connect(self->password_entry, "changed",
        G_CALLBACK(oct_settings_popover_entry_changed), self);

    self->test_button = GTK_BUTTON(gtk_button_new_with_label("Test"));
    g_signal_connect(self->test_button, "clicked",
        G_CALLBACK(oct_settings_popover_test_button_clicked), self);

    self->spinner = GTK_SPINNER(gtk_spinner_new());

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
        GTK_WIDGET(self->spinner), FALSE, FALSE, 0);
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

    oct_settings_popover_entry_changed(NULL, self);

    G_OBJECT_CLASS(oct_settings_popover_parent_class)->constructed(object);
}

static void
oct_settings_popover_entry_changed(GtkEditable*        editable,
                                   OctSettingsPopover* self)
{
    if (GTK_ENTRY(editable) == self->host_entry) {
        if (self->sync && self->sync->host) {
            g_free(self->sync->host);
        }
        self->sync->host = g_strdup(gtk_entry_get_text(self->host_entry));
    } else if (GTK_ENTRY(editable) == self->username_entry) {
        if (self->sync && self->sync->username) {
            g_free(self->sync->username);
        }
        self->sync->username = g_strdup(
            gtk_entry_get_text(self->username_entry));
    } else if (GTK_ENTRY(editable) == self->password_entry) {
        if (self->sync && self->sync->password) {
            g_free(self->sync->password);
        }
        self->sync->password = g_strdup(
            gtk_entry_get_text(self->password_entry));
    }

    gboolean enabled = gtk_entry_get_text_length(self->host_entry) > 0;
    enabled &= gtk_entry_get_text_length(self->username_entry) > 0;
    enabled &= gtk_entry_get_text_length(self->password_entry) > 0;
    gtk_widget_set_sensitive(GTK_WIDGET(self->test_button), enabled);
    gtk_widget_set_sensitive(GTK_WIDGET(self->save_button), enabled);
}

static void
oct_settings_popover_port_value_changed(GtkSpinButton*      spin_button,
                                        OctSettingsPopover* self)
{
    self->sync->port = gtk_spin_button_get_value_as_int(spin_button);
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
oct_settings_popover_test_button_clicked(GtkButton*          button,
                                         OctSettingsPopover* self)
{
    gtk_spinner_start(self->spinner);
    GTask* task = g_task_new(
        self, NULL, oct_settings_popover_test_complete, NULL);
    g_task_set_task_data(task, self->sync, NULL);
    g_task_run_in_thread(task, oct_sync_auth);
}

static void
oct_settings_popover_test_complete(GObject*      source_object,
                                   GAsyncResult* result,
                                   gpointer      user_data)
{
    OctSettingsPopover* self = OCT_SETTINGS_POPOVER(source_object);
    gtk_spinner_stop(self->spinner);
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

GtkWidget*
oct_settings_popover_new(GtkWidget* relative_to, OctSync* sync)
{
    return g_object_new(OCT_TYPE_SETTINGS_POPOVER,
        "relative-to", relative_to,
        "sync", sync,
        NULL);
}
