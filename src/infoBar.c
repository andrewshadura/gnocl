/**
\brief         This module implements the gnocl::infoBar widget.
**/

/*
   History:
   2010-10: Begin of developement
 */

/**
\page page_infoBar gnocl::infoBar
\htmlinclude infoBar.html
**/

#include "gnocl.h"
#include <string.h>
#include <assert.h>

/**
\brief
**/
static gboolean doOnClose ( GtkInfoBar *infobar, gpointer user_data )
{
	GnoclCommandData *cs = ( GnoclCommandData * ) user_data;

	//if ( *cs->interp->result == '\0' )
	//{
	GnoclPercSubst ps[] =
	{
		{ 'w', GNOCL_STRING },  /* printiconview */
		{ 0 }
	};

	ps[0].val.str = gnoclGetNameFromWidget ( infobar );
	gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 );
	//}
}

/**
\brief      This is for all callbacks which have only %w as substitution
**/
static int gnoclOptOnClose ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	assert ( opt->type == GNOCL_OBJ );

	return gnoclConnectOptCmd ( interp, obj, opt->propName, G_CALLBACK ( doOnClose ), opt, NULL, ret );
}

/**
\brief
**/
static gboolean doOnResponse ( GtkInfoBar *infobar, gint response_id, gpointer user_data )
{

	GnoclCommandData *cs = ( GnoclCommandData * ) user_data;

	//if ( *cs->interp->result == '\0' )
	//{
	GnoclPercSubst ps[] =
	{
		{ 'w', GNOCL_STRING },
		{ 'd', GNOCL_INT },
		{ 0 }
	};

	ps[0].val.str = gnoclGetNameFromWidget ( infobar );
	ps[1].val.i = response_id;
	gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 );
	//}
}

/**
\brief      This is for all callbacks which have only %w as substitution
**/
static int gnoclOptOnResponse ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	assert ( opt->type == GNOCL_OBJ );

	return gnoclConnectOptCmd ( interp, obj, opt->propName, G_CALLBACK ( doOnResponse ), opt, NULL, ret );
}

static const int childIdx = 0;
static const int typeIdx = 1;
static const int defaultIdx = 2;
static const int dataIdx = 3;

static GnoclOption infoBarOptions[] =
{
	/* set in configure */
	{ "-child", GNOCL_STRING, NULL },
	{ "-type", GNOCL_STRING, NULL },
	{ "-default", GNOCL_STRING, NULL },
	{ "-data", GNOCL_OBJ, "", gnoclOptData },

	/* signal handlers */
	{ "-onClose", GNOCL_OBJ, "close", gnoclOptOnClose },
	{ "-onResponse", GNOCL_OBJ, "response", gnoclOptOnResponse },

	/* inherited options */
	{ "-visible", GNOCL_BOOL, "visible" },
	{ "-name", GNOCL_STRING, "name" },
	{ "-onShowHelp", GNOCL_OBJ, "", gnoclOptOnShowHelp },

	{ NULL },
};


/**
/brief
**/
static int configure ( Tcl_Interp *interp, GtkWidget *infobar, GnoclOption options[] )
{
	if ( options[childIdx].status == GNOCL_STATUS_CHANGED )
	{
#ifdef DEBUG_INFOBAR
		g_print ( "CONFIGURE -> child -> %s\n", options[childIdx].val.str );
#endif

		GtkWidget *content_area;
		GtkWidget *child;

		child = gnoclGetWidgetFromName ( options[childIdx].val.str, interp );

		content_area = gtk_info_bar_get_content_area ( GTK_INFO_BAR ( infobar ) );
		gtk_container_add ( GTK_CONTAINER ( content_area ), child );

	}

	if ( options[typeIdx].status == GNOCL_STATUS_CHANGED )
	{
#ifdef DEBUG_INFOBAR
		g_print ( "CONFIGURE -> type -> %s\n", options[typeIdx].val.str );
#endif
		const char *orientation[] =
		{
			"info", "warning",
			"question", "error",
			"other",
			NULL
		};

		gint idx;

		getIdx ( orientation, options[typeIdx].val.str, &idx );

		switch ( idx )
		{
			case GTK_MESSAGE_INFO:
			case GTK_MESSAGE_WARNING:
			case GTK_MESSAGE_QUESTION:
			case GTK_MESSAGE_ERROR:
			case GTK_MESSAGE_OTHER:
				{
					gtk_info_bar_set_message_type ( GTK_INFO_BAR ( infobar ), idx );
				} break;
			default:
				{
					return TCL_ERROR;

				}
		}

	}

	if ( options[defaultIdx].status == GNOCL_STATUS_CHANGED )
	{
#ifdef DEBUG_INFOBAR
		g_print ( "CONFIGURE -> default -> %s\n", options[defaultIdx].val.str );
#endif
		gint id;

		sscanf ( options[defaultIdx].val.str, "%d", &id );

		gtk_info_bar_set_default_response ( GTK_INFO_BAR ( infobar ), id );

	}

	/*
		// template
		if ( options[selectionModeIdx].status == GNOCL_STATUS_CHANGED )
		{
		#ifdef DEBUG_INFOBAR
			g_print ( "CONFIGURE -> selectionMode -> %s\n", options[selectionModeIdx].val.str );
		#endif
		}
	*/
	return TCL_OK;
}

