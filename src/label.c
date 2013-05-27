/*
 * $Id: label.c,v 1.7 2005/08/16 20:57:45 baum Exp $
 *
 * This file implements the label widget
 *
 * Copyright (c) 2001 - 2005 Peter G. Baum  http://www.dr-baum.net
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */


/*
   History:
   2013-05: added -foreground, -background
			bugfix in cget
   2013-01: added -onDestroy
   2012-08: added -resizable
   2011-06: added -cursorPos, -singleLine, -trackVisitedLinks, -useUnderline
            resolved problems with use of pango markup with the -text option
   2010-01: added -ellipsize
   2009-11: added -tooltip
   2008-11: added option -angle
                         -textVariable
   2008-11: added command, class
   2008-10: added command, class
   2004-02: added -data
   2003-08: added cget
        08: switched from GnoclWidgetOptions to GnoclOption
        04: updates for gtk 2.0
   2002-01: _really_ set STD_OPTIONS
        12: removed {x,y}{Align,Pad}, use list parameters instead
        12: {x,y}{Align,Pad}
        09: underlined accelerators; bindUnderline widgetID
        07: Fixed reconfiguration, added std options
   2001-03: Begin of developement
 */

#include "gnocl.h"
#include "gnoclparams.h"

/**
\page page_label gnocl::label
\htmlinclude label.html
**/

static gnoclOptAttributes ( Tcl_Interp * interp, GnoclOption * opt, GObject * obj, Tcl_Obj **ret );

/*
typedef enum {
  PANGO_ELLIPSIZE_NONE,
  PANGO_ELLIPSIZE_START,
  PANGO_ELLIPSIZE_MIDDLE,
  PANGO_ELLIPSIZE_END
} PangoEllipsizeMode;
*/

static const int textVariableIdx = 0;
static const int onChangedIdx = 1;
static const int valueIdx = 2;
static const int textIdx = 3;
static const int cursorIdx = 4;
static const int widgetIdx = 5;
static const int mnemonicIdx = 6;
static const int resizableIdx = 7;
static const int dataIdx = 9;

