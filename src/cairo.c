/**
\brief	Cairo based drawing utility functions.
\author	Tadej Borovšak	tadeboro@gmail.com

The process of single drawing operations is processor intensive and inefficient when more than one drawing operation is needed.
In this case, create a sub-script which can be parsed and all operations completed sequentially.

**/

/*
	syntax:	gnocl::cairo cmd <widget-id> [options val.....]

	commands:
		line
		polyline
		arc
		circle
		rectangle
		square
		ellipse
		polygon
		text

		transalate
		rotate
		scale
		shear

		set

		composite
		clip
		mask

		default (reset defaults for options)

	options
		-lineColor / -lineColour
		-dash
		-lineWidth
		-fillColor / -fillColour
		-gradient
		-pattern
		-transparency
		-join
		-cap

*/

#include <gtk/gtk.h>
#include "gnocl.h"

struct ccolor
{
	float r;
	float g;
	float b;
	float a;
};

struct cfont
{
	gchar face[24];
	gint size;
	gchar slant[8];
	gchar weight[8];
};


struct _CairoParams
{
	struct ccolor line;
	struct ccolor fill;
	float lineWidth;
	gchar *dash;
	cairo_pattern_t *pat;
	struct cfont font;
};

struct _CairoParams CairoParams;

typedef struct _Data Data;

struct _Data
{
	GtkWidget *box,
			  *image,
			  *entry;
	GdkPixbuf *pixbuf;
};


static gchar *dash;


/* Key for automated pixbuf updating and destruction */
static const cairo_user_data_key_t pixbuf_key;

cairo_t   *gnoclPixbufCairoCreate ( GdkPixbuf *pixbuf );
GdkPixbuf *gnoclPixbufCairoDestroy ( cairo_t   *cr, gboolean   create_new_pixbuf );

/* Key for automated pixbuf updating and destruction */
static const cairo_user_data_key_t pixbuf_key;

/**
\brief
**/
static void getToken ( char *str, char **ptr, int *i )
{

	g_print ( "getToken ptr = %s\n", str );

	char buffer[] = "--------"; /* buffer to hold parsed elements */
	char *tmp = str;
	int k = *i;
	int b = 0;

	while ( tmp[k] != NULL )
	{
		if ( tmp[k] != ' ' )
		{
			/* add to token */
			buffer[b++] = tmp[k];
			g_print ( "buffer = %s\n", buffer );
		}

		else
		{
			/* add to buffer */
			g_print ( "returning value\n" );
			*ptr = buffer;
			*i = k;
			return 1;
		}

		g_print ( "ptr = %d\n", k );
		//tmp++;
		k++;
	}

	return 0;
}

/**
\brief Set drawing context.
\arguments

**/
static setLineContext ( cairo_t *cr )
{
#ifdef DEBUG_CAIRO
	g_print ( "%s\n", __FUNCTION__ );
#endif

	cairo_set_line_width ( cr, CairoParams.lineWidth );
	cairo_set_source_rgba ( cr, CairoParams.line.r, CairoParams.line.g, CairoParams.line.b, CairoParams.line.a );
}

/**
\brief
**/
static setFillContext ( cairo_t *cr )
{
#ifdef DEBUG_CAIRO
	g_print ( "%s\n", __FUNCTION__ );
#endif

	cairo_set_source_rgba ( cr, CairoParams.fill.r, CairoParams.fill.g, CairoParams.fill.b, CairoParams.fill.a );
}


/**

**/
static setTextContext ( cairo_t *cr )
{
#ifdef DEBUG_CAIRO
	g_print ( "%s\n", __FUNCTION__ );
#endif

	gint idx;
	gint slant, weight;

	slant = 0;
	weight = 0;

	g_print ( "font   => %s\n", CairoParams.font.face );
	g_print ( "size   => %d\n", CairoParams.font.size );
	g_print ( "slant  => %s\n", CairoParams.font.slant );
	g_print ( "weight => %s\n", CairoParams.font.weight );

	static char *slantOptions[] =
	{
		"normal", "italic", "oblique",
		NULL
	};

	static enum  slantIdx
	{
		SNormalIdx, ItalicIdx, ObliqueIdx
	};

	getIdx ( slantOptions, CairoParams.font.slant, &idx );

	switch ( idx )
	{
		case SNormalIdx:
			{
				slant = CAIRO_FONT_SLANT_NORMAL;
			}
			break;
		case ItalicIdx:
			{
				slant = CAIRO_FONT_SLANT_ITALIC;
			}
			break;
		case ObliqueIdx:
			{
				slant = CAIRO_FONT_SLANT_OBLIQUE;
			}
		default:
			{
				return TCL_ERROR;
			}
	}

	static char *weightOptions[] =
	{
		"normal", "bold",
		NULL
	};

	static enum  weightIdx
	{
		WNormalIdx, BoldIdx
	};

	getIdx ( weightOptions, CairoParams.font.weight, &idx );

	switch ( idx )
	{
		case WNormalIdx:
			{
				weight = CAIRO_FONT_WEIGHT_NORMAL;
			}
			break;
		case BoldIdx:
			{
				weight = CAIRO_FONT_WEIGHT_BOLD;
			}
			break;
		default:
			{
				return TCL_ERROR;
			}
	}

	cairo_select_font_face ( cr, CairoParams.font.weight, slant, weight );
	cairo_set_font_size ( cr, CairoParams.font.size );
}

/**
\brief	Get the number of entries in a line dash.
\note	Cannot use strtok or strtok_r but need a similar reetrant token parser.
**/
static int getDashLength ( char *str )
{
#ifdef DEBUG_CAIRO
	g_print ( "%s getDashLength %s\n", __FUNCTION__, str );
#endif

	char *cp;
	int l;

	cp = str;

	int	i = 0;	/* counter to step through the string */
	int	j = 1;	/* aggregate number of token seperators found */
	int	k = 0; /* toggle used to note whether previous char was ' ' */

	/* no value, no tokens */
	if ( cp == NULL )
	{
		return 0;
	}

	l = strlen ( str );

	g_printf ( "stringlen = %d\n", l );

	/* this step through the string, using array subscripts */
	while ( cp[i] != NULL )
	{
		/* detect whitespace */
		if ( cp[i] == ' ' )
		{

			/* ignore leading/trailing white space */
			if ( i == 0 || i == l - 1 )
			{
				i++ ;
				continue;
			}

			/* skip any duplicate spaces */
			if ( k )
			{
				j++;
			}

			k = 0;
		}

		else
		{
			k = 1;
		}

		i++;
	}

	return j;
}

/**
\brief set dash for the current drawing action.
**/
static void setDash ( cairo_t *cr )
{

	g_print ( "setDash:\n" );

	char *token;
	char *str;
	char *sep;
	char *_dash;
	gint i, len;

	sep = " ";

	_dash = g_new ( char, sizeof ( dash ) );

	strcpy ( _dash, dash );

	g_print ( "setDash: Splitting string \"%s\" into tokens:\n", dash );

	len = getDashLength ( dash );

	/* don't understand why this isn't working */
	if ( 0 )
	{
		sprintf ( dash, "%s", CairoParams.dash );
		len = getDashLength ( CairoParams.dash );
	}


	g_print ( "setDash: length %d\n", len );

	char *tmp;
	int ii = 0;

	getToken ( "apple bannana", &tmp, &ii );

	g_print ( "setDash: length ii = %d tmp = %s\n", ii, &tmp );

	/* set the dash for the current drawing context */

	/* create array to hold floats */


	//cairo_set_dash ( cr, dashes, len, 0 );

}

