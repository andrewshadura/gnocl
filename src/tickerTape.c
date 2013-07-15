/**
/brief	Create ticker-tape animated label.
**/

/**
\par Modification History
\verbatim
	2013-07: added commands, options, commands
\endverbatim
**/

#include "gnocl.h"
#include "./ticker/mymarquee.h"

/**
\brief
**/
static void  doOnCurveChanged ( GtkWidget *widget, GtkTreePath *path, gpointer user_data )
{
	GnoclCommandData *cs = ( GnoclCommandData * ) user_data;

	//if ( *cs->interp->result == '\0' )
	//{
	GnoclPercSubst ps[] =
	{
		{ 'w', GNOCL_STRING },  /* printiconview */
		{ 'g', GNOCL_STRING },  /* printiconview */
		{ 0 }
	};

	ps[0].val.str = gnoclGetNameFromWidget ( widget );
	ps[1].val.str = gtk_widget_get_name ( GTK_WIDGET ( widget ) );
	gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 );
	//}
}

/**
\brief      This is for all callbacks which have only %w as substitution
**/
static int gnoclOptOnCurveChanged ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	assert ( opt->type == GNOCL_OBJ );

	return gnoclConnectOptCmd ( interp, obj, opt->propName, G_CALLBACK ( doOnCurveChanged ), opt, NULL, ret );
}


/*
  "curve-type"               GtkCurveType          : Read / Write
  "max-x"                    gfloat                : Read / Write
  "max-y"                    gfloat                : Read / Write
  "min-x"                    gfloat                : Read / Write
  "min-y"                    gfloat                : Read / Write
*/

static const int textIdx = 0;

static GnoclOption tickerTapeOptions[] =
{
	{"-text", GNOCL_STRING, NULL},
	{ "-baseFont", GNOCL_OBJ, "Sans 14", gnoclOptGdkBaseFont },
	{ "-baseColor", GNOCL_OBJ, "normal", gnoclOptGdkColorBase },
	{ "-background", GNOCL_OBJ, "normal", gnoclOptGdkColorBg },
	/* custom commands */
	{ "-speed", GNOCL_INT, "speed" },    /* 0 */
	{ "-message", GNOCL_STRING, "message" }, /* 1 */
	{ NULL },
};

/**
\brief
\author     William J Giddings
\date       12-Feb-09
\since      0.9.94
**/
static int configure ( Tcl_Interp *interp, GtkWidget *widget, GnoclOption options[] )
{
#ifdef DEBUG_TICKERTAPE
	g_print ( "%s\n", __FUNCTION__ );
#endif

	if ( options[textIdx].status == GNOCL_STATUS_CHANGED )
	{
		char *str = options[textIdx].val.str;
		gtk_label_set_markup ( GTK_LABEL ( widget ) , str );
	}

	return TCL_OK;
}

/**
\brief
\author     William J Giddings
\date       12-Feb-09
\since      0.9.94
**/
static int cget ( Tcl_Interp *interp, GtkWidget *widget, GnoclOption options[], int idx )
{
	printf ( "cget\n" );

	return gnoclCgetNotImplemented ( interp, options + idx );
}

static const char *cmds[] =
{
	"set", "reset", "get",
	"delete", "configure",
	"cget", "class",
	NULL
};

/**
\brief
\author     William J Giddings
\date       12-Feb-09
\since      0.9.94
**/
static int tickerTapeFunc ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{
#ifdef DEBUG_CURVE
	listParameters ( objc, objv, __FUNCTION__ );
#endif


	enum cmdIdx
	{
		DeleteIdx, ConfigureIdx,
		CgetIdx, ClassIdx
	};

	GtkCurve *widget = GTK_WIDGET ( data );

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

				Tcl_SetObjResult ( interp, Tcl_NewStringObj ( "tickerTape", -1 ) );
			}
			break;
		case DeleteIdx:
			{
				return gnoclDelete ( interp, GTK_WIDGET ( widget ), objc, objv );
			}
		case ConfigureIdx:
			{
				int ret = TCL_ERROR;


				if ( gnoclParseAndSetOptions ( interp, objc - 1, objv + 1, tickerTapeOptions, G_OBJECT ( widget ) ) == TCL_OK )
				{


					ret = configure ( interp, widget, tickerTapeOptions );
				}

				gnoclClearOptions ( tickerTapeOptions );

				return ret;
			}

			break;

		case CgetIdx:
			{
				int     idx;

				switch ( gnoclCget ( interp, objc, objv, G_OBJECT ( widget ), tickerTapeOptions, &idx ) )
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
							return cget ( interp, widget, tickerTapeOptions, idx );
						}
				}
			}
	}

	return TCL_OK;
}

/**
\brief
\author     William J Giddings
\date       12-Feb-09
\since      0.9.94
**/
int gnoclTickerTapeCmd ( ClientData data, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[] )
{
	if ( gnoclGetCmdsAndOpts ( interp, cmds, tickerTapeOptions, objv, objc ) == TCL_OK )
	{
		return TCL_OK;
	}

	int            ret = TCL_OK;
	GtkWidget      *tickerTape;

	PangoFontDescription *fd;

	if ( 1 )
	{
		if ( gnoclParseOptions ( interp, objc, objv, tickerTapeOptions ) != TCL_OK )
		{
			gnoclClearOptions ( tickerTapeOptions );
			return TCL_ERROR;
		}
	}

	fd = pango_font_description_from_string ( "Monospace 30" );
	tickerTape = my_marquee_new() ;
	gtk_widget_modify_font ( tickerTape, fd );
	my_marquee_set_message ( MY_MARQUEE ( tickerTape ), "Gnocl Tcl/Gtk+ Bindings" );
	my_marquee_set_speed ( MY_MARQUEE ( tickerTape ), 10 );
	pango_font_description_free ( fd );

	g_timeout_add ( 150, ( GSourceFunc ) my_marquee_slide, ( gpointer ) tickerTape );


	gtk_widget_show ( GTK_WIDGET ( tickerTape ) );


	if ( 1 )
	{

		ret = gnoclSetOptions ( interp, tickerTapeOptions, G_OBJECT ( tickerTape ), -1 );


		if ( ret == TCL_OK )
		{
			ret = configure ( interp, G_OBJECT ( tickerTape ), tickerTapeOptions );
		}


	}

	gnoclClearOptions ( tickerTapeOptions );

	/* STEP 3)  -show the widget */

	if ( ret != TCL_OK )
	{
		gtk_widget_destroy ( GTK_WIDGET ( tickerTape ) );
		return TCL_ERROR;
	}

	/* STEP 4)  -everything has worked, register the widget with the Tcl interpretor. */
	return gnoclRegisterWidget ( interp, GTK_WIDGET ( tickerTape ), tickerTapeFunc );
}
