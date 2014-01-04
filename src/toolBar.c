/*
 * $Id: toolBar.c,v 1.13 2005/02/22 23:16:10 baum Exp $
 *
 * This file implements the toolbar widget
 *
 * Copyright (c) 2001 - 2005 Peter G. Baum  http://www.dr-baum.net
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

/*
   History:
   2014-01: moved toolbar item creation functions into own modules
   2013-11:	added -data option to button/item objects
   2013-07:	added commands options, commands
			cget -style now works
   2013-06: configure/cget -orientation now works
			added flip -an expiremental feature
   2013-05: added parent
   2012-05: toolbar cget -data now works
   2011-04: added -tooltips
   			nItems
   			re-wrote the code to use updated Toolbar API
   2010-09: added button, toggle, menu & radio as synonyms for item,
   			checkItem, menuButton & radioItem objects. Began updating this
   			code to remove deprecated Gtk+ function calls.
   2009-11: renamed static buttonFunc to non-static toolButtonFunc to
            enable use in parsing glade files
   2008-10: added command, class
   2007-12: propose addition of MenuToolButton
            http://www.gtk.org/api/2.6/gtk/GtkMenuToolButton.html
            change of term 'item' to button
            use command menuButton
   2004-02: added -data for radio and check item
   2003-09: changed -value to -onValue for radio item
            added cget to radio and check item
        12: switched from GnoclWidgetOptions to GnoclOption
        05: updates for gtk 2.0
   2002-02: percent subst. in command
            added STD_OPTIONS
        12: icon text qualified by "%#" or "%/"
   2001-03: Begin of developement
 */

/**
\page page_toolBar gnocl::toolBar
\htmlinclude toolBar.html
**/

#include "gnocl.h"
#include "./toolbar/toolbarItems.h"


/* static function declarations */
static int optOrientation ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret );
static int optVisibility ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret );
static int optStyle ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret );

/* referenced in addButton */
static const int SpaceIdx      	= 0;
static const int WidgetIdx      = 1;
static const int ItemIdx      	= 2;
static const int MenuButtonIdx  = 3;
static const int CheckButtonIdx = 4;
static const int RadioItemIdx  	= 5;
static const int ToggleIdx     	= 6;
static const int ButtonIdx     	= 7;
static const int MenuIdx      	= 8;
static const int RadioIdx      	= 9;

/* */
static const int dataIdx = 0;
static const int orientationIdx = 1;
static const int tooolbarStyleIdx = 2;
static const int reliefIdx = 3;

/* toolBar options */
static GnoclOption toolBarOptions[] =
{
	{ "-data", GNOCL_OBJ, "", gnoclOptData }, /* 0 */
	{ "-orientation", GNOCL_STRING, NULL},
	{ "-style", GNOCL_OBJ, "toolbar-style", optStyle },
	//{ "-relief", GNOCL_OBJ, "button-relief", gnoclOptButtonRelief },

	/* widget specific options */
	{ "-iconSize", GNOCL_INT, "icon-size"},
	{ "-iconSizeSet", GNOCL_BOOL, "icon-size-set"},
	{ "-showArrow", GNOCL_BOOL, "show-arrow"},
	{ "-tooltips", GNOCL_BOOL, "tooltips" },

	/* inherited container properties */
	{ "-borderWidth", GNOCL_OBJ, "border-width", gnoclOptPadding },	/* 6 */

	/* general options */
	{ "-visible", GNOCL_BOOL, "visible" },
	{ "-sensitive", GNOCL_BOOL, "sensitive" },
	{ "-name", GNOCL_STRING, "name" },

	/* drag and drop functionality */
	/*
		{ "-dropTargets", GNOCL_LIST, "t", gnoclOptDnDTargets },
		{ "-dragTargets", GNOCL_LIST, "s", gnoclOptDnDTargets },
		{ "-onDropData", GNOCL_OBJ, "", gnoclOptOnDropData },
		{ "-onDragData", GNOCL_OBJ, "", gnoclOptOnDragData },
	*/
	{ NULL, 0, 0 }
};

/* moved to gnocl.h */
/*
typedef struct
{
    GtkWidget  *item;
    char       *name;
    char       *onClicked;
    Tcl_Interp *interp;
} ToolButtonParams;
*/

