/* canvas-region.c
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

#include <adwaita.h>

#include "canvas-region.h"
#include "toolbar.h"

#include "drawing-tools/brush.h"
#include "drawing-tools/circle.h"
#include "drawing-tools/color-picker.h"
#include "drawing-tools/fill.h"
#include "drawing-tools/line.h"
#include "drawing-tools/rectangle.h"
#include "drawing-tools/text.h"

#include "utils/canvas-region-caretaker.h"
#include "utils/cairo-utils.h"
#include "utils/colors.h"

typedef struct _CanvasRegionUserData {
  CanvasRegion *self;
  gpointer      user_data;
} CanvasRegionUserData;

struct _CanvasRegion
{
  /* Widgets */
  GtkGrid                parent_type;
  GtkDrawingArea        *drawing_area;
  Toolbar               *toolbar;
  GtkPopover            *text_popover;
  GtkEntry              *text_popover_text_entry;
  AdwBin                *resize_corner;

  /* Draw */
  DrawEvent              draw_event;
  cairo_surface_t       *cairo_surface;
  cairo_surface_t       *cairo_surface_save;
  DRAWING_TOOL_TYPE      current_tool_type;

  /* Callbacks */
  on_draw_start_click    draw_start_click_cb;
  on_draw                draw_cb;

  /* Metadata */
  gint                   width;
  gint                   height;
  gchar                 *current_filename;
  gboolean               is_current_file_saved;
  gboolean               save_while_drawing;
  CanvasRegionCaretaker *caretaker;
};

enum {
  SAVE_STATUS_CHANGE,
  RESIZE,
  COLOR_PICKED,
  NUMBER_OF_SIGNALS
};

static guint canvas_region_signals [NUMBER_OF_SIGNALS];

G_DEFINE_FINAL_TYPE (CanvasRegion, canvas_region, GTK_TYPE_GRID);

static void
show_save_file_dialog (CanvasRegion       *self,
                       GAsyncReadyCallback cb,
                       CanvasRegionUserData *user_data);

static void
create_and_save_snapshot (CanvasRegion *self)
{
  CanvasRegionSnapshot *snapshot;
  cairo_surface_t *copy_surface;
  copy_surface = cairo_clone_surface (self->cairo_surface_save);

  snapshot = canvas_region_snapshot_new (self->width,
                                         self->height,
                                         self->is_current_file_saved,
                                         copy_surface);
  canvas_region_caretaker_save_snapshot (self->caretaker, snapshot);
}

static void
save_and_destroy_current_surface (CanvasRegion *self)
{
   if (self->cairo_surface != NULL)
    {
      cairo_surface_destroy (self->cairo_surface_save);
      self->cairo_surface_save = cairo_clone_surface (self->cairo_surface);
      self->width = cairo_image_surface_get_width (self->cairo_surface);
      self->height = cairo_image_surface_get_height (self->cairo_surface);
      cairo_surface_destroy (self->cairo_surface);
      self->cairo_surface = NULL;
    }

  create_and_save_snapshot (self);
}

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
                 canvas_region_signals[SAVE_STATUS_CHANGE],
                 0,
                 is_current_file_saved);
}

static void
save_to_current_file (CanvasRegion *self)
{
  if (self->is_current_file_saved)
    return;

  cairo_surface_write_to_png (self->cairo_surface_save, self->current_filename);
  set_is_current_file_saved (self, true);
  canvas_region_mark_current_snapshot_as_saved (self->caretaker);
}

static void
update_drawing_area_size (CanvasRegion *self,
                          gint          width,
                          gint          height)
{
  self->width = width;
  self->height = height;

  gtk_drawing_area_set_content_width (self->drawing_area, width);
  gtk_drawing_area_set_content_height (self->drawing_area, height);

  g_signal_emit (self,
                 canvas_region_signals[RESIZE],
                 0,
                 width,
                 height);
}

