/* canvas-region.c
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

#include "canvas-region.h"
#include "brush.h"

#include "toolbar.h"

struct _CanvasRegion
{
  /* Widgets */
  GtkBox parent_type;
  GtkDrawingArea *drawing_area;
  Toolbar *toolbar;

  /* Controllers */
  GtkGestureDrag *gesture_drag;
  GtkGestureClick *gesture_click;

  /* Draw */
  DrawEvent draw_event;
  cairo_surface_t *cairo_surface;

  /* Callbacks */
  on_draw_start_click draw_start_click_cb;
  on_draw draw_cb;

  /* Metadata */
  int width;
  int height;
  char *current_file_path;
  gboolean is_current_file_saved;
};

G_DEFINE_FINAL_TYPE (CanvasRegion, canvas_region, GTK_TYPE_BOX);

void
update_draw_event_from_toolbar (CanvasRegion *self)
{
  self->draw_event.draw_size = toolbar_get_draw_size (self->toolbar);
}

void
prompt_to_save_current_file (CanvasRegion *self)
{
  GtkWidget *dialog;
  GtkRoot *root = gtk_widget_get_root (GTK_WIDGET(self));

  dialog = adw_message_dialog_new (GTK_WINDOW(root),
                                   "Save changes?",
                                   "Your current file contains unsaved changes!");

  adw_message_dialog_add_responses (ADW_MESSAGE_DIALOG(dialog),
                                    "cancel", "_Cancel",
                                    "discard", "_Discard",
                                    "save", "_Save",
                                    NULL);

  adw_message_dialog_set_response_appearance (ADW_MESSAGE_DIALOG(dialog),
                                              "discard",
                                              ADW_RESPONSE_DESTRUCTIVE);

  adw_message_dialog_set_response_appearance (ADW_MESSAGE_DIALOG(dialog),
                                              "save",
                                              ADW_RESPONSE_SUGGESTED);

  gtk_window_present (GTK_WINDOW(dialog));

  // TODO: connect the response to open file function!
}

void
on_file_open (GObject      *source_object,
              GAsyncResult *res,
              gpointer      data)
{
  CanvasRegion *self = data;
  g_autoptr (GError) error = NULL;
  g_autoptr (GFile) file = gtk_file_dialog_open_finish(GTK_FILE_DIALOG(source_object),
                                                       res,
                                                       &error);

  if (error != NULL)
    {
      g_message ("Error opening file: %s", error->message);
      return;
    }

  if (!self->is_current_file_saved)
    {
      prompt_to_save_current_file (self);
      self->is_current_file_saved = true;
    }

  char *filename = g_file_get_path (file);
  GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file (filename, &error);

  int width = gdk_pixbuf_get_width (pixbuf);
  int height = gdk_pixbuf_get_height (pixbuf);

  cairo_format_t format = gdk_pixbuf_get_has_alpha (pixbuf) ?
                            CAIRO_FORMAT_ARGB32 : CAIRO_FORMAT_RGB30;

  int stride = cairo_format_stride_for_width (format, width);

  cairo_surface_destroy (self->cairo_surface);

  guchar *pixels = gdk_pixbuf_get_pixels (pixbuf);

  self->width = width;
  self->height = height;
  self->current_file_path = filename;

  gtk_drawing_area_set_content_width (self->drawing_area, width);
  gtk_drawing_area_set_content_height (self->drawing_area, height);

  self->cairo_surface = cairo_image_surface_create_for_data (pixels,
                                                             format,
                                                             width,
                                                             height,
                                                             stride);

  gtk_widget_queue_draw (GTK_WIDGET(self->drawing_area));
}

void
on_file_save (GObject      *source_object,
              GAsyncResult *res,
              gpointer      data)
{
  CanvasRegion *self = data;
  g_autoptr (GError) error = NULL;
  g_autoptr (GFile) file = gtk_file_dialog_save_finish(GTK_FILE_DIALOG(source_object),
                                                       res,
                                                       &error);

  if (error != NULL)
    {
      g_message ("Error saving file: %s", error->message);
      return;
    }

  char *filename = g_file_get_path (file);

  unsigned char *pixels = cairo_image_surface_get_data (self->cairo_surface);

  int stride = cairo_image_surface_get_stride (self->cairo_surface);

  GdkPixbuf *pixbuf = gdk_pixbuf_new_from_data (pixels,
                                                GDK_COLORSPACE_RGB,
                                                1,
                                                8,
                                                self->width,
                                                self->height,
                                                stride,
                                                NULL, NULL);

  gboolean is_saved = gdk_pixbuf_save (pixbuf, filename, "png", NULL, NULL);

  self->is_current_file_saved = is_saved;
}

static void
draw_function (GtkDrawingArea *area,
               cairo_t        *cr,
               int             width,
               int             height,
               gpointer        user_data)
{
  CanvasRegion *self = user_data;
  cairo_set_source_surface (cr, self->cairo_surface, 0, 0);
  cairo_paint (cr);
}

