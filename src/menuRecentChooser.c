/** menuRecentChooser.c
\author    William J Giddings email: wjgiddings@blueyonder.co.uk
\date      31-Dec-2011
\version   0.9.95
**/

/*
   History:
   2013-07:	added commands, options, commands
   2011-12: New module added.
 */

/**
\page page_menuRecentChooser gnocl::menuRecentChooser
\htmlinclude menuRecentChooser.html
**/

#include "gnocl.h"

static int gnoclOptOnSelectionDone ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret );

static const int patternsIdx = 0;
static const int sortIdx     = 1;

static GnoclOption recentChooserOptions[] =
{
	{ "-patterns", GNOCL_STRING, NULL }, /* 0 */
	{ "-sort", GNOCL_STRING, NULL },     /* 1 */

	{ "-limit", GNOCL_INT, "limit", },
	{ "-showIcons", GNOCL_BOOL, "show-icons" },
	{ "-showNotFound", GNOCL_BOOL, "show-not-found" },
	{ "-showNumbers", GNOCL_BOOL, "show-numbers" },
	{ "-showLocal", GNOCL_BOOL, "local-only" },
	{ "-showPrivate", GNOCL_BOOL, "show-private" },
	{ "-showTips", GNOCL_BOOL, "show-tips" },

	{ "-onClicked", GNOCL_OBJ, "selection-done", gnoclOptOnSelectionDone },

	{ NULL }
};


/**
\brief
**/
static void doOnSelectionDone ( GtkMenuShell *menushell, gpointer data )
{

	GnoclCommandData *cs = ( GnoclCommandData * ) data;
	GtkRecentInfo *info;


	info = gtk_recent_chooser_get_current_item ( GTK_RECENT_CHOOSER ( menushell ) );

	GnoclPercSubst ps[] =
	{
		{ 'w', GNOCL_STRING },  /* widget */
		{ 'f', GNOCL_STRING },  /* widget */
		{ 'l', GNOCL_STRING },  /* widget */
		{ 0 }
	};

	ps[0].val.str = gnoclGetNameFromWidget ( GTK_WIDGET ( menushell ) );
	ps[1].val.str =  gtk_recent_chooser_get_current_uri  ( GTK_RECENT_CHOOSER ( menushell ) );

	/*
		GList *items, *p;
		gchar *list;

		items = gtk_recent_chooser_get_items (chooser);


		for ( p = items ; p != NULL; p = p->next )
		{
			sprintf(list,"%s ", p->data);
		}

		ps[2].val.str = list;
	*/

	gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 );

}

/**
\brief
**/
static int gnoclOptOnSelectionDone ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{

	return gnoclConnectOptCmd ( interp, obj, "selection-done", G_CALLBACK ( doOnSelectionDone ), opt, NULL, ret );
}

/**
\brief
**/
static int configure ( Tcl_Interp *interp, GtkWidget *menu, GnoclOption options[] )
{
	g_print ( "%s \n", __FUNCTION__ );

	if ( options[sortIdx].status == GNOCL_STATUS_CHANGED )
	{
		char *type;
		type = options[sortIdx].val.str;

		if ( strcmp ( type, "none" ) == 0 )
		{
			/* no sort */
			gtk_recent_chooser_set_sort_type ( GTK_RECENT_CHOOSER ( menu ), GTK_RECENT_SORT_NONE );
		}

		else if ( strcmp ( type, "most" ) == 0 )
		{
			/* most used */
			gtk_recent_chooser_set_sort_type ( GTK_RECENT_CHOOSER ( menu ), GTK_RECENT_SORT_MRU );
		}

		else if ( strcmp ( type, "least" ) == 0 )
		{
			/* least used */
			gtk_recent_chooser_set_sort_type ( GTK_RECENT_CHOOSER ( menu ), GTK_RECENT_SORT_LRU );
		}

		else
		{
			/* error */
			Tcl_SetObjResult ( interp, Tcl_NewStringObj ( "Sort type must be one of none, most or lease.", -1 ) );
			return TCL_ERROR;
		}

	}


	if ( options[patternsIdx].status == GNOCL_STATUS_CHANGED )
	{
		GtkRecentFilter *filter;
		char *str, *pch;

		filter = gtk_recent_chooser_get_filter ( menu );

		if ( filter == NULL )
		{
			filter = gtk_recent_filter_new();
		}


		/* this list needs to be parsed */
		str = options[patternsIdx].val.str;
		pch = strtok ( str, " " );

		while ( pch != NULL )
		{
			gtk_recent_filter_add_pattern ( filter, pch );
			pch = strtok ( NULL, " " );
		}

		gtk_recent_chooser_add_filter ( GTK_RECENT_CHOOSER ( menu ), filter );
	}

	return TCL_OK;
}


/**
\brief	Create a menu populated with details of recentley loaded files.
**/
static int menuRecentChooserFunc ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{
	static const char *cmds[] = { "delete", "configure", "class", "options", "commands", NULL };
	enum cmdIdx { DeleteIdx, ConfigureIdx, ClassIdx, OptionsIdx, CommandsIdx };

	GtkWidget *menu = GTK_WIDGET ( data );

	int idx;

	if ( objc < 2 )
	{
		Tcl_WrongNumArgs ( interp, 1, objv, "command" );
		return TCL_ERROR;
	}

	if ( Tcl_GetIndexFromObj ( interp, objv[1], cmds, "command", TCL_EXACT, &idx ) != TCL_OK )
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
				gnoclGetOptions ( interp, recentChooserOptions );
			}
			break;
		case ClassIdx:
			{
				Tcl_SetObjResult ( interp, Tcl_NewStringObj ( "menuRecentChooser", -1 ) );
			}
			break;
		case DeleteIdx:
			{
				return gnoclDelete ( interp, GTK_WIDGET ( menu ), objc, objv );
			}
		case ConfigureIdx:
			{
				int ret = gnoclParseAndSetOptions ( interp, objc - 1, objv + 1, recentChooserOptions, G_OBJECT ( menu ) );
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
	int       ret;
	GtkWidget *menu;

	GtkRecentManager *manager;

	if ( gnoclParseOptions ( interp, objc, objv, recentChooserOptions ) != TCL_OK )
	{
		gnoclClearOptions ( recentChooserOptions );
		return TCL_ERROR;
	}

	manager = gtk_recent_manager_get_default ();
	menu = gtk_recent_chooser_menu_new_for_manager ( manager );

	gtk_recent_chooser_set_show_not_found ( GTK_RECENT_CHOOSER ( menu ), FALSE );
	gtk_recent_chooser_set_local_only ( GTK_RECENT_CHOOSER ( menu ), TRUE );
	gtk_recent_chooser_set_limit ( GTK_RECENT_CHOOSER ( menu ), 10 );
	gtk_recent_chooser_set_sort_type ( GTK_RECENT_CHOOSER ( menu ), GTK_RECENT_SORT_MRU );

	gtk_widget_show ( menu );

	/*----------------------*/
	ret = gnoclSetOptions ( interp, recentChooserOptions, G_OBJECT ( menu ), -1 );

	if ( ret == TCL_OK )
	{
		ret = configure ( interp, menu, recentChooserOptions );
	}

	gnoclClearOptions ( recentChooserOptions );

	if ( ret != TCL_OK )
	{
		gtk_widget_destroy ( GTK_WIDGET ( menu ) );
		return TCL_ERROR;
	}

	/*----------------------*/

	return gnoclRegisterWidget ( interp, GTK_WIDGET ( menu ), menuFunc );
}

