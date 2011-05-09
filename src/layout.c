/**
 \brief         This module implements the gnocl::layout widget.
 \authors       William J Giddings
 \date          20/05/09
*/

/**
\page page_infoBar gnocl::layout
\htmlinclude layout.html
**/


#include "gnocl.h"
#include <string.h>
#include <assert.h>

/**
\brief
**/
static int gnoclOptMetric ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{

	const char *txt[] = { "pixels", "inches", "centimeters", NULL };
	const int types[] = {  GTK_PIXELS, GTK_INCHES, GTK_CENTIMETERS };

	assert ( sizeof ( GTK_PIXELS ) == sizeof ( int ) );

	return gnoclOptGeneric ( interp, opt, obj, "metric", txt, types, ret );
}



/**
\brief
**/
static void  doOnSetScrollAdjustment ( GtkLayout *horizontal, GtkAdjustment *vertical, GtkAdjustment *arg2, gpointer user_data )
{
	GnoclCommandData *cs = ( GnoclCommandData * ) user_data;

	//if ( *cs->interp->result == '\0' )
	//{
	GnoclPercSubst ps[] =
	{
		{ 'w', GNOCL_STRING },  /* printiconview */
		{ 0 }
	};

	ps[0].val.str = gnoclGetNameFromWidget ( horizontal );
	gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 );
	//}
}

/**
\brief      This is for all callbacks which have only %w as substitution
**/
static int gnoclOponOnSetScrollAdjustment ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	assert ( opt->type == GNOCL_OBJ );

	return gnoclConnectOptCmd ( interp, obj, opt->propName, G_CALLBACK ( doOnSetScrollAdjustment ), opt, NULL, ret );
}


/*
  properties
  "hadjustment"              GtkAdjustment*        : Read / Write
  "height"                   guint                 : Read / Write
  "vadjustment"              GtkAdjustment*        : Read / Write
  "width"                    guint                 : Read / Write

  signals
  "set-scroll-adjustments"                         : Run Last / Action

*/

static const int hAdjustmentIdx  = 0;
static const int vAdjustmentIdx  = 1;

