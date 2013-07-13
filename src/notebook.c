/*
 * $Id: notebook.c,v 1.11 2005/01/01 15:27:54 baum Exp $
 *
 * This file implements the notebook widget
 *
 * Copyright (c) 2001 - 2005 Peter G. Baum  http://www.dr-baum.net
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

/*
   History:
   2013-07	new option
			-data
			added extra percentage subtitution strings to -onPageAdded, -onPageRemoved
				%c child
				%n page number
				%d data
			added commands, options, commands
   2013-01	added options
    		-startWidget
			-endWidget
			-onDestroy
			-onReorderTab
			-onSwitchPage
			-onFocusTab
			-onFocusMove
			-onPageAdded
			-onPageRemoved
			-onChangeCurrentPage
   2010-07: added -groupId
   			      -onCreateWindow
   			      -tabBorder
			      -tabHBorder
				  -tabVBorder
   2008-10: added command, class
   2003-10: added removePage
   2002-08: switched from GnoclWidgetOptions to GnoclOption
            gtk+ 2.0 related cleanups
        11: made accelerators work
        09: underlined character: accelerators still don't work
   2001-05: Begin of developement
 */

/**
\page page_noteBook gnocl::noteBook
\htmlinclude noteBook.html
**/

#include "gnocl.h"

/*
	Register new window, and return with
*/
//static GtkNotebook* doOnCreateWindow (GtkNotebook *source_notebook, GtkWidget *child, gint x, gint y, gpointer data)
//static void doOnCreateWindow (GtkNotebook *source_notebook, GtkWidget *child, gint x, gint y, gpointer data)
static int doOnCreateWindow ( GtkNotebook *source_notebook, GtkWidget *child, gint x, gint y, gpointer data )
{
	GtkWidget *window, *notebook;

	window = gtk_window_new ( GTK_WINDOW_TOPLEVEL );
	gtk_window_set_deletable ( GTK_WINDOW ( window ), TRUE );

	notebook = gtk_notebook_new ();

	gtk_notebook_set_group ( GTK_NOTEBOOK ( notebook ), gtk_notebook_get_group ( source_notebook ) );

	gtk_container_add ( GTK_CONTAINER ( window ), notebook );

	gtk_window_set_default_size ( GTK_WINDOW ( window ), 300, 300 );
	gtk_window_move ( GTK_WINDOW ( window ), x, y );
	gtk_widget_show_all ( window );


	GnoclCommandData *cs = ( GnoclCommandData * ) data;

	//if ( *cs->interp->result == '\0' )
	//{
	GnoclPercSubst ps[] =
	{
		{ 'w', GNOCL_STRING },  /* window */
		{ 'g', GNOCL_STRING },  /* window gladeName */
		{ 'n', GNOCL_STRING },  /* notebook */
		{ 'c', GNOCL_STRING },  /* child */
		{ 's', GNOCL_STRING },  /* child */
		{ 'x', GNOCL_INT },     /* x coordinate */
		{ 'y', GNOCL_INT },     /* y coordinate */
		{ 0 }
	};

	/* register the new main window */
	const char *windowName = gnoclGetAutoWidgetId();
	gnoclMemNameAndWidget ( windowName, window );
	gnoclRegisterWidget ( cs->interp, window, windowFunc );


	/* register the new notebook */
	const char *notebookName = gnoclGetAutoWidgetId();
	gnoclMemNameAndWidget ( notebookName, notebook );
	gnoclRegisterWidget ( cs->interp, notebook, notebookFunc );

	ps[0].val.str = gnoclGetNameFromWidget ( window );
	ps[1].val.str = gtk_widget_get_name ( GTK_WIDGET ( window ) );
	ps[2].val.str = gnoclGetNameFromWidget ( notebook );
	ps[3].val.str = gnoclGetNameFromWidget ( child );
	ps[4].val.str = gnoclGetNameFromWidget ( notebook );
	ps[5].val.i = x;
	ps[6].val.i = y;

	gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 );
	//}

	return GTK_NOTEBOOK ( notebook );

}


