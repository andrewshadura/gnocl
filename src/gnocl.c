/**
\brief
\include ../doc_includes/gnocl.dox
**/

/*
 * $Id: gnocl.c,v 1.45 2006-02-27 20:56:39 baum Exp $
 *
 * This file implements a Tcl interface to GTK+ and Gnome
 *
 * Copyright (c) 2001 - 2005 Peter G. Baum  http://www.dr-baum.net
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */


/* doc pages commands created within this module */

/**
\page page_inventory gnocl::inventory
\htmlinclude inventory.html
**/

/**
\page page_mainLoop gnocl::mainLoop
\htmlinclude mainLoop.html
**/

#include "gnocl.h"
#include <ctype.h>

static GHashTable *name2widgetList;
static const char idPrefix[] = "::gnocl::_WID";


/**
\brief      Convert contents of hash list to a glist
**/
void hash_to_list ( gpointer key, gpointer value, gpointer user_data )
{
	GList **list = user_data;

	*list = g_list_prepend ( *list, value );
}

/**
\brief
**/
gint sorter ( gconstpointer a, gconstpointer b )
{
	return ( strcmp ( ( const gchar * ) a, ( const gchar * ) b ) );
}

/**
\brief
*/
static void simpleDestroyFunc (
	GtkWidget *widget,
	gpointer data )
{
	const char *name = gnoclGetNameFromWidget ( widget );
	gnoclForgetWidgetFromName ( name );
	Tcl_DeleteCommand ( ( Tcl_Interp * ) data, ( char * ) name );
	g_free ( ( char * ) name );
}


/**
\brief
*/
int gnoclRegisterWidget ( Tcl_Interp *interp, GtkWidget *widget, Tcl_ObjCmdProc *proc )
{
	const char *name = gnoclGetAutoWidgetId();
	gnoclMemNameAndWidget ( name, widget );

	/* remove object from hash table and clear the associated function */
	g_signal_connect_after ( G_OBJECT ( widget ), "destroy", G_CALLBACK ( simpleDestroyFunc ), interp );

	if ( proc != NULL )
	{
		Tcl_CreateObjCommand ( interp, ( char * ) name, proc, widget, NULL );
	}

	Tcl_SetObjResult ( interp, Tcl_NewStringObj ( name, -1 ) );

	return TCL_OK;
}

/**
\brief
*/
GtkWidget *gnoclChildNotPacked ( const char *name, Tcl_Interp *interp )
{
	GtkWidget *child = gnoclGetWidgetFromName ( name, interp );

	if ( child == NULL )
		return NULL;

	if ( gnoclAssertNotPacked ( child, interp, name ) )
		return NULL;

	return child;
}

/**
\brief
*/
int gnoclAssertNotPacked (	GtkWidget *child,	Tcl_Interp *interp,	const char *name )
{
	if ( gtk_widget_get_parent ( child ) != NULL )
	{
		if ( name && interp )
			Tcl_AppendResult ( interp, "window \"", name, "\" has already a parent.", ( char * ) NULL );

		return 1;
	}

	return 0;
}


/**
\brief
\notes      "char *" and not "const char *" because of a not very strict
            handling of "const char *" in Tcl e.g. Tcl_CreateObjCommand
*/
char *gnoclGetAutoWidgetId ( void )
{
	static int no = 0;
	/*
	static char buffer[30];
	*/

	char *buffer = g_new ( char, sizeof ( idPrefix ) + 15 );
	strcpy ( buffer, idPrefix );

	/* with namespace, since the Id is also the widget command */
	sprintf ( buffer + sizeof ( idPrefix ) - 1, "%d", ++no );

	return buffer;
}

/**
\brief
*/
/* -----------------
   handle widget <-> name mapping
-------------------- */
GtkWidget *gnoclGetWidgetFromName ( const char *id, Tcl_Interp *interp )
{
	GtkWidget *widget = NULL;
	int       n;

	if ( strncmp ( id, idPrefix, sizeof ( idPrefix ) - 1 ) == 0
			&& ( n = atoi ( id + sizeof ( idPrefix ) - 1 ) ) > 0 )
	{
		widget = g_hash_table_lookup ( name2widgetList, GINT_TO_POINTER ( n ) );
	}

	if ( widget == NULL && interp != NULL )
	{
		Tcl_AppendResult ( interp, "Unknown widget \"", id, "\".", ( char * ) NULL );
	}

	return widget;
}

/**
\brief      Returns the widget name associated with pointer
\note       This will not apply to objects created from xml UI decscriptions.
*/
const char *gnoclGetNameFromWidget ( GtkWidget *widget )
{
	const char *name = g_object_get_data ( G_OBJECT ( widget ), "gnocl::name" );

	if ( name == NULL && ( GTK_IS_TREE_VIEW ( widget ) || GTK_IS_TEXT_VIEW ( widget ) ) )
	{
		name = gnoclGetNameFromWidget ( gtk_widget_get_parent ( widget ) );
	}

	return name;
}

