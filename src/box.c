/*
 * $Id: box.c,v 1.9 2005/01/01 15:27:54 baum Exp $
 *
 * This file implements the box widget which is a combination of the
 * gtk box widget and the gtk frame widget. The frame widget is only
 * created if necessary.
 *
 * Copyright (c) 2001 - 2005 Peter G. Baum  http://www.dr-baum.net
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

/*
   History:
   2013-05: debugged the remove command 
   2012-04: added commands, reorder, addStart
			corrected bugs in addBegin command
   2013-02: added gnocl::hBox and gnocl::vBox commands
   2012-04: added command, remove
   2010-05: added -labelWidget option
   2009-10: changed default padding from GNOCL_PAD (8 pixels) to GNOCL_PAD_TINY (2 pixels)
   2009-01: added -data
   2008-10: added command, class
        08: added gnoclOptPadding
   2003-02: added drag and drop options
        08: switched from GnoclWidgetOptions to GnoclOption
   2002-01: STD_OPTIONS
        12: added alignment
        11: new option layout
        09: bug fixes and cleanups
        04: added frame options
   2001-03: Begin of developement
 */

/**
\page page_box gnocl::box
\htmlinclude box.html
**/

#include "gnocl.h"

/* declarations */
static int optLayout ( Tcl_Interp *interp, GnoclOption *opt,  GObject *object, Tcl_Obj **ret );
int gnoclOptFrameLabelWidget ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret );


static const int orientationIdx  = 0;
static const int buttonTypeIdx   = 1;
static const int childrenIdx     = 2;
static const int layoutIdx       = 3;

/*
Indicies of options that specifically relate to sub-widgets
Used by the function needFrame()
*/

static const int startFrameOpts = 8;
static const int startCommonOpts = 13;
static const int startPackOpts = 26;

static const int paddingDiff = 0;
static const int fillDiff = 1;
static const int expandDiff = 2;
static const int alignDiff = 3;

static const int dataIdx = 7;

enum { ExpandDefault = 1, FillDefault = 1, PaddingDefault = 0 };

static GnoclOption boxOptions[] =
{
	/* box options */
	{ "-orientation", GNOCL_OBJ, NULL },    						/* 0 */
	{ "-buttonType", GNOCL_OBJ, NULL },     						/* 1 */
	{ "-children", GNOCL_OBJ, NULL },       						/* 2 */
	{ "-layout", GNOCL_OBJ, "layout-style", optLayout }, 			/* 3 */
	{ "-homogeneous", GNOCL_BOOL, "homogeneous" }, 					/* 4 */
	{ "-spacing", GNOCL_OBJ, "spacing", gnoclOptPadding }, 			/* 5 */
	{ "-borderWidth", GNOCL_OBJ, "border-width", gnoclOptPadding },	/* 6 */
	{ "-data", GNOCL_OBJ, "", gnoclOptData },						/* 7 */

	/* frame options : startFrameOpts = 8 */
	{ "-label", GNOCL_STRING, "label" },    						/* 8 */
	{ "-shadow", GNOCL_OBJ, "shadow", gnoclOptShadow },
	{ "-labelAlign", GNOCL_OBJ, "label-xalign", gnoclOptHalign },
	{ "-background", GNOCL_OBJ, "normal", gnoclOptGdkColorBg },
	{ "-labelWidget", GNOCL_OBJ, "", gnoclOptFrameLabelWidget },

	/* common options startCommonOpts = 13 */
	{ "-name", GNOCL_STRING, "name" },      						/* 13 */
	{ "-visible", GNOCL_BOOL, "visible" },
	{ "-sensitive", GNOCL_BOOL, "sensitive" },
	{ "-tooltip", GNOCL_OBJ, "", gnoclOptTooltip },
	{ "-onShowHelp", GNOCL_OBJ, "", gnoclOptOnShowHelp },
	{ "-onPopupMenu", GNOCL_OBJ, "popup-menu", gnoclOptCommand },
	{ "-dropTargets", GNOCL_LIST, "t", gnoclOptDnDTargets },
	{ "-dragTargets", GNOCL_LIST, "s", gnoclOptDnDTargets },
	{ "-onDropData", GNOCL_OBJ, "", gnoclOptOnDropData },
	{ "-onDragData", GNOCL_OBJ, "", gnoclOptOnDragData },
	{ "-widthGroup", GNOCL_OBJ, "w", gnoclOptSizeGroup },
	{ "-heightGroup", GNOCL_OBJ, "h", gnoclOptSizeGroup },
	{ "-sizeGroup", GNOCL_OBJ, "s", gnoclOptSizeGroup },

	/* pack options : startPackOpts = 25 */
	{ "-padding", GNOCL_OBJ, NULL },        /* 0 */
	{ "-fill", GNOCL_OBJ, NULL },           /* 1 */
	{ "-expand", GNOCL_BOOL, NULL },        /* 2 */
	{ "-align", GNOCL_OBJ, NULL },          /* 3 */

	{ "-activeBackgroundColor", GNOCL_OBJ, "active", gnoclOptGdkColorBg },
	{ "-normalBackgroundColor", GNOCL_OBJ, "normal", gnoclOptGdkColorBg },
	{ "-prelightBackgroundColor", GNOCL_OBJ, "prelight", gnoclOptGdkColorBg },

	{ NULL }
};


