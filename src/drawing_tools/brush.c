/* brush.c
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

#include "brush.h"

void
on_brush_draw_start_click (CanvasRegion *self,
                           cairo_t      *cr,
                           DrawEvent    *draw_event)
{
  cairo_arc (cr,
             draw_event->current.x,
             draw_event->current.y,
             draw_event->draw_size / 2,
             0, 2 * G_PI);
  cairo_fill (cr);
}

/**
 * Returns the next point with distance d from the point (x0, y0) on the line y = mx + b.
 */
static gdouble
get_next_point_on_line_with_distance_d (gdouble m,
                                        gdouble b,
                                        gdouble x0,
                                        gdouble d)
{
  return (sqrt (m * m + 1) * d + (m * m + 1) * x0) / (m * m + 1);
}

/**
 * Draws a line of circle from last drawn point. It moves along the x axis.
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

  delta_x = draw_event->current.x - draw_event->last.x;
  delta_y = draw_event->current.y - draw_event->last.y;

  m = delta_y / delta_x;
  b = draw_event->current.y - m * draw_event->current.x;

  x = draw_event->last.x;
  y = draw_event->last.y;
  end_x = draw_event->current.x;
   
  r = draw_event->draw_size / 2;

  if (draw_event->last.x > draw_event->current.x)
    {
      x = draw_event->current.x;
      y = draw_event->current.y;

      end_x = draw_event->last.x;
    }

  while (x <= end_x)
    {
      cairo_arc (cr, x, y, r, 0, 2 * G_PI);
      cairo_fill (cr);

      x = get_next_point_on_line_with_distance_d (m, b, x, r / 2);
      y = m * x + b;
    }
}

/**
 * Draws a line of circle from last drawn point. It moves along the y axis.
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

  delta_x = draw_event->current.x - draw_event->last.x;
  delta_y = draw_event->current.y - draw_event->last.y;

  m = delta_x / delta_y;
  b = draw_event->current.x - m * draw_event->current.y;

  x = draw_event->last.x;
  y = draw_event->last.y;
  end_y = draw_event->current.y;

  r = draw_event->draw_size / 2;

  if (draw_event->last.y > draw_event->current.y)
    {
      x = draw_event->current.x;
      y = draw_event->current.y;

      end_y = draw_event->last.y;
    }

  while (y <= end_y)
    {
      cairo_arc (cr, x, y, r, 0, 2 * G_PI);
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

  delta_x = ABS (draw_event->current.x - draw_event->last.x);
  delta_y = ABS (draw_event->current.y - draw_event->last.y);

  if (delta_x < delta_y)
    draw_along_y_axis (cr, draw_event);
  else
    draw_along_x_axis (cr, draw_event);
}
