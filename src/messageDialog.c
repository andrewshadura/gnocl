/*
 * $Id: dialog.c,v 1.16 2005/06/02 20:45:58 baum Exp $
 *
 * This file implements the gtk dialog and messageDialog
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
   2012-04: added -height, -width, -x and -y options
   2012-01: added "other" to the range of dialog types, i.e. no icon.
            implemented automatic setting of type to "other" if -child option set.
   2011-12: -onReponse event handler, new % sustitution parameter
				 %r (response identifier)
				 %d  widget data
			new option -data
   2009-12: module renamed back to messageDialog
   2009-12: added -visible
        10: added onKey{Press,Release}
        06: renamed from messageDialog to dialog,
            added options -child, -icon, -title
            added (if non modal) command destroy
        02: dialog is closed on "break"
   2003-01: switched from GnoclWidgetOptions to GnoclOption
            only message dialog, dialog gets its own file
   2002-05: moved from gnome dialog to gtk 2.0 dialog
   2001-05: Begin of developement
 */

/**
\page page_dialog gnocl::dialog
\htmlinclude dialog.html
**/

#include "gnocl.h"
//#include "gnoclparams.h"

static const int textIdx          = 0;
static const int childIdx         = 1;
static const int buttonsIdx       = 2;
static const int defaultButtonIdx = 3;
static const int typeIdx          = 4;
static const int modalIdx         = 5;
static const int onResponseIdx    = 6;
static const int setSizeIdx       = 7;
static const int xIdx           = 8;
static const int yIdx           = 9;
static const int widthIdx       = 10;
static const int heightIdx      = 11;

/* todo, organise ioptions to widget specific, followed by inherited */
static GnoclOption dialogOptions[] =
{
	{ "-text", GNOCL_STRING, NULL },           /* 0 */
	{ "-child", GNOCL_STRING, NULL },          /* 1 */
	{ "-buttons", GNOCL_LIST, NULL },          /* 2 */
	{ "-defaultButton", GNOCL_INT, NULL },     /* 3 */
	{ "-type", GNOCL_OBJ, NULL },              /* 4 */
	{ "-modal", GNOCL_BOOL, NULL },            /* 5 */
	{ "-onResponse", GNOCL_STRING, NULL },     /* 6 */
	{ "-setSize", GNOCL_DOUBLE, NULL },        /* 7 */

	{ "-x", GNOCL_INT, NULL },                           /* 8 */
	{ "-y", GNOCL_INT, NULL },                           /* 9 */
	{ "-width", GNOCL_INT, NULL },                       /* 10 */
	{ "-height", GNOCL_INT, NULL },                      /* 11 */


	{ "-onClose", GNOCL_OBJ, "close", gnoclOptCommand },

	{ "-allowGrow", GNOCL_BOOL, "allow-grow" },
	{ "-allowShrink", GNOCL_BOOL, "allow-shrink" },
	{ "-defaultHeight", GNOCL_INT, "default-height" },
	{ "-defaultWidth", GNOCL_INT, "default-width" },
	{ "-dragTargets", GNOCL_LIST, "s", gnoclOptDnDTargets },
	{ "-dropTargets", GNOCL_LIST, "t", gnoclOptDnDTargets },
	{ "-icon", GNOCL_OBJ, "", gnoclOptIcon },
	{ "-name", GNOCL_STRING, "name" },
	{ "-onDelete", GNOCL_OBJ, "", gnoclOptOnDelete },
	{ "-onDestroy", GNOCL_OBJ, "destroy", gnoclOptCommand },
	{ "-onDragData", GNOCL_OBJ, "", gnoclOptOnDragData },
	{ "-onDropData", GNOCL_OBJ, "", gnoclOptOnDropData },
	{ "-onKeyPress", GNOCL_OBJ, "", gnoclOptOnKeyPress },
	{ "-onKeyRelease", GNOCL_OBJ, "", gnoclOptOnKeyRelease },
	{ "-onPopupMenu", GNOCL_OBJ, "popup-menu", gnoclOptCommand },
	{ "-onRealize", GNOCL_OBJ, "realize", gnoclOptCommand },
	{ "-onResize", GNOCL_OBJ, "", gnoclOptOnConfigure },
	{ "-onShowHelp", GNOCL_OBJ, "", gnoclOptOnShowHelp },
	{ "-resizable", GNOCL_BOOL, "resizable" },
	{ "-sensitive", GNOCL_BOOL, "sensitive" },
	{ "-title", GNOCL_STRING, "title"},
	{ "-typeHint", GNOCL_OBJ, "", gnoclOptWindowTypeHint },
	{ "-tooltip", GNOCL_OBJ, "", gnoclOptTooltip },
	{ "-visible", GNOCL_BOOL, "visible" },
	{ "-data", GNOCL_OBJ, "", gnoclOptData },

	/* inherited windows option */
	{ "-keepAbove", GNOCL_OBJ, "", gnoclOptKeepAbove },
	{ "-keepBelow", GNOCL_OBJ, "", gnoclOptKeepBelow },
	{ "-acceptFocus", GNOCL_BOOL, "accept-focus" },


	{ NULL }
};

