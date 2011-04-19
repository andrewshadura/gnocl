/**
\brief		Bindingd to the GtkPrintOperation object.
\author     William J Giddings
\date       15/10/10
\notes		Provide exlanation of how the print process occurs.
			1) Initialize with gtk_print_operation_new().
			2) Apply print settings with gtk_print_operation_print_settings().

**/

/**
\page page_printing gnocl::printOperation
\htmlinclude printop.html
**/

#include "gnocl.h"
#include "gnoclparams.h"
#include <string.h>
#include <ctype.h>
#include <glade/glade.h>
#include <math.h>
#include <assert.h>

/* some default values */
#define GNOCL_HEADER_HEIGHT 20.0
#define GNOCL_HEADER_GAP 8.5

/* details about the print job */
typedef struct
{
	gchar *filename;
	gdouble font_size;
	gint lines_per_page;
	gchar **lines;
	gint total_lines;
	gint total_pages;
} PrintData;

/* user data */
typedef struct
{
	GtkWidget   *window;
	GtkWidget   *chooser;
	PrintData  *data;
} Widgets;

GtkPrintSettings *settings;

/* declarations from Krause sample code */
static void printFile ( gchar *filename, Widgets* );
static void print_file ( GtkButton*, Widgets* );
static void begin_print ( GtkPrintOperation*, GtkPrintContext*, Widgets* );
static void draw_page ( GtkPrintOperation*, GtkPrintContext*, gint, Widgets* );
static void end_print ( GtkPrintOperation*, GtkPrintContext*, Widgets* );

/* needs to be public to be accessed by gnocl::inventory */
static GHashTable *name2printopList;
static const char idPrefix[] = "::gnocl::_POP";

/**
\brief
**/
void gnoclGetPrintOpList ( GList **list )
{
	g_hash_table_foreach ( name2printopList, hash_to_list, list );
}

/**
\brief
**/
GtkPrintOperation *gnoclGetPrintOpFromName ( const char * id, Tcl_Interp * interp )
{
	GtkPrintOperation *operation = NULL;
	int n;

	if ( strncmp ( id, idPrefix, sizeof ( idPrefix ) - 1 ) == 0
			&& ( n = atoi ( id + sizeof ( idPrefix ) - 1 ) ) > 0 )
	{
		operation = g_hash_table_lookup ( name2printopList, GINT_TO_POINTER ( n ) );
	}

	if ( operation == NULL && interp != NULL )
	{
		Tcl_AppendResult ( interp, "Unknown print operation \"", id, "\".", ( char * ) NULL );
	}

	return operation;
}

/**
\brief
**/
int gnoclForgetPrintOpFromName ( const char * path )
{
	int n = atoi ( path + sizeof ( idPrefix ) - 1 );
	assert ( gnoclGetPrintOpFromName ( path, NULL ) );
	assert ( strncmp ( path, idPrefix, sizeof ( idPrefix ) - 1 ) == 0 );
	assert ( n > 0 );

	g_hash_table_remove ( name2printopList, GINT_TO_POINTER ( n ) );

	return 0;
}

/**
\brief
**/
int gnoclMemNameAndPrintOp ( const char * path,  GtkPrintOperation *operation )
{
	int n ;

	n = atoi ( path + sizeof ( idPrefix ) - 1 );

	assert ( n > 0 );
	assert ( g_hash_table_lookup ( name2printopList, GINT_TO_POINTER ( n ) ) == NULL );
	assert ( strncmp ( path, idPrefix, sizeof ( idPrefix ) - 1 ) == 0 );

	/* memorize the name of the widget in the widget */
	g_object_set_data ( G_OBJECT ( operation ), "gnocl::name", ( char * ) path );
	g_hash_table_insert ( name2printopList, GINT_TO_POINTER ( n ), operation );

	return 0;
}

/**
\brief
**/
char *gnoclGetAutoPrintOpId ( void )
{
	static int no = 0;
	/*
	static char buffer[30];
	*/

	char *buffer = g_new ( char, sizeof ( idPrefix ) + 15 );
	strcpy ( buffer, idPrefix );

	/* with namespace, since the Id is also the widget command */
	sprintf ( buffer + sizeof ( idPrefix ) - 1, "%d", ++no );

	return buffer;
}



