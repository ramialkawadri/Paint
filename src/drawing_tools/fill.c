/* fill.c
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

#include "fill.h"
#include "utils/cairo-utils.h"
#include "utils/point.h"

static void
add_neighbor_if_valid (GQueue          *queue,
                       gboolean        *added_points,
                       guchar          *pixels,
                       cairo_surface_t *cairo_surface,
                       GdkRGBA         *current_color,
                       gint             width,
                       gint             height,
                       gint             x,
                       gint             y)
{
  GdkRGBA *neighbor_color;
  Point *neighbor;

  if (x < 0 || y < 0 || x >= width || y >= height || added_points[y * width + x])
    return;

  neighbor_color = cairo_get_pixel_color_at (pixels, cairo_surface, x, y);

  if (gdk_rgba_equal (current_color, neighbor_color))
    {
      neighbor = point_new (x, y);
      added_points[y * width + x] = true;
      g_queue_push_tail (queue, neighbor);
    }
  
  g_free (neighbor_color);
}

static void
add_valid_surrounding_neighbors (GQueue          *queue,
                                 gboolean        *added_points,
                                 guchar          *pixels,
                                 cairo_surface_t *cairo_surface,
                                 GdkRGBA         *current_color,
                                 gint             width,
                                 gint             height,
                                 Point           *current_point)
{
  for (int i = -1; i <= 1; i++)
    {
      for (int j = -1; j <= 1; j++)
        {
          if (i == 0 && j == 0)
            continue;

          add_neighbor_if_valid (queue,
                                 added_points,
                                 pixels,
                                 cairo_surface,
                                 current_color,
                                 width, height,
                                 current_point->x + i, current_point->y + j);
        }
    }
}

static void
fill_surrounding_region_that_has_color (cairo_t         *cr,
                                        cairo_surface_t *cairo_surface,
                                        guchar          *pixels,
                                        GdkRGBA         *color,
                                        gint             width,
                                        gint             height,
                                        gint             start_x,
                                        gint             start_y)
{
  Point *current_point;
  GQueue *queue;
  gboolean *added_points;

  current_point = point_new (start_x, start_y);
  queue = g_queue_new ();

  added_points = g_malloc0 (height * width * sizeof (gboolean));
  g_queue_push_tail (queue, current_point);

  while (!g_queue_is_empty (queue))
    {
      current_point = g_queue_pop_head (queue);
      cairo_rectangle (cr, current_point->x, current_point->y, 1, 1);

      add_valid_surrounding_neighbors (queue,
                                       added_points,
                                       pixels,
                                       cairo_surface,
                                       color,
                                       width,
                                       height,
                                       current_point);

      point_dispose (current_point);
    }

  cairo_fill (cr);

  g_queue_free (queue);
  g_free (added_points);
}

void
on_fill_draw_start_click (CanvasRegion *self,
                          cairo_t      *cr,
                          DrawEvent    *draw_event)
{
  guchar *pixels;
  cairo_surface_t *cairo_surface;
  GdkRGBA *original_color;
  gint height;
  gint width;

  cairo_surface = canvas_region_get_image_surface (self);

  width = cairo_image_surface_get_width (cairo_surface);
  height = cairo_image_surface_get_height (cairo_surface);

  pixels = cairo_image_surface_get_data (cairo_surface);
  original_color = cairo_get_pixel_color_at (pixels,
                                             cairo_surface,
                                             draw_event->current.x, draw_event->current.y);

  fill_surrounding_region_that_has_color (cr,
                                          cairo_surface,
                                          pixels,
                                          original_color,
                                          width,
                                          height,
                                          draw_event->current.x,
                                          draw_event->current.y);

  g_free (original_color);
}
