/**
\brief	This module implements the gnocl::printDialog widget.
**/

/* user documentation */
/**
\page page_printDialog gnocl::printDialog
\htmlinclude printDialog.html
**/

/**
\par Modification History
\verbatim
	2013-07: added commands, options, commands
	2011-04: Begin developement
\endverbatim
**/

#include "gnocl.h"

/* static declarations */
static int gnoclOptPageSetup ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret );
static int gnoclOptPrintSettings ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret );
static int gnoclOptSelectedPrinter ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret );
static int gnoclOptSetPrintCapability ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret );

static GtkPrintSettings *settings = NULL;
static GtkPageSetup *page_setup = NULL;

static const int parentIdx    = 0;
static const int titleIdx	  = 1;

static GnoclOption options[] =
{
	{"-parent", GNOCL_STRING, NULL },                 /* 0 */
	{"-title", GNOCL_STRING, NULL },                 /* 1 */
	{"-currentPage", GNOCL_INT, "current-page"},
	{"-embedPageSetup", GNOCL_BOOL, "embed-page-setup"},
	{"-hasSelection", GNOCL_BOOL, "has-selection"},
	{"-pageSetup", GNOCL_OBJ, gnoclOptPageSetup},
	{"-printSettings", GNOCL_OBJ, gnoclOptPrintSettings},
	{"-selectedPrinter", GNOCL_OBJ, gnoclOptSelectedPrinter},
	{"-supportSelection", GNOCL_BOOL, "support-selection"},
	/* set capabilities */
	{"-setScale", GNOCL_OBJ, "scale", gnoclOptSetPrintCapability },
	{"-setPreview", GNOCL_OBJ, "preview", gnoclOptSetPrintCapability },
	{"-setReverse", GNOCL_OBJ, "reverse", gnoclOptSetPrintCapability },
	{ NULL }
};


/**
\brief
**/
static int gnoclOptSetPrintCapability ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{

#ifdef DEBUG_PRINTER_DIALOG
	printf ( "%s\n", __FUNCTION__ );
#endif

	g_print ( "WARNING! Setting printerDialog \"%s\" capability not yet implemented.\n", opt->propName );


	GtkPrintCapabilities capability;

	switch  ( *opt->propName )
	{

		case 'page-set':
			{
				capability = GTK_PRINT_CAPABILITY_PAGE_SET;
			}
			break;
		case 'copies':
			{
				capability = GTK_PRINT_CAPABILITY_COPIES;
			}
			break;
		case 'collate':
			{
				capability =  GTK_PRINT_CAPABILITY_COLLATE ;
			}
			break;
		case 'reverse':
			{
				capability =  GTK_PRINT_CAPABILITY_REVERSE ;
			}
			break;
		case 'scale':
			{
				capability =  GTK_PRINT_CAPABILITY_SCALE ;
			}
			break;
		case 'generate-pdf':
			{
				capability =  GTK_PRINT_CAPABILITY_GENERATE_PDF ;
			}
			break;
		case 'generate-ps':
			{
				capability =  GTK_PRINT_CAPABILITY_GENERATE_PS ;
			}
			break;
		case 'preview':
			{
				capability = GTK_PRINT_CAPABILITY_PREVIEW ;
			}
			break;
		case 'number-up':
			{
				capability =  GTK_PRINT_CAPABILITY_NUMBER_UP ;
			}
			break;
		case ' number-up-layout':
			{
				capability =  GTK_PRINT_CAPABILITY_NUMBER_UP_LAYOUT ;
			}
			break;
		default:
			{
				// assert( 0 );
				return TCL_ERROR;
			}

	}


	GtkPrintCapabilities tmp = gtk_print_unix_dialog_get_manual_capabilities ( GTK_PRINT_UNIX_DIALOG ( obj ) );

	capability = tmp | capability;

	gtk_print_unix_dialog_set_manual_capabilities ( GTK_PRINT_UNIX_DIALOG ( obj ), capability );

	return TCL_OK;
}

/**
\brief
**/
static int gnoclOptPageSetup ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
#ifdef DEBUG_PRINTER_DIALOG
	printf ( "%s\n", __FUNCTION__ );
#endif
	return TCL_OK;
}

/**
\brief
**/
static int gnoclOptPrintSettings ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
#ifdef DEBUG_PRINTER_DIALOG
	printf ( "%s\n", __FUNCTION__ );
#endif
	return TCL_OK;
}

/**
\brief
**/
static int gnoclOptSelectedPrinter ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
#ifdef DEBUG_PRINTER_DIALOG
	printf ( "%s\n", __FUNCTION__ );
#endif
	return TCL_OK;
}


/**
\brief
**/
static int configure ( Tcl_Interp *interp, GtkWidget *dialog, GnoclOption options[] )
{
#ifdef DEBUG_PRINTER_DIALOG
	printf ( "%s\n", __FUNCTION__ );
#endif

	return TCL_OK;
}



/**
\brief
\author     Peter G Baum
**/
int printDialogFunc ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{
#ifdef DEBUG_PRINTER_DIALOG
	printf ( "%s\n", __FUNCTION__ );
#endif

	/*
	case CommandsIdx:
	{
		gnoclGetOptions ( interp, cmds );
	}
	break;
	case OptionsIdx: {
		gnoclGetOptions (interp, options);
	}
	break;
	*/

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
int gnoclPrintDialogCmd ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{

#ifdef DEBUG_PRINTER_DIALOG
	printf ( "%s\n", __FUNCTION__ );
#endif

	GtkWidget *mainWindow;
	GtkWidget *dialog;
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


	if ( options[titleIdx].status == GNOCL_STATUS_CHANGED )
	{

		dialog = gtk_print_unix_dialog_new  ( options[titleIdx].val.str, GTK_WINDOW ( mainWindow ) );

	}

	else
	{
		dialog = gtk_print_unix_dialog_new  ( "Print Dialog", GTK_WINDOW ( mainWindow ) );
	}


	gtk_widget_show ( dialog );


	ret = gnoclSetOptions ( interp, options, G_OBJECT ( dialog ), -1 );

	if ( ret == TCL_OK )
	{
		ret = configure ( interp, dialog, options );
	}

	gnoclClearOptions ( options );


	return gnoclRegisterWidget ( interp, GTK_WIDGET ( dialog ), printDialogFunc );
}
