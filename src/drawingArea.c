/**
\brief      This module implements the gnocl::drawingArea widget.
**/

/**
\page page_drawingArea gnocl::drawingArea
\htmlinclude drawingArea.html
**/

/* user documentation */

#include "gnocl.h"
#include <gdk/gdkkeysyms.h>

/*
    The Widget implementation has a number of empty entries within its
    options array. This will allow the coder to configure the widget
    under scripted control, thereby providing a Gnocl framework for
    customised widget creation.
*/

static gint _n = 7; /* counter,number of implemented widget options */

static GnoclOption drawingAreaOptions[] =
{
	/* common widget options */
	{ "-tooltip", GNOCL_OBJ, "", gnoclOptTooltip },
	{ "-data", GNOCL_OBJ, "", gnoclOptData },
	{ "-name", GNOCL_STRING, "name" },
	{ "-onShowHelp", GNOCL_OBJ, "", gnoclOptOnShowHelp },
	{ "-cursor", GNOCL_OBJ, "", gnoclOptCursor },
	{ "-sensitive", GNOCL_BOOL, "sensitive" },
	{ "-visible", GNOCL_BOOL, "visible" },

	/* widget specific options, 30 blank spaces */
	{ "", GNOCL_OBJ, "", NULL },
	{ "", GNOCL_OBJ, "", NULL },
	{ "", GNOCL_OBJ, "", NULL },
	{ "", GNOCL_OBJ, "", NULL },
	{ "", GNOCL_OBJ, "", NULL },
	{ "", GNOCL_OBJ, "", NULL },
	{ "", GNOCL_OBJ, "", NULL },
	{ "", GNOCL_OBJ, "", NULL },
	{ "", GNOCL_OBJ, "", NULL },
	{ "", GNOCL_OBJ, "", NULL },
	{ "", GNOCL_OBJ, "", NULL },
	{ "", GNOCL_OBJ, "", NULL },
	{ "", GNOCL_OBJ, "", NULL },
	{ "", GNOCL_OBJ, "", NULL },
	{ "", GNOCL_OBJ, "", NULL },
	{ "", GNOCL_OBJ, "", NULL },
	{ "", GNOCL_OBJ, "", NULL },
	{ "", GNOCL_OBJ, "", NULL },
	{ "", GNOCL_OBJ, "", NULL },
	{ "", GNOCL_OBJ, "", NULL },
	{ "", GNOCL_OBJ, "", NULL },
	{ "", GNOCL_OBJ, "", NULL },
	{ "", GNOCL_OBJ, "", NULL },
	{ "", GNOCL_OBJ, "", NULL },
	{ "", GNOCL_OBJ, "", NULL },
	{ "", GNOCL_OBJ, "", NULL },
	{ "", GNOCL_OBJ, "", NULL },
	{ "", GNOCL_OBJ, "", NULL },
	{ "", GNOCL_OBJ, "", NULL },
	{ "", GNOCL_OBJ, "", NULL },
	{ "", GNOCL_OBJ, "", NULL },
	{ "", GNOCL_OBJ, "", NULL },
	{ NULL },
};

/*
      GDK_EXPOSURE_MASK         GDK_POINTER_MOTION_MASK     GDK_POINTER_MOTION_HINT_MASK
      GDK_BUTTON_MOTION_MASK    GDK_BUTTON1_MOTION_MASK     GDK_BUTTON2_MOTION_MASK
      GDK_BUTTON3_MOTION_MASK   GDK_BUTTON_PRESS_MASK       GDK_BUTTON_RELEASE_MASK
      GDK_KEY_PRESS_MASK        GDK_KEY_RELEASE_MASK        GDK_ENTER_NOTIFY_MASK
      GDK_LEAVE_NOTIFY_MASK     GDK_FOCUS_CHANGE_MASK       GDK_STRUCTURE_MASK
      GDK_PROPERTY_CHANGE_MASK  GDK_VISIBILITY_NOTIFY_MASK  GDK_PROXIMITY_IN_MASK
      GDK_PROXIMITY_OUT_MASK    GDK_SUBSTRUCTURE_MASK       GDK_SCROLL_MASK
      GDK_ALL_EVENTS_MASK
*/

static const char *options[] =
{
	"-onExpose",        "-onPointerMotion",  "motionHint",
	"-onButtonMotion",  "-onButton1Motion", "-onButton2Motion",
	"-onButton3Motion", "-onButtonPress",   "-onButtonRelease",
	"-onKeyPress",      "-onKeyRelease",    "-onEnter",
	"-onLeave",         "-onFocus",         "structure",
	"propertyChange",   "visibility",       "proximityIn",
	"proximityOut",     "substructure",     "-onScroll",
	"all",
	NULL
};

