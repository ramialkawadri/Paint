/* paint-window.c
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

#include "paint-window.h"
#include "toolbar.h"
#include "canvas-region.h"

struct _PaintWindow
{
  AdwApplicationWindow parent_instance;

  /* Template widgets */
  AdwHeaderBar        *header_bar;
  Toolbar             *toolbar;
  CanvasRegion        *canvas_region;
};

G_DEFINE_FINAL_TYPE (PaintWindow, paint_window, ADW_TYPE_APPLICATION_WINDOW)

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
  canvas_region_save_new_file (self->canvas_region);
}

static char *
get_window_title (gboolean is_file_saved,
                  char    *file_name)
{
  g_autoptr (GFile) file;
  char *file_name_to_show;

  if (file_name == NULL)
    return "*Paint - Untitled";
    
  file = g_file_parse_name (file_name);
  file_name_to_show = g_file_get_basename (file);

  if (is_file_saved)
    return g_strdup_printf ("Paint - %s", file_name_to_show);
  else
    return g_strdup_printf ("*Paint - %s", file_name_to_show);
}

static void
on_canvas_region_file_save_status_change (CanvasRegion *self,
                                          gboolean      is_file_saved,
                                          gpointer      user_data)
{
  char *current_file_name = canvas_region_get_current_file_name (self);
  gtk_window_set_title (user_data,
                        get_window_title (is_file_saved, current_file_name));
}

static void
paint_window_class_init (PaintWindowClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class,
                                               "/org/gnome/paint/ui/paint-window.ui");

  gtk_widget_class_bind_template_child (widget_class, PaintWindow, header_bar);
  gtk_widget_class_bind_template_child (widget_class, PaintWindow, toolbar);
  gtk_widget_class_bind_template_child (widget_class, PaintWindow, canvas_region);

  gtk_widget_class_bind_template_callback (widget_class, on_toolbar_file_open);
  gtk_widget_class_bind_template_callback (widget_class, on_toolbar_file_save);
  gtk_widget_class_bind_template_callback (widget_class, on_canvas_region_file_save_status_change);

  g_type_ensure (PAINT_TYPE_CANVAS_REGION);
  g_type_ensure (PAINT_TYPE_TOOLBAR);
}

static void
paint_window_init (PaintWindow *self)
{
  gtk_widget_init_template (GTK_WIDGET(self));

  canvas_region_set_toolbar (self->canvas_region, self->toolbar);
}