/**
\brief
*/
int gnoclMemNameAndWidget ( const char *path,  GtkWidget *widget )
{
	int n;

	n = atoi ( path + sizeof ( idPrefix ) - 1 );

	assert ( n > 0 );
	assert ( g_hash_table_lookup ( name2widgetList, GINT_TO_POINTER ( n ) ) == NULL );
	assert ( strncmp ( path, idPrefix, sizeof ( idPrefix ) - 1 ) == 0 );

	/* memorize the name of the widget in the widget */
	g_object_set_data ( G_OBJECT ( widget ), "gnocl::name", ( char * ) path );
	g_hash_table_insert ( name2widgetList, GINT_TO_POINTER ( n ), widget );

	return 0;
}

/**
\brief
*/
int gnoclForgetWidgetFromName ( const char *path )
{
	int n = atoi ( path + sizeof ( idPrefix ) - 1 );
	assert ( gnoclGetWidgetFromName ( path, NULL ) );
	assert ( strncmp ( path, idPrefix, sizeof ( idPrefix ) - 1 ) == 0 );
	assert ( n > 0 );

	g_hash_table_remove ( name2widgetList, GINT_TO_POINTER ( n ) );

	return 0;
}

/**
\brief      Return a list of all names in the specific hash lists.
\since      0.9.94g
**/
const char *gnoclGetInventory ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{

	/*--------------------------------------*/
	static GnoclOption options[] =
	{
		{ NULL, GNOCL_STRING, NULL },
		{ NULL },
	};

	static const char *cmds[] = { "widget", "pixBuf", NULL };

	if ( gnoclGetCmdsAndOpts ( interp, cmds, options, objv, objc ) == TCL_OK )
	{
		return TCL_OK;
	}

	/*--------------------------------------*/


	gint _i;


	enum optIdx { WidgetIdx, PixBufIdx };
	int idx;


	if ( Tcl_GetIndexFromObj ( interp, objv[1], cmds, "command", TCL_EXACT, &idx ) != TCL_OK )
	{
		Tcl_WrongNumArgs ( interp, 1, objv, "[widget | pixBuf]\n" );
		return TCL_ERROR;
	}

	GList *list;
	GList *l;
	GString *str;


	str = g_string_new ( "" );

	list = NULL;


	switch ( idx )
	{
		case WidgetIdx:
			{
				g_hash_table_foreach ( name2widgetList, hash_to_list, &list );
				list = g_list_sort ( list, sorter );

				for ( l = list; l != NULL; l = l->next )
				{
					str = g_string_append ( str, gnoclGetNameFromWidget ( l->data ) );
					str = g_string_append ( str, " " );
				}

			} break;
		case PixBufIdx:
			{
				gnoclGetPixBufList ( &list );
				list = g_list_sort ( list, sorter );

				for ( l = list; l != NULL; l = l->next )
				{
					str = g_string_append ( str, gnoclGetNameFromPixBuf ( l->data ) );
					str = g_string_append ( str, " " );
				}
			} break;
		default: {} break;
	}

	Tcl_SetResult ( interp, str->str, TCL_STATIC );
	return TCL_OK;
}

/**
\brief
*/
int gnoclDelete ( Tcl_Interp *interp, GtkWidget *widget, int objc, Tcl_Obj * const objv[] )
{
	if ( objc != 2 )
	{
		/* widget delete */
		Tcl_WrongNumArgs ( interp, 1, objv, "" );
		return TCL_ERROR;
	}

	/* TODO: all children*/
	gtk_widget_destroy ( widget );

	return TCL_OK;
}

/**
\brief
\note
   Prefix Explanation
   %%     % plus rest of string as is
   %!     rest of string as is
   %#     stock text/icon/accelerator/... first '_' is underline character
   %_     text with underline

   else:  use defaultStringPrefix

   FIXME: what about translation?
*/

GnoclStringType gnoclGetStringType ( Tcl_Obj *obj )
{

	char *name = Tcl_GetString ( obj );

	if ( *name == 0 )
	{
		return GNOCL_STR_EMPTY;
	}

	assert ( GNOCL_STOCK_PREFIX[0] == '%' && GNOCL_STOCK_PREFIX[1] == '#' );

	if ( *name == '%' )
	{
		/* if something is changed here, also change gnoclGetStringFromObj! */
		switch ( name[1] )
		{
			case '!':
			case '%': return GNOCL_STR_STR;
			case '#': return GNOCL_STR_STOCK | GNOCL_STR_UNDERLINE;
			case '/': return GNOCL_STR_FILE;
			case '_': return GNOCL_STR_UNDERLINE;
			case '<': return GNOCL_STR_MARKUP | GNOCL_STR_UNDERLINE;
			case '?': return GNOCL_STR_BUFFER;
		}
	}

	return GNOCL_STR_STR;
}


