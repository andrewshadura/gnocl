/*
* $Id: PBar.c,v 1.9 2005/01/01 15:27:54 baum Exp $
*
* This file implements the PBar widget
*
* Copyright (c) 2001 - 2005 Peter G. Baum  http://www.dr-baum.net
*
* See the file "license.terms" for information on usage and redistribution
* of this file, and for a DISCLAIMER OF ALL WARRANTIES.
*
*/

/*
   History:
   2013-07:	added commands, options, commands
   2008-10: added command, class
   2004-02: added -data
   2003-09: added cget
            removed getValue
        12: switched from GnoclWidgetOptions to GnoclOption
        02: removed "invoke"
   2002-01: new command "invoke"
   2001-10: Begin of developement
 */

/**
\page page_PBar gnocl::PBar
\htmlinclude PBar.html
**/

#include "gnocl.h"
//#include "gnoclparams.h"

/* static function declarations */
static int optUpdatePolicy ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret );
static int setValue ( GtkProgressBar *pbar, double d );
static int setText ( GtkProgressBar *pbar, gchar *text );
static void changedFunc ( GtkWidget *widget, gpointer data );

static int optProgressBarOrientation ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret );

static const int variableIdx       = 0;
static const int fractionIdx       = 1;
static const int onFractionChangedIdx = 2;

static const int textVariableIdx	= 3;
static const int textIdx       		= 4;
static const int onTextChangedIdx 	= 5;

static const int orientationIdx    = 9;

/* compare with spinButton widget which is very similar */
GnoclOption PBarOptions[] =
{
	{ "-variable", GNOCL_STRING, NULL },         	/* 0 */
	{ "-fraction", GNOCL_DOUBLE, NULL },            /* 1 */
	{ "-onFractionChanged", GNOCL_STRING, NULL },   /* 2 */

	{ "-textVariable", GNOCL_STRING, NULL }, 		/* 3 */
	{ "-text", GNOCL_STRING, NULL },				/* 4 */
	{ "-onTextChanged", GNOCL_STRING, NULL },		/* 5 */

	{ "-activityMode", GNOCL_BOOL, "activity-mode" },
//	{ "-fraction", GNOCL_DOUBLE, "fraction" },
	{ "-pulseStep", GNOCL_DOUBLE, "pulse-step" },
	{ "-orientation", GNOCL_OBJ, "orientation", optProgressBarOrientation },
//	{ "-text", GNOCL_STRING, "text" },
	{ "-textAlign", GNOCL_OBJ, "text-?align", gnoclOptBothAlign },
	{ "-showText", GNOCL_BOOL, "show-text" },
	{ "-name", GNOCL_STRING, "name" },
	{ "-visible", GNOCL_BOOL, "visible" },
	{ "-widthGroup", GNOCL_OBJ, "w", gnoclOptSizeGroup },
	{ "-heightGroup", GNOCL_OBJ, "h", gnoclOptSizeGroup },
	{ "-sizeGroup", GNOCL_OBJ, "s", gnoclOptSizeGroup },
	{ "-tooltip", GNOCL_OBJ, "", gnoclOptTooltip },
	{ NULL }
};


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

/* moved to gnocl.h */
/*
typedef struct
{
    char       *name;
    Tcl_Interp *interp;
    GtkProgressBar   *PBar;
    char       *onValueChanged;
    char       *variable;
    int        inSetVar;
} ProgressBarParams;
*/

/**
\brief
**/
static int optUpdatePolicy ( Tcl_Interp *interp, GnoclOption *opt,
							 GObject *obj, Tcl_Obj **ret )
{
	const char *txt[] = { "continuous", "discontinuous", "delayed", NULL };
	const int types[] = { GTK_UPDATE_CONTINUOUS,
						  GTK_UPDATE_DISCONTINUOUS, GTK_UPDATE_DELAYED
						};

	assert ( sizeof ( GTK_UPDATE_DELAYED ) == sizeof ( int ) );

	return gnoclOptGeneric ( interp, opt, obj, "updatePolicy", txt, types, ret );
}

