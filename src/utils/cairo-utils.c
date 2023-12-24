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
copy_cairo_surface (cairo_surface_t *src)
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
make_cairo_surface_white (cairo_surface_t *cairo_surface)
{
  cairo_t *cr = cairo_create (cairo_surface);

  cairo_set_source_rgb (cr, 1.0, 1.0, 1.0);
  cairo_paint (cr);
  cairo_destroy (cr);
}
