/*
 * $Id: toolPalette.c,v 1.4 2005/01/01 15:27:54 baum Exp $
 *
 * This file implements the toolPalette widget which is a combination of the
 * gtk toolPalette widget (TODO? and the gtk frame widget?)
 *
 * Copyright (c) 2001 - 2005 Peter G. Baum  http://www.dr-baum.net
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

/*
   History:
   2013-07: added commands, options, commands
   2011-04: Begin of developement
 */

/**
\page page_toolPalette gnocl::toolPalette
\htmlinclude toolPalette.html
**/

#include "gnocl.h"

int gnoclOptDragDest ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret );

static const int orientationIdx  = 0;

static GnoclOption options[] =
{
	/* widget specific options */
	{"-orientation", GNOCL_OBJ, NULL},
	{"-dragDest", GNOCL_OBJ, "", gnoclOptDragDest },

	{ "-iconSize", GNOCL_OBJ, NULL },
	{ "-iconSizeSet", GNOCL_OBJ, NULL },


	/* widget signals */
	{ "-onSetScrollAdjustments", GNOCL_OBJ, "", gnoclOptOnScrollAdjustments}, /* taken from text.c, perhaps custom handler needed, see layout.c */

	/* drag and drop */
	{ "-dropTargets", GNOCL_LIST, "t", gnoclOptDnDTargets },
	{ "-dragTargets", GNOCL_LIST, "s", gnoclOptDnDTargets },
	{ "-onDropData", GNOCL_OBJ, "", gnoclOptOnDropData },
	{ "-onDragData", GNOCL_OBJ, "", gnoclOptOnDragData },

	/* general options */
	{ "-name", GNOCL_STRING, "name" },
	{ "-visible", GNOCL_BOOL, "visible" },
	{ "-sensitive", GNOCL_BOOL, "sensitive" },
	{ "-data", GNOCL_OBJ, "", gnoclOptData },
	{ "-tooltip", GNOCL_OBJ, "", gnoclOptTooltip },
	{ NULL },
};

/**
**/
static void dragDest_data_received ( GtkWidget        *toolbar,
									 GdkDragContext   *context,
									 gint              x,
									 gint              y,
									 GtkSelectionData *selection,
									 guint             info,
									 guint             time,
									 gpointer          data )
{
	/* find the tool button, which is the source of this DnD operation */

	GtkWidget *palette = gtk_drag_get_source_widget ( context );
	//CanvasItem *canvas_item = NULL;

	GtkToolbar *toolBar = NULL;
	GtkWidget *tool_item = NULL;

	while ( palette && !GTK_IS_TOOL_PALETTE ( palette ) )
	{
		palette = gtk_widget_get_parent ( palette );
	}

	if ( palette )
	{
		tool_item = gtk_tool_palette_get_drag_item ( GTK_TOOL_PALETTE ( palette ), selection );
	}


	/* append a new toolbar item when a tool button was found */

	if ( GTK_IS_TOOL_ITEM ( tool_item ) )
	{

		gtk_toolbar_insert  ( toolbar, tool_item, -1 );
	}

}


/**
\brief
\note	This command will configure 'dnd' communications between palette and toolbar.
**/
int gnoclOptDragDest ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	GtkFrame *frame;
	GtkWidget *label_widget;
	const char *txt;

	GtkWidget *toolPalette = gtk_bin_get_child ( GTK_BIN ( obj ) );

	GtkWidget *dragDest = gnoclGetWidgetFromName ( Tcl_GetString ( opt->val.obj ), interp );

	/* Step 1) Configure DnD for the palette */

	gtk_tool_palette_add_drag_dest ( GTK_TOOL_PALETTE ( toolPalette ),
									 GTK_WIDGET ( dragDest ),
									 GTK_DEST_DEFAULT_ALL,
									 GTK_TOOL_PALETTE_DRAG_ITEMS |
									 GTK_TOOL_PALETTE_DRAG_GROUPS,
									 GDK_ACTION_MOVE );


	/* Step 2) Configure DnD for the toolbar */

	g_object_connect ( dragDest, "signal::drag-data-received", dragDest_data_received, NULL, NULL );

	return TCL_OK;
}