/* signal handlers */
/**
\brief
**/
void doOnReady ( GtkPrintOperationPreview *preview, GtkPrintContext *context, gpointer user_data )
{
	GnoclCommandData *cs = ( GnoclCommandData * ) user_data;

	if ( *cs->interp->result == '\0' )
	{
		GnoclPercSubst ps[] =
		{
			{ 'w', GNOCL_STRING },  /* printOperation */
			{ 0 }
		};

		//ps[0].val.str = gnoclGetNameFromWidget ( operation );
		gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 );
	}
}

/**
\brief      This is for all callbacks which have only %w as substitution
**/
int gnoclOptOnReady ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	assert ( opt->type == GNOCL_OBJ );

	return gnoclConnectOptCmd ( interp, obj, opt->propName, G_CALLBACK ( doOnReady ), opt, NULL, ret );
}

/**
\brief
**/
void doOnGotPageSize ( GtkPrintOperationPreview *preview, GtkPrintContext *context, GtkPageSetup *page_setup, gpointer user_data )
{
	GnoclCommandData *cs = ( GnoclCommandData * ) user_data;

	if ( *cs->interp->result == '\0' )
	{
		GnoclPercSubst ps[] =
		{
			{ 'w', GNOCL_STRING },  /* printOperation */
			{ 0 }
		};

		//ps[0].val.str = gnoclGetNameFromWidget ( operation );
		gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 );
	}
}


/**
\brief      This is for all callbacks which have only %w as substitution
**/
int gnoclOptOnGotPageSize ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	assert ( opt->type == GNOCL_OBJ );

	return gnoclConnectOptCmd ( interp, obj, opt->propName, G_CALLBACK ( doOnGotPageSize ), opt, NULL, ret );
}

/**
\brief
**/
void doOnStatusChanged ( GtkPrintOperation *operation, gpointer user_data )
{
	GnoclCommandData *cs = ( GnoclCommandData * ) user_data;

	if ( *cs->interp->result == '\0' )
	{
		GnoclPercSubst ps[] =
		{
			{ 'w', GNOCL_STRING },  /* printOperation */
			{ 0 }
		};

		ps[0].val.str = gnoclGetNameFromWidget ( operation );
		gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 );
	}
}

/**
\brief      This is for all callbacks which have only %w as substitution
**/
int gnoclOptOnStatusChanged ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	assert ( opt->type == GNOCL_OBJ );

	return gnoclConnectOptCmd ( interp, obj, opt->propName, G_CALLBACK ( doOnStatusChanged ), opt, NULL, ret );
}



/**
\brief
**/
void doOnRequestPageSetup ( GtkPrintOperation *operation, GtkPrintContext *context, gint page_nr, GtkPageSetup *setup, gpointer user_data )
{
	GnoclCommandData *cs = ( GnoclCommandData * ) user_data;

	if ( *cs->interp->result == '\0' )
	{
		GnoclPercSubst ps[] =
		{
			{ 'w', GNOCL_STRING },  /* printOperation */
			{ 0 }
		};

		ps[0].val.str = gnoclGetNameFromWidget ( operation );
		gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 );
	}
}

/**
\brief      This is for all callbacks which have only %w as substitution
**/
int gnoclOptOnRequestPageSetup ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	assert ( opt->type == GNOCL_OBJ );

	return gnoclConnectOptCmd ( interp, obj, opt->propName, G_CALLBACK ( doOnRequestPageSetup ), opt, NULL, ret );
}

/**
\brief
**/
gboolean doOnPreview ( GtkPrintOperation *operation, GtkPrintOperationPreview *preview, GtkPrintContext *context, GtkWindow *parent, gpointer user_data )
{
	GnoclCommandData *cs = ( GnoclCommandData * ) user_data;

	if ( *cs->interp->result == '\0' )
	{
		GnoclPercSubst ps[] =
		{
			{ 'w', GNOCL_STRING },  /* printOperation */
			{ 0 }
		};

		ps[0].val.str = gnoclGetNameFromWidget ( operation );
		gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 );
	}
}

/**
\brief      This is for all callbacks which have only %w as substitution
**/
int gnoclOptOnPreview ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	assert ( opt->type == GNOCL_OBJ );

	return gnoclConnectOptCmd ( interp, obj, opt->propName, G_CALLBACK ( doOnPreview ), opt, NULL, ret );
}



