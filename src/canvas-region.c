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

typedef struct _CanvasRegionUserData {
  CanvasRegion *self;
  gpointer      user_data;
} CanvasRegionUserData;

struct _CanvasRegion
{
  /* Widgets */
  GtkBox              parent_type;
  GtkDrawingArea     *drawing_area;
  Toolbar            *toolbar;

  /* Controllers */
  GtkGestureDrag     *gesture_drag;
  GtkGestureClick    *gesture_click;

  /* Draw */
  DrawEvent           draw_event;
  cairo_surface_t    *cairo_surface;

  /* Callbacks */
  on_draw_start_click draw_start_click_cb;
  on_draw             draw_cb;

  /* Metadata */
  int                 width;
  int                 height;
  char               *current_filename;
  gboolean            is_current_file_saved;
};

enum {
  ON_FILE_SAVE_STATUS_CHANGE,
  NUMBER_OF_SIGNALS
};

static guint canvas_region_signals [NUMBER_OF_SIGNALS];

G_DEFINE_FINAL_TYPE (CanvasRegion, canvas_region, GTK_TYPE_BOX);

static void
show_save_file_dialog (CanvasRegion       *self,
                       GAsyncReadyCallback cb,
                       CanvasRegionUserData *user_data);

static void
update_draw_event_from_toolbar (CanvasRegion *self)
{
  self->draw_event.draw_size = toolbar_get_draw_size (self->toolbar);
}

static void
set_is_current_file_saved (CanvasRegion *self,
                           gboolean      is_current_file_saved)
{
  self->is_current_file_saved = is_current_file_saved;
  g_signal_emit (self,
                 canvas_region_signals[ON_FILE_SAVE_STATUS_CHANGE],
                 0, is_current_file_saved);
}

static void
save_canvas (CanvasRegion *self)
{
  if (self->is_current_file_saved)
    return;

  cairo_surface_write_to_png (self->cairo_surface, self->current_filename);
  set_is_current_file_saved (self, true);
}

static void
load_file (CanvasRegion *self,
           char         *filename)
{
  cairo_surface_destroy (self->cairo_surface);
  self->cairo_surface = cairo_image_surface_create_from_png (filename);

  self->width = cairo_image_surface_get_width (self->cairo_surface);
  self->height = cairo_image_surface_get_height (self->cairo_surface);
  self->current_filename = filename;
  set_is_current_file_saved (self, true);

  gtk_drawing_area_set_content_width (self->drawing_area, self->width);
  gtk_drawing_area_set_content_height (self->drawing_area, self->height);

  gtk_widget_queue_draw (GTK_WIDGET(self->drawing_area));
}

void
canvas_region_prompt_to_save_current_file (CanvasRegion *self,
                                           GCallback     on_response,
                                           gpointer      user_data)
{
  GtkWidget *dialog;
  GtkRoot *root;

  root = gtk_widget_get_root (GTK_WIDGET(self));

  dialog = adw_message_dialog_new (GTK_WINDOW(root),
                                   "Save changes?",
                                   "Your current file contains unsaved changes!");

  adw_message_dialog_add_responses (ADW_MESSAGE_DIALOG(dialog),
                                    "cancel", "Cancel",
                                    "discard", "Discard",
                                    "save", "Save",
                                    NULL);

  adw_message_dialog_set_response_appearance (ADW_MESSAGE_DIALOG(dialog),
                                              "discard",
                                              ADW_RESPONSE_DESTRUCTIVE);

  adw_message_dialog_set_response_appearance (ADW_MESSAGE_DIALOG(dialog),
                                              "save",
                                              ADW_RESPONSE_SUGGESTED);

  g_signal_connect (dialog, "response", on_response, user_data);

  gtk_window_present (GTK_WINDOW(dialog));
}

static void
save_canvas_from_file_dialog_save_result (CanvasRegion *self,
                                          GObject      *source_object,
                                          GAsyncResult *res)
{
  g_autoptr (GError) error;
  g_autoptr (GFile) file;
  char *filepath;

  error = NULL;
  file = gtk_file_dialog_save_finish (GTK_FILE_DIALOG(source_object), res, &error);

  if (error != NULL)
    {
      g_message ("Error saving file: %s", error->message);
      return;
    }

  filepath = g_file_get_path (file);
  self->current_filename = filepath;
  save_canvas (self);
}

static void
prompt_to_save_on_file_dialog_save_finish (GObject      *source_object,
                                           GAsyncResult *res,
                                           gpointer      data)
{
  CanvasRegionUserData *cb_data;
  cb_data = data;

  save_canvas_from_file_dialog_save_result (cb_data->self,
                                            source_object,
                                            res);

  load_file (cb_data->self, cb_data->user_data);
  g_free (cb_data);
}

static void
on_prompt_to_save_response_file_open (AdwMessageDialog *dialog,
                                      gchar            *response,
                                      gpointer          user_data)
{
  CanvasRegionUserData *cb_data = user_data;

  if (g_strcmp0 (response, "cancel") == 0)
    {
      g_free (cb_data);
      return;
    }
  else if (g_strcmp0 (response, "save") == 0)
    {
      if (cb_data->self->current_filename == NULL)
        {
          show_save_file_dialog (cb_data->self,
                                 prompt_to_save_on_file_dialog_save_finish,
                                 cb_data);
          return;
        }
      else
        {
          save_canvas (cb_data->self);
        }
    }

  load_file (cb_data->self, cb_data->user_data);
  g_free (cb_data);
}