static void _doOnCreateWindow ( GtkNotebook *notebook, GtkWidget *page, gint x, gint y, gpointer user_data )
{
	GnoclCommandData *cs = ( GnoclCommandData * ) user_data;

	/* if we have set the result, we are in the middle of
	   error handling. In this case don't call any callbacks
	   (especially onDestroy!) because this overrides the result. */

	//if ( *cs->interp->result == '\0' )
	//{
	GnoclPercSubst ps[] =
	{
		{ 'W', GNOCL_STRING },  /* notebook */
		{ 'G', GNOCL_STRING },  /* notebook gladeName */
		{ 'w', GNOCL_STRING },  /* widget */
		{ 'g', GNOCL_STRING },  /* widget gladename */
		{ 'x', GNOCL_INT },     /* x coordinate */
		{ 'y', GNOCL_INT },     /* y coordinate */
		{ 0 }
	};

	ps[0].val.str = gnoclGetNameFromWidget ( notebook );
	ps[1].val.str = gtk_widget_get_name ( GTK_WIDGET ( notebook ) );
	ps[2].val.str = gnoclGetNameFromWidget ( GTK_WIDGET ( page ) );
	ps[3].val.str = gtk_widget_get_name ( GTK_WIDGET ( page ) );
	ps[4].val.i = x;
	ps[5].val.i = y;

	gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 );
	//}
}

/**
\brief
**/
int gnoclOptOnCreateWindow ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
#ifdef DEBUG_NOTEBOOK
	g_print ( "%s\n", __FUNCTION__ );
#endif
	return gnoclConnectOptCmd ( interp, obj, "create-window", G_CALLBACK ( doOnCreateWindow ), opt, NULL, ret );
}

/**
\brief
**/
static void onReorderTabFunc ( GtkNotebook *notebook, GtkDirectionType arg1, gboolean arg2, gpointer user_data )
{

	GnoclCommandData *cs = ( GnoclCommandData * ) user_data;

	GnoclPercSubst ps[] =
	{
		{ 'w', GNOCL_STRING },  /* window */
		{ 0 }
	};

	ps[0].val.str = gnoclGetNameFromWidget ( notebook );

	gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 );

}


/**
\brief
**/
static void onFocusTabFunc ( GtkNotebook *notebook, GtkNotebookTab arg1, gpointer user_data )
{

	GnoclCommandData *cs = ( GnoclCommandData * ) user_data;


	GnoclPercSubst ps[] =
	{
		{ 'w', GNOCL_STRING },  /* window */
		{ 0 }
	};

	ps[0].val.str = gnoclGetNameFromWidget ( notebook );

	gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 );

}

/**
\brief
typedef enum {
  GTK_DIR_TAB_FORWARD,
  GTK_DIR_TAB_BACKWARD,
  GTK_DIR_UP,
  GTK_DIR_DOWN,
  GTK_DIR_LEFT,
  GTK_DIR_RIGHT
} GtkDirectionType;
**/
static void onFocusOutFunc ( GtkNotebook *notebook, GtkDirectionType arg1, gpointer user_data )
{

	GnoclCommandData *cs = ( GnoclCommandData * ) user_data;


	GnoclPercSubst ps[] =
	{
		{ 'w', GNOCL_STRING },  /* window */
		{ 0 }
	};

	ps[0].val.str = gnoclGetNameFromWidget ( notebook );

	gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 );

}


static void onPageAddedFunc ( GtkNotebook *notebook, GtkWidget *child, guint page_num, gpointer user_data )
{

	GnoclCommandData *cs = ( GnoclCommandData * ) user_data;


	GnoclPercSubst ps[] =
	{
		{ 'w', GNOCL_STRING },  /* window */
		{ 'c', GNOCL_STRING },  /* window */
		{ 'n', GNOCL_INT },  /* window */
		{ 'd', GNOCL_STRING },  /* window */
		{ 0 }
	};

	ps[0].val.str = gnoclGetNameFromWidget ( notebook );
	ps[1].val.str = gnoclGetNameFromWidget ( child );
	ps[2].val.i = page_num;
	ps[3].val.str = g_object_get_data ( G_OBJECT ( notebook ), "gnocl::data" );

	gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 );

}