/**
\brief
**/
gboolean doOnPaginate ( GtkPrintOperation *operation, GtkPrintContext *context, gpointer user_data )
{
	GnoclCommandData *cs = ( GnoclCommandData * ) user_data;

	if ( *cs->interp->result == '\0' )
	{
		GnoclPercSubst ps[] =
		{
			{ 'w', GNOCL_STRING },  /* printOperation */
			{ 0 }
		};

		ps[0].val.str = gnoclGetNameFromWidget ( operation );
		gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 );
	}
}

/**
\brief      This is for all callbacks which have only %w as substitution
**/
int gnoclOptOnPaginate ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	assert ( opt->type == GNOCL_OBJ );

	return gnoclConnectOptCmd ( interp, obj, opt->propName, G_CALLBACK ( doOnPaginate ), opt, NULL, ret );
}


/**
\brief
**/
void doOnEndPrint ( GtkPrintOperation *operation, GtkPrintContext *context, gpointer user_data )
{
	GnoclCommandData *cs = ( GnoclCommandData * ) user_data;

	if ( *cs->interp->result == '\0' )
	{
		GnoclPercSubst ps[] =
		{
			{ 'w', GNOCL_STRING },  /* printOperation */
			{ 0 }
		};

		ps[0].val.str = gnoclGetNameFromWidget ( operation );
		gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 );
	}
}

/**
\brief      This is for all callbacks which have only %w as substitution
**/
int gnoclOptOnEndPrint ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	assert ( opt->type == GNOCL_OBJ );

	return gnoclConnectOptCmd ( interp, obj, opt->propName, G_CALLBACK ( doOnEndPrint ), opt, NULL, ret );
}

/**
\brief
**/
void doOnDrawPage ( GtkPrintOperation *operation, GtkPrintContext *context, gint page_nr, gpointer user_data )
{
	GnoclCommandData *cs = ( GnoclCommandData * ) user_data;

	if ( *cs->interp->result == '\0' )
	{
		GnoclPercSubst ps[] =
		{
			{ 'w', GNOCL_STRING },  /* printOperation */
			{ 0 }
		};

		ps[0].val.str = gnoclGetNameFromWidget ( operation );
		gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 );
	}
}

/**
\brief      This is for all callbacks which have only %w as substitution
**/
int gnoclOptOnDrawPage ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	assert ( opt->type == GNOCL_OBJ );

	return gnoclConnectOptCmd ( interp, obj, opt->propName, G_CALLBACK ( doOnDrawPage ), opt, NULL, ret );
}

/**
\brief
**/
void doOnDone ( GtkPrintOperation *operation, GtkPrintOperationResult result, gpointer user_data )
{
	GnoclCommandData *cs = ( GnoclCommandData * ) user_data;

	if ( *cs->interp->result == '\0' )
	{
		GnoclPercSubst ps[] =
		{
			{ 'w', GNOCL_STRING },  /* printOperation */
			{ 0 }
		};

		ps[0].val.str = gnoclGetNameFromWidget ( operation );
		gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 );
	}
}

/**
\brief      This is for all callbacks which have only %w as substitution
**/
int gnoclOptOnDone ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	assert ( opt->type == GNOCL_OBJ );

	return gnoclConnectOptCmd ( interp, obj, opt->propName, G_CALLBACK ( doOnDone ), opt, NULL, ret );
}


/**
\brief
**/
void doOnCustomWidgetApply ( GtkPrintOperation *operation, GtkWidget *widget, gpointer user_data )
{
	GnoclCommandData *cs = ( GnoclCommandData * ) user_data;

	if ( *cs->interp->result == '\0' )
	{
		GnoclPercSubst ps[] =
		{
			{ 'w', GNOCL_STRING },  /* printOperation */
			{ 0 }
		};

		ps[0].val.str = gnoclGetNameFromWidget ( operation );
		gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 );
	}
}

/**
\brief      This is for all callbacks which have only %w as substitution
**/
int gnoclOptOnCustomWidgetApply ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	assert ( opt->type == GNOCL_OBJ );

	return gnoclConnectOptCmd ( interp, obj, opt->propName, G_CALLBACK ( doOnCustomWidgetApply ), opt, NULL, ret );
}

