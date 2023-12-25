/* canvas-region-caretaker.h
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

#include "canvas-region-snapshot.h"

struct _CanvasRegionCaretaker;

typedef struct _CanvasRegionCaretaker CanvasRegionCaretaker;

CanvasRegionCaretaker *canvas_region_caretaker_new                  (void);
void                   canvas_region_caretaker_dispose              (CanvasRegionCaretaker *self);

void                   canvas_region_caretaker_save_snapshot        (CanvasRegionCaretaker *self,
                                                                     CanvasRegionSnapshot  *snapshot);

CanvasRegionSnapshot  *canvas_region_caretaker_previous_snapshot    (CanvasRegionCaretaker *self);
CanvasRegionSnapshot  *canvas_region_caretaker_next_snapshot        (CanvasRegionCaretaker *self);
void                   canvas_region_mark_current_snapshot_as_saved (CanvasRegionCaretaker *self);
