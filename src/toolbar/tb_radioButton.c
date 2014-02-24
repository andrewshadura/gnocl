/*
 * tb_radioButton.c
 *
 * This file implements the toolbar radio button item.
 *
 * Copyright (c) 2001 - 2005 Peter G. Baum  http://www.dr-baum.net
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

/*
   History:
   2014-01: moved toolbar item creation functions into own modules
*/

#include "../gnocl.h"

static int radiotoolButtonFunc ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] );

/* radioButton options */
static const int radioTextIdx      = 0;
static const int radioIconIdx      = 1;
static const int radioOnToggledIdx = 2;
static const int radioOnValueIdx   = 3;
static const int radioOffValueIdx  = 4;
static const int radioVariableIdx  = 5;
static const int radioActiveIdx    = 6;
static const int radioValueIdx     = 7;
static const int radioPositionIdx  = 8;

static GnoclOption radioOptions[] =
{
	/* gnocl specific options */
	{ "-text", GNOCL_OBJ, NULL },           /* 0 */
	{ "-icon", GNOCL_OBJ, NULL },           /* 1 */
	{ "-onToggled", GNOCL_STRING, NULL },   /* 2 the order is important! */
	{ "-onValue", GNOCL_OBJ, NULL },        /* 3 */
	{ "-offValue", GNOCL_OBJ, NULL },       /* 4 */
	{ "-variable", GNOCL_STRING, NULL },    /* 5 */
	{ "-active", GNOCL_BOOL, NULL },        /* 6 */
	{ "-value", GNOCL_OBJ, NULL },          /* 7 */
	{ "-position", GNOCL_OBJ, NULL },

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

#if 1
/**
\brief
**/
static int radioCget ( Tcl_Interp *interp, GnoclRadioParams *para, GnoclOption options[], int idx )
{


	Tcl_Obj *obj = NULL;

	if ( idx == radioTextIdx )
	{
		obj = cgetText ( para->widget );
	}

	else if ( idx == radioIconIdx )
	{
		; /* TODO */
	}

	else if ( idx == radioOnToggledIdx )
	{
		obj = Tcl_NewStringObj ( para->onToggled ? para->onToggled : "", -1 );
	}

	else if ( idx == radioOnValueIdx )
	{
		GnoclRadioParams *p = gnoclRadioGetActivePara ( para->group );
		obj = p->onValue;
	}

	else if ( idx == radioVariableIdx )
	{
		obj = Tcl_NewStringObj ( para->group->variable, -1 );
	}

	else if ( idx == radioActiveIdx )
	{
		gboolean on;
		g_object_get ( G_OBJECT ( para->widget ), "active", &on, NULL );
		obj = Tcl_NewBooleanObj ( on );
	}

	else if ( idx == radioValueIdx )
	{
		obj = gnoclRadioGetValue ( para );
	}

	if ( obj != NULL )
	{
		Tcl_SetObjResult ( interp, obj );
		return TCL_OK;
	}

	return gnoclCgetNotImplemented ( interp, options + idx );
}
#endif


/**
\brief
**/
/*
   -------------- radio functions ----------------------------
*/
static int radioConfigure ( Tcl_Interp *interp, GnoclRadioParams *para, GnoclOption options[] )
{

	if ( options[radioPositionIdx].status == GNOCL_STATUS_CHANGED )
	{

		gint pos;
		GtkWidget *toolBar;

		Tcl_GetIntFromObj ( NULL, options[radioPositionIdx].val.obj, &pos );
		toolBar = gtk_widget_get_parent ( para->widget );

		// gtk_widget_unparent (para->item);
		g_object_ref ( para->widget );
		gtk_container_remove ( GTK_CONTAINER ( toolBar ), para->widget );

		gtk_toolbar_insert ( toolBar, para->widget, pos );
	}

	/* set an initial value for the radio button group */
	if ( gnoclRadioSetValueActive ( para, &options[radioOnValueIdx], &options[radioActiveIdx] ) != TCL_OK )
	{
		return TCL_ERROR;
	}

	/* when toggled on/off */
	if ( options[radioOnToggledIdx].status == GNOCL_STATUS_CHANGED )
	{
		GNOCL_MOVE_STRING ( options[radioOnToggledIdx].val.str, para->onToggled );
	}

	if ( options[radioValueIdx].status == GNOCL_STATUS_CHANGED )
	{
		if ( gnoclRadioSetValue ( para, options[radioValueIdx].val.obj ) != TCL_OK )
		{
			return TCL_ERROR;
		}
	}

	if ( options[radioValueIdx].status == GNOCL_STATUS_CHANGED )
	{
		if ( gnoclRadioSetValue ( para, options[radioValueIdx].val.obj ) != TCL_OK )
		{
			return TCL_ERROR;
		}
	}

	return TCL_OK;
}

/**
\brief
**/
#if 0
static int radiotoolButtonFunc ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{
	print ( "%s\n", __FUNCTION__ );


	static const char *cmds[] = { "delete", "configure", "cget", "onToggled", "class", NULL };
	enum cmdIdx { DeleteIdx, ConfigureIdx, CgetIdx, OnToggledIdx, ClassIdx};
	GnoclRadioParams *para = ( GnoclRadioParams * ) data;
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
				Tcl_SetObjResult ( interp, Tcl_NewStringObj ( "toolBarRadioButton", -1 ) );
			}
			break;
		case DeleteIdx:
			{
				return gnoclDelete ( interp, para->widget, objc, objv );
			}
		case ConfigureIdx:
			{
				int ret = TCL_ERROR;

				if ( gnoclParseAndSetOptions ( interp, objc - 1, objv + 1, radioOptions, G_OBJECT ( para->widget ) ) == TCL_OK )
				{
					int k;

					for ( k = 0; k < radioActiveIdx; ++k )
					{
						if ( radioOptions[k].status == GNOCL_STATUS_CHANGED )
						{
							Tcl_AppendResult ( interp, "Option \"", radioOptions[k].optName, "\" cannot be set after widget creation.", NULL );
							gnoclClearOptions ( radioOptions );
							return TCL_ERROR;
						}
					}

					ret = radioConfigure ( interp, para, radioOptions );
				}

				gnoclClearOptions ( radioOptions );

				return ret;
			}

			break;

		case CgetIdx:
			{
				int     idx;

				switch ( gnoclCget ( interp, objc, objv, G_OBJECT ( para->widget ), radioOptions, &idx ) )
				{
					case GNOCL_CGET_ERROR:
						return TCL_ERROR;
					case GNOCL_CGET_HANDLED:
						return TCL_OK;
					case GNOCL_CGET_NOTHANDLED:
						return radioCget ( interp, para, radioOptions, idx );
				}
			}

		case OnToggledIdx:
			return gnoclRadioOnToggled ( interp, objc, objv, para );
	}

	return TCL_OK;
}
#endif