/**
\brief
**/
GObject* doOnCreateCustomWidget ( GtkPrintOperation *operation, gpointer user_data )
{
	GnoclCommandData *cs = ( GnoclCommandData * ) user_data;

	if ( *cs->interp->result == '\0' )
	{
		GnoclPercSubst ps[] =
		{
			{ 'w', GNOCL_STRING },  /* printOperation */
			{ 0 }
		};

		ps[0].val.str = gnoclGetNameFromWidget ( operation );
		gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 );
	}
}

/**
\brief      This is for all callbacks which have only %w as substitution
**/
int gnoclOptOnCreateCustomWidget ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	assert ( opt->type == GNOCL_OBJ );

	return gnoclConnectOptCmd ( interp, obj, opt->propName, G_CALLBACK ( doOnCreateCustomWidget ), opt, NULL, ret );
}

/**
\brief
**/
void doOnBeginPrint ( GtkPrintOperation *operation,	GtkPrintContext *context, gpointer user_data )
{
	GnoclCommandData *cs = ( GnoclCommandData * ) user_data;

	if ( *cs->interp->result == '\0' )
	{
		GnoclPercSubst ps[] =
		{
			{ 'w', GNOCL_STRING },  /* printOperation */
			{ 0 }
		};

		ps[0].val.str = gnoclGetNameFromWidget ( operation );
		gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 );
	}
}

/**
\brief      This is for all callbacks which have only %w as substitution
**/
int gnoclOptOnBeginPrint ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	assert ( opt->type == GNOCL_OBJ );

	return gnoclConnectOptCmd ( interp, obj, opt->propName, G_CALLBACK ( doOnBeginPrint ), opt, NULL, ret );
}


/* properties */
/*
  "default-page-setup"       GtkPageSetup*         : Read / Write
  "print-settings"           GtkPrintSettings*     : Read / Write
  "status"                   GtkPrintStatus        : Read
  "unit"                     GtkUnit               : Read / Write

  "allow-async"              gboolean              : Read / Write
  "current-page"             gint                  : Read / Write
  "custom-tab-label"         gchar*                : Read / Write
  "export-filename"          gchar*                : Read / Write
  "job-name"                 gchar*                : Read / Write *
  "n-pages"                  gint                  : Read / Write
  "show-progress"            gboolean              : Read / Write
  "status-string"            gchar*                : Read
  "track-print-status"       gboolean              : Read / Write
  "use-full-page"            gboolean              : Read / Write
*/


/* signals */
/*
  "begin-print"                                    : Run Last
  "create-custom-widget"                           : Run Last
  "custom-widget-apply"                            : Run Last
  "done"                                           : Run Last
  "draw-page"                                      : Run Last
  "end-print"                                      : Run Last
  "paginate"                                       : Run Last
  "preview"                                        : Run Last
  "request-page-setup"                             : Run Last
  "status-changed"                                 : Run Last
  "got-page-size"                                  : Run Last
  "ready"                                          : Run Last
*/


static const int pageSetupIdx = 0;
static const int printSettingsIdx = 1;
static const int statusIdx = 2;
static const int unitIdx = 3;
static const int fileNameIdx = 4;

