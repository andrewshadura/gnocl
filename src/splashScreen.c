/*

take an optional graphic,

	+--------------------------------------------+
	|                                            |
	|   PROGRAM NAME PROGRAM NAME PROGRAM NAME   |
	|   CAPTION                                  |
	|                                  VERSION   |
	|                                            |
	|   BYLINE BYLINE BYLINE BYLINE BYLINE       |
	|                                            |
	|   COPYRIGHT                                |
	|                                            |
	|   STATUS_MESSAGE STATUS_MESSAGE            |
	|   HHHHHHHHHHHH PROGRESS BAR HHHHHHHHHHHH   |
	+--------------------------------------------+

*/

/**
\page page_splashScreen gnocl::splashScreen
\htmlinclude splashScreen.html
**/

/**
\par Modification History
\verbatim
	2013-07: added commands, options, commands
\endverbatim
**/

#include "gnocl.h"
#include "gnoclparams.h"

static const int widthIdx			= 0;
static const int heightIdx			= 1;
static const int backgroundColorIdx = 3;
static const int progressBarIdx		= 4;
static const int appNameIdx			= 5;
static const int versionIdx			= 6;
static const int backgroundImageIdx	= 7;
static const int maskImageIdx		= 8;
static const int exitCommandIdx		= 9;
static const int byLineIdx			= 10;
static const int fontIdx			= 11;


/**
\brief
**/
static GnoclOption splashScreenOptions[] =
{
	/* inherited window options */
	{ "-width", GNOCL_INT, NULL },     /* these must be */
	{ "-height", GNOCL_INT, NULL },   /* before -visible! */
	{ "-backgroundColor", GNOCL_INT, NULL },   /* before -visible! */

	/* experimental options */
	{ "-child", GNOCL_OBJ, "", gnoclOptChild },
	{ "-data", GNOCL_OBJ, "", gnoclOptData },
	{ "-name", GNOCL_STRING, "name" },

	{ "-onDelete", GNOCL_OBJ, "", gnoclOptOnDelete },
	{ "-onDestroy", GNOCL_OBJ, "destroy", gnoclOptCommand },

	{ "-mask", GNOCL_OBJ, "", gnoclOptMask },
	//{ "-backgroundImage", GNOCL_OBJ, "", gnoclOptBackgroundImage },
	{NULL}
};


/**
\brief	Respond to the "destroy" signal: execute para->exitCommand and free memory associated with SplashScreenParams.
**/
static void destroyFunc ( GtkWidget *widget, gpointer data )
{

	SplashScreenParams *para = ( SplashScreenParams * ) data;

	//gnoclForgetWidgetFromName ( para->name );
	//Tcl_DeleteCommand ( para->interp, para->name );

	//g_print ( "destroyFunc 2\n" );
	//g_free ( para->name );
	//g_free ( para );

}

/**
\brief
**/
static int getParams ( int objc, Tcl_Obj * const objv[], gchar *f, gchar *c, gchar *p, gchar *w )
{
	if ( objc < 5 )
	{
		return;
	}


	gint i;

	for ( i = 4; i < objc - 1; i++ )
	{
		if ( strcmp ( Tcl_GetString ( objv[i] ), "-font" ) == 0 )
		{

			sprintf ( f, "%s", Tcl_GetString ( objv[i+1] ) );
		}

		if ( strcmp ( Tcl_GetString ( objv[i] ), "-color" ) == 0 )
		{

			sprintf ( c, "%s", Tcl_GetString ( objv[i+1] ) );
		}

		if ( strcmp ( Tcl_GetString ( objv[i] ), "-position" ) == 0 )
		{

			sprintf ( p, "%s", Tcl_GetString ( objv[i+1] ) );
		}

		if ( strcmp ( Tcl_GetString ( objv[i] ), "-weight" ) == 0 )
		{

			sprintf ( w, "%s", Tcl_GetString ( objv[i+1] ) );
		}
	}
}



/**
\brief	Code stub with all the pieces that work together.
**/

