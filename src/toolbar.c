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
    GtkSpinButton *spin_button
};

G_DEFINE_FINAL_TYPE(Toolbar, toolbar, GTK_TYPE_BOX);

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

    G_OBJECT_CLASS(klass)->dispose = toolbar_dispose;
}