/**
\brief
**/
static int cget ( Tcl_Interp *interp, GtkWidget *widget, GnoclOption options[], int idx )
{
#ifdef DEBUG_BOX
	g_print ( "%s\n", __FUNCTION__ );
#endif

	Tcl_Obj *obj = NULL;

	if ( idx == dataIdx )
	{
		obj = Tcl_NewStringObj ( g_object_get_data ( widget, "gnocl::data" ), -1 );
	}

	if ( obj != NULL )
	{
		Tcl_SetObjResult ( interp, obj );
		return TCL_OK;
	}

	return gnoclCgetNotImplemented ( interp, options + idx );
}

/**
\brief
**/
static void alignDestroyFunc ( GtkWidget *widget, gpointer data )
{
	gtk_widget_destroy ( GTK_WIDGET ( data ) );
}

/**
\brief
**/
static int addChildren ( GtkBox *box, Tcl_Interp *interp, Tcl_Obj *children, GnoclOption *options, int begin )
{
	int n, noChildren;
	gfloat xAlign = 0.5, yAlign = 0.5;  /* center */
	int    isHorizontal = GTK_IS_HBOX ( box );
	double xFill = isHorizontal ? 0. : 1., yFill = isHorizontal ? 1. : 0.;
	int    needAlign = 0;
	int    fill = 0;
	int    expand = 0;
	int    padding = 0;

	assert ( strcmp ( options[paddingDiff].optName, "-padding" ) == 0 );
	assert ( strcmp ( options[fillDiff].optName, "-fill" ) == 0 );
	assert ( strcmp ( options[expandDiff].optName, "-expand" ) == 0 );
	assert ( strcmp ( options[alignDiff].optName, "-align" ) == 0 );

	if ( Tcl_ListObjLength ( interp, children, &noChildren ) != TCL_OK )
	{
		Tcl_SetResult ( interp, "widget-list must be proper list", TCL_STATIC );
		return TCL_ERROR;
	}

	if ( options[paddingDiff].status == GNOCL_STATUS_CHANGED )
	{
		if ( gnoclGetPadding ( interp, options[paddingDiff].val.obj, &padding ) != TCL_OK )
		{
			return TCL_ERROR;
		}
	}

	if ( options[expandDiff].status == GNOCL_STATUS_CHANGED )
	{
		expand = options[expandDiff].val.b;
	}

	if ( options[fillDiff].status == GNOCL_STATUS_CHANGED )
	{
		if ( gnoclGet2Double ( interp, options[fillDiff].val.obj, &xFill, &yFill ) != TCL_OK )
		{
			return TCL_ERROR;
		}

		if ( xFill < .0 || yFill < .0 || xFill > 1. || yFill > 1. )
		{
			Tcl_SetResult ( interp, "Options \"fill\" must be between 0 and 1", TCL_STATIC );
			{
				return TCL_ERROR;
			}
		}
	}

	if ( options[alignDiff].status == GNOCL_STATUS_CHANGED )
	{
		if ( gnoclGetBothAlign ( interp, options[alignDiff].val.obj, &xAlign, &yAlign ) != TCL_OK )
		{
			return TCL_ERROR;
		}
	}

	/* we use GtkAlignment only if absolut necessary */

	if ( expand == 1 && xFill == 1. && yFill == 1. )
	{
		fill = 1;
	}

	else if ( isHorizontal &&  ! ( ( expand == 0 && yFill == 1. ) || ( expand == 1 && yFill == 1. && xFill == 0. && xAlign == .5 ) ) )
	{
		needAlign = 1.;
		fill = expand;
	}

	else if ( !isHorizontal &&  ! ( ( expand == 0 && xFill == 1. ) || ( expand == 1 && xFill == 1. && yFill == 0. && yAlign == .5 ) ) )
	{
		needAlign = 1.;
		fill = expand;
	}

	/* printf( "box: %salign\n", needAlign ? "" : "No " ); */

	for ( n = 0; n < noChildren; ++n )
	{
		GtkWidget *childWidget;
		Tcl_Obj *tp;

		if ( Tcl_ListObjIndex ( interp, children, n, &tp ) != TCL_OK )
		{
			return TCL_ERROR;
		}

		childWidget = gnoclChildNotPacked ( Tcl_GetString ( tp ), interp );

		if ( childWidget == NULL )
		{
			return TCL_ERROR;
		}

		if ( needAlign )
		{
			GtkWidget *alignment = gtk_alignment_new ( xAlign, yAlign, xFill, yFill );
			/* alignment is deleted on deletion of childWidget
			   only necessary, if not whole box is destroyed */
			g_signal_connect ( G_OBJECT ( childWidget ), "destroy", G_CALLBACK ( alignDestroyFunc ), alignment );
			gtk_widget_show ( alignment );
			gtk_container_add ( GTK_CONTAINER ( alignment ), childWidget );
			childWidget = alignment;
		}

		if ( begin )
		{
			gtk_box_pack_start ( box, childWidget, expand, fill, padding );
		}

		else
		{
			gtk_box_pack_end ( box, childWidget, expand, fill, padding );
		}
	}

	return TCL_OK;
}

