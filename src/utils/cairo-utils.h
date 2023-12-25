/* cairo-utils.h
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

#include <gtk/gtk.h>

cairo_surface_t *cairo_clone_surface      (cairo_surface_t *src);
void             cairo_whiten_surface     (cairo_surface_t *cairo_surface);

GdkRGBA         *cairo_get_pixel_color_at (guchar          *pixels,
                                           cairo_surface_t *cairo_surface,
                                           gint             x,
                                           gint             y);