static int drawSplashScreen ( SplashScreenParams *para )
{

#ifdef DEBUG_SPLASHSCREEN
	g_print ( "drawSplashScreen\n" );
#endif

	cairo_surface_t *surface;
	cairo_t *cr;
	GdkPixbuf *pixbuf;

	gint i_width, i_height, l_width, l_height;
	gint dx, dy;
	gint s_stride, p_stride;
	gint i, j;
	guchar *s_data, *p_data;

	cairo_font_slant_t slant;
	cairo_font_weight_t weight;

	/* intialize cairo surface */
	if ( para->bgImage == NULL )
	{
		//para->pixbuf = gdk_pixbuf_new  (GDK_COLORSPACE_RGB, 0, 8, para->width, para->height);

	}

	cr = gnoclPixbufCairoCreate ( para->pixbuf );

	/* resize the main window */
	gtk_window_resize ( para->window, para->width, para->height );

//gtk_window_set_default_size ( para->window, para->width, para->height );

	/* background colour needs setting */

	/* set default font */
	//cairo_select_font_face ( cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD );
	//cairo_set_font_size ( cr, 20 );
	cairo_set_source_rgba ( cr, 1, 1, 1, 1.0 );

	dx = 10;
	dy = 50;
	gchar family[12];
	int size;
	gchar _slant[12];
	gchar _weight[12];
	gint ret;
	float r, g, b;

	/* APPNAME --------------------------------------------------------*/
	ret = sscanf ( para->appPos, "%d %d", &dx, &dy );
	ret = sscanf ( para->appFont, "%s %d %s %s", &family, &size, &_slant, &_weight );
	ret = sscanf ( para->appClr, "%f %f %f", &r, &g, &b );

	cairo_set_source_rgba ( cr, ( double ) r, ( double ) g, ( double ) b, 1.0 );
	cairo_select_font_face ( cr, family, CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL );
	cairo_set_font_size ( cr, ( double ) size );

	cairo_move_to ( cr, dx, dy );
	cairo_show_text ( cr, para->appName );


	/* CAPTION  -------------------------------------------------------*/
	sscanf ( para->capPos, "%d %d", &dx, &dy );
	ret = sscanf ( para->capFont, "%s %d %s %s", &family, &size, &_slant, &_weight );
	ret = sscanf ( para->capClr, "%f %f %f", &r, &g, &b );

	cairo_set_source_rgba ( cr, ( double ) r, ( double ) g, ( double ) b, 1.0 );

	cairo_select_font_face ( cr, family, CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL );
	cairo_set_font_size ( cr, ( double ) size );

	cairo_move_to ( cr, dx, dy );
	cairo_show_text ( cr, para->caption );

#ifdef DEBUG_SPLASHSCREEN
	g_print ( "drawSplashScreen, caption = %s\n", para->caption );
#endif

	/* VERSION  -------------------------------------------------------*/
	sscanf ( para->verPos, "%d %d", &dx, &dy );
	ret = sscanf ( para->verFont, "%s %d %s %s", &family, &size, &_slant, &_weight );
	ret = sscanf ( para->verClr, "%f %f %f", &r, &g, &b );

	cairo_set_source_rgba ( cr, ( double ) r, ( double ) g, ( double ) b, 1.0 );
	cairo_select_font_face ( cr, family, CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL );
	cairo_set_font_size ( cr, ( double ) size );

	cairo_move_to ( cr, dx, dy );
	cairo_show_text ( cr, para->version );

#ifdef DEBUG_SPLASHSCREEN
	g_print ( "drawSplashScreen, version = %s\n", para->version );
#endif

	/* BYLINE  --------------------------------------------------------*/
	sscanf ( para->byPos, "%d %d", &dx, &dy );
	ret = sscanf ( para->byFont, "%s %d %s %s", &family, &size, &_slant, &_weight );
	ret = sscanf ( para->byClr, "%f %f %f", &r, &g, &b );

	cairo_set_source_rgba ( cr, ( double ) r, ( double ) g, ( double ) b, 1.0 );
	cairo_select_font_face ( cr, family, CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL );
	cairo_set_font_size ( cr, ( double ) size );

	cairo_move_to ( cr, dx, dy );
	cairo_show_text ( cr, para->byLine );

#ifdef DEBUG_SPLASHSCREEN
	g_print ( "drawSplashScreen, byLine = %s\n", para->byLine );
#endif

	/* COPYRIGHT  -----------------------------------------------------*/
	sscanf ( para->crPos, "%d %d", &dx, &dy );
	ret = sscanf ( para->crFont, "%s %d %s %s", &family, &size, &_slant, &_weight );
	ret = sscanf ( para->crClr, "%f %f %f", &r, &g, &b );

	cairo_set_source_rgba ( cr, ( double ) r, ( double ) g, ( double ) b, 1.0 );
	cairo_select_font_face ( cr, family, CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL );
	cairo_set_font_size ( cr, ( double ) size );

	cairo_move_to ( cr, dx, dy );
	cairo_show_text ( cr, para->copyright );

#ifdef DEBUG_SPLASHSCREEN
	g_print ( "drawSplashScreen, copyright = %s\n", para->copyright );
#endif

	/* STATUS & PRORGRESS BAR  ----------------------------------------*/
	/* create the progress bar */

	/* will this draw a rectangle? */
	double x;
	double y;
	double width;
	double height;

	x = 5;
	height = 5;
	y = ( long ) para->height - 2 * height;
	width = ( long ) para->width - 2 * x ;

	dy = y - 8;
	cairo_move_to ( cr, x, dy );

	cairo_set_source_rgba ( cr, 1, 1, 1, 1.0 );
	cairo_set_font_size ( cr, ( double ) 10 );
	cairo_show_text ( cr, para->message );

	/* draw layout */
	cairo_move_to ( cr, dx, dy );
	cairo_fill ( cr );

	/* trough */
	cairo_set_line_width ( cr, 2.0 );
	cairo_set_source_rgba ( cr, 0.7, 0.7, 0.7, 1.0 );
	cairo_rectangle ( cr, x, y, width, height );
	cairo_fill_preserve ( cr );
//cairo_set_source_rgb( cr, .0, .0, 1.0 );
	cairo_stroke ( cr );

	/* bar */
	cairo_set_line_width ( cr, 2.0 );
	cairo_set_source_rgba ( cr, 0.4, 0.4, 0.8, 1.0 );

	//cairo_rectangle ( cr, x, y, width - 100, height );

	//width * ( (double) para->progress / (double) para->range );
	cairo_rectangle ( cr, x, y, width * ( ( double ) para->progress / ( double ) para->range ), height );

	cairo_fill_preserve ( cr );
//cairo_set_source_rgb( cr, .0, .0, 1.0 );
	cairo_stroke ( cr );
	/*-----------------------------*/

#ifdef DEBUG_SPLASHSCREEN
	g_print ( "done 1\n" );
#endif

	para->pixbuf2 = gnoclPixbufCairoDestroy ( cr, TRUE );

	/* background colour needs setting */
#ifdef DEBUG_SPLASHSCREEN
	g_print ( "done 2\n" );
#endif

	/* update image */
	gtk_image_set_from_pixbuf ( para->image, para->pixbuf2 );

	/* clear outstanding drawing operations, same as gnocl::update */
	gint n;
	gint nMax = 500;

	for ( n = 0; n < nMax && gtk_events_pending(); ++n )
	{
		gtk_main_iteration_do ( 0 );
	}


	return TCL_OK;
}