/**
\brief	This function will initialize new cairo context with contents of
		@pixbuf. You can then draw using returned context. When finished
		drawing, you must call gnoclPixbufCairoDestroy() or your pixbuf will
		not be updated with new contents!

 	Return value: New cairo_t context. When you're done with it, call
 	gnoclPixbufCairoDestroy() to update your pixbuf and free memory.
**/
cairo_t * gnoclPixbufCairoCreate ( GdkPixbuf *pixbuf )
{

	gint width;					/* Width of both pixbuf and surface */
	gint height;    			/* Height of both pixbuf and surface */
	gint p_stride;  			/* Pixbuf stride value */
	gint p_n_channels; 			/* RGB -> 3, RGBA -> 4 */
	gint s_stride;     			/* Surface stride value */
	guchar  *p_pixels;  		/* Pixbuf's pixel data */
	guchar *s_pixels;			/* Surface's pixel data */
	cairo_surface_t *surface;	/* Temporary image surface */
	cairo_t *cr;           		/* Final context */

	g_object_ref ( G_OBJECT ( pixbuf ) );

	/* Inspect input pixbuf and create compatible cairo surface */
	g_object_get ( G_OBJECT ( pixbuf ), "width",           &width,
				   "height",          &height,
				   "rowstride",       &p_stride,
				   "n-channels",      &p_n_channels,
				   "pixels",          &p_pixels,
				   NULL );
	surface = cairo_image_surface_create ( CAIRO_FORMAT_ARGB32, width, height );
	s_stride = cairo_image_surface_get_stride ( surface );
	s_pixels = cairo_image_surface_get_data ( surface );

	/* Copy pixel data from pixbuf to surface */
	while ( height-- )
	{
		gint    i;
		guchar *p_iter = p_pixels,
				*s_iter = s_pixels;

		for ( i = 0; i < width; i++ )
		{
#if G_BYTE_ORDER == G_LITTLE_ENDIAN

			/* Pixbuf:  RGB(A)
			 * Surface: BGRA */
			if ( p_n_channels == 3 )
			{
				s_iter[0] = p_iter[2];
				s_iter[1] = p_iter[1];
				s_iter[2] = p_iter[0];
				s_iter[3] = 0xff;
			}

			else /* p_n_channels == 4 */
			{
				gdouble alpha_factor = p_iter[3] / ( gdouble ) 0xff;

				s_iter[0] = ( guchar ) ( p_iter[2] * alpha_factor + .5 );
				s_iter[1] = ( guchar ) ( p_iter[1] * alpha_factor + .5 );
				s_iter[2] = ( guchar ) ( p_iter[0] * alpha_factor + .5 );
				s_iter[3] =           p_iter[3];
			}

#elif G_BYTE_ORDER == G_BIG_ENDIAN

			/* Pixbuf:  RGB(A)
			 * Surface: ARGB */
			if ( p_n_channels == 3 )
			{
				s_iter[3] = p_iter[2];
				s_iter[2] = p_iter[1];
				s_iter[1] = p_iter[0];
				s_iter[0] = 0xff;
			}

			else /* p_n_channels == 4 */
			{
				gdouble alpha_factor = p_iter[3] / ( gdouble ) 0xff;

				s_iter[3] = ( guchar ) ( p_iter[2] * alpha_factor + .5 );
				s_iter[2] = ( guchar ) ( p_iter[1] * alpha_factor + .5 );
				s_iter[1] = ( guchar ) ( p_iter[0] * alpha_factor + .5 );
				s_iter[0] =           p_iter[3];
			}

#else /* PDP endianness */

			/* Pixbuf:  RGB(A)
			 * Surface: RABG */
			if ( p_n_channels == 3 )
			{
				s_iter[0] = p_iter[0];
				s_iter[1] = 0xff;
				s_iter[2] = p_iter[2];
				s_iter[3] = p_iter[1];
			}

			else /* p_n_channels == 4 */
			{
				gdouble alpha_factor = p_iter[3] / ( gdouble ) 0xff;

				s_iter[0] = ( guchar ) ( p_iter[0] * alpha_factor + .5 );
				s_iter[1] =           p_iter[3];
				s_iter[1] = ( guchar ) ( p_iter[2] * alpha_factor + .5 );
				s_iter[2] = ( guchar ) ( p_iter[1] * alpha_factor + .5 );
			}

#endif
			s_iter += 4;
			p_iter += p_n_channels;
		}

		s_pixels += s_stride;
		p_pixels += p_stride;
	}

	/* Create context and set user data */
	cr = cairo_create ( surface );
	cairo_surface_destroy ( surface );
	cairo_set_user_data ( cr, &pixbuf_key, pixbuf, g_object_unref );

	/* Return context */
	return ( cr );
}

/**

\note	If TRUE, new pixbuf will be created and returned.
		If FALSE, input pixbuf will be updated in place.
\brief	This function will destroy cairo context, created with gnoclPixbufCairoCreate().

	Return value: New or updated GdkPixbuf. You own a new reference on return
	value, so you need to call g_object_unref() on returned pixbuf when you don't
	need it anymore.
**/
GdkPixbuf *gnoclPixbufCairoDestroy ( cairo_t  *cr, gboolean  create_new_pixbuf )
{
	gint width;				/* Width of both pixbuf and surface */
	gint height;    		/* Height of both pixbuf and surface */
	gint p_stride;  		/* Pixbuf stride value */
	gint p_n_channels; 		/* RGB -> 3, RGBA -> 4 */
	gint s_stride;     		/* Surface stride value */
	guchar  *p_pixels;  	/* Pixbuf's pixel data */
	guchar *s_pixels;		/* Surface's pixel data */
	cairo_surface_t *surface;	/* Temporary image surface */
	GdkPixbuf *pixbuf;		/* Pixbuf to be returned */
	GdkPixbuf *tmp_pix;		/* Temporary storage */

	/* Obtain pixbuf to be returned */
	tmp_pix = cairo_get_user_data ( cr, &pixbuf_key );

	if ( create_new_pixbuf )
	{
		pixbuf = gdk_pixbuf_copy ( tmp_pix );
	}

	else
	{
		pixbuf = g_object_ref ( G_OBJECT ( tmp_pix ) );
	}

	/* Obtain surface from where pixel values will be copied */
	surface = cairo_get_target ( cr );

	/* Inspect pixbuf and surface */
	g_object_get ( G_OBJECT ( pixbuf ), "width", &width, "height", &height, "rowstride", &p_stride,
				   "n-channels", &p_n_channels, "pixels", &p_pixels, NULL );
	s_stride = cairo_image_surface_get_stride ( surface );
	s_pixels = cairo_image_surface_get_data ( surface );

	/* Copy pixel data from surface to pixbuf */
	while ( height-- )
	{
		gint    i;
		guchar *p_iter = p_pixels,
				*s_iter = s_pixels;

		for ( i = 0; i < width; i++ )
		{
#if G_BYTE_ORDER == G_LITTLE_ENDIAN
			/* Pixbuf:  RGB(A)
			 * Surface: BGRA */
			gdouble alpha_factor = ( gdouble ) 0xff / s_iter[3];

			p_iter[0] = ( guchar ) ( s_iter[2] * alpha_factor + .5 );
			p_iter[1] = ( guchar ) ( s_iter[1] * alpha_factor + .5 );
			p_iter[2] = ( guchar ) ( s_iter[0] * alpha_factor + .5 );

			if ( p_n_channels == 4 )
				p_iter[3] = s_iter[3];

#elif G_BYTE_ORDER == G_BIG_ENDIAN
			/* Pixbuf:  RGB(A)
			 * Surface: ARGB */
			gdouble alpha_factor = ( gdouble ) 0xff / s_iter[0];

			p_iter[0] = ( guchar ) ( s_iter[1] * alpha_factor + .5 );
			p_iter[1] = ( guchar ) ( s_iter[2] * alpha_factor + .5 );
			p_iter[2] = ( guchar ) ( s_iter[3] * alpha_factor + .5 );

			if ( p_n_channels == 4 )
				p_iter[3] = s_iter[0];

#else /* PDP endianness */
			/* Pixbuf:  RGB(A)
			 * Surface: RABG */
			gdouble alpha_factor = ( gdouble ) 0xff / s_iter[1];

			p_iter[0] = ( guchar ) ( s_iter[0] * alpha_factor + .5 );
			p_iter[1] = ( guchar ) ( s_iter[3] * alpha_factor + .5 );
			p_iter[2] = ( guchar ) ( s_iter[2] * alpha_factor + .5 );

			if ( p_n_channels == 4 )
				p_iter[3] = s_iter[1];

#endif
			s_iter += 4;
			p_iter += p_n_channels;
		}

		s_pixels += s_stride;
		p_pixels += p_stride;
	}

	/* Destroy context */
	cairo_destroy ( cr );

	/* Return pixbuf */
	return ( pixbuf );
}

