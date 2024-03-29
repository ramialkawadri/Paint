/* canvas-region-snapshot.c
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

#include "canvas-region-snapshot.h"

CanvasRegionSnapshot *
canvas_region_snapshot_new (gint             width,
                            gint             height,
                            gboolean         is_current_file_saved,
                            cairo_surface_t *surface)
{
  CanvasRegionSnapshot *obj;
  obj = g_malloc (sizeof (CanvasRegionSnapshot));
  obj->width = width;
  obj->height = height;
  obj->is_current_file_saved = is_current_file_saved;
  obj->surface = surface;
  return obj;
}

void
canvas_region_snapshot_dispose (CanvasRegionSnapshot *self)
{
  cairo_surface_destroy (self->surface);
  g_free (self);
}
