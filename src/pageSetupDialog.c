/**
\brief	This module implements the gnocl::pageSetupDialog widget.
**/

/* user documentation */
/**
\page page_pageSetupDialog gnocl::pageSetupDialog
\htmlinclude pageSetupDialog.html
**/

/**
 \par Modification History
 \verbatim
	2011-04: Begin developement
 \endverbatim
**/


#include "gnocl.h"
#include "string.h"
#include <assert.h>

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



/**
\brief
\author     Peter G Baum
**/
int pageSetupDialogFunc ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{


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
