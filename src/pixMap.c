/** pixMap.c
\brief     Pixmaps are offscreen drawables.
\author    William J Giddings email: wjgiddings@blueyonder.co.uk
\date      25-03-2010
\version   0.9.94
\note      Module generated by gnocl_template_assistant.
**/

#include "gnocl.h"
#include <string.h>
#include <assert.h>


/* needs to be public to be accessed by gnocl::inventory */
static GHashTable *name2pixmapList;
static const char idPrefix[] = "::gnocl::_PMAP";


static const int Option1Idx = 0;
static const int Option2Idx = 1;

static GnoclOption pixMapOptions[] =
{
	/*  pixmap custom options, later processed by configure */
	{ "-option1", GNOCL_STRING, NULL },      /* option1Idx = 0 */
	{ "-option2", GNOCL_STRING, NULL},       /* option2Idx = 1 */

	/* class specific properties go here */

	/* class specific signals go here */

	/* used by most pixmaps, delete if uncessary */
	{ "-tooltip", GNOCL_OBJ, "", gnoclOptTooltip },

	/* GdkPixmap properties common to all gnocl pixmaps */
	{ "-data", GNOCL_OBJ, "", gnoclOptData },
	{ "-name", GNOCL_STRING, "name" },

	{NULL}
};





/**
\brief
\author     William J Giddings
\date       12-Jan-2010
\since      0.9.94
**/
static int cget ( Tcl_Interp *interp, GdkPixmap *pixmap, GnoclOption options[], int idx )
{
	return gnoclCgetNotImplemented ( interp, options + idx );
}

/**
\brief
\author     William J Giddings
\date       12-Jan-2010
\since      0.9.94
**/
static int configure ( Tcl_Interp *interp, GdkPixmap *pixmap, GnoclOption options[] )
{

	if ( options[Option1Idx].status == GNOCL_STATUS_CHANGED )
	{
		printf ( "Do something here for -option1 \n" );

	}

	if ( options[Option2Idx].status == GNOCL_STATUS_CHANGED )
	{
		printf ( "Do something here for -option2 \n" );

	}

	return TCL_OK;
}