/**
\brief
*/
static char *getEscStringFromObj ( Tcl_Obj *op, int *len, int useEscape )
{

	char *ret;

	if ( op == NULL )
	{
		return NULL;
	}


	ret = Tcl_GetStringFromObj ( op, len );


	if ( useEscape && *ret == '%' )
	{

		/* if something is changed here, also change gnoclGetStringType! */
		switch ( ret[1] )
		{
			case '/':	/* GNOCL_STR_FILE */
			case '#':	/* GNOCL_STR_STOCK */
			case '?':	/* GNOCL_STR_BUFFER */
			case '_':	/* GNOCL_STR_UNDERLINE */
			case '<':	/* GNOCL_STR_MARKUP */
			case '!':	/* GNOCL_STR_STR */
				{
					ret += 2;

					if ( len )
					{
						*len -= 2;
					}
				}
				break;

			case '%': /* escaped % */
				{
					ret += 1;

					if ( len )
					{
						*len -= 1;
					}
				}
				break;

				/* translate
				case ???:   ret = _( ret + 2 );
				            if( len )   *len = strlen( ret );
				            break;
				*/
		}

		// printf ( " -> %s\n", ret );
	}

	return ret;
}

/**
\brief Strip away percentage markup from string names.
*/
char *gnoclGetStringFromObj ( Tcl_Obj *op,	int *len )
{
	/* TODO: global flag "gnoclNoStringEscape"? */
	return getEscStringFromObj ( op, len, 1 );
}

/**
\brief
*/
char *gnoclGetString ( Tcl_Obj *op )
{
	return gnoclGetStringFromObj ( op, NULL );
}

/**
\brief
*/
char *gnoclStringDup ( Tcl_Obj *op )
{
	int txtLen;
	const char *txt;

	if ( op == NULL )
		return NULL;

	txt = gnoclGetStringFromObj ( op, &txtLen );

	return g_memdup ( txt, txtLen + 1 );
}

/**
\brief
*/
Tcl_Obj *gnoclGtkToStockName ( const char *gtk )
{
	Tcl_Obj *ret;
	GString *name;

	assert ( strncmp ( gtk, "gtk-", 4 ) == 0 );

	name = g_string_new ( NULL );

	for ( gtk += 3; *gtk; ++gtk )
	{
		if ( *gtk == '-' )
		{
			++gtk;
			g_string_append_c ( name, toupper ( *gtk ) );
		}

		else
			g_string_append_c ( name, *gtk );
	}

	ret = Tcl_NewStringObj ( name->str, -1 );

	g_string_free ( name, 1 );
	return ret;
}

/**
\brief	Parse the markup string and create a notional stockitem name.
*/
GString *createStockName ( const char *init, Tcl_Obj *obj )
{
	int len;

	const char *name = getEscStringFromObj ( obj, &len, 1 );

	GString *gtkName = g_string_new ( init );
	int   isFirst = 1;

	/* we can use isupper and tolower since the IDs use only ASCII */
	/* "AbcDef" is changed to "init-abc-def" */

	for ( ; *name; ++name )
	{
		if ( isupper ( *name ) 	|| ( isdigit ( *name ) && ( isFirst || !isdigit ( name[-1] ) ) ) )
		{
			g_string_append_c ( gtkName, '-' );
			g_string_append_c ( gtkName, tolower ( *name ) );
		}

		else
			g_string_append_c ( gtkName, *name );

		isFirst = 0;
	}

	g_string_append_c ( gtkName, 0 );

	return gtkName;
}

/**
\brief
*/
int gnoclGetStockItem ( Tcl_Obj *obj, Tcl_Interp *interp, GtkStockItem *sp )
{

	GString *gtkName;

	gtkName = createStockName ( "gtk", obj );


	if ( gtk_stock_lookup ( gtkName->str, sp ) == 0 )
	{

		g_string_free ( gtkName, 1 );
		gtkName = createStockName ( "gnome-stock", obj );

		if ( gtk_stock_lookup ( gtkName->str, sp ) == 0 )
		{

			/* as last chance use the original string */
			int len;
			const char *name;
			g_string_free ( gtkName, 1 );

			name = getEscStringFromObj ( obj, &len, 1 );

			if ( gtk_stock_lookup ( name, sp ) == 0 )
			{

				/* TODO? test the original string? */
				if ( interp != NULL )
				{
					Tcl_AppendResult ( interp, "unknown stock item \"", Tcl_GetString ( obj ), "\"", ( char * ) NULL );
				}

				return TCL_ERROR;
			}
		}
	}

	g_string_free ( gtkName, 1 );

	return TCL_OK;
}

