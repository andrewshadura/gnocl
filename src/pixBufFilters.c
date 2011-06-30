/** filters.c
\brief
\author
\date
\since
\notes
\todo
        #define a clamp macro
\history
**/

#include "gnocl.h"
#include "gnoclparams.h"
#include <string.h>
#include <assert.h>

static guint32 convertRGBtoPixel ( gchar *clr );


int isBlack ( GdkPixbuf *pixbuf, gint x, gint y, gint threshold )
{
	guchar *pixels;
	guint rowstride, n_channels;
	gint r, g, b, a;
	guchar *p;
	double alpha;

	pixels = gdk_pixbuf_get_pixels ( pixbuf );
	rowstride = gdk_pixbuf_get_rowstride ( pixbuf );
	n_channels = gdk_pixbuf_get_n_channels ( pixbuf );

	p = pixels + x * n_channels + y * rowstride;

	r = p[0];
	g = p[1];
	b = p[2];
	a = ( n_channels == 4 ) ? p[3] : 0;

	//g_print ( "%d %d: %d %d %d\n", y,x, r, g, b );


	if ( ( r + g + b ) / 3 <= threshold )
	{
		//g_print ( "black\n" );
		//gdk_pixbuf_set_pixel ( pixbuf, 0x00ff0000, x, y );
		return 1;
	}

	else
	{
		gdk_pixbuf_set_pixel ( pixbuf, 0xff000000, x, y );
		return 0;
	}

}

/**
\brief	calculate angle between two points
	For each row, calculate first occurace of black, assume left margin!

	x1,y1
	+
	|\
	|a\
	|  \
	|   \
	-----+ x2,y2

	tan(a) = (x2-x1)/(y2-y1)

**/
double angle ( double x1, double y1, double x2, double y2 )
{
	double alpha;

	alpha = ( x2 - x1 ) / ( y2 - y1 );
}


/**
\brief	detect deskew angle
	For each row, calculate first occurace of black, assume left margin!
	Calculate angles between first point and other samples.
	Average out the angles.
**/
int deskew ( GdkPixbuf *pixbuf, Tcl_Interp * interp )
{
	double angle; /* the calculated angle */
	gint col, row; /* ie. x/y */
	gint w, h;
	double d;

	gint sum = 0;
	gint samples = 0;
	gint min = INT_MAX;
	gint max = 0;
	gint step = 1;
	gint threshold = 0; /* anything below this is black, anything above is white, ie ignored */
	gint margin = 0;
	gint offset = 10;
	gint mean = 0;
	gint first = 1;
	gint x1, y1, x2, y2;


	w = gdk_pixbuf_get_width ( pixbuf );
	h = gdk_pixbuf_get_height ( pixbuf );

	margin = w / 4;


	//g_print ( "width = %d height = %d\n", w, h );

	if ( w <= margin )
	{
		margin = w;
	}

	for ( row = step ; row < h; row += step )
	{
		for ( col = offset ; col <= margin; col++  )
		{
			if ( isBlack ( pixbuf, col, row, threshold ) )
			{
				sum += col;
				samples++;

				if ( col <= min )
				{
					min = col;
				}

				if ( col >= max )
				{
					max = col;
				}

				g_print ( "hit black row %d col %d min = %d max = %d\n", row, col, min, max );

				if ( first )
				{
					x1 = col; y1 = row;
					gdk_pixbuf_draw_circle ( pixbuf, 0x0000ff00, col, row, 5, 1 );
					first = 0;
				}

				else
				{
					x2 = col; y2 = row;
				}

				break;
			}

			else
			{
				//g_print ( "-----\n" );
				//gdk_pixbuf_set_pixel ( pixbuf, 0x0000ff00, col, row );
			}
		}
	}


	gdk_pixbuf_flood_fill ( pixbuf, 0x0000ff00, 5, 5 );

	gdk_pixbuf_draw_circle ( pixbuf, 0x0000ff00, x2, y2, 5, 1 );

	mean = sum / samples;

	g_print ( "samples = %d ; sum = %d ; average = %d\n", samples, sum, mean );
	g_print ( "min = %d ; max = %d\n", min, max );

	float slope;
	slope = ( ( float ) max - ( float ) min ) / h;
	g_print ( "Slope = %f\n", slope );

	x2 = ( int ) ( ( float ) y2 * slope );

	gdk_pixbuf_draw_circle ( pixbuf, 0x00ff0000, x1, y1, 5, 0 );
	gdk_pixbuf_draw_line ( pixbuf, 0x00ff0000, x2, y1, x1, y2 );

	angle = atan ( slope ) * 180 / G_PI;
	g_print ( "angle = %f %d\n", angle, ( int ) angle );

	GdkPixbuf *pb;
	pb = pixbufRotate ( pixbuf, angle, 255 );

	return gnoclRegisterPixBuf ( interp, pb, pixBufFunc );
}


