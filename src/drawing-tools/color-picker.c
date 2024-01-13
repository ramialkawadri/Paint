/* color-picker.c
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

#include "color-picker.h"
#include "utils/cairo-utils.h"

void
on_color_picker_draw_start_click (CanvasRegion *canvas_region,
                                  cairo_t      *cr,
                                  DrawEvent    *draw_event)
{
  guchar *pixels;
  cairo_surface_t *cairo_surface;
  GdkRGBA *color;

  cairo_surface = canvas_region_get_image_surface (canvas_region);
  pixels = cairo_image_surface_get_data (cairo_surface);
  color = cairo_get_pixel_color_at (pixels,
                                    cairo_surface,
                                    draw_event->current_mouse_position.x,
                                    draw_event->current_mouse_position.y);

  canvas_region_emit_color_picked_signal (canvas_region, color);
}