/**
\brief
*/
int gnoclGetImage ( Tcl_Interp *interp, Tcl_Obj *obj, GtkIconSize size, GtkWidget **widget )
{
	GnoclStringType type = gnoclGetStringType ( obj );

	if ( type == GNOCL_STR_EMPTY )
	{
		*widget = NULL;
	}

	else if ( type & GNOCL_STR_STOCK )
	{
#if 0
		int     len;
		char    *txt = gnoclGetStringFromObj ( obj, &len );
		Tcl_Obj *obj2 = Tcl_NewStringObj ( txt, len );
		int     idx;
		int     ret = Tcl_GetIndexFromObj ( interp, obj2, opts, "option", TCL_EXACT, &idx );
		Tcl_DecrRefCount ( obj2 );

		if ( ret != TCL_OK )
		{
			return ret;
		}

		*widget = gnome_stock_pixmap_widget ( window, opts[idx] );

#endif
		GtkStockItem sp;

		if ( gnoclGetStockItem ( obj, interp, &sp ) != TCL_OK )
		{
			return TCL_ERROR;
		}

		*widget = gtk_image_new_from_stock ( sp.stock_id, size );

		if ( *widget == NULL )
		{
			Tcl_AppendResult ( interp, "Unknown stock pixmap \"", sp.stock_id, "\".", ( char * ) NULL );
			return TCL_ERROR;
		}
	}

	else if ( type == GNOCL_STR_FILE )
	{
		char   *txt = gnoclGetStringFromObj ( obj, NULL );
#ifdef GNOCL_USE_GNOME
		fprintf ( stderr,
				  "gtk_image_new_from_file seems to hang with gnome support.\n"
				  "Please don't use menu icons if you need gnome support or\n"
				  "comment the line USE_GNOME in the makefile to \n"
				  "disable gnome support.\n"
				  "If this works for you *with* USE_GNOME please mail your\n"
				  "GTK+ and Gnome versions to peter@dr-baum.net.\n"
				  "Thank you!\n" );
#endif
		*widget = gtk_image_new_from_file ( txt );
	}

	else if ( type == GNOCL_STR_BUFFER )
	{
		GdkPixbuf *pixbuf = NULL;

		char   *txt = gnoclGetStringFromObj ( obj, NULL );
#ifdef GNOCL_USE_GNOME
		g_print ( "got a buffer = %s\n", txt );
#endif
		pixbuf = gnoclGetPixBufFromName ( txt, interp );
		*widget = gtk_image_new_from_file ( pixbuf );
	}

	else
	{
		Tcl_AppendResult ( interp, "invalid pixmap type for \"", Tcl_GetString ( obj ), "\"", ( char * ) NULL );
		return TCL_ERROR;
	}

	return TCL_OK;
}

/**
\note   This block needs to be placed into a speparate include file so
        that it can appended to by a module creation wizard.
**/

typedef struct
{
	char *name;
	Tcl_ObjCmdProc *proc;
} GnoclCmd;


