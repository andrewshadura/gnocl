/*
   History:
   2011-04: Begin of developement
 */

/**
\page page_spinner gnocl::spinner
\htmlinclude spinnner.html
**/

#include "gnocl.h"

static GnoclOption spinnerOptions[] =
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
static int configure ( Tcl_Interp *interp, GtkSpinner *spinner, GnoclOption options[] )
{
	return TCL_OK;
}

/**
\brief
**/
int spinnerFunc ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
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

	GtkSpinner *spinner = GTK_SPINNER ( data );

	int idx;

	if ( Tcl_GetIndexFromObj ( interp, objv[1], cmds, "command", TCL_EXACT, &idx ) != TCL_OK )
	{
		return TCL_ERROR;
	}

	switch ( idx )
	{
		case StartIdx:
			{
				gtk_spinner_start ( spinner );
			}
			break;
		case StopIdx:
			{
				gtk_spinner_stop ( spinner );
			}
			break;
		case ClassIdx:
			{
				Tcl_SetObjResult ( interp, Tcl_NewStringObj ( "spinner", -1 ) );
			}
			break;
		case DeleteIdx:
			{
				return gnoclDelete ( interp, GTK_WIDGET ( spinner ), objc, objv );
			}
		case ConfigureIdx:
			{
				int ret = TCL_ERROR;

				if ( gnoclParseAndSetOptions ( interp, objc - 1, objv + 1,
											   spinnerOptions, G_OBJECT ( spinner ) ) == TCL_OK )
				{
					ret = configure ( interp, spinner, spinnerOptions );
				}

				gnoclClearOptions ( spinnerOptions );

				return ret;
			}

			break;
	}

	return TCL_OK;
}

/**
\brief
**/
int gnoclSpinnerCmd ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{
	int        ret;
	GtkWidget *spinner;


	if ( gnoclParseOptions ( interp, objc, objv, spinnerOptions ) != TCL_OK )
	{
		gnoclClearOptions ( spinnerOptions );
		return TCL_ERROR;
	}

	spinner = gtk_spinner_new ();

	ret = gnoclSetOptions ( interp, spinnerOptions, G_OBJECT ( spinner ), -1 );

	gnoclClearOptions ( spinnerOptions );

	if ( ret != TCL_OK )
	{
		gtk_widget_destroy ( GTK_WIDGET ( spinner ) );
		return TCL_ERROR;
	}

	/* TODO: if not -visible == 0 */
	gtk_widget_show ( GTK_WIDGET ( spinner ) );

	return gnoclRegisterWidget ( interp, GTK_WIDGET ( spinner ), spinnerFunc );
}
