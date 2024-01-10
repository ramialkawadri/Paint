/* select.c
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

#include "utils/cairo-utils.h"
#include "select.h"
#include "cairo.h"

static gdouble SELECTION_DASH_PATTERN[] = { 6 };
static gint    SELECTION_LINE_WIDTH = 6;
static GdkRGBA SELECTION_RECTANGLE_COLOR = { 0.125, 0.310, 0.537, 0.8};

void
on_select_draw_start_click (CanvasRegion *self,
                            cairo_t      *cr,
                            DrawEvent    *draw_event)
{
    GdkRectangle selection_rect = canvas_region_get_selection_rectangle (self);
    gint end_x;
    gint end_y;

    end_x = selection_rect.x + selection_rect.width;
    end_y = selection_rect.y + selection_rect.height;

    if (draw_event->current.x < selection_rect.x ||
            draw_event->current.x > end_x ||
            draw_event->current.y < selection_rect.y ||
            draw_event->current.y > end_y)
      {
        draw_event ->is_dragging_selection = false;
        return;
      }

    draw_event->is_dragging_selection = true;

    draw_event->selection_offset.x = draw_event->current.x - selection_rect.x;
    draw_event->selection_offset.y = draw_event->current.y - selection_rect.y;
}

static void
draw_selection_rectangle (cairo_t      *cr,
                          GdkRectangle *rect)
{
  gdk_cairo_set_source_rgba (cr, &SELECTION_RECTANGLE_COLOR);

  cairo_set_line_width (cr, SELECTION_LINE_WIDTH);
  cairo_set_dash (cr, SELECTION_DASH_PATTERN, 1, 0);

  cairo_rectangle (cr, rect->x, rect->y, rect->width, rect->height);

  cairo_stroke (cr);
}

static void
move_selection (CanvasRegion *self,
                cairo_t      *cr,
                DrawEvent    *draw_event)
{
  GdkRectangle selection_rect;
  GdkRectangle dest_rect;
  cairo_surface_t *cr_surface;
  gint surface_width;
  gint surface_height;

  selection_rect = canvas_region_get_selection_rectangle (self);
  cr_surface = canvas_region_get_image_surface (self);
  surface_width = cairo_image_surface_get_width (cr_surface);
  surface_height = cairo_image_surface_get_height (cr_surface);

  dest_rect.x = draw_event->current.x - draw_event->selection_offset.x;
  dest_rect.y = draw_event->current.y - draw_event->selection_offset.y;

  // To not have a rectangle bigger than the selection
  dest_rect.width = selection_rect.width + selection_rect.x > surface_width ?
    surface_width - selection_rect.x : selection_rect.width;

  dest_rect.height = selection_rect.height + selection_rect.y > surface_height ?
    surface_height - selection_rect.y : selection_rect.height;

  cairo_move_rectangle (cr_surface,
                        cr, &selection_rect, &dest_rect);

  draw_selection_rectangle (cr, &dest_rect);

  canvas_region_set_selection_destnation (self, dest_rect);
}

void
on_select_draw (CanvasRegion *self,
                cairo_t      *cr,
                DrawEvent    *draw_event)
{
  GdkRectangle selection_rect;

  selection_rect.x = MIN (draw_event->current.x, draw_event->start.x); 
  selection_rect.y = MIN (draw_event->current.y, draw_event->start.y);

  selection_rect.width = ABS (draw_event->current.x - draw_event->start.x);
  selection_rect.height = ABS (draw_event->current.y - draw_event->start.y);

  if (draw_event->is_dragging_selection)
    {
      move_selection (self, cr, draw_event);
    }
  else
    {
      draw_selection_rectangle (cr, &selection_rect);
      canvas_region_set_selection_rectangle (self, selection_rect);
    }
}