static GnoclCmd commands[] =
{

	{ "commands", gnoclCommandsCmd },

	/* non-gtk widgets */
	{ "spinner", gnoclSpinnerCmd },
	{ "dial", gnoclDialCmd },
	{ "level", gnoclLevelCmd },
	{ "tickerTape", gnoclTickerTapeCmd },
	{ "richTextToolBar", gnoclRichTextToolBarCmd },

	/* recent manager */
	{ "recentManager", gnoclRecentManagerCmd},

	/* gnocl only widget */
	{ "splashScreen", gnoclSplashScreenCmd },
	{ "debug", gnoclDebugCmd },
	{ "callback", gnoclCallbackCmd },
	{ "clipboard", gnoclClipboardCmd },
	{ "configure", gnoclConfigureCmd },
	{ "info", gnoclInfoCmd },
	{ "mainLoop", gnoclMainLoop },
	{ "update", gnoclUpdateCmd },
	{ "resourceFile", gnoclResourceFileCmd },
	{ "getStyle", gnoclGetStyleCmd },
	{ "setStyle", gnoclSetStyleCmd },
	{ "winfo", gnoclWinfoCmd },
	{ "print", gnoclPrintCmd },
	{ "pixBuf", gnoclPixBufCmd },
	{ "pixMap", gnoclPixMapCmd },

	//{ "cairo", gnoclCairoCmd }, /* moved into own package */

	/* colour convertion commands */
	{ "clr2rgb", gnoclClr2RGBCmd },
	{ "rgb2hex", gnoclRGB2HexCmd },
	{ "parseColor", gnoclParseColorCmd },
	{ "screen", gnoclScreenCmd},

	/* Gtk Widgets */
	{ "button", gnoclButtonCmd },
	{ "box", gnoclBoxCmd },
	{ "hBox", gnoclHBoxCmd },
	{ "vBox", gnoclVBoxCmd },
	{ "fixed", gnoclFixedCmd },
	{ "checkButton", gnoclCheckButtonCmd },
//	{ "clock", gnoclClockCmd },
	{ "colorSelection", gnoclColorSelectionCmd },
	{ "colorSelectionDialog", gnoclColorSelectionDialogCmd },
	{ "colorWheel", gnoclColorWheelCmd },
	{ "combo", gnoclComboCmd },
	{ "dialog", gnoclDialogCmd },
	{ "entry", gnoclEntryCmd },
	{ "eventBox", gnoclEventBoxCmd },
	{ "fileSelection", gnoclFileSelectionCmd },
	{ "fontSelection", gnoclFontSelectionCmd },
	{ "image", gnoclImageCmd },
	{ "label", gnoclLabelCmd },
	{ "list", gnoclListCmd },
	{ "listStore", gnoclListCmd },
	{ "menu", gnoclMenuCmd },
	{ "menuBar", gnoclMenuBarCmd },
	{ "menuItem", gnoclMenuItemCmd },
	{ "menuCheckItem", gnoclMenuCheckItemCmd },
	{ "menuRadioItem", gnoclMenuRadioItemCmd },
	{ "menuRecentChooser", gnoclMenuRecentChooserCmd },
	{ "menuSeparator", gnoclMenuSeparatorCmd },
	{ "notebook", gnoclNotebookCmd },
	{ "optionMenu", gnoclOptionMenuCmd },
	{ "pageSetupDialog", gnoclPageSetupDialogCmd },
	{ "paned", gnoclPanedCmd },
	{ "plug", gnoclPlugCmd },
	{ "progressBar", gnoclProgressBarCmd },

	/* move megawidgets into seprate package? */
	{ "labelEntry", gnoclLabelEntryCmd },


	{ "pBar2", gnoclPBarCmd },

	{ "radioButton", gnoclRadioButtonCmd },
	{ "scale", gnoclScaleCmd },
	{ "scrolledWindow", gnoclScrolledWindowCmd },
	{ "separator", gnoclSeparatorCmd },
	{ "socket", gnoclSocketCmd },
	{ "spinButton", gnoclSpinButtonCmd },
	{ "statusBar", gnoclStatusBarCmd },
	{ "table", gnoclTableCmd },
	{ "text", gnoclTextCmd },

	{ "textView", gnoclTextViewCmd },

	{ "toolBar", gnoclToolBarCmd },
	{ "tree", gnoclTreeCmd },
	{ "treeStore", gnoclTreeCmd },
	{ "window", gnoclWindowCmd },
	{ "calendar", gnoclCalendarCmd },
	{ "curve", gnoclCurveCmd },
	{ "gammaCurve", gnoclGammaCurveCmd },
	{ "ruler", gnoclRulerCmd },
	{ "layout", gnoclLayoutCmd },
	{ "aspectFrame", gnoclAspectFrameCmd },
	{ "iconView", gnoclIconViewCmd },
	{ "infoBar", gnoclInfoBarCmd },
	{ "accelarator", gnoclAcceleratorCmd },
	{ "linkButton", gnoclLinkButtonCmd },
	{ "toggleButton", gnoclToggleButtonCmd },
	{ "scaleButton", gnoclScaleButtonCmd },
	{ "volumeButton", gnoclVolumeButtonCmd },
	{ "arrowButton", gnoclArrowButtonCmd },
	{ "recentChooser", gnoclRecentChooserCmd },
	{ "recentChooserDialog", gnoclRecentChooserDialogCmd },
	{ "fileChooserButton", gnoclFileChooserButtonCmd },
	{ "folderChooserButton", gnoclFolderChooserButtonCmd },
	{ "drawingArea", gnoclDrawingAreaCmd },
	{ "printerDialog", gnoclPrinterDialogCmd },
	{ "printDialog", gnoclPrintDialogCmd },
	{ "pageSetup", gnoclPageSetupCmd },
	{ "handleBox", gnoclHandleBoxCmd },

	{ "toolPalette", gnoclToolPaletteCmd },
	{ "toolItemGroup", gnoclToolItemGroupCmd },

	/* implement extra textbuffers */
	{ "textBuffer", gnoclTextBufferCmd },
	{ "assistant", gnoclAssistantCmd },

	/* miscellaneous funcs */
	{ "signalStop", gnoclSignalStopCmd },
	{ "pointer", gnoclPointerCmd },
	{ "exec", gnoclExecCmd },
	{ "toggle", gnoclToggleCmd },
	{ "setProperty", gnoclSetPropertyCmd },
	{ "setOpts", gnoclSetOpts},
	{ "showURI", gnoclShowUriCmd },
	{ "signalEmit", gnoclSignalEmitCmd },
	{ "beep", gnoclBeepCmd },
	{ "Hsv2Rgb", gnoclHsv2RgbCmd },
//	{ "string", gnoclStringCmd },
	{ "inventory", gnoclGetInventory },
	{ "stockItem", gnoclStockItemCmd },
	{ "bind", gnoclBindCmd },
	{ "colorButton", gnoclColorButtonCmd },
	{ "comboBox", gnoclComboBoxCmd },
	{ "comboEntry", gnoclComboEntryCmd },
	{ "expander", gnoclExpanderCmd },
	{ "fileChooser", gnoclFileChooserCmd },
	{ "fileChooserDialog", gnoclFileChooserDialogCmd },
	{ "folderChooserDialog", gnoclFileChooserDialogCmd },
	{ "fontSelectionDialog", gnoclFontSelectionDialogCmd },
	{ "fontButton", gnoclFontButtonCmd },
	{ "aboutDialog", gnoclAboutDialogCmd },
	{ "keyFile", gnoclKeyFileCmd},
	{ "tooltip", gnoclToolTip},

	{ "grab", gnoclGrabCmd},

#ifdef GNOCL_USE_GNOME
	{ "app", gnoclAppCmd },
	{ "appBar", gnoclAppBarCmd },
#endif
	{ NULL, NULL },
};

