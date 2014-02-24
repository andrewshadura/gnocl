/*
 * tb_button.c
 *
 * This file implements the toolbar button item.
 *
 * Copyright (c) 2001 - 2005 Peter G. Baum  http://www.dr-baum.net
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

/**
\page page_arrowButton gnocl::arrowButton
\htmlinclude arrowButton.html
\note The "-arrow" and "-shadow" options do not map to any widget property and are specified as GNOCL_STRINGS.
	These values will be read prior to creation of an instance the arrowButton object. The remaining properties are
	then passed for the configuration of the arrow's container, a button. These values are passed to comply with the
	gnocl format, otherwise custom buttons would need to be implement for each direction.
**/

/*
   History:
   2014-01: moved toolbar item creation functions into own modules
*/

#include "../gnocl.h"

static int optVisibility ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret );
static int buttonConfigure ( Tcl_Interp *interp, ToolButtonParams *para, GnoclOption options[] );

/* button options */
static const int buttonTextIdx = 0;
static const int buttonIconIdx = 1;
static const int buttonPostitionIdx = 2;
//static const int buttonOnClickedIdx = 2;

GnoclOption buttonOptions[] =
{
	/* gnocl specific options */
	{ "-text", GNOCL_OBJ, NULL },
	{ "-icon", GNOCL_OBJ, NULL },
	{ "-position", GNOCL_OBJ, NULL },

	{ "-data", GNOCL_OBJ, "", gnoclOptData }, /* 0 */
	{ "-labelWidget", GNOCL_OBJ, "", gnoclOptToolButtonLabelWidget},	/* useful for bonobo text */
	{ "-iconWidget", GNOCL_OBJ, "", gnoclOptToolButtonIconWidget},		/* odd option, use with caution */
	{ "-important", GNOCL_BOOL, "is-important" },

	/* widget properties */
	/* signal handling */
	/* general options */

	{ "-onClicked", GNOCL_OBJ, "clicked", gnoclOptOnClicked },
	{ "-visibility", GNOCL_OBJ, "", optVisibility },
	{ "-sensitive", GNOCL_BOOL, "sensitive" },
	{ "-tooltip", GNOCL_OBJ, "", gnoclOptTooltip },
	{ "-onShowHelp", GNOCL_OBJ, "", gnoclOptOnShowHelp },
	{ "-name", GNOCL_STRING, "name" },
	{ "-label", GNOCL_STRING, "label" },

	/* menu button option */
	//{ "-menu", GNOCL_OBJ, "", gnoclOptMenu },

	{ NULL, 0, 0 }
};

/*
   -------------- button functions ----------------------------
*/


/**
\brief  Obtain current -option values.
**/
static int cget (   Tcl_Interp *interp,  ToolButtonParams *para,  GnoclOption options[],  int idx )
{

	Tcl_Obj *obj = NULL;

	/*
		if ( idx == dataIdx )
		{
			obj = Tcl_NewStringObj ( para->data, -1 );
			gnoclOptParaData ( interp, para->data, &obj);
		}
	*/
	if ( idx == buttonTextIdx )
	{
		//gtk_tool_button_get_label  (para->item)
		//obj = Tcl_NewStringObj ( para->baseFont, -1 );
	}

	if ( idx == buttonIconIdx )
	{
		//obj = Tcl_NewStringObj ( para->baseFont, -1 );
	}

	if ( idx == buttonPostitionIdx )
	{
		//obj = Tcl_NewStringObj ( para->baseFont, -1 );
		// gint gtk_toolbar_get_item_index (GtkToolbar *toolbar, GtkToolItem *item);
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
static int optVisibility ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
#if 0
	g_print ( "GNOCL MESSAGE: ToolBar option \"-visibility\" not yet implemented.\n", __FUNCTION__ );
#endif

	return TCL_OK;
}

/**
\brief
**/
int toolButtonFunc ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{

#if 1
	printf ( "%s\n", __FUNCTION__ );
#endif

	static const char *cmds[] = { "delete", "configure", "clicked", "class", "cget", NULL };
	enum cmdIdx { DeleteIdx, ConfigureIdx, ClickedIdx, ClassIdx, CgetIdx };
	ToolButtonParams *para = ( ToolButtonParams * ) data;
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
		case CgetIdx:
			{
				int idx;

				switch ( gnoclCget ( interp, objc, objv, G_OBJECT ( para->item ), buttonOptions, &idx ) )
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
							return cget ( interp, para, buttonOptions, idx );
						}
				}
			}
			break;
		case ClassIdx:
			{
				Tcl_SetObjResult ( interp, Tcl_NewStringObj ( "toolBarButton", -1 ) );
				break;
			}
			break;
		case DeleteIdx:
			{
				return gnoclDelete ( interp, GTK_WIDGET ( para->item ), objc, objv );
			}
			break;
		case ConfigureIdx:
			{
#if 0
				printf ( "%s configure\n", __FUNCTION__ );
#endif
				int ret = TCL_ERROR;

				if ( gnoclParseAndSetOptions ( interp, objc - 1, objv + 1, buttonOptions, G_OBJECT ( para->item ) ) == TCL_OK )
				{
					ret = buttonConfigure ( interp, para, buttonOptions );
				}

				gnoclClearOptions ( buttonOptions );

				return ret;
			}

			break;
		case ClickedIdx:
			{
				/* convenience function, allow button to be 'clicked' under script control */
				if ( objc != 2 )
				{
					Tcl_WrongNumArgs ( interp, 2, objv, NULL );
					return TCL_ERROR;
				}

				g_signal_emit_by_name ( para->item , "clicked", NULL );
			}
	}

	return TCL_OK;
}

