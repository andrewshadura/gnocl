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
#include "gnoclparams.h"


typedef struct _Data Data;

struct _Data
{
	GtkWidget *box,
			  *image,
			  *entry;
	GdkPixbuf *pixbuf;
};


static gchar *dash;

/* needs to be public to be accessed by gnocl::inventory */
static GHashTable *name2cairoList;
static const char idPrefix[] = "::gnocl::_Cr";


/**********************************************************************/

/**
\brief
**/
int gnoclRegisterCairo ( Tcl_Interp * interp, cairo_t *cr, Tcl_ObjCmdProc * proc )
{

	const char *name;

	name = gnoclGetAutoCairoId();

	gnoclMemNameAndCairo ( name, cr ); //<--- problems here

	//g_signal_connect_after ( G_OBJECT ( pixbuf ), "destroy", G_CALLBACK ( simpleDestroyFunc ), interp );

	if ( proc != NULL )
	{

		Tcl_CreateObjCommand ( interp, ( char * ) name, proc, cr, NULL );
	}

	Tcl_SetObjResult ( interp, Tcl_NewStringObj ( name, -1 ) );

	return TCL_OK;
}

/**
\brief
**/
PixbufParams *gnoclGetCairoFromName ( const char * id, Tcl_Interp * interp )
{
#ifdef DEBUG_CAIRO
	g_print ( "%s\n", __FUNCTION__ );
#endif

	CairoParams *para = NULL;

	//GdkPixbuf *pixbuf = NULL;
	int       n;

	if ( strncmp ( id, idPrefix, sizeof ( idPrefix ) - 1 ) == 0
			&& ( n = atoi ( id + sizeof ( idPrefix ) - 1 ) ) > 0 )
	{
		para = g_hash_table_lookup ( name2cairoList, GINT_TO_POINTER ( n ) );
	}

	if ( para == NULL && interp != NULL )
	{
		Tcl_AppendResult ( interp, "Unknown Cairo context \"", id, "\".", ( char * ) NULL );
	}

	return para;
}


/**
\brief
**/
int cairoFunc ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj* const objv[] )
{

#ifdef DEBUG_CAIRO
	listParameters ( objc, objv, __FUNCTION__ );
#endif

	return TCL_OK;

	static const char *cmds[] = { "turn", "flip", "delete", "configure", "class", NULL };
	enum cmdIdx { TurnIdx, FlipIdx, DeleteIdx, ConfigureIdx, ClassIdx };
	int idx;
	//GtkImage *image = ( GtkImage* ) data;

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
		case FlipIdx:
			{
				//GdkPixbuf *pixbuf;
				//pixbuf = gtk_image_get_pixbuf ( image );

			default:
				{
				}
			}
			return TCL_OK;

	}
}


/**
\brief
**/
int gnoclMemNameAndCairo ( const char * path,  cairo_t *cr )
{
	int n ;

	n = atoi ( path + sizeof ( idPrefix ) - 1 );

	assert ( n > 0 );
	assert ( g_hash_table_lookup ( name2cairoList, GINT_TO_POINTER ( n ) ) == NULL );
	assert ( strncmp ( path, idPrefix, sizeof ( idPrefix ) - 1 ) == 0 );

	/* memorize the name of the widget in the widget */
	g_object_set_data ( G_OBJECT ( cr ), "gnocl::name", ( char * ) path );
	g_hash_table_insert ( name2cairoList, GINT_TO_POINTER ( n ), cr );

	return 0;
}


/**
\brief
**/
static void getToken ( char * str, char **ptr, int * i )
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
static setLineContext ( cairo_t * cr )
{
#ifdef DEBUG_CAIRO
	g_print ( "%s\n", __FUNCTION__ );
#endif

//	cairo_set_line_width ( cr, CairoParams.lineWidth );
//	cairo_set_source_rgba ( cr, CairoParams.line.r, CairoParams.line.g, CairoParams.line.b, CairoParams.line.a );
}

/**
\brief
**/
static setFillContext ( cairo_t * cr )
{
#ifdef DEBUG_CAIRO
	g_print ( "%s\n", __FUNCTION__ );
#endif

//	cairo_set_source_rgba ( cr, CairoParams.fill.r, CairoParams.fill.g, CairoParams.fill.b, CairoParams.fill.a );
}


/**

**/
static setTextContext ( cairo_t * cr )
{
#ifdef DEBUG_CAIRO
	g_print ( "%s\n", __FUNCTION__ );
#endif

	gint idx;
	gint slant, weight;

	slant = 0;
	weight = 0;

//	g_print ( "font   => %s\n", CairoParams.font.face );
//	g_print ( "size   => %d\n", CairoParams.font.size );
//	g_print ( "slant  => %s\n", CairoParams.font.slant );
//	g_print ( "weight => %s\n", CairoParams.font.weight );

	static char *slantOptions[] =
	{
		"normal", "italic", "oblique",
		NULL
	};

	static enum  slantIdx
	{
		SNormalIdx, ItalicIdx, ObliqueIdx
	};

//	getIdx ( slantOptions, CairoParams.font.slant, &idx );

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

//	getIdx ( weightOptions, CairoParams.font.weight, &idx );

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

//	cairo_select_font_face ( cr, CairoParams.font.weight, slant, weight );
//	cairo_set_font_size ( cr, CairoParams.font.size );
}

/**
\brief	Get the number of entries in a line dash.
\note	Cannot use strtok or strtok_r but need a similar reetrant token parser.
**/
static int getDashLength ( char * str )
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
static void setDash ( cairo_t * cr )
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
//		sprintf ( dash, "%s", CairoParams.dash );
//		len = getDashLength ( CairoParams.dash );
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
\brief
**/
int gnoclCairoCmd ( ClientData data, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[] )
{
#ifdef DEBUG_CAIRO
	g_print ( "%s\n", __FUNCTION__ );
#endif

	CairoParams *para;
	para = g_new ( CairoParams, 1 );	/* create new hash table if one does not exist */

	if ( name2cairoList == NULL )
	{
		name2cairoList = g_hash_table_new ( g_direct_hash, g_direct_equal );
	}

registerCairo:
	/* keep pointers */

	para->name = gnoclGetAutoWidgetId();
	//g_signal_connect ( G_OBJECT ( para->button ), "destroy", G_CALLBACK ( destroyFunc ), para );
	gnoclMemNameAndCairo ( para->name, para->cr );
	Tcl_CreateObjCommand ( interp, para->name, cairoFunc, para, NULL );
	Tcl_SetObjResult ( interp, Tcl_NewStringObj ( para->name, -1 ) );

	return TCL_OK;


//	return gnoclRegisterCairo ( interp, para->cr, cairoFunc );



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
				g_print ( "LineIdx %s\n", Tcl_GetString ( objv[2] ) );
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
				g_print ( "%s %s - option not yet implmented NOT YET IMPLEMENTED\n", Tcl_GetString ( objv[1] ), Tcl_GetString ( objv[2] ) );
			}
	}


	return TCL_OK;
}
