/*
 * tb_menuButton.c
 *
 * This file implements the toolbar menu button item.
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

static int gnoclOptMenu ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret );

/* menuButton options */
static const int menuButtonTextIdx      	= 0;
static const int menuButtonIconIdx 			= 1;
static const int menuButtonMenuIdx 			= 2;
static const int menuButtonPostitionIdx		= 3;

//static const int menuButtonArrowClickedIdx = 4;
GnoclOption menuButtonOptions[] =
{
	{ "-text", GNOCL_OBJ, NULL },
	{ "-icon", GNOCL_OBJ, NULL},
	{ "-menu", GNOCL_OBJ, "", gnoclOptMenu },
	{ "-position", GNOCL_OBJ, NULL },

	// { "-onArrowClicked", GNOCL_STRING, NULL }, /* 4 */

	/* object signals */
	{ "-onShowMenu", GNOCL_OBJ, "show-menu", gnoclOptCommand },
	{ "-onClicked", GNOCL_OBJ, "clicked", gnoclOptOnClicked },

	/* inherited options */
	{ "-visible", GNOCL_BOOL, "visible" },
	{ "-sensitive", GNOCL_BOOL, "sensitive" },
	{ "-tooltip", GNOCL_OBJ, "", gnoclOptTooltip },
	{ "-arrowTooltip", GNOCL_OBJ, "", gnoclOptArrowTooltip },
	{ "-onShowHelp", GNOCL_OBJ, "", gnoclOptOnShowHelp },
	{ "-name", GNOCL_STRING, "name" },

	{ NULL, 0, 0 }
};
/*
   -------------- button functions ----------------------------
*/


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
static int menuButtonDoCommand ( ToolButtonMenuParams *para, int background )
{
#if 0
	printf ( "%s\n", __FUNCTION__ );
#endif

	if ( para->onClicked )
	{
		GnoclPercSubst ps[] =
		{
			{ 'w', GNOCL_STRING },  /* widget */
			{ 0 }
		};


		ps[0].val.str = para->name;

		return gnoclPercentSubstAndEval ( para->interp, ps, para->onClicked, background );
	}

	return TCL_OK;
}

/**
\brief	Configure the menubutton item object
\note	There are two ways of creating this object using either:
			1) a pre-exisiting widget, or
			2) a toolbar button with a stock item. (default)
		This is a 'one-time' choice has to be made during widget creation.
		Problems redisplaying toolitem widget icon after changing.
		Resolved using call to gtk_widget_show (icon) after changing the icon.
**/
static int menuButtonConfigure ( Tcl_Interp *interp, ToolButtonMenuParams *para, GnoclOption options[] )
{

#if 0
	printf ( "%s\n", __FUNCTION__ );
#endif

	gchar *label = NULL;
	GtkWidget *icon_widget = NULL;


	if ( options[menuButtonPostitionIdx].status == GNOCL_STATUS_CHANGED )
	{

		gint pos;
		GtkWidget *toolBar;

		Tcl_GetIntFromObj ( NULL, options[menuButtonPostitionIdx].val.obj, &pos );
		toolBar = gtk_widget_get_parent ( para->item );

		// gtk_widget_unparent (para->item);
		g_object_ref ( para->item );
		gtk_container_remove ( GTK_CONTAINER ( toolBar ), para->item );

		gtk_toolbar_insert ( toolBar, para->item, pos );
	}


	/* step 1) create the menu button itself one of two methods: */

	/* method 1) use a predefined button for the menu button itself */


	if ( options[menuButtonTextIdx].status == GNOCL_STATUS_CHANGED )
	{
		label = options[menuButtonTextIdx].val.str;
	}

#if 0

	if ( options[menuButtonIconWidgetIdx].status == GNOCL_STATUS_CHANGED )
	{
		/* change the button clicked option */

		icon_widget = gnoclGetWidgetFromName ( options[menuButtonIconWidgetIdx].val.str, interp );

	}

	else
	{
		// para->item = gtk_menu_tool_button_new ( NULL, NULL );
		para->item = gtk_menu_tool_button_new_from_stock  ( GTK_STOCK_ABOUT );

	}

	para->item = gtk_menu_tool_button_new ( icon_widget, label );
#endif

	/* method 2) create a custom widget */

	if ( options[menuButtonIconIdx].status == GNOCL_STATUS_CHANGED )
	{

		para->item = gtk_menu_tool_button_new_from_stock  ( GTK_STOCK_COPY );
	}

	/*
		if ( options[menuButtonArrowClickedIdx].status == GNOCL_STATUS_CHANGED )
		{
			GNOCL_MOVE_STRING ( options[menuButtonArrowClickedIdx].val.str, para->onClicked );
		}
	*/

	/* step 2) attach a menu to this button */
	/*
		if ( options[menuButtonMenuIdx].status == GNOCL_STATUS_CHANGED )
		{
	#if 0
			g_print ( "menu = %s\n", options[menuButtonMenuIdx].val.str );
	#endif
			para->menu = gnoclGetWidgetFromName ( options[menuButtonMenuIdx].val.str, interp );

			gtk_menu_tool_button_set_menu ( para->item, para->menu );
		}
	*/
	return TCL_OK;
}