static GnoclOption printOptions[] =
{
	/* requires more configuration */
	{ "-defaultPageSetup", GNOCL_OBJ, NULL},      	/* 0 */
	{ "-printSettings", GNOCL_OBJ, NULL},      		/* 1 */
	{ "-status", GNOCL_OBJ, NULL},      			/* 2 */
	{ "-unit", GNOCL_OBJ, NULL},      				/* 3 */
	{ "-fileName", GNOCL_STRING, NULL},				/* 4 */

	/* simple properties */
	{ "-async", GNOCL_BOOL, "allow-async" },
	{ "-currentPage", GNOCL_INT,  "current-page"},
	{ "-customeTabLabel", GNOCL_STRING, "custom-tab-label" },
	{ "-exportFilename", GNOCL_STRING,  "export-filename"  },
	{ "-jobName", GNOCL_STRING, "job-name" },
	{ "-nPages", GNOCL_INT, "n-pages"},
	{ "-showProgress", GNOCL_BOOL,  "show-progress"},
	{ "-statusString", GNOCL_STRING, "status-string"}, /* should this be a command? */
	{ "-trackStatus", GNOCL_BOOL, "track-print-status"},
	{ "-fullPage", GNOCL_BOOL, "use-full-page"},
	{ "-supportSelection", GNOCL_BOOL, "support-selection"},

	/* signals, set up call-backs */
	{ "-onBeginPrint", GNOCL_OBJ, "begin-print", gnoclOptOnBeginPrint },
	{ "-onCreateCustomWidget", GNOCL_OBJ, "create-custom-widget", gnoclOptOnCreateCustomWidget },
	{ "-onCustomWidgetApply", GNOCL_OBJ, "custom-widget-apply", gnoclOptOnCustomWidgetApply },
	{ "-onDone", GNOCL_OBJ, "done", gnoclOptOnDone },
	{ "-onDrawPage", GNOCL_OBJ, "draw-page", gnoclOptOnDrawPage },
	{ "-onEndPrint", GNOCL_OBJ, "end-print", gnoclOptOnEndPrint },
	{ "-onPaginate", GNOCL_OBJ, "paginate", gnoclOptOnPaginate },
	{ "-onPreview", GNOCL_OBJ, "preview", gnoclOptOnPreview },
	{ "-onRequestPageSetup", GNOCL_OBJ, "request-page-setup", gnoclOptOnRequestPageSetup },
	{ "-onStatusChanged", GNOCL_OBJ, "status-changed", gnoclOptOnStatusChanged },
	{ "-onGotPageSize", GNOCL_OBJ, "got-page-size", gnoclOptOnGotPageSize },
	{ "-onReady", GNOCL_OBJ, "ready", gnoclOptOnReady },
	{ NULL },
};



/**
\brief
**/

static int configure (  Tcl_Interp *interp, GtkPrintOperation *printOp,  GnoclOption options[] )
{



	g_print ( "gnoclPrintOperationCmd -> configure \n" );


	if ( options[pageSetupIdx].status == GNOCL_STATUS_CHANGED )
	{
		g_print ( "gnoclPrintOperationCmd -> configure -> pageSetupIdx\n" );
	}

	if ( options[printSettingsIdx].status == GNOCL_STATUS_CHANGED )
	{
		g_print ( "gnoclPrintOperationCmd -> configure -> printSettingsIdx\n" );
	}

	if ( options[statusIdx].status == GNOCL_STATUS_CHANGED )
	{
		g_print ( "gnoclPrintOperationCmd -> configure -> statusIdx\n" );
	}

	if ( options[unitIdx].status == GNOCL_STATUS_CHANGED )
	{

		g_print ( "gnoclPrintOperationCmd -> configure -> unitIdx\n" );

		static const char *cmds[] =
		{
			"pixel", "point", "inch", "millimeter",
			"px", "pnt", "ins", "mm",
			NULL
		};

		enum cmdIdx
		{
			PixelIdx, PointIdx, InchIdx, MillimeterIdx,
			PxIdx, PntIdx, InsIdx, MmIdx
		};


		int idx;

		if ( Tcl_GetIndexFromObj ( interp, options[unitIdx].val.obj, cmds, "command", TCL_EXACT, &idx ) != TCL_OK )
		{
			return TCL_ERROR;
		}


		switch ( idx )
		{
			case PixelIdx:
			case PxIdx:
				{
					gtk_print_operation_set_unit ( printOp, GTK_UNIT_PIXEL );
				}
				break;
			case PointIdx:
			case PntIdx:
				{
					gtk_print_operation_set_unit ( printOp, GTK_UNIT_POINTS );
				}
				break;
			case InchIdx:
			case InsIdx:
				{
					g_print ( "INS\n" );
					gtk_print_operation_set_unit ( printOp, GTK_UNIT_INCH );
				}
				break;
			case MillimeterIdx:
			case MmIdx:
				{
					gtk_print_operation_set_unit ( printOp, GTK_UNIT_MM );
				}
				break;
			default: {}
		}

	}

	if ( options[fileNameIdx].status == GNOCL_STATUS_CHANGED )
	{
		g_print ( "gnoclPrintOperationCmd -> configure -> fileNameIdx\n" );

	}

}

