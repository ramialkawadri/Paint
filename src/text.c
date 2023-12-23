/* text.c
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

#include "canvas-region.h"
#include "draw-event.h"
#include "text.h"

void on_text_draw_start_click (CanvasRegion *self,
                               cairo_t *cr,
                               DrawEvent *draw_event)
{
  GtkPopover *text_popover = canvas_region_get_text_popver (self);
  GdkRectangle rect;
  rect.x = draw_event->current_x;
  rect.y = draw_event->current_y;
  rect.width = 1;
  rect.height = 1;

  gtk_popover_set_pointing_to (text_popover, &rect);
  gtk_popover_popup (text_popover);
}

void on_text_draw             (CanvasRegion *self,
                               cairo_t *cr,
                               DrawEvent *draw_event)
{
  gchar *text = canvas_region_get_text_popver_text (self);

  cairo_set_font_size (cr, draw_event->draw_size * 1.33);
  cairo_move_to (cr, draw_event->current_x, draw_event->current_y);
  cairo_show_text (cr, text);
}

