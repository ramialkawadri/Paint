/* line.c
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
#include "line.h"

void on_line_draw_start_click (CanvasRegion *self,
                               cairo_t *cr,
                               DrawEvent *draw_event)
{
}

void on_line_draw (CanvasRegion *self,
                   cairo_t *cr,
                   DrawEvent *draw_event)
{
  cairo_move_to (cr, draw_event->start_x, draw_event->start_y);
  cairo_line_to (cr, draw_event->current_x, draw_event->current_y);

  cairo_set_line_width (cr, draw_event->draw_size);
  cairo_stroke (cr);
}

