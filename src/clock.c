/**
/brief	Create clock widget.
**/

#include "gnocl.h"
#include "./clock/gtkclock.h"

static const int textIdx = 0;

static GnoclOption clockOptions[] =
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
static int configure ( Tcl_Interp *interp, GtkCurve *widget, GnoclOption options[] )
{
#ifdef DEBUG_CLOCK
	g_print ( "%s\n", __FUNCTION__ );
#endif

	if ( options[textIdx].status == GNOCL_STATUS_CHANGED )
	{
		char *str = options[textIdx].val.str;
		//gtk_label_set_markup ( widget, str );
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

/**
\brief
\author     William J Giddings
\date       12-Feb-09
\since      0.9.94
**/
static int clockFunc ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{
#ifdef DEBUG_CLOCK
	listParameters ( objc, objv, "clockFunc" );
#endif

	static const char *cmds[] =
	{
		"set", "reset", "get",
		"delete", "configure",
		"cget", "class",
		NULL
	};
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
				//printf ( "Class\n" );
				Tcl_SetObjResult ( interp, Tcl_NewStringObj ( "clock", -1 ) );
			}
			break;
		case DeleteIdx:
			{
				return gnoclDelete ( interp, GTK_WIDGET ( widget ), objc, objv );
			}
		case ConfigureIdx:
			{
				int ret = TCL_ERROR;
				printf ( "Configure\n" );

				if ( gnoclParseAndSetOptions ( interp, objc - 1, objv + 1, clockOptions, G_OBJECT ( widget ) ) == TCL_OK )
				{

					printf ( "Configure\n" );
					ret = configure ( interp, widget, clockOptions );
				}

				gnoclClearOptions ( clockOptions );

				return ret;
			}

			break;

		case CgetIdx:
			{
				int     idx;

				switch ( gnoclCget ( interp, objc, objv, G_OBJECT ( widget ), clockOptions, &idx ) )
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
							return cget ( interp, widget, clockOptions, idx );
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
int gnoclClockCmd ( ClientData data, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[] )
{

	int            ret = TCL_OK;
	GtkWidget      *clock;

	PangoFontDescription *fd;

	if ( 1 )
	{
		if ( gnoclParseOptions ( interp, objc, objv, clockOptions ) != TCL_OK )
		{
			gnoclClearOptions ( clockOptions );
			return TCL_ERROR;
		}
	}

	fd = pango_font_description_from_string ( "Monospace 30" );
	clock = gtk_clock_new() ;
	
	gtk_widget_show ( GTK_WIDGET ( clock ) );

	if ( 1 )
	{

		ret = gnoclSetOptions ( interp, clockOptions, G_OBJECT ( clock ), -1 );

		if ( ret == TCL_OK )
		{
			ret = configure ( interp, G_OBJECT ( clock ), clockOptions );
		}

	}

	gnoclClearOptions ( clockOptions );

	/* STEP 3)  -show the widget */

	if ( ret != TCL_OK )
	{
		gtk_widget_destroy ( GTK_WIDGET ( clock ) );
		return TCL_ERROR;
	}

	/* STEP 4)  -everything has worked, register the widget with the Tcl interpretor. */
	return gnoclRegisterWidget ( interp, GTK_WIDGET ( clock ), clockFunc );
}