/**
\brief
\author     William J Giddings
\date       12-Jan-2010
\since      0.9.94
**/
int pixMapFunc ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{


#ifdef DEBUG_PIXMAP
	g_printf ( "pixMapFunc\n" );
	gint _i;

	for ( _i = 0; _i < objc; _i++ )
	{
		g_printf ( "\targ %d = %s\n", _i,  Tcl_GetString ( objv[_i] ) );
	}

#endif


	/* set list of valid commands for this pixmap */
	static const char *cmds[] = { "class", "delete", "draw", "get", "copy", NULL };
	enum cmdIdx { ClassIdx, DeleteIdx, DrawIdx, GetIdx, CopyIdx };

	int idx;
	GdkPixmap *pixmap;

	pixmap  = GDK_PIXMAP ( data );

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
			/* return class of pixmap */
		case ClassIdx:
			{
				Tcl_SetObjResult ( interp, Tcl_NewStringObj ( "pixmap", -1 ) );
			}
			break;
			/* detroy the pixmap */
		case DeleteIdx:
			{
				return gnoclDelete ( interp, GDK_PIXMAP ( pixmap ), objc, objv );
			}
		case DrawIdx:
			{
				static const char *subCmd[] =
				{
					"line", "polyLine", "point", "points",
					"pixBuf", "segments", "rectangle", "arc",
					"polygon", "trapezoid", "glyph", "layout",
					"layoutLine", "image",  "drawable",
					NULL
				};

				enum cmdIdx
				{
					LineIdx, LinesIdx, PointIdx, PointsIdx,
					PixBufIdx, SegmentsIdx, RectangelIdx, ArcIdx,
					PolygonIDx, TrapezoidIdx, GlyphIdx, LayoutIdx,
					LayoutLineIdx, ImageIdx,  DrawableIdx
				};

				int cIdx;

				/* must have the form <command-name> action objectName <options> */
				if ( objc <= 1 )
				{
					Tcl_WrongNumArgs ( interp, 1, objv, "command option... " );
					return TCL_ERROR;
				}

				if ( Tcl_GetIndexFromObj ( interp, objv[2], subCmd, "option", TCL_EXACT, &cIdx ) != TCL_OK )
				{
					return TCL_ERROR;
				}

				switch ( cIdx )
				{
					case LineIdx:
						{
							GdkDrawable *drawable;
							GdkGC *gc;
							gint x1_;
							gint y1_;
							gint x2_;
							gint y2_;

							gdk_draw_line ( drawable, gc, x1_, y1_, x2_, y2_ );
						} break;
					case LinesIdx:
						{
							GdkDrawable *drawable;
							GdkGC *gc;
							GdkPoint *points;
							gint n_points;

							gdk_draw_lines ( drawable, gc, points, n_points );
						} break;
					case PointIdx:
						{
							g_print ( "\t\tdraw -> point\n" );

							GdkDrawable *drawable;
							GdkGC *gc;
							gint x;
							gint y;

							x = 10;
							y = 10;
							gc = gdk_gc_new ( drawable );
							gdk_draw_point ( drawable, gc, x, y );

						} break;
					case PointsIdx:
						{
							GdkDrawable *drawable;
							GdkGC *gc;
							GdkPoint *points;
							gint n_points;

							gdk_draw_points ( drawable, gc, points, n_points );
						} break;
					case PixBufIdx:
						{
							GdkDrawable *drawable;
							GdkGC *gc;
							GdkPixbuf *pixbuf;
							gint src_x;
							gint src_y;
							gint dest_x;
							gint dest_y;
							gint width;
							gint height;
							GdkRgbDither dither;
							gint x_dither;
							gint y_dither;

							gdk_draw_pixbuf ( drawable, gc, pixbuf, src_x, src_y, dest_x, dest_y, width, height, dither, x_dither, y_dither );
						} break;
					case SegmentsIdx:
						{
							GdkDrawable *drawable;
							GdkGC *gc;
							const GdkSegment *segs;
							gint n_segs;

							gdk_draw_segments ( drawable, gc, segs, n_segs );
						} break;
					case RectangelIdx:
						{
							GdkDrawable *drawable;
							GdkGC *gc;
							gboolean filled;
							gint x;
							gint y;
							gint width;
							gint height;

							gdk_draw_rectangle ( drawable, gc, filled, x, y, width, height );
						} break;
					case ArcIdx:
						{
							GdkDrawable *drawable;
							GdkGC *gc;
							gboolean filled;
							gint x;
							gint y;
							gint width;
							gint height;
							gint angle1;
							gint angle2;

							gdk_draw_arc ( drawable, gc, filled, x, y, width, height, angle1, angle2 );

						} break;
					case PolygonIDx:
						{

							GdkDrawable *drawable;
							GdkGC *gc;
							gboolean filled;
							const GdkPoint *points;
							gint n_points;

							gdk_draw_polygon ( drawable, gc, filled, points, n_points );

						} break;
					case TrapezoidIdx:
						{
							GdkDrawable *drawable;
							GdkGC *gc;
							GdkTrapezoid *trapezoids;
							gint n_trapezoids;

							gdk_draw_trapezoids ( drawable, gc, trapezoids, n_trapezoids );
						} break;
					case GlyphIdx:
						{
							GdkDrawable *drawable;
							GdkGC *gc;
							PangoFont *font;
							gint x;
							gint y;
							PangoGlyphString *glyphs;;
							const PangoMatrix *matrix;

							gdk_draw_glyphs ( drawable, gc, font, x, y, glyphs );

							gdk_draw_glyphs_transformed ( drawable, gc, matrix, font, x, y, glyphs );


						} break;
					case LayoutIdx:
						{
							GdkDrawable *drawable;
							GdkGC *gc;
							gint x;
							gint y;
							PangoLayout *layout;

							gdk_draw_layout ( drawable, gc, x, y, layout );

						} break;

					case LayoutLineIdx:
						{

							GdkDrawable *drawable;
							GdkGC *gc;
							gint x;
							gint y;
							PangoLayoutLine *line;

							const GdkColor *foreground;
							const GdkColor *background;

							gdk_draw_layout_line ( drawable, gc, x, y, line );

							gdk_draw_layout_line_with_colors ( drawable, gc, x, y, line, foreground, background );


						}
						break;



					case ImageIdx:
						{
//deprecated String

							GdkDrawable *drawable;
							GdkGC *gc;
							GdkImage *image;
							gint xsrc;
							gint ysrc;
							gint xdest;
							gint ydest;
							gint width;
							gint height;


							gdk_draw_image ( drawable, gc, image, xsrc, ysrc, xdest, ydest, width, height );

						} break;

					case DrawableIdx:
						{
							GdkDrawable *drawable;
							GdkGC *gc;
							GdkDrawable *src;
							gint xsrc;
							gint ysrc;
							gint xdest;
							gint ydest;
							gint width;
							gint height;

							gdk_draw_drawable ( drawable, gc, src, xsrc, ysrc, xdest, ydest, width, height );


						} break;

				}
			}
			break;
		case GetIdx:
			{

				GdkDrawable *drawable;
				gint x;
				gint y;
				gint width;
				gint height;
				GdkImage *image;
				image = gdk_drawable_get_image ( drawable, x, y, width, height );
			}
		case CopyIdx:
			{
				GdkDrawable *drawable;
				GdkImage *image1;
				gint src_x;
				gint src_y;
				gint dest_x;
				gint dest_y;
				gint width;
				gint height;

				GdkImage *image2;

				image2 = gdk_drawable_copy_to_image ( drawable, image1, src_x, src_y, dest_x, dest_y, width, height );
			}
			break;
		default:
			{
				break;
			}

	}

	return TCL_OK;
}