/**
\brief
**/
static Tcl_Obj *getObjValue ( GtkProgressBar *pbar )
{
	double d = gtk_progress_bar_get_fraction ( pbar );
	return Tcl_NewDoubleObj ( d );
}

/**
\brief
**/
static Tcl_Obj *getObjText ( GtkProgressBar *pbar )
{
	const gchar *txt;
	txt =  gtk_progress_bar_get_text ( pbar );
	return Tcl_NewStringObj ( txt, -1 );
}

/**
\brief
**/
static int doCommand ( ProgressBarParams *para, Tcl_Obj *val, int background )
{
	if ( para->onFractionChanged )
	{
		GnoclPercSubst ps[] =
		{
			{ 'w', GNOCL_STRING },  /* widget */
			{ 'v', GNOCL_OBJ },  /* value */
			{ 0 }
		};

		ps[0].val.str = para->name;
		ps[1].val.obj = val;

		return gnoclPercentSubstAndEval ( para->interp, ps, para->onFractionChanged, background );
	}

	return TCL_OK;
}

/**
\brief
**/
static int doTextCommand ( ProgressBarParams *para, Tcl_Obj *val, int background )
{
	if ( para->onTextChanged )
	{
		GnoclPercSubst ps[] =
		{
			{ 'w', GNOCL_STRING },  /* widget */
			{ 't', GNOCL_OBJ },  /* value */
			{ 0 }
		};

		ps[0].val.str = para->name;
		ps[1].val.obj = val;

		return gnoclPercentSubstAndEval ( para->interp, ps, para->onTextChanged, background );
	}

	return TCL_OK;
}

/**
\brief
**/
static int setValue ( GtkProgressBar *pbar, double fraction )
{
	g_print ( "%s %d\n", __FUNCTION__, 1 );

	//GtkAdjustment *adjust = gtk_range_get_adjustment ( GTK_RANGE ( PBar ) );
	//int blocked = g_signal_handlers_block_matched ( G_OBJECT ( adjust ), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, ( gpointer * ) changedFunc, NULL );
	//gtk_adjustment_set_value ( adjust, d );

	gtk_progress_bar_set_fraction ( pbar, fraction );

	/*
		if ( blocked )
			g_signal_handlers_unblock_matched (
				G_OBJECT ( adjust ), G_SIGNAL_MATCH_FUNC,
				0, 0, NULL, ( gpointer * ) changedFunc, NULL );
	*/

	return TCL_OK;
}

/**
\brief
**/
static int setText ( GtkProgressBar *pbar, gchar *text )
{
	g_print ( "%s %d\n", __FUNCTION__, 1 );

	//GtkAdjustment *adjust = gtk_range_get_adjustment ( GTK_RANGE ( PBar ) );
	//int blocked = g_signal_handlers_block_matched ( G_OBJECT ( adjust ), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, ( gpointer * ) changedFunc, NULL );
	//gtk_adjustment_set_value ( adjust, d );

	gtk_progress_bar_set_text ( pbar, text );

	/*
		if ( blocked )
			g_signal_handlers_unblock_matched (
				G_OBJECT ( adjust ), G_SIGNAL_MATCH_FUNC,
				0, 0, NULL, ( gpointer * ) changedFunc, NULL );
	*/

	return TCL_OK;
}


/**
\brief
**/
static char *traceFunc ( ClientData data, Tcl_Interp *interp, const char *name1, const char *name2, int flags )
{
	g_print ( "%s\n", __FUNCTION__ );

	ProgressBarParams *para = ( ProgressBarParams * ) data;

	if ( para->inSetVar == 0 && name1 != NULL )
	{
		Tcl_Obj *val = Tcl_GetVar2Ex ( interp, name1, name2, flags );
		double d;

		if ( val && Tcl_GetDoubleFromObj ( NULL, val, &d ) == TCL_OK )
		{
			setValue ( para->pbar, d );
			doCommand ( para, getObjValue ( para->pbar ), 1 );
		}
	}

	return NULL;
}