/**
\brief	Draw an image onto to current surface.
\arguments
**/
static int gnoclCairoDrawImage ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{

#ifdef DEBUG_CAIRO
	listParameters ( objc, objv, __FUNCTION__ );
#endif

	cairo_t *cr;
	cairo_surface_t *image;
	GdkPixbuf *pixbuf;
	int w, h;
	float x, y, r, sw, sh;
	gchar *fname; //[256];
	int i;

	int idx;
	static const char *opts[] =
	{
		"-image", "-translate",
		"-scale", "-rotate",
		NULL
	};
	enum optsIdx
	{
		ImageIdx, TranslateIdx,
		ScaleIdx, RotateIdx
	};

	/* check the number of arguments */
	if ( objc < 5 )
	{
		Tcl_SetObjResult ( interp, Tcl_NewStringObj ( "ERROR: Wrong number of arguments.", -1  ) );
		return TCL_ERROR;
	}


	i = 3;

	/* parse all the options */
	while ( i <  objc )
	{
		getIdx ( opts, Tcl_GetString ( objv[i] ), &idx );

		switch ( idx )
		{
			case ImageIdx:
				{
					fname = Tcl_GetString ( objv[i+1] );
					//sscanf ( Tcl_GetString ( objv[i+1] ), "%s", &fname );
					g_print ( "fname = %s\n", fname );
				}
				break;
			case TranslateIdx:
				{
					sscanf ( Tcl_GetString ( objv[i+1] ), "%f %f", &x, &y );
				}
				break;
			case ScaleIdx:
				{
					sscanf ( Tcl_GetString ( objv[i+1] ), "%f %f", &sw, &sh );
				}
				break;
			case RotateIdx:
				{
					sscanf ( Tcl_GetString ( objv[i+1] ), "%f", &r );
				}
				break;
			default:
				{
					return TCL_ERROR;
				}

		}

		i += 2;
	}

	g_print ( "1\n" );
	/* currently only working on pixbufs */
	g_print ( "fname = %s\n", fname );

	pixbuf = gnoclGetPixBufFromName ( Tcl_GetString ( objv[2] ), interp );
	cr = gnoclPixbufCairoCreate ( pixbuf );

	g_print ( "2\n" );

	image = cairo_image_surface_create_from_png ( fname );

	g_print ( "3\n" );

	w = cairo_image_surface_get_width ( image );
	h = cairo_image_surface_get_height ( image );

	g_print ( "4 sw = %f sh = %f\n", sw, sh );

	cairo_translate ( cr, -w / 2, -h / 2 );
	cairo_scale  ( cr, sw , sh );
	cairo_rotate ( cr, r * G_PI / 180 );

//	w = cairo_image_surface_get_width ( image );
//	h = cairo_image_surface_get_height ( image );
//	cairo_translate ( cr, w/2, h/2 );
	cairo_set_source_surface ( cr, image, x, y );

	g_print ( "5\n" );

	cairo_paint ( cr );

	g_print ( "6\n" );

	cairo_surface_destroy ( image );

	/* update the buffer content */
	pixbuf = gnoclPixbufCairoDestroy ( cr, FALSE );

}


/**
\brief	Draw a line.
\arguments
**/
static int gnoclCairoDrawLine ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{

#ifdef DEBUG_CAIRO
	listParameters ( objc, objv, __FUNCTION__ );
#endif

	cairo_t *cr;
	GdkPixbuf *pixbuf;
	GtkWidget *widget;
	float x1, y1, x2, y2;
	gint i;
	gint tpts;
	gchar *str;
	float *points;

	int idx;
	static const char *opts[] = {"-from", "-to", NULL};
	enum optsIdx { FromIdx, ToIdx };

	/* check the number of arguments */
	if ( objc < 5 )
	{
		Tcl_SetObjResult ( interp, Tcl_NewStringObj ( "ERROR: Wrong number of arguments.", -1  ) );
		return TCL_ERROR;
	}

	i = 3;

	/* parse all the options */
	while ( i <  objc )
	{
		getIdx ( opts, Tcl_GetString ( objv[i] ), &idx );

		switch ( idx )
		{
			case FromIdx:
				{
					sscanf ( Tcl_GetString ( objv[i+1] ), "%f %f", &x1, &y1 );
				}
				break;
			case ToIdx:
				{
					sscanf ( Tcl_GetString ( objv[i+1] ), "%f %f", &x2, &y2 );
				}
				break;
			default:
				{
					return TCL_ERROR;

				}

		}

		i += 2;
	}


	/* currently only working on pixbufs */

	pixbuf = gnoclGetPixBufFromName ( Tcl_GetString ( objv[2] ), interp );

	cr = gnoclPixbufCairoCreate ( pixbuf );

	setLineContext ( cr );


	/* do the drawing */
	cairo_move_to ( cr, x1, y1 );
	cairo_line_to ( cr, x2, y2 );

	cairo_stroke_preserve ( cr );
	cairo_fill ( cr );

	/* update the buffer content */
	pixbuf = gnoclPixbufCairoDestroy ( cr, FALSE );

}

/**
**/
static int drawPolyline ( cairo_t *cr, gchar *points )
{

#ifdef DEBUG_CAIRO
	g_print ( "%s >%s<\n", __FUNCTION__, points );
#endif

	int i = 0;
	int j = 1;
	float x, y;
	char * pch;

	pch = strtok ( points, " " );

	while ( pch != NULL )
	{


		if ( i )
		{
			sscanf ( pch, "%f", &y );


			cairo_line_to ( cr, x, y );

			i = 0;

		}

		else
		{
			sscanf ( pch, "%f", &x );
			i = 1;

		}

		pch = strtok ( NULL, " " );
	}

}