static void onPageRemovedFunc ( GtkNotebook *notebook, GtkWidget *child, guint page_num, gpointer user_data )
{

	GnoclCommandData *cs = ( GnoclCommandData * ) user_data;


	GnoclPercSubst ps[] =
	{
		{ 'w', GNOCL_STRING },  /* window */
		{ 'c', GNOCL_STRING },  /* window */
		{ 'n', GNOCL_INT },  /* window */
		{ 'd', GNOCL_STRING },  /* window */
		{ 0 }
	};

	ps[0].val.str = gnoclGetNameFromWidget ( notebook );
	ps[1].val.str = gnoclGetNameFromWidget ( child );
	ps[2].val.i = page_num;
	ps[3].val.str = g_object_get_data ( G_OBJECT ( notebook ), "gnocl::data" );

	gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 );

}

/**
\brief
**/
static void onSelectPageFunc ( GtkNotebook *notebook, gboolean arg1, gpointer user_data )
{

	GnoclCommandData *cs = ( GnoclCommandData * ) user_data;


	GnoclPercSubst ps[] =
	{
		{ 'w', GNOCL_STRING },  /* window */
		{ 'd', GNOCL_STRING },  /* window */
		{ 0 }
	};

	ps[0].val.str = gnoclGetNameFromWidget ( notebook );
	ps[1].val.str = g_object_get_data ( G_OBJECT ( notebook ), "gnocl::data" );

	gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 );

}
/**
\brief	Assign a group name for the notebooks to allow drag and drop.
\note	requires Gtk 2.24
**/
/*
int gnoclOptGroupName ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{

	const gchar *group_name;

	group_name = Tcl_GetString ( opt->val.obj);

	gtk_notebook_set_group_name (GTK_NOTEBOOK ( obj ), group_name );

	return TCL_OK;
}
*/
/**
\brief	Assign a group Id for the notebooks to allow drag and drop.
**/
int gnoclOptGroupId ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{

	gint i;

	Tcl_GetIntFromObj ( NULL, opt->val.obj, &i );

	gtk_notebook_set_group_id ( GTK_NOTEBOOK ( obj ), i );


	/*
	const gchar *group_name;
	group_name = Tcl_GetString ( opt->val.obj);

		gtk_notebook_set_group_name (GTK_NOTEBOOK ( obj ), group_name );
	*/

	return TCL_OK;
}


static const int childrenIdx     = 0;
static const int onSwitchPageIdx = 1; // X
static const int onCreateWindowIdx = 2; // X
static const int onFocusTabIdx = 3; // X
static const int onFocusMoveIdx = 4; // X
static const int onPageAddedIdx = 5; // X
static const int onPageRemovedIdx = 6; //X
static const int onReorderTabIdx = 7; // X
static const int onSelectPageIdx = 8;
static const int startWidgetIdx = 9;
static const int endWidgetIdx = 10;
static const int onChangeCurrentPageIdx = 11;

static GnoclOption notebookOptions[] =
{
	/* widget specific options */
	{ "-children", GNOCL_LIST, NULL },        /* 0 */
	{ "-onSwitchPage", GNOCL_OBJ, NULL },     /* 1 */
	{ "-onCreateWindow", GNOCL_OBJ, "", gnoclOptOnCreateWindow},  /* 2 */
	{ "-onFocusTab", GNOCL_OBJ, NULL },       /* 3 */
	{ "-onFocusOut", GNOCL_OBJ, NULL },       /* 4 */
	{ "-onPageAdded", GNOCL_OBJ, NULL },      /* 5 */
	{ "-onPageRemoved", GNOCL_OBJ, NULL },    /* 6 */
	{ "-onReorderTab", GNOCL_OBJ, NULL },     /* 7 */
	{ "-onSelectPage", GNOCL_OBJ, NULL },     /* 8 */
	{ "-startWidget", GNOCL_OBJ, NULL },     /* 9 */
	{ "-endWidget", GNOCL_OBJ, NULL },     /* 10 */
	{ "-onChangeCurrentPage", GNOCL_OBJ, NULL },     /* 11 */

	{ "-data", GNOCL_OBJ, "", gnoclOptData },

	/* GtkNoteBook soecific properties */
	{ "-enablePopup", GNOCL_BOOL, "enable-popup" },
	{ "-group", GNOCL_OBJ, "", ""},
	{ "-groupId", GNOCL_OBJ, "", gnoclOptGroupId }, //*
	// { "-groupName", GNOCL_OBJ, "", gnoclOptGroupName }, /* requires ver. 2.24 */
	{ "-homogeneous", GNOCL_BOOL, "homogeneous" },
	{ "-page", GNOCL_INT, "page" },
	{ "-scrollable", GNOCL_BOOL, "scrollable" },
	{ "-showBorder", GNOCL_BOOL, "show-border" },
	{ "-showTabs", GNOCL_BOOL, "show-tabs" },
	{ "-tabBorder", GNOCL_INT, "show-border" }, //*
	{ "-tabHBorder", GNOCL_INT, "show-hborder" }, //*
	{ "-tabPosition", GNOCL_OBJ, "tab-pos", gnoclOptPosition },
	{ "-tabVBorder", GNOCL_INT, "show-hborder" }, //*


	/* inherited properties */
	{ "-tooltip", GNOCL_OBJ, "", gnoclOptTooltip },
	{ "-onShowHelp", GNOCL_OBJ, "", gnoclOptOnShowHelp },
	{ "-name", GNOCL_STRING, "name" },
	{ "-visible", GNOCL_BOOL, "visible" },
	{ "-sensitive", GNOCL_BOOL, "sensitive" },
	{ "-borderWidth", GNOCL_OBJ, "border-width", gnoclOptPadding },

	/* respond to widget destruction */
	{ "-onDestroy", GNOCL_OBJ, "destroy", gnoclOptCommand },

	{ NULL }

};