/**
\brief
**/
static int optOrientation ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	const char *txt[] = { "horizontal", "vertical", NULL };
	const int types[] = { GTK_ORIENTATION_HORIZONTAL,
						  GTK_ORIENTATION_VERTICAL
						};

	assert ( sizeof ( GTK_ORIENTATION_VERTICAL ) == sizeof ( int ) );

	return gnoclOptGeneric ( interp, opt, obj, "orientation", txt, types, ret );
}

/**
\brief
**/
static int optStyle ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{

#if 0
	printf ( "%s\n", __FUNCTION__ );
#endif

	const char *txt[] =
	{
		"icons", "text", "both", "horizontal", NULL
	};

	const int types[] =
	{
		GTK_TOOLBAR_ICONS, GTK_TOOLBAR_TEXT, GTK_TOOLBAR_BOTH, GTK_TOOLBAR_BOTH_HORIZ
	};

	assert ( sizeof ( GTK_TOOLBAR_ICONS ) == sizeof ( int ) );

	return gnoclOptGeneric ( interp, opt, obj, "style", txt, types, ret );
}

/**
\brief
**/
static int configure ( Tcl_Interp *interp, GtkWidget *widget, GnoclOption options[] )
{

	if ( options[orientationIdx].status == GNOCL_STATUS_CHANGED )
	{
		if ( strcmp ( options[orientationIdx].val.str, "horizontal" ) == 0 )
		{
			gtk_orientable_set_orientation ( GTK_ORIENTABLE ( widget ), GTK_ORIENTATION_HORIZONTAL );

		}
		else if ( strcmp ( options[orientationIdx].val.str, "vertical" ) == 0 )
		{
			gtk_orientable_set_orientation ( GTK_ORIENTABLE ( widget ), GTK_ORIENTATION_VERTICAL );
		}
		else
		{
			return TCL_ERROR;
		}
	}

	return TCL_OK;
}


/**
\brief  Obtain current -option values.
**/
static int toolBarCget (  Tcl_Interp *interp, GtkWidget *widget,  GnoclOption options[],  int idx )
{

	Tcl_Obj *obj = NULL;

	if ( idx == dataIdx )
	{
		obj = Tcl_NewStringObj ( g_object_get_data ( widget, "gnocl::data" ), -1 );
	}

	if ( idx == reliefIdx )
	{
		switch ( gtk_toolbar_get_relief_style ( widget ) )
		{
			case GTK_RELIEF_NORMAL:
				{
					obj = Tcl_NewStringObj ( "normal", -1 );
				} break;
			case GTK_RELIEF_HALF:
				{
					obj = Tcl_NewStringObj ( "half", -1 );
				} break;
			case GTK_RELIEF_NONE:
				{
					obj = Tcl_NewStringObj ( "none", -1 );
				} break;
			default:
				{
					return TCL_ERROR;
				}
		}
	}

	if ( idx == orientationIdx )
	{

		switch ( gtk_orientable_get_orientation ( widget ) )
		{
			case GTK_ORIENTATION_HORIZONTAL:
				{
					obj = Tcl_NewStringObj ( "horizontal", -1 );
				} break;
			case  GTK_ORIENTATION_VERTICAL:
				{
					obj = Tcl_NewStringObj ( "vertical", -1 );
				} break;
			default:
				{
					return TCL_ERROR;
				}
		}
	}

	if ( idx == tooolbarStyleIdx )
	{
		switch ( gtk_toolbar_get_style ( widget ) )
		{
			case GTK_TOOLBAR_ICONS:
				{
					obj = Tcl_NewStringObj ( "icons", -1 );
				} break;
			case  GTK_TOOLBAR_TEXT:
				{
					obj = Tcl_NewStringObj ( "text", -1 );
				} break;
			case  GTK_TOOLBAR_BOTH:
				{
					obj = Tcl_NewStringObj ( "both", -1 );
				} break;
			case  GTK_TOOLBAR_BOTH_HORIZ:
				{
					obj = Tcl_NewStringObj ( "horizontal", -1 );
				} break;
			default:
				{
					return TCL_ERROR;
				}
		}
	}

	if ( obj != NULL )
	{
		Tcl_SetObjResult ( interp, obj );
		return TCL_OK;
	}

	return gnoclCgetNotImplemented ( interp, options + idx );
}



/**
\brief
**/
/* WJG added menuButton item 29/12/07 */