/**
\brief	Obtain current values for widget options.

static const int progressBarIdx  = 0;
static const int appNameIdx = 1;
static const int versionIdx     = 2;
static const int backgroundImageIdx  = 3;
static const int maskIdx = 4;
static const int exitCommandIdx = 5;
static const int byLineIdx     = 6;

typedef struct
{
	int  progBar;
	char *appName;
	char *version;
	char *byLine;
	char *bgImage;
	char *maskImage;
	char *exitCommand;
	GdkPixbuf *pixbuf;
	GdkPixbuf *mask;
	GtkImage *image;
	char *message;
	char *name;
	Tcl_Interp *interp;
	GtkWidget *window;
} SplashScreenParams;


**/
static int cget (   Tcl_Interp *interp, SplashScreenParams *para,  GnoclOption options[], int idx )
{

#ifdef DEBUG_SPLASHSCREEN
	printf ( "cget\n" );
#endif

	printf ( "CGET 3\n" );
	Tcl_Obj *obj = NULL;

	/* step 1) obtain the required option value */
	if ( idx == progressBarIdx )
	{
		gchar *str;
		sprintf ( str, "%d", para->progBar );
		obj = Tcl_NewStringObj ( str, -1 );
	}

	else if ( idx == appNameIdx )
	{
		obj = Tcl_NewStringObj ( para->appName, -1 );
	}

	else if ( idx == versionIdx )
	{
		obj = Tcl_NewStringObj ( para->version, -1 );
	}

	else if ( idx == backgroundImageIdx )
	{
		obj = Tcl_NewStringObj ( para->bgImage, -1 );
	}

	else if ( idx == maskImageIdx )
	{
		obj = Tcl_NewStringObj ( para->maskImage, -1 );
	}

	else if ( idx == exitCommandIdx )
	{
		obj = Tcl_NewStringObj ( para->exitCommand, -1 );
	}


	else if ( idx == byLineIdx )
	{
		printf ( "CGET 4\n" );
		//printf("byline = %s\n", para->byLine );
		printf ( "CGET 5\n" );
		//obj = Tcl_NewStringObj ( para->byLine, -1 );
		printf ( "CGET 6\n" );
	}

	/* step 2) return that value */
	if ( obj != NULL )
	{
		printf ( "CGET 7\n" );
		Tcl_SetObjResult ( interp, obj );
		return TCL_OK;
	}


	return gnoclCgetNotImplemented ( interp, options + idx );
}