static int gnoclCairoDrawRoundedRectangle ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{

#ifdef DEBUG_CAIRO
	listParameters ( objc, objv, __FUNCTION__ );
#endif

	/* a custom shape that could be wrapped in a function */
	double x         = 10,        /* parameters like cairo_rectangle */
		   y         = 10,
		   width         = 50,
		   height        = 50,
		   aspect        = 1.0,     /* aspect ratio */
		   corner_radius = height / 10.0;   /* and corner curvature radius */

	double radius = corner_radius / aspect;
	double degrees = G_PI / 180.0;
	gint fill;


	int i, idx;
	static const char *opts[] =
	{
		"-center", "-width", "-height",
		"-aspect", "-radius", "-fill",
		NULL
	};
	enum optsIdx
	{
		CentreIdx, WidthIdx, HeightIdx,
		AspectIdx, RadiusIdx, FillIdx
	};


	i = 3;

	/* parse all the options */
	while ( i <  objc )
	{
		getIdx ( opts, Tcl_GetString ( objv[i] ), &idx );

		switch ( idx )
		{
			case CentreIdx:
				{
					sscanf ( Tcl_GetString ( objv[i+1] ), "%f %f", &x, &y );
				}
				break;
			case WidthIdx:
				{
					sscanf ( Tcl_GetString ( objv[i+1] ), "%f", &width );
				}
				break;
			case HeightIdx:
				{
					sscanf ( Tcl_GetString ( objv[i+1] ), "%f", &height );
				}
				break;
			case AspectIdx:
				{
					sscanf ( Tcl_GetString ( objv[i+1] ), "%f", &height );
				}
				break;
			case RadiusIdx:
				{
					sscanf ( Tcl_GetString ( objv[i+1] ), "%f", &radius );
				}
				break;
			case FillIdx:
				{
					sscanf ( Tcl_GetString ( objv[i+1] ), "%d", &fill );
				}
				break;
			default:
				{}
		}

		i += 2;
	}

	corner_radius = height / 10.0;
	radius = corner_radius / aspect;




	cairo_t *cr;
	GdkPixbuf *pixbuf;

	pixbuf = gnoclGetPixBufFromName ( Tcl_GetString ( objv[2] ), interp );
	cr = gnoclPixbufCairoCreate ( pixbuf );
	setLineContext ( cr );



	cairo_new_sub_path ( cr );
	cairo_arc ( cr, x + width - radius, y + radius, radius, -90 * degrees, 0 * degrees );
	cairo_arc ( cr, x + width - radius, y + height - radius, radius, 0 * degrees, 90 * degrees );
	cairo_arc ( cr, x + radius, y + height - radius, radius, 90 * degrees, 180 * degrees );
	cairo_arc ( cr, x + radius, y + radius, radius, 180 * degrees, 270 * degrees );
	cairo_close_path ( cr );



	setFillContext ( cr );


	cairo_fill_preserve ( cr );

	setLineContext ( cr );

	cairo_set_line_width ( cr, 10.0 );
	cairo_stroke ( cr );


	/* update the buffer content */
	pixbuf = gnoclPixbufCairoDestroy ( cr, FALSE );
}


/**
\brief	Draw a polyline.

void cairo_move_to (cairo_t *cr, double x, double y);
void cairo_line_to (cairo_t *cr, double x, double y);

\arguments
**/
static int gnoclCairoDrawPolyLine ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{

#ifdef DEBUG_CAIRO
	listParameters ( objc, objv, __FUNCTION__ );
#endif

	cairo_t *cr;
	GdkPixbuf *pixbuf;
	GtkWidget *widget;
	float x, y;
	gint close, fill, i;
	gchar *points;
	float xf, yf, xl, yl;

	int idx;
	static const char *opts[] =
	{
		"-points", "-close", "-fill",
		NULL
	};

	enum optsIdx
	{
		PointsIdx, CloseIdx, FillIdx
	};

	/* set some defaults */
	close = 0; fill = 0;


	pixbuf = gnoclGetPixBufFromName ( Tcl_GetString ( objv[2] ), interp );
	cr = gnoclPixbufCairoCreate ( pixbuf );
	setLineContext ( cr );


	i = 3;

	/* parse all the options */
	while ( i <  objc )
	{
		getIdx ( opts, Tcl_GetString ( objv[i] ), &idx );

		switch ( idx )
		{
			case PointsIdx:
				{

					int t = strlen ( Tcl_GetString ( objv[i+1] ) );

					gchar str[t+5];

					strcpy ( str, Tcl_GetString ( objv[i+1] ) );

					points = str;

					drawPolyline ( cr, Tcl_GetString ( objv[i+1] ) );

					sscanf ( Tcl_GetString ( objv[i+1] ), "%f %f", &xf, &yf );


				}
				break;
			case CloseIdx:
				{
					sscanf ( Tcl_GetString ( objv[i+1] ), "%d", &close );
				}
				break;
			case FillIdx:
				{
					sscanf ( Tcl_GetString ( objv[i+1] ), "%d", &fill );
				}
				break;
			default:
				{
					return TCL_ERROR;

				}

		}

		i += 2;
	}

	if ( close )
	{
		cairo_line_to ( cr, xf, yf );
	}

	if ( fill )
	{
		cairo_stroke_preserve ( cr );
		setFillContext ( cr );
		cairo_fill ( cr );
	}

	else
	{
		cairo_stroke ( cr );
	}

	/* update the buffer content */
	pixbuf = gnoclPixbufCairoDestroy ( cr, FALSE );

}


/**
\brief	Draw text.
\arguments
**/
static int gnoclCairoDrawText ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{

#ifdef DEBUG_CAIRO
	listParameters ( objc, objv, __FUNCTION__ );
#endif

	cairo_t *cr;
	GdkPixbuf *pixbuf;
	GtkWidget *widget;
	char *str;
	float x, y;


	if ( strcmp ( Tcl_GetString ( objv[3] ), "-position" ) == 0 && strcmp ( Tcl_GetString ( objv[5] ), "-text" ) == 0  )
	{
		sscanf ( Tcl_GetString ( objv[4] ), "%f %f", &x, &y );
		str = Tcl_GetString ( objv[6] );
	}

	else if ( strcmp ( Tcl_GetString ( objv[5] ), "-text" ) == 0 && strcmp ( Tcl_GetString ( objv[3] ), "-position" ) == 0  )
	{
		sscanf ( Tcl_GetString ( objv[6] ), "%f %f", &x, &y );
		str = Tcl_GetString ( objv[4] );
	}

	else
	{
		return TCL_ERROR;
	}

	;

	pixbuf = gnoclGetPixBufFromName ( Tcl_GetString ( objv[2] ), interp );

	cr = gnoclPixbufCairoCreate ( pixbuf );

	cairo_set_line_width ( cr, 9 );

	setLineContext ( cr );

	/* do the drawing */

	setTextContext ( cr );

	//cairo_select_font_face ( cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL );

	//cairo_set_font_size ( cr, 15.0 );

	cairo_move_to ( cr, x, y );

	cairo_show_text ( cr, str );

	cairo_stroke_preserve ( cr );

	cairo_set_source_rgb ( cr, 0.3, 0.4, 0.6 );

	cairo_fill ( cr );

	/* update the buffer content */
	pixbuf = gnoclPixbufCairoDestroy ( cr, FALSE );

}

