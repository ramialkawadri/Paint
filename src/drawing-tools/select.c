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
#include "utils/colors.h"
#include "select.h"
#include "cairo.h"

static gdouble SELECTION_DASH_PATTERN[] = { 6 };
static gint    SELECTION_LINE_WIDTH = 6;
static gint    NUM_DASHES = 1;
static gdouble DASH_OFFSET = 0;

static void
draw_selection_rectangle (cairo_t      *cr,
                          GdkRectangle *rect)
{
  gdk_cairo_set_source_rgba (cr, &SELECTION_RECTANGLE_COLOR);
  cairo_set_line_width (cr, SELECTION_LINE_WIDTH);
  cairo_set_dash (cr, SELECTION_DASH_PATTERN, NUM_DASHES, DASH_OFFSET);
  cairo_rectangle (cr, rect->x, rect->y, rect->width, rect->height);
  cairo_stroke (cr);
}

static void
move_selection (CanvasRegion *canvas_region,
                cairo_t      *cr,
                DrawEvent    *draw_event)
{
  GdkRectangle selection_rect;
  GdkRectangle dest_rect;
  cairo_surface_t *cr_surface;
  gint surface_width;
  gint surface_height;

  selection_rect = canvas_region_get_selection_rectangle (canvas_region);
  cr_surface = canvas_region_get_image_surface (canvas_region);
  surface_width = cairo_image_surface_get_width (cr_surface);
  surface_height = cairo_image_surface_get_height (cr_surface);

  dest_rect.x = draw_event->current_mouse_position.x - draw_event->selection_offset.x;
  dest_rect.y = draw_event->current_mouse_position.y - draw_event->selection_offset.y;

  // Avoid going out of bounds
  dest_rect.width = selection_rect.width + selection_rect.x > surface_width ?
    surface_width - selection_rect.x : selection_rect.width;

  dest_rect.height = selection_rect.height + selection_rect.y > surface_height ?
    surface_height - selection_rect.y : selection_rect.height;

  cairo_move_rectangle (cr_surface, cr, &selection_rect, &dest_rect);

  draw_selection_rectangle (cr, &dest_rect);

  canvas_region_set_selection_destnation (canvas_region, dest_rect);
}

void
on_select_draw_start_click (CanvasRegion *canvas_region,
                            cairo_t      *cr,
                            DrawEvent    *draw_event)
{
    GdkRectangle selection_rect = canvas_region_get_selection_rectangle (canvas_region);

    draw_event->is_dragging_selection =
        point_is_inside_rectangle (&draw_event->current_mouse_position, &selection_rect);

    if (draw_event->is_dragging_selection)
      {
        draw_event->selection_offset.x = draw_event->current_mouse_position.x - selection_rect.x;
        draw_event->selection_offset.y = draw_event->current_mouse_position.y - selection_rect.y;
      }
}

void
on_select_draw (CanvasRegion *canvas_region,
                cairo_t      *cr,
                DrawEvent    *draw_event)
{
  GdkRectangle selection_rect;

  selection_rect.x = MIN (draw_event->current_mouse_position.x, draw_event->drag_start.x); 
  selection_rect.y = MIN (draw_event->current_mouse_position.y, draw_event->drag_start.y);

  if (selection_rect.x < 0)
      selection_rect.x = 0;

  if (selection_rect.y < 0)
      selection_rect.y = 0;

  selection_rect.width = ABS (MAX (draw_event->current_mouse_position.x, 0) -
      draw_event->drag_start.x);
  selection_rect.height = ABS (MAX (draw_event->current_mouse_position.y, 0) -
      draw_event->drag_start.y);

  if (draw_event->is_dragging_selection)
    {
      move_selection (canvas_region, cr, draw_event);
    }
  else
    {
      draw_selection_rectangle (cr, &selection_rect);
      canvas_region_set_selection_rectangle (canvas_region, selection_rect);
    }
}
