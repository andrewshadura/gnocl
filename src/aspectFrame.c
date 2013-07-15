/**
 \brief         This module implements the gnocl::aspectFrame widget.
 \authors       William J Giddings
 \date          20/05/09
*/

/**
\par Modification History
\verbatim
	2013-07:	added commands, options, commands
\endverbatim
**/


/**
\page page_aspectFrame gnocl::aspectFrame
\htmlinclude aspectFrame.html
**/

#include "gnocl.h"

/**
\brief
\note	Taken from gtkaspectframe.c as this is not available via API.
**/

enum
{
	PROP_0,
	PROP_XALIGN,
	PROP_YALIGN,
	PROP_RATIO,
	PROP_OBEY_CHILD
};

static void gtk_aspect_frame_get_property ( GObject *object, guint prop_id, GValue *value, GParamSpec *pspec )
{
	GtkAspectFrame *aspect_frame = GTK_ASPECT_FRAME ( object );

	switch ( prop_id )
	{
		case PROP_XALIGN:
			g_value_set_float ( value, aspect_frame->xalign );
			break;
		case PROP_YALIGN:
			g_value_set_float ( value, aspect_frame->yalign );
			break;
		case PROP_RATIO:
			g_value_set_float ( value, aspect_frame->ratio );
			break;
		case PROP_OBEY_CHILD:
			g_value_set_boolean ( value, aspect_frame->obey_child );
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID ( object, prop_id, pspec );
			break;
	}
}

/*
  "obey-child"               gboolean              : Read / Write
  "ratio"                    gfloat                : Read / Write
  "xalign"                   gfloat                : Read / Write
  "yAlign"                   gfloat                : Read / Write

*/

static const int labelIdx = 0;

static GnoclOption aspectFrameOptions[] =
{
	{ "-label", GNOCL_STRING, NULL}, // there's no property for label
	{ "-obeyChild", GNOCL_BOOL, "obey-child"},
	{ "-ratio", GNOCL_DOUBLE, "ratio"},
	{ "-xalign", GNOCL_DOUBLE, "xalign"},
	{ "-yalign", GNOCL_DOUBLE, "yalign"},
	{ "-child", GNOCL_OBJ, "", gnoclOptChild },

	/* inherited options */
	{ "-tooltip", GNOCL_OBJ, "", gnoclOptTooltip },
	{ "-data", GNOCL_OBJ, "", gnoclOptData },
	{ "-name", GNOCL_STRING, "name" },
	{ "-onShowHelp", GNOCL_OBJ, "", gnoclOptOnShowHelp },
	{ "-cursor", GNOCL_OBJ, "", gnoclOptCursor },
	{ "-sensitive", GNOCL_BOOL, "sensitive" },
	{ "-visible", GNOCL_BOOL, "visible" },

	{ NULL },
};




/**
\brief
**/
static int configure ( Tcl_Interp *interp, GtkWidget *widget, GnoclOption options[] )
{
#ifdef DEBUG_ASPECTFRAME
	g_print ( "%s\n", __FUNCTION__, );
#endif


	if ( options[labelIdx].status == GNOCL_STATUS_CHANGED )
	{

		gtk_frame_set_label ( GTK_FRAME ( widget ), options[labelIdx].val.str );

	}

	return TCL_OK;
}

/**
\brief
**/
static int cget ( Tcl_Interp *interp, GtkWidget *widget, GnoclOption options[], int idx )
{
#ifdef DEBUG_ASPECTFRAME
	g_print ( "%s\n", __FUNCTION__, );
#endif


	GtkAspectFrame *aspect_frame = GTK_ASPECT_FRAME ( widget );

#ifdef DEBUG_ASPECTFRAME
	g_printf ( "xalign     = %f\n", aspect_frame->xalign );
	g_printf ( "yalign     = %f\n", aspect_frame->yalign );
	g_printf ( "ratio      = %f\n", aspect_frame->ratio );
	g_printf ( "obey-child = %d\n", aspect_frame->obey_child );
	g_printf ( "label      = %s\n", gtk_frame_get_label ( GTK_FRAME ( aspect_frame ) ) );
#endif

	return TCL_OK;

	return gnoclCgetNotImplemented ( interp, options + idx );
}