/**
\brief	Set the menu to display when menuButton downarrow clicked.
**/
static int gnoclOptMenu ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{

	GtkWidget *menu;

	menu = gnoclGetWidgetFromName ( Tcl_GetString ( opt->val.obj ), interp );
	gtk_menu_tool_button_set_menu ( GTK_MENU_TOOL_BUTTON ( obj ), menu );

	return TCL_OK;
}


/**
\brief
**/
static void menuButtonCallback ( GtkWidget *widget, gpointer data )
{
#if 0
	printf ( "%s\n", __FUNCTION__ );
#endif
	ToolButtonMenuParams *para = ( ToolButtonMenuParams * ) data;


	menuButtonDoCommand ( para, 1 );

}


/**
\brief
\note	Using new Toolbar API. Gtk+ 2.16
**/
int addMenuButton ( GtkToolbar *toolbar, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[], int pos )
{

#if 0
	listParameters ( objc, objv, __FUNCTION__ );
#endif


	int          ret;
	ToolButtonParams *para = NULL;
	char         *txt = NULL;
	int          isUnderline;
	GtkWidget    *icon;				/* image created by getTextAndIcon to hold icon graphic*/

	if ( gnoclParseOptions ( interp, objc - 2, objv + 2, menuButtonOptions ) != TCL_OK )
	{
		gnoclClearOptions ( menuButtonOptions );
		return TCL_ERROR;
	}

	gint i;

	/* add label and icon widget */
	if ( getTextAndIcon (
				interp, toolbar,
				menuButtonOptions + menuButtonTextIdx,
				menuButtonOptions + menuButtonIconIdx,
				&txt, &icon, &isUnderline ) != TCL_OK )
	{
		return TCL_ERROR;
	}

	para = g_new ( ToolButtonParams, 1 );

	para->onClicked = NULL;

	para->interp = interp;

	para->name = gnoclGetAutoWidgetId();

	//para->item =  gtk_tool_button_new ( icon, txt );

	para->item = gtk_menu_tool_button_new ( icon, txt );

	//g_signal_connect ( para->item , "clicked", G_CALLBACK ( buttonCallback ), para );

	gtk_toolbar_insert ( GTK_TOOLBAR ( toolbar ),  para->item, pos );
	gtk_widget_show ( GTK_WIDGET (  para->item ) );
	/*-----*/

	if ( isUnderline )
	{
		setUnderline ( para->item );
	}

	ret = gnoclSetOptions ( interp, menuButtonOptions, G_OBJECT ( para->item ), -1 );

	if ( ret == TCL_OK )
	{
		ret = menuButtonConfigure ( interp, para, menuButtonOptions );
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

	gnoclClearOptions ( menuButtonOptions );

	return ret;
}
