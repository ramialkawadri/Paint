/* toolbar.c
 *
 * Copyright 2023 Ramikw
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "config.h"

#include "toolbar.h"

struct _Toolbar
{
    /* Widgets */
    GtkBox parent_type;
    GtkColorDialogButton *color_button;
    GtkSpinButton *spin_button;

    /* Callbacks */
    GAsyncReadyCallback on_file_open_cb;
    gpointer file_open_user_data;
};

G_DEFINE_FINAL_TYPE(Toolbar, toolbar, GTK_TYPE_BOX);

static void
on_open_button_click(GtkButton *button,
                     gpointer user_data)
{
    Toolbar *self = user_data;

    g_autoptr(GtkFileDialog) file_dialog = gtk_file_dialog_new();

    GtkRoot *root = gtk_widget_get_root(GTK_WIDGET(self));

    g_autoptr(GtkFileFilter) file_filter = gtk_file_filter_new();

    gtk_file_filter_add_mime_type(file_filter, "image/*");

    gtk_file_dialog_set_default_filter(file_dialog, file_filter);

    gtk_file_dialog_open(file_dialog,
                         GTK_WINDOW(root),
                         NULL,
                         self->on_file_open_cb,
                         self->file_open_user_data);
}

const GdkRGBA *
toolbar_get_current_color(Toolbar *self)
{
    return gtk_color_dialog_button_get_rgba(self->color_button);
}

double
toolbar_get_draw_size(Toolbar *self)
{
    return gtk_spin_button_get_value(self->spin_button);
}

void toolbar_set_file_open_cb(Toolbar *self, GAsyncReadyCallback cb)
{
    self->on_file_open_cb = cb;
}

void toolbar_set_file_open_user_data(Toolbar *self, gpointer user_data)
{
    self->file_open_user_data = user_data;
}

static void
toolbar_init(Toolbar *self)
{
    gtk_widget_init_template(GTK_WIDGET(self));
}

static void
toolbar_dispose(GObject *gobject)
{
    gtk_widget_dispose_template(GTK_WIDGET(gobject), PAINT_TYPE_TOOLBAR);

    G_OBJECT_CLASS(toolbar_parent_class)->dispose(gobject);
}

static void
toolbar_class_init(ToolbarClass *klass)
{
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

    gtk_widget_class_set_template_from_resource(widget_class,
                                                "/org/gnome/paint/toolbar.ui");

    gtk_widget_class_bind_template_child(widget_class, Toolbar, color_button);
    gtk_widget_class_bind_template_child(widget_class, Toolbar, spin_button);

    gtk_widget_class_bind_template_callback(widget_class, on_open_button_click);

    G_OBJECT_CLASS(klass)->dispose = toolbar_dispose;
}
