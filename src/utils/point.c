/* point.c
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

#include "point.h"

Point *
point_new (gint x,
           gint y)
{
  Point *point = g_malloc (sizeof (Point));
  point->x = x;
  point->y = y;
  return point;
}

void
point_dispose (Point *self)
{
  g_free (self);
}

gboolean
point_is_inside_rectangle (Point        *self,
                           GdkRectangle *rectangle)
{
  return rectangle->x <= self->x && self->x <= rectangle->x + rectangle->width &&
      rectangle->y <= self->y && self->y <= rectangle->y + rectangle->height;
}