/**
/brief
**/
static int cget ( Tcl_Interp *interp, GtkWidget *widget, GnoclOption options[], int idx )
{

	return gnoclCgetNotImplemented ( interp, options + idx );
}

/**
/brief
**/
static int infobarFunc ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{

#ifdef DEBUG_INFOBAR
	listParameters ( objc, objv, "infobarFunc" );
#endif

	static const char *cmds[] =
	{
		"add", "item", "reponse",
		"delete", "configure", "cget",
		"onClicked", "class",
		NULL
	};

	enum cmdIdx
	{
		AddIdx, ItemIdx, ResponseIdx,
		DeleteIdx, ConfigureIdx, CgetIdx,
		OnClickedIdx, ClassIdx
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
		case ResponseIdx:
			{
				/* emit response signal for item */

				int id;
				sscanf ( Tcl_GetString ( objv[2] ), "%d", &id );
				gtk_info_bar_response ( widget, id );
			}
			break;
		case ItemIdx:
			{
				/* configure valid item settings */
			}
			break;
		case AddIdx:
			{

				if ( objc < 2 )
				{
					Tcl_WrongNumArgs ( interp, 1, objv, "" );
					return TCL_ERROR;
				}

				/* need to create an 'id' counter */
				static int no = 0;
				gchar str[4];
				GtkWidget *item;

				const char *items[] =
				{
					"button", "widget",
					NULL
				};

				enum itemIdx
				{
					buttonIdx, widgetIdx
				};

				gint idx;

				getIdx ( items, Tcl_GetString ( objv[2] ), &idx );


				switch ( idx )
				{
					case buttonIdx:
						{
							/*
							GtkWidget *gtk_info_bar_add_button(GtkInfoBar *info_bar,const gchar *button_text,gint response_id);
							*/
							item = gtk_info_bar_add_button ( widget, Tcl_GetString ( objv[3] ), ++no );
							gtk_info_bar_set_response_sensitive ( widget, no, 1 );

							/* create a default option flag */
							//if ( strcmp(Tcl_GetString ( objv[4] ),"-default") == 0 && strcmp(Tcl_GetString ( objv[5] ),"1") == 0 )  {
							//	g_print("set default\n");
							//	gtk_info_bar_set_default_response  ( widget, no );
							//}


						} break;
					case widgetIdx:
						{
							GtkWidget *child;
							child = gnoclGetWidgetFromName ( Tcl_GetString ( objv[3] ), interp );
							gtk_info_bar_add_action_widget ( widget, child, ++no );
							gtk_info_bar_set_response_sensitive ( widget, no, 1 );
						} break;
					default:
						{
#ifdef DEBUG_INFOBAR
							g_print ( "NOPE!\n" );
#endif
						}
				}


				sprintf ( str, "%d", no );

				Tcl_SetObjResult ( interp, Tcl_NewStringObj ( str, -1 ) );

			}
			break;
		case ClassIdx:
			Tcl_SetObjResult ( interp, Tcl_NewStringObj ( "infoBar", -1 ) );
			break;
		case DeleteIdx:
			return gnoclDelete ( interp, GTK_WIDGET ( widget ), objc, objv );

		case ConfigureIdx:
			{
				int ret = TCL_ERROR;

				ret = configure ( interp, widget, infoBarOptions );

				if ( 1 )
				{
					if ( gnoclParseAndSetOptions ( interp, objc - 1, objv + 1, infoBarOptions, G_OBJECT ( widget ) ) == TCL_OK )
					{
						ret = configure ( interp, widget, infoBarOptions );
					}
				}

				gnoclClearOptions ( infoBarOptions );

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

				switch ( gnoclCget ( interp, objc, objv, G_OBJECT ( widget ), infoBarOptions, &idx ) )
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
							return cget ( interp, widget, infoBarOptions, idx );
						}
				}
			}
	}/*

   History:
   2009-12: adapted for use in glade files.
   2008-10: added parent command
   2008-10: added class command
   2002-10: switched from GnoclWidgetOptions to GnoclOption
   2001-06: Begin of developement
 */

	return TCL_OK;
}

/**
/brief
**/
int gnoclInfoBarCmd ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{

	int            ret = TCL_OK;
	GtkWidget      *infobar;


	/* STEP 1) -Check the options */
	if ( gnoclParseOptions ( interp, objc, objv, infoBarOptions ) != TCL_OK )
	{
		gnoclClearOptions ( infoBarOptions );
		return TCL_ERROR;
	}


	/* STEP 2) -Create and configure the widget */
	infobar = gtk_info_bar_new ();

	ret = gnoclSetOptions ( interp, infoBarOptions, G_OBJECT ( infobar ), -1 );

	if ( ret == TCL_OK )
	{
		ret = configure ( interp, G_OBJECT ( infobar ), infoBarOptions );
	}

	gnoclClearOptions ( infoBarOptions );


	/* STEP 3)  -show the widget */
	gtk_widget_show ( infobar );


	if ( ret != TCL_OK )
	{
		gtk_widget_destroy ( GTK_WIDGET ( infobar ) );
		return TCL_ERROR;
	}

	/* STEP 4)  -everything has worked, register the widget with the Tcl interpretor. */
	return gnoclRegisterWidget ( interp, GTK_WIDGET ( infobar ), infobarFunc );
}