static int addItem ( GtkToolbar *toolbar, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[], int atEnd )
{

	const char *txt[] =
	{
		"button", "item",
		"toggleButton", "checkItem",
		"radioButton", "radioItem",
		"separator", "space",
		"menuButton",
		"widget",
		NULL
	};
	enum typeIdx
	{
		ButtonIdx, ItemIdx,
		ToggleButtonIdx, CheckItemIdx,
		RadioButtonIdx, RadioItemIdx,
		SpaceIdx, SeparatorIdx,
		MenuButtonIdx,
		WidgetIdx,
	};
	int idx;

	if ( objc < 3 )
	{
		Tcl_WrongNumArgs ( interp, 2, objv, "type ?option val ...?" );
		return TCL_ERROR;
	}

	if ( Tcl_GetIndexFromObj ( interp, objv[2], txt, "type", TCL_EXACT, &idx ) != TCL_OK )
	{
		return TCL_ERROR;
	}

	switch ( idx )
	{
		case SeparatorIdx:
		case SpaceIdx:
			{
				gint pos;
				gint n = -1;

				GtkToolItem *item = NULL;

				if ( objc < 3 )
				{
					Tcl_WrongNumArgs ( interp, 3, objv, NULL );
					return TCL_ERROR;
				}

				item = gtk_separator_tool_item_new ();

				gtk_widget_show ( item );

				if ( objc == 5 )
				{

					Tcl_GetIntFromObj ( NULL, objv[4], &pos );

					n = gtk_toolbar_get_n_items ( toolbar );

					gtk_toolbar_insert ( GTK_TOOLBAR ( toolbar ), item, n );

					return TCL_OK;

				}

				if ( atEnd )
				{
					gtk_toolbar_insert ( GTK_TOOLBAR ( toolbar ), item, -1 );
				}

				else
				{
					gtk_toolbar_insert ( GTK_TOOLBAR ( toolbar ), item, 0 );
				}

			}

			break;
		case WidgetIdx:
			{
				GtkWidget *child;

				if ( objc != 4 )
				{
					Tcl_WrongNumArgs ( interp, 3, objv, "widget-ID" );
					return TCL_ERROR;
				}

				child = gnoclGetWidgetFromName ( Tcl_GetString ( objv[3] ),	 interp );

				if ( child == NULL )
				{
					return TCL_ERROR;
				}

				// GtkToolItems are widgets that can appear on a toolbar.
				// To create a toolbar item that contain something else than a button, use gtk_tool_item_new().
				// Use gtk_container_add() to add a child widget to the tool item.

				GtkToolItem *item = gtk_tool_item_new ();
				gtk_container_add ( item, child );


				if ( atEnd )
				{
					gtk_toolbar_insert ( toolbar, item, -1 );
				}

				else
				{
					gtk_toolbar_insert ( toolbar, item, 0 );
				}

				gtk_widget_show_all ( item );
				Tcl_SetObjResult ( interp, objv[3] );

				return TCL_OK;
			}

		case ItemIdx:
		case ButtonIdx:
			{
				return addButton ( toolbar, interp, objc, objv, atEnd );
			}
			break;
		case ToggleButtonIdx:
		case CheckItemIdx:
			{
				return addCheckButton ( toolbar, interp, objc, objv, atEnd );
			}
			break;
		case RadioButtonIdx:
		case RadioItemIdx:
			{
				return addRadioButton ( toolbar, interp, objc, objv, atEnd );
			}
			break;
		case MenuButtonIdx:
			{
				return addMenuButton ( toolbar, interp, objc, objv, atEnd );
			}

	}

	return TCL_OK;
}


static const char *cmds[] =
{
	"flip", "add", "addBegin", "addEnd",
	"class", "configure", "delete",
	"insert", "nItems", "cget",
	"parent",
	NULL
};