/**
\brief	Set widget options based upon recieved values.

static const int progressBarIdx  = 0;
static const int appNameIdx = 1;
static const int versionIdx     = 2;
static const int backgroundImageIdx  = 3;
static const int maskIdx = 4;
static const int exitCommandIdx = 5;
static const int byLineIdx     = 6;

typedef struct
{
	int  progBar;
	char *appName;
	char *version;
	char *byLine;
	GdkPixbuf *pixbuf;
	GdkPixbuf *mask;
	GtkImage *image;
	char *message;
	char *name;
	Tcl_Interp *interp;
	GtkWidget *window;

} SplashScreenParams;

**/
static int configure ( Tcl_Interp *interp, SplashScreenParams *para, GnoclOption options[] )
{

#ifdef DEBUG_SPLASHSCREEN
	printf ( "splashscreen/configure\n" );
#endif


	if ( options[widthIdx].status == GNOCL_STATUS_CHANGED
			&& options[heightIdx].status == GNOCL_STATUS_CHANGED )
	{
		para->width  = options[widthIdx].val.i;
		para->height = options[heightIdx].val.i;
	}

	if ( options[backgroundColorIdx].status == GNOCL_STATUS_CHANGED )
	{
		para->bgclr  = options[backgroundColorIdx].val.str;
	}

	/* handle -progressBar option */
	if ( options[progressBarIdx].status == GNOCL_STATUS_CHANGED )
	{
		sscanf ( options[progressBarIdx].val.str, "%d", &para->progBar );

#ifdef DEBUG_SPLASHSCREEN
		printf ( "\tprogressBarIdx %d\n", para->progBar );
#endif

	}

	/* handle -programeName option */
	if ( options[appNameIdx].status == GNOCL_STATUS_CHANGED )
	{
		para->name = options[appNameIdx].val.str;

#ifdef DEBUG_SPLASHSCREEN
		printf ( "\tappNameIdx %s\n", para->name );
#endif

	}

	/* handle -mask option */
	if ( options[maskImageIdx].status == GNOCL_STATUS_CHANGED )
	{
		para->maskImage = options[maskImageIdx].val.str;

#ifdef DEBUG_SPLASHSCREEN
		printf ( "\tmaskIdx %s\n", para->maskImage );
#endif

	}

	/* handle -exitCommand option */
	if ( options[exitCommandIdx].status == GNOCL_STATUS_CHANGED )
	{
		para->exitCommand = options[exitCommandIdx].val.str;

#ifdef DEBUG_SPLASHSCREEN
		printf ( "\texitCommandIdx %s\n", para->exitCommand );
#endif

	}



	return TCL_OK;
}


