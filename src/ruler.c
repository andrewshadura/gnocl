/**
 \brief         This module implements the gnocl::ruler widget.
 \authors       William J Giddings
 \date          20/05/09
*/

/**
\par Modification History
\verbatim
	2013-07: added commands, options, commands
\endverbatim
**/

/**
\page page_infoBar gnocl::ruler
\htmlinclude ruler.html
**/

#include "gnocl.h"

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
static int gnoclOptRange ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{

	char *str = Tcl_GetString ( opt->val.str );
	GtkWidget *ruler = GTK_WIDGET ( obj );
	gdouble lower, upper, position, max_size;

	/* why isn't this parsing the string correctly? */
	sscanf ( str, "%f %f %f %f", &lower, &upper, &position, &max_size );

	/*
		g_print ( "%s %f %f %f %f\n", str, lower, upper, position, max_size );
	*/

	/* Paramters for the set range library call
	lower :     the lower limit of the ruler
	upper :     the upper limit of the ruler
	position :  the mark on the ruler
	max_size :  the maximum size of the ruler used when calculating the space to leave for the text
	*/

	gtk_ruler_set_range ( GTK_RULER ( ruler ), lower, upper, position, max_size );

	return TCL_OK;
}

/*
  "lower"                    gdouble               : Read / Write
  "max-size"                 gdouble               : Read / Write
  "metric"                   GtkMetricType         : Read / Write
  "position"                 gdouble               : Read / Write
  "upper"                    gdouble               : Read / Write
*/

static const int orientationIdx  = 0;

static GnoclOption rulerOptions[] =
{
	/* options based upon widget properties */
	{ "-orientation", GNOCL_OBJ, NULL },    /* 0 */

	{ "-lower", GNOCL_DOUBLE, "lower"},
	{ "-maxSize", GNOCL_DOUBLE, "max-size"},
	{ "-metric", GNOCL_OBJ, "metric", gnoclOptMetric},
	{ "-position", GNOCL_DOUBLE, "position"},
	{ "-upper", GNOCL_DOUBLE, "upper"},

	/* custom options */
	{ "-range", GNOCL_OBJ, "", gnoclOptRange},

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
#ifdef DEBUG_RULER
	g_print ( "CONFIGURE \n" );
#endif
	const char *dataIDa = "gnocl::data1";
	const char *dataIDb = "gnocl::data2";

	GtkWidget *arrow = gnoclFindChild ( GTK_WIDGET ( widget ), GTK_TYPE_BUTTON );
	char *str;

	/* this will enable us to obtain widget data for the arrow object */
	char *result = g_object_get_data ( G_OBJECT ( arrow ), dataIDa );


	result = g_object_get_data ( G_OBJECT ( arrow ), dataIDb );


	str = gnoclGetNameFromWidget ( arrow );

	return TCL_OK;
}

/**
\brief
**/
int gnoclConfigRuler ( Tcl_Interp *interp, GtkWidget *widget, Tcl_Obj *txtObj )
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

static const char *cmds[] =
{
	"delete", "configure",
	"cget",	"class",
	NULL
};

/**
\brief
**/
static int rulerFunc ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{



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

				ret = configure ( interp, widget, rulerOptions );

				if ( 1 )
				{
					if ( gnoclParseAndSetOptions ( interp, objc - 1, objv + 1, rulerOptions, G_OBJECT ( widget ) ) == TCL_OK )
					{
						ret = configure ( interp, widget, rulerOptions );
					}
				}

				gnoclClearOptions ( rulerOptions );

				return ret;
			}

			break;

		case CgetIdx:
			{
				int     idx;

				switch ( gnoclCget ( interp, objc, objv, G_OBJECT ( widget ), rulerOptions, &idx ) )
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
							return cget ( interp, widget, rulerOptions, idx );
						}
				}
			}
	}

	return TCL_OK;
}

/**
\brief
**/
int gnoclRulerCmd ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{

	if ( gnoclGetCmdsAndOpts ( interp, cmds, rulerOptions, objv, objc ) == TCL_OK )
	{
		return TCL_OK;
	}



	GtkOrientation orient = GTK_ORIENTATION_HORIZONTAL;
	int            ret = TCL_OK;
	GtkWidget      *ruler;

	if ( gnoclParseOptions ( interp, objc, objv, rulerOptions ) != TCL_OK )
	{
		gnoclClearOptions ( rulerOptions );
		return TCL_ERROR;
	}


	/* create the required type of ruler, it cannot be re-configured */
	if ( rulerOptions[orientationIdx].status == GNOCL_STATUS_CHANGED )
	{
		if ( gnoclGetOrientationType ( interp, rulerOptions[orientationIdx].val.obj, &orient ) != TCL_OK )
		{
			gnoclClearOptions ( rulerOptions );
			return TCL_ERROR;
		}
	}

	if ( orient == GTK_ORIENTATION_HORIZONTAL )
	{
		ruler = GTK_WIDGET ( gtk_hruler_new() );
	}

	else
	{
		ruler = GTK_WIDGET ( gtk_vruler_new() );
	}

	/* set some ruler default values */
	gtk_ruler_set_metric ( GTK_RULER ( ruler ), GTK_PIXELS );
	gtk_ruler_set_range ( GTK_RULER ( ruler ), 0, 100, 0, 20 );


	if ( gnoclParseOptions ( interp, objc, objv, rulerOptions ) != TCL_OK )
	{
		gnoclClearOptions ( rulerOptions );
		return TCL_ERROR;
	}

	/* STEP 3)  -show the widget */
	gtk_widget_show ( GTK_WIDGET ( ruler ) );

	ret = gnoclSetOptions ( interp, rulerOptions, G_OBJECT ( ruler ), -1 );

	if ( ret == TCL_OK )
	{
		ret = configure ( interp, G_OBJECT ( ruler ), rulerOptions );
	}

	gnoclClearOptions ( rulerOptions );

	if ( ret != TCL_OK )
	{
		gtk_widget_destroy ( GTK_WIDGET ( ruler ) );
		return TCL_ERROR;
	}

	/* STEP 4)  -everything has worked, register the widget with the Tcl interpretor. */
	return gnoclRegisterWidget ( interp, GTK_WIDGET ( ruler ), rulerFunc );
}