/* moved to gnocl.h */
/*
typedef struct
{
    char       *onResponse;
    char       *name;
    GtkDialog  *dialog;
    Tcl_Interp *interp;
    GPtrArray  *butRes;
    int        isModal;
    int        ret;
} DialogParams;
*/

/**
\brief
**/
static int configure ( Tcl_Interp *interp, GtkAboutDialog *dialog, GnoclOption options[] )
{

	/* resize window in proportion to the screen */
	if ( dialogOptions[setSizeIdx].status == GNOCL_STATUS_CHANGED )
	{

		gint width = 0;
		gint height = 0;
		GdkScreen *screen;

		screen = gdk_screen_get_default();

		width = gdk_screen_get_width ( screen );
		height = gdk_screen_get_height ( screen );

//		g_print ( "size = %f\n", dialogOptions[setSizeIdx].val.d );
//		g_print ( "w = %f ; h = %f\n", dialogOptions[setSizeIdx].val.d * ( float ) width , dialogOptions[setSizeIdx].val.d * ( float ) height );

		float w, h;

		w = dialogOptions[setSizeIdx].val.d * ( float ) width;
		h =  dialogOptions[setSizeIdx].val.d * ( float ) height;

		width = ( gint ) w;
		height = ( gint ) h;

//		g_print ( "width = %d ; height = %d\n", width, height );

		gtk_window_resize ( dialog, width, height );

	}


	return TCL_OK;
}


/**
\brief
**/
static int getType ( Tcl_Interp *interp, GnoclOption *opt, int *type )
{
	const char *txt[] =
	{
		"info", "warning",
		"question", "error",
		"other",
		NULL
	};

	const int types[] =
	{
		GTK_MESSAGE_INFO, GTK_MESSAGE_WARNING,
		GTK_MESSAGE_QUESTION, GTK_MESSAGE_ERROR,
		GTK_MESSAGE_OTHER
	};

	if ( opt->status == GNOCL_STATUS_CHANGED )
	{
		int idx;

		if ( Tcl_GetIndexFromObj ( interp, opt->val.obj, txt, "dialog type", TCL_EXACT, &idx ) != TCL_OK )
		{
			return TCL_ERROR;
		}

		*type = types[idx];
	}

	else
	{
		*type = GTK_MESSAGE_INFO;
	}

	return TCL_OK;
}

/**
\brief
**/
static Tcl_Obj *getObjFromRet ( DialogParams *para, int ret )
{
	switch ( ret )
	{
		case GTK_RESPONSE_NONE:
			return Tcl_NewStringObj ( "#NONE", -1 );

		case GTK_RESPONSE_DELETE_EVENT:
			return Tcl_NewStringObj ( "#DELETE", -1 );

		case GTK_RESPONSE_ACCEPT:
			return Tcl_NewStringObj ( "#ACCEPT", -1 );

		case GTK_RESPONSE_OK:
			return Tcl_NewStringObj ( "#OK", -1 );

		case GTK_RESPONSE_CANCEL:
			return Tcl_NewStringObj ( "#CANCEL", -1 );

		case GTK_RESPONSE_CLOSE:
			return Tcl_NewStringObj ( "#CLOSE", -1 );

		case GTK_RESPONSE_YES:
			return Tcl_NewStringObj ( "#YES", -1 );

		case GTK_RESPONSE_NO:
			return Tcl_NewStringObj ( "#NO", -1 );

		case GTK_RESPONSE_APPLY:
			return Tcl_NewStringObj ( "#APPLY", -1 );

		case GTK_RESPONSE_HELP:
			return Tcl_NewStringObj ( "#HELP", -1 );

		default: ;

	}

	assert ( ret >= 0 );

	return Tcl_NewStringObj ( g_ptr_array_index ( para->butRes, ret ), -1 );
}

