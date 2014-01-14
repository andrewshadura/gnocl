/*
 * $Id: menu.c,v 1.4 2005/01/01 15:27:54 baum Exp $
 *
 * This file implements the menu widget
 *
 * Copyright (c) 2001 - 2005 Peter G. Baum  http://www.dr-baum.net
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

/*
   History:
   2014-01: added -position to the add command
   2013-11: added commad attach
   2013-07:	added commands, options, commands
   2012-09: added -data option
   2008-10: added command, class
        10: switched from GnoclWidgetOptions to GnoclOption
   2002-04: updates for gtk 2.0
        09: accelerator for menuItems
   2001-03: Begin of developement
 */

/*
   TODO?: menu factory? But as tcl function!
*/

/**
\page page_menu gnocl::menu
\htmlinclude menu.html
**/

#include "gnocl.h"

static GnoclOption menuOptions[] =
{
	{ "-children", GNOCL_LIST, NULL },
	{ "-tearoff", GNOCL_BOOL, NULL },
	{ "-title", GNOCL_STRING, "tearoff-title" },
	{ "-name", GNOCL_STRING, "name" },
	{ "-visible", GNOCL_BOOL, "visible" },
	{ "-sensitive", GNOCL_BOOL, "sensitive" },

	/* GtkObject Properties */
	{ "-data", GNOCL_OBJ, "", gnoclOptData },
	{ NULL }
};


static const int childrenIdx = 0;
static const int tearoffIdx  = 1;


/**
/brief      Structure used to pass use script supplied values pointer positition to popup menu command.
**/

typedef struct
{
	int x;
	int y;
} MenuPositionData;

static void position_function ( GtkMenu *menu, gint *x, gint *y, gboolean *push_in, gpointer user_data )
{
	/* Do not do this in your own code */

	MenuPositionData *position_data = user_data;

	if ( x )
	{
		*x = position_data->x;
	}

	if ( y )
	{
		*y = position_data->y;
	}

	if ( push_in )
	{
		*push_in = FALSE;
	}
}


/**
\brief
args
	Tcl_Interp *interp
	GtkMenuShell *shell
	Tcl_Obj *children
	int pos
**/
int gnoclMenuShellAddChildren ( Tcl_Interp *interp, GtkMenuShell *shell, Tcl_Obj *children, int pos )
{
	int n, noChilds;

	if ( Tcl_ListObjLength ( interp, children, &noChilds ) != TCL_OK || noChilds < 1 )
	{
		Tcl_SetResult ( interp, "GNOCL ERROR: Widget-list must be a list with at least one element", TCL_STATIC );
		return TCL_ERROR;
	}

	for ( n = 0; n < noChilds; ++n )
	{
		GtkWidget   *childWidget;
		Tcl_Obj     *tp;
		const char  *childName;

		if ( Tcl_ListObjIndex ( interp, children, n, &tp ) != TCL_OK )
		{
			return TCL_ERROR;
		}

		childName = Tcl_GetString ( tp );

		childWidget = gnoclChildNotPacked ( childName, interp );

		if ( childWidget == NULL )
		{
			return TCL_ERROR;
		}

		if ( !GTK_CHECK_TYPE ( childWidget, GTK_TYPE_MENU_ITEM ) )
		{
			Tcl_AppendResult ( interp, "child window \"", childName, "\" is not a menu item.", ( char * ) NULL );
			return TCL_ERROR;
		}

		if ( pos < -1 )
		{
			pos = -1;
		}

		gtk_menu_shell_insert ( shell, childWidget, pos );

	}

	return TCL_OK;
}


/**
\brief
**/
static int configure ( Tcl_Interp *interp, GtkMenu *menu, GnoclOption options[] )
{
	if ( options[tearoffIdx].status == GNOCL_STATUS_CHANGED )
	{
		/* the tearoff widget is created on creation of the menu.
		   Here it is only hidden or shown */
		GtkWidget *widget = GTK_WIDGET ( GTK_MENU_SHELL ( menu )->children->data );

		if ( options[tearoffIdx].val.b )
		{
			gtk_widget_show ( widget );
		}

		else
		{
			gtk_widget_hide ( widget );
		}
	}

	if ( options[childrenIdx].status == GNOCL_STATUS_CHANGED )
	{
		return gnoclMenuShellAddChildren ( interp, GTK_MENU_SHELL ( menu ), options[childrenIdx].val.obj, 1 );
	}

	return TCL_OK;
}


static const char *cmds[] =
{
	"delete", "configure", "add",
	"addBegin", "addEnd", "popup",
	"popdown", "class", "attach",
	NULL
};

