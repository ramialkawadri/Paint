/* canvas-region-caretaker.c
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

#include "utils/canvas-region-caretaker.h"

typedef struct _SnapshotNode SnapshotNode;

struct _SnapshotNode {
  SnapshotNode         *previous;
  SnapshotNode         *next;
  CanvasRegionSnapshot *snapshot;
};

struct _CanvasRegionCaretaker {
  SnapshotNode *current;
  SnapshotNode *head;
  gint          length;
};

static const gint MAX_NUMBER_OF_SNAPSHOTS = 16;

static SnapshotNode *
snapshot_node_new (void)
{
  SnapshotNode *obj = g_malloc (sizeof (SnapshotNode));
  obj->next = NULL;
  obj->previous = NULL;
  obj->snapshot = NULL;
  return obj;
}

static void
snapshot_node_dispose (SnapshotNode *node)
{
  canvas_region_snapshot_dispose (node->snapshot);
  g_free (node);
}

/**
 * Passed node is not included
 */
static void
free_all_next_nodes (SnapshotNode *node)
{
  SnapshotNode *current, *temp;
  current = node->next;
  
  while (current != NULL)
    {
      temp = current->next;
      snapshot_node_dispose (current);
      current = temp;
    }
}


CanvasRegionCaretaker *
canvas_region_caretaker_new (void)
{
  CanvasRegionCaretaker *obj = g_malloc (sizeof (CanvasRegionCaretaker));
  obj->current = NULL;
  obj->head = NULL;
  obj->length = 0;
  return obj;
}

void
canvas_region_caretaker_dispose (CanvasRegionCaretaker *self)
{
  if (self->head != NULL)
    {
      free_all_next_nodes (self->head);
      snapshot_node_dispose (self->head);
    }

  g_free (self);
}

void
canvas_region_caretaker_save_snapshot (CanvasRegionCaretaker *self,
                                       CanvasRegionSnapshot  *snapshot)
{
  SnapshotNode *temp;
  SnapshotNode *node;

  node = snapshot_node_new ();
  node->snapshot = snapshot;

  if (self->head == NULL)
    {
      self->head = node;
      self->current = node;
      self->length = 1;
      return;
    }

  if (self->length == MAX_NUMBER_OF_SNAPSHOTS)
    {
      temp = self->head->next;
      temp->previous = NULL;
      snapshot_node_dispose (self->head);
      self->head = temp;
    }
  else
    {
      self->length += 1;
    }

  free_all_next_nodes (self->current);

  node->previous = self->current;
  self->current->next = node;
  self->current = node;
}

CanvasRegionSnapshot *
canvas_region_caretaker_previous_snapshot (CanvasRegionCaretaker *self)
{
  if (self->current == NULL || self->current->previous == NULL)
    return NULL;
  
  self->current = self->current->previous;
  return self->current->snapshot;
}

CanvasRegionSnapshot *
canvas_region_caretaker_next_snapshot (CanvasRegionCaretaker *self)
{
  if (self->current == NULL || self->current->next == NULL)
    return NULL;
  
  self->current = self->current->next;
  return self->current->snapshot;
}

void
canvas_region_mark_current_snapshot_as_saved (CanvasRegionCaretaker *self)
{
  SnapshotNode *current;

  current = self->current->previous;
  while (current != NULL)
    {
      current->snapshot->is_current_file_saved = false;
      current = current->previous;
    }

  current = self->current->next;
  while (current != NULL)
    {
      current->snapshot->is_current_file_saved = false;
      current = current->next;
    }
  
  self->current->snapshot->is_current_file_saved = true;
}
