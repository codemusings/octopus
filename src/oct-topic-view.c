#include <sqlite3.h>

#include "octopus.h"
#include "oct-application.h"
#include "oct-topic-view.h"


struct _OctTopicView {
    GtkBox     parent_instance;
    GtkWidget *add_topic_button;
    GtkWidget *add_topic_confirm_button;
    GtkWidget *add_topic_error_label;
    GtkWidget *add_topic_popover;
    GtkWidget *add_topic_title_entry;
    GtkWidget *delete_topic_button;
    GtkWidget *edit_topic_button;
    GtkWidget *next_topic_button;
    GtkWidget *previous_topic_button;
    GtkWidget *topic_combo_box;
};
G_DEFINE_TYPE(OctTopicView, oct_topic_view, GTK_TYPE_BOX)

enum {
    COLUMN_ROWID,
    COLUMN_TITLE,
    NUM_COLUMNS
};


static void
on_add_topic_button_clicked(GtkButton*, OctTopicView*);

static void
on_add_topic_confirm_button_clicked(GtkButton*, OctTopicView*);

static void
on_add_topic_popover_closed(GtkPopover*, OctTopicView*);

static void
on_add_topic_title_entry_activate(GtkEntry*, OctTopicView*);

static void
on_add_topic_title_entry_changed(GtkEntry*, OctTopicView*);

static void
on_delete_topic_button_clicked(GtkButton*, OctTopicView*);

static void
on_edit_topic_button_clicked(GtkButton*, OctTopicView*);

static void
on_next_topic_button_clicked(GtkButton*, OctTopicView*);

static void
on_previous_topic_button_clicked(GtkButton*, OctTopicView*);

static void
on_topic_combo_box_changed(GtkComboBox*, OctTopicView*);


static void
oct_topic_view_class_init(OctTopicViewClass *class)
{
}

static void
oct_topic_view_init(OctTopicView *self)
{
    GtkWidget *add_topic_box;
    GtkWidget *add_topic_entry_box;
    GtkTreeIter iter;
    GtkCssProvider *provider;
    GtkListStore *store;
    GtkCellRenderer *title_renderer;
    OctTopic **topic;
    OctTopic **topics;

    /* topic view box */
    gtk_orientable_set_orientation(
        GTK_ORIENTABLE(self), GTK_ORIENTATION_HORIZONTAL);
    GtkStyleContext *context = gtk_widget_get_style_context(GTK_WIDGET(self));
    gtk_style_context_add_class(context, "linked");

    /* previous topic button */
    self->previous_topic_button = gtk_button_new_from_icon_name(
        "pan-start-symbolic", GTK_ICON_SIZE_BUTTON);
    gtk_widget_set_sensitive(self->previous_topic_button, FALSE);
    g_signal_connect(self->previous_topic_button, "clicked",
        G_CALLBACK(on_previous_topic_button_clicked), self);

    /* delete topic button */
    self->delete_topic_button = gtk_button_new_from_icon_name(
        "user-trash-symbolic", GTK_ICON_SIZE_BUTTON);
    gtk_widget_set_sensitive(self->delete_topic_button, FALSE);
    g_signal_connect(self->delete_topic_button, "clicked",
        G_CALLBACK(on_delete_topic_button_clicked), self);

    /* edit topic button */
    self->edit_topic_button = gtk_button_new_from_icon_name(
        "document-edit-symbolic", GTK_ICON_SIZE_BUTTON);
    gtk_widget_set_sensitive(self->edit_topic_button, FALSE);
    g_signal_connect(self->edit_topic_button, "clicked",
        G_CALLBACK(on_edit_topic_button_clicked), self);

    /* topic combo box */
    self->topic_combo_box = gtk_combo_box_new();
    title_renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(
        GTK_CELL_LAYOUT(self->topic_combo_box), title_renderer, TRUE);
    gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(self->topic_combo_box),
        title_renderer, "text", COLUMN_TITLE, NULL);

    store = gtk_list_store_new(NUM_COLUMNS,
        G_TYPE_INT64, /* COLUMN_ROWID */
        G_TYPE_STRING /* COLUMN_TITLE */
    );
    topics = oct_topic_fetch_all(oct_application_get_database());
    for (topic = topics; *topic != NULL; topic++) {
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter,
            COLUMN_ROWID, (*topic)->rowid,
            COLUMN_TITLE, (*topic)->title,
            -1);
    }

    gtk_combo_box_set_model(
        GTK_COMBO_BOX(self->topic_combo_box), GTK_TREE_MODEL(store));
    gtk_combo_box_set_active(GTK_COMBO_BOX(self->topic_combo_box), 0);
    g_object_unref(store);

    g_signal_connect(self->topic_combo_box, "changed",
        G_CALLBACK(on_topic_combo_box_changed), self);

    /* add topic button */
    self->add_topic_button = gtk_button_new_from_icon_name(
        "list-add-symbolic", GTK_ICON_SIZE_BUTTON);
    g_signal_connect(self->add_topic_button, "clicked",
        G_CALLBACK(on_add_topic_button_clicked), self);

    /* next topic button */
    self->next_topic_button = gtk_button_new_from_icon_name(
        "pan-end-symbolic", GTK_ICON_SIZE_BUTTON);
    gtk_widget_set_sensitive(self->next_topic_button,
        gtk_tree_model_iter_n_children(GTK_TREE_MODEL(store), NULL) > 1);
    g_signal_connect(self->next_topic_button, "clicked",
        G_CALLBACK(on_next_topic_button_clicked), self);

    /* topic view box layout */
    gtk_box_pack_start(
        GTK_BOX(self), self->previous_topic_button, FALSE, FALSE, 0);
    gtk_box_pack_start(
        GTK_BOX(self), self->delete_topic_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(self), self->edit_topic_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(self), self->topic_combo_box, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(self), self->add_topic_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(self), self->next_topic_button, FALSE, FALSE, 0);

    /* add topic popover */
    self->add_topic_title_entry = gtk_entry_new();
    g_signal_connect(self->add_topic_title_entry, "activate",
        G_CALLBACK(on_add_topic_title_entry_activate), self);
    g_signal_connect(self->add_topic_title_entry, "changed",
        G_CALLBACK(on_add_topic_title_entry_changed), self);

    self->add_topic_confirm_button = gtk_button_new_from_icon_name(
        "object-select-symbolic", GTK_ICON_SIZE_BUTTON);
    gtk_widget_set_sensitive(self->add_topic_confirm_button, FALSE);

    add_topic_entry_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(add_topic_entry_box),
        self->add_topic_title_entry, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(add_topic_entry_box),
        self->add_topic_confirm_button, FALSE, FALSE, 0);
    context = gtk_widget_get_style_context(add_topic_entry_box);
    gtk_style_context_add_class(context, "linked");

    self->add_topic_error_label = gtk_label_new("Topic already exists");
    gtk_label_set_xalign(GTK_LABEL(self->add_topic_error_label), 0);
    provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(
        provider, "label { color: red; font-size: small; }", -1, NULL);
    context = gtk_widget_get_style_context(self->add_topic_error_label);
    gtk_style_context_add_provider(context, GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    add_topic_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_set_border_width(GTK_CONTAINER(add_topic_box), 10);
    gtk_box_pack_start(
        GTK_BOX(add_topic_box), add_topic_entry_box, FALSE, FALSE, 0);
    gtk_box_pack_start(
        GTK_BOX(add_topic_box), self->add_topic_error_label, FALSE, FALSE, 0);
    gtk_widget_show_all(add_topic_box);
    gtk_widget_hide(self->add_topic_error_label);

    self->add_topic_popover = gtk_popover_new(NULL);
    gtk_container_add(GTK_CONTAINER(self->add_topic_popover), add_topic_box);
    g_signal_connect(self->add_topic_popover, "closed",
        G_CALLBACK(on_add_topic_popover_closed), self);

    gtk_widget_show_all(GTK_WIDGET(self));

    oct_topic_list_free(topics);
}