static GnoclOption labelOptions[] =
{
	/* gnocl specific options */
	{ "-textVariable", GNOCL_STRING, NULL },            /* 0 */
	{ "-onChanged", GNOCL_STRING, NULL },               /* 1 */
	{ "-value", GNOCL_STRING, NULL},                    /* 2 */
	{ "-text", GNOCL_STRING, NULL},                     /* 3 */
	{ "-showCursor", GNOCL_BOOL, NULL},					/* 4 */
	{ "-mnemonicWidget", GNOCL_STRING, NULL }, //6
	{ "-mnemonicText", GNOCL_STRING, NULL }, //7
	{ "-resizable", GNOCL_BOOL, NULL }, //8
	{ "-data", GNOCL_OBJ, "", gnoclOptData }, //9

	/* gtklabel specific properties */
	{ "-angle", GNOCL_DOUBLE, "angle" },
	//{ "-attributes", GNOCL_DOUBLE, "angle" },
	{ "-cursorPos", GNOCL_INT, "cursor-position" },
	{ "-ellipsize", GNOCL_OBJ, "ellipsize" , gnoclOptEllipsize },
	{ "-justify", GNOCL_OBJ, "justify", gnoclOptJustification },
	//{ "-label", GNOCL_STRING, "label" },
	{ "-maxWidthChars", GNOCL_INT, "max-width-chars" },
	//{ "-mneumonicKeyval", GNOCL_INT, "mnemonic-keyval" },
	//{ "-pattern", GNOCL_STRING, "pattern" },
	{ "-selectable", GNOCL_BOOL, "selectable" },
	//{ "-selectionBound", GNOCL_INT,"selection-bound" },
	{ "-singleLine", GNOCL_BOOL, "single-line-mode" },
	{ "-trackVisitedLinks", GNOCL_BOOL, "track-visited-links" },
	{ "-useMarkup", GNOCL_BOOL, "use-markup" },
	{ "-useUnderline", GNOCL_BOOL, "use-underline" },
	{ "-widthChars", GNOCL_INT, "width-chars" },
	{ "-wrap", GNOCL_BOOL, "wrap" },
	//{ "-wrapMode", GNOCL_BOOL, "wrap" },

	/* inherited properties */
	{ "-align", GNOCL_OBJ, "?align", gnoclOptBothAlign },

	{ "-heightGroup", GNOCL_OBJ, "h", gnoclOptSizeGroup },
	{ "-name", GNOCL_STRING, "name" },

	{ "-sensitive", GNOCL_BOOL, "sensitive" },
	{ "-sizeGroup", GNOCL_OBJ, "s", gnoclOptSizeGroup },
	{ "-visible", GNOCL_BOOL, "visible" },

	{ "-widthGroup", GNOCL_OBJ, "w", gnoclOptSizeGroup },

	{ "-xPad", GNOCL_INT, "xpad" },
	{ "-yPad", GNOCL_INT, "ypad" },

	/* set using pango */
	{ "-baseFont", GNOCL_OBJ, "", gnoclOptGdkBaseFont },
	{ "-foreground", GNOCL_OBJ, "f", gnoclOptAttributes },
	{ "-background", GNOCL_OBJ, "b", gnoclOptAttributes },

	/* these don't work */
	//{ "-background", GNOCL_OBJ, "background-gdk", gnoclOptGdkColor },
	//{ "-foreground", GNOCL_OBJ, "foreground-gdk", gnoclOptGdkColor },

	{ "-tooltip", GNOCL_OBJ, "", gnoclOptTooltip },

	/* respond to widget destruction */
	{ "-onDestroy", GNOCL_OBJ, "destroy", gnoclOptCommand },

	/* set base attributes */
	//{ "-atrributes", GNOCL_OBJ, "", gnoclOptAttributes },

	{ NULL }
};


/* moved to gnocl.h */
/*
typedef struct
{
    GtkLabel    *label;
    Tcl_Interp  *interp;
    char        *name;
    char        *textVariable;
    char        *onChanged;
    int         inSetVar;
} LabelParams;
*/

static int setVal ( GtkLabel *label, const char *txt );
static void changedFunc ( GtkWidget *widget, gpointer data );

/**
\brief Set foreground attribute
**/
static setFg ( GtkLabel *label, GdkColor color )
{
	PangoAttrList *list;
	PangoAttribute *attr;

	list = gtk_label_get_attributes ( label );
	attr = pango_attr_foreground_new ( color.red, color.green, color.blue );
	pango_attr_list_insert ( list, attr );
	gtk_label_set_attributes ( label, list );
	//pango_attr_list_unref ( list );
}

/**
\brief Set background attribute
**/
static setBg ( GtkLabel *label, GdkColor color )
{
	PangoAttrList *list;
	PangoAttribute *attr;

	list =  gtk_label_get_attributes ( label );
	attr = pango_attr_background_new ( color.red, color.green, color.blue );
	pango_attr_list_insert ( list, attr );
	gtk_label_set_attributes ( label, list );
	//pango_attr_list_unref ( list );
}

/**
\brief	Set Pango attributes for entire label contents.
		Useful for setting markup for variables entries.
**/
static gnoclOptAttributes ( Tcl_Interp * interp, GnoclOption * opt, GObject * obj, Tcl_Obj **ret )
{

	const char *clr;
	GdkColor color;

	clr = Tcl_GetString ( opt->val.obj );
	gdk_color_parse ( clr, &color );

	/*
		switch ( opt->optName[1] )
		{
			case 'f':   mode = GTK_SIZE_GROUP_HORIZONTAL; break;
			case 'b':   mode = GTK_SIZE_GROUP_VERTICAL;   break;
			default: assert ( opt->optName[1] == 's' );
		}
	*/

//g_print ("%s\n", opt->optName[0]);


	switch ( opt->optName[1] )
	{
		case 'f':
			{
				setFg ( GTK_WIDGET ( obj ), color );
			}
			break;
		case 'b':
			{
				setBg ( GTK_WIDGET ( obj ), color );
			}
			break;
	}


	//setFg ( GTK_WIDGET ( obj ), color);

	//g_print ( "OK, clr = %s ; r %d g %d b %d\n", clr, color.red, color.green, color.blue );

	return TCL_OK;

}