/**
\brief
**/
static char *traceTextVarFunc ( ClientData data, Tcl_Interp *interp, const char *name1, const char *name2, int flags )
{
	g_print ( "%s\n", __FUNCTION__ );

	ProgressBarParams *para = ( ProgressBarParams * ) data;

	if ( para->inSetVar == 0 && name1 != NULL )
	{
		Tcl_Obj *val = Tcl_GetVar2Ex ( interp, name1, name2, flags );
		double d;
		gchar *txt;
		txt = Tcl_GetString ( val );

		if ( val && Tcl_GetDoubleFromObj ( NULL, val, &d ) == TCL_OK )
		{
			setText ( para->pbar, txt );
			doTextCommand ( para, getObjText ( para->pbar ), 1 );
		}
	}

	return NULL;
}

/**
\brief
**/
static void destroyFunc ( GtkWidget *widget, gpointer data )
{
	ProgressBarParams *para = ( ProgressBarParams * ) data;

	/*
		gnoclAttachOptCmdAndVar (
			NULL, &para->onFractionChanged,
			NULL, &para->variable,
			"value-changed",
			G_OBJECT ( gtk_range_get_adjustment ( GTK_RANGE ( para->pbar ) ) ),
			G_CALLBACK ( changedFunc ), para->interp, traceFunc, para );
	*/




	gnoclForgetWidgetFromName ( para->name );
	Tcl_DeleteCommand ( para->interp, para->name );

	g_free ( para->name );
	g_free ( para );
}

/**
\brief
**/
static int setVariable ( ProgressBarParams *para, Tcl_Obj *val )
{
	if ( para->variable && para->inSetVar == 0 )
	{
		Tcl_Obj *ret;
		para->inSetVar = 1;
		ret = Tcl_SetVar2Ex ( para->interp, para->variable, NULL, val, TCL_GLOBAL_ONLY );
		para->inSetVar = 0;
		return ret == NULL ? TCL_ERROR : TCL_OK;
	}

	return TCL_OK;
}

/**
\brief
**/
static int setTextVariable ( ProgressBarParams *para, Tcl_Obj *val )
{
	if ( para->variable && para->inSetVar == 0 )
	{
		Tcl_Obj *ret;
		para->inSetText = 1;
		ret = Tcl_SetVar2Ex ( para->interp, para->textVariable, NULL, val, TCL_GLOBAL_ONLY );
		para->inSetText = 0;
		return ret == NULL ? TCL_ERROR : TCL_OK;
	}

	return TCL_OK;
}

/**
\brief
**/
static void changedFunc ( GtkWidget *widget, gpointer data )
{
	ProgressBarParams *para = ( ProgressBarParams * ) data;
	Tcl_Obj *val = getObjValue ( para->pbar );
	setVariable ( para, val );
	doCommand ( para, val, 1 );
}

/**
\brief
**/
static void textChangedFunc ( GtkWidget *widget, gpointer data )
{
	ProgressBarParams *para = ( ProgressBarParams * ) data;
	Tcl_Obj *val = getObjText ( para->pbar );
	setTextVariable ( para, val );
	doTextCommand ( para, val, 1 );
}

