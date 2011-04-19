/**
\brief     This module implements the .
\author    Peter G. Baum, William J Giddings
\history
 	01/05/10	Began Development

\notes
	A GtkPageSetup object stores the page size, orientation and margins.
	The idea is that you can get one of these from the page setup dialog
	and then pass it to the GtkPrintOperation when printing. The benefit
	of splitting this out of the GtkPrintSettings is that these affect
	the actual layout of the page, and thus need to be set long before
	user prints.

	The margins specified in this object are the "print margins",
	i.e. the parts of the page that the printer cannot print on. These
	are different from the layout margins that a word processor uses;
	they are typically used to determine the minimal size for the layout
	margins.


struct _GtkPaperSize
{
  const PaperInfo *info;

  gchar *name;
  gchar *display_name;
  gchar *ppd_name;

  gdouble width, height;
  gboolean is_custom;
};


**/

#include "gnocl.h"
#include "gnoclparams.h"

#include <string.h>
#include <assert.h>

/* needs to be public to be accessed by gnocl::inventory */
static GHashTable *name2pagesetupList;
static const char idPrefix[] = "::gnocl::_PS";

/*
typedef enum
{
  GTK_UNIT_PIXEL,
  GTK_UNIT_POINTS,
  GTK_UNIT_INCH,
  GTK_UNIT_MM
} GtkUnit;
*/

static GnoclOption pageSetUpOptions[] =
{
	/* gnocl options */
	{ "-orientation", GNOCL_STRING, NULL },
	{ "-paperSize", GNOCL_STRING, NULL},
	{ "-margins", GNOCL_STRING, NULL},
	{ "-units", GNOCL_STRING, NULL},
	{ "-height", GNOCL_STRING, NULL},
	{ "-width", GNOCL_STRING, NULL},
	{ NULL },
};

/* used by configure */
static const int orientationIdx  = 0;
static const int paperSizeIdx  = 1;
static const int marginsIdx  = 2;
static const int unitsIdx  = 3;
static const int heightIdx  = 4;
static const int widthIdx  = 5;


/**
\brief
\author     Peter G Baum, William J Giddings
\date       01/05/10
\since      0.9.95
\notes
typedef enum
{
  GTK_UNIT_PIXEL,
  GTK_UNIT_POINTS,
  GTK_UNIT_INCH,
  GTK_UNIT_MM
} GtkUnit;

GTK_PAPER_NAME_A3
GTK_PAPER_NAME_A4
GTK_PAPER_NAME_A5
GTK_PAPER_NAME_B5
GTK_PAPER_NAME_LETTER
GTK_PAPER_NAME_EXECUTIVE
GTK_PAPER_NAME_LEGAL

*/
static int configure ( Tcl_Interp *interp,	PaperSetupParams *para, GnoclOption options[] )
{

	//g_print ( "configure 1 setup = %s\n", para->name );

	/*
	g_print ( "%d %d %d %d\n",
			  options[orientationIdx].status,
			  options[paperSizeIdx].status,
			  options[marginsIdx].status,
			  options[unitsIdx].status );
	*/

	if ( options[orientationIdx].status == GNOCL_STATUS_CHANGED )
	{

		//g_print ( "---------->orientation = %s\n", options[orientationIdx].val.str );

		if ( !strcmp ( options[orientationIdx].val.str, "portrait" ) )
		{
			gtk_page_setup_set_orientation ( para->setup, GTK_PAGE_ORIENTATION_PORTRAIT );
		}

		if ( !strcmp ( options[orientationIdx].val.str, "landscape" ) )
		{
			gtk_page_setup_set_orientation ( para->setup, GTK_PAGE_ORIENTATION_LANDSCAPE );
		}

		if ( !strcmp ( options[orientationIdx].val.str, "reverse-portrait" ) )
		{
			gtk_page_setup_set_orientation ( para->setup, GTK_PAGE_ORIENTATION_REVERSE_PORTRAIT );
		}

		if ( !strcmp ( options[orientationIdx].val.str, "reverse-landscape" ) )
		{
			gtk_page_setup_set_orientation ( para->setup, GTK_PAGE_ORIENTATION_REVERSE_LANDSCAPE );
		}

	}

	//g_print ( "configure 2\n" );

	if ( options[paperSizeIdx].status == GNOCL_STATUS_CHANGED )
	{
		//gint size;

		//size = GTK_PAPER_NAME_A4;
		//gtk_page_setup_set_paper_size (para->setup,size);

		//para->size = options[paperSizeIdx].val.str;

		/*
				if  ( strcmp ( options[paperSizeIdx].val.str, "iso_a3 iso_a4 iso_a5 iso_b5 na_letter na_executive na_legal") )
				{
					gtk_page_setup_set_paper_size ( para->setup, options[paperSizeIdx].val.str );
					g_print ( "OK %s\n", options[paperSizeIdx].val.str );
				}
		*/
		//g_print ( "---------->papersize = %s\n", options[paperSizeIdx].val.str );

	}

	//g_print ( "configure 3\n" );

	if ( options[marginsIdx].status == GNOCL_STATUS_CHANGED )
	{
		//GtkUnit unit;

		//gdouble margin;

		//gtk_page_setup_set_top_margin (setup,margin.unit );
		//gtk_page_setup_set_bottom_margin (setup, margin, unit);
		//gtk_page_setup_set_left_margin(setup,margin.unit );
		//gtk_page_setup_set_left_margin(setup,margin.unit );
		//g_print ( "---------->margins = %s\n", options[marginsIdx].val.str );
	}

	//g_print ( "configure 4\n" );

	if ( options[unitsIdx].status == GNOCL_STATUS_CHANGED )
	{

		if  ( !strcmp ( options[unitsIdx].val.str , "pixel" ) )
		{
			//g_print ( "OK pixel\n" );
			para->unit = GTK_UNIT_PIXEL;
		}

		if  ( !strcmp ( options[unitsIdx].val.str, "points" ) )
		{
			//g_print ( "OK points\n" );
			para->unit = GTK_UNIT_POINTS;
		}

		if  ( !strcmp ( options[unitsIdx].val.str, "inch" ) )
		{
			//g_print ( "OK inch\n" );
			para->unit = GTK_UNIT_INCH;
		}


		if  ( !strcmp ( options[unitsIdx].val.str, "mm" ) )
		{
			//g_print ( "OK mm\n" );
			para->unit = GTK_UNIT_MM;
		}

		//g_print ( "---------->units = %s %d\n", options[unitsIdx].val.str, para->unit );


	}

	return TCL_OK;
}


