
/**
\page page_colorSelection gnocl::colorSelection
\htmlinclude colorSelection.html
**/

/**
\par Modification History
\verbatim
	2013-07:	added commands, options, commands
\endverbatim
**/

#include "gnocl.h"


static GnoclOption colorSelectionOptions[] =
{
	{ "-tooltip", GNOCL_OBJ, "", gnoclOptTooltip },
	{ "-palette", GNOCL_BOOL, "has-palette" },
	{ "-opacity", GNOCL_BOOL, "has-opacity-control"  },
	{ NULL },
};

/**
\brief
**/
static int configure ( Tcl_Interp *interp, GtkWidget *widget, GnoclOption options[] )
{
	return gnoclCgetNotImplemented ( interp, options );
}

/**
\brief
**/
static int cget ( Tcl_Interp *interp, GtkWidget *widget, GnoclOption options[], int idx )
{
	return gnoclCgetNotImplemented ( interp, options + idx );
}

/**
\brief
**/
int colorSelectionFunc ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
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
				gnoclGetOptions ( interp, colorSelectionOptions );
			}		break;
		case ClassIdx:
			Tcl_SetObjResult ( interp, Tcl_NewStringObj ( "fileChooser", -1 ) );
			break;
		case DeleteIdx:
			return gnoclDelete ( interp, GTK_WIDGET ( widget ), objc, objv );

		case ConfigureIdx:
			{
				int ret = TCL_ERROR;

				ret = configure ( interp, widget, colorSelectionOptions );

				if ( 1 )
				{
					if ( gnoclParseAndSetOptions ( interp, objc - 1, objv + 1, colorSelectionOptions, G_OBJECT ( widget ) ) == TCL_OK )
					{
						ret = configure ( interp, widget, colorSelectionOptions );
					}
				}

				gnoclClearOptions ( colorSelectionOptions );

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

				switch ( gnoclCget ( interp, objc, objv, G_OBJECT ( widget ), colorSelectionOptions, &idx ) )
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
							return cget ( interp, widget, colorSelectionOptions, idx );
						}
				}
			}
	}

	return TCL_OK;
}

/**
\brief
**/
int gnoclColorSelectionCmd ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{

	int            ret = TCL_OK;
	GtkWidget      *widget;

	if ( 1 )
	{
		if ( gnoclParseOptions ( interp, objc, objv, colorSelectionOptions ) != TCL_OK )
		{
			gnoclClearOptions ( colorSelectionOptions );
			return TCL_ERROR;
		}
	}

	widget = gtk_color_selection_new   ( ) ;

	gtk_widget_show ( GTK_WIDGET ( widget ) );


	if ( 0 )
	{
		ret = gnoclSetOptions ( interp, colorSelectionOptions, G_OBJECT ( widget ), -1 );

		if ( ret == TCL_OK )
		{
			ret = configure ( interp, G_OBJECT ( widget ), colorSelectionOptions );
		}

		gnoclClearOptions ( colorSelectionOptions );

		/* STEP 3)  -show the widget */

		if ( ret != TCL_OK )
		{
			gtk_widget_destroy ( GTK_WIDGET ( widget ) );
			return TCL_ERROR;
		}
	}

	/* STEP 4)  -everything has worked, register the widget with the Tcl interpretor. */
	return gnoclRegisterWidget ( interp, GTK_WIDGET ( widget ), colorSelectionFunc );
}
