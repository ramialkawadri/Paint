/* toolbar.c
 *
 * Copyright 2023 Rami Alkawadri
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

#include "toolbar.h"
#include "drawing-tools/drawing-tool-type.h"

enum {
  OPEN_FILE,
  SAVE_FILE,
  TOOL_CHANGE,
  NUMBER_OF_SIGNALS
};

struct _Toolbar
{
  /* Widgets */
  GtkBox                parent_type;
  GtkColorDialogButton *color_button;
  GtkSpinButton        *drawing_size_spin_button;

  /* Drawing tools buttons */
  GtkButton            *currently_selected_button;

  GtkButton            *brush_button;
  GtkButton            *eraser_button;
  GtkButton            *rectangle_button;
  GtkButton            *circle_button;
  GtkButton            *line_button;
  GtkButton            *text_button;
  GtkButton            *fill_button;
  GtkButton            *color_picker_button;
  GtkButton            *select_button;
};

G_DEFINE_FINAL_TYPE (Toolbar, toolbar, GTK_TYPE_BOX);

static guint toolbar_signals[NUMBER_OF_SIGNALS];

static const gchar *SELECTED_TOOL_CSS_CLASS = "suggested-action";

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
update_current_selected_tool (Toolbar          *self,
                              GtkButton        *selected_tool,
                              DRAWING_TOOL_TYPE tool_type)
{
  gtk_widget_remove_css_class (GTK_WIDGET (self->currently_selected_button),
                               SELECTED_TOOL_CSS_CLASS);

  self->currently_selected_button = selected_tool;

  gtk_widget_add_css_class (GTK_WIDGET (selected_tool), SELECTED_TOOL_CSS_CLASS);

  g_signal_emit (self, toolbar_signals[TOOL_CHANGE], 0, tool_type);
}

static void
on_brush_button_click (GtkButton *button,
                       gpointer   user_data)
{
  Toolbar *self = user_data;
  update_current_selected_tool (self, self->brush_button, BRUSH);
}

static void
on_eraser_button_click (GtkButton *button,
                        gpointer   user_data)
{
  Toolbar *self = user_data;
  update_current_selected_tool (self, self->eraser_button, ERASER);
}

static void
on_rectangle_button_click (GtkButton *button,
                           gpointer   user_data)
{
  Toolbar *self = user_data;
  update_current_selected_tool (self, self->rectangle_button, RECTANGLE);
}

static void
on_circle_button_click (GtkButton *button,
                        gpointer   user_data)
{
  Toolbar *self = user_data;
  update_current_selected_tool (self, self->circle_button, CIRCLE);
}

static void
on_line_button_click (GtkButton *button,
                      gpointer   user_data)
{
  Toolbar *self = user_data;
  update_current_selected_tool (self, self->line_button, LINE);
}

static void
on_text_button_click (GtkButton *button,
                      gpointer   user_data)
{
  Toolbar *self = user_data;
  update_current_selected_tool (self, self->text_button, TEXT);
}

static void
on_fill_button_click (GtkButton *button,
                      gpointer   user_data)
{
  Toolbar *self = user_data;
  update_current_selected_tool (self, self->fill_button, FILL);
}

static void
on_color_picker_button_click (GtkButton *button,
                              gpointer   user_data)
{
  Toolbar *self = user_data;
  update_current_selected_tool (self, self->color_picker_button, COLOR_PICKER);
}


static void
on_select_button_click (GtkButton *button,
                        gpointer   user_data)
{
  Toolbar *self = user_data;
  update_current_selected_tool (self, self->select_button, SELECT);
}

static void
toolbar_init (Toolbar *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));

  self->currently_selected_button = self->brush_button;
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
                                               "/org/gnome/paint/ui/toolbar.ui");

  /* Widgets */
  gtk_widget_class_bind_template_child (widget_class, Toolbar, color_button);
  gtk_widget_class_bind_template_child (widget_class, Toolbar, drawing_size_spin_button);
  gtk_widget_class_bind_template_child (widget_class, Toolbar, brush_button);
  gtk_widget_class_bind_template_child (widget_class, Toolbar, eraser_button);
  gtk_widget_class_bind_template_child (widget_class, Toolbar, rectangle_button);
  gtk_widget_class_bind_template_child (widget_class, Toolbar, circle_button);
  gtk_widget_class_bind_template_child (widget_class, Toolbar, line_button);
  gtk_widget_class_bind_template_child (widget_class, Toolbar, text_button);
  gtk_widget_class_bind_template_child (widget_class, Toolbar, fill_button);
  gtk_widget_class_bind_template_child (widget_class, Toolbar, color_picker_button);
  gtk_widget_class_bind_template_child (widget_class, Toolbar, select_button);

  /* Callbacks */
  gtk_widget_class_bind_template_callback (widget_class, on_open_button_click);
  gtk_widget_class_bind_template_callback (widget_class, on_save_button_click);
  gtk_widget_class_bind_template_callback (widget_class, on_brush_button_click);
  gtk_widget_class_bind_template_callback (widget_class, on_eraser_button_click);
  gtk_widget_class_bind_template_callback (widget_class, on_rectangle_button_click);
  gtk_widget_class_bind_template_callback (widget_class, on_circle_button_click);
  gtk_widget_class_bind_template_callback (widget_class, on_line_button_click);
  gtk_widget_class_bind_template_callback (widget_class, on_text_button_click);
  gtk_widget_class_bind_template_callback (widget_class, on_fill_button_click);
  gtk_widget_class_bind_template_callback (widget_class, on_color_picker_button_click);
  gtk_widget_class_bind_template_callback (widget_class, on_select_button_click);

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

  /* Dispose */
  G_OBJECT_CLASS (klass)->dispose = toolbar_dispose;
}

const GdkRGBA *
toolbar_get_current_color (Toolbar *self)
{
  return gtk_color_dialog_button_get_rgba (self->color_button);
}

gdouble
toolbar_get_draw_size (Toolbar *self)
{
  return gtk_spin_button_get_value (self->drawing_size_spin_button);
}

void
toolbar_set_selected_color (Toolbar *self,
                            GdkRGBA *color)
{
  gtk_color_dialog_button_set_rgba (self->color_button, color);
}