/**
\brief
\note
**/
static int optLayout ( Tcl_Interp *interp, GnoclOption *opt,  GObject *obj, Tcl_Obj **ret )
{
	static const char *txt[] =
	{
		"default", "spread",
		"edge", "start",
		"end", NULL
	};

	int types[] = { GTK_BUTTONBOX_DEFAULT_STYLE, GTK_BUTTONBOX_SPREAD,
					GTK_BUTTONBOX_EDGE, GTK_BUTTONBOX_START,
					GTK_BUTTONBOX_END
				  };

	assert ( sizeof ( GTK_BUTTONBOX_DEFAULT_STYLE ) == sizeof ( int ) );

	return gnoclOptGeneric ( interp, opt, obj, "button box layout", txt, types, ret );
}

/**
\brief	Parses list of set options and determine if a frame in neccessary.
**/
static int needFrame ( const GnoclOption options[] )
{
	int k;

	for ( k = startFrameOpts; k < startCommonOpts; ++k )
	{
		if ( options[k].status == GNOCL_STATUS_CHANGED )
		{
			return 1;
		}
	}

	return 0;
}

/**
\brief
\note	Not used?.
**/
static void removeChild ( GtkWidget *widget, gpointer data )
{
	GtkBox *box = GTK_BOX ( data );
	/* FIXME: is this correct? where to put the g_object_unref? */
	g_object_ref ( widget );
	gtk_container_remove ( GTK_CONTAINER ( box ), widget );

	if ( GTK_IS_ALIGNMENT ( widget ) )
	{
		GtkWidget *child = gtk_bin_get_child ( GTK_BIN ( widget ) );
		g_object_ref ( child );
		gtk_container_remove ( GTK_CONTAINER ( widget ), child );
		g_object_unref ( widget );
	}
}