/**
\brief
**/
static void switchPageFunc ( GtkNotebook *notebook, GtkNotebookPage *page, gint page_num, gpointer data )
{
	GnoclCommandData *cs = ( GnoclCommandData * ) data;

	GnoclPercSubst ps[] =
	{
		{ 'w', GNOCL_STRING },  /* widget */
		{ 'p', GNOCL_INT },
		{ 0 }
	};
	ps[0].val.str = gnoclGetNameFromWidget ( GTK_WIDGET ( notebook ) );
	ps[1].val.i = page_num;
	gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 );
}

/**
\brief
**/
static GtkWidget *getLabel ( Tcl_Interp *interp, Tcl_Obj *obj )
{
	const char *name = Tcl_GetString ( obj );
	GtkWidget *widget = gnoclGetWidgetFromName ( name, NULL );

	if ( widget == NULL )
	{
		GnoclStringType type = gnoclGetStringType ( obj );
		widget = gtk_label_new ( gnoclGetString ( obj ) );
		gtk_label_set_use_markup ( GTK_LABEL ( widget ),
								   ( type & GNOCL_STR_MARKUP ) != 0 );
		gtk_label_set_use_underline ( GTK_LABEL ( widget ),
									  ( type & GNOCL_STR_UNDERLINE ) != 0 );
	}

	else if ( gnoclAssertNotPacked ( widget, interp, name ) )
		return NULL;

	return widget;
}

/**
\brief	Add new page to the notebook.
\notes	Need reworking to allow for page options.
**/

/*
  "detachable"               gboolean              : Read / Write
  "menu-label"               gchar*                : Read / Write
  "position"                 gint                  : Read / Write
  "reorderable"              gboolean              : Read / Write
  "tab-expand"               gboolean              : Read / Write
  "tab-fill"                 gboolean              : Read / Write
  "tab-label"                gchar*                : Read / Write
  "tab-pack"                 GtkPackType           : Read / Write

*/

//ret = gnoclParseAndSetOptions ( interp, objc - cmdNo - 1, objv + cmdNo + 1, pageOptions, G_OBJECT ( tag ) );

