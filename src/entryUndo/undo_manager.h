/*
 * Copyright (C) 1998, 1999 Alex Roberts, Evan Lawrence
 * Copyright (C) 2000, 2001 Chema Celorio, Paolo Maggi
 * Copyright (C) 2002, 2003 Paolo Maggi
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef CHROME_BROWSER_AUTOCOMPLETE_UNDO_MANAGER_H_
#define CHROME_BROWSER_AUTOCOMPLETE_UNDO_MANAGER_H_

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define gtk_entry_TYPE_UNDO_MANAGER            (gtk_entry_undo_manager_get_type())

#define gtk_entry_UNDO_MANAGER(obj)            \
  (G_TYPE_CHECK_INSTANCE_CAST((obj), gtk_entry_TYPE_UNDO_MANAGER, GtkEntryUndoManager))

#define gtk_entry_UNDO_MANAGER_CLASS(klass)    \
  (G_TYPE_CHECK_CLASS_CAST((klass), gtk_entry_TYPE_UNDO_MANAGER, GtkEntryUndoManagerClass))

#define gtk_entry_IS_UNDO_MANAGER(obj)         \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj), gtk_entry_TYPE_UNDO_MANAGER))

#define gtk_entry_IS_UNDO_MANAGER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE((klass), gtk_entry_TYPE_UNDO_MANAGER))

#define gtk_entry_UNDO_MANAGER_GET_CLASS(obj)  \
  (G_TYPE_INSTANCE_GET_CLASS((obj), gtk_entry_TYPE_UNDO_MANAGER, GtkEntryUndoManagerClass))

typedef struct _GtkEntryUndoManager          GtkEntryUndoManager;
typedef struct _GtkEntryUndoManagerClass   GtkEntryUndoManagerClass;

typedef struct _GtkEntryUndoManagerPrivate   GtkEntryUndoManagerPrivate;

struct _GtkEntryUndoManager
{
	GObject base;

	GtkEntryUndoManagerPrivate *priv;
};

struct _GtkEntryUndoManagerClass
{
	GObjectClass parent_class;

	/* Signals */
	void ( *can_undo ) ( GtkEntryUndoManager *um, gboolean can_undo );
	void ( *can_redo ) ( GtkEntryUndoManager *um, gboolean can_redo );
};

GType            gtk_entry_undo_manager_get_type ( void ) G_GNUC_CONST;

GtkEntryUndoManager*   gtk_entry_undo_manager_new ( GtkEntryBuffer     *buffer );

gboolean    gtk_entry_undo_manager_can_undo ( const GtkEntryUndoManager *um );
gboolean    gtk_entry_undo_manager_can_redo ( const GtkEntryUndoManager *um );

void      gtk_entry_undo_manager_undo ( GtkEntryUndoManager   *um );
void      gtk_entry_undo_manager_redo ( GtkEntryUndoManager   *um );

void      gtk_entry_undo_manager_begin_not_undoable_action ( GtkEntryUndoManager  *um );
void      gtk_entry_undo_manager_end_not_undoable_action ( GtkEntryUndoManager  *um );

gint      gtk_entry_undo_manager_get_max_undo_levels ( GtkEntryUndoManager   *um );
void      gtk_entry_undo_manager_set_max_undo_levels ( GtkEntryUndoManager   *um,
		gint        undo_levels );

G_END_DECLS

#endif  // CHROME_BROWSER_AUTOCOMPLETE_UNDO_MANAGER_H_

