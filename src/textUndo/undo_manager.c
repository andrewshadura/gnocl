/*
 * Copyright (C) 1998, 1999 Alex Roberts, Evan Lawrence
 * Copyright (C) 2000, 2001 Chema Celorio, Paolo Maggi
 * Copyright (C) 2002-2005 Paolo Maggi
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib.h>
#include <stdlib.h>
#include <string.h>

#include "undo_manager.h"

#define DEFAULT_MAX_UNDO_LEVELS 25

typedef struct _GtkSourceUndoAction GtkSourceUndoAction;
typedef struct _GtkSourceUndoInsertAction GtkSourceUndoInsertAction;
typedef struct _GtkSourceUndoDeleteAction GtkSourceUndoDeleteAction;

typedef enum
{
	GTK_SOURCE_UNDO_ACTION_INSERT,
	GTK_SOURCE_UNDO_ACTION_DELETE,
} GtkSourceUndoActionType;

/*
 * We use offsets instead of GtkTextIters because the last ones
 * require to much memory in this context without giving us any advantage.
 */

struct _GtkSourceUndoInsertAction
{
	gint pos;
	gchar *text;
	gint length;
	gint chars;
};

struct _GtkSourceUndoDeleteAction
{
	gint start;
	gint end;
	gchar *text;
	gboolean forward;
};

struct _GtkSourceUndoAction
{
	GtkSourceUndoActionType action_type;

	union
	{
		GtkSourceUndoInsertAction insert;
		GtkSourceUndoDeleteAction delete;
	} action;

	gint order_in_group;

	/* It is TRUE whether the action can be merged with the following action. */
	guint mergeable : 1;

	/* It is TRUE whether the action is marked as "modified".
	 * An action is marked as "modified" if it changed the
	 * state of the buffer from "not modified" to "modified". Only the first
	 * action of a group can be marked as modified.
	 * There can be a single action marked as "modified" in the actions list.
	 */
	guint modified : 1;
};

/* INVALID is a pointer to an invalid action */
#define INVALID ((void *) "IA")

struct _GtkSourceUndoManagerPrivate
{
	GtkTextBuffer *document;

	GList* actions;
	gint next_redo;

	gint actions_in_current_group;

	gint running_not_undoable_actions;

	gint num_of_groups;

	gint max_undo_levels;

	guint can_undo : 1;
	guint can_redo : 1;

	/* It is TRUE whether, while undoing an action of the current group (with order_in_group > 1),
	 * the state of the buffer changed from "not modified" to "modified".
	 */
	guint modified_undoing_group : 1;

	/* Pointer to the action (in the action list) marked as "modified".
	 * It is NULL when no action is marked as "modified".
	 * It is INVALID when the action marked as "modified" has been removed
	 * from the action list (freeing the list or resizing it) */
	GtkSourceUndoAction *modified_action;
};

enum
{
	CAN_UNDO,
	CAN_REDO,
	LAST_SIGNAL
};

/**
\brief
**/
#if !defined(NDEBUG)
static void
print_state ( GtkSourceUndoManager* um )
{
	fprintf ( stderr, "\n***\n" );
	GList* actions = um->priv->actions;

	for ( ; actions; actions = g_list_next ( actions ) )
	{
		GtkSourceUndoAction* act = actions->data;
		fprintf ( stderr, "* type = %s\n", act->action_type == GTK_SOURCE_UNDO_ACTION_DELETE ?
	