/**
\brief
**/
static void doAllocate ( GtkWidget *label, GtkAllocation *allocation, gpointer data )
{
	gtk_widget_set_size_request ( label, allocation->width - 2, -1 );
}


/**
\brief
**/
static int doCommand ( LabelParams *para, const char *val, int background )
{
	if ( para->onChanged )
	{
		GnoclPercSubst ps[] =
		{
			{ 'w', GNOCL_STRING },  /* widget */
			{ 'v', GNOCL_STRING },  /* value */
			{ 0 }
		};

		ps[0].val.str = para->name;
		ps[1].val.str = val;

		return gnoclPercentSubstAndEval ( para->interp, ps, para->onChanged, background );
	}

	return TCL_OK;
}

/**
\brief
**/
static int setVal ( GtkLabel *label, const char *txt )
{
	int blocked;
	blocked = g_signal_handlers_block_matched ( G_OBJECT ( label ), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, ( gpointer * ) changedFunc, NULL );
	//gtk_label_set_text ( label, txt );
	gtk_label_set_markup ( label, txt );

	//OptLabelFull ( label, txt );

	if ( blocked )
	{
		g_signal_handlers_unblock_matched ( G_OBJECT ( label ), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, ( gpointer * ) changedFunc, NULL );
	}

	return TCL_OK;
}

/**
\brief
**/
static char *traceFunc (	ClientData data,	Tcl_Interp *interp,	const char *name1,	const char *name2,	int flags )
{
	LabelParams *para = ( LabelParams * ) data;

	if ( para->inSetVar == 0 && name1 )
	{
		const char *txt = name1 ? Tcl_GetVar2 ( interp, name1, name2, 0 ) : NULL;

		if ( txt )
		{
			setVal ( para->label, txt );
			doCommand ( para, txt, 1 );
		}
	}

	return NULL;
}

/**
\brief
**/
static int setTextVariable (	LabelParams *para,	const char *val )
{
#ifdef DEBUG_LABEL
	printf ( "label/staticFuncs/setTextVariable\n" );
#endif


	if ( para->textVariable && para->inSetVar == 0 )
	{
		const char *ret;
		para->inSetVar = 1;
		ret = Tcl_SetVar ( para->interp, para->textVariable, val, TCL_GLOBAL_ONLY );
		para->inSetVar = 0;
		return ret == NULL ? TCL_ERROR : TCL_OK;
	}

	return TCL_OK;
}

/**
\brief
**/
static void changedFunc ( GtkWidget *widget, gpointer data )
{
#ifdef DEBUG_LABEL
	printf ( "label/staticFuncs/changedFunc\n" );
#endif

	LabelParams *para = ( LabelParams * ) data;

	const char *val = gtk_label_get_text ( GTK_LABEL ( para->label ) );
	setTextVariable ( para, val );
	doCommand ( para, val, 1 );
}

/**
\brief
**/
static void destroyFunc ( GtkWidget *widget, gpointer data )
{
#ifdef DEBUG_LABEL
	printf ( "label/staticFuncs/destroyFunc\n" );
#endif


	LabelParams *para = ( LabelParams * ) data;

	gnoclForgetWidgetFromName ( para->name );
	Tcl_DeleteCommand ( para->interp, para->name );

	/* free up pango attributes list */
	PangoAttrList *list;
	list = gtk_label_get_attributes ( para->label );
	pango_attr_list_unref ( list );

	gnoclAttachOptCmdAndVar (
		NULL, &para->onChanged,
		NULL, &para->textVariable,
		"changed", G_OBJECT ( para->label ),
		G_CALLBACK ( changedFunc ), para->interp, traceFunc, para );

	g_free ( para->textVariable );
	g_free ( para->name );
	g_free ( para );
}

