/**
\brief      This module implements the gnocl::printerDialog widget.
\author     Peter G. Baum, William J Giddings
\date       2010-05:
\todo       The dialog is displayed using gtk_dialog_run function rather than gtk_widget_show_all.
\see        http://library.gnome.org/devel/gtk/2.16/GtkDialog.html#gtk-dialog-run
\see        Krause, (Ch.5),


	The dialog returns the name of a custom setup inless -pageSetup option given

**/

/*
 * $Id: aboutDialog.c,v 1.1 2005/08/16 20:57:45 baum Exp $
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
	2013-07: added commands, options, commands
	2010-01: Began Development
 */

#include "gnocl.h"

/* declarations */
static int optStrv ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret );

/**
\brief
\author     William J Giddings
\date       01/05/2010
\since      0.9.95
**/
static int gnoclOptPrinterPageSetup ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	printf ( "gnoclOptPrinterPageSetup\n" );
	return TCL_OK;
}


/**
\brief
\author     William J Giddings
\date       01/05/2010
\since      0.9.95
**/
int gnoclOptPrinterSettings ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	printf ( "gnoclOptPrinterSettings\n" );
	return TCL_OK;
}

/**
\brief
\author     William J Giddings
\date       01/05/2010
\since      0.9.95
**/
static int gnoclOptPrinter ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	printf ( "gnoclOptPrinter\n" );
	return TCL_OK;
}

/**
\brief
\author     William J Giddings
\date       01/05/2010
\since      0.9.95
**/
int gnoclOptTitle ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	printf ( "gnoclOptTitle\n" );
	//gtk_window_set_title (GtkWindow *window, const gchar *title);

	return TCL_OK;
}



static const int titleIdx = 0;
static const int parentIdx = 1;

/**
\brief
\author		William J Giddings
\date       01/05/2010
\since      0.9.95
**/
static GnoclOption printerDialogOptions[] =
{

	/* gnocl options */
	{ "-title", GNOCL_OBJ, NULL },
	{ "-parent", GNOCL_OBJ, NULL },


	/* properties */
	{ "-currentPage", GNOCL_STRING, "current-page" },
	{ "-pageSetup", GNOCL_OBJ, "", gnoclOptPrinterPageSetup },
	{ "-settings", GNOCL_OBJ, "", gnoclOptPrinterSettings },
	{ "-printer", GNOCL_OBJ, "", gnoclOptPrinter },

	{ NULL },
};


/**
\brief
\author		William J Giddings
\date       01/05/2010
\since      0.9.95
**/
static int optStrv ( Tcl_Interp *interp, GnoclOption *opt,
					 GObject *obj, Tcl_Obj **ret )
{
	if ( ret == NULL ) /* set value */
	{
		typedef char *charp;
		int  no;
		int  k;
		char **strv;

		Tcl_ListObjLength ( interp, opt->val.obj, &no );
		strv = g_new ( charp, no + 1 );

		for ( k = 0; k < no; ++k )
		{
			Tcl_Obj *pobj;

			if ( Tcl_ListObjIndex ( interp, opt->val.obj, k, &pobj ) != TCL_OK )
			{
				g_free ( strv );
				return TCL_ERROR;
			}

			strv[k] = Tcl_GetString ( pobj );
		}

		strv[no] = NULL;

		g_object_set ( obj, opt->propName, strv, NULL );
		g_free ( strv );
	}

	else /* get value */
	{
		gchar **strv;
		int   no;
		int   k;

		g_object_get ( obj, opt->propName, &strv, NULL );

		for ( no = 0; strv[no] != NULL; ++no )
			;

		*ret = Tcl_NewListObj ( 0, NULL );

		for ( k = 0; k < no; ++k )
			Tcl_ListObjAppendElement ( NULL, *ret,
									   Tcl_NewStringObj ( strv[k], -1 ) );

		g_strfreev ( strv );
	}

	return TCL_OK;
}

/**
\brief
\author    William J Giddings
\date     01/05/10
\note
**/
static int configure ( Tcl_Interp *interp, GtkAboutDialog *dialog,
					   GnoclOption options[] )
{

	if ( options[titleIdx].status == GNOCL_STATUS_CHANGED )
	{
		GdkPixbuf *pix;
		/*
				if ( gnoclGetStringType ( options[titleIdx].val.obj ) != GNOCL_STR_FILE )
				{
					Tcl_SetResult ( interp, "Logo must be of file type", TCL_STATIC );
					return TCL_ERROR;
				}

				pix = gnoclPixbufFromObj ( interp, options + titleIdx );

				if ( pix == NULL )
					return TCL_ERROR;

				gtk_about_dialog_set_logo ( dialog, pix );
		*/
	}

	return TCL_OK;
}

/**
\brief
\author		William J Giddings
\date		01/05/10
\note
**/
static int cget ( Tcl_Interp *interp, GtkLabel *label, GnoclOption options[], int idx )
{
	Tcl_Obj *obj = NULL;

	return gnoclCgetNotImplemented ( interp, options + idx );
}

