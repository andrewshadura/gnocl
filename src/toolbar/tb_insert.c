/*
 * tb_insert.c
 *
 * This file implements the toolbar toggle button item.
 *
 * Copyright (c) 2001 - 2005 Peter G. Baum  http://www.dr-baum.net
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */


/**
\page page_toolbar_widget "gnocl::toolbar add widget"
\htmlinclude arrowButton.html
\note If the widget is to be relocated, first remove, and then reinsert.
	e.g.  $tbar remove $wid
		  $tbar add widget $wid -position 2
**/

/*
   History:
   2014-01: moved toolbar item creation functions into own modules
*/

#include "../gnocl.h"

/**
\brief  Insert item to a toolbar widget.
\note	Using new Toolbar API. Gtk+ 2.16
**/
int insertItem ( GtkToolbar *toolbar, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[], int pos )
{

#if 0
	listParameters ( objc, objv, __FUNCTION__ );
#endif

	GtkWidget *item;

	if ( objc != 3 && objc != 5 )
	{
		Tcl_WrongNumArgs ( interp, 3, objv, "<optional> -position pos" );
		return TCL_ERROR;
	}

	item = gnoclGetWidgetFromName ( Tcl_GetString ( objv[2] ),	 interp );

	if ( item == NULL )
	{
		return TCL_ERROR;
	}

	if ( strcmp ( Tcl_GetString ( objv[3] ), "-position" ) == 0 )
	{
		Tcl_GetIntFromObj ( NULL, objv[4], &pos );
	}

	// GtkToolItems are widgets that can appear on a toolbar.
	// To create a toolbar item that contain something else than a button, use gtk_tool_item_new().
	// Use gtk_container_add() to add a child widget to the tool item.

	gtk_toolbar_insert ( toolbar, item, pos );

	gtk_widget_show_all ( item );

	Tcl_SetObjResult ( interp, objv[2] );

	return TCL_OK;
}