static GnoclOption layoutOptions[] =
{
	/* options based upon widget properties */
	{ "-hadjustment", GNOCL_OBJ, NULL },
	{ "-vadjustment", GNOCL_OBJ, NULL},
	{ "-width", GNOCL_INT, "width"},
	{ "-height", GNOCL_INT, "height"},

	/* custom options */
	{ "-onSetScrollAdjustment", GNOCL_OBJ, "set-scroll-adjustments", gnoclOponOnSetScrollAdjustment},

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
static int addChildren ( GtkBox *box, Tcl_Interp *interp, Tcl_Obj *children, GnoclOption *options, int begin )
{
	gint n, noChildren;
	gint x, y;
	gfloat xAlign = 0.5, yAlign = 0.5;  /* center */

	//assert ( strcmp ( options[xIdx].optName, "-x" ) == 0 );
	//assert ( strcmp ( options[yIdx].optName, "-y" ) == 0 );


	Tcl_ListObjLength ( interp, children, &noChildren );

	g_print ( "total = %d\n", noChildren );


	/*
		if ( options[xIdx].status == GNOCL_STATUS_CHANGED )
		{
			x = options[xIdx].val.i;
		}

		if ( options[yIdx].status == GNOCL_STATUS_CHANGED )
		{
			y = options[yIdx].val.i;
		}

		//printf( "box: %salign\n", needAlign ? "" : "No " );
	*/

	for ( n = 0; n < noChildren; ++n )
	{
		GtkWidget *childWidget;
		Tcl_Obj *tp;

	}

	return TCL_OK;
}


/**
\brief
**/
static int configure ( Tcl_Interp *interp, GtkLayout *layout, GnoclOption options[] )
{
#ifdef DEBUG_LAYOUT
	g_print ( "CONFIGURE \n" );
#endif

	/*
		if ( options[childrenIdx].status == GNOCL_STATUS_CHANGED )
		{
			g_print ( "CONFIGURE --> children\n" );

			addChildren ( widget, interp, options[childrenIdx].val.obj, options, 1 );

		}
	*/

	return TCL_OK;
}

/**
\brief
**/
int gnoclConfigLayout ( Tcl_Interp *interp, GtkWidget *widget, Tcl_Obj *txtObj )
{
	printf ( "gnoclConfigArrowButton\n" );

	GtkArrow *arrow;

	/* the arrow is the child of the widget */

	arrow = gnoclFindChild ( GTK_WIDGET ( widget ), GTK_TYPE_ARROW ) ;
	gtk_arrow_set ( arrow, GTK_ARROW_DOWN, GTK_SHADOW_OUT );

	return TCL_OK;
}

/**
\brief
**/
static int cget ( Tcl_Interp *interp, GtkWidget *widget, GnoclOption options[], int idx )
{
	printf ( "cget\n" );

	const char *dataIDa = "gnocl::data1";
	const char *dataIDb = "gnocl::data2";

	GtkWidget *arrow = gnoclFindChild ( GTK_WIDGET ( widget ), GTK_TYPE_BUTTON );
	char *str;

	/* this will enable us to obtain widget data for the arrow object */
	char *result = g_object_get_data ( G_OBJECT ( arrow ), dataIDa );
	printf ( "cget result = %s\n", result );

	result = g_object_get_data ( G_OBJECT ( arrow ), dataIDb );
	printf ( "cget result = %s\n", result );

	str = gnoclGetNameFromWidget ( arrow );
	printf ( "configure %s\n", str );


	/*-----*/
	/*
	    Tcl_Obj *obj = NULL;

	    if ( idx == textIdx )
	    {
	        obj = gnoclCgetButtonText ( interp, widget );
	    }

	    else if ( idx == iconsIdx )
	    {
	        GtkWidget *image = gnoclFindChild ( GTK_WIDGET ( widget ), GTK_TYPE_IMAGE );

	        if ( image == NULL )
	            obj = Tcl_NewStringObj ( "", 0 );
	        else
	        {
	            gchar   *st;
	            g_object_get ( G_OBJECT ( image ), "stock", &st, NULL );

	            if ( st )
	            {
	                obj = Tcl_NewStringObj ( "%#", 2 );
	                Tcl_AppendObjToObj ( obj, gnoclGtkToStockName ( st ) );
	                g_free ( st );
	            }

	            else
	            {
	                Tcl_SetResult ( interp, "Could not determine icon type.",
	                                TCL_STATIC );
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
\brief
**/
static int layoutFunc ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{
#ifdef DEBUG_LAYOUT
	listParameters ( objc, objv, "layoutFunc" );
#endif


	static const char *cmds[] =
	{
		"add", "move", "remove",
		"delete", "configure", "cget",
		"class",
		NULL
	};

	enum cmdIdx
	{
		AddIdx, MoveIdx, RemoveIdx,
		DeleteIdx, ConfigureIdx, CgetIdx,
		ClassIdx
	};

	GtkScrolledWindow   *scrolled = GTK_SCROLLED_WINDOW ( data );
	GtkLayout *layout = GTK_LAYOUT ( gtk_bin_get_child ( GTK_BIN ( scrolled ) ) );

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
		case RemoveIdx:
			{
				/* remove but not delete the child widget */
				GtkWidget *child;
				char *name;

				name = Tcl_GetString ( objv[2] );
				child = gnoclGetWidgetFromName ( name, interp );
				g_object_ref ( child );
				gtk_container_remove ( layout, child );

			}
			break;
		case MoveIdx:
		case AddIdx:
			{
				/* similar to code for the gnocl::fixed widget */
				/* place widget at position specified by -x / -y : default = 0,0 */
				GtkWidget *child;
				char *name;
				gint x;
				gint y;
				gint width;
				gint height;

				x = 0; y = 0;
				name = Tcl_GetString ( objv[2] );
				child = gnoclGetWidgetFromName ( name, interp );

				if ( GTK_IS_WIDGET ( child ) )
				{
				}
				else
				{
					Tcl_SetResult ( interp, "GNOCL ERROR: Object is not a valid widget.", TCL_STATIC );
					return TCL_ERROR;
				}

				/* add appropriate error message */
				if ( objc < 3 )
				{
					Tcl_WrongNumArgs ( interp, 1, objv, "add <widget-id> [optional -x <int> -y <int> -width <int> -height <int>]" );
					return TCL_ERROR;
				}

				name = Tcl_GetString ( objv[2] );

				static char *newOptions[] =
				{
					"-x", "-y", NULL
				};

				static enum  optsIdx
				{
					XIdx, YIdx,
				};

				gint i;
				gint j;
				gint idx2;
				gchar *opt;

				for ( i = 3; i < objc; i += 2 )
				{
					j = i + 1;

					opt = Tcl_GetString ( objv[i] );

					if ( Tcl_GetIndexFromObj ( interp, objv[i], newOptions, "command", TCL_EXACT, &idx2 ) != TCL_OK )
					{
						return TCL_ERROR;
					}

					switch ( idx2 )
					{
						case XIdx:
							{
								Tcl_GetIntFromObj ( NULL, objv[j], &x );
							} break;
						case YIdx:
							{
								Tcl_GetIntFromObj ( NULL, objv[j], &y );
							} break;

						default: {}
					}
				}

				/* get widget */
				child = gnoclGetWidgetFromName ( name, interp );

				/* check, then insert */
				if ( GTK_IS_WIDGET ( child ) )
				{

					if ( idx == AddIdx )
					{

						gtk_layout_put ( GTK_LAYOUT ( layout ), child, x, y );
					}

					else
					{
						gtk_layout_move ( GTK_LAYOUT ( layout ), child, x, y );
					}
				}

				else
				{
					Tcl_SetResult ( interp, "GNOCL ERROR: Object is not a valid widget.", TCL_STATIC );
					return TCL_ERROR;
				}

			}
			break;
		case ClassIdx:
			{
				Tcl_SetObjResult ( interp, Tcl_NewStringObj ( "arrowButton", -1 ) );
			}
			break;
		case DeleteIdx:
			{
				return gnoclDelete ( interp, GTK_WIDGET ( layout ), objc, objv );
			}
		case ConfigureIdx:
			{
				int ret = TCL_ERROR;

				ret = configure ( interp, layout, layoutOptions );

				if ( 1 )
				{
					if ( gnoclParseAndSetOptions ( interp, objc - 1, objv + 1, layoutOptions, G_OBJECT ( layout ) ) == TCL_OK )
					{
						ret = configure ( interp, layout, layoutOptions );
					}
				}

				gnoclClearOptions ( layoutOptions );

				return ret;
			}

			break;

		case CgetIdx:
			{
				int     idx;

				switch ( gnoclCget ( interp, objc, objv, G_OBJECT ( layout ), layoutOptions, &idx ) )
				{
					case GNOCL_CGET_ERROR:
						{
							return TCL_ERROR;
						}

					case GNOCL_CGET_HANDLED:
						{
							return TCL_OK;
						}

					case GNOCL_CGET_NOTHANDLED:
						{
							return cget ( interp, layout, layoutOptions, idx );
						}
				}
			}
	}

	return TCL_OK;
}

/**
\brief
**/
int gnoclLayoutCmd ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{

	int            ret = TCL_OK;
	GtkLayout      *layout;

	if ( gnoclParseOptions ( interp, objc, objv, layoutOptions ) != TCL_OK )
	{
		gnoclClearOptions ( layoutOptions );
		return TCL_ERROR;
	}

	GtkScrolledWindow *scrolledWindow = gtk_scrolled_window_new ( NULL, NULL );
	gtk_scrolled_window_set_policy ( scrolledWindow, GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC );
	gtk_scrolled_window_set_shadow_type ( scrolledWindow, GTK_SHADOW_IN );

	layout = gtk_layout_new ( NULL , NULL );

	gtk_container_add ( GTK_CONTAINER ( scrolledWindow ), layout );
	gtk_widget_show_all ( GTK_CONTAINER ( scrolledWindow ) );

	if ( gnoclParseOptions ( interp, objc, objv, layoutOptions ) != TCL_OK )
	{
		gnoclClearOptions ( layoutOptions );
		return TCL_ERROR;
	}

	/* STEP 3)  -show the widget */
	ret = gnoclSetOptions ( interp, layoutOptions, G_OBJECT ( layout ), -1 );

	if ( ret == TCL_OK )
	{
		ret = configure ( interp, layout , layoutOptions );
	}

	gnoclClearOptions ( layoutOptions );

	if ( ret != TCL_OK )
	{
		gtk_widget_destroy ( GTK_WIDGET ( scrolledWindow ) );
		return TCL_ERROR;
	}

	/* STEP 4)  -everything has worked, register the widget with the Tcl interpretor. */
	return gnoclRegisterWidget ( interp, GTK_WIDGET ( scrolledWindow ), layoutFunc );
}
