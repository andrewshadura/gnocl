/*
 * tb_separator.c
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
\page page_toolbar_separator "gnocl::toolbar add separator"
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
\brief  Add button item to a toolbar widget.
\note	Using new Toolbar API. Gtk+ 2.16
**/
int addSeparator ( GtkToolbar *toolbar, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[], int pos )
{

#if 0
	listParameters ( objc, objv, __FUNCTION__ );
#endif

	gint n = -1;

	GtkToolItem *item = NULL;

	if ( objc != 3 && objc != 5 )
	{
		Tcl_WrongNumArgs ( interp, 3, objv, "<optional> -position pos" );
		return TCL_ERROR;
	}

	if ( objc == 5 && strcmp ( Tcl_GetString ( objv[3] ), "-position" ) == 0 )
	{
		Tcl_GetIntFromObj ( NULL, objv[4], &pos );
	}

	item = gtk_separator_tool_item_new ();

	gtk_widget_show ( item );

	gtk_toolbar_insert ( GTK_TOOLBAR ( toolbar ), item, pos );


	return gnoclRegisterWidget ( interp, GTK_WIDGET ( item ), separatorFunc );

}