/**
\brief
**/
static void onResponse ( GtkDialog *dialog, gint arg1, gpointer data )
{

	DialogParams *para = ( DialogParams * ) data;

	GnoclPercSubst ps[] =
	{
		{ 'w', GNOCL_STRING },  /* widget */
		{ 'v', GNOCL_OBJ },
		{ 'r', GNOCL_INT}, 	/* reposonse identifier */
		{ 'd', GNOCL_STRING},
		{ 0 }
	};

	if ( para->isModal )
	{
		ps[0].val.str = "";
	}

	else
	{
		ps[0].val.str = gnoclGetNameFromWidget ( GTK_WIDGET ( dialog ) );
	}

	ps[1].val.obj = getObjFromRet ( para, arg1 );
	ps[2].val.i = arg1;
	ps[3].val.str = g_object_get_data ( G_OBJECT ( dialog ), "gnocl::data" );

	/* only for tests
	Tcl_AllowExceptions( para->interp );
	para->ret = Tcl_EvalEx( para->interp, para->onResponse, -1,
	      TCL_EVAL_GLOBAL|TCL_EVAL_DIRECT );
	printf( "in onResponse: %s -> %d %s\n", para->onResponse,
	      para->ret, Tcl_GetString( Tcl_GetObjResult( para->interp ) ) );
	*/
	para->ret = gnoclPercentSubstAndEval ( para->interp, ps, para->onResponse, 0 );
	/*
	   we get TCL_ERROR if break is executed in a proc
	   we get TCL_BREAK if break is directly executed in onResponse
	*/

	if ( para->ret == TCL_ERROR )
	{
		/* FIXME: this is ugly, so leave it out?
		if( strcmp( Tcl_GetStringResult( para->interp ),
		               "invoked \"break\" outside of a loop" ) == 0 )
		{
		   Tcl_ResetResult( para->interp );
		   para->ret = TCL_BREAK;
		}
		else
		*/
		Tcl_BackgroundError ( para->interp );
	}

	/* dialog is destroyed if onResponse returns not TCL_OK
	   (e.g. TCL_ERROR or TCL_BREAK).
	   if dialog is modal we destroy the widget in gnoclMessageDialogCmd
	   since we need para for the return value */

	/* reverted to previous code from 0.9.95, since destroying a modal
	   dialog too early breaks the return value -ag */
	if ( para->isModal == 0 && para->ret != TCL_OK )
		gtk_widget_destroy ( GTK_WIDGET ( dialog ) );

}

/**
\brief
**/
static void destroyFunc ( GtkWidget *widget, gpointer data )
{
	DialogParams *para = ( DialogParams * ) data;
	g_ptr_array_free ( para->butRes, 1 );
	g_free ( para->onResponse );

	if ( para->name )
	{
		gnoclForgetWidgetFromName ( para->name );
		Tcl_DeleteCommand ( para->interp, para->name );
		g_free ( para->name );
	}

	g_free ( para );
}

static const char *cmds[] = { "delete", "configure", "class", "hide", "show", NULL };

/**
\brief
**/
int dialogFunc ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{

	enum cmdIdx { DeleteIdx, ConfigureIdx, ClassIdx, HideIdx, ShowIdx };

	DialogParams *para = ( DialogParams * ) data;
	GtkWidget *widget = GTK_WIDGET ( para->dialog );
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

		case HideIdx:
			{
				gtk_widget_hide ( widget );
			}
			break;
		case ShowIdx:
			{
				gtk_widget_show_all ( widget );
			}
			break;
		case ClassIdx:
			{
				Tcl_SetObjResult ( interp, Tcl_NewStringObj ( "messageDialog", -1 ) );
			}

			break;
		case DeleteIdx:
			{
				return gnoclDelete ( interp, widget, objc, objv );
			} break;
		case ConfigureIdx:
			{
				int ret = TCL_ERROR;

				if ( gnoclParseAndSetOptions ( interp, objc - 1, objv + 1, dialogOptions, G_OBJECT ( widget ) ) == TCL_OK )
				{
					ret = configure ( interp, widget, dialogOptions );
				}

				gnoclClearOptions ( dialogOptions );

				return ret;
			}
			break;
	}

	return TCL_OK;
}

/*
   returns true if obj is
      - a list of a widgetID and a value
      - a list of text and a stock icon
*/

