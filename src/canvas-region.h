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

#include <gtk/gtk.h>

#include "draw-event.h"
#include "drawing-tools/drawing-tool-type.h"
#include "toolbar.h"

G_BEGIN_DECLS

#define PAINT_TYPE_CANVAS_REGION (canvas_region_get_type ())

G_DECLARE_FINAL_TYPE (CanvasRegion, canvas_region, PAINT, CANVAS_REGION, GtkGrid)

/* Callback types */
typedef void (*on_draw_start_click) (CanvasRegion *canvas_region,
                                     cairo_t      *cr,
                                     DrawEvent    *draw_event);
typedef void (*on_draw)             (CanvasRegion *canvas_region,
                                     cairo_t      *cr,
                                     DrawEvent    *draw_event);

typedef void (*on_save_finish)      (CanvasRegion *canvas_region);

/* Methods */
void                canvas_region_set_toolbar                 (CanvasRegion        *self,
                                                               Toolbar             *toolbar);

void                canvas_region_open_new_file               (CanvasRegion       *self);
void                canvas_region_save                        (CanvasRegion       *self,
                                                               on_save_finish      on_save_finish);

gchar              *canvas_region_get_current_file_name       (CanvasRegion       *self);
gboolean            canvas_region_is_current_file_saved       (CanvasRegion       *self);

void                canvas_region_prompt_to_save_current_file (CanvasRegion       *self,
                                                               GCallback           on_response,
                                                               gpointer            user_data);

void                canvas_region_set_selected_tool           (CanvasRegion       *self,
                                                               DRAWING_TOOL_TYPE   tool);

void                canvas_region_show_text_popover           (CanvasRegion       *self,
                                                               GdkRectangle       *pointing_to);
gchar              *canvas_region_get_text_popover_text       (CanvasRegion       *self);

void                canvas_region_undo                        (CanvasRegion       *self);
void                canvas_region_redo                        (CanvasRegion       *self);

cairo_surface_t    *canvas_region_get_image_surface           (CanvasRegion       *self);
void                canvas_region_emit_color_picked_signal    (CanvasRegion       *self,
                                                               GdkRGBA            *color);

GdkRectangle        canvas_region_get_selection_rectangle     (CanvasRegion       *self);
void                canvas_region_set_selection_rectangle     (CanvasRegion       *self,
                                                               GdkRectangle        rect);
void                canvas_region_set_selection_destination   (CanvasRegion       *self,
                                                               GdkRectangle        dest);

void                canvas_region_draw_selection_rectangle    (CanvasRegion       *self,
                                                               GdkRectangle       *rect);

void                canvas_region_select_all                  (CanvasRegion       *self);

G_END_DECLS