static void
make_surface_white (cairo_surface_t *cairo_surface)
{
  cairo_t *cr = cairo_create (cairo_surface);

  cairo_set_source_rgb (cr, 1.0, 1.0, 1.0);
  cairo_paint (cr);
  cairo_destroy (cr);
}

static void
on_mouse_press (GtkGestureClick *gesture,
                gint             n_press,
                gdouble          x,
                gdouble          y,
                gpointer         user_data)
{
  CanvasRegion *self = user_data;
  cairo_t *cr;

  if (self->draw_start_click_cb == NULL)
    {
      return;
    }

  cr = cairo_create (self->cairo_surface);

  self->draw_event.current_x = x;
  self->draw_event.current_y = y;

  gdk_cairo_set_source_rgba (cr, toolbar_get_current_color(self->toolbar));
  update_draw_event_from_toolbar (self);

  self->draw_start_click_cb (self, cr, &self->draw_event);

  self->draw_event.last_x = self->draw_event.current_x;
  self->draw_event.last_y = self->draw_event.current_y;

  cairo_destroy (cr);

  gtk_widget_queue_draw (GTK_WIDGET(self->drawing_area));
}

static void
on_gesture_drag_begin (GtkGestureDrag *gesture,
                       gdouble         start_x,
                       gdouble         start_y,
                       gpointer        user_data)
{
  CanvasRegion *self = user_data;

  self->draw_event.start_x = start_x;
  self->draw_event.start_y = start_y;

  self->draw_event.last_x = -1;
  self->draw_event.last_y = -1;
}

static void
on_gesture_drag_update (GtkGestureDrag *gesture,
                        gdouble         offset_x,
                        gdouble         offset_y,
                        gpointer        user_data)
{
  CanvasRegion *self = user_data;
  cairo_t *cr = cairo_create (self->cairo_surface);

  self->draw_event.offset_x = offset_x;
  self->draw_event.offset_y = offset_y;

  self->draw_event.current_x = self->draw_event.start_x + offset_x;
  self->draw_event.current_y = self->draw_event.start_y + offset_y;

  gdk_cairo_set_source_rgba (cr, toolbar_get_current_color(self->toolbar));
  update_draw_event_from_toolbar (self);

  self->draw_cb (self, cr, &self->draw_event);

  self->draw_event.last_x = self->draw_event.current_x;
  self->draw_event.last_y = self->draw_event.current_y;

  cairo_destroy (cr);

  gtk_widget_queue_draw (GTK_WIDGET(self->drawing_area));
}

static void
on_gesture_drag_end (GtkGestureDrag *gesture,
                     gdouble         offset_x,
                     gdouble         offset_y,
                     gpointer        user_data)
{
}

void
canvas_region_set_toolbar (CanvasRegion *self,
                           Toolbar      *toolbar)
{
  self->toolbar = toolbar;
}

GAsyncReadyCallback
canvas_region_get_file_open_callback (CanvasRegion *self)
{
  return on_file_open;
}

GAsyncReadyCallback
canvas_region_get_file_save_callback (CanvasRegion *self)
{
  return on_file_save;
}

static void
canvas_region_init (CanvasRegion *self)
{
  gtk_widget_init_template (GTK_WIDGET(self));

  self->width = gtk_drawing_area_get_content_width (self->drawing_area);
  self->height = gtk_drawing_area_get_content_height (self->drawing_area);

  self->cairo_surface = cairo_image_surface_create (CAIRO_FORMAT_RGB24,
                                                    self->width,
                                                    self->height);

  self->is_current_file_saved = true;

  self->draw_start_click_cb = &on_brush_draw_start_click;
  self->draw_cb = &on_brush_draw;

  make_surface_white (self->cairo_surface);

  gtk_drawing_area_set_draw_func (self->drawing_area, draw_function, self, NULL);
}

static void
canvas_region_dispose (GObject *gobject)
{
  CanvasRegion *self = (CanvasRegion *) gobject;
  cairo_surface_destroy (self->cairo_surface);

  gtk_widget_dispose_template (GTK_WIDGET(gobject), PAINT_TYPE_CANVAS_REGION);

  G_OBJECT_CLASS (canvas_region_parent_class)->dispose(gobject);
}

static void
canvas_region_class_init (CanvasRegionClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class,
                                               "/org/gnome/paint/canvas-region.ui");

  gtk_widget_class_bind_template_child (widget_class, CanvasRegion, drawing_area);
  gtk_widget_class_bind_template_child (widget_class, CanvasRegion, gesture_drag);
  gtk_widget_class_bind_template_child (widget_class, CanvasRegion, gesture_click);

  /* Callbacks */
  gtk_widget_class_bind_template_callback (widget_class, on_mouse_press);

  gtk_widget_class_bind_template_callback (widget_class, on_gesture_drag_begin);
  gtk_widget_class_bind_template_callback (widget_class, on_gesture_drag_update);
  gtk_widget_class_bind_template_callback (widget_class, on_gesture_drag_end);

  G_OBJECT_CLASS (klass)->dispose = canvas_region_dispose;
}