/**
/brief
/author   Peter G Baum
          William J Giddings
/date
**/
int splashScreenFunc ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{

	static const char *cmds[] =
	{
		"add", "progress", "push",
		"delete", "configure", "cget",
		"class", "show", "options", "commands",
		NULL
	};
	enum cmdIdx
	{
		AddIdx, ProgressIdx, PushIdx,
		DeleteIdx, ConfigureIdx, CgetIdx,
		ClassIdx, ShowIdx, OptionsIdx, CommandsIdx
	};

	SplashScreenParams *para = ( SplashScreenParams * ) data;

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
				gnoclGetOptions ( interp, splashScreenOptions );
			}
			break;
		case ShowIdx:
			{
				drawSplashScreen ( para );
				gtk_widget_show_all ( para->window );
				gnoclUpdateCmd ( data, interp, objc, objv );
			}
			break;
		case AddIdx:
			{

				static const char *subCmds[] =
				{
					"appName", "caption", "version",
					"byLine", "copyright", "image",
					"mask", "background",
					NULL
				};

				enum subCmdIdx
				{
					AppNameIdx, CaptionIdx, VersionIdx,
					ByLineIdx, CopyrightIdx, ImageIdx,
					MaskIdx, BackgroundIdx
				};

				int subcmdidx;


				if ( Tcl_GetIndexFromObj ( interp, objv[2], subCmds, "command", TCL_EXACT, &subcmdidx ) != TCL_OK )
				{
					return TCL_ERROR;
				}


				switch ( subcmdidx )
				{

					case BackgroundIdx:
						{
#ifdef DEBUG_SPLASHSCREEN
							g_print ( "bgcolor = %s\n", Tcl_GetString ( objv[3] ) );
#endif
							para->bgclr = Tcl_GetString ( objv[3] );
							g_object_unref ( G_OBJECT ( para->pixbuf ) );
							para->pixbuf = gdk_pixbuf_new  ( GDK_COLORSPACE_RGB, 0, 8, para->width, para->height );
						}
						break;
					case MaskIdx:
						{
#ifdef DEBUG_SPLASHSCREEN
							g_print ( "mask -yet to be implemented\n" );
#endif
						}
						break;
					case ImageIdx:
						{

							para->bgImage = Tcl_GetString ( objv[3] );
#ifdef DEBUG_SPLASHSCREEN
							g_print ( "image = %s.\n", para->bgImage );
#endif
							/* release memory held in buffer */
							g_object_unref ( G_OBJECT ( para->pixbuf ) );
							para->pixbuf = gdk_pixbuf_new_from_file ( para->bgImage, NULL );

							/* get size of buffer */
							gdk_pixbuf_get_file_info ( para->bgImage, &para->width, &para->height );

							if ( para->pixbuf == NULL )
							{
								char *str;

								sprintf ( str, "ERROR: Image file \"%s\" could not be found.", para->bgImage );

								Tcl_SetResult ( interp, str, TCL_STATIC );
								return TCL_ERROR;
							}
						}
						break;
					case AppNameIdx:
						{
							sprintf ( para->appName, "%s", Tcl_GetString ( objv[3] ) );
							getParams ( objc, objv, &para->appFont,  &para->appClr, &para->appPos, &para->appWeight );
						}
						break;
					case CaptionIdx:
						{

							sprintf ( para->caption, "%s", Tcl_GetString ( objv[3] ) );
							getParams ( objc, objv, &para->capFont,  &para->capClr, &para->capPos, &para->capWeight );

						}
						break;
					case VersionIdx:
						{
							sprintf ( para->version, "%s", Tcl_GetString ( objv[3] ) );
							getParams ( objc, objv, &para->verFont,  &para->verClr, &para->verPos, &para->verWeight );

						}
					case ByLineIdx:
						{
							sprintf ( para->byLine, "%s", Tcl_GetString ( objv[3] ) );
							getParams ( objc, objv, &para->byFont, &para->byClr, &para->byPos, &para->byWeight );

						}
						break;
					case CopyrightIdx:
						{
							sprintf ( para->copyright, "%s", Tcl_GetString ( objv[3] ) );
							getParams ( objc, objv, &para->crFont,  &para->crClr, &para->crPos, &para->crWeight );

						}
						break;
					default: {}
						break;
				}

				gnoclUpdateCmd ( data, interp, objc, objv );
			}

			break;
		case ProgressIdx:
			{

				sscanf ( Tcl_GetString ( objv[2] ), "%d %d", &para->progress, &para->range );
				drawSplashScreen ( para );

			}
			break;

		case PushIdx:
			{

				sprintf ( para->message, "%s", Tcl_GetString ( objv[2] ) );
				drawSplashScreen ( para );
			}
			break;

		case ClassIdx:
			{
				Tcl_SetObjResult ( interp, Tcl_NewStringObj ( "splashScreen", -1 ) );
			}
			break;
		case DeleteIdx:
			{

				return gnoclDelete ( interp, GTK_WIDGET ( para->window ), objc, objv );
			}

			break;
		case ConfigureIdx:
			{
				int ret = TCL_ERROR;

				if ( gnoclParseAndSetOptions ( interp, objc - 1, objv + 1,
											   splashScreenOptions, G_OBJECT ( para->window ) ) == TCL_OK )
				{
					ret = configure ( interp, para, splashScreenOptions );
				}

				gnoclClearOptions ( splashScreenOptions );

				return ret;
			}

			break;
		case CgetIdx:
			{
				int     idx;
				printf ( "CGET 1\n" );

				switch ( gnoclCget ( interp, objc, objv, G_OBJECT ( para->window ), splashScreenOptions, &idx ) )
				{
					case GNOCL_CGET_ERROR:
						return TCL_ERROR;
					case GNOCL_CGET_HANDLED:
						return TCL_OK;
					case GNOCL_CGET_NOTHANDLED:
						printf ( "CGET 2\n" );
						return cget ( interp, para->window, splashScreenOptions, idx );
				}

				assert ( 0 );
			}

			break;
		default: {}
	}

	return TCL_OK;
}

