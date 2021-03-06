/* arrowButton.c
authors	Peter G. Baum, William J Giddings
date	2001-03:
*/

/**
\page page_arrowButton gnocl::arrowButton
\htmlinclude arrowButton.html
\note The "-arrow" and "-shadow" options do not map to any widget property and are specified as GNOCL_STRINGS.
	These values will be read prior to creation of an instance the arrowButton object. The remaining properties are
	then passed for the configuration of the arrow's container, a button. These values are passed to comply with the
	gnocl format, otherwise custom buttons would need to be implement for each direction.
**/

/**
\par Modification History
\verbatim
	2013-07:	added commands, options, commands
	2009-10:    added -width -height -size
	2008-10:    added command, class
	2008-08:	began development
\endverbatim
**/

#include "gnocl.h"

static GnoclOption arrowOptions[] =
{
	{ "-arrow", GNOCL_STRING, "arrow"},
	{ "-shadow", GNOCL_STRING, "shadow"},
	{ "-size", GNOCL_DOUBLE, "arrow-scaling"},

	{ "-activeBackgroundColor", GNOCL_OBJ, "active", gnoclOptGdkColorBg },
	{ "-data", GNOCL_OBJ, "", gnoclOptData },
	{ "-hasFocus", GNOCL_BOOL, "has-focus" },
	{ "-heightGroup", GNOCL_OBJ, "h", gnoclOptSizeGroup },
	{ "-name", GNOCL_STRING, "name" },
	{ "-normalBackgroundColor", GNOCL_OBJ, "normal", gnoclOptGdkColorBg },
	{ "-onClicked", GNOCL_OBJ, "clicked", gnoclOptCommand },
	{ "-onRealize", GNOCL_OBJ, "realize", gnoclOptCommand },
	{ "-onShowHelp", GNOCL_OBJ, "", gnoclOptOnShowHelp },
	{ "-prelightBackgroundColor", GNOCL_OBJ, "prelight", gnoclOptGdkColorBg },
	{ "-relief", GNOCL_OBJ, "relief", gnoclOptRelief },
	{ "-sensitive", GNOCL_BOOL, "sensitive" },
	{ "-sizeGroup", GNOCL_OBJ, "s", gnoclOptSizeGroup },
	{ "-tooltip", GNOCL_OBJ, "", gnoclOptTooltip },
	{ "-visible", GNOCL_BOOL, "visible" },
	{ "-widthGroup", GNOCL_OBJ, "w", gnoclOptSizeGroup },

	{ "-width", GNOCL_INT, "width-request" },     /* these must be */
	{ "-height", GNOCL_INT, "height-request" },   /* before -visible! */

	{ NULL },
};

static const int textIdx = 0;
static const int iconsIdx = 1;

/**
\brief
**/
static int configure ( Tcl_Interp *interp, GtkWidget *widget, GnoclOption options[] )
{

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
int gnoclConfigArrowButton ( Tcl_Interp *interp, GtkWidget *widget, Tcl_Obj *txtObj )
{
#ifdef DEBUG_ARROWBUTTON
	g_print ( "%s\n", __FUNCTION__, );
#endif

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
#ifdef DEBUG_ARROWBUTTON
	g_print ( "%s %s \n", __FUNCTION__, options[1] );
#endif

	const char *dataIDa = "gnocl::data1";
	const char *dataIDb = "gnocl::data2";

	GtkWidget *arrow = gnoclFindChild ( GTK_WIDGET ( widget ), GTK_TYPE_BUTTON );
	char *str;

	/* this will enable us to obtain widget data for the arrow object */
	char *result = g_object_get_data ( G_OBJECT ( arrow ), dataIDa );


	result = g_object_get_data ( G_OBJECT ( arrow ), dataIDb );


	str = gnoclGetNameFromWidget ( arrow );

	/*-----*/

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

	return gnoclCgetNotImplemented ( interp, options + idx );
}

static const char *cmds[] = { "delete", "configure", "cget", "onClicked", "class", NULL };

/**
\brief
**/
int arrowButtonFunc ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{
#ifdef DEBUG_ARROWBUTTON
	g_print ( "%s\n", __FUNCTION__, );
#endif


	enum cmdIdx { DeleteIdx, ConfigureIdx, CgetIdx, OnClickedIdx, ClassIdx };

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

				ret = configure ( interp, widget, arrowOptions );

				if ( 1 )
				{
					if ( gnoclParseAndSetOptions ( interp, objc - 1, objv + 1, arrowOptions, G_OBJECT ( widget ) ) == TCL_OK )
					{
						ret = configure ( interp, widget, arrowOptions );
					}
				}

				gnoclClearOptions ( arrowOptions );

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
				int     idx;

				switch ( gnoclCget ( interp, objc, objv, G_OBJECT ( widget ), arrowOptions, &idx ) )
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
							return cget ( interp, widget, arrowOptions, idx );
						}
				}
			}
	}

	return TCL_OK;
}

