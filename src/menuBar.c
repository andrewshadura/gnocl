/*
 * $Id: menuBar.c,v 1.4 2005/01/01 15:27:54 baum Exp $
 *
 * This file implements the menuBar widget
 *
 * Copyright (c) 2001 - 2005 Peter G. Baum  http://www.dr-baum.net
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

/*
   History:
   2014-01: new options -packDirection and -childPackDirection
			added options -position -childPackDirection to the add command
   2013-07:	added commands, options, commands
        10: switched from GnoclWidgetOptions to GnoclOption
   2002-04: updates for gtk 2.0
        09: accelerator for menuItems
   2001-03: Begin of developement
 */

/**
\page page_menuBar gnocl::menuBar
\htmlinclude menuBar.html
**/

#include "gnocl.h"

static const int childrenIdx = 0;
static const int packDirectionIdx = 1;
static const int childPackDirectionIdx = 2;

static GnoclOption menuBarOptions[] =
{
	{ "-children", GNOCL_LIST, NULL },
	{ "-packDirection", GNOCL_LIST, NULL },
	{ "-childPackDirection", GNOCL_LIST, NULL },

	{ "-name", GNOCL_STRING, "name" },
	{ "-visible", GNOCL_BOOL, "visible" },
	{ "-sensitive", GNOCL_BOOL, "sensitive" },
	{ NULL }
};



/**
\brief

typedef enum
{
  GTK_PACK_DIRECTION_LTR,
  GTK_PACK_DIRECTION_RTL,
  GTK_PACK_DIRECTION_TTB,
  GTK_PACK_DIRECTION_BTT
} GtkPackDirection;


**/
static int configure ( Tcl_Interp *interp, GtkMenuBar *menuBar, GnoclOption options[] )
{

	gint idx;
	const char *txt[] = { "ltr", "rtl", "ttb", "btt", NULL };
	const int types[] = { GTK_PACK_DIRECTION_LTR, GTK_PACK_DIRECTION_RTL, GTK_PACK_DIRECTION_TTB, GTK_PACK_DIRECTION_BTT };

	if ( options[packDirectionIdx].status == GNOCL_STATUS_CHANGED )
	{
		g_print ( "PACK 1 %s\n", Tcl_GetString ( options[packDirectionIdx].val.obj ) );

		if ( Tcl_GetIndexFromObj ( NULL, options[packDirectionIdx].val.obj, txt, NULL, TCL_EXACT, &idx ) == TCL_OK )
		{

			gtk_menu_bar_set_pack_direction ( menuBar, types[idx] );

			g_print ( "PACK 3 %d %d\n", idx, types[idx] );
		}
	}

	if ( options[childPackDirectionIdx].status == GNOCL_STATUS_CHANGED )
	{
		g_print ( "PACK 1 %s\n", Tcl_GetString ( options[childPackDirectionIdx].val.obj ) );

		if ( Tcl_GetIndexFromObj ( NULL, options[childPackDirectionIdx].val.obj, txt, NULL, TCL_EXACT, &idx ) == TCL_OK )
		{

			gtk_menu_bar_set_child_pack_direction ( menuBar, types[idx] );

			g_print ( "PACK 3 %d %d\n", idx, types[idx] );
		}
	}

	if ( options[childrenIdx].status == GNOCL_STATUS_CHANGED )
	{
		return gnoclMenuShellAddChildren ( interp, GTK_MENU_SHELL ( menuBar ), options[childrenIdx].val.obj, 1 );
	}

	return TCL_OK;
}

static const char *cmds[] =
{
	"delete", "configure",
	"add", "addBegin",
	"addEnd",
	NULL
};

/**
\brief
**/
int menuBarFunc ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{


	enum cmdIdx
	{
		DeleteIdx, ConfigureIdx,
		AddIdx, BeginIdx,
		EndIdx
	};

	GtkMenuBar *menuBar = GTK_MENU_BAR ( data );
	int idx;

	if ( objc < 2 )
	{
		Tcl_WrongNumArgs ( interp, 1, objv, "command" );
		return TCL_ERROR;
	}

	if ( Tcl_GetIndexFromObj ( interp, objv[1], cmds, "command", TCL_EXACT, &idx ) != TCL_OK )
	{
		return TCL_ERROR;
	}

	switch ( idx )
	{

		case DeleteIdx:
			{
				return gnoclDelete ( interp, GTK_WIDGET ( menuBar ), objc, objv );
			}

		case ConfigureIdx:
			{
				int ret = TCL_ERROR;

				if ( gnoclParseAndSetOptions ( interp, objc - 1, objv + 1, menuBarOptions, G_OBJECT ( menuBar ) ) == TCL_OK )
				{
					ret = configure ( interp, menuBar, menuBarOptions );
				}

				gnoclClearOptions ( menuBarOptions );

				return ret;
			}

			break;

		case AddIdx:
			{
				gint pos = -1; /* default to end */

				if ( objc != 3 && objc != 5 )
				{
					Tcl_WrongNumArgs ( interp, 2, objv, "widget-list <optional> -position n" );
					return TCL_ERROR;
				}

				if ( objc == 5 && strcmp ( Tcl_GetString ( objv[3] ), "-position" ) == 0 )
				{

					// g_print("HERE~~~~~~~~~~~\n");
					Tcl_GetIntFromObj ( interp, objv[4], &pos );
				}

				return gnoclMenuShellAddChildren (  interp, GTK_MENU_SHELL ( menuBar ), objv[2], pos ); //idx != BeginIdx );
			}
			break;
		case BeginIdx:
		case EndIdx:
			{
				if ( objc != 3 )
				{
					Tcl_WrongNumArgs ( interp, 2, objv, "widget-list" );
					return TCL_ERROR;
				}

				return gnoclMenuShellAddChildren ( interp, GTK_MENU_SHELL ( menuBar ), objv[2], idx != EndIdx );

			}
	}

	return TCL_OK;
}

/**
\brief
**/
int gnoclMenuBarCmd ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{

	if ( gnoclGetCmdsAndOpts ( interp, cmds, menuBarOptions, objv, objc ) == TCL_OK )
	{
		return TCL_OK;
	}

	int        ret;
	GtkMenuBar *menuBar;

	if ( gnoclParseOptions ( interp, objc, objv, menuBarOptions )
			!= TCL_OK )
	{
		gnoclClearOptions ( menuBarOptions );
		return TCL_ERROR;
	}

	menuBar = GTK_MENU_BAR ( gtk_menu_bar_new( ) );

	gtk_widget_show ( GTK_WIDGET ( menuBar ) );

	ret = gnoclSetOptions ( interp, menuBarOptions, G_OBJECT ( menuBar ), -1 );

	if ( ret == TCL_OK )
		ret = configure ( interp, menuBar, menuBarOptions );

	gnoclClearOptions ( menuBarOptions );

	if ( ret != TCL_OK )
	{
		gtk_widget_destroy ( GTK_WIDGET ( menuBar ) );
		return TCL_ERROR;
	}

	return gnoclRegisterWidget ( interp, GTK_WIDGET ( menuBar ), menuBarFunc );
}

