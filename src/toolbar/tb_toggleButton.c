/*
 * tb_toggleButton.c
 *
 * This file implements the toolbar toggle button item.
 *
 * Copyright (c) 2001 - 2005 Peter G. Baum  http://www.dr-baum.net
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */


/**
\page page_arrowButton gnocl::arrowButton
\htmlinclude arrowButton.html
\note The "-arrow" and "-shadow" options do not map to any widget property and are specified as GNOCL_STRINGS.
	These values will be read prior to creation of an instance the arrowButton object. The remaining properties are
	then passed for the configuration of the arrow's container, a button. These values are passed to comply with the
	gnocl format, otherwise custom buttons would need to be implement for each direction.
**/

/*
   History:
   2014-01: moved toolbar item creation functions into own modules
*/

#include "../gnocl.h"

/* Checkbutton Options */
static const int checkTextIdx      = 0;
static const int checkIconIdx      = 1;
static const int checkOnToggledIdx = 2;
static const int checkOnValueIdx   = 3;
static const int checkOffValueIdx  = 4;
static const int checkVariableIdx  = 5;
static const int checkActiveIdx    = 6;
static const int checkValueIdx     = 7;
static const int checkPositionIdx  = 8;

static GnoclOption checkOptions[] =
{
	{ "-text", GNOCL_OBJ, NULL },           /* 0 */
	{ "-icon", GNOCL_OBJ, NULL },           /* 1 */
	{ "-onToggled", GNOCL_STRING, NULL },   /* 2 the order is important! */
	{ "-onValue", GNOCL_OBJ, NULL },        /* 3 */
	{ "-offValue", GNOCL_OBJ, NULL },       /* 4 */
	{ "-variable", GNOCL_STRING, NULL },    /* 5 */
	{ "-active", GNOCL_BOOL, NULL },        /* 6 */
	{ "-value", GNOCL_OBJ, NULL },          /* 7 */
	{ "-position", GNOCL_OBJ, NULL },		/* 8 */

	{ "-data", GNOCL_OBJ, "", gnoclOptData },
	{ "-name", GNOCL_STRING, "name" },
	{ "-onButtonPress", GNOCL_OBJ, "P", gnoclOptOnButton },
	{ "-onButtonRelease", GNOCL_OBJ, "R", gnoclOptOnButton },
	{ "-onShowHelp", GNOCL_OBJ, "", gnoclOptOnShowHelp },
	{ "-sensitive", GNOCL_BOOL, "sensitive" },
	{ "-tooltip", GNOCL_OBJ, "", gnoclOptTooltip },
	{ "-visible", GNOCL_BOOL, "visible" },
	{ NULL, 0, 0 }
};


/**
\brief
**/
/*
   -------------- check functions ----------------------------
*/
void checkDestroyFunc ( GtkWidget *widget, gpointer data )
{
	GnoclToolBarCheckParams *para = ( GnoclToolBarCheckParams * ) data;

	gnoclAttachVariable ( NULL, &para->variable, "toggled", G_OBJECT ( para->item ), G_CALLBACK ( gnoclCheckToggledFunc ), para->interp, gnoclCheckTraceFunc, para );

	gnoclForgetWidgetFromName ( para->name );
	Tcl_DeleteCommand ( para->interp, para->name );
	g_free ( para->name );

	Tcl_DecrRefCount ( para->onValue );
	Tcl_DecrRefCount ( para->offValue );
	g_free ( para );
}