static void
open_file (CanvasRegion *self,
           gchar        *filename)
{
  gint width;
  gint height;

  cairo_surface_destroy (self->cairo_surface_save);
  self->cairo_surface_save = cairo_image_surface_create_from_png (filename);

  width = cairo_image_surface_get_width (self->cairo_surface_save);
  height = cairo_image_surface_get_height (self->cairo_surface_save);
  update_drawing_area_size (self, width, height);

  self->current_filename = filename;

  set_is_current_file_saved (self, true);

  canvas_region_caretaker_dispose (self->caretaker);
  self->caretaker = canvas_region_caretaker_new ();
  create_and_save_snapshot (self);

  gtk_widget_queue_draw (GTK_WIDGET (self->drawing_area));
}

void
canvas_region_prompt_to_save_current_file (CanvasRegion *self,
                                           GCallback     on_response,
                                           gpointer      user_data)
{
  GtkWidget *dialog;
  GtkRoot *root;

  root = gtk_widget_get_root (GTK_WIDGET (self));

  dialog = adw_message_dialog_new (GTK_WINDOW (root),
                                   "Save changes?",
                                   "Your current file contains unsaved changes!");

  adw_message_dialog_add_responses (ADW_MESSAGE_DIALOG (dialog),
                                    "cancel", "Cancel",
                                    "discard", "Discard",
                                    "save", "Save",
                                    NULL);

  adw_message_dialog_set_response_appearance (ADW_MESSAGE_DIALOG (dialog),
                                              "discard",
                                              ADW_RESPONSE_DESTRUCTIVE);

  adw_message_dialog_set_response_appearance (ADW_MESSAGE_DIALOG (dialog),
                                              "save",
                                              ADW_RESPONSE_SUGGESTED);

  g_signal_connect (dialog, "response", on_response, user_data);

  gtk_window_present (GTK_WINDOW (dialog));
}

static void
save_canvas_from_file_dialog_save_result (CanvasRegion *self,
                                          GObject      *source_object,
                                          GAsyncResult *res)
{
  g_autoptr (GError) error;
  g_autoptr (GFile) file;
  gchar *filepath;

  error = NULL;
  file = gtk_file_dialog_save_finish (GTK_FILE_DIALOG (source_object), res, &error);

  if (error != NULL)
    {
      g_message ("Error saving file: %s", error->message);
      return;
    }

  filepath = g_file_get_path (file);
  self->current_filename = filepath;
  save_to_current_file (self);
}

static void
on_file_dialog_save_finish_save_and_open_file (GObject      *source_object,
                                               GAsyncResult *res,
                                               gpointer      user_data)
{
  CanvasRegionUserData *cb_data = user_data;

  save_canvas_from_file_dialog_save_result (cb_data->self, source_object, res);
  open_file (cb_data->self, cb_data->user_data);
  g_free (cb_data);
}

static void
on_open_file_prompt_response (AdwMessageDialog *dialog,
                              gchar            *response,
                              gpointer          user_data)
{
  CanvasRegionUserData *cb_data;
  CanvasRegion *self;

  cb_data = user_data;
  self = cb_data->self;

  if (g_strcmp0 (response, "cancel") == 0)
    {
      g_free (cb_data);
      return;
    }
  else if (g_strcmp0 (response, "save") == 0)
    {
      if (self->current_filename == NULL)
        {
          show_save_file_dialog (self, on_file_dialog_save_finish_save_and_open_file, cb_data);
          return;
        }
      else
        {
          save_to_current_file (self);
        }
    }

  open_file (self, cb_data->user_data);
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
  gchar *filename;

  self = data;
  error = NULL;
  file = gtk_file_dialog_open_finish (GTK_FILE_DIALOG (source_object), res, &error);

  if (error != NULL)
    {
      g_message ("Error opening file: %s", error->message);
      return;
    }

  filename = g_file_get_path (file);

  if (self->is_current_file_saved)
    {
      open_file (self, filename);
    }
  else
    {
      cb_data = g_malloc (sizeof (CanvasRegionUserData));
      cb_data->self = self;
      cb_data->user_data = filename;

      canvas_region_prompt_to_save_current_file (self,
                                                 G_CALLBACK (on_open_file_prompt_response),
                                                 cb_data);
    }
}

