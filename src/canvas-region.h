/* canvas-region.h
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

#pragma once

#include <adwaita.h>
#include "draw-event.h"

G_BEGIN_DECLS

#define PAINT_TYPE_CANVAS_REGION (canvas_region_get_type())

G_DECLARE_FINAL_TYPE(CanvasRegion, canvas_region, PAINT, CANVAS_REGION, GtkBox)

G_END_DECLS

// Functions
#define DRAW_FUNCTIONS_PARAMETERS (CanvasRegion * self, cairo_t * cr, DrawEvent * draw_event)
typedef void(*on_draw_start_click) DRAW_FUNCTIONS_PARAMETERS;
typedef void(*on_draw) DRAW_FUNCTIONS_PARAMETERS;