/**
\brief
**/
static int addButtonList ( Tcl_Interp *interp, Tcl_Obj *obj, GtkDialog *dialog,
						   int k, GPtrArray *array )
{
	Tcl_Obj *first, *second;
	int     n;

	if ( Tcl_ListObjLength ( interp, obj, &n ) == TCL_OK
			&& n == 2
			&& Tcl_ListObjIndex ( interp, obj, 0, &first ) == TCL_OK
			&& Tcl_ListObjIndex ( interp, obj, 1, &second ) == TCL_OK )
	{
		char      *value = NULL;
		GtkWidget *widget = gnoclGetWidgetFromName ( Tcl_GetString ( first ),
							NULL );

		if ( widget != NULL )
		{
			if ( gnoclAssertNotPacked ( widget, interp, Tcl_GetString ( first ) ) )
				return -1;

			/* list of: widgetID and value */
			value = Tcl_GetString ( second );
		}

		else if ( gnoclGetStringType ( second ) & GNOCL_STR_STOCK )
		{
			/* list of: text and stock icon */
			int     ret;
			Tcl_Obj *objv[6];
			objv[0] = NULL;
			objv[1] = Tcl_NewStringObj ( "-text", -1 );
			objv[2] = first;
			objv[3] = Tcl_NewStringObj ( "-icon", -1 );
			objv[4] = second;
			objv[5] = NULL;

			ret = gnoclButtonCmd ( NULL, interp, 5, objv );
			Tcl_DecrRefCount ( objv[1] );
			Tcl_DecrRefCount ( objv[3] );

			if ( ret != TCL_OK )
				return -1;

			widget = gnoclGetWidgetFromName ( Tcl_GetStringResult ( interp ),
											  interp );

			assert ( widget );

			value = gnoclGetString ( second );

			Tcl_ResetResult ( interp );
		}

		if ( widget != NULL )
		{
			gtk_dialog_add_action_widget ( dialog, widget, k );
			g_object_set ( G_OBJECT ( widget ), "can-default", 1, NULL );
			g_ptr_array_add ( array, g_strdup ( value ) );

			return 1;
		}
	}

	return 0;
}

/**
\brief
**/
static int handleButtons ( Tcl_Interp *interp, GtkDialog *dialog,
						   DialogParams *para, Tcl_Obj *buttons )
{
	int k, no;
	/* this can't fail, since buttonsIdx is a GNOCL_LIST */
	Tcl_ListObjLength ( interp, buttons, &no );

	for ( k = 0; k < no; ++k )
	{
		Tcl_Obj *tp;
		int     isButton;

		Tcl_ListObjIndex ( interp, buttons, k, &tp );

		/*
		   tp is either
		      - a list of a widgetID and a value, or
		      - the name of a stock item, or
		      - the text of a button
		*/
		isButton = addButtonList ( interp, tp, dialog, k, para->butRes );
		/*
		if( k == no-1 )
		   g_object_set( G_OBJECT( widget ), "has-default", 1, NULL );
		*/

		if ( isButton < 0 )
			return TCL_ERROR;

		if ( isButton == 0 )
		{
			GtkStockItem    sp;
			GnoclStringType strType = gnoclGetStringType ( tp );
			char            *str = gnoclGetString ( tp );
			g_ptr_array_add ( para->butRes, g_strdup ( str ) );

			if ( strType & GNOCL_STR_STOCK
					&& gnoclGetStockItem ( tp, interp, &sp ) == TCL_OK )
			{
				gtk_dialog_add_button ( dialog, sp.stock_id, k );
			}

			else
				gtk_dialog_add_button ( dialog, str, k );
		}
	}

	return TCL_OK;
}

