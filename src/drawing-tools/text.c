/* text.c
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

#include "text.h"

void
on_text_draw_start_click (CanvasRegion *canvas_region,
                          cairo_t      *cr,
                          DrawEvent    *draw_event)
{
  GdkRectangle rect;
  rect.x = draw_event->current_mouse_position.x;
  rect.y = draw_event->current_mouse_position.y;
  rect.width = 0;
  rect.height = 0;

  canvas_region_show_text_popover (canvas_region, &rect);
}

void
on_text_draw (CanvasRegion *canvas_region,
              cairo_t      *cr,
              DrawEvent    *draw_event)
{
  gchar *text = canvas_region_get_text_popover_text (canvas_region);

  cairo_set_font_size (cr, draw_event->draw_size * 1.33);
  cairo_move_to (cr, draw_event->current_mouse_position.x, draw_event->current_mouse_position.y);
  cairo_show_text (cr, text);
}
