/* circle.c
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

#include "canvas-region.h"
#include "draw-event.h"
#include "circle.h"

void on_circle_draw_start_click (CanvasRegion *self,
                                 cairo_t *cr,
                                 DrawEvent *draw_event)
{
}

void on_circle_draw (CanvasRegion *self,
                     cairo_t *cr,
                     DrawEvent *draw_event)
{
  gdouble x;
  gdouble y;
  gdouble width;
  gdouble height;

  width = ABS(draw_event->current_x - draw_event->start_x);
  height = ABS(draw_event->current_y - draw_event->start_y);

  x = (draw_event->start_x + draw_event->current_x) / 2;
  y = (draw_event->start_y + draw_event->current_y) / 2;

  cairo_save (cr);

  cairo_translate (cr, x, y);
  cairo_scale (cr, width / 2, height / 2);
  cairo_arc (cr, 0, 0, 1, 0, 2 * G_PI);

  cairo_restore (cr);

  cairo_set_line_width (cr, draw_event->draw_size);
  cairo_stroke (cr);
}