/**
\brief
\note FIXME: create or delete frame if necessary also change: gnoclMemNameAndWidget( name, widget );
**/

static int configure ( Tcl_Interp *interp, GtkFrame *frame, GtkBox *box, GnoclOption options[] )
{
	GtkWidget *widget = frame ? GTK_WIDGET ( frame ) : GTK_WIDGET ( box );

	if ( options[layoutIdx].status == GNOCL_STATUS_CHANGED && !GTK_CHECK_TYPE ( box, GTK_TYPE_BUTTON_BOX ) )
	{
		Tcl_SetResult ( interp, "Option \"layout\" only valid for box of buttonType", TCL_STATIC );
		return TCL_ERROR;
	}

	if ( frame == NULL && needFrame ( options ) )
	{
		Tcl_SetResult ( interp, "Frame options can only be set if a "
						"frame option is given on creation (for example -shadow none).", TCL_STATIC );
		return TCL_ERROR;
	}


	if ( frame != NULL )
	{
		if ( gnoclSetOptions ( interp, options + startFrameOpts, G_OBJECT ( frame ), startCommonOpts - startFrameOpts ) != TCL_OK )
		{
			return TCL_ERROR;
		}
	}

	if ( gnoclSetOptions ( interp, options, G_OBJECT ( box ), startFrameOpts ) != TCL_OK )
	{
		return TCL_ERROR;
	}

	if ( gnoclSetOptions ( interp, options + startCommonOpts, G_OBJECT ( widget ), startPackOpts - startCommonOpts ) != TCL_OK )
	{
		return TCL_ERROR;
	}

	if ( options[childrenIdx].status == GNOCL_STATUS_CHANGED )
	{
		gtk_container_foreach ( GTK_CONTAINER ( box ), removeChild, box );

		if ( addChildren ( box, interp, options[childrenIdx].val.obj, options + startPackOpts, 1 ) != TCL_OK )
		{
			return TCL_ERROR;
		}
	}

	return TCL_OK;
}

/**
\brief
**/
static int boxFuncAdd ( GtkBox *box, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[], int begin )
{
	int ret = TCL_ERROR;

	if ( objc < 3 )
	{
		Tcl_WrongNumArgs ( interp, 2, objv, "widget-list ?option val ...?" );
		return TCL_ERROR;
	}

	if ( gnoclParseOptions ( interp, objc - 2, objv + 2, boxOptions + startPackOpts ) == TCL_OK )
	{
		ret = addChildren ( box, interp, objv[2], boxOptions + startPackOpts, begin );
	}

	gnoclClearOptions ( boxOptions + startPackOpts );

	return ret;
}