/**
\brief  		Obtain current -option values.
\author		Peter G Baum, William J Giddings
\date			05/05/2010

struct _GtkPaperSize
{
  const PaperInfo *info;

  gchar *name;
  gchar *display_name;
  gchar *ppd_name;

  gdouble width, height;
  gboolean is_custom;
};

**/
static int cget ( Tcl_Interp *interp, PaperSetupParams *para, GnoclOption options[], int idx )
{

	//g_print ( "cget, idx = %d\n", idx );

	gchar *buffer;

	/* 0 */
	if ( idx == orientationIdx )
	{

		//g_print ( "orientation = %d \n", gtk_page_setup_get_orientation  ( para->setup ) );

		/*
				GTK_PAGE_ORIENTATION_PORTRAIT,
				GTK_PAGE_ORIENTATION_LANDSCAPE,
				GTK_PAGE_ORIENTATION_REVERSE_PORTRAIT,
				GTK_PAGE_ORIENTATION_REVERSE_LANDSCAPE
		*/
		switch ( gtk_page_setup_get_orientation  ( para->setup ) )
		{
			case GTK_PAGE_ORIENTATION_PORTRAIT:
				{
					Tcl_SetResult ( interp, "portrait", TCL_STATIC );
				} break;
			case GTK_PAGE_ORIENTATION_LANDSCAPE:
				{
					Tcl_SetResult ( interp, "landscape", TCL_STATIC );
				} break;
			case GTK_PAGE_ORIENTATION_REVERSE_PORTRAIT:
				{
					Tcl_SetResult ( interp, "reverse-portrait", TCL_STATIC );
				} break;
			case GTK_PAGE_ORIENTATION_REVERSE_LANDSCAPE:
				{
					Tcl_SetResult ( interp, "reverse-landscape", TCL_STATIC );
				} break;
		}

		return TCL_OK;
	}

	/* 1 */
	if ( idx == paperSizeIdx )
	{

		/* this is causing an error, not accepting para->setup as GtkPageSetup */
		/* obtain from para->size */

		/*
		g_print ( "size = %s\n", gtk_paper_size_get_name  ( para->size ) );
		g_print ( "display_name = %s\n", gtk_paper_size_get_display_name  ( para->size ) );
		g_print ( "ppd_name = %s\n",  gtk_paper_size_get_ppd_name  ( para->size ) );

		g_print ( "width = %f mm\n", gtk_paper_size_get_name  ( para->size ) );
		g_print ( "height = %f mm\n", gtk_paper_size_get_name  ( para->size ) );
		*/

		char str[256];
		sprintf ( str, "%s", gtk_paper_size_get_name  ( para->size ) );

		Tcl_SetResult ( interp, str, TCL_STATIC );
		return TCL_OK;
	}

	/* 2 */
	if ( idx == marginsIdx )
	{

		char str[256];

		gdouble topMargin = gtk_page_setup_get_top_margin ( para->setup, para->unit );
		gdouble bottomMargin = gtk_page_setup_get_bottom_margin ( para->setup, para->unit );
		gdouble leftMargin = gtk_page_setup_get_left_margin ( para->setup, para->unit );
		gdouble rightMargin = gtk_page_setup_get_right_margin ( para->setup, para->unit );

		sprintf ( str, "%f %f %f %f", topMargin, bottomMargin, leftMargin, rightMargin  );

		Tcl_SetResult ( interp, str, TCL_STATIC );
		return TCL_OK;
	}

	/* 3 */
	if ( idx == unitsIdx )
	{

		//g_print ( "unitIdx para->unit = %d\n", para->unit );

		if ( para->unit == GTK_UNIT_PIXEL )
		{
			Tcl_SetResult ( interp, "pixel", TCL_STATIC );
		}

		if ( para->unit ==  GTK_UNIT_POINTS )
		{
			Tcl_SetResult ( interp, "points", TCL_STATIC );
		}

		if ( para->unit ==  GTK_UNIT_INCH )
		{
			Tcl_SetResult ( interp, "inch", TCL_STATIC );
		}

		if ( para->unit ==  GTK_UNIT_MM )
		{
			Tcl_SetResult ( interp, "mm", TCL_STATIC );
		}

		return TCL_OK;
	}

	if ( idx == heightIdx )
	{
		//g_print ( "cget -height 	working on this one...\n" );
		return TCL_OK;
	}

	if ( idx == widthIdx )
	{
		//g_print ( "cget -width 	working on this one...\n" );
		return TCL_OK;
	}


	return gnoclCgetNotImplemented ( interp, options + idx );
}