enum OptsIdx
{
	OnExposeIdx,        OnPointerMotionIdx, PointerMotionHintIdx,
	OnButtonMotionIdx,  OnButton1MotionIdx, OnButton2MotionIdx,
	OnButton3MotionIdx, OnButtonPressIdx,   OnButtonReleaseIdx,
	OnKeyPressIdx,      OnKeyReleaseIdx,    OnEnterIdx,
	OnLeaveIdx,         OnFocusIdx,         StructureIdx,
	PropertyChangeIdx,  VisibilityIdx,      ProximityInIdx,
	ProximityOutIdx,    SubstructureIdx,    OnScrollIdx,
	AllIdx
};

/**
\brief
**/
static int configure ( Tcl_Interp *interp, GtkDrawingArea *area, GnoclOption options[] )
{

	gint ret = gnoclSetOptions ( interp, options, area, -1 );

	return TCL_OK;
}

/**
/brief
**/
int drawingAreaFunc ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{
#ifdef DEBUG_DRAWING_AREA
	listParameters ( objc, objv, "drawingAreaFunc" );
#endif

	static const char *cmds[] =
	{
		"draw", "cget", "configure",
		"delete", "class", "erase",
		"option",
		NULL
	};
	enum cmdIdx
	{
		DrawIdx, CgetIdx, ConfigureIdx,
		DeleteIdx, ClassIdx, EraseIdx,
		OptionIdx,
	};

	/*
	static const char *events[] =
	{
	    "expose",           "motion",           "motionHint",
	    "buttonMotion",     "button1Motion",    "button2Motion",
	    "button3Motion",    "buttonPress",      "buttonRelease",
	    "keyPress",         "keyRelease",       "enter",
	    "leave",            "focus",            "structure",
	    "propertyChange",   "visibility",       "proximityIn",
	    "proximityOut",     "substructure",     "scroll",
	    "all", NULL
	};
	*/

	GtkWidget *area = GTK_WIDGET ( data );

	int idx;
	int idx2;

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

		case OptionIdx:
			{
#ifdef DEBUG_DRAWING_AREA
				g_print ( "drawingArea OptionIdx\n" );
#endif

				/*
				other options here could include:
				    remove      -remove the option from the options array (incl. script)
				    available   -return a list of currently configured options
				    suspend     -remove option from the event mask only
				    resume      -add the option to the event mask
				*/

				if ( strcmp ( Tcl_GetString ( objv[2] ), "add" )  )
				{
					return TCL_ERROR;
				}


				/* determine how many items in the add list */
				gint noOptions;
				gint n;

				Tcl_ListObjLength ( interp, objv[3], &noOptions );

				for ( n = 0; n < noOptions; ++n )
				{
					GtkWidget *childWidget;
					Tcl_Obj *tp;

					if ( Tcl_ListObjIndex ( interp,  objv[3], n, &tp ) != TCL_OK )
					{
						return TCL_ERROR;
					}

					int OptIdx;

					if ( Tcl_GetIndexFromObj ( interp, tp, options, "option", TCL_EXACT, &OptIdx ) != TCL_OK )
					{
						return TCL_ERROR;
					}

					_n = doOptionAdd ( interp, area, OptIdx );

				}

				break;
			case ConfigureIdx:
				{
#ifdef DEBUG_DRAWING_AREA
					g_print ( "drawingArea ConfigureIdx\n" );
#endif
					int ret = TCL_ERROR;

					if ( gnoclParseAndSetOptions ( interp, objc - 1, objv + 1, drawingAreaOptions, area ) == TCL_OK )
					{
						ret = configure ( interp, area, drawingAreaOptions );
					}

					gnoclClearOptions ( drawingAreaOptions );

					return ret;
				}

				break;
			case DeleteIdx:
				{
#ifdef DEBUG_DRAWING_AREA
					g_print ( "delete\n" );
#endif
					return gnoclDelete ( interp, area, objc, objv );
				}

				break;
			case ClassIdx:
				{
#ifdef DEBUG_DRAWING_AREA
					g_print ( "Class = drawingArea\n" );
#endif
					Tcl_SetObjResult ( interp, Tcl_NewStringObj ( "drawingArea", -1 ) );
				}

				break;
			case DrawIdx:
				{
#ifdef DEBUG_DRAWING_AREA
					g_print ( "draw\n" );
#endif
					/* relocate these to relevant parts of the souce file */
					static const char *drawOpts[] =
					{
						"point", "points", "line",
						"lines", "pixbuf", "segments",
						"rectangle", "arc", "polygon",
						"trapezoids", "glyph", "glyphTransformed",
						"layoutLine", "layoutWithColors", "string",
						"text", "image", "queue",
						NULL
					};

					enum drawOptsIdx
					{
						PointIdx, PointsIdx, LineIdx,
						LinesIdx, PixbufIdx, SegmentsIdx,
						RectangleIdx, ArcIdx, PolygonIdx,
						TrapezoidsIdx, GlyphIdx, GlyphTransformedIdx,
						LayoutLineIdx, LayoutWithColorsIdx, StringIdx,
						TextIdx, ImageIdx, QueueIdx
					};

					gint idx2;

					if ( Tcl_GetIndexFromObj ( interp, objv[2], drawOpts, "option", TCL_EXACT, &idx2 ) != TCL_OK )
					{
						return TCL_ERROR;
					}

					switch ( idx2 )
					{
						case QueueIdx:
							{
								gtk_widget_queue_draw ( area );
							}
							break;
						case PointIdx:
							{
								g_print ( "drawing point %s\n", Tcl_GetString ( objv[3] ) );
								gint x, y;
								sscanf ( Tcl_GetString ( objv[3] ), "%d %d", &x, &y );
								gdk_draw_point ( area->window, area->style->fg_gc[GTK_WIDGET_STATE ( area ) ], x, y );
							}
							break;
						case PointsIdx:
							{

								int k, n, npoints, x, y;
								Tcl_ListObjLength ( interp, objv[3], &npoints );
								g_print ( "drawing points %s (%d)\n", Tcl_GetString ( objv[3] ), npoints  );


								GdkPoint points[npoints]; // = { {x, y} , {x + 1, y} , {x - 1, y} , {x, y + 1} , {x, y - 1} };
								k = 0;

								for ( n = 0; n < npoints; n += 2 )
								{
									GtkWidget *childWidget;
									Tcl_Obj *tpx, *tpy;

									if ( Tcl_ListObjIndex ( interp,  objv[3], n, &tpx ) != TCL_OK )
									{
										return TCL_ERROR;
									}

									if ( Tcl_ListObjIndex ( interp,  objv[3], n + 1, &tpy ) != TCL_OK )
									{
										return TCL_ERROR;
									}

									Tcl_GetIntFromObj ( interp, tpx, &x );
									Tcl_GetIntFromObj ( interp, tpy, &y );

									g_print ( "points = %d: item %d x = %d : y = %d\n", npoints, k, x, y );

									points[k].x = x;
									points[k].y = y;
									k++;

								}

								gdk_draw_points ( area->window, area->style->fg_gc[GTK_WIDGET_STATE ( area ) ], points, npoints / 2 );

							}
							break;
						case LineIdx:
							{
								g_print ( "drawing line %s\n", Tcl_GetString ( objv[3] ) );
								gint x1, y1, x2, y2;
								sscanf ( Tcl_GetString ( objv[3] ), "%d %d %d %d", &x1, &y1, &x2, &y2 );
								gdk_draw_line ( area->window, area->style->fg_gc[GTK_WIDGET_STATE ( area ) ], x1, y1, x2, y2 );
							}
							break;
						case LinesIdx:
							{

								int k, n, npoints, x, y;
								Tcl_ListObjLength ( interp, objv[3], &npoints );
								g_print ( "drawing lines %s (%d)\n", Tcl_GetString ( objv[3] ), npoints  );


								GdkPoint points[npoints]; // = { {x, y} , {x + 1, y} , {x - 1, y} , {x, y + 1} , {x, y - 1} };
								k = 0;

								for ( n = 0; n < npoints; n += 2 )
								{
									GtkWidget *childWidget;
									Tcl_Obj *tpx, *tpy;

									if ( Tcl_ListObjIndex ( interp,  objv[3], n, &tpx ) != TCL_OK )
									{
										return TCL_ERROR;
									}

									if ( Tcl_ListObjIndex ( interp,  objv[3], n + 1, &tpy ) != TCL_OK )
									{
										return TCL_ERROR;
									}

									Tcl_GetIntFromObj ( interp, tpx, &x );
									Tcl_GetIntFromObj ( interp, tpy, &y );

									g_print ( "points = %d: item %d x = %d : y = %d\n", npoints, k, x, y );

									points[k].x = x;
									points[k].y = y;
									k++;

								}

								gdk_draw_lines ( area->window, area->style->fg_gc[GTK_WIDGET_STATE ( area ) ], points, npoints / 2 );


								//gdk_draw_lines (GdkDrawable *drawable, GdkGC *gc, const GdkPoint *points, gint n_points);

							}
							break;
						case PixbufIdx:
							{
								g_print ( "drawing pixbuf %s\n", Tcl_GetString ( objv[3] ) );
								GdkPixbuf *pixbuf;
								GdkRgbDither dither;
								gint src_x, src_y, dest_x, dest_y, width, height;
								gint x_dither, y_dither;
								gint ret;

								/* set some defaults */
								dither = GDK_RGB_DITHER_NONE;
								src_x = 0; src_y = 0;
								dest_x = 0; dest_y = 0;
								width = -1; height = -1;
								x_dither = 0; y_dither = 0;

								pixbuf = gnoclGetPixBufFromName ( Tcl_GetString ( objv[3] ), interp );

								width = gdk_pixbuf_get_width ( pixbuf );
								height = gdk_pixbuf_get_height ( pixbuf );

								int Idx2;
								static const char *pbOpts[] =
								{
									"-from", "-to",
									"-dither", "-offset",
									NULL
								};

								enum pbIdx
								{
									FromIdx, ToIdx,
									DitherIdx, OffsetIdx,
								};

								/* parse all the options */
								int i = 4;

								while ( i < objc )
								{
									//g_print ( "i = %d %s\n", i, Tcl_GetString ( objv[i] ) );

									getIdx ( pbOpts, Tcl_GetString ( objv[i] ), &Idx2 );

									switch ( Idx2 )
									{
										case FromIdx:
											{

												ret = sscanf ( Tcl_GetString ( objv[i+1] ), "%d %d %d %d", &src_x, &src_y, &width, &height );

												if ( ret < 2 )
												{
													return TCL_ERROR;
												}

												g_print ( "-from %d %d %d %d\n", src_x, src_y, width, height );
											}
											break;
										case ToIdx:
											{

												ret = sscanf ( Tcl_GetString ( objv[i+1] ), "%d %d", &dest_x, &dest_y );

												if ( ret != 2 )
												{
													return TCL_ERROR;
												}

												g_print ( "-to %d %d\n", dest_x, dest_y );

											}
											break;
										case DitherIdx:
											{
												/* one of none, normal or max : defaul = none */
												if ( strcmp ( Tcl_GetString ( objv[i+1] ), "none" ) == 0 )
												{
													dither = GDK_RGB_DITHER_NONE;
												}

												if ( strcmp ( Tcl_GetString ( objv[i+1] ), "normal" ) == 0 )

												{
													dither = GDK_RGB_DITHER_NORMAL;
												}

												if ( strcmp ( Tcl_GetString ( objv[i+1] ), "max" ) == 0 )
												{
													dither = GDK_RGB_DITHER_MAX;
												}

												g_print ( "-dither %d\n", dither );
											}

											break;
										case OffsetIdx:
											{
												ret = sscanf ( Tcl_GetString ( objv[i+1] ), "%d %d", &x_dither, &y_dither );

												if ( ret != 2 )
												{
													return TCL_ERROR;
												}

												g_print ( "-offset %d %d\n", x_dither, y_dither );
											}
											break;
										default:
											{

												return TCL_ERROR;
											}
									}

									i += 2;
								}

								gdk_draw_pixbuf ( area->window, NULL, GDK_PIXBUF ( pixbuf ), src_x, src_y, dest_x, dest_y, width, height, dither, x_dither, y_dither );


							}
							break;

						case SegmentsIdx:
							{

								int k, n, nsegments, x1, y1, x2, y2;
								Tcl_ListObjLength ( interp, objv[3], &nsegments );
								g_print ( "drawing segments %s (%d)\n", Tcl_GetString ( objv[3] ), nsegments  );
								GtkWidget *childWidget;
								Tcl_Obj *tpx1, *tpy1, *tpx2, *tpy2;


								GdkSegment segments[nsegments];
								k = 0;

								for ( n = 0; n < nsegments; n += 4 )
								{

									g_print ( "n = %d\n", n );


									if ( Tcl_ListObjIndex ( interp,  objv[3], n, &tpx1 ) != TCL_OK )
									{
										return TCL_ERROR;
									}

									if ( Tcl_ListObjIndex ( interp,  objv[3], n + 1, &tpy1 ) != TCL_OK )
									{
										return TCL_ERROR;
									}

									if ( Tcl_ListObjIndex ( interp,  objv[3], n + 2, &tpx2 ) != TCL_OK )
									{
										return TCL_ERROR;
									}

									if ( Tcl_ListObjIndex ( interp,  objv[3], n + 3, &tpy2 ) != TCL_OK )
									{
										return TCL_ERROR;
									}


									Tcl_GetIntFromObj ( interp, tpx1, &x1 );
									Tcl_GetIntFromObj ( interp, tpy1, &y1 );
									Tcl_GetIntFromObj ( interp, tpx2, &x2 );
									Tcl_GetIntFromObj ( interp, tpy2, &y2 );

									segments[k].x1 = x1;
									segments[k].y1 = y1;
									segments[k].x2 = x2;
									segments[k].y2 = y2;

									g_print ( "points = %d: item %d x1 = %d : y1 = %d : x2 = %d : y2 = %d\n", nsegments, k, x1, y1, x2, y2 );
									k++;

								}

								gdk_draw_segments ( area->window, area->style->fg_gc[GTK_WIDGET_STATE ( area ) ], segments, nsegments / 4 );

							}
							break;
						case RectangleIdx:
							{
								g_print ( "drawing rectangle %s\n", Tcl_GetString ( objv[3] ) );
								gint x, y, width, height;
								gint filled;

								filled = 0;

								sscanf ( Tcl_GetString ( objv[3] ), "%d %d %d %d", &x, &y, &width, &height );

								int Idx3;
								static const char *rectOpts[] =
								{
									"-filled",
									NULL
								};

								enum rectIdx
								{
									FilledIdx
								};

								int i = 4;

								while ( i < objc )
								{
									g_print ( "%d %d\n", i, objc );
									getIdx ( rectOpts, Tcl_GetString ( objv[i] ), &Idx3 );

									switch ( Idx3 )
									{
										case FilledIdx:
											{
												g_print ( "yo\n" );
												Tcl_GetIntFromObj ( NULL, objv[i+1], &filled );
											}
											break;
										default: {};
									}

									i += 2;
								}

								gdk_draw_rectangle ( area->window, area->style->fg_gc[GTK_WIDGET_STATE ( area ) ], filled, x, y, width, height );

							} break;
						case ArcIdx:
							{
								g_print ( "drawing arc %s\n", Tcl_GetString ( objv[3] ) );


								gboolean filled;
								gint x, y;
								gint width, height;
								gint angle1, angle2;


								sscanf ( Tcl_GetString ( objv[3] ), "%d %d %d %d %d %D", &x, &y, &width, &height, &angle1, &angle2 );

								int Idx4;
								static const char *arcOpts[] =
								{
									"-filled",
									NULL
								};

								enum arcIdx
								{
									FilledIdx
								};

								int i = 4;

								while ( i < objc )
								{
									g_print ( "%d %d\n", i, objc );
									getIdx ( arcOpts, Tcl_GetString ( objv[i] ), &Idx4 );

									switch ( Idx4 )
									{
										case FilledIdx:
											{
												Tcl_GetIntFromObj ( NULL, objv[i+1], &filled );
											}
											break;
										default: {};
									}

									i += 2;
								}


								gdk_draw_arc ( area->window, area->style->fg_gc[GTK_WIDGET_STATE ( area ) ], filled, x, y, width, height, angle1, angle2 );


							}
							break;
						case PolygonIdx:
							{

								int k, n, npoints, x, y;
								gboolean filled;
								Tcl_ListObjLength ( interp, objv[3], &npoints );
								g_print ( "drawing polygon %s (%d)\n", Tcl_GetString ( objv[3] ), npoints  );


								GdkPoint points[npoints];
								k = 0;

								for ( n = 0; n < npoints; n += 2 )
								{
									GtkWidget *childWidget;
									Tcl_Obj *tpx, *tpy;

									if ( Tcl_ListObjIndex ( interp,  objv[3], n, &tpx ) != TCL_OK )
									{
										return TCL_ERROR;
									}

									if ( Tcl_ListObjIndex ( interp,  objv[3], n + 1, &tpy ) != TCL_OK )
									{
										return TCL_ERROR;
									}

									Tcl_GetIntFromObj ( interp, tpx, &x );
									Tcl_GetIntFromObj ( interp, tpy, &y );

									g_print ( "points = %d: item %d x = %d : y = %d\n", npoints, k, x, y );

									points[k].x = x;
									points[k].y = y;
									k++;

								}


								int Idx5;
								static const char *polyOpts[] =
								{
									"-filled",
									NULL
								};

								enum polyIdx
								{
									FilledIdx
								};

								int i = 4;

								while ( i < objc )
								{
									g_print ( "%d %d\n", i, objc );
									getIdx ( polyOpts, Tcl_GetString ( objv[i] ), &Idx5 );

									switch ( Idx5 )
									{
										case FilledIdx:
											{
												Tcl_GetIntFromObj ( NULL, objv[i+1], &filled );
											}
											break;
										default: {};
									}

									i += 2;
								}

								//gdk_draw_points ( area->window, area->style->fg_gc[GTK_WIDGET_STATE ( area ) ], points, npoints / 2 );

								gdk_draw_polygon ( area->window, area->style->fg_gc[GTK_WIDGET_STATE ( area ) ], filled, points, npoints / 2 );

							} break;
						case TrapezoidsIdx:
							{
								/*
								double y1;	the y coordinate of the start point.
								double x11; the x coordinate of the top left corner
								double x21; the x coordinate of the top right corner
								double y2; 	the y coordinate of the end point.
								double x12; the x coordinate of the bottom left corner
								double x22; the x coordinate of the bottom right corner
								*/

								int k, n, ntrapezoids;
								double y1, x11, x21, y2, x12, x22;

								Tcl_ListObjLength ( interp, objv[3], &ntrapezoids );
								g_print ( "drawing polygon %s (%d)\n", Tcl_GetString ( objv[3] ), ntrapezoids  );

								GdkTrapezoid trapezoids[ntrapezoids];
								k = 0;

								for ( n = 0; n < ntrapezoids; n += 6 )
								{
									GtkWidget *childWidget;
									Tcl_Obj *tpy1, *tpx11, *tpx21, *tpy2, *tpx12, *tpx22;

									Tcl_ListObjIndex ( interp,  objv[3], n, &tpy1 );
									Tcl_ListObjIndex ( interp,  objv[3], n + 1, &tpx11 );
									Tcl_ListObjIndex ( interp,  objv[3], n + 2, &tpx21 );
									Tcl_ListObjIndex ( interp,  objv[3], n + 3, &tpy2 );
									Tcl_ListObjIndex ( interp,  objv[3], n + 4 , &tpx12 );
									Tcl_ListObjIndex ( interp,  objv[3], n + 5, &tpx22 );


									Tcl_GetIntFromObj ( interp, tpy1, &y1 );
									Tcl_GetIntFromObj ( interp, tpx11, &x11 );
									Tcl_GetIntFromObj ( interp, tpx21, &x21 );
									Tcl_GetIntFromObj ( interp, tpy2, &y2 );
									Tcl_GetIntFromObj ( interp, tpx12, &x12 );
									Tcl_GetIntFromObj ( interp, tpx22, &x22 );

									trapezoids[k].y1 = y1;
									trapezoids[k].x11 = x11;
									trapezoids[k].x21 = x21;
									trapezoids[k].y2 = y2;
									trapezoids[k].x12 = x12;
									trapezoids[k].x22 = x11;

									k++;

								}

								gdk_draw_trapezoids ( area->window, area->style->fg_gc[GTK_WIDGET_STATE ( area ) ], trapezoids, ntrapezoids / 2 );

							} break;
						case GlyphIdx:
							{
								gint x, y;
								PangoGlyphString *glyphs;
								g_print ( "NOT YET IMPLEMENTED drawing glyph %s\n", Tcl_GetString ( objv[3] ) );
//gdk_draw_glyphs (GdkDrawable *drawable, GdkGC *gc, PangoFont *font, gint x, gint y, PangoGlyphString *glyphs);






							} break;
						case GlyphTransformedIdx:
							{
								g_print ( "NOT YET IMPLEMENTED  drawing glyphTransformed %s\n", Tcl_GetString ( objv[3] ) );
							} break;
						case LayoutLineIdx:
							{
								g_print ( "NOT YET IMPLEMENTED  drawing layoutLine %s\n", Tcl_GetString ( objv[3] ) );
							} break;
						case LayoutWithColorsIdx:
							{
								g_print ( "NOT YET IMPLEMENTED  drawing layoutWithColors %s\n", Tcl_GetString ( objv[3] ) );
							} break;
						case StringIdx:
							{
								g_print ( "drawing string %s\n", Tcl_GetString ( objv[3] ) );

								GdkFont *font;
								GdkGC *gc;
								gint x;
								gint y;
								const gchar *string;
								/*
								typedef struct {
								  GdkFontType type;
								  gint ascent;
								  gint descent;
								} GdkFont;
								*/

								font = gdk_font_load ( "-adobe-helvetica-medium-r-normal--*-120-*-*-*-*-*-*" );
								x = 100;
								y = 100;
								string = "Hello World!";


								int Idx3;
								static const char *rectOpts[] =
								{
									"-string", "-font",
									"-x", "-y",
									NULL
								};

								enum rectIdx
								{
									StringIdx, FontIdx, XIdx, YIdx
								};

								int i = 4;

								while ( i < objc )
								{
									g_print ( "%d %d\n", i, objc );
									getIdx ( rectOpts, Tcl_GetString ( objv[i] ), &Idx3 );

									switch ( Idx3 )
									{
										case StringIdx:
											{
												g_print ( "yo\n" );
												string = Tcl_GetString ( objv[i+1] );
											}
											break;
										case FontIdx:
											{
												g_print ( "yo\n" );
												string = Tcl_GetString ( objv[i+1] );
											}
											break;
										case XIdx:
											{
												g_print ( "yo\n" );
												Tcl_GetIntFromObj ( NULL, objv[i+1], &x );
											}
											break;
										case YIdx:
											{
												g_print ( "yo\n" );
												Tcl_GetIntFromObj ( NULL, objv[i+1], &y );
											}
											break;
										default: {};
									}

									i += 2;
								}

								gdk_draw_string ( area->window, font,  area->style->fg_gc[GTK_WIDGET_STATE ( area ) ], x, y, Tcl_GetString ( objv[3] ) );

							} break;
						case TextIdx:
							{
								g_print ( "drawing text %s\n", Tcl_GetString ( objv[3] ) );
								/*
									char *fnt = Tcl_GetStringFromObj ( opt->val.obj, NULL );
									PangoFontDescription *font_desc = pango_font_description_from_string ( fnt );

									gtk_widget_modify_font ( GTK_WIDGET ( obj ), font_desc );
									pango_font_description_free ( font_desc );
								*/

							} break;
						case ImageIdx:
							{
								g_print ( "NOT YET IMPLEMENTED  drawing image %s\n", Tcl_GetString ( objv[3] ) );
							} break;
						default:
							{
								g_print ( "drawing defaul %s\n", Tcl_GetString ( objv[3] ) );
							}
					}
				}
				break;
			default: {}
			}
	}



	return TCL_OK;
}