static const char *cmds[] =
{
	"delete", "configure",
	"cget", "onClicked",
	"class",
	NULL
};

/**
\brief
**/
static int aspectFrameFunc ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{
#ifdef DEBUG_ASPECTFRAME
	listParameters ( objc, objv, "aspectFrameFunc" );
#endif



	enum cmdIdx
	{
		DeleteIdx, ConfigureIdx,
		CgetIdx, OnClickedIdx,
		ClassIdx
	};

	GtkWidget *widget = GTK_WIDGET ( data );
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

		case ClassIdx:
			{
				Tcl_SetObjResult ( interp, Tcl_NewStringObj ( "arrowButton", -1 ) );
			}
			break;
		case DeleteIdx:
			{
				return gnoclDelete ( interp, GTK_WIDGET ( widget ), objc, objv );
			}

		case ConfigureIdx:
			{
				int ret = TCL_ERROR;

				ret = configure ( interp, widget, aspectFrameOptions );

				if ( 1 )
				{
					if ( gnoclParseAndSetOptions ( interp, objc - 1, objv + 1, aspectFrameOptions, G_OBJECT ( widget ) ) == TCL_OK )
					{
						ret = configure ( interp, widget, aspectFrameOptions );
					}
				}

				gnoclClearOptions ( aspectFrameOptions );

				return ret;
			}

			break;
		case OnClickedIdx:

			if ( objc != 2 )
			{
				Tcl_WrongNumArgs ( interp, 2, objv, NULL );
				return TCL_ERROR;
			}

			if ( GTK_WIDGET_IS_SENSITIVE ( GTK_WIDGET ( widget ) ) )
			{
				gtk_button_clicked ( widget );
			}

			break;

		case CgetIdx:
			{
				//return cget ( interp, widget, aspectFrameOptions, 1 );

				int idx;

				switch ( gnoclCget ( interp, objc, objv, G_OBJECT ( widget ), aspectFrameOptions, &idx ) )
				{
					case GNOCL_CGET_ERROR:
						return TCL_ERROR;
					case GNOCL_CGET_HANDLED:
						return TCL_OK;
					case GNOCL_CGET_NOTHANDLED:
						return cget ( interp, widget, aspectFrameOptions, &idx );
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
**/
int gnoclAspectFrameCmd ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{
	if ( gnoclGetCmdsAndOpts ( interp, cmds, aspectFrameOptions, objv, objc ) == TCL_OK )
	{
		return TCL_OK;
	}


	GtkOrientation orient = GTK_ORIENTATION_HORIZONTAL;
	int            ret = TCL_OK;
	GtkWidget      *aspectFrame;

	if ( gnoclParseOptions ( interp, objc, objv, aspectFrameOptions ) != TCL_OK )
	{
		gnoclClearOptions ( aspectFrameOptions );
		return TCL_ERROR;
	}



	/* create an empty frame */
	//gtk_aspect_frame_new (const gchar *label, gfloat xalign, gfloat yalign, gfloat ratio, gboolean obey_child);

	aspectFrame = gtk_aspect_frame_new ( "", 0, 0, 0, 1 );


	if ( gnoclParseOptions ( interp, objc, objv, aspectFrameOptions ) != TCL_OK )
	{
		gnoclClearOptions ( aspectFrameOptions );
		return TCL_ERROR;
	}

	/* STEP 3)  -show the widget */
	gtk_widget_show ( GTK_WIDGET ( aspectFrame ) );

	ret = gnoclSetOptions ( interp, aspectFrameOptions, G_OBJECT ( aspectFrame ), -1 );

	if ( ret == TCL_OK )
	{
		ret = configure ( interp, G_OBJECT ( aspectFrame ), aspectFrameOptions );
	}

	gnoclClearOptions ( aspectFrameOptions );

	if ( ret != TCL_OK )
	{
		gtk_widget_destroy ( GTK_WIDGET ( aspectFrame ) );
		return TCL_ERROR;
	}

	/* STEP 4)  -everything has worked, register the widget with the Tcl interpretor. */
	return gnoclRegisterWidget ( interp, GTK_WIDGET ( aspectFrame ), aspectFrameFunc );
}