/**
\brief
\author     William J Giddings
\date       12-Jan-2010
\since      0.9.94
**/
int gnoclPixMapCmd ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{

#ifdef DEBUG_PIXMAP
	g_printf ( "gnoclPixMapCmd\n" );
	gint _i;

	for ( _i = 0; _i < objc; _i++ )
	{
		g_printf ( "\targ %d = %s\n", _i,  Tcl_GetString ( objv[_i] ) );
	}

#endif


	/* create new hash table if one does not exist */
	if ( name2pixmapList == NULL )
	{
		name2pixmapList = g_hash_table_new ( g_direct_hash, g_direct_equal );
	}

	GdkPixmap *pixmap;
	GdkDrawable *drawable;
	pixmap = NULL;
	drawable = NULL;

	gint width, height, depth;

	/* set some default values, then re-assign based upon received values */
	width = 640;
	height = 480;
	depth = 8;

	static const char *cmd[] =
	{
		"new",
		NULL
	};

	enum cmdIdx
	{
		NewIdx
	};

	int cIdx;


	/* must have the form <command-name> action objectName <options> */
	if ( objc <= 1 )
	{
		Tcl_WrongNumArgs ( interp, 1, objv, "command option... " );
		return TCL_ERROR;
	}

	if ( Tcl_GetIndexFromObj ( interp, objv[1], cmd, "option", TCL_EXACT, &cIdx ) != TCL_OK )
	{
		return TCL_ERROR;
	}

	switch ( cIdx )
	{
		case NewIdx:
			{

				gint i, j;
				int idx;

				gchar *opt, *val;

				static char *newOptions[] =
				{
					"-drawable", "-width", "-height", "-depth", NULL
				};

				static enum  optsIdx
				{
					DrawableIdx, WidthIdx, HeightIdx, DepthIdx
				};

				/* parse remaining the switches */
				/* set relevant options for commands / sub-commands */

				for ( i = 2; i < objc; i += 2 )
				{
					j = i + 1;

					opt = Tcl_GetString ( objv[i] );
					val = Tcl_GetString ( objv[j] );

					if ( Tcl_GetIndexFromObj ( interp, objv[i], newOptions, "command", TCL_EXACT, &idx ) != TCL_OK )
					{
						return TCL_ERROR;
					}


					switch ( idx )
					{
						case DrawableIdx:
							{
								g_printf ( "drawable = %s.\n", val );
								GtkWidget *widget;
								widget = gnoclGetWidgetFromName ( val, interp );
								drawable = GDK_DRAWABLE ( widget->window );
								depth = -1;

								gdk_drawable_get_size ( drawable, &width, &height );

							}
							break;
						case WidthIdx:
							{
								g_printf ( "\t\toption1, parameters = %s val = %s\n", opt, val );
								Tcl_GetIntFromObj ( NULL, objv[j], &width );
							}
							break;
						case HeightIdx:
							{
								g_printf ( "\t\toption1, parameters = %s val = %s\n", opt, val );
								Tcl_GetIntFromObj ( NULL, objv[j], &height );
							}
							break;
						case DepthIdx:
							{
								g_printf ( "\t\toption1, parameters = %s val = %s\n", opt, val );
								Tcl_GetIntFromObj ( NULL, objv[j], &depth );
							}
							break;
						default:
							{
								return TCL_ERROR;
							}
					}

				} /* end of switch parsing */

				pixmap = gdk_pixmap_new ( drawable, width, height, depth );


				return gnoclRegisterPixMap ( interp, pixmap, pixMapFunc );
				//Tcl_SetObjResult ( interp, Tcl_NewStringObj ( name, -1 ) ) ;
			} break;
	}


	/* successfully parsed options passed to the 'new' command, so create pixmap */



	int ret;


	/* 1) check the validity of the options passed from the interpeter */
	if ( gnoclParseOptions ( interp, objc, objv, pixMapOptions ) != TCL_OK )
	{
		gnoclClearOptions ( pixMapOptions );
		return TCL_ERROR;
	}

	/* a) apply base pixmap properties and signals */
	ret = gnoclSetOptions ( interp, pixMapOptions, G_OBJECT ( pixmap ), -1 );

	/* b) apply options not handled above */
	if ( ret == TCL_OK )
	{
		ret = configure ( interp, pixmap, pixMapOptions );
	}

	/* c) free up memory */
	gnoclClearOptions ( pixMapOptions );

	/* d) in the case of invalid options, destroy the pixmgap instances and quit */
	if ( ret != TCL_OK )
	{
		//gdk_pixmap_destroy ( GDK_PIXMAP ( pixmap ) );
		return TCL_ERROR;
	}


	/* 4) register the pixmap with the interpreter and associate it with a handler function */
	return gnoclRegisterPixmap ( interp, GDK_PIXMAP ( pixmap ), pixMapFunc );

}

