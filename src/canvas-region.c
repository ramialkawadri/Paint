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


struct _CanvasRegion
{
    GtkBox parent_type;

    GtkDrawingArea *drawing_area;

    /* Controllers */
    GtkEventControllerMotion *drawing_area_motion_controller;
    GtkGestureDrag           *gesture_drag;

    gdouble mouse_x;
    gdouble mouse_y;

    cairo_surface_t *cairo_surface;
};

G_DEFINE_FINAL_TYPE (CanvasRegion, canvas_region, GTK_TYPE_BOX);

static void
draw_function (GtkDrawingArea *area,
               cairo_t        *cr,
               int             width,
               int             height,
               gpointer       user_data)
{
    CanvasRegion *self = user_data;
    cairo_set_source_surface (cr, self->cairo_surface, 0, 0);
    cairo_paint (cr);
}

static void
on_resize (GtkWidget *widget,
           int width,
           int height,
           gpointer user_data)
{
    CanvasRegion *self = user_data;
    cairo_t *cr =  cairo_create (self->cairo_surface);

    cairo_set_source_rgb (cr, 1.0, 1.0, 1.0);
    cairo_paint (cr);
    cairo_destroy (cr);
}

static void
on_motion (GtkEventControllerMotion *controller,
           gdouble                   x,
           gdouble                   y,
           CanvasRegion          *self)
{
    self->mouse_x = x;
    self->mouse_y = y;
}

static void
on_gesture_drag_begin (GtkGestureDrag *gesture,
                       gdouble start_x,
                       gdouble start_y,
                       gpointer user_data)
{
    CanvasRegion *self = user_data;
    cairo_t *cr =  cairo_create (self->cairo_surface);

    cairo_arc (cr, self->mouse_x, self->mouse_y, 10, 0, 2 * G_PI);

    GdkRGBA color = {0, 0, 0, 1};

    cairo_fill (cr);

    cairo_destroy (cr);

    gtk_widget_queue_draw (GTK_WIDGET (self->drawing_area));
}

static void
canvas_region_init (CanvasRegion *self)
{
  gtk_widget_init_template (GTK_WIDGET (self));

  self->cairo_surface = cairo_image_surface_create (CAIRO_FORMAT_RGB24, 800, 400);
  gtk_drawing_area_set_draw_func (self->drawing_area, draw_function, self, NULL);
}

static void
canvas_region_dispose (GObject *gobject)
{
  gtk_widget_dispose_template (GTK_WIDGET (gobject), PAINT_TYPE_CANVAS_REGION);

  G_OBJECT_CLASS (canvas_region_parent_class)->dispose (gobject);
}

static void
canvas_region_class_init (CanvasRegionClass *klass)
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

  gtk_widget_class_set_template_from_resource (widget_class,
                                               "/org/gnome/paint/canvas-region.ui");

  gtk_widget_class_bind_template_child (widget_class, CanvasRegion, drawing_area);
  gtk_widget_class_bind_template_child (widget_class, CanvasRegion, drawing_area_motion_controller);
  gtk_widget_class_bind_template_child (widget_class, CanvasRegion, gesture_drag);

  gtk_widget_class_bind_template_callback (widget_class, on_motion);
  gtk_widget_class_bind_template_callback (widget_class, on_resize);
  gtk_widget_class_bind_template_callback (widget_class, on_gesture_drag_begin);

  G_OBJECT_CLASS (klass)->dispose = canvas_region_dispose;
}