/**
\brief
**/
static int checkCget ( Tcl_Interp *interp, GnoclToolBarCheckParams *para, GnoclOption options[], int idx )
{

	Tcl_Obj *obj = NULL;

	if ( idx == checkTextIdx )
	{
		obj = cgetText ( para->item );
	}

	else if ( idx == checkIconIdx )
	{
		; /* TODO */
	}

	else if ( idx == checkOnToggledIdx )
	{
		obj = Tcl_NewStringObj ( para->onToggled ? para->onToggled : "", -1 );
	}

	else if ( idx == checkOnValueIdx )
	{
		obj = para->onValue;
	}

	else if ( idx == checkOffValueIdx )
	{
		obj = para->offValue;
	}

	else if ( idx == checkVariableIdx )
	{
		obj = Tcl_NewStringObj ( para->variable, -1 );
	}

	else if ( idx == checkActiveIdx )
	{
		gboolean on;
		g_object_get ( G_OBJECT ( para->item ), "active", &on, NULL );
		obj = Tcl_NewBooleanObj ( on );
	}

	else if ( idx == checkValueIdx )
	{
		gboolean on;
		g_object_get ( G_OBJECT ( para->item ), "active", &on, NULL );
		obj = on ? para->onValue : para->offValue;
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
static int checkConfigure ( Tcl_Interp *interp, GnoclToolBarCheckParams *para, GnoclOption options[] )
{
	if ( options[checkPositionIdx].status == GNOCL_STATUS_CHANGED )
	{

		gint pos;
		GtkWidget *toolBar;

		Tcl_GetIntFromObj ( NULL, options[checkPositionIdx].val.obj, &pos );
		toolBar = gtk_widget_get_parent ( para->item );

		// gtk_widget_unparent (para->item);
		g_object_ref ( para->item );
		gtk_container_remove ( GTK_CONTAINER ( toolBar ), para->item );

		gtk_toolbar_insert ( toolBar, para->item, pos );
	}


	if ( options[checkOnValueIdx].status == GNOCL_STATUS_CHANGED )
	{
		GNOCL_MOVE_OBJ ( options[checkOnValueIdx].val.obj, para->onValue );
	}

	if ( options[checkOffValueIdx].status == GNOCL_STATUS_CHANGED )
	{
		GNOCL_MOVE_OBJ ( options[checkOffValueIdx].val.obj, para->offValue );
	}

//	if ( options[checkOnToggledIdx].status == GNOCL_STATUS_CHANGED )
//	{
//		g_print ( "checkConfigure checkOnToggledIdx\n" );

	//GNOCL_MOVE_STRING ( options[checkOnToggledIdx].val.str, para->onToggled );

	gnoclAttachOptCmdAndVar (
		&options[checkOnToggledIdx], &para->onToggled,
		&options[checkVariableIdx], &para->variable,
		"toggled", G_OBJECT ( para->item ),
		GTK_SIGNAL_FUNC ( gnoclToggleToggledFunc ),
		para->interp, gnoclToggleTraceFunc, para );
//	}

	if ( gnoclCheckSetActive ( para, &options[checkActiveIdx] ) == 0
			&& ( options[checkVariableIdx].status == GNOCL_STATUS_CHANGED
				 || options[checkOnValueIdx].status == GNOCL_STATUS_CHANGED
				 || options[checkOffValueIdx].status == GNOCL_STATUS_CHANGED ) )
	{
		gnoclCheckVariableValueChanged ( para );
	}

	if ( options[checkValueIdx].status == GNOCL_STATUS_CHANGED )
	{
		if ( gnoclCheckSetValue ( para, options[checkValueIdx].val.obj ) != TCL_OK )
			return TCL_ERROR;
	}

	return TCL_OK;
}



/**
\brief
**/
static int checktoolButtonFunc ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{
#if 1
	g_print ( "%s\n", __FUNCTION__ );
#endif

	static const char *cmds[] = { "delete", "configure", "cget", "onToggled", "class", NULL };
	enum cmdIdx { DeleteIdx, ConfigureIdx, CgetIdx, OnToggledIdx, ClassIdx };
	GnoclToolBarCheckParams *para = ( GnoclToolBarCheckParams * ) data;
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
				Tcl_SetObjResult ( interp, Tcl_NewStringObj ( "toolBarCheckButton", -1 ) );
			}
			break;
		case DeleteIdx:
			{
				return gnoclDelete ( interp, para->item, objc, objv );
			}
		case ConfigureIdx:
			{
				int ret = TCL_ERROR;

				if ( gnoclParseAndSetOptions ( interp, objc - 1, objv + 1, checkOptions, G_OBJECT ( para->item ) ) == TCL_OK )
				{
					int k;

					for ( k = 0; k < checkOnToggledIdx; ++k )
					{
						if ( checkOptions[k].status == GNOCL_STATUS_CHANGED )
						{
							Tcl_AppendResult ( interp, "Option \"", checkOptions[k].optName, "\" cannot be set after widget creation.", NULL );
							gnoclClearOptions ( checkOptions );
							return TCL_ERROR;
						}
					}

					//ret = checkConfigure ( interp, para, checkOptions );
				}

				gnoclClearOptions ( checkOptions );

				return ret;
			}

			break;

		case CgetIdx:
			{
				int     idx;

				switch ( gnoclCget ( interp, objc, objv, G_OBJECT ( para->item ), checkOptions, &idx ) )
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
							//return checkCget ( interp, para, checkOptions, idx );
						}
				}
			}

		case OnToggledIdx:
			{
				// g_print ("OnToggledIdx\n");
				//return gnoclCheckOnToggled ( interp, objc, objv, para );
			}
	}

	return TCL_OK;
}



