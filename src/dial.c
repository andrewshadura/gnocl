/*
   History:
   2011-04: Begin of developement
 */

/**
\page page_dial gnocl::dial
\htmlinclude dial.html
**/

#include "gnocl.h"
#include <string.h>
#include <assert.h>
#include "./dial/gtkdial.h"

static GnoclOption dialOptions[] =
{
	/* widget specific options */
	{ "-active", GNOCL_BOOL, "active" },

	/* general options */
	{ "-name", GNOCL_STRING, "name" },
	{ "-visible", GNOCL_BOOL, "visible" },
	{ "-tooltip", GNOCL_OBJ, "", gnoclOptTooltip },
	{ NULL },
};



/**
\brief
**/
static int configure ( Tcl_Interp *interp, GtkDial *dial, GnoclOption options[] )
{
	return TCL_OK;
}

/**
\brief
**/
int dialFunc ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{
	static const char *cmds[] =
	{
		"delete", "configure", "class", "start", "stop",
		NULL
	};

	enum cmdIdx
	{
		DeleteIdx, ConfigureIdx, ClassIdx, ParentIdx, StartIdx, StopIdx
	};

	GtkDial *dial = GTK_WIDGET ( data );

	int idx;

	if ( Tcl_GetIndexFromObj ( interp, objv[1], cmds, "command", TCL_EXACT, &idx ) != TCL_OK )
	{
		return TCL_ERROR;
	}

	switch ( idx )
	{

		case ClassIdx:
			{
				Tcl_SetObjResult ( interp, Tcl_NewStringObj ( "dial", -1 ) );
			}
			break;
		case DeleteIdx:
			{
				return gnoclDelete ( interp, GTK_WIDGET ( dial ), objc, objv );
			}
		case ConfigureIdx:
			{
				int ret = TCL_ERROR;

				if ( gnoclParseAndSetOptions ( interp, objc - 1, objv + 1,
											   dialOptions, G_OBJECT ( dial ) ) == TCL_OK )
				{
					ret = configure ( interp, dial, dialOptions );
				}

				gnoclClearOptions ( dialOptions );

				return ret;
			}

			break;
	}

	return TCL_OK;
}

/**
\brief
**/
int gnoclDialCmd ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{
	int        ret;
	GtkWidget *dial;
	GtkAdjustment *adjustment;

	if ( gnoclParseOptions ( interp, objc, objv, dialOptions ) != TCL_OK )
	{
		gnoclClearOptions ( dialOptions );
		return TCL_ERROR;
	}

	adjustment = GTK_ADJUSTMENT ( gtk_adjustment_new ( 0, 0, 100, 0.01, 0.1, 0 ) );

	dial = gtk_dial_new ( adjustment );

	ret = gnoclSetOptions ( interp, dialOptions, G_OBJECT ( dial ), -1 );

	gnoclClearOptions ( dialOptions );

	if ( ret != TCL_OK )
	{
		gtk_widget_destroy ( GTK_WIDGET ( dial ) );
		return TCL_ERROR;
	}

	/* TODO: if not -visible == 0 */
	gtk_widget_show ( GTK_WIDGET ( dial ) );

	return gnoclRegisterWidget ( interp, GTK_WIDGET ( dial ), dialFunc );
}