/**
\brief
\note	Due to the deprecation of gtk_toolbar_prepend_item etc., the following command need to
		be recoded to reflect the use of gtk_toolbar_insert

		"add", "addBegin", "addEnd"

**/
int toolBarFunc ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{
#if 0
	printf ( "%s\n", __FUNCTION__ );
#endif
	enum cmdIdx
	{
		FlipIdx, AddIdx, BeginIdx, EndIdx,
		ClassIdx, ConfigureIdx, DeleteIdx,
		InsertIdx, NitemsIdx, CgetIdx,
		ParentIdx
	};

	GtkToolbar *toolBar = GTK_TOOLBAR ( data );
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

		case FlipIdx:
			{

				if ( gtk_toolbar_get_orientation ( toolBar ) == GTK_ORIENTATION_HORIZONTAL )
				{
					gtk_orientable_set_orientation ( GTK_ORIENTABLE ( toolBar ), GTK_ORIENTATION_VERTICAL );
				}

				else
				{
					gtk_orientable_set_orientation ( GTK_ORIENTABLE ( toolBar ), GTK_ORIENTATION_HORIZONTAL );
				}
			}
			break;
		case ParentIdx:
			{

				GtkWidget * parent;
				Tcl_Obj *obj = NULL;
				parent = gtk_widget_get_parent ( GTK_WIDGET ( toolBar ) );
				obj = Tcl_NewStringObj ( gnoclGetNameFromWidget ( parent ), -1 );
				Tcl_SetObjResult ( interp, obj );

				/* this function not working too well! */
				/* return gnoclGetParent ( interp, data ); */
				return TCL_OK;
			}

			break;
		case CgetIdx:
			{
				int idx;

				switch ( gnoclCget ( interp, objc, objv, G_OBJECT ( toolBar ), toolBarOptions, &idx ) )
				{
					case GNOCL_CGET_ERROR:
						{
							return TCL_ERROR;
						}
					case GNOCL_CGET_HANDLED:
						{
							return TCL_OK;
						}
					case GNOCL_CGET_NOTHANDLED:
						{
							return toolBarCget ( interp, toolBar, toolBarOptions, idx );
						}
				}
			}
			break;
		case NitemsIdx:
			{

				gchar str[4];
				gint n;

				n = gtk_toolbar_get_n_items ( toolBar );

				sprintf ( str, "%d", n );

				Tcl_SetObjResult ( interp, Tcl_NewStringObj ( str, -1 ) );
			}
			break;
		case ClassIdx:
			{
				Tcl_SetObjResult ( interp, Tcl_NewStringObj ( "toolBar", -1 ) );
			}
			break;
		case DeleteIdx:
			{
				return gnoclDelete ( interp, GTK_WIDGET ( toolBar ), objc, objv );
			}
		case ConfigureIdx:
			{
				//int ret = gnoclParseAndSetOptions ( interp, objc - 1, objv + 1, toolBarOptions, G_OBJECT ( toolBar ) );
				//gnoclClearOptions ( toolBarOptions );
				//return ret;
				int ret = TCL_ERROR;

				if ( gnoclParseAndSetOptions ( interp, objc - 1, objv + 1, toolBarOptions, G_OBJECT ( toolBar ) ) == TCL_OK )
				{
					ret = configure ( interp, toolBar, toolBarOptions );
				}

				gnoclClearOptions ( toolBarOptions );

				return ret;

			}

			break;
		case BeginIdx:
			{
				return addItem ( toolBar, interp, objc, objv, 0 );
			}

		case AddIdx:      /* add is a shortcut for addEnd */
		case EndIdx:
			{
				return addItem ( toolBar, interp, objc, objv, 1 );
			}
	}

	return TCL_OK;
}

/**
\brief	Create an instance of a gnocl:toolBar widget.
**/
int gnoclToolBarCmd ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{

	if ( gnoclGetCmdsAndOpts ( interp, cmds, toolBarOptions, objv, objc ) == TCL_OK )
	{
		return TCL_OK;
	}


	int        ret;
	GtkToolbar *toolBar;

	if ( gnoclParseOptions ( interp, objc, objv, toolBarOptions ) != TCL_OK )
	{
		gnoclClearOptions ( toolBarOptions );
		return TCL_ERROR;
	}

	toolBar = GTK_TOOLBAR ( gtk_toolbar_new() );

	/* turn arrows off, this will prevent problems with toolbars in handleboxes */
	gtk_toolbar_set_show_arrow ( toolBar, 0 );

	gtk_widget_show ( GTK_WIDGET ( toolBar ) );

	ret = gnoclSetOptions ( interp, toolBarOptions, G_OBJECT ( toolBar ), -1 );

	if ( ret == TCL_OK )
	{
		ret = configure ( interp, toolBar, toolBarOptions );
	}

	gnoclClearOptions ( toolBarOptions );

	if ( ret != TCL_OK )
	{
		gtk_widget_destroy ( GTK_WIDGET ( toolBar ) );
		return TCL_ERROR;
	}

	return gnoclRegisterWidget ( interp, GTK_WIDGET ( toolBar ), toolBarFunc );
}