/**
\brief
**/
static int cget (   Tcl_Interp *interp, GtkPrintOperation *printOp,  GnoclOption options[],  int idx )
{

	/* this is a straight copy from button.c  */

	/*
		Tcl_Obj *obj = NULL;

		if ( idx == textIdx )
		{
			obj = gnoclCgetButtonText ( interp, button );
		}

		else if ( idx == iconIdx )
		{
			GtkWidget *image = gnoclFindChild ( GTK_WIDGET ( button ), GTK_TYPE_IMAGE );

			if ( image == NULL )
			{
				obj = Tcl_NewStringObj ( "", 0 );
			}

			else
			{
				gchar  *st;
				g_object_get ( G_OBJECT ( image ), "stock", &st, NULL );

				if ( st )
				{
					obj = Tcl_NewStringObj ( "%#", 2 );
					Tcl_AppendObjToObj ( obj, gnoclGtkToStockName ( st ) );
					g_free ( st );
				}

				else
				{
					Tcl_SetResult ( interp, "Could not determine icon type.", TCL_STATIC );
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
\brief  Function associated with the widget.
**/
int printOperationFunc ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{
#ifdef DEBUG_PRINT_OPERATION
	listParameters ( objc, objv, "printOperationFunc" );
#endif

	static const char *cmds[] =
	{
		"export", "cancel", "finish",
		"preview", "print", "file",
		"class", "configure", "cget",
		"status",
		NULL
	};

	enum cmdIdx
	{
		ExportIdx, CancelIdx, FinishIdx,
		PreviewIdx, PrintIdx, FileIdx,
		ClassIdx, ConfigureIdx, CgetIdx,
		StatusIdx
	};

	//GtkPrintOperation *operation = GTK_PRINT_OPERATION ( data );
	GtkPrintOperation *operation = data ;


	PrintOperationsParams *para = ( PrintOperationsParams * ) data;

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
		case StatusIdx:
			{

				//const gchar * gtk_print_operation_get_status_string (printOp);

				Tcl_SetObjResult ( interp, Tcl_NewStringObj ( gtk_print_operation_get_status_string ( para->operation ) , -1 ) );


			}
			break;
		case ConfigureIdx:
			{
				int ret = TCL_ERROR;

				if ( gnoclParseAndSetOptions ( interp, objc - 1, objv + 1, printOptions, G_OBJECT ( para->operation ) ) == TCL_OK )
				{
					ret = configure ( interp, para->operation , printOptions );
				}

				gnoclClearOptions ( printOptions );

				return ret;
			}
			break;
		case CgetIdx:
			{
				int idx2;

				switch ( gnoclCget ( interp, objc, objv, G_OBJECT ( para->operation ), printOptions, &idx2 ) )
				{
					case GNOCL_CGET_ERROR:
						return TCL_ERROR;
					case GNOCL_CGET_HANDLED:
						return TCL_OK;
					case GNOCL_CGET_NOTHANDLED:
						return cget ( interp, para->operation, printOptions, idx2 );
				}
			}
			break;
		case ExportIdx:
			{
				if ( objc == 3 )
				{
					g_print ( "printOperationFunc -> ExportIdx -> %s\n", Tcl_GetString ( objv[2] ) );
					gtk_print_operation_set_export_filename ( operation, Tcl_GetString ( objv[2] ) );
				}

				else
				{

					Tcl_SetObjResult ( interp, Tcl_NewStringObj ( "ERROR: Must specify export filename. (type=pdf)", -1 ) );

					return TCL_ERROR;
				}

			}
			break;
		case ClassIdx:
			{
				Tcl_SetObjResult ( interp, Tcl_NewStringObj ( "printOperation", -1 ) );

			}
			break;
		case CancelIdx:
			{
				gtk_print_operation_cancel ( operation );
			}
			break;
		case FinishIdx:
			{
				gtk_print_operation_draw_page_finish ( operation );
			}
			break;
		case PreviewIdx:
			/* test the as subcommand */
			{
				g_print ( "printOperationFunc -> PreviewIdx -> %s\n", Tcl_GetString ( objv[2] ) );
				static const char *subCmds[] =
				{
					"end", "page", "selected",
					NULL
				};

				enum subCmdIdx
				{
					EndIdx, PageIdx, SelectedIdx
				};

				gint idx3;
				gint page_nr;

				if ( Tcl_GetIndexFromObj ( interp, objv[2], subCmds, "command", TCL_EXACT, &idx3 ) != TCL_OK )
				{
					return TCL_ERROR;
				}

				switch ( idx3 )
				{
					case SelectedIdx:
						{
							Tcl_GetIntFromObj ( interp, objv[3], &page_nr );
							g_print ( "SELECTED\n" );

							// gboolean gtk_print_operation_preview_is_selected (GtkPrintOperationPreview *preview, gint page_nr);

							if ( 1 )
							{
								Tcl_SetObjResult ( interp, Tcl_NewStringObj ( "1", -1 ) );
							}

							else
							{
								Tcl_SetObjResult ( interp, Tcl_NewStringObj ( "0", -1 ) );
							}
						}
						break;
					case EndIdx:
						{
							g_print ( "END\n" );
							// gtk_print_operation_preview_end_preview (GtkPrintOperationPreview *preview);
						}
						break;
					case PageIdx:
						{
							Tcl_GetIntFromObj ( interp, objv[3], &page_nr );
							g_print ( "PAGE %d\n", page_nr );
							// gtk_print_operation_preview_render_page (GtkPrintOperationPreview *preview, gint page_nr);
						}
						break;
					default: {}
				}

			}
			break;
		default: {}
	}

	return TCL_OK;

}