static void
on_file_dialog_save_finish (GObject      *source_object,
                            GAsyncResult *res,
                            gpointer      data)
{
  CanvasRegionUserData *cb_data;
  on_save_finish finish_cb;

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
drawing_area_draw_function (GtkDrawingArea *area,
                            cairo_t        *cr,
                            gint            width,
                            gint            height,
                            gpointer        user_data)
{
  CanvasRegion *self = user_data;

  if (self->cairo_surface)
    cairo_set_source_surface (cr, self->cairo_surface, 0, 0);
  else
    cairo_set_source_surface (cr, self->cairo_surface_save, 0, 0);

  cairo_paint (cr);
}

static void
drawing_area_on_mouse_press (GtkGestureClick *gesture,
                             gint             n_press,
                             gdouble          x,
                             gdouble          y,
                             gpointer         user_data)
{
  CanvasRegion *self;
  cairo_t *cr;

  self = user_data;

  if (self->draw_start_click_cb == NULL)
    return;

  cr = cairo_create (self->cairo_surface_save);

  if (self->current_tool_type == ERASER)
    gdk_cairo_set_source_rgba (cr, &WHITE_COLOR);
  else
    gdk_cairo_set_source_rgba (cr, toolbar_get_current_color (self->toolbar));

  update_draw_event_from_toolbar (self);

  self->draw_event.current.x = x;
  self->draw_event.current.y = y;

  self->draw_start_click_cb (self, cr, &self->draw_event);

  self->draw_event.last.x = self->draw_event.current.x;
  self->draw_event.last.y = self->draw_event.current.y;

  cairo_destroy (cr);

  gtk_widget_queue_draw (GTK_WIDGET (self->drawing_area));
}

static void
on_gesture_drag_begin (GtkGestureDrag *gesture,
                       gdouble         start_x,
                       gdouble         start_y,
                       gpointer        user_data)
{
  CanvasRegion *self = user_data;

  // Initializing draw event
  self->draw_event.start.x = start_x;
  self->draw_event.start.y = start_y;

  self->draw_event.offset.x = 0;
  self->draw_event.offset.y = 0;

  self->draw_event.last.x = start_x;
  self->draw_event.last.y = start_y;

  self->draw_event.current.x = start_x;
  self->draw_event.current.y = start_y;
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

  if (self->draw_cb == NULL)
    return;

  if (self->save_while_drawing && self->cairo_surface != NULL)
    {
      cairo_surface_destroy (self->cairo_surface_save);
      self->cairo_surface_save = cairo_clone_surface (self->cairo_surface);
    }

  if (self->cairo_surface != NULL)
    cairo_surface_destroy (self->cairo_surface);

  self->cairo_surface = cairo_clone_surface (self->cairo_surface_save);
  cr = cairo_create (self->cairo_surface);

  if (self->current_tool_type == ERASER)
    gdk_cairo_set_source_rgba (cr, &WHITE_COLOR);
  else
    gdk_cairo_set_source_rgba (cr, toolbar_get_current_color (self->toolbar));

  update_draw_event_from_toolbar (self);
  self->draw_event.offset.x = offset_x;
  self->draw_event.offset.y = offset_y;

  self->draw_event.current.x = self->draw_event.start.x + offset_x;
  self->draw_event.current.y = self->draw_event.start.y + offset_y;

  self->draw_cb (self, cr, &self->draw_event);

  self->draw_event.last.x = self->draw_event.current.x;
  self->draw_event.last.y = self->draw_event.current.y;

  cairo_destroy (cr);
  gtk_widget_queue_draw (GTK_WIDGET (self->drawing_area));
}

static void
on_gesture_drag_end (GtkGestureDrag *gesture,
                     gdouble         offset_x,
                     gdouble         offset_y,
                     gpointer        user_data)
{
  CanvasRegion *self = user_data;

  // Nothing changed
  if (self->current_tool_type == COLOR_PICKER)
    return;

  set_is_current_file_saved (self, false);
  save_and_destroy_current_surface (self);
}

static void
on_resize_corner_drag_update (GtkGestureDrag *gesture,
                              gdouble        offset_x,
                              gdouble        offset_y,
                              gpointer       user_data)
{
  CanvasRegion *self;
  cairo_format_t format;
  cairo_t *cr;
  gint width;
  gint height;

  self = user_data;
  format = cairo_image_surface_get_format (self->cairo_surface_save);
  width = MAX (self->width + offset_x, 1);
  height = MAX (self->height + offset_y, 1);

  if (self->cairo_surface)
    cairo_surface_destroy (self->cairo_surface);

  update_drawing_area_size (self, width, height);

  self->cairo_surface = cairo_image_surface_create (format, width, height);

  cr = cairo_create (self->cairo_surface);

  cairo_set_source_rgb (cr, 1, 1, 1);
  cairo_paint (cr);

  cairo_set_source_surface (cr, self->cairo_surface_save, 0.0, 0.0);
  cairo_paint (cr);

  cairo_destroy (cr);

  gtk_widget_queue_draw (GTK_WIDGET (self->drawing_area));
}

static void
on_resize_corner_drag_end (GtkGestureDrag *gesture,
                           gdouble         offset_x,
                           gdouble         offset_y,
                           gpointer        user_data)
{
  CanvasRegion *self = user_data;
  set_is_current_file_saved (self, false);
  save_and_destroy_current_surface (self);
}

static void
on_text_popover_close (GtkPopover *popover,
                       gpointer    user_data)
{
  CanvasRegion *self;
  GtkEntryBuffer *buffer;

  self = user_data;
  buffer = gtk_entry_get_buffer (self->text_popover_text_entry);

  cairo_surface_destroy (self->cairo_surface);
  self->cairo_surface = NULL;
  gtk_entry_buffer_set_text (buffer, "", 0);
  gtk_widget_queue_draw (GTK_WIDGET (self->drawing_area));
}

static void
on_text_popover_text_change (GtkEditable *editable,
                             gpointer     user_data)
{
  CanvasRegion *self;
  cairo_t *cr;

  self = user_data;

  if (!gtk_widget_get_visible (GTK_WIDGET (self->text_popover)))
    return;

  if (self->cairo_surface)
    cairo_surface_destroy (self->cairo_surface);

  self->cairo_surface = cairo_clone_surface (self->cairo_surface_save);

  cr = cairo_create (self->cairo_surface);
  gdk_cairo_set_source_rgba (cr, toolbar_get_current_color (self->toolbar));

  self->draw_cb (self, cr, &self->draw_event);

  cairo_destroy (cr);

  gtk_widget_queue_draw (GTK_WIDGET (self->drawing_area));
}

static void
on_text_popover_activate (GtkEntry     *entry,
                          CanvasRegion *self)
{
  save_and_destroy_current_surface (self);
  gtk_popover_popdown (self->text_popover);
}

static void
on_text_popover_submit (GtkButton    *button,
                        CanvasRegion *self)
{
  save_and_destroy_current_surface (self);
  gtk_popover_popdown (self->text_popover);
}

void
canvas_region_set_toolbar (CanvasRegion *self,
                           Toolbar      *toolbar)
{
  self->toolbar = toolbar;
}

gchar *
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

  root = gtk_widget_get_root (GTK_WIDGET (self));

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
canvas_region_save (CanvasRegion           *self,
                    on_save_finish          save_finish_cb)
{
  CanvasRegionUserData *cb_data;

  if (self->current_filename != NULL)
    {
      save_to_current_file (self);

      if (save_finish_cb != NULL)
        save_finish_cb (self);
    }
  else
    {
      cb_data = g_malloc (sizeof (CanvasRegionUserData));
      cb_data->self = self;
      cb_data->user_data = save_finish_cb;
      show_save_file_dialog (self, on_file_dialog_save_finish, cb_data);
    }
}

void
canvas_region_set_selected_tool (CanvasRegion      *self,
                                 DRAWING_TOOL_TYPE  tool)
{
  self->current_tool_type = tool;

  switch (tool)
    {
    // An eraser uses a brush but with white color, this is known by current_tool_type
    case BRUSH:
    case ERASER:
      self->save_while_drawing = true;
      self->draw_start_click_cb = &on_brush_draw_start_click;
      self->draw_cb = &on_brush_draw;
      break;

    case RECTANGLE:
      self->save_while_drawing = false;
      self->draw_start_click_cb = NULL;
      self->draw_cb = &on_rectangle_draw;
      break;

    case CIRCLE:
      self->save_while_drawing = false;
      self->draw_start_click_cb = NULL;
      self->draw_cb = &on_circle_draw;
      break;

    case LINE:
      self->save_while_drawing = false;
      self->draw_start_click_cb = NULL;
      self->draw_cb = &on_line_draw;
      break;
    
    case TEXT:
      self->save_while_drawing = false;
      self->draw_start_click_cb = &on_text_draw_start_click;
      self->draw_cb = &on_text_draw;
      break;

    case COLOR_PICKER:
      self->save_while_drawing = false;
      self->draw_start_click_cb = &on_color_picker_draw_start_click;
      self->draw_cb = NULL;
      break;

    case FILL:
      self->save_while_drawing = false;
      self->draw_start_click_cb = &on_fill_draw_start_click;
      self->draw_cb = NULL;
      break;

    default:
      g_message ("Unknown tool %d", tool);
      break;
  }
}

void
canvas_region_show_text_popover (CanvasRegion *self,
                                 GdkRectangle *pointing_to)
{
  gtk_popover_set_pointing_to (self->text_popover, pointing_to);
  gtk_popover_popup (self->text_popover);
}

gchar *
canvas_region_get_text_popover_text (CanvasRegion *self)
{
  GtkEntryBuffer *buffer;
  gchar *text;

  buffer = gtk_entry_get_buffer (self->text_popover_text_entry);
  g_object_get (G_OBJECT (buffer), "text", &text, NULL);

  return text;
}

static void
restore_from_snapshot (CanvasRegion         *self,
                       CanvasRegionSnapshot *snapshot)
{
  update_drawing_area_size (self, snapshot->width, snapshot->height);

  if (self->cairo_surface != NULL)
    {
      cairo_surface_destroy (self->cairo_surface);
      self->cairo_surface = NULL;
    }

  cairo_surface_destroy (self->cairo_surface_save);
  
  self->cairo_surface_save = cairo_clone_surface (snapshot->surface);

  set_is_current_file_saved (self, snapshot->is_current_file_saved);

  gtk_widget_queue_draw (GTK_WIDGET (self->drawing_area));
}

void
canvas_region_undo (CanvasRegion *self)
{
  CanvasRegionSnapshot *snapshot;

  snapshot = canvas_region_caretaker_previous_snapshot (self->caretaker);

  if (snapshot != NULL)
    restore_from_snapshot (self, snapshot);
}

void
canvas_region_redo (CanvasRegion *self)
{
  CanvasRegionSnapshot *snapshot;

  snapshot = canvas_region_caretaker_next_snapshot (self->caretaker);

  if (snapshot != NULL)
    restore_from_snapshot (self, snapshot);
}

cairo_surface_t *
canvas_region_get_image_surface (CanvasRegion *self)
{
  return self->cairo_surface_save;
}

void
canvas_region_emit_color_picked_signal (CanvasRegion *self,
                                        GdkRGBA      *color)
{
  g_signal_emit (self, canvas_region_signals[COLOR_PICKED], 0, color);
}

static void
canvas_region_init (CanvasRegion *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));

  self->width = gtk_drawing_area_get_content_width (self->drawing_area);
  self->height = gtk_drawing_area_get_content_height (self->drawing_area);

  self->draw_start_click_cb = &on_brush_draw_start_click;
  self->draw_cb = &on_brush_draw;
  self->current_tool_type = BRUSH;

  self->save_while_drawing = true;
  self->caretaker = canvas_region_caretaker_new ();


  self->cairo_surface_save = cairo_image_surface_create (CAIRO_FORMAT_RGB24,
                                                         self->width, self->height);

  cairo_whiten_surface (self->cairo_surface_save);

  gtk_widget_set_cursor_from_name (GTK_WIDGET (self->resize_corner), "nwse-resize");

  set_is_current_file_saved (self, true);

  create_and_save_snapshot (self);

  gtk_drawing_area_set_draw_func (self->drawing_area, drawing_area_draw_function, self, NULL);
}

