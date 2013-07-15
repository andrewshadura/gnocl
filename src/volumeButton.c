
/**
\page page_volumeButton gnocl::volumeButton
\htmlinclude volumeButton.html
**/
/**
\par Modification History
\verbatim
	2013-07: added commands, options, commands
\endverbatim
**/
#include "gnocl.h"

/**
\brief
**/
static gboolean doOnPop ( GtkScaleButton *button, gpointer user_data )
{
	GnoclCommandData *cs = ( GnoclCommandData * ) user_data;

	//if ( *cs->interp->result == '\0' )
	//{
	GnoclPercSubst ps[] =
	{
		{ 'w', GNOCL_STRING },
		{ 'g', GNOCL_STRING },  /* glade name */
		{ 'v', GNOCL_DOUBLE },  /* glade name */
		{ 0 }
	};

	ps[0].val.str = gnoclGetNameFromWidget ( button );
	ps[1].val.str = gtk_widget_get_name ( GTK_WIDGET ( button ) );
	ps[2].val.d = gtk_scale_button_get_value ( button );
	gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 );
	//}
}

/**
\brief      This is for all callbacks which have only %w as substitution
**/
static int gnoclOptOnPop ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	assert ( opt->type == GNOCL_OBJ );
	return gnoclConnectOptCmd ( interp, obj, opt->propName, G_CALLBACK ( doOnPop ), opt, NULL, ret );

//	assert ( strcmp ( opt->propName, "-onPopup" ) == 0 || strcmp ( opt->propName, "-onPopdown" ) == 0 );
//	return gnoclConnectOptCmd ( interp, obj, opt->optName[3] == 'U' ? "popup" : "popdown", G_CALLBACK ( doOnPop ), opt, NULL, ret );

}



static const int orientationIdx = 0;

GnoclOption volumeButtonOptions[] =
{
	/* Properties inherited from GtkScaleButton*/
	{ "-value", GNOCL_DOUBLE, "value"},

	/* Other utltility options */
	{ "-data", GNOCL_OBJ, "", gnoclOptData },
	{ "-name", GNOCL_STRING, "name" },

	/* Signal handlers */
	{ "-onPopdown", GNOCL_OBJ, "popdown", gnoclOptOnPop }, 	// press Escape
	{ "-onPopup", GNOCL_OBJ, "popup", gnoclOptOnPop }, 		// press Space, Enter and Return.
	{ "-onValueChanged", GNOCL_OBJ, "value-changed", gnoclOptOnValueChanged }, // as in parseOptions.c

	/* inherited properties */
	{ "-normalBackgroundColor", GNOCL_OBJ, "normal", gnoclOptGdkColorBg },
	{ "-prelightBackgroundColor", GNOCL_OBJ, "prelight", gnoclOptGdkColorBg }, //yes
	{ "-borderWidth", GNOCL_OBJ, "border-width", gnoclOptPadding },
	{ "-sensitive", GNOCL_BOOL, "sensitive" },
	{ "-visible", GNOCL_BOOL, "visible" },

	{ NULL },
};



/**
\brief
**/
static int configure ( Tcl_Interp *interp, GtkWidget *widget, GnoclOption options[] )
{

	/*
		// template
		if ( options[selectionModeIdx].status == GNOCL_STATUS_CHANGED )
		{
		#ifdef DEBUG_INFOBAR
			g_print ( "CONFIGURE -> selectionMode -> %s\n", options[selectionModeIdx].val.str );
		#endif
		}
	*/

	if ( options[orientationIdx].status == GNOCL_STATUS_CHANGED )
	{
#ifdef DEBUG_VOLUME_BUTTON
		g_print ( "CONFIGURE -> orientation -> %s\n", options[typeIdx].val.str );
#endif

		/*
		  GTK_ICON_SIZE_INVALID,
		  GTK_ICON_SIZE_MENU,
		  GTK_ICON_SIZE_SMALL_TOOLBAR,
		  GTK_ICON_SIZE_LARGE_TOOLBAR,
		  GTK_ICON_SIZE_BUTTON,
		  GTK_ICON_SIZE_DND,
		  GTK_ICON_SIZE_DIALOG
		*/

		const char *sizes[] =
		{
			"horizontal", "vertical", NULL
		};

		gint idx;

		getIdx ( sizes, options[orientationIdx].val.str, &idx );

		switch ( idx )
		{
			case GTK_ORIENTATION_HORIZONTAL:
			case GTK_ORIENTATION_VERTICAL:
				{
					gtk_scale_button_set_orientation ( widget, GTK_ORIENTATION_HORIZONTAL );
				} break;
			default:
				{
					return TCL_ERROR;

				}
		}
	}


	return TCL_OK;
}