/**
\brief	Draw a curve.
\arguments
**/
static int gnoclCairoDrawCurve ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{
#ifdef DEBUG_CAIRO
	listParameters ( objc, objv, __FUNCTION__ );
#endif

	cairo_t *cr;
	GdkPixbuf *pixbuf;

	float x, y;
	float x1, y1, x2, y2, x3, y3;

	gint i, fill;

	int idx;
	static const char *opts[] = {"-points", "-fill", NULL};
	enum optsIdx { PointsIdx, FillIdx };

	/* check the number of arguments */
	if ( objc < 5 )
	{
		Tcl_SetObjResult ( interp, Tcl_NewStringObj ( "ERROR: Wrong number of arguments.", -1  ) );
		return TCL_ERROR;
	}

	i = 3;

	/* parse all the options */
	while ( i <  objc )
	{

		getIdx ( opts, Tcl_GetString ( objv[i] ), &idx );

		switch ( idx )
		{
			case PointsIdx:
				{

					sscanf ( Tcl_GetString ( objv[i+1] ), "%f %f %f %f %f %f %f %f ",
							 &x, &y, &x1, &y1, &x2, &y2, &x3, &y3  );
				}
				break;
			case FillIdx:
				{
					sscanf ( Tcl_GetString ( objv[i+1] ), "%d", &fill );
				}
				break;
			default:
				{

					gchar *str;
					sprintf ( "ERROR: Invalid option %s. Must be one of %s\n", Tcl_GetString ( objv[i] ), opts );
					Tcl_SetObjResult ( interp, Tcl_NewStringObj ( str, -1  ) );
					return TCL_ERROR;
				}
				break;
		}

		i += 2;
	}

	pixbuf = gnoclGetPixBufFromName ( Tcl_GetString ( objv[2] ), interp );

	cr = gnoclPixbufCairoCreate ( pixbuf );

	cairo_set_line_width ( cr, 9 );

	cairo_set_source_rgba ( cr, CairoParams.line.r, CairoParams.line.g, CairoParams.line.b, CairoParams.line.a );



	/* do the drawing */

	cairo_move_to ( cr, x, y );
	cairo_curve_to ( cr, x1, y1, x2, y2, x3, y3 );

	if ( fill )
	{

		cairo_stroke_preserve ( cr );

		cairo_set_source_rgba ( cr, CairoParams.fill.r, CairoParams.fill.g, CairoParams.fill.b, CairoParams.fill.a );


		cairo_fill ( cr );
	}

	/* update the buffer content */
	pixbuf = gnoclPixbufCairoDestroy ( cr, FALSE );

}

/**
\brief	Draw a rectangle.
\arguments
**/
static int gnoclCairoDrawRectangle ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{
#ifdef DEBUG_CAIRO
	listParameters ( objc, objv, __FUNCTION__ );
#endif

	cairo_t *cr;
	GdkPixbuf *pixbuf;

	float xc;
	float yc;
	float width;
	float height;
	gint i, fill;

	int idx;
	static const char *opts[] =
	{
		"-center", "-width",
		"-height", "-fill",
		NULL
	};
	enum optsIdx
	{
		CenterIdx, WidthIdx,
		HeightIdx, FillIdx
	};

	/* check the number of arguments */
	if ( objc < 9 )
	{
		Tcl_SetObjResult ( interp, Tcl_NewStringObj ( "ERROR: Wrong number of arguments.", -1  ) );
		return TCL_ERROR;
	}


	i = 3;

	/* parse all the options */
	while ( i <  objc )
	{

		getIdx ( opts, Tcl_GetString ( objv[i] ), &idx );

		switch ( idx )
		{
			case CenterIdx:
				{
					g_print ( "Center\n" );
					sscanf ( Tcl_GetString ( objv[i+1] ), "%f %f", &xc, &yc );
				}
				break;
			case WidthIdx:
				{
					sscanf ( Tcl_GetString ( objv[i+1] ), "%f", &width );
				}
				break;
			case HeightIdx:
				{
					sscanf ( Tcl_GetString ( objv[i+1] ), "%f", &height );
				}
				break;
			case FillIdx:
				{
					sscanf ( Tcl_GetString ( objv[i+1] ), "%d", &fill );
				}
				break;
			default:
				{

					gchar *str;
					sprintf ( "ERROR: Invalid option %s. Must be one of %s\n", Tcl_GetString ( objv[i] ), opts );
					Tcl_SetObjResult ( interp, Tcl_NewStringObj ( str, -1  ) );
					return TCL_ERROR;
				}
				break;
		}

		i += 2;
	}

	pixbuf = gnoclGetPixBufFromName ( Tcl_GetString ( objv[2] ), interp );

	cr = gnoclPixbufCairoCreate ( pixbuf );

	setLineContext ( cr );



	/* do the drawing */

	cairo_rectangle ( cr, xc - width / 2, yc - height / 2, width, height );

	if ( fill )
	{

		cairo_stroke_preserve ( cr );

		setFillContext ( cr );

		cairo_fill ( cr );
	}

	/* update the buffer content */
	pixbuf = gnoclPixbufCairoDestroy ( cr, FALSE );

}

/**
\brief
**/
static void linearGradient ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{
#ifdef DEBUG_CAIRO
	g_print ( "linearGradient 1\n" );
	g_printf ( "objc = %d\n", objc );
	listParameters ( objc, objv, __FUNCTION__ );
#endif


	cairo_t *cr;
	GdkPixbuf *pixbuf;

	float x1, y1, x2, y2;
	float offset1, offset2;
	float r1, g1, b1, a1;
	float r2, g2, b2, a2;
	gint i;

	int idx;


	static const char *opts[] =
	{
		"-type",
		"-from", "-to",
		"-startColor", "-endColor",
		"-startOffset", "-endOffset",
		NULL
	};

	enum optsIdx
	{
		TypeIdx,
		FromIdx, ToIdx,
		StartColorIdx, EndColorIdx,
		StartOffsetIdx, EndOffsetIdx
	};

	g_print ( "linearGradient 2\n" );

	/* check the number of arguments */
	if ( objc < 9 )
	{
		Tcl_SetObjResult ( interp, Tcl_NewStringObj ( "ERROR: Wrong number of arguments.", -1  ) );
		return TCL_ERROR;
	}


	i = 3;

	/* parse all the options */
	while ( i <  objc )
	{
		g_print ( "linearGradient 3 - %d\n", i );

		getIdx ( opts, Tcl_GetString ( objv[i] ), &idx );

		g_print ( "linearGradient 4 - \n" );

		switch ( idx )
		{
			case FromIdx:
				{
					g_print ( "from\n" );
					sscanf ( Tcl_GetString ( objv[i+1] ), "%f %f", &x1, &y1 );
				}
				break;
			case ToIdx:
				{
					g_print ( "to\n" );
					sscanf ( Tcl_GetString ( objv[i+1] ), "%f %f", &x2, &y2 );
				}
				break;
			case StartColorIdx:
				{
					g_print ( "start\n" );
					sscanf ( Tcl_GetString ( objv[i+1] ), "%f %f %f %f", &r1, &g1, &b1, &a1 );
				}
				break;
			case EndColorIdx:
				{
					g_print ( "end\n" );
					sscanf ( Tcl_GetString ( objv[i+1] ),  "%f %f %f %f", &r2, &g2, &b2, &a2 );
				}
				break;
			case StartOffsetIdx:
				{
					g_print ( "start\n" );
					sscanf ( Tcl_GetString ( objv[i+1] ), "%f", &offset1 );
				}
				break;
			case EndOffsetIdx:
				{
					g_print ( "end\n" );
					sscanf ( Tcl_GetString ( objv[i+1] ),  "%f", &offset2 );
				}
				break;
			default:
				{
					/* ignore any other options and values */
				}
				break;
		}

		i += 2;
	}

	g_print ( "from  %f %f to %f %f\n", x1, y1, x2, y2 );
	g_print ( "start %f %f %f %f\n", r1, g1, b1, a1 );
	g_print ( "end   %f %f %f %f\n", r2, g2, b2, a2 );
	g_print ( "os1   %f\n", offset1 );
	g_print ( "os2   %f\n", offset2 );

	pixbuf = gnoclGetPixBufFromName ( Tcl_GetString ( objv[2] ), interp );

	cr = gnoclPixbufCairoCreate ( pixbuf );

	//cairo_set_line_width ( cr, 9 );
	//cairo_set_source_rgb ( cr, 0.69, 0.19, 0 );

	/* do the drawing */
	//cairo_pattern_t *pat;

	CairoParams.pat = cairo_pattern_create_linear ( 0 , 0, 0, y2 );
	cairo_pattern_add_color_stop_rgba ( CairoParams.pat, offset1, r1, g1, b1, a1 );
	cairo_pattern_add_color_stop_rgba ( CairoParams.pat, offset2, r2, g2, b2, a2 );
	cairo_rectangle ( cr, x1, y1, y2, y2 );

	cairo_set_source ( cr, CairoParams.pat );
	cairo_fill ( cr );
	cairo_pattern_destroy ( CairoParams.pat );

	/* update the buffer content */
	pixbuf = gnoclPixbufCairoDestroy ( cr, FALSE );

}