/**
\brief
\author     William J Giddings
\date       20/03/2010
\note
\todo
**/
static void clamp_8bit ( gint *r, gint *g, gint *b, gint *a )
{

	if ( *r > 255 ) *r = 255; if ( *r < 0 ) *r = 0;

	if ( *g > 255 ) *g = 255; if ( *g < 0 ) *g = 0;

	if ( *b > 255 ) *b = 255; if ( *b < 0 ) *b = 0;

	if ( *a > 255 ) *a = 255; if ( *a < 0 ) *a = 0;
}

/**
\brief      Template for the creation of new filters.
\author     William J Giddings
\date       20/03/2010
\note
\todo
**/
int filter_template ( GdkPixbuf *pixbuf, gint x, gint y, gint w, gint h, gint dr, gint dg, gint db )
{

	gint j, k;
	gint p = 0;
	gint r, g, b, a;

	g_return_if_fail ( GDK_IS_PIXBUF ( pixbuf ) );

	/* this method prevents a deferencing error */
	k = gdk_pixbuf_get_width ( pixbuf );
	j = gdk_pixbuf_get_height ( pixbuf );

	gchar col[32];

	for ( j = y; j < y + h; j++ )
	{
		for ( k = x; k < x + h; k++ )
		{
			gdk_pixbuf_get_pixel ( pixbuf, k, j, &r, &g, &b, &a );

			/* put action here ! */
			r += dr; g += dg; b += db;

			clamp_8bit ( &r, &g, &b, &a );

			/* action done */

			sprintf ( col, "#%.2x%.2x%.2x", r, g, b );
			gdk_pixbuf_set_pixel ( pixbuf, convertRGBtoPixel ( col ), k, j );
			p++;
		}

	}

	return p;
}

/**
\brief
\author     William J Giddings
\date       20/03/2010
\note
\todo
**/
static void clamp_16bit ( gint *r, gint *g, gint *b, gint *a )
{

	if ( *r > 65535 ) *r = 65535; if ( *r < 0 ) *r = 0;

	if ( *g > 65535 ) *g = 65535; if ( *g < 0 ) *g = 0;

	if ( *b > 65535 ) *b = 65535; if ( *b < 0 ) *b = 0;

	if ( *a > 65535 ) *a = 65535; if ( *a < 0 ) *a = 0;
}


/**
\brief
\author     William J Giddings
\date       20/03/2010
\note
\todo
**/
int filter_color ( GdkPixbuf *pixbuf, gint x, gint y, gint w, gint h, gint dr, gint dg, gint db )
{

	gint j, k;
	gint p = 0;
	gint r, g, b, a;

	g_return_if_fail ( GDK_IS_PIXBUF ( pixbuf ) );

	/* this method prevents a deferencing error */
	k = gdk_pixbuf_get_width ( pixbuf );
	j = gdk_pixbuf_get_height ( pixbuf );

	gchar col[32];

	for ( j = y; j < y + h; j++ )
	{
		for ( k = x; k < x + h; k++ )
		{
			gdk_pixbuf_get_pixel ( pixbuf, k, j, &r, &g, &b, &a );

			/* put action here ! */
			r += dr; g += dg; b += db;

			clamp_8bit ( &r, &g, &b, &a );

			/* action done */

			sprintf ( col, "#%.2x%.2x%.2x", r, g, b );
			gdk_pixbuf_set_pixel ( pixbuf, convertRGBtoPixel ( col ), k, j );
			p++;
		}

		k = gdk_pixbuf_get_width ( pixbuf );
	}

	return p;
}