/**
\brief
**/
static int configure ( Tcl_Interp *interp, GtkWidget *palette_scroller, GnoclOption options[] )
{
	debugStep ( __FUNCTION__, 1 );

	GtkWidget *toolPalette = gtk_bin_get_child ( GTK_BIN ( palette_scroller ) );

	/* set orientation of the toolpalette */
	if ( options[orientationIdx].status == GNOCL_STATUS_CHANGED )
	{

		if ( strcmp ( Tcl_GetString ( options[orientationIdx].val.obj ), "horizontal" ) == 0 )
		{
			gtk_orientable_set_orientation ( GTK_ORIENTABLE ( toolPalette ), GTK_ORIENTATION_HORIZONTAL );

		}

		else if (  strcmp ( Tcl_GetString ( options[orientationIdx].val.obj ), "vertical" ) == 0 )
		{

			gtk_orientable_set_orientation ( GTK_ORIENTABLE ( toolPalette ), GTK_ORIENTATION_VERTICAL );
		}

		else
		{
			return TCL_ERROR;
		}

	}

	return TCL_OK;
}


/**
\brief
**/
int toolPaletteFunc ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{

	debugStep ( __FUNCTION__, 1 );

	static const char *cmds[] =
	{
		"addGroup",
		"delete", "configure", "class", "parent", "options", "commands",
		NULL
	};

	enum cmdIdx
	{
		AddGroupIdx,
		DeleteIdx, ConfigureIdx, ClassIdx, ParentIdx, OptionsIdx, CommandsIdx
	};

	GtkWidget *palette_scroller = GTK_WIDGET ( data );

	GtkWidget *toolPalette = gtk_bin_get_child ( GTK_BIN ( palette_scroller ) );

	int idx;

	if ( Tcl_GetIndexFromObj ( interp, objv[1], cmds, "command",  TCL_EXACT, &idx ) != TCL_OK )
		return TCL_ERROR;

	switch ( idx )
	{
		case CommandsIdx:
			{
				gnoclGetOptions ( interp, cmds );
			}
			break;
		case OptionsIdx:
			{
				gnoclGetOptions ( interp, options );
			}
			break;
		case AddGroupIdx:
			{

				return addGroup ( toolPalette, interp, objc, objv );
			}
			break;
		case ClassIdx:
			{
				Tcl_SetObjResult ( interp, Tcl_NewStringObj ( "toolPalette", -1 ) );
			}
			break;
		case DeleteIdx:
			{
				return gnoclDelete ( interp, GTK_WIDGET ( palette_scroller ), objc, objv );
			}

		case ConfigureIdx:
			{
				int ret = TCL_ERROR;

				if ( gnoclParseAndSetOptions ( interp, objc - 1, objv + 1,
											   options, G_OBJECT ( palette_scroller ) ) == TCL_OK )
				{
					ret = configure ( interp, palette_scroller, options );
				}

				gnoclClearOptions ( options );

				return ret;
			}
		case ParentIdx:
			{
				return TCL_OK;
			}

			break;
	}

	return TCL_OK;
}

/**
\brief
**/
int gnoclToolPaletteCmd ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{
	int        ret;
	GtkWidget  *group;

	GtkToolItem *item;


	GtkWidget *palette = NULL;
	GtkWidget *palette_scroller = NULL;
	GtkWidget *notebook = NULL;
	GtkWidget *contents = NULL;
	GtkWidget *contents_scroller = NULL;


	if ( gnoclParseOptions ( interp, objc, objv, options ) != TCL_OK )
	{
		gnoclClearOptions ( options );
		return TCL_ERROR;
	}

	palette = gtk_tool_palette_new ();

	palette_scroller = gtk_scrolled_window_new ( NULL, NULL );
	gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( palette_scroller ),
									 GTK_POLICY_AUTOMATIC,
									 GTK_POLICY_AUTOMATIC );

	gtk_container_add ( GTK_CONTAINER ( palette_scroller ), palette );


	ret = gnoclSetOptions ( interp, options, G_OBJECT ( palette ), -1 );

	if ( ret == TCL_OK )
	{
		ret = configure ( interp, palette_scroller, options );
	}

	gnoclClearOptions ( options );

	if ( ret != TCL_OK )
	{
		gtk_widget_destroy ( GTK_WIDGET ( palette_scroller ) );
		return TCL_ERROR;
	}

	/* TODO: if not -visible == 0 */
	gtk_widget_show_all ( GTK_WIDGET ( palette_scroller ) );


	return gnoclRegisterWidget ( interp, GTK_WIDGET ( palette_scroller ), toolPaletteFunc );
}