static void radialGradient ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{

#ifdef DEBUG_CAIRO
	listParameters ( objc, objv, __FUNCTION__ );
#endif


	cairo_t *cr;
	GdkPixbuf *pixbuf;

	float cx0, cy0, radius0;
	float cx1, cy1, radius1;

	float r1, g1, b1, a1;
	float r2, g2, b2, a2;
	//cairo_pattern_t *pat;

	gint i;

	int idx;


	static const char *opts[] =
	{
		"-center1", "-center2",
		"-startColor", "-endColor",
		"-radius1", "-radius2",
		NULL
	};

	enum optsIdx
	{
		Center1Idx, Centre2Idx,
		StartColorIdx, EndColorIdx,
		Radius1Idx, Radius2Idx
	};

	g_print ( "linearGradient 2\n" );

	/* check the number of arguments */
	if ( objc < 9 )
	{
		Tcl_SetObjResult ( interp, Tcl_NewStringObj ( "ERROR: Wrong number of arguments.", -1  ) );
		return TCL_ERROR;
	}

	i = 3;

	/* parse all the options */
	while ( i <  objc )
	{
		g_print ( "radialGradient 3 - %d\n", i );

		getIdx ( opts, Tcl_GetString ( objv[i] ), &idx );

		g_print ( "radialGradient 4 - \n" );

		switch ( idx )
		{
			case Radius1Idx:
				{
					g_print ( "radius1\n" );
					sscanf ( Tcl_GetString ( objv[i+1] ), "%f", &radius0 );
				}
				break;
			case Radius2Idx:
				{
					g_print ( "radius2\n" );
					sscanf ( Tcl_GetString ( objv[i+1] ), "%f", &radius1 );
				}
				break;
			case Center1Idx:
				{
					g_print ( "center1\n" );
					sscanf ( Tcl_GetString ( objv[i+1] ), "%f %f", &cx0, &cy0 );
				}
				break;
			case Centre2Idx:
				{
					g_print ( "center2\n" );
					sscanf ( Tcl_GetString ( objv[i+1] ), "%f %f", &cx1, &cy1 );
				}
				break;
			case StartColorIdx:
				{
					g_print ( "start\n" );
					sscanf ( Tcl_GetString ( objv[i+1] ), "%f %f %f %f", &r1, &g1, &b1, &a1 );
				}
				break;
			case EndColorIdx:
				{
					g_print ( "end\n" );
					sscanf ( Tcl_GetString ( objv[i+1] ),  "%f %f %f %f", &r2, &g2, &b2, &a2 );
				}
				break;
			default:
				{
					/* ignore any other options and values */
				}
				break;
		}

		i += 2;
	}

	g_print ( "from  %f %f to %f %f\n", cx0, cy0, cx1, cy1 );
	g_print ( "start %f %f %f %f\n", r1, g1, b1, a1 );
	g_print ( "end   %f %f %f %f\n", r2, g2, b2, a2 );

	pixbuf = gnoclGetPixBufFromName ( Tcl_GetString ( objv[2] ), interp );

	cr = gnoclPixbufCairoCreate ( pixbuf );

	//cairo_set_line_width ( cr, 9 );

	//cairo_set_source_rgb ( cr, 0.69, 0.19, 0 );


	/* do the drawing */

	//pat = cairo_pattern_create_radial ( 115.2, 102.4, 25.6, 102.4,  102.4, 128.0 );

	CairoParams.pat = cairo_pattern_create_radial ( cx0, cy0, radius0, cx1, cy1, radius1 );

	cairo_pattern_add_color_stop_rgba ( CairoParams.pat, 1, r1, g1, b1, a1 );
	cairo_pattern_add_color_stop_rgba ( CairoParams.pat, 0, r2, g2, b2, a2 );
	cairo_set_source ( cr, CairoParams.pat );
//cairo_arc (cr, 128.0, 128.0, 76.8, 0, 2 * G_PI);
	cairo_fill ( cr );
	cairo_pattern_destroy ( CairoParams.pat );

//
	/*
	pat = cairo_pattern_create_radial (115.2, 102.4, 25.6, 102.4, 102.4,128.0);
	cairo_pattern_add_color_stop_rgba (pat, 0, 1, 1, 1, 1);
	cairo_pattern_add_color_stop_rgba (pat, 1, 0, 0, 0, 1);
	cairo_set_source (cr, pat);
	cairo_arc (cr, 128.0, 128.0, 76.8, 0, 2 * G_PI);
	cairo_fill (cr);
	cairo_pattern_destroy (pat);
	*/

	/* update the buffer content */
	pixbuf = gnoclPixbufCairoDestroy ( cr, FALSE );
}




/**
\brief	Draw a gradient.
\arguments
**/
static int gnoclCairoDrawSet ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{
#ifdef DEBUG_CAIRO
	listParameters ( objc, objv, __FUNCTION__ );
#endif


	int i;
	int idx;
	float r, g, b, a;
	float lineWidth;

	static const char *types[] =
	{
		"-lineColor", "-fillColor", "-lineWidth",
		"-dash", "-font",
		NULL
	};

	enum typesIdx
	{
		LineColorIdx, FillColorIdx, LineWidthIdx,
		DashIdx, FontIdx
	};

	i = 2;


	/* parse all the options */
	while ( i <  objc )
	{
		//g_print ("\t\tgnoclCairoDrawSet while %i\n",i);
		getIdx ( types, Tcl_GetString ( objv[i] ), &idx );

		switch ( idx )
		{
			case FontIdx:
				{
					gchar face[24];
					gint size;
					gchar slant[8];
					gchar weight[8];




					sscanf ( Tcl_GetString ( objv[i+1] ), "%s %d %s %s", &face, &size, &slant, &weight );

					strcpy ( CairoParams.font.face, face );
					CairoParams.font.size = size;
					strcpy ( CairoParams.font.slant, slant );
					strcpy ( CairoParams.font.weight, weight );


				}
				break;
			case DashIdx:
				{

					CairoParams.dash = Tcl_GetString ( objv[i+1] );
					dash = Tcl_GetString ( objv[i+1] );
				}
				break;
			case LineColorIdx:
				{

					sscanf ( Tcl_GetString ( objv[i+1] ), "%f %f %f %f", &r, &g, &b, &a );

					CairoParams.line.r = r;
					CairoParams.line.g = g;
					CairoParams.line.b = b;
					CairoParams.line.a = a;
				}
				break;
			case FillColorIdx:
				{

					sscanf ( Tcl_GetString ( objv[i+1] ), "%f %f %f %f", &r, &g, &b, &a );

					CairoParams.fill.r = r;
					CairoParams.fill.g = g;
					CairoParams.fill.b = b;
					CairoParams.fill.a = a;

				}
				break;
			case LineWidthIdx:
				{


					sscanf ( Tcl_GetString ( objv[i+1] ), "%f", &lineWidth );
					CairoParams.lineWidth = lineWidth;
				}
				break;
			default: { };
		}

		/* increment the counter */
		i += 2;
	}

	return TCL_OK;
}



