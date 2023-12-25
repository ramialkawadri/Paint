/* canvas-region.h
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

#pragma once

#include <adwaita.h>
#include "draw-event.h"
#include "toolbar.h"
#include "drawing_tools/drawing-tool-type.h"

G_BEGIN_DECLS

#define PAINT_TYPE_CANVAS_REGION (canvas_region_get_type ())

G_DECLARE_FINAL_TYPE (CanvasRegion, canvas_region, PAINT, CANVAS_REGION, GtkGrid)

/* Callback types */
typedef void (*on_draw_start_click) (CanvasRegion *self,
                                     cairo_t      *cr,
                                     DrawEvent    *draw_event);
typedef void (*on_draw)             (CanvasRegion *self,
                                     cairo_t      *cr,
                                     DrawEvent    *draw_event);

typedef void (*on_file_save_finish) (CanvasRegion *self);

/* Methods */
void                canvas_region_set_toolbar                 (CanvasRegion        *self,
                                                               Toolbar             *toolbar);

void                canvas_region_open_new_file               (CanvasRegion       *self);
void                canvas_region_save_current_file           (CanvasRegion       *self,
                                                               on_file_save_finish on_save_finish);

char               *canvas_region_get_current_file_name       (CanvasRegion       *self);
gboolean            canvas_region_is_current_file_saved       (CanvasRegion       *self);

void                canvas_region_prompt_to_save_current_file (CanvasRegion       *self,
                                                               GCallback           on_response,
                                                               gpointer            user_data);

void                canvas_region_set_selected_tool           (CanvasRegion       *self,
                                                               DRAWING_TOOL_TYPE   tool);

GtkPopover         *canvas_region_get_text_popover            (CanvasRegion       *self);
char               *canvas_region_get_text_popover_text       (CanvasRegion       *self);

void                canvas_region_undo                        (CanvasRegion       *self);
void                canvas_region_redo                        (CanvasRegion       *self);

G_END_DECLS

