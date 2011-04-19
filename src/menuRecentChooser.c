/*
	menuRecentChooser.c
*/

/*
   History:
   29/06/2010: Begin of developement
 */

/**
\page page_menuRecentChooser gnocl::menuRecentChooser
\htmlinclude menuRecentChooser.html
**/

#include "gnocl.h"
#include <string.h>
#include <assert.h>

static GnoclOption recentChooserOptions[] =
{
	{ "-showNumbers", GNOCL_BOOL, "show-numbers" },
	{ NULL }
};


/**
\brief
**/
static int menuRecentChooserFunc ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{
	static const char *cmds[] = { "delete", "configure", "class", NULL };
	enum cmdIdx { DeleteIdx, ConfigureIdx, ClassIdx };
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
		case ClassIdx:
			{
				Tcl_SetObjResult ( interp, Tcl_NewStringObj ( "menuSeparator", -1 ) );
			}
			break;
		case DeleteIdx:
			{
				return gnoclDelete ( interp, GTK_WIDGET ( separator ), objc, objv );
			}
		case ConfigureIdx:
			{
				int ret = gnoclParseAndSetOptions ( interp, objc - 1, objv + 1, recentChooserOptions, G_OBJECT ( separator ) );
				gnoclClearOptions ( recentChooserOptions );
				return ret;
			}

			break;
	}

	return TCL_OK;
}

/**
\brief
**/
int gnoclMenuRecentChooserCmd ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{
	int ret;
	GtkSeparatorMenuItem *separator;

	if ( gnoclParseOptions ( interp, objc - 1, objv + 1, recentChooserOptions ) != TCL_OK )
	{
		gnoclClearOptions ( recentChooserOptions );
		return TCL_ERROR;
	}


	GtkRecentManager *manager;
	GtkRecentFilter *filter;
	GtkWidget *menu, *menu_item;

	filter = gtk_recent_filter_new ();
	gtk_recent_filter_add_application ( filter, g_get_application_name() );

	manager = gtk_recent_manager_get_default ();
	menu = gtk_recent_chooser_menu_new_for_manager ( manager );

	gtk_recent_chooser_set_show_private ( GTK_RECENT_CHOOSER ( menu ), TRUE );
	gtk_recent_chooser_set_show_not_found ( GTK_RECENT_CHOOSER ( menu ), FALSE );
	gtk_recent_chooser_set_show_tips ( GTK_RECENT_CHOOSER ( menu ), TRUE );
	gtk_recent_chooser_set_sort_type ( GTK_RECENT_CHOOSER ( menu ), GTK_RECENT_SORT_MRU );
	gtk_recent_chooser_add_filter ( GTK_RECENT_CHOOSER ( menu ), filter );

	//menu_item = gtk_ui_manager_get_widget (db->uimanager, "/MenuBar/File/RecentFiles");
	gtk_menu_item_set_submenu ( GTK_MENU_ITEM ( menu_item ), menu );

	//g_signal_connect (G_OBJECT (menu), "item-activated", G_CALLBACK (on_activate_recent_item), (gpointer) db);



	//separator = GTK_SEPARATOR_MENU_ITEM ( gtk_separator_menu_item_new( ) );

	gtk_widget_show ( GTK_WIDGET ( menu ) );
	/* gtk_widget_set_sensitive( GTK_WIDGET( para->menuItem ), 0 ); */

	ret = gnoclSetOptions ( interp, recentChooserOptions, G_OBJECT ( menu ), -1 );
	gnoclClearOptions ( recentChooserOptions );

	if ( ret != TCL_OK )
	{
		gtk_widget_destroy ( GTK_WIDGET ( menu ) );
		return TCL_ERROR;
	}


	return gnoclRegisterWidget ( interp, GTK_WIDGET ( menu ), menuRecentChooserFunc );
}

