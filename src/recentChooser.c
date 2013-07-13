/**
\brief      This module implements the gnocl::recentChooser widget.
\author     William J Giddings
\since      20/12/09
**/

/**
\page page_recentChooser gnocl::recentChooser
\htmlinclude recentChooser.html
**/

/**
 \par Modification History
 \verbatim
	2013-07: added commands, options, commands
	2009-12: Began developement
 \endverbatim
**/

#include "gnocl.h"

static int gnoclOptOnSelectionChanged ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret );
static int gnoclOptOnItemActivated ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret );

/*
  "filter"                   GtkRecentFilter*      : Read / Write
  "limit"                    gint                  : Read / Write
  "local-only"               gboolean              : Read / Write
  "recent-manager"           GtkRecentManager*     : Write / Construct Only
  "select-multiple"          gboolean              : Read / Write
  "show-icons"               gboolean              : Read / Write
  "show-not-found"           gboolean              : Read / Write
  "show-private"             gboolean              : Read / Write
  "show-tips"                gboolean              : Read / Write
  "sort-type"                GtkRecentSortType     : Read / Write

  "item-activated"                                 : Run Last
  "selection-changed"                              : Run Last


*/

/*
	{"-filter", GNOCL_STRING, NULL},
x	{"-limit", GNOCL_INT, NULL},
x	{"-localOnly", GNOCL_BOOL, NULL},
	{"-recentManger", GNOCL_STRING, NULL},
x	{"-selectMultiple", GNOCL_BOOL, NULL},
x	{"-showIcons", GNOCL_BOOL, NULL},
x	{"-showNotFound", GNOCL_BOOL, NULL},
x	{"-showPrivate", GNOCL_BOOL, NULL},
x	{"-selectMultiple", GNOCL_BOOL, NULL},
x	{"-selectTips", GNOCL_BOOL, NULL},
	{"-sortType", GNOCL_STRING, NULL},
	{ "-name", GNOCL_STRING, "name" },
*/


static GnoclOption recentChooserOptions[] =
{
	{"-limit", GNOCL_INT, "limit"},						/* default = -1 (ie all) */
	{"-localOnly", GNOCL_BOOL,  "local-only"},			/* default = TRUE */
	{"-selectMultiple", GNOCL_BOOL, "select-multiple"},	/* default = FALSE */
	{"-selectTips", GNOCL_BOOL, "show-tips"},			/* default = FALSE */
	{"-showIcons", GNOCL_BOOL, "show-icons"},			/* default = TRUE */
	{"-showNotFound", GNOCL_BOOL, "show-not-found"},	/* default = TRUE */
	{"-showPrivate", GNOCL_BOOL, "show-private"}, 		/* default = FALSE */
	{"-onItemClick", GNOCL_OBJ, "item-activated", gnoclOptOnItemActivated},
	{"-onChanged", GNOCL_OBJ, "selection-changed", gnoclOptOnSelectionChanged},
	{"-tooltip", GNOCL_OBJ, "", gnoclOptTooltip },
	{ NULL },
};

/*
	// GtkRecentChooserMenu only
	{"-showNumbers", GNOCL_OBJ, "", gnoclOptShowNumbers}, 	// default = FALSE
*/

static int gnoclOptShowNumbers ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	//gtk_recent_chooser_menu_set_show_numbers (GtkRecentChooserMenu *menu, gboolean show_numbers);
	gtk_recent_chooser_menu_set_show_numbers ( obj, 1 );
	return TCL_OK;
}

