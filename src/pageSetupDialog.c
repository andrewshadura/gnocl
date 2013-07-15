/**
\brief	This module implements the gnocl::pageSetupDialog widget.
\todo	CODE INCOMPLETE
**/

/* user documentation */
/**
\page page_pageSetupDialog gnocl::pageSetupDialog
\htmlinclude pageSetupDialog.html
**/

/**
 \par Modification History
 \verbatim
	2013-07: added commands, options, commands
	2011-04: Begin developement
 \endverbatim
**/


#include "gnocl.h"

static GtkPrintSettings *settings = NULL;
static GtkPageSetup *page_setup = NULL;



static GnoclOption options[] =
{
	{ "-parent", GNOCL_STRING, NULL },                 /* 0 */
	{ NULL }
};

static const int parentIdx    = 0;



/* moved to gnocl.h */
/*
typedef struct
{
    char             *name;
    Tcl_Interp       *interp;
    char             *onClicked;
    GtkPageSetupDialog *fontSel;
} FontSelDialogParams;
*/

static const char *cmds[] = { "delete", "configure", "class", "hide", "show",  NULL };

/**
\brief
\author     Peter G Baum
**/
int pageSetupDialogFunc ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{

#if 0

	enum cmdIdx { DeleteIdx, ConfigureIdx, ClassIdx, HideIdx, ShowIdx, OptionsIdx, CommandsIdx };

	GtkWidget *widget = GTK_WIDGET ( data );

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

		case HideIdx:
			{
				gtk_widget_hide ( widget );
			}
			break;
		case ShowIdx:
			{
				gtk_widget_show_all ( widget );
			}
			break;
		case ClassIdx:
			{
				Tcl_SetObjResult ( interp, Tcl_NewStringObj ( "pageSetupDialog", -1 ) );
			}

			break;
		case DeleteIdx:
			{
				return gnoclDelete ( interp, widget, objc, objv );
			} break;
		case ConfigureIdx:
			{
				int ret = TCL_ERROR;

				if ( gnoclParseAndSetOptions ( interp, objc - 1, objv + 1,
											   options, G_OBJECT ( widget ) ) == TCL_OK )
				{
					ret = configure ( interp, widget, options );
				}

				gnoclClearOptions ( options );

				return ret;
			}
			break;
	}

#endif
	return TCL_OK;
}

/**
\brief
\author     Peter G Baum

 GtkPageSetup *new_page_setup;
  if (settings == NULL)
    settings = gtk_print_settings_new ();
  new_page_setup = gtk_print_run_page_setup_dialog (GTK_WINDOW (main_window),
                                                    page_setup, settings);
  if (page_setup)
    g_object_unref (page_setup);
  page_setup = new_page_setup;



**/
int gnoclPageSetupDialogCmd ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{
	if ( gnoclGetCmdsAndOpts ( interp, cmds, options, objv, objc ) == TCL_OK )
	{
		return TCL_OK;
	}

	GtkWidget *mainWindow;
	gint ret;

	if ( gnoclParseOptions ( interp, objc, objv, options ) != TCL_OK )
	{
		gnoclClearOptions ( options );
		return TCL_ERROR;
	}


	if ( options[parentIdx].status == GNOCL_STATUS_CHANGED )
	{
		mainWindow = gnoclGetWidgetFromName (  options[parentIdx].val.str, interp );

	}

	else
	{
		return TCL_ERROR;
	}


	if ( settings == NULL )
	{
		settings = gtk_print_settings_new ();
	}

	GtkPageSetup *new_page_setup;


	new_page_setup = gtk_print_run_page_setup_dialog ( GTK_WINDOW ( mainWindow ), page_setup, settings );

	if ( page_setup )
	{
		g_object_unref ( page_setup );
	}

	page_setup = new_page_setup;


	ret = TCL_OK;

cleanExit:
	/* TODO:
	if( ret != TCL_OK )
	   freeParams
	*/

	gnoclClearOptions ( options );

	return ret;
}