/**
\brief Function called by the interpreter in response to widget command.
**/
int boxFunc ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{
	static const char *cmds[] =
	{
		"cget", "delete", "configure", "add",
		"addBegin", "addStart", "addEnd", "class", "remove",
		"reorder",
		NULL
	};

	enum cmdIdx
	{
		CgetIdx, DeleteIdx, ConfigureIdx, AddIdx,
		BeginIdx, StartIdx, EndIdx, ClassIdx, RemoveIdx,
		ReorderIdx
	};

	int idx;

	GtkWidget *widget = GTK_WIDGET ( data );
	GtkFrame *frame;
	GtkBox   *box;

	if ( GTK_IS_FRAME ( widget ) )
	{
		frame = GTK_FRAME ( widget );
		box = GTK_BOX ( gtk_bin_get_child ( GTK_BIN ( frame ) ) );
	}

	else
	{
		frame = NULL;
		box = GTK_BOX ( widget );
	}

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
		/* may require implementation of error checking */
		case RemoveIdx:
			{
			
				GtkWidget *child = gnoclGetWidgetFromName ( Tcl_GetString ( objv[2] ), interp );
			
				if (child==NULL) {
					
					char msg[125];
					sprintf(msg,"WARNING: No such widget %s.", Tcl_GetString ( objv[2] ));
					Tcl_SetResult ( interp, msg, TCL_STATIC );					
					break;
				}
			
				g_object_ref ( child );
				
				/* possible error checking using */
				// GList * gtk_container_get_children (GtkContainer *container);
				
				gtk_container_remove ( box, child ) ;
			}
			break;
		case CgetIdx:
			{
				int     idx;

				switch ( gnoclCget ( interp, objc, objv,  widget, boxOptions, &idx ) )
				{
					case GNOCL_CGET_ERROR: {
						return TCL_ERROR; }
					case GNOCL_CGET_HANDLED: {
						return TCL_OK; }
					case GNOCL_CGET_NOTHANDLED: {
						return cget ( interp, widget, boxOptions, idx );}
				}
			}
			break;

		case ClassIdx:
			{
				Tcl_SetObjResult ( interp, Tcl_NewStringObj ( "box", -1 ) );
			}
			break;

		case DeleteIdx:
			{
				return gnoclDelete ( interp, widget, objc, objv );
			}
		case ConfigureIdx:
			{
				int ret = TCL_ERROR;

				if ( gnoclParseOptions ( interp, objc - 1, objv + 1, boxOptions )
						== TCL_OK )
				{
					if ( boxOptions[orientationIdx].status == GNOCL_STATUS_CHANGED || boxOptions[buttonTypeIdx].status == GNOCL_STATUS_CHANGED )
					{
						Tcl_SetResult ( interp, "Option \"-orientation\" and "
										"\"-buttonType\" can only set on creation.",
										TCL_STATIC );
						ret = TCL_ERROR;
					}

					else
						ret = configure ( interp, frame, box, boxOptions );
				}

				gnoclClearOptions ( boxOptions );

				return ret;
			}

			break;
		case StartIdx:
		case BeginIdx:
			{
				gint ret;
				/* increase reference count of object to prevent loss on possible call by remove */
				g_object_ref ( G_OBJECT ( gnoclGetWidgetFromName ( Tcl_GetString ( objv[2] ), interp ) ) );
				ret = boxFuncAdd ( box, interp, objc, objv, idx != EndIdx );
				gtk_box_reorder_child  ( box, gnoclGetWidgetFromName ( Tcl_GetString ( objv[2] ), interp ), 0 );
				return ret;
			}
			break;
		case AddIdx:
		case EndIdx:
			{
				gint ret;
				/* increase reference count of object to prevent loss on possible call by remove */
				g_object_ref ( G_OBJECT ( gnoclGetWidgetFromName ( Tcl_GetString ( objv[2] ), interp ) ) );
				ret = boxFuncAdd ( box, interp, objc, objv, idx != EndIdx );
				return ret;
			} break;
		case ReorderIdx:
			{
				gint pos;
				Tcl_GetIntFromObj ( interp, objv[3], &pos );
				gtk_box_reorder_child  ( box, gnoclGetWidgetFromName ( Tcl_GetString ( objv[2] ), interp ), pos );

			}
			break;
		default: {}
	}

	return TCL_OK;
}