/**
\brief
**/
static void buttonDestroyFunc ( GtkWidget *widget, gpointer data )
{
	ToolButtonParams *p = ( ToolButtonParams * ) data;

	gnoclForgetWidgetFromName ( p->name );
	Tcl_DeleteCommand ( p->interp, p->name );

	g_free ( p->onClicked );
	g_free ( p->name );
	g_free ( p );
}


/**
\brief
**/
static int buttonConfigure ( Tcl_Interp *interp, ToolButtonParams *para, GnoclOption options[] )
{
#if 0
	printf ( "%s\n", __FUNCTION__ );
#endif

	if ( options[buttonPostitionIdx].status == GNOCL_STATUS_CHANGED )
	{

		gint pos;
		GtkWidget *toolBar;

		Tcl_GetIntFromObj ( NULL, options[buttonPostitionIdx].val.obj, &pos );
		toolBar = gtk_widget_get_parent ( para->item );

		// gtk_widget_unparent (para->item);
		g_object_ref ( para->item );
		gtk_container_remove ( GTK_CONTAINER ( toolBar ), para->item );

		gtk_toolbar_insert ( toolBar, para->item, pos );
	}

	return TCL_OK;
}


/**
\brief  Add button item to a toolbar widget.
\note	Using new Toolbar API. Gtk+ 2.16
**/
int addButton ( GtkToolbar *toolbar, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[], int pos )
{
#if 0
	listParameters ( objc, objv, __FUNCTION__ );
#endif

	int          ret;
	ToolButtonParams *para = NULL;
	char         *txt = NULL;
	int          isUnderline;
	GtkWidget    *icon;				/* image created by getTextAndIcon to hold icon graphic*/

	if ( gnoclParseOptions ( interp, objc - 2, objv + 2, buttonOptions ) != TCL_OK )
	{
		gnoclClearOptions ( buttonOptions );
		return TCL_ERROR;
	}

	gint i;

	/* add label and icon widget */
	if ( getTextAndIcon (
				interp, toolbar,
				buttonOptions + buttonTextIdx,
				buttonOptions + buttonIconIdx,
				&txt, &icon, &isUnderline ) != TCL_OK )
	{
		return TCL_ERROR;
	}

	para = g_new ( ToolButtonParams, 1 );
	para->onClicked = NULL;
	para->interp = interp;
	para->name = gnoclGetAutoWidgetId();
	para->item =  gtk_tool_button_new ( icon, txt );

	//g_signal_connect ( para->item , "clicked", G_CALLBACK ( buttonCallback ), para );

	/*
			if ( buttonOptions[buttonPostitionIdx].status == GNOCL_STATUS_CHANGED )
			{
				Tcl_GetIntFromObj(NULL, buttonOptions[buttonPostitionIdx].val.obj, &pos);
				g_print ( "pos = %d\n", pos );
				gtk_toolbar_insert (gtk_widget_get_parent ( para->item ), para->item, pos);
			}
	*/

	gtk_toolbar_insert ( GTK_TOOLBAR ( toolbar ),  para->item, pos );
	gtk_widget_show ( GTK_WIDGET (  para->item ) );

	if ( isUnderline )
	{
		setUnderline ( para->item );
	}

	ret = gnoclSetOptions ( interp, buttonOptions, G_OBJECT ( para->item ), -1 );

	if ( ret == TCL_OK )
	{
		ret = buttonConfigure ( interp, para, buttonOptions );
	}

	if ( ret != TCL_OK )
	{
		g_free ( para );
	}

	else
	{
		g_signal_connect_after ( G_OBJECT ( para->item ), "destroy", G_CALLBACK ( buttonDestroyFunc ), para );

		gnoclMemNameAndWidget ( para->name, GTK_WIDGET ( para->item ) );

		Tcl_CreateObjCommand ( interp, para->name, toolButtonFunc, para, NULL );
		Tcl_SetObjResult ( interp, Tcl_NewStringObj ( para->name, -1 ) );
	}

	gnoclClearOptions ( buttonOptions );

	return ret;
}

