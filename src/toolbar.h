/* toolbar.h
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

G_BEGIN_DECLS

#define PAINT_TYPE_TOOLBAR (toolbar_get_type())

G_DECLARE_FINAL_TYPE(Toolbar, toolbar, PAINT, TOOLBAR, GtkBox)

const GdkRGBA *
toolbar_get_current_color(Toolbar *self);

double
toolbar_get_draw_size(Toolbar *self);

void toolbar_set_file_open_cb(Toolbar *self, GAsyncReadyCallback cb);
void toolbar_set_file_open_user_data(Toolbar *self, gpointer user_data);

G_END_DECLS