/**
\brief	Draw a gradient.
\arguments
**/
static int gnoclCairoDrawPattern ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{
#ifdef DEBUG_CAIRO
	listParameters ( objc, objv, __FUNCTION__ );
#endif


	int i;
	int idx;
	cairo_t *cr;
	GdkPixbuf *pixbuf;

	static const char *types[] =
	{
		"-type",
		NULL
	};

	enum typesIdx
	{
		TypeIdx
	};


	pixbuf = gnoclGetPixBufFromName ( Tcl_GetString ( objv[2] ), interp );

	cr = gnoclPixbufCairoCreate ( pixbuf );

	i = 3;

	/* parse all the options */
	while ( i <  objc )
	{

		g_print ( "item = %s\n", Tcl_GetString ( objv[i+1] ) );

		getIdx ( types, Tcl_GetString ( objv[i] ), &idx );

		switch ( idx )
		{
			case TypeIdx:
				{
					if ( strcmp ( Tcl_GetString ( objv[i+1] ), "linear" ) == 0 )
					{
						linearGradient ( data, interp, objc, objv );

					}

					else if ( strcmp ( Tcl_GetString ( objv[i+1] ), "radial" ) == 0 )
					{
						radialGradient ( data, interp, objc, objv );
					}
				}
				break;
			default: { };
		}

		i += 2;
	}

	return TCL_ERROR;
}


/**
\brief	Draw a square.
\arguments
**/
static int gnoclCairoDrawSquare ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{
#ifdef DEBUG_CAIRO
	g_print ( "gnoclCairoDrawSquare\n" );
	g_printf ( "objc = %d\n", objc );
	listParameters ( objc, objv, __FUNCTION__ );
#endif

	cairo_t *cr;
	GdkPixbuf *pixbuf;

	float xc;
	float yc;
	float length;
	gint i, fill;

	int idx;
	static const char *opts[] = {"-center", "-length", "-fill", NULL};
	enum optsIdx { CenterIdx, LengthIdx, FillIdx };

	/* check the number of arguments */
	if ( objc < 7 )
	{
		Tcl_SetObjResult ( interp, Tcl_NewStringObj ( "ERROR: Wrong number of arguments.", -1  ) );
		return TCL_ERROR;
	}


	i = 3;

	/* parse all the options */
	while ( i <  objc )
	{

		getIdx ( opts, Tcl_GetString ( objv[i] ), &idx );

		switch ( idx )
		{
			case CenterIdx:
				{
					g_print ( "Center\n" );
					sscanf ( Tcl_GetString ( objv[i+1] ), "%f %f", &xc, &yc );
				}
				break;
			case LengthIdx:
				{
					sscanf ( Tcl_GetString ( objv[i+1] ), "%f", &length );
				}
				break;
			case FillIdx:
				{
					sscanf ( Tcl_GetString ( objv[i+1] ), "%d", &fill );
				}
				break;
			default:
				{

					gchar *str;
					sprintf ( "ERROR: Invalid option %s. Must be one of %s\n", Tcl_GetString ( objv[i] ), opts );
					Tcl_SetObjResult ( interp, Tcl_NewStringObj ( str, -1  ) );
					return TCL_ERROR;
				}
				break;
		}

		i += 2;
	}

	pixbuf = gnoclGetPixBufFromName ( Tcl_GetString ( objv[2] ), interp );

	cr = gnoclPixbufCairoCreate ( pixbuf );

	setLineContext ( cr );



	/* do the drawing */

	cairo_rectangle ( cr, xc - length / 2, yc - length / 2, length, length );

	if ( fill )
	{

		cairo_stroke_preserve ( cr );

		setFillContext ( cr );


		cairo_fill ( cr );
	}

	/* update the buffer content */
	pixbuf = gnoclPixbufCairoDestroy ( cr, FALSE );

}

/**
\brief	Draw a circle.
\arguments
**/
static int gnoclCairoDrawArc ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{
#ifdef DEBUG_CAIRO
	listParameters ( objc, objv, "gnoclCairoDrawArc" );
#endif

	cairo_t *cr;
	GdkPixbuf *pixbuf;

	float xc;
	float yc;
	float radius;
	float angle1;
	float angle2;
	gint i, fill, negative;



	int idx;
	static const char *opts[] = {"-center", "-radius", "-startAngle", "-endAngle", "-fill", "-negative", NULL};
	enum optsIdx { CenterIdx, RadiusIdx, StartAngleIdx, EndAngleIdx, FillIdx, NegativeIdx };

	/* check the number of arguments */
	if ( objc < 11 )
	{
		Tcl_SetObjResult ( interp, Tcl_NewStringObj ( "ERROR: Wrong number of arguments.", -1  ) );
		return TCL_ERROR;
	}


	i = 3;

	/* parse and set all options */
	while ( i <  objc )
	{


		getIdx ( opts, Tcl_GetString ( objv[i] ), &idx );



		switch ( idx )
		{
			case CenterIdx:
				{
					g_print ( "Center\n" );
					sscanf ( Tcl_GetString ( objv[i+1] ), "%f %f", &xc, &yc );
				}
				break;
			case RadiusIdx:
				{
					sscanf ( Tcl_GetString ( objv[i+1] ), "%f", &radius );
				}
				break;
			case StartAngleIdx:
				{
					sscanf ( Tcl_GetString ( objv[i+1] ), "%f", &angle1 );

				}
				break;
			case EndAngleIdx:
				{
					sscanf ( Tcl_GetString ( objv[i+1] ), "%f", &angle2 );
				}
				break;
			case FillIdx:
				{
					sscanf ( Tcl_GetString ( objv[i+1] ), "%d", &fill );
				}
				break;
			case NegativeIdx:
				{
					sscanf ( Tcl_GetString ( objv[i+1] ), "%d", &negative );
				}
				break;
			default:
				{

					gchar *str;
					sprintf ( "ERROR: Invalid option %s. Must be one of %s\n", Tcl_GetString ( objv[i] ), opts );
					Tcl_SetObjResult ( interp, Tcl_NewStringObj ( str, -1  ) );
					return TCL_ERROR;
				}
				break;
		}

		i += 2;
	}



	angle1 = angle1 * ( G_PI / 180.0 );
	angle2 = angle2 * ( G_PI / 180.0 );


	pixbuf = gnoclGetPixBufFromName ( Tcl_GetString ( objv[2] ), interp );

	cr = gnoclPixbufCairoCreate ( pixbuf );


	setLineContext ( cr );


	/* do the drawing */

	if ( negative )
	{
		cairo_arc_negative ( cr, xc, yc, radius, angle1, angle2 );
	}

	else
	{
		cairo_arc ( cr, xc, yc, radius, angle1, angle2 );
	}


	if ( fill )
	{

		cairo_stroke_preserve ( cr );

		setFillContext ( cr );

		cairo_fill ( cr );
	}


	/* update the buffer content */
	pixbuf = gnoclPixbufCairoDestroy ( cr, FALSE );



}

