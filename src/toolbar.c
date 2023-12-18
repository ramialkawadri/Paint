/* gcal-month-view.h
 *
 * Copyright (C) 2012 - Erick Pérez Castellanos
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"

#include "toolbar.h"

struct _Toolbar
{
  GtkBox parent_type;

};

G_DEFINE_FINAL_TYPE (Toolbar, toolbar, GTK_TYPE_BOX);

static void
toolbar_init (Toolbar *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
}

static void
toolbar_dispose (GObject *gobject)
{
  gtk_widget_dispose_template (GTK_WIDGET (gobject), PAINT_TYPE_TOOLBAR);

  G_OBJECT_CLASS (toolbar_parent_class)->dispose (gobject);
}

static void
toolbar_class_init (ToolbarClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class,
                                               "/org/gnome/paint/toolbar.ui");

  G_OBJECT_CLASS (klass)->dispose = toolbar_dispose;
}