/**
\brief
\author     William J Giddings
\date       12/Jan/2010
**/
int _SplashScreenFunc ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{


	/* set list of valid commands for this widget */
	static const char *cmds[] = { "delete", "configure", "cget", "show", "hide", "class", NULL };
	enum cmdIdx { DeleteIdx, ConfigureIdx, CgetIdx, ShowIdx, HideIdx, ClassIdx };

	int idx;
	GtkWidget *widget;

	widget  = GTK_WIDGET ( data );

	/* check the number of arguments received from the interpreter */
	if ( objc < 2 )
	{
		Tcl_WrongNumArgs ( interp, 1, objv, "command" );
		return TCL_ERROR;
	}

	/* get which command was passed */
	if ( Tcl_GetIndexFromObj ( interp, objv[1], cmds, "command", TCL_EXACT, &idx ) != TCL_OK )
	{
		return TCL_ERROR;
	}

	/* respsond to the commands received */
	switch ( idx )
	{
		case ClassIdx:
			{
				Tcl_SetObjResult ( interp, Tcl_NewStringObj ( "SplashScreen_CLASS", -1 ) );
			}
			break;
		case DeleteIdx:
			{
				return gnoclDelete ( interp, GTK_WIDGET ( widget ), objc, objv );
			}
		case ConfigureIdx:
			{
				int ret = TCL_ERROR;

				if ( gnoclParseAndSetOptions ( interp, objc - 1, objv + 1, splashScreenOptions, G_OBJECT ( widget ) ) == TCL_OK )
				{
					ret = configure ( interp, widget, splashScreenOptions );
				}

				gnoclClearOptions ( splashScreenOptions );

				return ret;
			}

			break;
		case CgetIdx:
			{
				int     idx;

				switch ( gnoclCget ( interp, objc, objv, G_OBJECT ( widget ), splashScreenOptions, &idx ) )
				{
					case GNOCL_CGET_ERROR:
						return TCL_ERROR;
					case GNOCL_CGET_HANDLED:
						return TCL_OK;
					case GNOCL_CGET_NOTHANDLED:
						return cget ( interp, widget, splashScreenOptions, idx );
				}
			}
	}

	return TCL_OK;

}

