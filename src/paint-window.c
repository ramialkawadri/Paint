/* paint-window.c
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

#include "paint-window.h"
#include "toolbar.h"
#include "canvas-region.h"

#include "drawing_tools/drawing-tool-type.h"

struct _PaintWindow
{
  AdwApplicationWindow parent_instance;

  /* Widgets */
  AdwHeaderBar        *header_bar;
  Toolbar             *toolbar;
  CanvasRegion        *canvas_region;
  GtkLabel            *size_label;

  /* Metadata */
  gboolean             force_close;
};

G_DEFINE_FINAL_TYPE (PaintWindow, paint_window, ADW_TYPE_APPLICATION_WINDOW)

static void
on_undo (GtkButton *button,
         gpointer   user_data)
{
  PaintWindow *self = user_data;
  canvas_region_undo (self->canvas_region);
}

static void
on_redo (GtkButton *button,
         gpointer   user_data)
{
  PaintWindow *self = user_data;
  canvas_region_redo (self->canvas_region);
}

static void
on_file_dialog_save_finish (CanvasRegion *canvas_region)
{
  GtkRoot *root = gtk_widget_get_root (GTK_WIDGET (canvas_region));
  gtk_window_close (GTK_WINDOW (root));
}

static void
on_open_file_prompt_response (AdwMessageDialog *dialog,
                                 gchar            *response,
                                 gpointer          user_data)
{
  PaintWindow *self = user_data;

  if (g_strcmp0 (response, "cancel") == 0)
    return;

  self->force_close = true;

  if (g_strcmp0 (response, "discard") == 0)
    gtk_window_close (user_data);
  else if (g_strcmp0 (response, "save") == 0)
    canvas_region_save_current_file (self->canvas_region, on_file_dialog_save_finish);
}

static gboolean
on_close_request (GtkWindow *window,
                  gpointer   user_data)
{
  PaintWindow *self = user_data;

  if (canvas_region_is_current_file_saved (self->canvas_region) || self->force_close)
    return false;

  canvas_region_prompt_to_save_current_file (self->canvas_region,
                                             G_CALLBACK (on_open_file_prompt_response),
                                             self);

  return true;
}

static void
on_toolbar_file_open (Toolbar *toolbar,
                      gpointer user_data)
{
  PaintWindow *self = user_data;
  canvas_region_open_new_file (self->canvas_region);
}

static void
on_toolbar_file_save (Toolbar *toolbar,
                      gpointer user_data)
{
  PaintWindow *self = user_data;
  canvas_region_save_current_file (self->canvas_region, NULL);
}

static void
on_toolbar_tool_change (Toolbar          *toolbar,
                        DRAWING_TOOL_TYPE selected_tool,
                        gpointer          user_data)
{
  PaintWindow *self = user_data;
  canvas_region_set_selected_tool (self->canvas_region, selected_tool);
}

static const char *
get_window_title (gboolean is_file_saved,
                  char    *filename)
{
  g_autoptr (GFile) file;
  char *file_basename;

  file = NULL;

  if (filename == NULL)
    return "*Paint - Untitled";

  file = g_file_parse_name (filename);
  file_basename = g_file_get_basename (file);

  if (is_file_saved)
    return g_strdup_printf ("Paint - %s", file_basename);
  else
    return g_strdup_printf ("*Paint - %s", file_basename);
}

static void
on_canvas_region_file_save_status_change (CanvasRegion *canvas_region,
                                          gboolean      is_file_saved,
                                          gpointer      user_data)
{
  char *current_file_name = canvas_region_get_current_file_name (canvas_region);
  gtk_window_set_title (user_data, get_window_title (is_file_saved, current_file_name));
}

static void
on_canvas_region_color_picked (CanvasRegion *canvas_region,
                               GdkRGBA      *color,
                               gpointer      user_data)
{
  PaintWindow *self = user_data;
  toolbar_set_selected_color (self->toolbar, color);
}

static void
on_canvas_region_resize (CanvasRegion *canvas_region,
                         gint          width,
                         gint          height,
                         gpointer      user_data)
{
  PaintWindow *self;
  char *text;

  self = user_data;
  text = g_strdup_printf ("%d x %d", width, height);

  gtk_label_set_label (self->size_label, text);
}

void
paint_window_save_current_file (PaintWindow *self)
{
  canvas_region_save_current_file (self->canvas_region, NULL);
}

void
paint_window_open_new_file (PaintWindow *self)
{
  canvas_region_open_new_file (self->canvas_region);
}

void
paint_window_undo (PaintWindow *self)
{
  canvas_region_undo (self->canvas_region);
}

void
paint_window_redo (PaintWindow *self)
{
  canvas_region_redo (self->canvas_region);
}

static void
paint_window_class_init (PaintWindowClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class,
                                               "/org/gnome/paint/ui/paint-window.ui");

  /* Widgets */
  gtk_widget_class_bind_template_child (widget_class, PaintWindow, header_bar);
  gtk_widget_class_bind_template_child (widget_class, PaintWindow, toolbar);
  gtk_widget_class_bind_template_child (widget_class, PaintWindow, canvas_region);
  gtk_widget_class_bind_template_child (widget_class, PaintWindow, size_label);

  /* Callback */
  gtk_widget_class_bind_template_callback (widget_class, on_undo);
  gtk_widget_class_bind_template_callback (widget_class, on_redo);
  gtk_widget_class_bind_template_callback (widget_class, on_close_request);
  gtk_widget_class_bind_template_callback (widget_class, on_toolbar_file_open);
  gtk_widget_class_bind_template_callback (widget_class, on_toolbar_file_save);
  gtk_widget_class_bind_template_callback (widget_class, on_toolbar_tool_change);
  gtk_widget_class_bind_template_callback (widget_class, on_canvas_region_file_save_status_change);
  gtk_widget_class_bind_template_callback (widget_class, on_canvas_region_color_picked);
  gtk_widget_class_bind_template_callback (widget_class, on_canvas_region_resize);

  /* Types */
  g_type_ensure (PAINT_TYPE_CANVAS_REGION);
  g_type_ensure (PAINT_TYPE_TOOLBAR);
}

static void
paint_window_init (PaintWindow *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));
  canvas_region_set_toolbar (self->canvas_region, self->toolbar);
}
