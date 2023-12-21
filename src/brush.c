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

void on_brush_draw_start_click(CanvasRegion *self,
                               cairo_t *cr,
                               DrawEvent *draw_event)
{
    cairo_arc(cr, draw_event->current_x, draw_event->current_y, 1, 0, 2 * G_PI);
    cairo_fill(cr);
}

void on_brush_draw(CanvasRegion *self,
                   cairo_t *cr,
                   DrawEvent *draw_event)
{
    if (draw_event->last_x != -1 && draw_event->last_y != -1)
    {
        cairo_move_to(cr, draw_event->last_x, draw_event->last_y);
        cairo_line_to(cr, draw_event->current_x, draw_event->current_y);
        cairo_stroke(cr);
    }
}