/**
\brief	Create and instance of the gnocl::splashScreen widget.

typedef struct
{
	int  progBar;
	char *appName;
	char *version;
	char *byLine;
	char *bgImage;
	char *maskImage;
	char *exitCommand;
	GdkPixbuf *pixbuf;
	GdkPixbuf *mask;
	GtkImage *image;
	char *message;
	char *name;
	Tcl_Interp *interp;
	GtkWidget *window;
} SplashScreenParams;


**/
int gnoclSplashScreenCmd ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{

	int ret;
	//GtkImage *image;
	GdkPixbuf *pixbuf;

	SplashScreenParams *para;

	/* 1) check the validity of the options passed from the interpeter */
	if ( gnoclParseOptions ( interp, objc, objv, splashScreenOptions ) != TCL_OK )
	{
		gnoclClearOptions ( splashScreenOptions );
		return TCL_ERROR;
	}

	/* obtain necessary values */
	para = g_new ( SplashScreenParams, 1 );
	para->interp = interp;


	para->progress = 1;
	para->range = 100;

	/* some defaults */

	/* 2) initialize the widget and set options passed from the interpreter */
	para->window = GTK_WINDOW ( gtk_window_new ( GTK_WINDOW_TOPLEVEL ) );
	gtk_window_set_type_hint ( para->window, GDK_WINDOW_TYPE_HINT_SPLASHSCREEN );
	//gtk_window_set_decorated  ( para->window, 0 );
	gtk_window_set_position ( para->window , GTK_WIN_POS_CENTER );

	/* a) apply base widget properties and signals */
	ret = gnoclSetOptions ( interp, splashScreenOptions, G_OBJECT ( para->window ), -1 );

	/* b) apply options not handled above */
	if ( ret == TCL_OK )
	{
		ret = configure ( interp, para, splashScreenOptions );
	}

	/* c) free up memory */
	gnoclClearOptions ( splashScreenOptions );

	/* d) in the case of invalid options, destroy the widget instances and quit */
	if ( ret != TCL_OK )
	{
		gtk_widget_destroy ( GTK_WIDGET ( para->window ) );
		return TCL_ERROR;
	}

	/* create a default image to place into window */
	/* window sizes obtained from configure above */
	para->pixbuf = gdk_pixbuf_new  ( GDK_COLORSPACE_RGB, 0, 8, para->width, para->height );

	/* build up graphic */
	para->image = gtk_image_new_from_pixbuf ( para->pixbuf );
	gtk_container_add ( GTK_CONTAINER ( para->window ), para->image );

	/* 3) everything is ok, so realize the widget */

	/* 4) register the widget with the interpreter and associate it with a handler function */
	para->name = gnoclGetAutoWidgetId();

	g_signal_connect ( G_OBJECT ( para->window ), "destroy", G_CALLBACK ( destroyFunc ), para );

	gnoclMemNameAndWidget ( para->name, GTK_WIDGET ( para->window ) );

	Tcl_CreateObjCommand ( interp, para->name, splashScreenFunc, para, NULL );

	Tcl_SetObjResult ( interp, Tcl_NewStringObj ( para->name, -1 ) );

	return TCL_OK;
}

