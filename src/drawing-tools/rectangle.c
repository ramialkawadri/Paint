/* rectangle.c
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

#include "rectangle.h"

void
on_rectangle_draw (CanvasRegion *self,
                   cairo_t      *cr,
                   DrawEvent    *draw_event)
{
  gdouble start_x = MIN (draw_event->current.x, draw_event->start.x);
  gdouble start_y = MIN (draw_event->current.y, draw_event->start.y);
  gdouble width;
  gdouble height;

  width = ABS (draw_event->current.x - draw_event->start.x);
  height = ABS (draw_event->current.y - draw_event->start.y);

  cairo_set_line_width (cr, draw_event->draw_size);

  cairo_rectangle (cr,
                   start_x,
                   start_y,
                   width,
                   height);
  
  cairo_stroke (cr);
}