/**
\brief
\author
\date
\since
**/
int pageSetupFunc ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{

	static const char *cmds[] =
	{
		"delete", "configure", "cget", "class", NULL
	};

	enum cmdIdx
	{
		DeleteIdx, ConfigureIdx, CgetIdx, ClassIdx
	};


	//g_print ( "pageSetupFunc 1\n" );

	PaperSetupParams *para = ( PaperSetupParams * ) data;

	int idx;

	if ( objc < 2 )
	{
		Tcl_WrongNumArgs ( interp, 1, objv, "command" );
		return TCL_ERROR;
	}

	//g_print ( "pageSetupFunc 2\n" );

	if ( Tcl_GetIndexFromObj ( interp, objv[1], cmds, "command", TCL_EXACT, &idx ) != TCL_OK )
	{
		return TCL_ERROR;
	}

	//g_print ( "pageSetupFunc 3\n" );

	switch ( idx )
	{
		case DeleteIdx: {} break;
		case ConfigureIdx:
			{


				int ret = TCL_ERROR;

				if ( gnoclParseAndSetOptions ( interp, objc - 1, objv + 1,
											   pageSetUpOptions, G_OBJECT ( para->setup ) ) == TCL_OK )
				{
					ret = configure ( interp, para, pageSetUpOptions );
				}



				//g_print ( "ConfigureIdx\n" );
				//configure ( interp, para, pageSetUpOptions );
				gnoclClearOptions ( pageSetUpOptions );
				return ret;
			} break;
		case CgetIdx:
			{
				int idx;
				//g_print ( "-----\n" );

				switch ( gnoclCget ( interp, objc, objv,  para, pageSetUpOptions, &idx ) )
				{
					case GNOCL_CGET_ERROR:
						return TCL_ERROR;
					case GNOCL_CGET_HANDLED:
						return TCL_OK;
					case GNOCL_CGET_NOTHANDLED:
						return cget ( interp, para, pageSetUpOptions, idx );
				}

				//g_print ( "-----\n" );
			}
			break;
		case ClassIdx:
			{
			}
			break;
		default : {}

	}

	return TCL_OK;
}

/**
\brief
\author     Peter G Baum, William J Giddings
\date       01/05/10
\since      0.9.95
*/
void gnoclGetPageSetupList ( GList **list )
{
	g_hash_table_foreach ( name2pagesetupList, hash_to_list, list );
}

/**
\brief
\author     Peter G Baum, William J Giddings
\date       01/05/10
\since      0.9.95
\note      "char *" and not "const char *" because of a not very strict
            handling of "const char *" in Tcl e.g. Tcl_CreateObjCommand
*/
char *gnoclGetAutoPageSetupId ( void )
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

/**
\brief
\author     Peter G Baum, William J Giddings
\date       01/05/10
\since      0.9.95
**/
int gnoclMemNameAndPageSetup ( const char *path,  GtkPageSetup *setup )
{

	int n ;

	n = atoi ( path + sizeof ( idPrefix ) - 1 );

	assert ( n > 0 );
	assert ( g_hash_table_lookup ( name2pagesetupList, GINT_TO_POINTER ( n ) ) == NULL );
	assert ( strncmp ( path, idPrefix, sizeof ( idPrefix ) - 1 ) == 0 );

	/* memorize the name of the widget in the widget */
	g_object_set_data ( G_OBJECT ( setup ), "gnocl::name", ( char * ) path );
	g_hash_table_insert ( name2pagesetupList, GINT_TO_POINTER ( n ), setup );

	return 0;
}