/**
\brief
\todo   Modify code to specify default padding option for the new box.
**/
int gnoclBoxCmd ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{
	GtkOrientation orient = GTK_ORIENTATION_HORIZONTAL;
	int            isButtonType = 0;
	int            ret = TCL_OK;
	GtkBox         *box;
	GtkFrame       *frame = NULL;
	GtkWidget      *widget;

	assert ( strcmp ( boxOptions[startFrameOpts].optName, "-label" ) == 0 );
	assert ( strcmp ( boxOptions[startCommonOpts].optName, "-name" ) == 0 );
	assert ( strcmp ( boxOptions[startPackOpts+paddingDiff].optName, "-padding" ) == 0 );
	assert ( strcmp ( boxOptions[startPackOpts+fillDiff].optName, "-fill" ) == 0 );
	assert ( strcmp ( boxOptions[startPackOpts+expandDiff].optName, "-expand" ) == 0 );
	assert ( strcmp ( boxOptions[startPackOpts+alignDiff].optName, "-align" ) == 0 );

	if ( gnoclParseOptions ( interp, objc, objv, boxOptions ) != TCL_OK )
	{
		gnoclClearOptions ( boxOptions );
		return TCL_ERROR;
	}

	if ( boxOptions[orientationIdx].status == GNOCL_STATUS_CHANGED )
	{
		if ( gnoclGetOrientationType ( interp, boxOptions[orientationIdx].val.obj, &orient ) != TCL_OK )
		{
			gnoclClearOptions ( boxOptions );
			return TCL_ERROR;
		}
	}

	if ( boxOptions[buttonTypeIdx].status == GNOCL_STATUS_CHANGED )
	{
		isButtonType = boxOptions[buttonTypeIdx].val.b;
	}

	if ( orient == GTK_ORIENTATION_HORIZONTAL )
	{
		box = GTK_BOX ( isButtonType ? gtk_hbutton_box_new( ) : gtk_hbox_new ( 0, GNOCL_PAD_TINY ) );
	}

	else
	{
		box = GTK_BOX ( isButtonType ?	gtk_vbutton_box_new( ) : gtk_vbox_new ( 0, GNOCL_PAD_TINY ) );
	}

	/* set default value */
	gtk_container_set_border_width ( GTK_CONTAINER ( box ), GNOCL_PAD_TINY );

	if ( needFrame ( boxOptions ) )
	{
		frame = GTK_FRAME ( gtk_frame_new ( NULL ) );
		gtk_container_add ( GTK_CONTAINER ( frame ), GTK_WIDGET ( box ) );
		widget = GTK_WIDGET ( frame );
	}

	else
	{
		widget = GTK_WIDGET ( box );
	}

	ret = configure ( interp, frame, box, boxOptions );

	gnoclClearOptions ( boxOptions );

	if ( ret != TCL_OK )
	{
		gtk_widget_destroy ( widget );
		return TCL_ERROR;
	}

	gtk_widget_show_all ( widget );

	return gnoclRegisterWidget ( interp, widget, boxFunc );
}

/**
\brief
\todo   Modify code to specify default padding option for the new box.
**/
int gnoclVBoxCmd ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{
	GtkOrientation orient = GTK_ORIENTATION_HORIZONTAL;
	int            isButtonType = 0;
	int            ret = TCL_OK;
	GtkBox         *box;
	GtkFrame       *frame = NULL;
	GtkWidget      *widget;

	assert ( strcmp ( boxOptions[startFrameOpts].optName, "-label" ) == 0 );
	assert ( strcmp ( boxOptions[startCommonOpts].optName, "-name" ) == 0 );
	assert ( strcmp ( boxOptions[startPackOpts+paddingDiff].optName, "-padding" ) == 0 );
	assert ( strcmp ( boxOptions[startPackOpts+fillDiff].optName, "-fill" ) == 0 );
	assert ( strcmp ( boxOptions[startPackOpts+expandDiff].optName, "-expand" ) == 0 );
	assert ( strcmp ( boxOptions[startPackOpts+alignDiff].optName, "-align" ) == 0 );

	if ( gnoclParseOptions ( interp, objc, objv, boxOptions ) != TCL_OK )
	{
		gnoclClearOptions ( boxOptions );
		return TCL_ERROR;
	}

	if ( boxOptions[orientationIdx].status == GNOCL_STATUS_CHANGED )
	{
		Tcl_SetResult ( interp, "ERROR: The \"-orientation\" switch in invalid for this object.", TCL_STATIC );
		return TCL_ERROR;

	}

	if ( boxOptions[buttonTypeIdx].status == GNOCL_STATUS_CHANGED )
	{
		isButtonType = boxOptions[buttonTypeIdx].val.b;
	}

	box = GTK_BOX ( isButtonType ?	gtk_vbutton_box_new( ) : gtk_vbox_new ( 0, GNOCL_PAD_TINY ) );


	/* set default value */
	gtk_container_set_border_width ( GTK_CONTAINER ( box ), GNOCL_PAD_TINY );

	if ( needFrame ( boxOptions ) )
	{
		frame = GTK_FRAME ( gtk_frame_new ( NULL ) );
		gtk_container_add ( GTK_CONTAINER ( frame ), GTK_WIDGET ( box ) );
		widget = GTK_WIDGET ( frame );
	}

	else
	{
		widget = GTK_WIDGET ( box );
	}

	ret = configure ( interp, frame, box, boxOptions );

	gnoclClearOptions ( boxOptions );

	if ( ret != TCL_OK )
	{
		gtk_widget_destroy ( widget );
		return TCL_ERROR;
	}

	gtk_widget_show_all ( widget );

	return gnoclRegisterWidget ( interp, widget, boxFunc );
}