/**
\brief
\author     William J Giddings
\date       20/03/2010
\note
\todo
**/
int filter_gamma ( GdkPixbuf *pixbuf, gint x, gint y, gint w, gint h, gfloat gamma )
{
	gint j, k;
	gint p = 0;
	gint r, g, b, a;

	g_return_if_fail ( GDK_IS_PIXBUF ( pixbuf ) );

	/* this method prevents a deferencing error */
	k = gdk_pixbuf_get_width ( pixbuf );
	j = gdk_pixbuf_get_height ( pixbuf );

	/* create lookup tables */
	gfloat redGamma[255];
	gfloat greenGamma[255];
	gfloat blueGamma[255];

	gfloat red, green, blue;

	red = gamma;
	green = gamma;
	blue = gamma;

	gfloat tmp;

	gchar col[32];

	int i;

	/* fill the table with values */
	for ( i = 0; i < 256; ++i )
	{
		tmp = 255.0 * pow ( i / 255.0, 1.0 / red ) + 0.5;
		redGamma[i] = ( tmp < 255 ) ? tmp : 255;

		tmp = 255.0 * pow ( i / 255.0, 1.0 / green ) + 0.5;
		greenGamma[i] = ( tmp < 255 ) ? tmp : 255;

		tmp = 255.0 * pow ( i / 255.0, 1.0 / blue ) + 0.5;
		blueGamma[i] = ( tmp < 255 ) ? tmp : 255;

	}

	for ( j = y; j < y + h; j++ )
	{
		for ( k = x; k < x + h; k++ )
		{
			gdk_pixbuf_get_pixel ( pixbuf, k, j, &r, &g, &b, &a );

			g_print ( "\t\tgamma r = %f g = %f b = %f \n", redGamma[r], greenGamma[g], blueGamma[b] );

			/*------------ action done -------------*/

			sprintf ( col, "#%.2x%.2x%.2x", redGamma[r], greenGamma[g], blueGamma[b] );
			gdk_pixbuf_set_pixel ( pixbuf, convertRGBtoPixel ( col ), k, j );
			p++;
		}

		k = gdk_pixbuf_get_width ( pixbuf );
	}

	return p;
}

/**
\brief
\author     William J Giddings
\date       20/03/2010
\note
\todo
**/
int filter_brightness_range ( GdkPixbuf *pixbuf, gint x, gint y, gint w, gint h, gint *min, gint *max )
{

	gint j, k;
	gint p = 0;
	guchar r, g, b, a;

	g_return_if_fail ( GDK_IS_PIXBUF ( pixbuf ) );

	/* this method prevents a deferencing error */
	k = gdk_pixbuf_get_width ( pixbuf );
	j = gdk_pixbuf_get_height ( pixbuf );

	gchar col[32];

	guchar gray, bmin, bmax;

	for ( j = y; j < y + h; j++ )
	{
		for ( k = x; k < x + h; k++ )
		{
			gdk_pixbuf_get_pixel ( pixbuf, k, j, &r, &g, &b, &a );

			/*---------- put action here! ----------*/
			gray = ( 0.299 * r ) + ( 0.587 * g ) + ( 0.114 * b );

			if ( gray >= bmax )
			{
				bmax = gray;
			}

			if ( gray <= bmin )
			{
				bmin = gray;
			}

			/*------------ action done -------------*/
			p++;
		}

		k = gdk_pixbuf_get_width ( pixbuf );
	}

	*min = bmin;
	*max = bmax;
	return p;
}

/**
\brief      Adjust contrast of image based upon grayscale sample midpoint.
\author     William J Giddings
\date       20/03/2010
\note       The aim is to produce contrast chage by adjusting brightness
            rather than colour, ideally to eliminsate excessive false colour.
**/