/**
\brief

**/
int gnoclConfigVolumeButtonText ( Tcl_Interp *interp, GtkWidget *widget,
								  Tcl_Obj *txtObj )
{
	GnoclStringType type = gnoclGetStringType ( txtObj );

	if ( type & GNOCL_STR_STOCK )
	{
		GtkStockItem sp;

		if ( gnoclGetStockItem ( txtObj, interp, &sp ) != TCL_OK )
			return TCL_ERROR;

		gtk_button_set_label ( widget, sp.stock_id );

		gtk_button_set_use_stock ( widget, 1 );
	}

	else
	{
		GtkLabel *label;
		char *txt = gnoclGetString ( txtObj );

		gtk_button_set_label ( widget, txt );
		gtk_button_set_use_stock ( widget, 0 );

		label = GTK_LABEL ( gnoclFindChild ( GTK_WIDGET ( widget ),
											 GTK_TYPE_LABEL ) );
		assert ( label );
		/* TODO? pango_parse_markup for error message */
		gtk_label_set_use_markup ( label, ( type & GNOCL_STR_MARKUP ) != 0 );
		gtk_label_set_use_underline ( label, ( type & GNOCL_STR_UNDERLINE ) != 0 );
	}

	return TCL_OK;
}

/**
\brief
**/
Tcl_Obj *gnoclCgetVolumeButtonText ( Tcl_Interp *interp, GtkWidget *widget )
{
	Tcl_Obj *obj = NULL;

	if ( gtk_button_get_use_stock ( widget ) )
	{
		const char *st = gtk_button_get_label ( widget );

		if ( st == NULL )
			obj = Tcl_NewStringObj ( "", 0 );
		else
		{
			obj = Tcl_NewStringObj ( "%#", 2 );
			Tcl_AppendObjToObj ( obj, gnoclGtkToStockName ( st ) );
		}
	}

	else
	{
		GtkLabel *label = GTK_LABEL ( gnoclFindChild ( GTK_WIDGET ( widget ),
									  GTK_TYPE_LABEL ) );
		Tcl_Obj *old = Tcl_NewStringObj ( gtk_label_get_label ( label ), -1 );
		assert ( label );

		if ( gtk_label_get_use_markup ( label ) )
		{
			obj = Tcl_NewStringObj ( "%<", 2 );
			Tcl_AppendObjToObj ( obj, old );
		}

		else if ( gtk_label_get_use_underline ( label ) )
		{
			obj = Tcl_NewStringObj ( "%_", 2 );
			Tcl_AppendObjToObj ( obj, old );
		}

		else
			obj = old;
	}

	return obj;
}

/**
\brief
**/
static int cget ( Tcl_Interp *interp, GtkWidget *widget, GnoclOption options[], int idx )
{


	return gnoclCgetNotImplemented ( interp, options + idx );
}

static const char *cmds[] = { "delete", "configure", "cget", "class", NULL };

/**
\brief
**/
int volumeButtonFunc ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{

	enum cmdIdx { DeleteIdx, ConfigureIdx, CgetIdx, OnClickedIdx, ClassIdx};
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

		case ClassIdx:
			{
				Tcl_SetObjResult ( interp, Tcl_NewStringObj ( "volumeButton", -1 ) );
			}
			break;
		case DeleteIdx:
			{
				return gnoclDelete ( interp, GTK_WIDGET ( widget ), objc, objv );
			}
		case ConfigureIdx:
			{
				int ret = TCL_ERROR;

				if ( gnoclParseAndSetOptions ( interp, objc - 1, objv + 1, volumeButtonOptions, G_OBJECT ( widget ) ) == TCL_OK )
				{
					ret = configure ( interp, widget, volumeButtonOptions );
				}

				gnoclClearOptions ( volumeButtonOptions );

				return ret;
			}

			break;

		case CgetIdx:
			{
				int     idx;

				switch ( gnoclCget ( interp, objc, objv, G_OBJECT ( widget ), volumeButtonOptions, &idx ) )
				{
					case GNOCL_CGET_ERROR:
						return TCL_ERROR;
					case GNOCL_CGET_HANDLED:
						return TCL_OK;
					case GNOCL_CGET_NOTHANDLED:
						return cget ( interp, widget, volumeButtonOptions, idx );
				}
			}
	}

	return TCL_OK;
}

/**
\brief
**/
int gnoclVolumeButtonCmd ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{
	if ( gnoclGetCmdsAndOpts ( interp, cmds, volumeButtonOptions, objv, objc ) == TCL_OK )
	{
		return TCL_OK;
	}

	int       ret;
	GtkWidget *widget;

	if ( gnoclParseOptions ( interp, objc, objv, volumeButtonOptions ) != TCL_OK )
	{
		gnoclClearOptions ( volumeButtonOptions );
		return TCL_ERROR;
	}

	widget = gtk_volume_button_new () ;

	gtk_widget_show ( GTK_WIDGET ( widget ) );

	ret = gnoclSetOptions ( interp, volumeButtonOptions, G_OBJECT ( widget ), -1 );

	if ( ret == TCL_OK )
	{
		ret = configure ( interp, widget, volumeButtonOptions );
	}

	gnoclClearOptions ( volumeButtonOptions );

	if ( ret != TCL_OK )
	{
		gtk_widget_destroy ( GTK_WIDGET ( widget ) );
		return TCL_ERROR;
	}

	return gnoclRegisterWidget ( interp, GTK_WIDGET ( widget ), volumeButtonFunc );
}