/**
\brief
\author     Peter G Baum, William J Giddings
\date       01/05/10
\since      0.9.95
**/
/* -----------------
   handle widget <-> name mapping
-------------------- */
GtkPageSetup *gnoclGetPageSetupName ( const char *id, Tcl_Interp *interp )
{
	GtkPageSetup *pagesetup = NULL;
	int       n;

	if ( strncmp ( id, idPrefix, sizeof ( idPrefix ) - 1 ) == 0
			&& ( n = atoi ( id + sizeof ( idPrefix ) - 1 ) ) > 0 )
	{
		pagesetup = g_hash_table_lookup ( name2pagesetupList, GINT_TO_POINTER ( n ) );
	}

	if ( pagesetup == NULL && interp != NULL )
	{
		Tcl_AppendResult ( interp, "Unknown pixbuf \"", id, "\".", ( char * ) NULL );
	}

	return pagesetup;
}

/**
\brief      Returns the widget name associated with pointer
\author     Peter G Baum, William J Giddings
\date       01/05/10
\since      0.9.95
**/
const char *gnoclGetNameFromPageSetup ( GtkPageSetup *pagesetup )
{
	const char *name = g_object_get_data ( G_OBJECT ( pagesetup ), "gnocl::name" );

	return name;
}

/**
\brief      Returns the widget name associated with pointer
\author     Peter G Baum, William J Giddings
\date       01/05/10
\since      0.9.95
**/
GtkPageSetup *gnoclGetPageSetupFromName ( const char *id, Tcl_Interp *interp )
{
	GtkPageSetup *pagesetup = NULL;
	int       n;

	if ( strncmp ( id, idPrefix, sizeof ( idPrefix ) - 1 ) == 0
			&& ( n = atoi ( id + sizeof ( idPrefix ) - 1 ) ) > 0 )
	{
		pagesetup = g_hash_table_lookup ( name2pagesetupList, GINT_TO_POINTER ( n ) );
	}

	if ( pagesetup == NULL && interp != NULL )
	{
		Tcl_AppendResult ( interp, "Unknown pagesetup \"", id, "\".", ( char * ) NULL );
	}

	return pagesetup;
}



/**
\brief
\author     William J Giddings
\date       01/05/10
\since      0.9.95
**/
int gnoclPageSetupCmd ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{

#ifdef DEBUG_PRINTER_DIALOG
	g_printf ( "gnoclPageSetupCmd\n" );
	gint _i;


	for ( _i = 0; _i < objc; _i++ )
	{
		g_printf ( "\targ %d = %s\n", _i,  Tcl_GetString ( objv[_i] ) );
	}

#endif

	/* create new hash table if one does not exist */
	if ( name2pagesetupList == NULL )
	{
		name2pagesetupList = g_hash_table_new ( g_direct_hash, g_direct_equal );
	}

#ifdef DEBUG_PAGESETUP
	g_printf ( "gnoclPageSetupCmd\n" );
	gint _i;

	for ( _i = 0; _i < objc; _i++ )
	{
		g_printf ( "\targ %d = %s\n", _i,  Tcl_GetString ( objv[_i] ) );
	}

#endif

	PaperSetupParams *para;

	para = g_new ( PaperSetupParams, 1 );

	/* create page using default sizes */
	para->size = gtk_paper_size_new ( "iso_a4" );

	/* create setup */
	para->setup = GTK_PAGE_SETUP ( gtk_page_setup_new () );

	/* assign paper size to the page setup */
	gtk_page_setup_set_paper_size_and_default_margins ( para->setup, para->size );


	para->unit = GTK_UNIT_MM;
	para->name = gnoclGetAutoPageSetupId();

	//g_printf ( "gnoclPageSetupCmd -1 name = %s\n", para->name );

	/*
		if ( gnoclParseOptions ( interp, objc, objv, pageSetUpOptions ) != TCL_OK )
		{
			gnoclClearOptions ( pageSetUpOptions );
			return TCL_ERROR;
		}
	*/
	configure ( interp, para, pageSetUpOptions );

	//g_printf ( "gnoclPageSetupCmd -2 name = %s\n", para->name );
	gnoclClearOptions ( pageSetUpOptions );
	gnoclMemNameAndPageSetup ( para->name, para->setup );

	//g_printf ( "gnoclPageSetupCmd -3 name = %s\n", para->name );
	Tcl_CreateObjCommand ( interp, para->name, pageSetupFunc, para, NULL );
	Tcl_SetObjResult ( interp, Tcl_NewStringObj ( para->name, -1 ) );

	return TCL_OK;

}