/**
\brief
\author		William J Giddings
\date		01/05/10
\note
**/
int printerDialogFunc ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{

	static const char *cmds[] = { "class", "delete", "configure", "cget", "show", "hide", "options", "commands", NULL };
	enum cmdIdx { ClassIdx, DeleteIdx, ConfigureIdx, CgetIdx, ShowIdx, HideIdx, OptionsIdx, CommandsIdx };
	int idx;
	GtkLabel *dialog = ( GtkLabel * ) data;

	if ( objc < 2 )
	{
		Tcl_WrongNumArgs ( interp, 1, objv, "command" );
		return TCL_ERROR;
	}

	if ( Tcl_GetIndexFromObj ( interp, objv[1], cmds, "command",
							   TCL_EXACT, &idx ) != TCL_OK )
	{
		return TCL_ERROR;
	}

	switch ( idx )
	{
		case CommandsIdx:
			{
				gnoclGetOptions ( interp, cmds );
			}
			break;
		case OptionsIdx:
			{
				gnoclGetOptions ( interp, printerDialogOptions );
			}
			break;
		case ClassIdx:
			{
				Tcl_SetObjResult ( interp, Tcl_NewStringObj ( "printerDialog", -1 ) );
			}
			break;
		case HideIdx:
			{
				gtk_widget_hide ( GTK_WIDGET ( dialog ) );
			}
			break;
		case ShowIdx:
			{
				gtk_dialog_run ( GTK_WIDGET ( dialog ) );
				gtk_widget_destroy ( dialog );
				//gtk_widget_show_all ( GTK_WIDGET ( dialog ) );
			}
			break;
		case DeleteIdx:
			return gnoclDelete ( interp, GTK_WIDGET ( dialog ), objc, objv );

		case ConfigureIdx:
			{
				int ret = TCL_ERROR;

				if ( gnoclParseAndSetOptions ( interp, objc - 1, objv + 1,
											   printerDialogOptions, G_OBJECT ( dialog ) ) == TCL_OK )
				{
					ret = configure ( interp, dialog, printerDialogOptions );
				}

				gnoclClearOptions ( printerDialogOptions );

				return ret;
			}

			break;
		case CgetIdx:
			{
				int     idx;

				/* kill( 0, SIGINT ); */

				switch ( gnoclCget ( interp, objc, objv, G_OBJECT ( dialog ),
									 printerDialogOptions, &idx ) )
				{
					case GNOCL_CGET_ERROR:
						return TCL_ERROR;
					case GNOCL_CGET_HANDLED:
						return TCL_OK;
					case GNOCL_CGET_NOTHANDLED:
						return cget ( interp, dialog, printerDialogOptions, idx );
				}
			}
	}

	return TCL_OK;
}

/**
\brief
\author		William J Giddings
\date		01/05/10
\note
**/
int gnoclPrinterDialogCmd ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{

#ifdef DEBUG_PRINTER_DIALOG
	g_printf ( "gnoclPrinterDialogCmd\n" );
	gint _i;


	for ( _i = 0; _i < objc; _i++ )
	{
		g_printf ( "\targ %d = %s\n", _i,  Tcl_GetString ( objv[_i] ) );
	}

#endif


	int ret;
	GtkWidget *dialog;
	GtkWindow *toplevel;
	gchar *title = NULL;
	gchar *parent = NULL;
	gint i;

	if ( gnoclParseOptions ( interp, objc, objv, printerDialogOptions ) != TCL_OK )
	{
		gnoclClearOptions ( printerDialogOptions );
		return TCL_ERROR;
	}

	/* obtain values for title and parent */
	for ( i = 0; i < objc; i++ )
	{

		if ( !strcmp ( Tcl_GetString ( objv[i] ), "-title" )  )
		{
			title = Tcl_GetString ( objv[i+1] );
		}

		if ( !strcmp ( Tcl_GetString ( objv[i] ), "-parent" )  )
		{
			parent = Tcl_GetString ( objv[i+1] );
			toplevel = gnoclGetWidgetFromName ( parent, interp );
		}
	}

	dialog = gtk_print_unix_dialog_new ( title, toplevel );


	ret = gnoclSetOptions ( interp, printerDialogOptions, G_OBJECT ( dialog ), -1 );

	if ( ret == TCL_OK )
	{
		ret = configure ( interp, dialog, printerDialogOptions );
	}

	gnoclClearOptions ( printerDialogOptions );

	if ( ret != TCL_OK )
	{
		gtk_widget_destroy ( GTK_WIDGET ( dialog ) );
		return TCL_ERROR;
	}

	gtk_dialog_run ( GTK_DIALOG ( dialog ) );
	gtk_widget_destroy ( dialog );

	return TCL_OK;
	//return gnoclRegisterWidget ( interp, GTK_WIDGET ( dialog ), aboutDialogFunc );
}