/**
/brief
**/
int gnoclDrawingAreaCmd ( ClientData data, Tcl_Interp *interp, int objc,	Tcl_Obj * const objv[] )
{
	int       ret;
	GtkWidget *widget;

	if ( gnoclParseOptions ( interp, objc, objv, drawingAreaOptions ) != TCL_OK )
	{
		gnoclClearOptions ( drawingAreaOptions );
		return TCL_ERROR;
	}

	widget = gtk_drawing_area_new ();
	gtk_widget_set_can_focus ( widget, 1 );	// allow widget to receive keyboard events

	gtk_widget_show ( GTK_WIDGET ( widget ) );

	gnoclSetOptions ( interp, drawingAreaOptions, G_OBJECT ( widget ), -1 );

	gnoclClearOptions ( drawingAreaOptions );


	/* register the new widget for use with the Tcl interpretor */
	return gnoclRegisterWidget ( interp, GTK_WIDGET ( widget ), drawingAreaFunc );

}

/**
\brief      Add new settings to the widget option array.
\author     William J Giddings
\date       19/Feb/2010
\since      0.9.94
\see        http://library.gnome.org/devel/gdk/stable/gdk-Events.html
\note       Event Mask Options:

      GDK_EXPOSURE_MASK         GDK_POINTER_MOTION_MASK     GDK_POINTER_MOTION_HINT_MASK
      GDK_BUTTON_MOTION_MASK    GDK_BUTTON1_MOTION_MASK     GDK_BUTTON2_MOTION_MASK
      GDK_BUTTON3_MOTION_MASK   GDK_BUTTON_PRESS_MASK       GDK_BUTTON_RELEASE_MASK
      GDK_KEY_PRESS_MASK        GDK_KEY_RELEASE_MASK        GDK_ENTER_NOTIFY_MASK
      GDK_LEAVE_NOTIFY_MASK     GDK_FOCUS_CHANGE_MASK       GDK_STRUCTURE_MASK
      GDK_PROPERTY_CHANGE_MASK  GDK_VISIBILITY_NOTIFY_MASK  GDK_PROXIMITY_IN_MASK
      GDK_PROXIMITY_OUT_MASK    GDK_SUBSTRUCTURE_MASK       GDK_SCROLL_MASK
      GDK_ALL_EVENTS_MASK

**/
int doOptionAdd ( Tcl_Interp *interp, GtkWidget *area, gint OptIdx )
{
#ifdef DEBUG_DRAWING_AREA
	g_print ( "drawingArea OptionIdx = %s\n", options[OptIdx] );
#endif


	/* return error the maximum number of options has been reached */
	gint j = sizeof ( drawingAreaOptions ) / sizeof ( GnoclOption );

	if ( j == _n )
	{
		return TCL_ERROR;
	}

	/* set the option name directly from list of possible options */
	drawingAreaOptions[_n].optName = options[OptIdx];

	/* set the remaining values, assign callback function */
	switch ( OptIdx )
	{
		case OnExposeIdx:
			{
				//g_print ( "adding GDK_EXPOSURE_MASK OptIdx = %d\n", OptIdx );
				gtk_widget_add_events ( area, GDK_EXPOSURE_MASK );
				drawingAreaOptions[_n].func = gnoclOptOnExpose;
			} break;

		case OnButton1MotionIdx:
			{
				//g_print ( "adding GDK_BUTTON1_MOTION_MASK OptIdx = %d\n", OptIdx );
				gtk_widget_add_events ( area, GDK_BUTTON1_MOTION_MASK );
				drawingAreaOptions[_n].func = gnoclOptOnKeyPress;
			} break;

		case OnButton2MotionIdx:
			{
				//g_print ( "adding GDK_BUTTON2_MOTION_MASK OptIdx = %d\n", OptIdx );
				gtk_widget_add_events ( area, GDK_BUTTON2_MOTION_MASK );
				drawingAreaOptions[_n].func = gnoclOptOnKeyPress;
			} break;

		case OnButton3MotionIdx:
			{
				//g_print ( "adding GDK_BUTTON1_MOTION_MASK OptIdx = %d\n", OptIdx );
				gtk_widget_add_events ( area, GDK_BUTTON2_MOTION_MASK );
				drawingAreaOptions[_n].func = gnoclOptOnKeyPress;
			} break;

		case OnKeyPressIdx:
			{
				//g_print ( "adding GDK_KEY_PRESS_MASK OptIdx = %d\n", OptIdx );
				gtk_widget_add_events ( area, GDK_KEY_PRESS_MASK );
				drawingAreaOptions[_n].func = gnoclOptOnKeyPress;
			} break;

		case OnKeyReleaseIdx:
			{
				//g_print ( "adding GDK_KEY_RELEASE_MASK OptIdx = %d\n", OptIdx );
				gtk_widget_add_events ( area, GDK_KEY_RELEASE_MASK );
				drawingAreaOptions[_n].func = gnoclOptOnKeyRelease;
			}
			break;

		case OnButtonMotionIdx:
			{
				//g_print ( "adding GDK_BUTTON_MOTION_MASK OptIdx = %d\n", OptIdx );
				gtk_widget_add_events ( area, GDK_BUTTON_MOTION_MASK );
				drawingAreaOptions[_n].func = gnoclOptOnButtonMotion;
			}
			break;

		case OnPointerMotionIdx: // working ok!
			{
				//g_print ( "adding GDK_POINTER_MOTION_MASK  OptIdx = %d\n", OptIdx );
				gtk_widget_add_events ( area, GDK_POINTER_MOTION_MASK );
				drawingAreaOptions[_n].func = gnoclOptOnMotion;
			}
			break;

		case OnEnterIdx: // working ok!
			{
				//g_print ( "adding GDK_ENTER_NOTIFY_MASK  OptIdx = %d\n", OptIdx );
				gtk_widget_add_events ( area, GDK_ENTER_NOTIFY_MASK );
				drawingAreaOptions[_n].propName = "E";
				drawingAreaOptions[_n].func = gnoclOptOnEnterLeave;
			}
			break;

		case OnLeaveIdx: // working ok!
			{
				//g_print ( "adding GDK_ENTER_NOTIFY_MASK  OptIdx = %d\n", OptIdx );
				gtk_widget_add_events ( area, GDK_LEAVE_NOTIFY_MASK );
				drawingAreaOptions[_n].propName = "L";
				drawingAreaOptions[_n].func = gnoclOptOnEnterLeave;
			}
			break;
		case OnButtonPressIdx: // working ok!
			{
				//g_print ( "adding GDK_BUTTON_PRESS_MASK  OptIdx = %d\n", OptIdx );
				gtk_widget_add_events ( area, GDK_BUTTON_PRESS_MASK ) ;
				drawingAreaOptions[_n].propName = "P";
				drawingAreaOptions[_n].func = gnoclOptOnButton;
			}
			break;

		case OnButtonReleaseIdx: // working ok!
			{
				//g_print ( "adding GDK_BUTTON_RELEASE_MASK  OptIdx = %d\n", OptIdx );
				gtk_widget_add_events ( area, GDK_BUTTON_RELEASE_MASK ) ;
				drawingAreaOptions[_n].propName = "R";
				drawingAreaOptions[_n].func = gnoclOptOnButton;
			}
			break;
	}

	/* implement changes */
	gnoclSetOptions ( interp, drawingAreaOptions, area, -1 );

	return ++_n;
}