/**
\brief
**/
int addRadioButton ( GtkToolbar *toolbar, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[], int pos )
{
	int              ret;
	GnoclRadioParams *para = NULL;
	char             *txt = NULL;
	int              isUnderline;
	GtkWidget        *icon;
	GtkWidget        *firstWidget = NULL;

	GSList* list = NULL;

	/* check options */
	if ( gnoclParseOptions ( interp, objc - 2, objv + 2, radioOptions ) != TCL_OK )
	{
		gnoclClearOptions ( radioOptions );
		return TCL_ERROR;
	}

	g_print ( "%s HERE 1\n", __FUNCTION__ );

	/* Confirm that the mandatory -value or -variable options have been set. */
	if ( radioOptions[radioOnValueIdx].status != GNOCL_STATUS_CHANGED || radioOptions[radioVariableIdx].status != GNOCL_STATUS_CHANGED )
	{
		gnoclClearOptions ( radioOptions );
		Tcl_SetResult ( interp, "Option \"-onValue\" and \"-variable\" are required.", TCL_STATIC );
		return TCL_ERROR;
	}

	g_print ( "%s HERE 2\n", __FUNCTION__ );

	/* Create the toobutton item and assign text label and icon. */
	if ( getTextAndIcon ( interp, toolbar, radioOptions + radioTextIdx, radioOptions + radioIconIdx, &txt, &icon, &isUnderline ) != TCL_OK )
	{
		return TCL_ERROR;
	}

	g_print ( "%s HERE 3\n", __FUNCTION__ );

	/* Initialize object parameters. */
	para = g_new ( GnoclRadioParams, 1 );
	para->name = gnoclGetAutoWidgetId();
	para->onToggled = NULL;
	para->onValue = NULL;
	para->group = gnoclRadioGetGroupFromVariable ( radioOptions[radioVariableIdx].val.str );

	g_print ( "%s HERE 4\n", __FUNCTION__ );

	/* create new group if necessary */
	if ( para->group == NULL )
	{
		para->group = gnoclRadioGroupNewGroup ( radioOptions[radioVariableIdx].val.str, interp );
		para->widget = gtk_radio_tool_button_new ( NULL );

	}
	else
	{
		GnoclRadioParams *p = gnoclRadioGetParam ( para->group, 0 );
		firstWidget = p->widget;
		para->widget =  gtk_radio_tool_button_new_from_widget ( firstWidget );
	}

	g_print ( "%s HERE 5\n", __FUNCTION__ );

	/* should this be moved elsewhere? */
	g_signal_connect ( para->widget , "toggled", G_CALLBACK ( gnoclRadioToggledFunc ), para );

	g_print ( "%s HERE 6\n", __FUNCTION__ );

	/* use custom icon and label widgets */
	gtk_tool_button_set_icon_widget ( para->widget, icon );
	gtk_widget_show ( icon );
	gtk_tool_button_set_label ( para->widget, txt );

	g_print ( "%s HERE 7\n", __FUNCTION__ );

	/* add widget to toolbar and display */
	gtk_toolbar_insert ( GTK_TOOLBAR ( toolbar ),  para->widget, pos );
	gtk_widget_show ( GTK_WIDGET (  para->widget ) );

	g_print ( "%s HERE 8\n", __FUNCTION__ );

	/*-----*/
	if ( isUnderline )
	{
		setUnderline ( para->widget );
	}

	g_print ( "%s HERE 9\n", __FUNCTION__ );

	/* add widget to group */
	gnoclRadioGroupAddWidgetToGroup ( para->group, para );

	g_print ( "%s HERE 10\n", __FUNCTION__ );

	/* configure the radiobutton */
	if ( gnoclSetOptions ( interp, radioOptions, G_OBJECT ( para->widget ), -1 ) == TCL_OK )
	{
		ret = radioConfigure ( interp, para, radioOptions );

	}

	g_print ( "%s HERE 11 ; para->name = %s\n", __FUNCTION__, para->name );

	if ( ret != TCL_OK )
	{
		g_free ( para );
	}

	else
	{
		g_signal_connect_after ( G_OBJECT ( para->widget ), "destroy", G_CALLBACK ( gnoclRadioDestroyFunc ), para );

		gnoclMemNameAndWidget ( para->name, para->widget );

		/* PROBLEMS START FROM HERE!!!! */

		//Tcl_CreateObjCommand ( interp, para->name, radiotoolButtonFunc, para, NULL );
		Tcl_SetObjResult ( interp, Tcl_NewStringObj ( para->name, -1 ) );

	}

	g_print ( "%s HERE 12\n", __FUNCTION__ );

	gnoclClearOptions ( radioOptions );

	g_print ( "%s HERE 13\n", __FUNCTION__ );

	return ret;
}