/**
\brief
**/
int gnoclArrowButtonCmd ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{
	if ( gnoclGetCmdsAndOpts ( interp, cmds, arrowOptions, objv, objc ) == TCL_OK )
	{
		return TCL_OK;
	}

	int pos = 0;

	int       ret;
	GtkWidget *widget;
	GtkWidget *arrow;

	/* STEP 1)  -check validity of switches */
	/* there are two objects here, the button and the arrow itself
	 * the arrow and shadow options are for the arrow widget and not the button
	 */
	int k;
	char *arrow_type = 1;       /* define default value, ie. 1 = down */
	char *shadow_type = 1;      /* define default value, ie. 1 = in */
	float scaling = 0.7;        /* Gtk Api docs say that this is the default size */


	/* to prevent warnings, these values need to be stripped from the list passed on to the configuration satge */

	for ( k = 1; k < objc; k += 2 )
	{
		int idx;

		if ( gnoclGetIndexFromObjStruct ( interp, objv[k], ( char ** ) &arrowOptions[0].optName, sizeof ( GnoclOption ), "option", TCL_EXACT, &idx ) != TCL_OK )
		{
			return -1;
		}

		if ( strcmp ( Tcl_GetString ( objv[k] ), "-arrow" ) == 0 )
		{
			arrow_type = Tcl_GetString ( objv[k+1] );
		}

		if ( strcmp ( Tcl_GetString ( objv[k] ), "-shadow" ) == 0 )
		{
			shadow_type = Tcl_GetString ( objv[k+1] );
		}

		if ( strcmp ( Tcl_GetString ( objv[k] ), "-scale" ) == 0 )
		{
			scaling = atof ( Tcl_GetString ( objv[k+1] ) );
		}


	}

	if ( gnoclParseOptions ( interp, objc, objv, arrowOptions ) != TCL_OK )
	{
		gnoclClearOptions ( arrowOptions );
		return TCL_ERROR;
	}

	static const char *cmds[] = { "up", "down", "left", "right", "none", NULL };
	enum cmdIdx { UpIdx, DownIdx, LeftIdx, RightIdx, NoneIdx };
	int idx;

	for ( idx = 0; idx <= 3 ; idx++ )
	{

		if ( strcmp ( arrow_type, cmds[idx] ) == 0 )
		{
			break;
		}
	}

	/* STEP 2)  -create widget, configure it, then clear memory */
	widget = gtk_button_new ();

	arrow = gtk_arrow_new ( idx , GTK_SHADOW_ETCHED_IN );

	gtk_container_add ( GTK_CONTAINER ( widget ), arrow );

	gtk_widget_show ( GTK_WIDGET ( widget ) );

	gtk_widget_show ( arrow );

	ret = gnoclSetOptions ( interp, arrowOptions, G_OBJECT ( widget ), -1 );

	if ( ret == TCL_OK )
	{
		ret = configure ( interp, G_OBJECT ( widget ), arrowOptions );
	}

	gnoclClearOptions ( arrowOptions );

	/* STEP 3)  -show the widget */

	if ( ret != TCL_OK )
	{
		gtk_widget_destroy ( GTK_WIDGET ( widget ) );
		return TCL_ERROR;
	}

	/* STEP 4)  -everything has worked, register the widget with the Tcl interpretor. */
	return gnoclRegisterWidget ( interp, GTK_WIDGET ( widget ), arrowButtonFunc );
}