static int addPage ( GtkNotebook *notebook, Tcl_Interp *interp, Tcl_Obj *child, Tcl_Obj *label, Tcl_Obj *menu, int begin )
{
	GtkWidget *menuWidget;
	GtkWidget *labelWidget;
	GtkWidget *childWidget = gnoclChildNotPacked ( Tcl_GetString ( child ), interp );


	GnoclOption pageOptions[] =
	{
		{ "-detachable", GNOCL_STRING, "detachable" },
		{ "-menuLabel", GNOCL_STRING, "menu-label" },
		{ "-position", GNOCL_STRING, "position" },
		{ "-reorderable", GNOCL_STRING, "reorderable" },
		{ "-tabExpand", GNOCL_STRING, "tab-expand" },
		{ "-tabFill", GNOCL_STRING, "tab-fill" },
		{ "-tabLabel", GNOCL_STRING, "tab-label" },
		{ "-tabPack", GNOCL_STRING, "tab-pack" },
		{ NULL }
	};

	enum cmdIdx { DetachableIdx, MenuLabelIdx, Position, BeginIdx, EndIdx, ClassIdx };

	if ( childWidget == NULL )
		return -1;

	labelWidget = getLabel ( interp, label );

	if ( labelWidget == NULL )
		return -1;

	menuWidget = getLabel ( interp, menu ? menu : label );

	if ( menuWidget == NULL )
		return -1;

	if ( begin )
	{
		gtk_notebook_append_page_menu ( notebook, childWidget, labelWidget, menuWidget );
	}

	else
	{
		gtk_notebook_prepend_page_menu ( notebook, childWidget, labelWidget, menuWidget );
	}

	//ret = gnoclParseAndSetOptions ( interp, objc - cmdNo - 1, objv + cmdNo + 1, pageOptions, G_OBJECT ( tag ) );

	gtk_notebook_set_tab_detachable ( notebook, childWidget, 1 );
	gtk_notebook_set_tab_reorderable   ( notebook, childWidget, 1 );


	return gtk_notebook_page_num ( notebook, childWidget );
}


/**
\brief
    Description yet to be added.
**/
static int addChildren ( GtkNotebook *notebook, Tcl_Interp *interp, Tcl_Obj *children, int begin )
{
	int n, noChilds;

	if ( Tcl_ListObjLength ( interp, children, &noChilds ) != TCL_OK
			|| noChilds < 1 )
	{
		Tcl_SetResult ( interp, "widget-list must be proper list",
						TCL_STATIC );
		return TCL_ERROR;
	}

	for ( n = 0; n < noChilds; ++n )
	{
		Tcl_Obj   *subList, *child, *label;
		Tcl_Obj   *menu = NULL;
		int       noMem;

		if ( Tcl_ListObjIndex ( interp, children, n, &subList ) != TCL_OK )
		{
			return TCL_ERROR;
		}

		if ( Tcl_ListObjLength ( interp, subList, &noMem ) != TCL_OK
				|| ( noMem != 2 && noMem != 3 ) )
		{
			/* if it's not a list of lists, test, if it is a single list
			   with content and bookmark */
			if ( noMem == 1 && ( noChilds == 2 || noChilds == 3 ) )
			{
				noMem = noChilds;
				noChilds = 1;
				subList = children;
			}

			else
			{
				Tcl_SetResult ( interp, "list must consists of two or three elements: "
								"\"widget\" \"bookmark\" \"menu\"", TCL_STATIC );
				return TCL_ERROR;
			}
		}

		if ( Tcl_ListObjIndex ( interp, subList, 0, &child ) != TCL_OK )
		{
			return TCL_ERROR;
		}

		if ( Tcl_ListObjIndex ( interp, subList, 1, &label ) != TCL_OK )
		{
			return TCL_ERROR;
		}

		if ( noMem > 2 )
		{
			if ( Tcl_ListObjIndex ( interp, subList, 2, &menu ) != TCL_OK )
			{
				return TCL_ERROR;
			}
		}

		if ( addPage ( notebook, interp, child, label, menu, begin ) < 0 )
		{
			return TCL_ERROR;
		}
	}

	return TCL_OK;
}

/**
\brief

static const int childrenIdx     = 0;
static const int onSwitchPageIdx = 1;
static const int onCreateWindowIdx = 2;
static const int onFocusTabIdx = 3;
static const int onFocusOut = 4;
static const int onPageAdded = 5;
static const int onPageRemoved = 6;
static const int onReorderTabIdx = 7;
static const int onSelectPage = 8;

**/