/**
\brief
**/
static int configure ( Tcl_Interp *interp, ProgressBarParams *para, GnoclOption options[] )
{

	g_print ( "%s %d\n", __FUNCTION__, 1 );

	int ret = TCL_ERROR;


	if ( gnoclSetOptions ( interp, options, G_OBJECT ( para->pbar ), -1 ) != TCL_OK )
		goto cleanExit;

	g_print ( "%s %d\n", __FUNCTION__, 2 );

	gnoclAttachOptCmdAndVar (
		options + onFractionChangedIdx, &para->onFractionChanged,
		options + variableIdx, &para->variable,
		"value-changed", G_OBJECT ( para->pbar ),
		G_CALLBACK ( changedFunc ), interp, traceFunc, para );

	gnoclAttachOptCmdAndVar (
		options + onTextChangedIdx, &para->onTextChanged,
		options + textIdx, &para->textVariable,
		"changed", G_OBJECT ( para->pbar ),
		G_CALLBACK ( textChangedFunc ), interp, traceTextVarFunc, para );

	g_print ( "%s %d\n", __FUNCTION__, 3 );

	/* if variable is set, synchronize variable and widget */
	if ( options[variableIdx].status == GNOCL_STATUS_CHANGED
			&& para->variable != NULL
			&& options[fractionIdx].status != GNOCL_STATUS_CHANGED )
	{
		Tcl_Obj *var = Tcl_GetVar2Ex ( interp, para->variable, NULL, TCL_GLOBAL_ONLY );

		assert ( strcmp ( options[variableIdx].optName, "-variable" ) == 0 );

		if ( var == NULL ) /* variable does not yet exist */
		{
			setVariable ( para, getObjValue ( para->pbar ) );
		}

		else
		{
			double d;

			if ( Tcl_GetDoubleFromObj ( interp, var, &d ) != TCL_OK )
				goto cleanExit;

			setValue ( para->pbar, d );
		}

	}

	/* if textVariable is set, synchronize variable and widget */
	if ( options[textVariableIdx].status == GNOCL_STATUS_CHANGED
			&& para->textVariable != NULL
			&& options[textVariableIdx].status != GNOCL_STATUS_CHANGED )
	{
		//Tcl_Obj *textVar = Tcl_GetVar2Ex ( interp, para->textVariable, NULL, TCL_GLOBAL_ONLY );
		const char *val = Tcl_GetVar ( interp, para->textVariable, TCL_GLOBAL_ONLY );
		assert ( strcmp ( options[textIdx].optName, "-textVariable" ) == 0 );

		if ( val == NULL ) /* variable does not yet exist */
		{
			setTextVariable ( para, getObjText ( para->pbar ) );
		}

		else
		{
			gchar *txt;

			if ( Tcl_GetStringFromObj ( para->textVariable, txt ) != TCL_OK )
				goto cleanExit;

			setText ( para->pbar, txt );
		}

	}

	g_print ( "%s %d\n", __FUNCTION__, 4 );

	if ( options[fractionIdx].status == GNOCL_STATUS_CHANGED )
	{

		g_print ( "%s set fraction\n", __FUNCTION__ );
		assert ( strcmp ( options[fractionIdx].optName, "-fraction" ) == 0 );
		setValue ( para->pbar, options[fractionIdx].val.d );
		setVariable ( para, getObjValue ( para->pbar ) );
	}

	if ( options[textIdx].status == GNOCL_STATUS_CHANGED )
	{

		g_print ( "%s set text\n", __FUNCTION__ );
		assert ( strcmp ( options[textIdx].optName, "-text" ) == 0 );
		setText ( para->pbar, options[textIdx].val.str );
		setTextVariable ( para, getObjText ( para->pbar ) );
	}

	g_print ( "%s %d\n", __FUNCTION__, 5 );

	ret = TCL_OK;

cleanExit:

	return ret;
}

/**
\brief
**/
static int cget ( Tcl_Interp *interp, ProgressBarParams *para, GnoclOption options[], int idx )
{
	//GtkAdjustment *adjust = gtk_range_get_adjustment ( GTK_RANGE ( para->pbar ) );

	Tcl_Obj *obj = NULL;

	if ( idx == orientationIdx )
	{

		switch ( gtk_progress_bar_get_orientation ( para->pbar ) )
		{
			case GTK_PROGRESS_LEFT_TO_RIGHT:
				{
					obj = Tcl_NewStringObj ( "leftToRight", -1 );
				}
				break;
			case GTK_PROGRESS_RIGHT_TO_LEFT:
				{
					obj = Tcl_NewStringObj ( "rightToLeft", -1 );
				}
				break;
			case GTK_PROGRESS_BOTTOM_TO_TOP:
				{
					obj = Tcl_NewStringObj ( "bottomToTop", -1 );
				}
				break;
			case GTK_PROGRESS_TOP_TO_BOTTOM:
				{
					obj = Tcl_NewStringObj ( "topToBottom", -1 );
				}
				break;
			default: {}
		}
	}

	else if ( idx == variableIdx )
		obj = Tcl_NewStringObj ( para->variable, -1 );
	else if ( idx == onFractionChangedIdx )
	{
		obj = Tcl_NewStringObj (
				  para->onFractionChanged ? para->onFractionChanged : "", -1 );
	}

	else if ( idx == fractionIdx )
		obj = getObjValue ( para->pbar );

	if ( obj != NULL )
	{
		Tcl_SetObjResult ( interp, obj );
		return TCL_OK;
	}

	return gnoclCgetNotImplemented ( interp, options + idx );
}