int filter_contrast ( GdkPixbuf *pixbuf, gint x, gint y, gint w, gint h, gfloat contrast )
{
	gint j, k;
	gint p = 0;
	gint r, g, b, a;

	g_print ( "%s 1\n", __FUNCTION__ );

	g_return_if_fail ( GDK_IS_PIXBUF ( pixbuf ) );

	g_print ( "%s 2\n", __FUNCTION__ );

	/* this method prevents a deferencing error */
	k = gdk_pixbuf_get_width ( pixbuf );
	j = gdk_pixbuf_get_height ( pixbuf );

	gchar col[32];

	/* determine brightness range of the sample */

	gint min, max, mid;
	filter_brightness_range ( pixbuf, x, y, w, h, &min, &max );

	mid = ( min + max ) / 2;

	for ( j = y; j < y + h; j++ )
	{
		for ( k = x; k < x + h; k++ )
		{
			gdk_pixbuf_get_pixel ( pixbuf, k, j, &r, &g, &b, &a );

			/*---------- put action here! ----------*/

			/* get the gray value at this point */
			guchar gray = ( 0.299 * r ) + ( 0.587 * g ) + ( 0.114 * b );

			if ( gray > mid )
			{
				r += ( r * contrast );
				g += ( g * contrast );
				b += ( b * contrast );
			}

			else
			{
				r -= ( r * contrast );
				g -= ( g * contrast );
				b -= ( b * contrast );
			}

			clamp_8bit ( &r, &g, &b, &a );
			/*------------ action done -------------*/

			sprintf ( col, "#%.2x%.2x%.2x", r, g, b );
			gdk_pixbuf_set_pixel ( pixbuf, convertRGBtoPixel ( col ), k, j );
			p++;
		}

		k = gdk_pixbuf_get_width ( pixbuf );
	}

	return p;
}

/**
\brief
\author     William J Giddings
\date       20/03/2010
\note
\todo
**/
int filter_brightness ( GdkPixbuf *pixbuf, gint x, gint y, gint w, gint h, gint brightness )
{

	gint j, k;
	gint p = 0;
	gint r, g, b, a;

	g_return_if_fail ( GDK_IS_PIXBUF ( pixbuf ) );

	/* this method prevents a deferencing error */
	k = gdk_pixbuf_get_width ( pixbuf );
	j = gdk_pixbuf_get_height ( pixbuf );

	gchar col[32];

	for ( j = y; j < y + h; j++ )
	{
		for ( k = x; k < x + h; k++ )
		{
			gdk_pixbuf_get_pixel ( pixbuf, k, j, &r, &g, &b, &a );

			/*---------- put action here! ----------*/
			r += brightness;
			g += brightness;
			b += brightness;
			clamp_8bit ( &r, &g, &b, &a );
			/*------------ action done -------------*/

			sprintf ( col, "#%.2x%.2x%.2x", r, g, b );
			gdk_pixbuf_set_pixel ( pixbuf, convertRGBtoPixel ( col ), k, j );
			p++;
		}

		k = gdk_pixbuf_get_width ( pixbuf );
	}

	return p;
}

/**
\brief
\author     William J Giddings
\date       20/03/2010
\note
\todo
**/
int filter_grayscale ( GdkPixbuf *pixbuf, gint x, gint y, gint w, gint h )
{

	gint j, k;
	gint p = 0;
	guchar r, g, b, a;

	g_return_if_fail ( GDK_IS_PIXBUF ( pixbuf ) );

	/* this method prevents a deferencing error */
	k = gdk_pixbuf_get_width ( pixbuf );
	j = gdk_pixbuf_get_height ( pixbuf );

	gchar col[32];

	for ( j = y; j < y + h; j++ )
	{
		for ( k = x; k < x + h; k++ )
		{
			gdk_pixbuf_get_pixel ( pixbuf, k, j, &r, &g, &b, &a );

			/* put action here ! */
			guchar gray = ( 0.299 * r ) + ( 0.587 * g ) + ( 0.114 * b );
			/* action done */

			sprintf ( col, "#%.2x%.2x%.2x", gray, gray, gray );
			gdk_pixbuf_set_pixel ( pixbuf, convertRGBtoPixel ( col ), k, j );
			p++;
		}

		k = gdk_pixbuf_get_width ( pixbuf );
	}

	return p;
}

/**
\brief
\author     William J Giddings
\date       20/03/2010
\note
\todo
**/
int filter_invert ( GdkPixbuf *pixbuf, gint x, gint y, gint w, gint h )
{
	gint j, k;
	gint p = 0;
	guchar r, g, b, a;

	g_return_if_fail ( GDK_IS_PIXBUF ( pixbuf ) );

	/* this method prevents a deferencing error */
	k = gdk_pixbuf_get_width ( pixbuf );
	j = gdk_pixbuf_get_height ( pixbuf );

	gchar col[32];

	for ( j = y; j < y + h; j++ )
	{
		for ( k = x; k < x + h; k++ )
		{
			gdk_pixbuf_get_pixel ( pixbuf, k, j, &r, &g, &b, &a );

			/*---------- put action here! ----------*/

			r = 255 - r;
			g = 255 - g;
			b = 255 - b;
			a = 225 - a;
			/*------------ action done -------------*/

			sprintf ( col, "#%.2x%.2x%.2x", r, g, b );
			gdk_pixbuf_set_pixel ( pixbuf, convertRGBtoPixel ( col ), k, j );
			p++;
		}

		k = gdk_pixbuf_get_width ( pixbuf );
	}

	return p;
}