static int configure ( Tcl_Interp *interp, GtkNotebook *notebook, GnoclOption options[] )
{
	if ( options[childrenIdx].status == GNOCL_STATUS_CHANGED )
	{
		int ret = addChildren ( notebook, interp, options[childrenIdx].val.obj, 1 );

		if ( ret != TCL_OK )
		{
			return ret;
		}
	}


	/*
	typedef enum {
	  GTK_PACK_START,
	  GTK_PACK_END
	} GtkPackType;
	*/

	if ( options[startWidgetIdx].status == GNOCL_STATUS_CHANGED )
	{
		GtkWidget *widget = NULL;
		widget =  gnoclGetWidgetFromName ( Tcl_GetString ( options[startWidgetIdx].val.obj ), interp );

		gtk_notebook_set_action_widget ( G_OBJECT ( notebook ), widget, GTK_PACK_START );
	}

	if ( options[endWidgetIdx].status == GNOCL_STATUS_CHANGED )
	{
		GtkWidget *widget = NULL;
		widget =  gnoclGetWidgetFromName ( Tcl_GetString ( options[endWidgetIdx].val.obj ), interp );

		gtk_notebook_set_action_widget ( G_OBJECT ( notebook ), widget, GTK_PACK_END );
	}

	if ( options[onFocusTabIdx].status == GNOCL_STATUS_CHANGED )
	{
		if ( gnoclConnectOptCmd ( interp, G_OBJECT ( notebook ), "focus-tab" , G_CALLBACK ( onFocusTabFunc ), options + onFocusTabIdx, NULL, NULL ) != TCL_OK )
		{
			return TCL_ERROR;
		}
	}

	if ( options[onFocusMoveIdx].status == GNOCL_STATUS_CHANGED )
	{
		if ( gnoclConnectOptCmd ( interp, G_OBJECT ( notebook ), "move-focus-out" , G_CALLBACK ( onFocusOutFunc ), options + onFocusMoveIdx, NULL, NULL ) != TCL_OK )
		{
			return TCL_ERROR;
		}
	}

	if ( options[onPageAddedIdx].status == GNOCL_STATUS_CHANGED )
	{
		if ( gnoclConnectOptCmd ( interp, G_OBJECT ( notebook ), "page-added" , G_CALLBACK ( onPageAddedFunc ), options + onPageAddedIdx, NULL, NULL ) != TCL_OK )
		{
			return TCL_ERROR;
		}
	}

	if ( options[onPageRemovedIdx].status == GNOCL_STATUS_CHANGED )
	{
		if ( gnoclConnectOptCmd ( interp, G_OBJECT ( notebook ), "page-removed" , G_CALLBACK ( onPageRemovedFunc ), options + onPageRemovedIdx, NULL, NULL ) != TCL_OK )
		{
			return TCL_ERROR;
		}
	}

	if ( options[onReorderTabIdx].status == GNOCL_STATUS_CHANGED )
	{
		if ( gnoclConnectOptCmd ( interp, G_OBJECT ( notebook ), "page-reordered", G_CALLBACK ( onReorderTabFunc ), options + onReorderTabIdx, NULL, NULL ) != TCL_OK )
		{
			return TCL_ERROR;
		}
	}

	if ( options[onSwitchPageIdx].status == GNOCL_STATUS_CHANGED )
	{
		if ( gnoclConnectOptCmd ( interp, G_OBJECT ( notebook ), "switch-page", G_CALLBACK ( switchPageFunc ), options + onSwitchPageIdx, NULL, NULL ) != TCL_OK )
		{
			return TCL_ERROR;
		}
	}

	if ( options[onSelectPageIdx].status == GNOCL_STATUS_CHANGED )
	{
		if ( gnoclConnectOptCmd ( interp, G_OBJECT ( notebook ), "select-page", G_CALLBACK ( switchPageFunc ), options + onSelectPageIdx, NULL, NULL ) != TCL_OK )
		{
			return TCL_ERROR;
		}
	}

	if ( options[onChangeCurrentPageIdx].status == GNOCL_STATUS_CHANGED )
	{
		if ( gnoclConnectOptCmd ( interp, G_OBJECT ( notebook ), "change-current-page", G_CALLBACK ( switchPageFunc ), options + onSelectPageIdx, NULL, NULL ) != TCL_OK )
		{
			return TCL_ERROR;
		}
	}

	return TCL_OK;
}