/**
\brief
**/
int menuFunc ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{

#if 1
	listParameters ( objc, objv, __FUNCTION__ );
#endif

	enum cmdIdx
	{
		DeleteIdx, ConfigureIdx, AddIdx,
		BeginIdx, EndIdx, PopupIdx,
		PopdownIdx, ClassIdx, AttachIdx
	};

	GtkMenu *menu = GTK_MENU ( data );
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
			/*
			left_attach : The column number to attach the left side of the item to.
			right_attach : 	The column number to attach the right side of the item to.
			top_attach : The row number to attach the top of the item to.
			bottom_attach : The row number to attach the bottom of the item to.
			*/
		case AttachIdx:
			{
				g_print ( "AttachIdx objc=%d\n", objc );
				uint r;
				uint c;
				uint rowSpan = 1;
				uint colSpan = 1;

				GtkWidget *item;

				if ( objc < 5 )
				{
					Tcl_WrongNumArgs ( interp, 1, objv, "command" );
					return TCL_ERROR;
				}

				item = gnoclGetWidgetFromName ( Tcl_GetString ( objv[2] ), interp );
				Tcl_GetIntFromObj ( interp, objv[3], &r );
				Tcl_GetIntFromObj ( interp, objv[4], &c );

				gint i;

				for ( i = 5 ; i < objc ; i += 2 )
				{
					if ( strcmp ( Tcl_GetString ( objv[i] ), "-rowSpan" ) == 0 )
					{
						g_print ( "row\n" );
						Tcl_GetIntFromObj ( interp, objv[i+1], &rowSpan );
					}

					if ( strcmp ( Tcl_GetString ( objv[i] ), "-colSpan" ) == 0 )
					{
						g_print ( "col\n" );
						Tcl_GetIntFromObj ( interp, objv[i+1], &colSpan );
					}
				}

				/* final error check */
				if ( rowSpan == 0 )
				{
					rowSpan = 1;
				}

				if ( colSpan == 0 )
				{
					colSpan = 1;
				}

				gtk_menu_attach ( GTK_MENU ( menu ), item, c, c + colSpan, r, r + rowSpan );

			}
			break;
		case ClassIdx:
			{
				Tcl_SetObjResult ( interp, Tcl_NewStringObj ( "menu", -1 ) );
			}
			break;
		case DeleteIdx:
			{
				return gnoclDelete ( interp, GTK_WIDGET ( menu ), objc, objv );
			}
			break;
		case ConfigureIdx:
			{
				int ret = TCL_ERROR;

				if ( gnoclParseAndSetOptions ( interp, objc - 1, objv + 1, menuOptions, G_OBJECT ( menu ) ) == TCL_OK )
				{
					ret = configure ( interp, menu, menuOptions );
				}

				gnoclClearOptions ( menuOptions );

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

					//g_print ( "HERE~~~~~~~~~~~\n" );
					Tcl_GetIntFromObj ( interp, objv[4], &pos );
				}

				return gnoclMenuShellAddChildren (  interp, GTK_MENU_SHELL ( menu ), objv[2], pos ); //idx != BeginIdx );
			}
			break;
		case BeginIdx:
		case EndIdx:
			{

				if ( objc != 3 )
				{
					Tcl_WrongNumArgs ( interp, 2, objv, "widget-list <optional> -position n" );
					return TCL_ERROR;
				}

				return gnoclMenuShellAddChildren (  interp, GTK_MENU_SHELL ( menu ), objv[2], idx != BeginIdx );
			}

		case PopupIdx:
			{
				/* supply coordinates to accurately place the menu */

				MenuPositionData position_data;
				position_data.x = atoi ( Tcl_GetString ( objv[2] ) );
				position_data.y = atoi ( Tcl_GetString ( objv[3] ) );

#ifdef DEBUG_MENU
				g_print ( "POPUP x: %d y: %d\n", position_data.x, position_data.y );
#endif

				if ( position_data.x  != 0 && position_data.y  != 0 )
				{
					gtk_menu_popup ( menu, NULL, NULL, position_function, &position_data, 0, 0 );
				}

				else
				{
					gtk_menu_popup ( menu, NULL, NULL, NULL, NULL, 0, 0 );
				}
			}
			break;

		case PopdownIdx:
#ifdef DEBUG_ME
			g_print ( "POPDOWN x: %d y: %d\n", objv[3], objv[4] );
#endif
			gtk_menu_popdown ( menu );
			break;
	}

	return TCL_OK;
}


/**
\brief
**/
int gnoclMenuCmd ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{

	if ( gnoclGetCmdsAndOpts ( interp, cmds, menuOptions, objv, objc ) == TCL_OK )
	{
		return TCL_OK;
	}

	int       ret;
	GtkMenu   *menu;
	GtkWidget *tearoff;

	if ( gnoclParseOptions ( interp, objc, objv, menuOptions ) != TCL_OK )
	{
		gnoclClearOptions ( menuOptions );
		return TCL_ERROR;
	}

	/* set tearoff to "on" as default */
	menu = GTK_MENU ( gtk_menu_new() );

	tearoff = gtk_tearoff_menu_item_new();

	gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), tearoff );

	gtk_widget_show ( tearoff );

	gtk_widget_show ( GTK_WIDGET ( menu ) );

	ret = gnoclSetOptions ( interp, menuOptions, G_OBJECT ( menu ), -1 );

	if ( ret == TCL_OK )
		ret = configure ( interp, menu, menuOptions );

	gnoclClearOptions ( menuOptions );

	if ( ret != TCL_OK )
	{
		gtk_widget_destroy ( GTK_WIDGET ( menu ) );
		return TCL_ERROR;
	}

	return gnoclRegisterWidget ( interp, GTK_WIDGET ( menu ), menuFunc );
}