/**
\brief	Draw a circle.
\arguments
**/
static int gnoclCairoDrawCircle ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{
#ifdef DEBUG_CAIRO
	listParameters ( objc, objv, __FUNCTION__ );
#endif

	cairo_t *cr;
	GdkPixbuf *pixbuf;

	float xc;
	float yc;
	float radius;
	float angle1 = 0;
	float angle2 = 2 * G_PI;


	if ( strcmp ( Tcl_GetString ( objv[3] ), "-center" ) == 0 && strcmp ( Tcl_GetString ( objv[5] ), "-radius" ) == 0  )
	{
		sscanf ( Tcl_GetString ( objv[4] ), "%f %f", &xc, &yc );
		sscanf ( Tcl_GetString ( objv[6] ), "%f", &radius );
	}

	else if ( strcmp ( Tcl_GetString ( objv[5] ), "-center" ) == 0 && strcmp ( Tcl_GetString ( objv[3] ), "-radius" ) == 0  )
	{
		sscanf ( Tcl_GetString ( objv[6] ), "%f %f", &xc, &yc );
		sscanf ( Tcl_GetString ( objv[4] ), "%f", &radius );
	}

	else
	{
		return TCL_ERROR;
	}


	pixbuf = gnoclGetPixBufFromName ( Tcl_GetString ( objv[2] ), interp );

	cr = gnoclPixbufCairoCreate ( pixbuf );


	setDash ( cr );

	setLineContext ( cr );

	/* do the drawing */
	cairo_arc ( cr, xc, yc, radius, angle1, angle2 );

	cairo_fill ( cr );

	//cairo_stroke_preserve ( cr );

	cairo_arc ( cr, xc, yc, radius - CairoParams.lineWidth / 2, angle1, angle2 );


	setFillContext ( cr );

	cairo_fill ( cr );

	/* update the buffer content */
	pixbuf = gnoclPixbufCairoDestroy ( cr, FALSE );

}

/**
\brief
**/
int _gnoclCairoCmd ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{
	/* get the various parameters */
	//	listParameters ( objc, objv, __FUNCTION__ );

	int Idx;
	const char *cmds[] =
	{
		"line", "curve", "polyline", "arc", "circle", "rectangle",
		"square", "ellipse", "polygon", "text",
		"translate", "rotate", "scale", "shear",
		"composite", "clip", "mask", "pattern",
		"set", "roundedRectangle",
		NULL
	};

	enum optsIdx
	{
		LineIdx, CurveIdx, PolyLineIdx, ArcIdx, CircleIdx,
		RectangleIdx, SquareIdx, EllipseIdx, PolygonIdx, TextIdx,
		TranslateIdx, RotateIdx, ScaleIdx, ShearIdx,
		CompositeIdx, ClipIdx, MaskIdx, PatternIdx,
		SetIdx, RoundedRectangleIdx
	};


	/* set some default values */

	/*
		CairoParams.line.r = 0;
		CairoParams.line.g = 1;
		CairoParams.line.b = 0;
		CairoParams.line.a = 1;

		CairoParams.fill.r = 1;
		CairoParams.fill.g = 0;
		CairoParams.fill.b = 0;
		CairoParams.fill.a = 1;

		CairoParams.lineWidth = 5.0;
		CairoParams.dash = "4.0, 1.0";
	*/

	/* determine what to do with values */
	getIdx ( cmds, Tcl_GetString ( objv[1] ), &Idx );

	switch ( Idx )
	{
		case RoundedRectangleIdx:
			{
#ifdef DEBUG_CAIRO
				g_print ( "RoundedRectangleIdx\n" );
#endif
				gnoclCairoDrawRoundedRectangle ( NULL, interp, objc, objv );
			}
			break;
		case PatternIdx:
			{
#ifdef DEBUG_CAIRO
				g_print ( "PatternIdx\n" );
#endif
				gnoclCairoDrawPattern ( NULL, interp, objc, objv );
			}
			break;
		case LineIdx:
			{
#ifdef DEBUG_CAIRO
				g_print ( "LineIdx\n" );
#endif
				gnoclCairoDrawLine ( NULL, interp, objc, objv );
			}
			break;
		case CurveIdx:
			{
#ifdef DEBUG_CAIRO
				g_print ( "CurveIdx\n" );
#endif
				gnoclCairoDrawCurve ( NULL, interp, objc, objv );
			}
			break;
		case PolyLineIdx:
			{
#ifdef DEBUG_CAIRO
				g_print ( "PolyLineIdx\n" );
#endif
				gnoclCairoDrawPolyLine ( NULL, interp, objc, objv );
			}
			break;

		case ArcIdx:
			{
#ifdef DEBUG_CAIRO
				g_print ( "ArcIdx\n" );
#endif
				gnoclCairoDrawArc ( NULL, interp, objc, objv );
			}
			break;
		case CircleIdx:
			{
#ifdef DEBUG_CAIRO
				g_print ( "CircleIdx\n" );
#endif
				gnoclCairoDrawCircle ( NULL, interp, objc, objv );
			}
			break;
		case RectangleIdx:
			{
#ifdef DEBUG_CAIRO
				g_print ( "RectangleIdx\n" );
#endif
				gnoclCairoDrawRectangle ( NULL, interp, objc, objv );
			}
			break;
		case SquareIdx:
			{
#ifdef DEBUG_CAIRO
				g_print ( "SquareIdx\n" );
#endif
				gnoclCairoDrawSquare ( NULL, interp, objc, objv );
			}
			break;
		case EllipseIdx:
			{
#ifdef DEBUG_CAIRO
				g_print ( "EllipseIdx\n" );
#endif
				gnoclCairoDrawLine ( NULL, interp, objc, objv );
			}
			break;
		case PolygonIdx:
			{
#ifdef DEBUG_CAIRO
				g_print ( "PolygonIdx\n" );
#endif
				gnoclCairoDrawLine ( NULL, interp, objc, objv );
			}
			break;
		case TextIdx:
			{
#ifdef DEBUG_CAIRO
				g_print ( "TextIdx\n" );
#endif
				gnoclCairoDrawText ( NULL, interp, objc, objv );
			}
			break;
		case TranslateIdx:
			{
#ifdef DEBUG_CAIRO
				g_print ( "TranslateIdx\n" );
#endif
				gnoclCairoDrawLine ( NULL, interp, objc, objv );
			}
			break;
		case RotateIdx:
			{
#ifdef DEBUG_CAIRO
				g_print ( "RotateIdx\n" );
#endif
				gnoclCairoDrawLine ( NULL, interp, objc, objv );
			}
			break;
		case ScaleIdx:
			{
#ifdef DEBUG_CAIRO
				g_print ( "ScaleIdxn" );
#endif
				gnoclCairoDrawLine ( NULL, interp, objc, objv );
			}
			break;
		case ShearIdx:
			{
#ifdef DEBUG_CAIRO

				g_print ( "ShearIdx\n" );
#endif
				gnoclCairoDrawLine ( NULL, interp, objc, objv );
			}
			break;
		case CompositeIdx:
			{
#ifdef DEBUG_CAIRO
				g_print ( "CompositeIdx\n" );
#endif
				gnoclCairoDrawImage ( NULL, interp, objc, objv );
			}
			break;
		case ClipIdx:
			{
#ifdef DEBUG_CAIRO
				g_print ( "ClipIdx\n" );
#endif
				gnoclCairoDrawLine ( NULL, interp, objc, objv );
			}
			break;
		case MaskIdx:
			{
#ifdef DEBUG_CAIRO

				g_print ( "MaskIdx\n" );
#endif
				gnoclCairoDrawLine ( NULL, interp, objc, objv );
			}
			break;
		case SetIdx:
			{
#ifdef DEBUG_CAIRO

				g_print ( "Set\n" );
#endif
				gnoclCairoDrawSet ( NULL, interp, objc, objv );
			}
			break;
		default:
			{
				g_print ( "NOT YET IMPLEMENTED\n", Tcl_GetString ( objv[1] ) );
			}
	}


	return TCL_OK;
}