static const char *cmds[] = { "delete", "configure", "cget", "onValueChanged", "class", NULL };

/**
\brief
**/
int PBarFunc ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{

	enum cmdIdx { DeleteIdx, ConfigureIdx, CgetIdx, OnValueChangedIdx, ClassIdx };
	ProgressBarParams *para = ( ProgressBarParams * ) data;
	GtkProgressBar   *widget = GTK_WIDGET ( para->pbar );
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
				Tcl_SetObjResult ( interp, Tcl_NewStringObj ( "PBar", -1 ) );
			}
			break;
		case DeleteIdx:
			{
				return gnoclDelete ( interp, widget, objc, objv );
			}

		case ConfigureIdx:
			{
				int ret = TCL_ERROR;
				/*
								if ( gnoclParseOptions ( interp, objc - 1, objv + 1, PBarOptions ) == TCL_OK )
								{
									ret = configure ( interp, para, PBarOptions );
								}
				*/
				gnoclClearOptions ( PBarOptions );

				return ret;
			}

			break;
		case CgetIdx:
			{
				int     idx;

				switch ( gnoclCget ( interp, objc, objv, G_OBJECT ( para->pbar ), PBarOptions, &idx ) )
				{
					case GNOCL_CGET_ERROR:
						return TCL_ERROR;
					case GNOCL_CGET_HANDLED:
						return TCL_OK;
					case GNOCL_CGET_NOTHANDLED:
						return cget ( interp, para, PBarOptions, idx );
				}
			}

		case OnValueChangedIdx:
			{
				if ( objc != 2 )
				{
					Tcl_WrongNumArgs ( interp, 2, objv, NULL );
					return TCL_ERROR;
				}

				return doCommand ( para, getObjValue ( para->pbar ), 0 );
			}
	}

	return TCL_OK;
}

/**
\brief
**/
int gnoclPBarCmd ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{
	if ( gnoclGetCmdsAndOpts ( interp, cmds, PBarOptions, objv, objc ) == TCL_OK )
	{
		return TCL_OK;
	}


	ProgressBarParams    *para;
	int ret;

	if ( gnoclParseOptions ( interp, objc, objv, PBarOptions ) != TCL_OK )
	{
		gnoclClearOptions ( PBarOptions );
		return TCL_ERROR;
	}

	para = g_new ( ProgressBarParams, 1 );
	para->pbar = GTK_PROGRESS_BAR ( gtk_progress_bar_new() );
	para->interp = interp;
	para->variable = NULL;
	para->textVariable = NULL;
	para->onFractionChanged = NULL;
	para->onTextChanged = NULL;
	para->inSetVar = 0;
	para->inSetText = 0;

	gtk_widget_show ( GTK_WIDGET ( para->pbar ) );

	g_print ( "%s %d\n", __FUNCTION__, 1 );

	if ( configure ( interp, para, PBarOptions ) != TCL_OK )
	{
		g_print ( "%s %d\n", __FUNCTION__, 2 );
		g_free ( para );
		gtk_widget_destroy ( GTK_WIDGET ( para->pbar ) );
		return TCL_ERROR;
	}


	ret = gnoclSetOptions ( interp, PBarOptions, G_OBJECT ( para->pbar ), -1 );

	if ( ret == TCL_OK )
	{
		ret = configure ( interp, para, PBarOptions );
	}

	gnoclClearOptions ( PBarOptions );


	para->name = gnoclGetAutoWidgetId();

	g_signal_connect ( G_OBJECT ( para->pbar ), "destroy", G_CALLBACK ( destroyFunc ), para );

	gnoclMemNameAndWidget ( para->name, GTK_WIDGET ( para->pbar ) );

	Tcl_CreateObjCommand ( interp, para->name, PBarFunc, para, NULL );

	Tcl_SetObjResult ( interp, Tcl_NewStringObj ( para->name, -1 ) );

	return TCL_OK;
}