/**
\brief
**/
int gnoclDialogCmd ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{

	if ( gnoclGetCmdsAndOpts ( interp, cmds, dialogOptions, objv, objc ) == TCL_OK )
	{
		return TCL_OK;
	}


	int          ret;
	int          type;
	DialogParams *para;

	if ( gnoclParseOptions ( interp, objc, objv, dialogOptions ) != TCL_OK )
		goto clearError1;

	if ( getType ( interp, &dialogOptions[typeIdx], &type ) != TCL_OK )
		goto clearError1;

	if ( ! ( ( dialogOptions[textIdx].status == GNOCL_STATUS_CHANGED )
			 ^ ( dialogOptions[childIdx].status == GNOCL_STATUS_CHANGED ) ) )
	{
		Tcl_SetResult ( interp, "Either option -text or -child must be given.",
						TCL_STATIC );
		goto clearError1;
	}

	para = g_new ( DialogParams, 1 );

	if ( dialogOptions[textIdx].status == GNOCL_STATUS_CHANGED )
	{
		para->dialog = GTK_DIALOG ( gtk_message_dialog_new ( NULL,
									0, type, GTK_BUTTONS_NONE, "%s",
									dialogOptions[textIdx].val.str ) );
	}

	else
	{
		GtkWidget *child = gnoclChildNotPacked ( dialogOptions[childIdx].val.str,
						   interp );

		if ( child == NULL )
			goto clearError1;

		para->dialog = GTK_DIALOG ( gtk_dialog_new( ) );

		if ( dialogOptions[typeIdx].status == GNOCL_STATUS_CHANGED )
		{
			GtkWidget  *stock;
			const char *sId;
			GtkWidget  *box = gtk_hbox_new ( 0, 8 );

			gtk_container_set_border_width ( GTK_CONTAINER ( box ), 8 );

			switch ( type )
			{
				case GTK_MESSAGE_INFO:     sId = GTK_STOCK_DIALOG_INFO; break;
				case GTK_MESSAGE_WARNING:  sId = GTK_STOCK_DIALOG_WARNING; break;
				case GTK_MESSAGE_QUESTION: sId = GTK_STOCK_DIALOG_QUESTION; break;
				case GTK_MESSAGE_ERROR:    sId = GTK_STOCK_DIALOG_ERROR; break;
			}

			stock = gtk_image_new_from_stock ( sId, GTK_ICON_SIZE_DIALOG );

			gtk_misc_set_alignment ( GTK_MISC ( stock ), 0.5, 0.0 );
			gtk_box_pack_start ( GTK_BOX ( box ), stock, 0, 0, 0 );
			gtk_box_pack_start ( GTK_BOX ( box ), child, 1, 1, 0 );
			gtk_widget_show_all ( box );
			gtk_box_pack_start (  gtk_dialog_get_content_area ( para->dialog ), box, 1, 1, 0 );
		}

		else
			gtk_container_add ( gtk_dialog_get_content_area ( para->dialog ), child );
	}


	/* resize window in proportion to the screen */
	if ( dialogOptions[setSizeIdx].status == GNOCL_STATUS_CHANGED )
	{

		gint width = 0;
		gint height = 0;
		GdkScreen *screen;

		screen = gdk_screen_get_default();

		width = gdk_screen_get_width ( screen );
		height = gdk_screen_get_height ( screen );

		float w, h;

		w = dialogOptions[setSizeIdx].val.d * ( float ) width;
		h =  dialogOptions[setSizeIdx].val.d * ( float ) height;

		width = ( gint ) w;
		height = ( gint ) h;

		gtk_window_set_default_size ( GTK_WINDOW ( para->dialog ), width, height );

	}



	// set width and height
	if ( dialogOptions[widthIdx].status == GNOCL_STATUS_CHANGED
			&& dialogOptions[heightIdx].status == GNOCL_STATUS_CHANGED )
	{
		gtk_window_resize ( GTK_WINDOW ( para->dialog ), dialogOptions[widthIdx].val.i,
							dialogOptions[heightIdx].val.i );
	}

	else if ( dialogOptions[widthIdx].status == GNOCL_STATUS_CHANGED
			  || dialogOptions[heightIdx].status == GNOCL_STATUS_CHANGED )
	{
		int width, height;
		gtk_window_get_size ( GTK_WINDOW ( para->dialog ), &width, &height );

		if ( dialogOptions[widthIdx].status == GNOCL_STATUS_CHANGED )
			width = dialogOptions[widthIdx].val.i;
		else
			height = dialogOptions[heightIdx].val.i;

		gtk_window_resize ( GTK_WINDOW ( para->dialog ), width, height );
	}



	// make only one move if x and y are set
	if ( dialogOptions[xIdx].status == GNOCL_STATUS_CHANGED
			&& dialogOptions[xIdx].status == GNOCL_STATUS_CHANGED )
	{
		gtk_window_move ( GTK_WINDOW ( para->dialog ), dialogOptions[xIdx].val.i, dialogOptions[yIdx].val.i );
	}

	else if ( dialogOptions[xIdx].status == GNOCL_STATUS_CHANGED
			  || dialogOptions[yIdx].status == GNOCL_STATUS_CHANGED )
	{
		int x, y;
		gtk_window_get_position ( GTK_WINDOW ( para->dialog ), &x, &y );

		if ( dialogOptions[xIdx].status == GNOCL_STATUS_CHANGED )
			x = dialogOptions[xIdx].val.i;
		else
			y = dialogOptions[yIdx].val.i;

		gtk_window_move ( GTK_WINDOW ( para->dialog ), x, y );
	}



	para->interp = interp;

	para->name = NULL;
	para->butRes = g_ptr_array_new();
	para->isModal = 1;

	if ( dialogOptions[modalIdx].status == GNOCL_STATUS_CHANGED )
		para->isModal = dialogOptions[modalIdx].val.b;

	if ( dialogOptions[onResponseIdx].status == GNOCL_STATUS_CHANGED )
		para->onResponse = g_strdup ( dialogOptions[onResponseIdx].val.str );
	else
		para->onResponse = g_strdup ( "break" );

	g_signal_connect ( G_OBJECT ( para->dialog ), "response", G_CALLBACK ( onResponse ), para );

	if ( dialogOptions[buttonsIdx].status == GNOCL_STATUS_CHANGED )
	{
		if ( handleButtons ( interp, para->dialog, para, dialogOptions[buttonsIdx].val.obj ) != TCL_OK )
		{
			goto clearError3;
		}
	}

	else
	{
		g_ptr_array_add ( para->butRes, g_strdup ( "Ok" ) );
		gtk_dialog_add_button ( para->dialog, GTK_STOCK_OK, 0 );
	}

	/* hmm, this works only if gtk_widget_show is before
	   gtk_dialog_add_button.  It can't be reassigned and gives errors
	   on buttons added via gtk_dialog_add_button */

	if ( dialogOptions[defaultButtonIdx].status == GNOCL_STATUS_CHANGED )
	{
		gtk_dialog_set_default_response ( para->dialog, dialogOptions[defaultButtonIdx].val.i );
	}

	else
	{
		/* default should be always the last one */
		gtk_dialog_set_default_response ( para->dialog, para->butRes->len - 1 );
	}

	ret = gnoclSetOptions ( interp, dialogOptions, G_OBJECT ( para->dialog ), -1 );


	/* no icon if -child option is set */
	/* GtkDialog has not property nameed 'message-type'.
	/*

		if ( dialogOptions[childIdx].status == GNOCL_STATUS_CHANGED )
		{
			g_object_set ( G_OBJECT ( para->dialog ), "message-type", GTK_MESSAGE_OTHER, NULL );
		}
	*/

	gtk_widget_show ( GTK_WIDGET ( para->dialog ) );
	/* gnoclClearOptions must be called before we enter the
	   gtk event loop: we can't be called recursively! */
	gnoclClearOptions ( dialogOptions );

	if ( ret != TCL_OK )
		goto clearError2;

	g_signal_connect ( G_OBJECT ( para->dialog ), "destroy", G_CALLBACK ( destroyFunc ), para );

	if ( para->isModal )
	{
		gtk_window_set_modal ( GTK_WINDOW ( para->dialog ), 1 );

		do
		{
			ret = gtk_dialog_run ( GTK_WINDOW ( para->dialog ) );
			/* exit if no button exit, or onResponse returned break */
		}

		while ( ret >= 0 && para->ret == TCL_OK );

		Tcl_SetObjResult ( interp, getObjFromRet ( para, ret ) );

		/* removed, because it causes warnings when the dialog is closed */
		/* reenabled to make return value work -ag */
		gtk_widget_destroy ( GTK_WIDGET ( GTK_WINDOW ( para->dialog ) ) );
	}

	else
	{
		para->name = gnoclGetAutoWidgetId();
		gnoclMemNameAndWidget ( para->name, GTK_WIDGET ( para->dialog ) );
		Tcl_CreateObjCommand ( interp, para->name, dialogFunc, para, NULL );
		Tcl_SetObjResult ( interp, Tcl_NewStringObj ( para->name, -1 ) );
	}

	return TCL_OK;

clearError3:
	g_ptr_array_free ( para->butRes, 1 );
	g_free ( para->onResponse );
	g_free ( para );

clearError2:
	gtk_widget_destroy ( GTK_WIDGET ( para->dialog ) );
	return ret;

clearError1:
	gnoclClearOptions ( dialogOptions );
	return TCL_ERROR;
}

