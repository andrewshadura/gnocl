/*
 * $Id: menuSeparator.c,v 1.4 2005/01/01 15:27:54 baum Exp $
 *
 * This file implements the menuSeparator widget
 *
 * Copyright (c) 2001 - 2005 Peter G. Baum  http://www.dr-baum.net
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

/*
   History:
   2013-07:	added commands, options, commands
   2009-12: renamed separatorFunc to menuSeparatorFunc to avoid name clash,
   2008-10: added command, class
   2002-10: split from menuItem
   2001-03: Begin of developement
 */

/**
\page page_menuSeparator gnocl::menuSeparator
\htmlinclude menuSeparator.html
**/

#include "gnocl.h"

static GnoclOption separatorOptions[] =
{
	{ "-name", GNOCL_STRING, "name" },
	{ "-visible", GNOCL_BOOL, "visible" },
	{ "-sensitive", GNOCL_BOOL, "sensitive" },
	{ NULL }
};


/**
\brief
**/
static int menuSeparatorFunc ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{
	static const char *cmds[] = { "delete", "configure", "class", "options", "commands", NULL };
	enum cmdIdx { DeleteIdx, ConfigureIdx, ClassIdx, OptionsIdx, CommandsIdx };

	GtkSeparatorMenuItem *separator = GTK_SEPARATOR_MENU_ITEM ( data );
	int idx;

	if ( objc < 2 )
	{
		Tcl_WrongNumArgs ( interp, 1, objv, "command" );
		return TCL_ERROR;
	}

	if ( Tcl_GetIndexFromObj ( interp, objv[1], cmds, "command",
							   TCL_EXACT, &idx ) != TCL_OK )
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
				gnoclGetOptions ( interp, separatorOptions );
			}
		case ClassIdx:
			Tcl_SetObjResult ( interp, Tcl_NewStringObj ( "menuSeparator", -1 ) );
			break;
		case DeleteIdx:
			return gnoclDelete ( interp, GTK_WIDGET ( separator ), objc, objv );
		case ConfigureIdx:
			{
				int ret = gnoclParseAndSetOptions ( interp, objc - 1, objv + 1,
													separatorOptions, G_OBJECT ( separator ) );
				gnoclClearOptions ( separatorOptions );
				return ret;
			}

			break;
	}

	return TCL_OK;
}

/**
\brief
**/
int gnoclMenuSeparatorCmd ( ClientData data, Tcl_Interp *interp,
							int objc, Tcl_Obj * const objv[] )
{
	int        ret;
	GtkSeparatorMenuItem *separator;

	if ( gnoclParseOptions ( interp, objc - 1, objv + 1, separatorOptions )
			!= TCL_OK )
	{
		gnoclClearOptions ( separatorOptions );
		return TCL_ERROR;
	}

	separator = GTK_SEPARATOR_MENU_ITEM ( gtk_separator_menu_item_new( ) );

	gtk_widget_show ( GTK_WIDGET ( separator ) );
	/* gtk_widget_set_sensitive( GTK_WIDGET( para->menuItem ), 0 ); */

	ret = gnoclSetOptions ( interp, separatorOptions, G_OBJECT ( separator ), -1 );
	gnoclClearOptions ( separatorOptions );

	if ( ret != TCL_OK )
	{
		gtk_widget_destroy ( GTK_WIDGET ( separator ) );
		return TCL_ERROR;
	}


	return gnoclRegisterWidget ( interp, GTK_WIDGET ( separator ), menuSeparatorFunc );
}

/*****/
