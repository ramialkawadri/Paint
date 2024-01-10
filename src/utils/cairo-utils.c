/* cairo-utils.c
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

cairo_surface_t *
cairo_clone_surface (cairo_surface_t *src)
{
  cairo_surface_t *dst;
  cairo_format_t format;
  gdouble width;
  gdouble height;
  cairo_t *cr;

  format = cairo_image_surface_get_format (src);
  width = cairo_image_surface_get_width (src);
  height = cairo_image_surface_get_height (src);
  dst = cairo_image_surface_create (format, width, height);

  cr = cairo_create (dst);
  cairo_set_source_surface (cr, src, 0.0, 0.0);
  cairo_paint (cr);
  cairo_destroy (cr);

  return dst;
}

void
cairo_whiten_surface (cairo_surface_t *cairo_surface)
{
  cairo_t *cr = cairo_create (cairo_surface);
  cairo_set_source_rgb (cr, 1.0, 1.0, 1.0);
  cairo_paint (cr);
  cairo_destroy (cr);
}

GdkRGBA *
cairo_get_pixel_color_at (guchar          *pixels,
                          cairo_surface_t *cairo_surface,
                          gint             x,
                          gint             y)
{
  GdkRGBA *color;
  guchar *current_pixel;
  cairo_format_t format;
  gint width;

  format = cairo_image_surface_get_format (cairo_surface);
  width = cairo_image_surface_get_width (cairo_surface);

  current_pixel = pixels + y * cairo_format_stride_for_width (format, width) + x * 4;

  color = g_malloc (sizeof (GdkRGBA));

  color->red = current_pixel[2] / 255.0;
  color->green = current_pixel[1] / 255.0;
  color->blue = current_pixel[0] / 255.0;
  color->alpha = 1.0;

  return color;
}


/* The moving work by making a new surface that copies the rectangle
 * and then repaint in the new position */
void
cairo_move_rectangle (cairo_surface_t *src_surface,
                      cairo_t         *cr,
                      GdkRectangle    *from,
                      GdkRectangle    *to)
{
  cairo_surface_t *copy_surface;
  cairo_t *copy_cr;
  gint copy_area_width;
  gint copy_area_height;

  // To not copy anything outside the surface
  copy_area_width = from->width + from->x > cairo_image_surface_get_width (src_surface) ?
      cairo_image_surface_get_width (src_surface) - from->x : from->width;

  copy_area_height = from->height + from->y > cairo_image_surface_get_height (src_surface) ?
      cairo_image_surface_get_height (src_surface) - from->y : from->height;
  
  copy_surface = cairo_image_surface_create (cairo_image_surface_get_format (src_surface),
                                             copy_area_width, copy_area_height);
  
  copy_cr = cairo_create (copy_surface);
  cairo_set_source_surface (copy_cr, src_surface, -from->x, -from->y);
  
  cairo_paint (copy_cr);

  // Whitening the original rectangle
  cairo_rectangle (cr, from->x, from->y, copy_area_width, copy_area_height);
  cairo_set_source_rgb (cr, 1, 1, 1);
  cairo_fill (cr);
  
  cairo_set_source_surface (cr, copy_surface, to->x, to->y);
  
  cairo_paint (cr);
  
  cairo_surface_destroy (copy_surface);
  cairo_destroy (copy_cr);
}