static void
on_add_topic_button_clicked(GtkButton *button, OctTopicView *self)
{
    gtk_entry_set_text(GTK_ENTRY(self->add_topic_title_entry), "");
    gtk_popover_set_relative_to(
        GTK_POPOVER(self->add_topic_popover), GTK_WIDGET(button));
    gtk_popover_popup(GTK_POPOVER(self->add_topic_popover));
}

static void
on_add_topic_confirm_button_clicked(GtkButton *button, OctTopicView *self)
{
    gint active_topic;
    GtkTreeIter iter;
    GtkTreeModel *model;
    GtkWidget *relative_to;
    sqlite3_int64 rowid;
    gchar* title;

    title = g_strdup(
        gtk_entry_get_text(GTK_ENTRY(self->add_topic_title_entry)));

    model = gtk_combo_box_get_model(GTK_COMBO_BOX(self->topic_combo_box));
    relative_to = gtk_popover_get_relative_to(
        GTK_POPOVER(self->add_topic_popover));
    if (relative_to == self->add_topic_button) {
        rowid = oct_topic_add(oct_application_get_database(), title);
        gtk_list_store_append(GTK_LIST_STORE(model), &iter);
        gtk_list_store_set(GTK_LIST_STORE(model), &iter,
            COLUMN_ROWID, rowid,
            COLUMN_TITLE, title,
            -1);
    } else {
        active_topic = gtk_combo_box_get_active(
            GTK_COMBO_BOX(self->topic_combo_box));
        gtk_tree_model_iter_nth_child(model, &iter, NULL, active_topic);
        gtk_tree_model_get(model, &iter, COLUMN_ROWID, &rowid, -1);
        oct_topic_update(oct_application_get_database(), rowid, title);
        gtk_list_store_set(
            GTK_LIST_STORE(model), &iter, COLUMN_TITLE, title, -1);
    }

    gtk_entry_set_text(GTK_ENTRY(self->add_topic_title_entry), "");
    gtk_popover_popdown(GTK_POPOVER(self->add_topic_popover));
    if (relative_to == self->add_topic_button) {
        gtk_combo_box_set_active(GTK_COMBO_BOX(self->topic_combo_box),
            gtk_tree_model_iter_n_children(model, NULL) - 1);
    }

    /* signal that a topic has been added */
}