/**
\brief
**/
static void doOnSelectionChanged ( GtkRecentChooser *chooser, gpointer data )
{

	GnoclCommandData *cs = ( GnoclCommandData * ) data;
	GtkRecentInfo *info;


	info = gtk_recent_chooser_get_current_item ( chooser );

	GnoclPercSubst ps[] =
	{
		{ 'w', GNOCL_STRING },  /* widget */
		{ 'f', GNOCL_STRING },  /* widget */
		{ 'l', GNOCL_STRING },  /* widget */
		{ 0 }
	};

	ps[0].val.str = gnoclGetNameFromWidget ( chooser );
	ps[1].val.str =  gtk_recent_chooser_get_current_uri  ( chooser );

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
static int gnoclOptOnSelectionChanged ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{

	return gnoclConnectOptCmd ( interp, obj, "selection-changed", G_CALLBACK ( doOnSelectionChanged ), opt, NULL, ret );
}

/**
\brief
**/
static int gnoclOptOnItemActivated ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{

	return gnoclConnectOptCmd ( interp, obj, "item-activated", G_CALLBACK ( doOnSelectionChanged ), opt, NULL, ret );
}


/**
\brief
**/
static int configure ( Tcl_Interp *interp, GtkWidget *widget, GnoclOption options[] )
{
#ifdef DEBUG_RECENTCHOOSER
	printf ( "configure\n" );
#endif

	return TCL_OK;
}

/**
\brief
**/
static int cget ( Tcl_Interp *interp, GtkWidget *widget, GnoclOption options[], int idx )
{
	printf ( "cget\n" );

	const char *dataIDa = "gnocl::data1";
	const char *dataIDb = "gnocl::data2";

	GtkWidget *arrow = gnoclFindChild ( GTK_WIDGET ( widget ), GTK_TYPE_BUTTON );
	char *str;

	/* this will enable us to obtain widget data for the arrow object */
	char *result = g_object_get_data ( G_OBJECT ( arrow ), dataIDa );
	printf ( "cget result = %s\n", result );

	result = g_object_get_data ( G_OBJECT ( arrow ), dataIDb );
	printf ( "cget result = %s\n", result );

	str = gnoclGetNameFromWidget ( arrow );
	printf ( "configure %s\n", str );


	/*-----*/
	/*
	    Tcl_Obj *obj = NULL;

	    if ( idx == textIdx )
	    {
	        obj = gnoclCgetButtonText ( interp, widget );
	    }

	    else if ( idx == iconsIdx )
	    {
	        GtkWidget *image = gnoclFindChild ( GTK_WIDGET ( widget ), GTK_TYPE_IMAGE );

	        if ( image == NULL )
	            obj = Tcl_NewStringObj ( "", 0 );
	        else
	        {
	            gchar   *st;
	            g_object_get ( G_OBJECT ( image ), "stock", &st, NULL );

	            if ( st )
	            {
	                obj = Tcl_NewStringObj ( "%#", 2 );
	                Tcl_AppendObjToObj ( obj, gnoclGtkToStockName ( st ) );
	                g_free ( st );
	            }

	            else
	            {
	                Tcl_SetResult ( interp, "Could not determine icon type.",
	                                TCL_STATIC );
	                return TCL_ERROR;
	            }
	        }
	    }

	    if ( obj != NULL )
	    {
	        Tcl_SetObjResult ( interp, obj );
	        return TCL_OK;
	    }
	*/


	return gnoclCgetNotImplemented ( interp, options + idx );
}

/**
\brief
**/
int recentChooserFunc ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{
	printf ( "widgetFunc\n" );

	static const char *cmds[] = { "delete", "configure", "cget", "onClicked", "class", "options", "commands", NULL };
	enum cmdIdx { DeleteIdx, ConfigureIdx, CgetIdx, OnClickedIdx, ClassIdx, OptionsIdx, CommandsIdx };
	GtkWidget *widget = GTK_WIDGET ( data );
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
				gnoclGetOptions ( interp, recentChooserOptions );
			}
			break;
		case ClassIdx:
			{

				Tcl_SetObjResult ( interp, Tcl_NewStringObj ( "recentChooser", -1 ) );
			}
			break;
		case DeleteIdx:
			{
				return gnoclDelete ( interp, GTK_WIDGET ( widget ), objc, objv );
			}
			break;
		case ConfigureIdx:
			{
				int ret = TCL_ERROR;

				ret = configure ( interp, widget, recentChooserOptions );

				if ( 1 )
				{
					if ( gnoclParseAndSetOptions ( interp, objc - 1, objv + 1, recentChooserOptions, G_OBJECT ( widget ) ) == TCL_OK )
					{
						ret = configure ( interp, widget, recentChooserOptions );
					}
				}

				gnoclClearOptions ( recentChooserOptions );

				return ret;
			}

			break;
		case OnClickedIdx:

			if ( objc != 2 )
			{
				Tcl_WrongNumArgs ( interp, 2, objv, NULL );
				return TCL_ERROR;
			}

			if ( GTK_WIDGET_IS_SENSITIVE ( GTK_WIDGET ( widget ) ) )
			{
				gtk_button_clicked ( widget );
			}

			break;

		case CgetIdx:
			{
				int     idx;

				switch ( gnoclCget ( interp, objc, objv, G_OBJECT ( widget ), recentChooserOptions, &idx ) )
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
							return cget ( interp, widget, recentChooserOptions, idx );
						}
				}
			}
	}

	return TCL_OK;
}

/**
\brief
**/
int gnoclRecentChooserCmd ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{

	int            ret = TCL_OK;
	GtkWidget      *widget;

	if ( gnoclParseOptions ( interp, objc, objv, recentChooserOptions ) != TCL_OK )
	{
		gnoclClearOptions ( recentChooserOptions );
		return TCL_ERROR;
	}

	widget = gtk_recent_chooser_widget_new() ;

	gtk_widget_show ( widget );


	if ( configure ( interp, widget , recentChooserOptions ) != TCL_OK )
	{
		gtk_widget_destroy ( widget );
		return TCL_ERROR;
	}

	/* STEP 4)  -everything has worked, register the widget with the Tcl interpretor. */
	return gnoclRegisterWidget ( interp, widget , recentChooserFunc );
}