/**
\brief
\todo   Modify code to specify default padding option for the new box.
**/
int gnoclHBoxCmd ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{
	GtkOrientation orient = GTK_ORIENTATION_HORIZONTAL;
	int            isButtonType = 0;
	int            ret = TCL_OK;
	GtkBox         *box;
	GtkFrame       *frame = NULL;
	GtkWidget      *widget;

	assert ( strcmp ( boxOptions[startFrameOpts].optName, "-label" ) == 0 );
	assert ( strcmp ( boxOptions[startCommonOpts].optName, "-name" ) == 0 );
	assert ( strcmp ( boxOptions[startPackOpts+paddingDiff].optName, "-padding" ) == 0 );
	assert ( strcmp ( boxOptions[startPackOpts+fillDiff].optName, "-fill" ) == 0 );
	assert ( strcmp ( boxOptions[startPackOpts+expandDiff].optName, "-expand" ) == 0 );
	assert ( strcmp ( boxOptions[startPackOpts+alignDiff].optName, "-align" ) == 0 );

	if ( gnoclParseOptions ( interp, objc, objv, boxOptions ) != TCL_OK )
	{
		gnoclClearOptions ( boxOptions );
		return TCL_ERROR;
	}

	if ( boxOptions[orientationIdx].status == GNOCL_STATUS_CHANGED )
	{
		Tcl_SetResult ( interp, "ERROR: The \"-orientation\" switch in invalid for this object.", TCL_STATIC );
		return TCL_ERROR;
	}

	if ( boxOptions[buttonTypeIdx].status == GNOCL_STATUS_CHANGED )
	{
		isButtonType = boxOptions[buttonTypeIdx].val.b;
	}

	box = GTK_BOX ( isButtonType ? gtk_hbutton_box_new( ) : gtk_hbox_new ( 0, GNOCL_PAD_TINY ) );


	/* set default value */
	gtk_container_set_border_width ( GTK_CONTAINER ( box ), GNOCL_PAD_TINY );

	if ( needFrame ( boxOptions ) )
	{
		frame = GTK_FRAME ( gtk_frame_new ( NULL ) );
		gtk_container_add ( GTK_CONTAINER ( frame ), GTK_WIDGET ( box ) );
		widget = GTK_WIDGET ( frame );
	}

	else
	{
		widget = GTK_WIDGET ( box );
	}

	ret = configure ( interp, frame, box, boxOptions );

	gnoclClearOptions ( boxOptions );

	if ( ret != TCL_OK )
	{
		gtk_widget_destroy ( widget );
		return TCL_ERROR;
	}

	gtk_widget_show_all ( widget );

	return gnoclRegisterWidget ( interp, widget, boxFunc );
}