/**
\brief
**/
static int configure ( Tcl_Interp *interp, LabelParams *para, GnoclOption options[] )
{
#ifdef DEBUG_LABEL
	printf ( "label/staticFuncs/configure\n" );
#endif


	gnoclAttachOptCmdAndVar (
		&options[onChangedIdx], &para->onChanged,
		&options[textVariableIdx], &para->textVariable,
		"changed", G_OBJECT ( para->label ),
		G_CALLBACK ( changedFunc ), interp, traceFunc, para );

	if ( options[textVariableIdx].status == GNOCL_STATUS_CHANGED
			&& options[valueIdx].status == 0  /* value is handled below */
			&& para->textVariable != NULL )
	{
		/* if variable does not exist -> set it, else set widget state */
		const char *val = Tcl_GetVar ( interp, para->textVariable, TCL_GLOBAL_ONLY );

		if ( val == NULL )
		{
			val = gtk_label_get_text ( GTK_LABEL ( para->label ) );
			setTextVariable ( para, val );
		}

		else
			setVal ( para->label, val );
	}

	if ( options[dataIdx].status == GNOCL_STATUS_CHANGED )
	{
		gnoclOptData ( interp, &options[dataIdx], para->label, NULL );
		return TCL_OK;
	}


	if ( options[valueIdx].status == GNOCL_STATUS_CHANGED )
	{
		char *str = options[valueIdx].val.str;
		setVal ( para->label, str );
		setTextVariable ( para, str );
	}

	if ( options[textIdx].status == GNOCL_STATUS_CHANGED )
	{
		char *str = options[textIdx].val.str;
		gtk_label_set_markup ( para->label, str );

	}


	if ( options[widgetIdx].status == GNOCL_STATUS_CHANGED )
	{

		GtkWidget *widget = gnoclGetWidgetFromName ( options[widgetIdx].val.str, interp );
		gtk_label_set_mnemonic_widget ( para->label, widget );

	}

	if ( options[mnemonicIdx].status == GNOCL_STATUS_CHANGED )
	{

		gtk_label_set_markup_with_mnemonic ( para->label, options[mnemonicIdx].val.str );

	}

	if ( options[resizableIdx].status == GNOCL_STATUS_CHANGED )
	{
		if ( options[resizableIdx].val.b )
		{
			g_signal_connect ( G_OBJECT ( para->label ), "size-allocate", G_CALLBACK ( doAllocate ), NULL );
		}

		else
		{
			g_signal_connect ( G_OBJECT ( para->label ), "size-allocate", NULL, NULL );
		}



	}

	return TCL_OK;
}

