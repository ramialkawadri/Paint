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
#include "drawing-tool.h"

struct _Toolbar
{
  /* Widgets */
  GtkBox                parent_type;
  GtkColorDialogButton *color_button;
  GtkSpinButton        *spin_button;

  /* Drawing tools buttons */
  GtkButton            *currently_selected_button;

  GtkButton            *brush_button;
  GtkButton            *eraser_button;
  GtkButton            *rectangle_button;
  GtkButton            *circle_button;
};

enum {
  OPEN_FILE,
  SAVE_FILE,
  TOOL_CHANGE,
  NUMBER_OF_SIGNALS
};

static guint toolbar_signals[NUMBER_OF_SIGNALS];

G_DEFINE_FINAL_TYPE (Toolbar, toolbar, GTK_TYPE_BOX);


static void
on_open_button_click (GtkButton *button,
                      gpointer   user_data)
{
  Toolbar *self = user_data;
  g_signal_emit (self, toolbar_signals[OPEN_FILE], 0);
}

static void
on_save_button_click (GtkButton *button,
                      gpointer   user_data)
{
  Toolbar *self = user_data;
  g_signal_emit (self, toolbar_signals[SAVE_FILE], 0);
}

static void
update_current_selected_tool (Toolbar   *self,
                              GtkButton *selected_tool)
{
  gtk_widget_remove_css_class (GTK_WIDGET (self->currently_selected_button),
                               "suggested-action");

  self->currently_selected_button = selected_tool;

  gtk_widget_add_css_class (GTK_WIDGET (selected_tool), "suggested-action");
}

static void
on_brush_button_click (GtkButton *button,
                       gpointer   user_data)
{
  Toolbar *self = user_data;

  update_current_selected_tool (self, self->brush_button);
  g_signal_emit (self, toolbar_signals[TOOL_CHANGE], 0, BRUSH);
}

static void
on_eraser_button_click (GtkButton *button,
                       gpointer   user_data)
{
  Toolbar *self = user_data;

  update_current_selected_tool (self, self->eraser_button);
  g_signal_emit (self, toolbar_signals[TOOL_CHANGE], 0, ERASER);
}

static void
on_rectangle_button_click (GtkButton *button,
                           gpointer   user_data)
{
  Toolbar *self = user_data;

  update_current_selected_tool (self, self->rectangle_button);
  g_signal_emit (self, toolbar_signals[TOOL_CHANGE], 0, RECTANGLE);
}

static void
on_circle_button_click (GtkButton *button,
                        gpointer   user_data)
{
  Toolbar *self = user_data;

  update_current_selected_tool (self, self->circle_button);
  g_signal_emit (self, toolbar_signals[TOOL_CHANGE], 0, CIRCLE);
}

const GdkRGBA *
toolbar_get_current_color (Toolbar *self)
{
  return gtk_color_dialog_button_get_rgba (self->color_button);
}

double
toolbar_get_draw_size (Toolbar *self)
{
  return gtk_spin_button_get_value (self->spin_button);
}

static void
toolbar_init (Toolbar *self)
{
  gtk_widget_init_template (GTK_WIDGET(self));

  self->currently_selected_button = self->brush_button;
}

static void
toolbar_dispose (GObject *gobject)
{
  gtk_widget_dispose_template (GTK_WIDGET(gobject), PAINT_TYPE_TOOLBAR);

  G_OBJECT_CLASS (toolbar_parent_class)->dispose(gobject);
}

static void
toolbar_class_init (ToolbarClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class,
                                               "/org/gnome/paint/ui/toolbar.ui");

  gtk_widget_class_bind_template_child (widget_class, Toolbar, color_button);
  gtk_widget_class_bind_template_child (widget_class, Toolbar, spin_button);
  gtk_widget_class_bind_template_child (widget_class, Toolbar, brush_button);
  gtk_widget_class_bind_template_child (widget_class, Toolbar, eraser_button);
  gtk_widget_class_bind_template_child (widget_class, Toolbar, rectangle_button);
  gtk_widget_class_bind_template_child (widget_class, Toolbar, circle_button);

  gtk_widget_class_bind_template_callback (widget_class, on_open_button_click);
  gtk_widget_class_bind_template_callback (widget_class, on_save_button_click);
  gtk_widget_class_bind_template_callback (widget_class, on_brush_button_click);
  gtk_widget_class_bind_template_callback (widget_class, on_eraser_button_click);
  gtk_widget_class_bind_template_callback (widget_class, on_rectangle_button_click);
  gtk_widget_class_bind_template_callback (widget_class, on_circle_button_click);

  G_OBJECT_CLASS (klass)->dispose = toolbar_dispose;

  /* Signals */
  toolbar_signals[OPEN_FILE] = g_signal_new ("open-file",
                                             G_TYPE_FROM_CLASS (klass),
                                             G_SIGNAL_RUN_LAST,
                                             0,
                                             NULL,
                                             NULL,
                                             NULL,
                                             G_TYPE_NONE,
                                             0);

  toolbar_signals[SAVE_FILE] = g_signal_new ("save-file",
                                             G_TYPE_FROM_CLASS (klass),
                                             G_SIGNAL_RUN_LAST,
                                             0,
                                             NULL,
                                             NULL,
                                             NULL,
                                             G_TYPE_NONE,
                                             0);

  toolbar_signals[TOOL_CHANGE] = g_signal_new ("tool-change",
                                               G_TYPE_FROM_CLASS (klass),
                                               G_SIGNAL_RUN_LAST,
                                               0,
                                               NULL,
                                               NULL,
                                               NULL,
                                               G_TYPE_NONE,
                                               1,
                                               G_TYPE_INT);
}