static void
on_add_topic_popover_closed(GtkPopover* popover, OctTopicView* self)
{
    gtk_entry_set_text(GTK_ENTRY(self->add_topic_title_entry), "");
}

static void
on_add_topic_title_entry_activate(GtkEntry *entry, OctTopicView *self)
{
    if (gtk_entry_get_text_length(entry) > 0) {
        on_add_topic_confirm_button_clicked(
            GTK_BUTTON(self->add_topic_confirm_button), self);
    }
}

static void
on_add_topic_title_entry_changed(GtkEntry *entry, OctTopicView *self)
{
    bool exists;
    gint length;
    GtkWidget *relative_to;
    const gchar *title;

    title = gtk_entry_get_text(entry);
    length = gtk_entry_get_text_length(entry);
    exists = oct_topic_exists(oct_application_get_database(), title);

    gtk_widget_set_sensitive(
        self->add_topic_confirm_button, length > 0 && !exists);

    relative_to = gtk_popover_get_relative_to(
        GTK_POPOVER(self->add_topic_popover));
    gtk_widget_set_visible(self->add_topic_error_label,
        exists && relative_to == self->add_topic_button);
}

static void
on_delete_topic_button_clicked(GtkButton* button, OctTopicView* self)
{
    gint active_topic;
    GtkTreeIter iter;
    GtkTreeModel *model;
    sqlite3_int64 rowid;

    active_topic = gtk_combo_box_get_active(
        GTK_COMBO_BOX(self->topic_combo_box));
    gtk_combo_box_set_active(GTK_COMBO_BOX(self->topic_combo_box), 0);

    model = gtk_combo_box_get_model(GTK_COMBO_BOX(self->topic_combo_box));
    gtk_tree_model_iter_nth_child(model, &iter, NULL, active_topic);
    gtk_tree_model_get(model, &iter, COLUMN_ROWID, &rowid, -1);

    gtk_list_store_remove(GTK_LIST_STORE(model), &iter);
    on_topic_combo_box_changed(GTK_COMBO_BOX(self->topic_combo_box), self);

    oct_task_delete_by_topic(oct_application_get_database(), rowid);
    oct_topic_delete(oct_application_get_database(), rowid);
}

static void
on_edit_topic_button_clicked(GtkButton* button, OctTopicView* self)
{
    gint active_topic;
    GtkTreeIter iter;
    GtkTreeModel *model;
    gchar* title;

    /* needs to be set before the entry is filled
     * to ensure that the error label stays hidden */
    gtk_popover_set_relative_to(
        GTK_POPOVER(self->add_topic_popover), GTK_WIDGET(button));

    active_topic = gtk_combo_box_get_active(
        GTK_COMBO_BOX(self->topic_combo_box));
    model = gtk_combo_box_get_model(GTK_COMBO_BOX(self->topic_combo_box));
    gtk_tree_model_iter_nth_child(model, &iter, NULL, active_topic);
    gtk_tree_model_get(model, &iter, COLUMN_TITLE, &title, -1);
    gtk_entry_set_text(GTK_ENTRY(self->add_topic_title_entry), title);

    gtk_popover_popup(GTK_POPOVER(self->add_topic_popover));
}

static void
on_next_topic_button_clicked(GtkButton* button, OctTopicView* self)
{
    gtk_combo_box_set_active(GTK_COMBO_BOX(self->topic_combo_box),
        gtk_combo_box_get_active(GTK_COMBO_BOX(self->topic_combo_box)) + 1);
}

static void
on_previous_topic_button_clicked(GtkButton* button, OctTopicView* self)
{
    gtk_combo_box_set_active(GTK_COMBO_BOX(self->topic_combo_box),
        gtk_combo_box_get_active(GTK_COMBO_BOX(self->topic_combo_box)) - 1);
}

static void
on_topic_combo_box_changed(GtkComboBox* combo_box, OctTopicView* self)
{
    GtkTreeIter iter;
    GtkTreeModel *model;
    gint active_topic;
    gint num_topics;
    sqlite3_int64 rowid;

    model = gtk_combo_box_get_model(combo_box);
    num_topics = gtk_tree_model_iter_n_children(model, NULL);
    active_topic = gtk_combo_box_get_active(combo_box);
    gtk_tree_model_iter_nth_child(model, &iter, NULL, active_topic);

    gtk_widget_set_sensitive(self->previous_topic_button, active_topic > 0);
    gtk_widget_set_sensitive(self->delete_topic_button, active_topic > 0);
    gtk_widget_set_sensitive(self->edit_topic_button, active_topic > 0);
    gtk_widget_set_sensitive(
        self->next_topic_button, active_topic < num_topics - 1);

    gtk_tree_model_get(model, &iter, COLUMN_ROWID, &rowid, -1);
    g_signal_emit_by_name(oct_application_shared(), "topic-selected", rowid);
}

GtkWidget*
oct_topic_view_new(void)
{
    return g_object_new(OCT_TYPE_TOPIC_VIEW, NULL);
}