/**
\brief
**/
static int notebookNext ( GtkNotebook *notebook, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[], int isNext )
{
	int cur = gtk_notebook_get_current_page ( notebook );
	int num = 1;

	/* widget next ?count? */

	if ( objc < 2 || objc > 3 )
	{
		Tcl_WrongNumArgs ( interp, 2, objv, "?count?" );
		return TCL_ERROR;
	}

	if ( objc == 3 && Tcl_GetIntFromObj ( interp, objv[2], &num ) != TCL_OK )
		return TCL_ERROR;

	if ( isNext )
	{
		cur = gtk_notebook_get_current_page ( notebook );
		gtk_notebook_set_current_page ( notebook, cur + num );
	}

	else if ( objc == 3 ) /* current val */
		gtk_notebook_set_current_page ( notebook, num );

	cur = gtk_notebook_get_current_page ( notebook );

	Tcl_SetObjResult ( interp, Tcl_NewIntObj ( cur ) );

	return TCL_OK;
}


/**
\brief
**/
int notebookFunc ( ClientData data,	Tcl_Interp *interp,	int objc, Tcl_Obj * const objv[] )
{
	/* TODO?: notebook insert pos child label ?menu? */
	static const char *cmds[] = { "delete", "configure", "addPage", "currentPage", "nextPage", "removePage", "class", "options", "commands", NULL };
	enum cmdIdx { DeleteIdx, ConfigureIdx, AddPageIdx, CurrentIdx, NextPageIdx, RemovePageIdx, ClassIdx, OptionsIdx, CommandsIdx };

	GtkNotebook *notebook = GTK_NOTEBOOK ( data );
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
		case CommandsIdx:
			{
				gnoclGetOptions ( interp, cmds );
			}
			break;
		case OptionsIdx:
			{
				gnoclGetOptions ( interp, notebookOptions );
			}
			break;
		case ClassIdx:
			Tcl_SetObjResult ( interp, Tcl_NewStringObj ( "notebook", -1 ) );
			break;
		case DeleteIdx:
			return gnoclDelete ( interp, GTK_WIDGET ( notebook ), objc, objv );

		case ConfigureIdx:
			{
				int ret = TCL_ERROR;

				if ( gnoclParseAndSetOptions ( interp, objc - 1, objv + 1,
											   notebookOptions, G_OBJECT ( notebook ) ) == TCL_OK )
				{
					ret = configure ( interp, notebook, notebookOptions );
				}

				gnoclClearOptions ( notebookOptions );

				return ret;
			}

			break;
		case AddPageIdx:
			{
				int ret;

				/* ID addPage child tab ?label? */

				if ( objc < 4 || objc > 5 )
				{
					Tcl_WrongNumArgs ( interp, 2, objv, "child label ?menu?" );
					return TCL_ERROR;
				}

				ret = addPage ( notebook, interp, objv[2], objv[3], objc < 5 ? NULL : objv[4], 1 );

				if ( ret < 0 )
				{
					return TCL_ERROR;
				}

				/* TODO? associate an ID to every page? */
				Tcl_SetObjResult ( interp, Tcl_NewIntObj ( ret ) );

				return TCL_OK;
			}

		case CurrentIdx:
		case NextPageIdx:
			return notebookNext ( notebook, interp, objc, objv, idx == NextPageIdx );
		case RemovePageIdx:
			{
				int k;

				if ( objc != 3 )
				{
					Tcl_WrongNumArgs ( interp, 2, objv, "pageNumber" );
					return TCL_ERROR;
				}

				if ( Tcl_GetIntFromObj ( interp, objv[2], &k ) != TCL_OK )
					return TCL_ERROR;

				gtk_notebook_remove_page ( notebook, k );

				return TCL_OK;
			}

	}

	return TCL_OK;
}

/**
\brief
**/
int gnoclNotebookCmd ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{
	int ret;
	GtkWidget *widget;

	if ( gnoclParseOptions ( interp, objc, objv, notebookOptions )
			!= TCL_OK )
	{
		gnoclClearOptions ( notebookOptions );
		return TCL_ERROR;
	}

	widget = gtk_notebook_new();

	ret = gnoclSetOptions ( interp, notebookOptions, G_OBJECT ( widget ), -1 );

	if ( ret == TCL_OK )
		ret = configure ( interp, GTK_NOTEBOOK ( widget ), notebookOptions );

	gnoclClearOptions ( notebookOptions );

	if ( ret != TCL_OK )
	{
		gtk_widget_destroy ( widget );
		return TCL_ERROR;
	}

	gtk_widget_show ( widget );

	return gnoclRegisterWidget ( interp, widget, notebookFunc );
}