static void
canvas_region_dispose (GObject *gobject)
{
  CanvasRegion *self = (CanvasRegion *) gobject;
  canvas_region_caretaker_dispose (self->caretaker);
  cairo_surface_destroy (self->cairo_surface_save);

  gtk_widget_dispose_template (GTK_WIDGET (gobject), PAINT_TYPE_CANVAS_REGION);
  G_OBJECT_CLASS (canvas_region_parent_class)->dispose (gobject);
}

static void
canvas_region_class_init (CanvasRegionClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class,
                                               "/org/gnome/paint/ui/canvas-region.ui");

  /* Widgets */
  gtk_widget_class_bind_template_child (widget_class, CanvasRegion, drawing_area);
  gtk_widget_class_bind_template_child (widget_class, CanvasRegion, text_popover);
  gtk_widget_class_bind_template_child (widget_class, CanvasRegion, text_popover_text_entry);
  gtk_widget_class_bind_template_child (widget_class, CanvasRegion, resize_corner);

  /* Callbacks */
  gtk_widget_class_bind_template_callback (widget_class, drawing_area_on_mouse_press);
  gtk_widget_class_bind_template_callback (widget_class, on_gesture_drag_begin);
  gtk_widget_class_bind_template_callback (widget_class, on_gesture_drag_update);
  gtk_widget_class_bind_template_callback (widget_class, on_gesture_drag_end);
  gtk_widget_class_bind_template_callback (widget_class, on_resize_corner_drag_update);
  gtk_widget_class_bind_template_callback (widget_class, on_resize_corner_drag_end);
  gtk_widget_class_bind_template_callback (widget_class, on_text_popover_text_change);
  gtk_widget_class_bind_template_callback (widget_class, on_text_popover_close);
  gtk_widget_class_bind_template_callback (widget_class, on_text_popover_submit);
  gtk_widget_class_bind_template_callback (widget_class, on_text_popover_activate);

  /* Signals */
  canvas_region_signals[SAVE_STATUS_CHANGE] = g_signal_new ("file-save-status-change",
                                                             G_TYPE_FROM_CLASS (klass),
                                                             G_SIGNAL_RUN_LAST,
                                                             0,
                                                             NULL,
                                                             NULL,
                                                             NULL,
                                                             G_TYPE_NONE,
                                                             1,
                                                             G_TYPE_BOOLEAN);

  canvas_region_signals[RESIZE] = g_signal_new ("resize",
                                                G_TYPE_FROM_CLASS (klass),
                                                G_SIGNAL_RUN_LAST,
                                                0,
                                                NULL,
                                                NULL,
                                                NULL,
                                                G_TYPE_NONE,
                                                2,
                                                G_TYPE_INT,
                                                G_TYPE_INT);

  canvas_region_signals[COLOR_PICKED] = g_signal_new ("color-picked",
                                                      G_TYPE_FROM_CLASS (klass),
                                                      G_SIGNAL_RUN_LAST,
                                                      0,
                                                      NULL,
                                                      NULL,
                                                      NULL,
                                                      G_TYPE_NONE,
                                                      1,
                                                      GDK_TYPE_RGBA);

  /* Dispose */
  G_OBJECT_CLASS (klass)->dispose = canvas_region_dispose;
}