/**
\brief
**/
const char *gnoclGetNameFromPrintOp ( GtkPrintOperation *operation )
{
	const char *name = g_object_get_data ( G_OBJECT ( operation ), "gnocl::name" );

	return name;
}

/**
\brief
*/
static void simpleDestroyFunc (	GtkPrintOperation *operation, gpointer data )
{
	const char *name = gnoclGetNameFromPrintOp ( operation );
	gnoclForgetPrintOpFromName ( name );
	Tcl_DeleteCommand ( ( Tcl_Interp * ) data, ( char * ) name );
	g_free ( ( char * ) name );
}

/**
\brief     Function to create and configure an new instance of the button widget.
**/
int gnoclPrintOperationCmd ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{

#ifdef DEBUG_PRINT_OPERATION
	listParameters ( objc, objv, "gnoclPrintOperationCmd" );
#endif

	/* create new hash table if one does not exist */
	if ( name2printopList == NULL )
	{
		name2printopList = g_hash_table_new ( g_direct_hash, g_direct_equal );
	}


	PrintOperationsParams *para;
	int  ret;

	/* step 1) check validity of switches */
	g_print ( "gnoclPrintOperationCmd 1\n" );

	if ( gnoclParseOptions ( interp, objc, objv, printOptions ) != TCL_OK )
	{
		gnoclClearOptions ( printOptions );
		g_print ( "gnoclPrintOperationCmd 2\n" );
		return TCL_ERROR;
	}

	para = g_new ( PrintOperationsParams, 1 );
	/* initialize print operation and create settings */
	para->operation = GTK_PRINT_OPERATION ( gtk_print_operation_new() );
	para->settings = GTK_PRINT_SETTINGS ( gtk_print_settings_new () );


	gtk_print_operation_set_print_settings ( para->operation, para->settings );


	para->interp = interp;

	/* step 2) create an instance of the object */

	g_print ( "gnoclPrintOperationCmd 3\n" );

	/* step 3) check the options passed for the creation of the widget */
	ret = gnoclSetOptions ( interp, printOptions, G_OBJECT ( para->operation ), -1 );

	g_print ( "gnoclPrintOperationCmd 4\n" );

	/* step 4) if this is ok, then configure the new widget */
	if ( ret == TCL_OK )
	{
		ret = configure ( interp, para->operation, printOptions );
		g_print ( "gnoclPrintOperationCmd 5\n" );
	}

	/* step 5) clear the memory set assigned to the options */
	gnoclClearOptions ( printOptions );
	g_print ( "gnoclPrintOperationCmd 6\n" );


	/* step 6) if the options passed were incorrect, then delete the widget */
	if ( ret != TCL_OK )
	{
		g_print ( "gnoclPrintOperationCmd 7\n" );

	}

	else
	{
		g_print ( "gnoclPrintOperationCmd 8\n" );
	}

	/* step 7) the process has been successful, so 'register' the widget with the interpreter */

	para->name = gnoclGetAutoPrintOpId();

	g_signal_connect_after ( G_OBJECT ( para->operation ), "done", G_CALLBACK ( simpleDestroyFunc ), interp );

	gnoclMemNameAndPrintOp ( para->name, GTK_PRINT_OPERATION ( para->operation ) );

	Tcl_CreateObjCommand ( interp, para->name, printOperationFunc, para, NULL );

	Tcl_SetObjResult ( interp, Tcl_NewStringObj ( para->name, -1 ) );


}