static void
on_file_dialog_open_finish (GObject      *source_object,
                            GAsyncResult *res,
                            gpointer      data)
{
  CanvasRegion *self;
  CanvasRegionUserData *cb_data;
  g_autoptr (GError) error;
  g_autoptr (GFile) file;
  char *filename;

  self = data;
  error = NULL;
  file = gtk_file_dialog_open_finish(GTK_FILE_DIALOG(source_object), res, &error);

  if (error != NULL)
    {
      g_message ("Error opening file: %s", error->message);
      return;
    }

  filename = g_file_get_path (file);

  if (!self->is_current_file_saved)
    {
      cb_data = g_malloc (sizeof (CanvasRegionUserData));
      cb_data->self = self;
      cb_data->user_data = filename;

      canvas_region_prompt_to_save_current_file (self,
                                                G_CALLBACK (on_prompt_to_save_response_file_open),
                                                cb_data);
    }
  else
    {
      load_file (self, filename);
    }
}

static void
on_file_dialog_save_finish (GObject      *source_object,
                            GAsyncResult *res,
                            gpointer      data)
{
  CanvasRegionUserData *cb_data;
  on_file_save_finish finish_cb;

  cb_data = data;

  save_canvas_from_file_dialog_save_result (cb_data->self,
                                            source_object,
                                            res);

  if (cb_data->user_data != NULL)
    {
      finish_cb = cb_data->user_data;
      finish_cb (cb_data->self);
    }

  g_free (cb_data);
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
  CanvasRegion *self;
  cairo_t *cr;

  self = user_data;

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
  CanvasRegion *self;
  cairo_t *cr;

  self = user_data;
  cr = cairo_create (self->cairo_surface);

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
  CanvasRegion *self = user_data;

  set_is_current_file_saved (self, false);
}

void
canvas_region_set_toolbar (CanvasRegion *self,
                           Toolbar      *toolbar)
{
  self->toolbar = toolbar;
}

char *
canvas_region_get_current_file_name (CanvasRegion *self)
{
  return self->current_filename;
}

gboolean
canvas_region_is_current_file_saved (CanvasRegion *self)
{
  return self->is_current_file_saved;
}

void
canvas_region_open_new_file (CanvasRegion *self)
{
  GtkRoot *root;

  g_autoptr (GtkFileDialog) file_dialog;
  g_autoptr (GtkFileFilter) file_filter;

  root = gtk_widget_get_root (GTK_WIDGET (self));

  file_dialog = gtk_file_dialog_new ();
  file_filter = gtk_file_filter_new ();

  gtk_file_filter_add_suffix (file_filter, "png");
  gtk_file_dialog_set_default_filter (file_dialog, file_filter);

  gtk_file_dialog_open (file_dialog,
                        GTK_WINDOW (root),
                        NULL,
                        on_file_dialog_open_finish,
                        self);
}

static void
show_save_file_dialog (CanvasRegion         *self,
                       GAsyncReadyCallback   cb,
                       CanvasRegionUserData *user_data)
{
  GtkRoot *root;

  g_autoptr (GtkFileDialog) file_dialog;
  g_autoptr (GtkFileFilter) file_filter;

  root = gtk_widget_get_root (GTK_WIDGET(self));

  file_dialog = gtk_file_dialog_new ();
  file_filter = gtk_file_filter_new ();

  gtk_file_filter_add_suffix (file_filter, "png");
  gtk_file_dialog_set_default_filter (file_dialog, file_filter);
  gtk_file_dialog_set_initial_name (file_dialog, "untitled.png");

  gtk_file_dialog_save (file_dialog,
                        GTK_WINDOW (root),
                        NULL,
                        cb,
                        user_data);
}

void
canvas_region_save_file (CanvasRegion           *self,
                         on_file_save_finish     on_save_finish)
{
  if (self->current_filename != NULL)
    {
      save_canvas (self);

      if (on_save_finish != NULL)
        on_save_finish (self);
    }
  else
    {
      CanvasRegionUserData *cb_data = g_malloc (sizeof (CanvasRegionUserData));
      cb_data->self = self;
      cb_data->user_data = on_save_finish;
      show_save_file_dialog (self, on_file_dialog_save_finish, cb_data);
    }
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

  set_is_current_file_saved (self, true);

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
                                               "/org/gnome/paint/ui/canvas-region.ui");

  gtk_widget_class_bind_template_child (widget_class, CanvasRegion, drawing_area);
  gtk_widget_class_bind_template_child (widget_class, CanvasRegion, gesture_drag);
  gtk_widget_class_bind_template_child (widget_class, CanvasRegion, gesture_click);

  /* Callbacks */
  gtk_widget_class_bind_template_callback (widget_class, on_mouse_press);

  gtk_widget_class_bind_template_callback (widget_class, on_gesture_drag_begin);
  gtk_widget_class_bind_template_callback (widget_class, on_gesture_drag_update);
  gtk_widget_class_bind_template_callback (widget_class, on_gesture_drag_end);

  G_OBJECT_CLASS (klass)->dispose = canvas_region_dispose;

  canvas_region_signals[ON_FILE_SAVE_STATUS_CHANGE] = g_signal_new ("on-file-save-status-change",
                                                                    G_TYPE_FROM_CLASS (klass),
                                                                    G_SIGNAL_RUN_LAST,
                                                                    0,
                                                                    NULL,
                                                                    NULL,
                                                                    NULL,
                                                                    G_TYPE_NONE,
                                                                    1,
                                                                    G_TYPE_BOOLEAN);
}
