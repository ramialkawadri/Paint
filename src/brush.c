/* brush.c
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

#include "brush.h"

void
on_brush_draw_start_click (CanvasRegion *self,
                           cairo_t      *cr,
                           DrawEvent    *draw_event)
{
  cairo_arc (cr,
             draw_event->current_x,
             draw_event->current_y,
             draw_event->draw_size,
            0, 2 * G_PI);
  cairo_fill (cr);
}

static gdouble
get_next_point_on_line_with_distance_d (gdouble m,
                                        gdouble b,
                                        gdouble x0,
                                        gdouble d)
{
  return (sqrt (m * m + 1) * d + (m * m + 1) * x0) / (m * m + 1);
}
/**
 * Draw a line from last drawn point, it moves along the x axis
 */
static void
draw_along_x_axis (cairo_t   *cr,
                   DrawEvent *draw_event)
{
  gdouble delta_x;
  gdouble delta_y;

  gdouble m;
  gdouble b;

  gdouble x;
  gdouble y;
  gdouble end_x;
  
  gdouble r;

  delta_x = draw_event->current_x - draw_event->last_x;
  delta_y = draw_event->current_y - draw_event->last_y;

  m = delta_y / delta_x;
  b = draw_event->current_y - m * draw_event->current_x;

  x = draw_event->last_x;
  y = draw_event->last_y;
  end_x = draw_event->current_x;
   
  r = draw_event->draw_size;

  if (draw_event->last_x > draw_event->current_x)
    {
      x = draw_event->current_x;
      y = draw_event->current_y;

      end_x = draw_event->last_x;
    }

  while (x <= end_x)
    {
      cairo_arc (cr, x, y, draw_event->draw_size, 0, 2 * G_PI);
      cairo_fill (cr);

      x = get_next_point_on_line_with_distance_d (m, b, x, r / 2);
      y = m * x + b;
    }
}

/**
 * Draw a line from last drawn point, it moves along the y axis
 */
static void
draw_along_y_axis (cairo_t   *cr,
                   DrawEvent *draw_event)
{
  gdouble delta_x;
  gdouble delta_y;

  gdouble m;
  gdouble b;

  gdouble x;
  gdouble y;
  gdouble end_y;

  gdouble r;

  delta_x = draw_event->current_x - draw_event->last_x;
  delta_y = draw_event->current_y - draw_event->last_y;

  m = delta_x / delta_y;
  b = draw_event->current_x - m * draw_event->current_y;

  x = draw_event->last_x;
  y = draw_event->last_y;
  end_y = draw_event->current_y;

  r = draw_event->draw_size;

  if (draw_event->last_y > draw_event->current_y)
    {
      x = draw_event->current_x;
      y = draw_event->current_y;

      end_y = draw_event->last_y;
    }

  while (y <= end_y)
    {
      cairo_arc (cr, x, y, draw_event->draw_size, 0, 2 * G_PI);
      cairo_fill (cr);

      y = get_next_point_on_line_with_distance_d (m, b, y, r / 2);
      x = m * y + b;
    }
}

void
on_brush_draw (CanvasRegion *self,
               cairo_t      *cr,
               DrawEvent    *draw_event)
{
  gdouble delta_x;
  gdouble delta_y;

  if (draw_event->last_x == -1 || draw_event->last_y == -1)
    return;

  delta_x = ABS (draw_event->current_x - draw_event->last_x);
  delta_y = ABS (draw_event->current_y - draw_event->last_y);

  if (delta_x < delta_y)
    draw_along_y_axis (cr, draw_event);
  else
    draw_along_x_axis (cr, draw_event);
}