/**
\brief
\author     Peter G Baum, William J Giddings
\date
**/
static int cget ( Tcl_Interp *interp, LabelParams *para, GnoclOption options[], int idx )
{
#ifdef DEBUG_LABEL
	printf ( "label/staticFuncs/cget\n" );
#endif

	Tcl_Obj *obj = NULL;

	if ( idx == dataIdx )
	{
		obj = Tcl_NewStringObj ( g_object_get_data ( G_OBJECT ( para->label ), "gnocl::data" ), -1 );
	}

	if ( idx == textVariableIdx )
	{
		obj = Tcl_NewStringObj ( para->textVariable, -1 );
	}

	if ( idx == onChangedIdx )
	{
		obj = Tcl_NewStringObj ( para->onChanged ? para->onChanged : "", -1 );
	}

	if ( idx == textIdx )
	{

		obj = Tcl_NewStringObj ( gtk_label_get_text ( para->label ) , -1 );
		/* test for use markup */
		//obj = Tcl_NewStringObj ( gtk_label_get_label ( para->label ) , -1 );

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
int labelFunc (	ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{
	static const char *cmds[] = { "delete", "configure", "cget", "onChanged", "class", NULL };
	enum cmdIdx { DeleteIdx, ConfigureIdx, CgetIdx, OnChangedIdx, ClassIdx};

	LabelParams *para = ( LabelParams * ) data;
	GtkWidget *widget = GTK_WIDGET ( para->label );
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
				Tcl_SetObjResult ( interp, Tcl_NewStringObj ( "label", -1 ) );
			}
			break;
		case DeleteIdx:
			{
				return gnoclDelete ( interp, widget, objc, objv );
			}

		case ConfigureIdx:
			{
				int ret = TCL_ERROR;

				if ( gnoclParseAndSetOptions ( interp, objc - 1, objv + 1, labelOptions, G_OBJECT ( widget ) ) == TCL_OK )
				{
					ret = configure ( interp, para, labelOptions );
				}

				gnoclClearOptions ( labelOptions );

				return ret;
			}

			break;

		case CgetIdx:
			{

				int     idx;

				switch ( gnoclCget ( interp, objc, objv, G_OBJECT ( para->label ), labelOptions, &idx ) )
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
							return cget ( interp, para, labelOptions, idx );
						}
				}
			}

		case OnChangedIdx:
			{

				const char *txt = gtk_label_get_text ( GTK_LABEL ( para->label ) );

				if ( objc != 2 )
				{
					Tcl_WrongNumArgs ( interp, 2, objv, NULL );
					return TCL_ERROR;
				}

				return doCommand ( para, txt, 0 );
			}
	}

	return TCL_OK;
}

/**
\brief
**/
int gnoclLabelCmd (	ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{
#ifdef DEBUG_LABEL
	printf ( "label/staticFuncs/gnoclLabelCmd\n" );
#endif

	LabelParams *para;
	int ret;

	if ( gnoclParseOptions ( interp, objc, objv, labelOptions ) != TCL_OK )
	{
		gnoclClearOptions ( labelOptions );
		return TCL_ERROR;
	}

	para = g_new ( LabelParams, 1 );


	if ( labelOptions[mnemonicIdx].status == GNOCL_STATUS_CHANGED )
	{

		para->label = GTK_LABEL ( gtk_label_new_with_mnemonic ( NULL ) );

	}

	else
	{

		para->label = GTK_LABEL ( gtk_label_new ( NULL ) );
	}

	para->interp = interp;
	para->textVariable = NULL;
	para->onChanged = NULL;
	para->inSetVar = 0;

	/* add default attributes list */
	PangoAttrList *list = pango_attr_list_new ();
	gtk_label_set_attributes ( para->label, list );

	gtk_widget_show ( GTK_WIDGET ( para->label ) );

	/* added 14/Jan/2010 */
	gtk_label_set_use_markup ( para->label, TRUE );


	ret = gnoclSetOptions ( interp, labelOptions, G_OBJECT ( para->label ), -1 );

	if ( ret == TCL_OK )
	{
		ret = configure ( interp, para, labelOptions );
	}

	gnoclClearOptions ( labelOptions );

	if ( ret != TCL_OK )
	{
		gtk_widget_destroy ( GTK_WIDGET ( para->label ) );
		g_free ( para );
		return TCL_ERROR;
	}

	para->name = gnoclGetAutoWidgetId();

	g_signal_connect ( G_OBJECT ( para->label ), "destroy", G_CALLBACK ( destroyFunc ), para );

	gnoclMemNameAndWidget ( para->name, GTK_WIDGET ( para->label ) );

	Tcl_CreateObjCommand ( interp, para->name, labelFunc, para, NULL );

	Tcl_SetObjResult ( interp, Tcl_NewStringObj ( para->name, -1 ) );

	return TCL_OK;
}

/*****/