/**
\brief
\author    William J Giddings
\date
\since      0.9.94
**/
int gnoclRegisterPixmap ( Tcl_Interp *interp, GdkPixmap *pixmap, Tcl_ObjCmdProc *proc )
{

	const char *name;

	name = gnoclGetAutoPixBufId();

	gnoclMemNameAndPixMap ( name, pixmap ); //<--- problems here

	//g_signal_connect_after ( G_OBJECT ( pixbuf ), "destroy", G_CALLBACK ( simpleDestroyFunc ), interp );

	if ( proc != NULL )
	{

		Tcl_CreateObjCommand ( interp, ( char * ) name, proc, pixmap, NULL );
	}

	Tcl_SetObjResult ( interp, Tcl_NewStringObj ( name, -1 ) );

	return TCL_OK;
}


/**
\brief
\author     William J Giddings
\date       12-Feb-09
\since      0.9.94
\note      "char *" and not "const char *" because of a not very strict
            handling of "const char *" in Tcl e.g. Tcl_CreateObjCommand
*/


char *gnoclGetAutoPixMapId ( void )
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
\author     William J Giddings
\date       12-Feb-09
\since      0.9.94
**/
int gnoclMemNameAndPixMap ( const char *path,  GdkPixmap *pixmap )
{

	int n ;

	n = atoi ( path + sizeof ( idPrefix ) - 1 );

	assert ( n > 0 );
	assert ( g_hash_table_lookup ( name2pixmapList, GINT_TO_POINTER ( n ) ) == NULL );
	assert ( strncmp ( path, idPrefix, sizeof ( idPrefix ) - 1 ) == 0 );

	/* memorize the name of the widget in the widget */
	g_object_set_data ( G_OBJECT ( pixmap ), "gnocl::name", ( char * ) path );
	g_hash_table_insert ( name2pixmapList, GINT_TO_POINTER ( n ), pixmap );

	return 0;
}

/**
\brief
\author    William J Giddings
\date
\since      0.9.94
**/
int gnoclRegisterPixMap ( Tcl_Interp *interp, GdkPixmap *pixmap, Tcl_ObjCmdProc *proc )
{

	const char *name;

	name = gnoclGetAutoPixMapId();

	gnoclMemNameAndPixMap ( name, pixmap ); //<--- problems here

	//g_signal_connect_after ( G_OBJECT ( pixbuf ), "destroy", G_CALLBACK ( simpleDestroyFunc ), interp );

	if ( proc != NULL )
	{

		Tcl_CreateObjCommand ( interp, ( char * ) name, proc, pixmap, NULL );
	}

	Tcl_SetObjResult ( interp, Tcl_NewStringObj ( name, -1 ) );

	return TCL_OK;
}

/**
\brief      Returns the widget name associated with pointer
\author     Peter G Baum
\date
\since      0.9.94
**/
const char *gnoclGetNameFromPixMap ( GdkPixmap *pixmap )
{
	const char *name = g_object_get_data ( G_OBJECT ( pixmap ), "gnocl::name" );

	return name;
}

/**
\brief
\author     William J Giddings, Peter G Baum
\date
\since      0.9.94
**/
/* -----------------
   handle widget <-> name mapping
-------------------- */
GdkPixbuf *gnoclGetPixMapFromName ( const char *id, Tcl_Interp *interp )
{
	GdkPixmap *pixmap = NULL;
	int       n;

	if ( strncmp ( id, idPrefix, sizeof ( idPrefix ) - 1 ) == 0
			&& ( n = atoi ( id + sizeof ( idPrefix ) - 1 ) ) > 0 )
	{
		pixmap = g_hash_table_lookup ( name2pixmapList, GINT_TO_POINTER ( n ) );
	}

	if ( pixmap == NULL && interp != NULL )
	{
		Tcl_AppendResult ( interp, "Unknown pixbuf \"", id, "\".", ( char * ) NULL );
	}

	return pixmap;
}

/**
\brief
\author     William J Giddings, Peter G Baum
\date
\since      0.9.94
**/
void gnoclGetPixMapList ( GList **list )
{
	g_hash_table_foreach ( name2pixmapList, hash_to_list, list );
}


/**********************************************************************
pixmap draw
/**********************************************************************/