/**
\brief	Same as toggleButton
**/
int addCheckButton ( GtkToolbar *toolbar, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[], int pos )
{

	int              ret;
	GnoclToolBarCheckParams *para = NULL;
	char             *txt = NULL;
	int              isUnderline;
	GtkWidget        *icon;

	if ( gnoclParseOptions ( interp, objc - 2, objv + 2, checkOptions ) != TCL_OK )
	{
		gnoclClearOptions ( checkOptions );
		return TCL_ERROR;
	}



	if ( getTextAndIcon ( interp, toolbar, checkOptions + checkTextIdx, checkOptions + checkIconIdx, &txt, &icon, &isUnderline ) != TCL_OK )
	{
		return TCL_ERROR;
	}


	para = g_new ( GnoclToolBarCheckParams, 1 );

	para->onToggled = NULL;
	para->interp = interp;
	para->name = gnoclGetAutoWidgetId();
	para->variable = NULL;
	para->onValue = Tcl_NewIntObj ( 1 );

	Tcl_IncrRefCount ( para->onValue );

	para->offValue = Tcl_NewIntObj ( 0 );

	Tcl_IncrRefCount ( para->offValue );

	para->inSetVar = 0;

	/* this is the requirement after Gtk+ 2.4 */
	//para->item = gtk_tool_button_new_from_stock ( GTK_STOCK_COPY );
	//para->widget =   gtk_toggle_tool_button_new_from_stock ( GTK_STOCK_COPY );

	//GtkToggleAction *   gtk_toggle_action_new (const gchar *name, const gchar *label, const gchar *tooltip, const gchar *stock_id);

	/* create base object then add icon and label widgets */
	para->item = gtk_toggle_tool_button_new ();

	const char *dataID = "gnocl::para";
	g_object_set_data ( G_OBJECT ( para->item ), dataID, para );

	gtk_tool_button_set_icon_widget ( para->item, icon );
	gtk_widget_show ( icon );
	gtk_tool_button_set_label ( para->item, txt );


	//gtk_tool_button_set_label_widget (para->item, txt);
	//g_signal_connect ( para->item , "toggled", G_CALLBACK ( gnoclCheckToggledFunc ), para );

	gtk_toolbar_insert ( GTK_TOOLBAR ( toolbar ),  para->item, pos );
	gtk_widget_show ( GTK_WIDGET (  para->item ) );

	if ( isUnderline )
	{
		//setUnderline ( para->item );
	}

	ret = gnoclSetOptions ( interp, checkOptions, G_OBJECT ( para->item ), -1 );

	if ( ret == TCL_OK )
	{
		ret = checkConfigure ( interp, para, checkOptions );
	}

	if ( ret != TCL_OK )
	{
		g_free ( para );
	}

	else
	{
		g_signal_connect_after ( G_OBJECT ( para->item ), "destroy", G_CALLBACK ( checkDestroyFunc ), para );

		gnoclMemNameAndWidget ( para->name, para->item );

		Tcl_CreateObjCommand ( interp, para->name, checktoolButtonFunc, para, NULL );
		Tcl_SetObjResult ( interp, Tcl_NewStringObj ( para->name, -1 ) );
	}

	gnoclClearOptions ( checkOptions );

	return ret;
}