/**
\brief
**/
static void get_region_from_obj ( int objc, Tcl_Obj * const objv[], int *x, int *y , int *width, int *height )
{

	if ( strcmp ( Tcl_GetString ( objv[objc-2] ), "-region" ) != 0  )
	{
		return;
	}

	sscanf ( Tcl_GetString ( objv[objc-1] ), "%d %d %d %d", x, y, width, height );

	*width = *x + *width;
	*height = *y + *height;

}



/**
\brief
\author     William J Giddings
\date       20/03/2010
\note
\todo
**/
void gnoclPixBufFilters ( Tcl_Interp *interp, GdkPixbuf *pixbuf, int objc, Tcl_Obj * const objv[] )
{

//GdkPixbuf *pixbuf = NULL;
//pixbuf = para->pixbuf;

	g_printf ( "gnoclPixBufFilters\n" );
	gint _i;

	for ( _i = 0; _i < objc; _i++ )
	{
		g_printf ( "\targ %d = %s\n", _i,  Tcl_GetString ( objv[_i] ) );
	}

	GdkRectangle region;

	gint x, y;
	gint width;
	gint height;

	x = 0;
	y = 0;

	width = gdk_pixbuf_get_width ( pixbuf );
	height = gdk_pixbuf_get_height ( pixbuf );

	static char *filterOptions[] =
	{
		"invert", "grayscale", "brightness", "contrast",
		"gamma", "color", "brightnessRange", "deskew",
		"convolute",
		NULL
	};

	static enum  optsIdx
	{
		InvertIdx, GrayScaleIdx, BrightnessIdx, ContrastIdx,
		GammaIdx, ColorIdx, BrightnessRangeIdx, DeskewIdx,
		ConvoluteIdx
	};

	int Idx;

	if ( Tcl_GetIndexFromObj ( interp, objv[2], filterOptions, "option", TCL_EXACT, &Idx ) != TCL_OK )
	{
		return TCL_ERROR;
	}

	switch ( Idx )
	{
		case ConvoluteIdx:
			{

				gint ret;
				gint a1, a2, a3, b1, b2, b3, c1, c2, c3;
				int matrix [3][3];

				ret = sscanf ( Tcl_GetString ( objv[3] ), "%d %d %d %d %d %d %d %d %d", &a1, &a2, &a3, &b1, &b2, &b3, &c1, &c2, &c3 );

				if ( ret != 9 )
				{
					Tcl_SetResult ( interp, "Convulation matrix must have a minium of 9 elements.", TCL_STATIC );
					return TCL_ERROR;
				}

				matrix[0][0] = a1; 	matrix[0][1] = a2; 	matrix[0][2] = a3;
				matrix[1][0] = b1; 	matrix[1][1] = b2; 	matrix[1][2] = b3;
				matrix[2][0] = c1; 	matrix[2][1] = c2; 	matrix[2][2] = c3;

				filter_convolute ( pixbuf, matrix );
			}
			break;
		case DeskewIdx:
			{
				deskew ( pixbuf, interp );
			}
			break;
		case BrightnessRangeIdx:
			{
				gint i;
				gint min = 0;
				gint max = 0;
				g_print ( "Invert 1\n", i );

				get_region_from_obj ( objc, objv, &x, &y, &width, &height );

				i = filter_brightness_range ( pixbuf, x, y, width, height, &min, &max );
				g_print ( "Total Pixels Draw = %d, min = %d max = %d\n", i, min, max );
			}
			break;
		case InvertIdx:
			{
				gint i;
				g_print ( "Invert 1\n", i );
				get_region_from_obj ( objc, objv, &x, &y, &width, &height );
				i = filter_invert ( pixbuf, x, y, width, height );
				g_print ( "Total Pixels Draw = %d\n", i );
			} break;
		case GrayScaleIdx:
			{
				gint i;
				g_print ( "Grayscale 1\n", i );
				get_region_from_obj ( objc, objv, &x, &y, &width, &height );
				i = filter_grayscale ( pixbuf, x, y, width, height );
				g_print ( "Total Pixels Draw = %d\n", i );
			} break;
		case BrightnessIdx:
			{
				gint i;
				g_print ( "Brightness 1\n", i );
				i = filter_brightness ( pixbuf, x, y, width, height, 50 );
				g_print ( "Total Pixels Draw = %d\n", i );
			} break;
		case ContrastIdx:
			{

				gint i;
				g_print ( "Contrast 1\n" );

				gdouble factor;

				if ( Tcl_GetDoubleFromObj ( interp, objv[3], &factor ) )
				{
					Tcl_SetResult ( interp, "Must have contrast factor in range 0 - 1,0.", TCL_STATIC );
					TCL_ERROR;
				}

				g_print ( "Contrast 2\n" );

				get_region_from_obj ( objc, objv, &x, &y, &width, &height );

				i = filter_contrast ( pixbuf, x, y, width, height, factor );
				g_print ( "Total Pixels Draw = %d\n", i );

				g_print ( "Contrast 2\n" );

			} break;
		case GammaIdx:
			{
				gint i;
				g_print ( "Gamma 1\n", i );

				gdouble factor;

				if ( Tcl_GetDoubleFromObj ( interp, objv[3], &factor ) )
				{
					Tcl_SetResult ( interp, "Must have contrast factor in range 0 - 1,0.", TCL_STATIC );
					TCL_ERROR;
				}

				i = filter_gamma ( pixbuf, x, y, width, height, factor );
				g_print ( "Total Pixels Draw = %d\n", i );
			} break;
		case ColorIdx:
			{
				gint i, j;
				gint x, y, width, height;
				g_print ( "Color 1\n", i );
				gchar *opt;
				int idx;
				guchar r, g, b;

				static enum opIdx { OP_ADD, OP_SUBTRACT };
				gint op = OP_ADD;

				gdouble factor;

				static char *newOptions[] =
				{
					"-operation", "-x", "-y", "-width", "-height", "-color", NULL
				};

				static enum  optsIdx
				{
					OperationIdx, XIdx, YIdx, WidthIdx, HeightIdx, ColorIdx
				};

				for ( i = 3; i < objc; i += 2 )
				{

					j = i + 1;

					opt = Tcl_GetString ( objv[i] );

					if ( Tcl_GetIndexFromObj ( interp, objv[i], newOptions, "command", TCL_EXACT, &idx ) != TCL_OK )
					{
						return TCL_ERROR;
					}

					switch ( idx )
					{

						case OperationIdx:
							{
								g_print ( "Operations = %s\n", Tcl_GetString ( objv[j] ) );

								if ( g_string_equal ( Tcl_GetString ( objv[j] ) , "add" ) )
								{
									op = OP_ADD;
									g_print ( "ADD\n" );
								}

								if ( g_string_equal ( Tcl_GetString ( objv[j] ) , "subtract" ) )
								{
									g_print ( "SUBTRACT\n" );
									op = OP_SUBTRACT;
								}

								g_print ( "operation  = %d\n", op );
							}
							break;
						case XIdx:
							{

								g_print ( "X\n" );
								Tcl_GetIntFromObj ( NULL, objv[j], &x );
							} break;
						case YIdx:

							g_print ( "Y\n" );
							{
								Tcl_GetIntFromObj ( NULL, objv[j], &y );
							} break;
						case WidthIdx:
							{

								g_print ( "Width \n" );
								Tcl_GetIntFromObj ( NULL, objv[j], &width );
							} break;
						case HeightIdx:
							{

								g_print ( "Height \n" );
								Tcl_GetIntFromObj ( NULL, objv[j], &height );
							} break;
						case ColorIdx:
							{
								g_print ( "Color\n" );
								sscanf ( Tcl_GetString ( objv[j] ) , " % d % d % d", &r, &g, &b );
							}
							break;
						default: {}
					}

					g_print ( "op = %d x = % d y = % d width = % d height = % d r = % d g = % d b = % d\n", op, x, y, width, height, r, g, b );
				}

				i = filter_color ( pixbuf, x, y, x + width, y + height, r, g, b );
				g_print ( "Total Pixels Draw = % d\n", i );

			} break;
		default:
			{

				g_print ( "Got nothing to do!\n" );
			}
	}

	return TCL_OK;
}