/**
\brief
\author
\date
\note
**/
int getCommandsNames ( Tcl_Interp *interp )
{

	GnoclCmd *cmds = commands;

	for ( ; cmds->name; ++cmds )
	{
		Tcl_AppendResult ( interp, cmds->name, " ", NULL );
	}

	return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * eventSetupProc --
 *
 *      This procedure implements the setup part of the gtk
 *      event source.  It is invoked by Tcl_DoOneEvent before entering
 *      the notifier to check for events on all displays.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      ??? TODO
 *      the maximum
 *      block time will be set to 0 to ensure that the notifier returns
 *      control to Tcl even if there is no more data on the X connection.
 *
 *----------------------------------------------------------------------
 */
static void eventSetupProc ( ClientData clientData, int flags )
{
	/*
	   TODO: make blockTime configurable
	         via Tcl_GetVar( "::gnocl::blockTime" )
	*/
	Tcl_Time blockTime = { 0, 10000 };

	if ( ! ( flags & TCL_WINDOW_EVENTS ) )
	{
		/* under which circumstances do we get here? */
		return;
	}

	/*
	   is this the correct way?
	   polling is bad, but how to do better?
	*/
	Tcl_SetMaxBlockTime ( &blockTime );

	return;
}


/*
 *----------------------------------------------------------------------
 *
 * eventCheckProc --
 *
 *      This procedure checks for events sitting in the gtk event
 *      queue.
 *
 * Results:
 *      None.
 *
 * Side effects:
 *      Moves queued events onto the Tcl event queue.
 *
 *----------------------------------------------------------------------
 */

typedef struct
{
	Tcl_Event header;      /* Standard information for all events. */
	GdkEvent event;
} GnoclEvent;


/**
\brief
\note
 *----------------------------------------------------------------------
 *
 * eventProc --
 *
 *      This procedure is called by Tcl_DoOneEvent when a window event
 *      reaches the front of the event queue.  This procedure is responsible
 *      for actually handling the event.
 *
 * Results:
 *      Returns 1 if the event was handled, meaning it should be removed
 *      from the queue.  Returns 0 if the event was not handled, meaning
 *      it should stay on the queue.  The event isn't handled if the
 *      TCL_WINDOW_EVENTS bit isn't set in flags, if a restrict proc
 *      prevents the event from being handled.
 *
 * Side effects:
 *      Whatever the event handlers for the event do.
 *
 *----------------------------------------------------------------------
 */
static int eventProc ( Tcl_Event *evPtr, int flags )
{
	/*
	   with while loop more responsive than
	   with Tcl_SetMaxBlockTime( 0 );
	   is this the right way?
	*/
	/* assert( gtk_events_pending() );
	   This is not true, since between eventCheckProc and eventProc
	   the event can have been handled by the gtk event loop if we
	   are in mainLoop
	*/
	gtk_main_iteration_do ( 0 );
	return 1;
}

/**
\brief
*/
static void eventCheckProc ( ClientData clientData, int flags )
{
	/* FIXME: what to do with flags? */
	if ( gtk_events_pending() )
	{
		/* Tcl_Time blockTime = { 0, 0 }; */
		GnoclEvent *gp = ( GnoclEvent * ) ckalloc ( sizeof ( GnoclEvent ) );
		gp->header.proc = eventProc;
		Tcl_QueueEvent ( ( Tcl_Event * ) gp, TCL_QUEUE_TAIL );

		/* Tcl_SetMaxBlockTime( &blockTime ); */

		/* gp->event = *gtk_get_current_event( ); */
	}

	return;
}

/**
\brief
**/
#ifdef GNOCL_USE_GNOME
static GnomeAppBar *gnoclAppBar = NULL;
int gnoclRegisterHintAppBar (
	GtkWidget *widget,
	GnomeAppBar *appBar )
{
	gnoclAppBar = appBar;
	return 0;
}

GnomeAppBar *gnoclGetHintAppBar ( GtkWidget *widget )
{
	if ( gnoclAppBar != NULL )
	{
		GtkWidget *top = gtk_widget_get_toplevel ( GTK_WIDGET ( gnoclAppBar ) );
		GtkWidget *wTop = widget->parent;

		/*
		if( top == wTop )
		   return gnoclAppBar;
		else
		*/

		while ( wTop != NULL && GTK_CHECK_TYPE ( wTop, GTK_TYPE_MENU ) )
		{
			wTop = gtk_menu_get_attach_widget ( GTK_MENU ( wTop ) );
			/* this happens for popup menus */

			if ( wTop == NULL )
				break;

			if ( gtk_widget_get_toplevel ( wTop ) == top )
				return gnoclAppBar;

			wTop = wTop->parent;
		}

		printf ( "DEBUG: gnoclAppBar not found\n" );
	}

	return NULL;
}

/**
\brief
*/
static void putHintInAppBar (
	GtkWidget* menuitem,
	gpointer data
)
{
	/*
	MenuLabelParams *para = (MenuLabelParams *)data;
	const char *txt = gtk_object_get_data( GTK_OBJECT(menuitem),
	      "gnocl_tooltip" );
	*/
	const char *txt = ( const char * ) data;
	GnomeAppBar *appBar = gnoclGetHintAppBar ( GTK_WIDGET ( menuitem ) );

	if ( appBar )
		gnome_appbar_set_status ( appBar, txt );
}

/**
\brief
*/
static void removeHintFromAppBar (
	GtkWidget *menuitem,
	gpointer data )
{
	GnomeAppBar *appBar = gnoclGetHintAppBar ( GTK_WIDGET ( data ) );

	if ( appBar )
		gnome_appbar_refresh ( appBar );
}

#endif

/**
\brief
\todo   Update using new modules, current method deprecated since Gtk+ 2.12
**/
GtkTooltips *gnoclGetTooltips( )
{
	static GtkTooltips *tooltips = NULL;

	if ( tooltips == NULL )
	{
		tooltips = gtk_tooltips_new( );
	}

	return tooltips;
}

/**
\brief
**/
GtkAccelGroup *gnoclGetAccelGroup( )
{
	static GtkAccelGroup *accelGroup = NULL;

	if ( accelGroup == NULL )
	{
		accelGroup = gtk_accel_group_new();
	}

	/* return gtk_accel_group_get_default( ); */
	return accelGroup;
}

/**
\brief
**/
static gint tclTimerFunc ( gpointer data )
{
	/* Tcl_Interp *interp = (Tcl_Interp *)data; */
	while ( Tcl_DoOneEvent ( TCL_DONT_WAIT ) )
	{
		/* printf( "t" ); fflush( stdout ); */
		if ( gtk_events_pending() )
		{
			break;
		}
	}

	/* TODO: or Tcl_ServiceAll(); ? */
	return 1;
}

/**
\brief
**/
int gnoclMainLoop (	ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{

	/*--------------------------------------*/
	static GnoclOption options[] =
	{
		{ "-timeout", GNOCL_STRING, NULL },
		{ NULL },
	};

	static const char *cmds[] =  { NULL, NULL};

	if ( gnoclGetCmdsAndOpts ( interp, cmds, options, objv, objc ) == TCL_OK )
	{
		return TCL_OK;
	}

	/*--------------------------------------*/


	guint32 timeout = 100;

	/* TODO? flag for not looping in tclTimerFunc? */

	if ( ( objc != 1 && objc != 3 ) || ( objc == 3 && strcmp ( Tcl_GetString ( objv[1] ), "-timeout" ) ) )
	{
		Tcl_WrongNumArgs ( interp, 1, objv, "?-timeout val?" );
		return TCL_ERROR;
	}

	if ( objc == 3 )
	{
		long    val;

		if ( Tcl_GetLongFromObj ( interp, objv[2], &val ) != TCL_OK )
			return TCL_ERROR;

		/* beware of problems with casting signed -> unsigned! */
		if ( val < 0 )
		{
			Tcl_SetResult ( interp, "Timeout value must be greater or equal zero.", TCL_STATIC );
			return TCL_ERROR;
		}

		timeout = ( guint32 ) val;
	}

	/* TODO
	   this prevents the rekursive calls of tcl and gtk event loops
	   but then we can't use "vwait var" if var is set in a gnocl::callback
	Tcl_DeleteEventSource( eventSetupProc, eventCheckProc, interp );
	*/

	/* if timeout == 0 don't call tcl's event loop */

	if ( timeout > 0 )
	{
		g_timeout_add ( timeout, tclTimerFunc, NULL );
	}

	/* TODO? use gtk_idle_add( tclTimerFunc, NULL ); ? */

	gtk_main();

	return 0;
}

/**
\brief
**/
char **gnoclGetArgv (
	Tcl_Interp *interp,
	int *pargc )
{
	int  narg;
	char **argv;
	int k;

	typedef char *Pchar;

	Tcl_Obj *obj = Tcl_ObjGetVar2 ( interp, Tcl_NewStringObj ( "::argv", -1 ), NULL, 0 );

	if ( obj == NULL || Tcl_ListObjLength ( NULL, obj, &narg ) != TCL_OK )
	{
		narg = 0;
	}

	argv = g_new ( Pchar, narg + 2 );

	argv[0] = ( char * ) gnoclGetAppName ( interp );

	for ( k = 0; k < narg; ++k )
	{
		Tcl_Obj *child;

		if ( Tcl_ListObjIndex ( NULL, obj, k, &child ) == TCL_OK )
		{
			argv[k+1] = Tcl_GetString ( child );
		}

		else
		{
			argv[k+1] = "";
		}
	}

	argv[narg+1] = NULL;

	*pargc = narg + 1;

	return argv;
}

/**
\brief
**/
const char *gnoclGetAppName ( Tcl_Interp *interp )
{
	const char *ret = Tcl_GetVar ( interp, "gnocl::appName", TCL_GLOBAL_ONLY );

	if ( ret == NULL )
		ret = Tcl_GetVar ( interp, "argv0", TCL_GLOBAL_ONLY );

	if ( ret == NULL )
		ret = "gnocl";

	return ret;
}

/**
\brief
**/
const char *gnoclGetAppVersion ( Tcl_Interp *interp )
{
	const char *ret = Tcl_GetVar ( interp, "gnocl::appVersion",
								   TCL_GLOBAL_ONLY );

	if ( ret == NULL )
		ret = VERSION;

	return ret;
}

/**
\brief
**/
int Gnocl_Init ( Tcl_Interp *interp )
{
	char cmdBuf[128] = "gnocl::";
	int k;
	int argc;
	char **argv;
	char **argvp;

	/* printf( "Initializing gnocl version %s\n", VERSION ); */

	if ( Tcl_InitStubs ( interp, "8.3", 0 ) == NULL )
	{
		return TCL_ERROR;
	}

	/* TODO? set locale before every jump to Tcl? */
	Tcl_PutEnv ( "LC_NUMERIC=C" );

	argvp = argv = gnoclGetArgv ( interp, &argc );

	if ( !gtk_init_check ( &argc, &argvp ) )
	{
		Tcl_SetResult ( interp, "could not initialize gtk", TCL_STATIC );
		return TCL_ERROR;
	}

	/* TODO: change argv in Tcl */
	g_free ( argv );

	Tcl_CreateEventSource ( eventSetupProc, eventCheckProc, interp );

	/* Tcl_CreateExitHandler( exitHandler, NULL ); */
	if ( Tcl_PkgProvide ( interp, "Gnocl", VERSION ) != TCL_OK )
		return TCL_ERROR;

	/* Create command within the interpret to make new widgets available */
	for ( k = 0; commands[k].name; ++k )
	{

		strcpy ( cmdBuf + 7, commands[k].name );
		Tcl_CreateObjCommand ( interp, cmdBuf, commands[k]. proc, NULL, NULL );
	}

	/* initialize the hash table to contain list of pointers to named widgets */
	name2widgetList = g_hash_table_new ( g_direct_hash, g_direct_equal );


	/* FIXME: is there a more elegant way? */
	/*        use gtk_idle_add( tclTimerFunc, NULL ); ? */
	g_timeout_add ( 100, tclTimerFunc, NULL );
	Tcl_SetMainLoop ( gtk_main );

	return TCL_OK;
}


