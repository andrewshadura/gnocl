/*
 * $Id: progressBar.c,v 1.7 2005/01/01 15:27:54 baum Exp $
 *
 * This file implements the progressBar widget
 *
 * Copyright (c) 2001 - 2005 Peter G. Baum  http://www.dr-baum.net
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

/*
   History:
   2008-10: added command, class
   2002-10: Begin of developement
 */

/**
\page page_progressBar gnocl::progressBar
\htmlinclude progressBar.html
**/

#include "gnocl.h"

/**
\brief
**/
static int optProgressBarOrientation ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	const char *txt[] =
	{
		"leftToRight", "rightToLeft",
		"bottomToTop", "topToBottom",
		NULL
	};

	int types[] =
	{
		GTK_PROGRESS_LEFT_TO_RIGHT,
		GTK_PROGRESS_RIGHT_TO_LEFT,
		GTK_PROGRESS_BOTTOM_TO_TOP,
		GTK_PROGRESS_TOP_TO_BOTTOM
	};

	assert ( sizeof ( GTK_PROGRESS_LEFT_TO_RIGHT ) == sizeof ( int ) );

	return gnoclOptGeneric ( interp, opt, obj, "progressBar style", txt, types, ret );
}

/**
\brief
**/
static const int visibleIdx = 0;
static const int hHeightIdx = 1;
static GnoclOption progressBarOptions[] =
{
	/* gnocl specific options */
	{ "-visible", GNOCL_INT, NULL },
	{ "-hHeight", GNOCL_INT, NULL },
	{ "-hWidth", GNOCL_INT, NULL },
	{ "-vHeight", GNOCL_INT, NULL },
	{ "-vWidth", GNOCL_INT, NULL },

	/* gtkProgress specific properties */
	{ "-activityMode", GNOCL_BOOL, "activity-mode" },
	{ "-fraction", GNOCL_DOUBLE, "fraction" },
	{ "-pulseStep", GNOCL_DOUBLE, "pulse-step" },
	{ "-orientation", GNOCL_OBJ, "orientation", optProgressBarOrientation },
	{ "-text", GNOCL_STRING, "text" },
	{ "-textAlign", GNOCL_OBJ, "text-?align", gnoclOptBothAlign },
	{ "-showText", GNOCL_BOOL, "show-text" },
	{ "-name", GNOCL_STRING, "name" },
	{ "-widthGroup", GNOCL_OBJ, "w", gnoclOptSizeGroup },
	{ "-heightGroup", GNOCL_OBJ, "h", gnoclOptSizeGroup },
	{ "-sizeGroup", GNOCL_OBJ, "s", gnoclOptSizeGroup },
	{ "-tooltip", GNOCL_OBJ, "", gnoclOptTooltip },
	{ "-data", GNOCL_OBJ, "", gnoclOptData },

	{ "-heightRequest", GNOCL_INT, "height-request" },
	{ "-widthRequest", GNOCL_INT, "width-request" },

	{ NULL }
};



/**
\brief      Modify the line pattern template used to render tree heirarchy lines.
\note       Maybe use
             void gtk_rc_parse_string (const gchar *rc_string);
             http://gtk2-engines-cleanice.sourcearchive.com/documentation/2.4.1-0ubuntu2/cleanice-style_8c-source.html
**/
static int setHHeight ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{

#ifdef DEBUG_PROGRESS_BAR
	g_print ( "progressbar height -this option currently under development.\n" );
#endif
	static const gchar *rc_string =
	{
		"style \"progressBarHeight\"\n"
		"{\n"
		"   GtkProgressBar::min-horizontal-bar-height = 15\n"
		"}\n"
		"\n"
		"class \"GtkProgressBar\" style \"progressBarHeight\"\n"
	};

	gtk_rc_parse_string ( rc_string );

	return TCL_OK;
}


static int configure ( Tcl_Interp *interp, GtkProgressBar *progressBar, GnoclOption options[] )
{
	if ( options[visibleIdx].status == GNOCL_STATUS_CHANGED )
	{
		g_print ( "-visible %d\n", options[visibleIdx].val.i );

		if ( options[visibleIdx].val.i == 1 )
		{
			g_print ( "show\n" );
			gtk_widget_show ( GTK_WIDGET ( progressBar ) );
		}

		else
		{
			g_print ( "hide\n" );
			gtk_widget_hide ( GTK_WIDGET ( progressBar ) );
		}
	}

	if ( options[hHeightIdx].status == GNOCL_STATUS_CHANGED )
	{
		//setHHeight ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
	}


	return TCL_OK;
}


/**
\brief
**/
int progressBarFunc ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{

	static const char *cmds[] = { "delete", "configure", "pulse", "class", NULL };
	enum cmdIdx { DeleteIdx, ConfigureIdx, PulseIdx, ClassIdx };
	int idx;
	GtkProgressBar *progressBar = ( GtkProgressBar * ) data;

	if ( objc < 2 )
	{
		Tcl_WrongNumArgs ( interp, 1, objv, "command" );
		return TCL_ERROR;
	}

	if ( Tcl_GetIndexFromObj ( interp, objv[1], cmds, "command", TCL_EXACT, &idx ) != TCL_OK )
		return TCL_ERROR;

	switch ( idx )
	{
		case ClassIdx:
			{
				Tcl_SetObjResult ( interp, Tcl_NewStringObj ( "progressBar", -1 ) );
			}
			break;
		case DeleteIdx:
			{
				return gnoclDelete ( interp, GTK_WIDGET ( progressBar ), objc, objv );
			}

		case ConfigureIdx:
			{
				/*
				int ret = gnoclParseAndSetOptions ( interp, objc - 1, objv + 1, progressBarOptions, G_OBJECT ( progressBar ) );
				gnoclClearOptions ( progressBarOptions );
				return ret;
				*/
				int ret = TCL_ERROR;

				if ( gnoclParseAndSetOptions ( interp, objc - 1, objv + 1,
											   progressBarOptions, G_OBJECT ( progressBar ) ) == TCL_OK )
				{
					ret = configure ( interp, progressBar, progressBarOptions );
				}

				gnoclClearOptions ( progressBarOptions );

				return ret;



			}

			break;
		case PulseIdx:
			{
				gtk_progress_bar_pulse ( progressBar );
			}
			break;
	}

	return TCL_OK;
}

/**
\brief
**/
int gnoclProgressBarCmd ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{
	GtkProgressBar *progressBar;
	int            ret;

	if ( gnoclParseOptions ( interp, objc, objv, progressBarOptions ) != TCL_OK )
	{
		gnoclClearOptions ( progressBarOptions );
		return TCL_ERROR;
	}

	static const gchar *rc_string =
	{
		"style \"progressBarHeight\"\n"
		"{\n"
		"   GtkProgressBar::min-horizontal-bar-height = 15\n"
		"}\n"
		"\n"
		"class \"GtkProgressBar\" style \"progressBarHeight\"\n"
	};

	gtk_rc_parse_string ( rc_string );



	progressBar = GTK_PROGRESS_BAR ( gtk_progress_bar_new( ) );

	ret = gnoclSetOptions ( interp, progressBarOptions,	G_OBJECT ( progressBar ), -1 );

	gtk_widget_show ( GTK_WIDGET ( progressBar ) );


	if ( ret == TCL_OK )
	{
		ret = configure ( interp, progressBar, progressBarOptions );
	}

	gnoclClearOptions ( progressBarOptions );


	return gnoclRegisterWidget ( interp, GTK_WIDGET ( progressBar ), progressBarFunc );
}