/*---------------------------------------------------------------------*/

/**
\brief	Utitlity function to write out matrix details
\note	When a multidimensional array is passed, its dimensions must be stated.
**/
static void write_matrix ( int sum, int matrix [3][3] )
{
	g_print ( "matrix 0,0 = %d\n", matrix[0][0] );
	g_print ( "matrix 0,1 = %d\n", matrix[0][1] );
	g_print ( "matrix 0,2 = %d\n", matrix[0][2] );

	g_print ( "matrix 1,0 = %d\n", matrix[1][0] );
	g_print ( "matrix 1,1 = %d\n", matrix[1][1] );
	g_print ( "matrix 1,2 = %d\n", matrix[1][2] );

	g_print ( "matrix 2,0 = %d\n", matrix[2][0] );
	g_print ( "matrix 2,1 = %d\n", matrix[2][1] );
	g_print ( "matrix 2,2 = %d\n", matrix[2][2] );

	g_print ( "sum = %d\n", sum );

}


/**
\brief
\note	When a multidimensional array is passed, its dimensions must be stated.
**/
void filter_convolute ( GdkPixbuf *pixbuf, int matrix [3][3] ) //int matrix[3][3] )
{
	GdkPixbuf *copy_pb;
	gint w, h;
	gint x, y, i, j;
	guchar r, g, b, a;
	gchar col[32];
	gint sum;

	gfloat rsum, gsum, bsum, asum, shift;
	gfloat val;

	sum = 0;

	debugStep ( __FUNCTION__, 1.1 );


	for ( i = 0; i <= 2; i++ ) /* row */
	{
		for ( j = 0; j <= 2; j++ ) /* col */
		{
			sum += matrix[i][j]; /* this is ok */
		}
	}

	debugStep ( __FUNCTION__, 2.1 );

	//write_matrix (sum, matrix);

	copy_pb =  gdk_pixbuf_copy ( pixbuf );

	w = gdk_pixbuf_get_width ( copy_pb );
	h = gdk_pixbuf_get_height ( copy_pb );

	debugStep ( __FUNCTION__, 3.1 );

	/* for each row */
	for ( y = 70; y <= h - 70; y++ )
	{
		for ( x = 70; x <= w - 70; x++ )
		{
			rsum = gsum = bsum = asum = 0.0;

			/* use copy as source, original pixbuf as destination */

			for ( i = 0; i <= 2; i++ ) /* row */
			{
				for ( j = 0; j <= 2; j++ ) /* col */
				{

					val = ( float ) matrix[i][j] / sum ; /* this is ok */

					gdk_pixbuf_get_pixel ( copy_pb, x, y, &r, &g, &b, &a );

					//g_print("rgb = %d %d %d val = %f\n",r,g,b,val);

					if ( 1 )
					{
						rsum += ( float ) r * val;
						gsum += ( float ) g * val;
						bsum += ( float ) b * val;
					}

					else
					{
						rsum +=  r ;
						gsum +=  g ;
						bsum +=  b ;
					}

					//g_print("rgbsum = %f %f %f sum = %d\n",rsum,gsum,bsum,sum );

				}
			}


			if ( rsum < 0 )
			{
				rsum = 0;
			}

			if ( gsum < 0 )
			{
				gsum = 0;
			}

			if ( bsum < 0 )
			{
				bsum = 0;
			}

			if ( rsum > 255 )
			{
				rsum = 255;
			}

			if ( gsum > 255 )
			{
				gsum = 255;
			}

			if ( bsum > 255 )
			{
				bsum = 255;
			}

			//r = ( int ) rsum; g = ( int ) gsum ; b = ( int ) bsum;

			if ( 1 )
			{

				//sprintf ( col, "#%.2x%.2x%.2x", r, g, b );

				sprintf ( col, "#%.2x%.2x%.2x", rsum, gsum, bsum );
			}

			else
			{
				sprintf ( col, "#%.2x%.2x%.2x", r, g, b );
			}

//			g_print ("col = %s rgb = %d %d %d rgbsum = %f %f %f\n",col, r, g, b, rsum,gsum,bsum);

			//sprintf ( col, "#%.2x%.2x%.2x", 255,0,0 );

			gdk_pixbuf_set_pixel ( pixbuf, convertRGBtoPixel ( col ), x, y );
		}
	}

	debugStep ( __FUNCTION__, 4.1 );

}
