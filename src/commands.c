/*
 * $Id: commands.c,v 1.9 2005/02/22 23:16:10 baum Exp $
 *
 * This file implements a Tcl interface to GTK+ and Gnome
 *
 * Copyright (c) 2001 - 2005 Peter G. Baum  http://www.dr-baum.net
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

/*
   History:
   2014-05: Added GtkTextView pseudo-styles: "rollover-fg" and "rollover-bg".
			e.g. gnocl::setStyle GtkTextView "rollover-bg" yellow
   2014-01: added gnocl::iconTheme
   2013-09: added gnocl::pango
				validate markup strings
   2013-03: bug-fixed gnocl::setStyle
   2013-02: added utility command, gnocl::toggle
   2012-01: gnocl::winfo
				added exists subcommand
   2012-11: added gnocl::tooltip
   2012-03: added gnocl::lset
   2011-12: added gnocl::showURI
   2011-08: added gnocl::exec
   2010-04: added gnocl::sound moved to gnome package
   2010-10: added gnocl::stockItem
   2010-08: added gnocl::pointer
   2010-07: added gnocl::sound
   				  gnocl::parseColor			-still has problems
   				  gnocl::GetStyleCmd
   2010-06: added gnocl::winfo fonts
				  gnocl::winfo path
   2010-05: added gnocl::winfo pointer
   2009-04: added gnocl::signalStop
   2009-03: added gnocl::grab, gnocl::beep
            added gnocl::winfo geometry
   2009-03: added gnocl::winfo
   2009-02: added gnocl::resource
   2003-03: split from gnocl.c
**/

/**
\page page_clipboard gnocl::clipboard
\htmlinclude clipboard.html
**/

/**
\page page_configure gnocl::configure
\htmlinclude configure.html
**/

/**
\page page_gconf gnocl::gconf
\htmlinclude gconf.html
**/

/**
\page page_info gnocl::info
\htmlinclude info.html
**/

/**
\page page_pointer gnocl::pointer
\htmlinclude pointer.html
**/

/**
\page page_resourceFile gnocl::resourceFile
\htmlinclude resourceFile.html
**/

/**
\page page_screen gnocl::screen
\htmlinclude screen.html
**/

/**
\page page_signalStop gnocl::signalStop
\htmlinclude signalStop.html
**/

/**
\page page_stockitem gnocl::stockItem
\htmlinclude stockitem.html
**/

/**
\page page_update gnocl::update
\htmlinclude update.html
**/

/**
\page page_winfo gnocl::winfo
\htmlinclude winfo.html
**/

#include "gnocl.h"
#include <ctype.h>


/**
\brief	Interface to basic pango related functions.
\date	07/Sep/13
**/
int gnoclPangoCmd ( ClientData data, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[] )
{

#if 0
	g_print ( "%s %s %s\n", __FUNCTION__,  Tcl_GetString ( objv[1] ) , Tcl_GetString ( objv[2] ) );
#endif

	static const char *types[] =
	{
		"validate", "strip",
		NULL
	};

	enum typesIdx
	{
		ValidateIdx, StripIdx
	};

	gint idx;
	gchar *text = NULL;

	getIdx ( types, Tcl_GetString ( objv[1] ), &idx );

	switch ( idx )
	{
			/* return string, original if valid, stripped if not */
		case ValidateIdx :
			{
				if (  pango_parse_markup ( Tcl_GetStringFromObj ( objv[2], NULL ), -1 , NULL, NULL, &text, NULL, NULL ) == 0 )
				{
					Tcl_SetResult ( interp, text , TCL_STATIC );
				}

				else
				{
					Tcl_SetResult ( interp,  Tcl_GetString ( objv[2] ) , TCL_STATIC );
				}
			}
			break;
			/* return string stripped of all markup */
		case StripIdx:
			{
				pango_parse_markup ( Tcl_GetStringFromObj ( objv[2], NULL ), -1 , NULL, NULL, &text, NULL, NULL );
				Tcl_SetResult ( interp, text , TCL_STATIC );
			}

			break;
	}

	return TCL_OK;
}


/**
\brief Modify this callback to present data to the Tcl interpreter
**/
static void post_process ( GPid pid, int status, gpointer data )
{
	printf ( "---------------------->Child is done\n" );
}


/**
\brief	block create a set of values, stripping away leading '
**/
int gnoclSetOpts ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{

	int argc, code, i;
	char *string;
	char **argv;

	char str[50];

	code = Tcl_SplitList ( interp, Tcl_GetString ( objv[1] ), &argc, &argv );

	for ( i = 0; i < argc; i += 2 )
	{
		sprintf ( str, "%s", argv[i] );

		if ( str[0] == '-' )
		{
			Tcl_SetVar ( interp, substring ( argv[i], 1, strlen ( argv[i] ) ), argv[i+1], NULL );
		}

		else
		{
			Tcl_SetVar ( interp, argv[i], argv[i+1], NULL );
		}
	}

	Tcl_Free ( ( char * ) argv );

	return TCL_OK;
}


/*
\brief  EXPIREMENTAL STUB: Explore the possibilty of setting an object property
*/
int gnoclSetPropertyCmd ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{

#ifdef DEBUG_COMMANDS
	g_print ( "%s\n", __FUNCTION__ );
#endif

	//GPid cpid;
	//char *com[] = {"./ls"};
	//char str[6];

	g_print ( "widget = %s\n", Tcl_GetString ( objv[1] ) );
	g_print ( "property = %s\n", Tcl_GetString ( objv[2] ) );
	g_print ( "value = %s\n", Tcl_GetString ( objv[3] ) );
	g_print ( "type = %s\n", Tcl_GetString ( objv[4] ) );

	static const char *types[] =
	{
		"i", "d", "c", "b",
		"int", "double", "char", "bool",
		NULL
	};

	enum typesIdx
	{
		IIdx, DIdx, CIdx, BIdx,
		IntIdx, DoubleIdx, CharIdx, BoolIdx
	};

	gint idx;
	gboolean b;
	gint i;
	gfloat d;
	GtkWidget *widget;

	getIdx ( types, Tcl_GetString ( objv[4] ), &idx );

	switch ( idx )
	{
		case BIdx:
		case BoolIdx:
			{
				widget = gnoclGetWidgetFromName (  Tcl_GetString ( objv[1] ), interp );
				Tcl_GetIntFromObj ( interp, objv[2], &b );
				g_object_set ( G_OBJECT ( widget ), Tcl_GetString ( objv[2] ), i, NULL );
			}
			break;
defaul:
			{
			}
	}





	//Tcl_SetResult ( interp, str , TCL_STATIC );
	return TCL_OK;
}


/**
\brief	simple boolean toggle command
**/
int gnoclToggleCmd ( ClientData data, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[] )
{

	const char *val = Tcl_GetString ( objv[1] );

	if ( strcmp ( val, "1" ) == 0 ||
			strcmp ( val, "TRUE" ) == 0 ||
			strcmp  ( val, "ON" ) == 0  )
	{
		Tcl_SetResult ( interp, "0" , TCL_STATIC );
		return TCL_OK;
	}

	if ( strcmp ( val, "0" ) == 0 ||
			strcmp ( val, "FALSE" ) == 0 ||
			strcmp  ( val, "OFF" ) == 0 )
	{
		Tcl_SetResult ( interp, "1" , TCL_STATIC );
		return TCL_OK;
	}

	Tcl_SetResult ( interp, "ERROR: Must be one of 1 0 TRUE FALSE ON or OFF." , TCL_STATIC );
	return TCL_ERROR;

}

/**
\brief  Return string values in a GList as a Tcl list.
**/
int glist2TclList ( Tcl_Interp *interp, GSList *list, Tcl_Obj *obj )

{
	gint n;
	GSList *p;

	n = 0;

	for ( p = list; p != NULL; p = p->next )
	{
		Tcl_ListObjAppendElement ( interp, obj, Tcl_NewStringObj ( p->data, -1 ) );
		n++;
	}

	return n;
}


/**
\brief  Wrapper around the Gtk+ Icontheme API.
**/
int gnoclIconThemeCmd ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{

#if 0
	g_print ( "cmd = %s\n", Tcl_GetString ( objv[1] ) );
#endif


	Tcl_Obj *res = Tcl_NewListObj ( 0, NULL );
	GtkIconTheme *icon_theme = gtk_icon_theme_get_default();

	GSList *p;
	GSList *i;


	static const char *cmds[] =
	{
		"contexts", "hasIcon", "icons", "load",
		NULL
	};

	static GnoclOption options[] =
	{
		{ NULL, GNOCL_STRING, NULL },
		{ NULL },
	};

	if ( gnoclGetCmdsAndOpts ( interp, cmds, options, objv, objc ) == TCL_OK )
	{
		return TCL_OK;
	}

	/*--------------------------------------*/

	enum optIdx
	{
		ContextsIdx, HasIconIdx, IconsIdx, LoadIdx
	};

	int idx;

	if ( objc != 2 && objc != 4 )
	{
		Tcl_WrongNumArgs ( interp, 1, objv, "option" );
		return TCL_ERROR;
	}

	if ( Tcl_GetIndexFromObj ( interp, objv[1], cmds, "option", TCL_EXACT, &idx ) != TCL_OK )
	{
		return TCL_ERROR;
	}

	switch ( idx )
	{
		case LoadIdx:
			{
				/* create a pixbuf containing the specified icon */
				/*
				GdkPixbuf * gtk_icon_theme_load_icon (GtkIconTheme *icon_theme,
				                                      const gchar *icon_name,
				                                      gint size,
				                                      GtkIconLookupFlags flags,
				                                      GError **error);
				*/
				/*
				typedef enum
				{
				  GTK_ICON_LOOKUP_NO_SVG           = 1 << 0,
				  GTK_ICON_LOOKUP_FORCE_SVG        = 1 << 1,
				  GTK_ICON_LOOKUP_USE_BUILTIN      = 1 << 2,
				  GTK_ICON_LOOKUP_GENERIC_FALLBACK = 1 << 3,
				  GTK_ICON_LOOKUP_FORCE_SIZE       = 1 << 4
				} GtkIconLookupFlags;

				typedef enum
				{
				  GTK_ICON_SIZE_INVALID,
				  GTK_ICON_SIZE_MENU,
				  GTK_ICON_SIZE_SMALL_TOOLBAR,
				  GTK_ICON_SIZE_LARGE_TOOLBAR,
				  GTK_ICON_SIZE_BUTTON,
				  GTK_ICON_SIZE_DND,
				  GTK_ICON_SIZE_DIALOG
				} GtkIconSize;

				*/

				/*
								if ( objc == 4 )
								{
									if ( strcmp ( Tcl_GetString ( objv[2] ), "-size" ) == 0 )
									{
										context = Tcl_GetString ( objv[3] );
									}
								}
				*/

				GdkPixbuf *pix;
				gchar *icon_name;
				gint size;



				return gnoclCreatePixbufFromIcon ( interp, icon_theme, icon_name, GTK_ICON_SIZE_BUTTON );
			}
			break;
		case HasIconIdx:
			{
				// gboolean gtk_icon_theme_has_icon ( GtkIconTheme * icon_theme, const gchar * icon_name );
			}
			break;
		case ContextsIdx:
			{

				i = gtk_icon_theme_list_contexts ( icon_theme );

#if 0
				glist2TclList ( interp, i , res ); // something not quite right with this func
#else

				for ( p = i; p != NULL; p = p->next )
				{

					Tcl_ListObjAppendElement ( interp, res, Tcl_NewStringObj ( p->data, -1 ) );

				}

#endif
				Tcl_SetObjResult ( interp, res );
			}
			break;
		case IconsIdx:
			{
				//g_slist_free ( p );
				//g_slist_free ( ids );

				gchar *context = NULL;

				if ( objc == 4 )
				{
					if ( strcmp ( Tcl_GetString ( objv[2] ), "-context" ) == 0 )
					{
						context = Tcl_GetString ( objv[3] );
					}
				}

				i = gtk_icon_theme_list_icons ( icon_theme, context );

				for ( p = i; p != NULL; p = p->next )
				{
					Tcl_ListObjAppendElement ( interp, res, Tcl_NewStringObj ( ( char * ) p->data, -1 ) );
				}

				//g_slist_free ( ids );

				Tcl_SetObjResult ( interp, res );
			}

			break;
	}

	g_slist_free ( p );
	g_slist_free ( i );

	return TCL_OK;
}



/**
\brief  Spawn a background process.
**/
int gnoclExecCmd ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{


	static GnoclOption options[] =
	{
		{ NULL, GNOCL_STRING, NULL },
		{ NULL },
	};

	static const char *cmds[] =  { NULL, NULL};

	if ( gnoclGetCmdsAndOpts ( interp, cmds, options, objv, objc ) == TCL_OK )
	{
		return TCL_OK;
	}


	GPid cpid;
	char *com[] = {"./ls"};
	char str[6];

#if 0
	g_print ( "cmd = %s\n", Tcl_GetString ( objv[1] ) );
#endif

	if ( g_spawn_async ( NULL, com, NULL, G_SPAWN_DO_NOT_REAP_CHILD, NULL, NULL, &cpid, NULL ) )
	{
		sprintf ( str, "%d", cpid );
		g_child_watch_add ( cpid, ( GChildWatchFunc ) post_process, NULL );
	}

	Tcl_SetResult ( interp, str , TCL_STATIC );
	return TCL_OK;
}


/**
\brief	Launch the default application to view specified URI.

	file:///home/gnome/pict.jpg
	http://www.gnome.org
	mailto:me@gnome.org
**/
int gnoclShowUriCmd ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{

	/*--------------------------------
		NOT USED AT THE MOMENT
	--------------------------------*/
	static GnoclOption options[] =
	{
		{ NULL, GNOCL_STRING, NULL },
		{ NULL },
	};

	static const char *cmds[] =  { NULL, NULL};

	if ( gnoclGetCmdsAndOpts ( interp, cmds, options, objv, objc ) == TCL_OK )
	{
		return TCL_OK;
	}

	/*--------------------------------*/

	gint ret = 0;
	const char *uri = Tcl_GetString ( objv[1] );

	if ( gtk_show_uri  ( NULL, uri, GDK_CURRENT_TIME, NULL ) )
	{
		return TCL_OK;
	}

	else
	{
		Tcl_SetResult ( interp, "ERROR: Invalid URI.\n" , TCL_STATIC );
		return TCL_OK;
	}
}

/**
\brief      Manipulate the screen pointer.

static int configure ( Tcl_Interp *interp, GtkImage *image, GnoclOption options[] )
int gnoclGetStockItem ( Tcl_Obj *obj, Tcl_Interp *interp, GtkStockItem *sp )

**/
int gnoclPointerCmd ( ClientData data, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[] )
{



	/*--------------------------------------*/
	static GnoclOption options[] =
	{
		{ NULL, GNOCL_STRING, NULL },
		{ NULL },
	};

	static const char *cmds[] = { "warp", "set", NULL };

	if ( gnoclGetCmdsAndOpts ( interp, cmds, options, objv, objc ) == TCL_OK )
	{
		return TCL_OK;
	}

	/*--------------------------------------*/



	enum optIdx { WarpIdx, SetIdx };
	int idx1;

	if ( objc <= 3 )
	{
		Tcl_WrongNumArgs ( interp, 1, objv, "option value " );
		return TCL_ERROR;
	}

	if ( Tcl_GetIndexFromObj ( interp, objv[1], cmds, "option", TCL_EXACT, &idx1 ) != TCL_OK )
	{
		return TCL_ERROR;
	}

	switch ( idx1 )
	{

			/* move the pointer to specified screen coordinates */
		case WarpIdx:
			{

				listParameters ( objc, objv );

				gint x = 0, y = 0, xadd = 0, yadd = 0;
				GdkDisplay *display = NULL;
				GdkScreen *screen = NULL;

				int i, idx2;
				static const char *opts[] = {"-x", "-y", "-offset", "-set", NULL};
				enum optsIdx { XIdx, YIdx, OffsetIdx, SetIdx };

				i = 0;
				idx2 = 0;

				/* get default display and screen */
				display = gdk_display_get_default ();
				screen = gdk_display_get_default_screen ( display );

				gdk_display_get_pointer ( display, NULL, &x, &y, NULL );

				/* parse all the options */
				while ( i <  objc )
				{

					getIdx ( opts, Tcl_GetString ( objv[i] ), &idx2 );

					switch ( idx2 )
					{
						case XIdx:
							{
								sscanf ( Tcl_GetString ( objv[i+1] ), "%d", &x );
							}
							break;
						case YIdx:
							{
								sscanf ( Tcl_GetString ( objv[i+1] ), "%d", &y );
							}
							break;
							/* is this right? check against the docs */
						case OffsetIdx:
							{
								sscanf ( Tcl_GetString ( objv[i+1] ), "%d %d", &xadd, &yadd );
								x += xadd;
								y += yadd;
							}
							break;
					}

					i += 2;

				}

				gdk_display_warp_pointer ( display, screen, x, y );
			}
			break;
		case SetIdx:
			{
				if ( objc < 6 )
				{
					Tcl_WrongNumArgs ( interp, 1, objv, "option value " );
					return TCL_ERROR;
				}

				GtkWidget *widget;
				GdkCursor *cursor;
				GdkDisplay *display;
				GdkPixbuf *pixbuf;
				gchar *imageName;
				GnoclStringType type;

				gint xoffset = 0;
				gint yoffset = 0;

				int i, idx2;
				static const char *opts[] = {"-widget", "-image", "-offset", NULL};
				enum optsIdx { WidgetIdx, ImageIdx, OffsetIdx };

				i = 0;
				idx2 = 0;

				/* get default display*/
				display = gdk_display_get_default ();

				/* parse all the options */
				while ( i <  objc )
				{

					getIdx ( opts, Tcl_GetString ( objv[i] ), &idx2 );

					switch ( idx2 )
					{
						case WidgetIdx:
							{
								widget = gnoclGetWidgetFromName (  Tcl_GetString ( objv[i+1] ), interp );
							}
							break;
						case ImageIdx:
							{
								imageName = gnoclGetString ( objv[i+1] );
								type = gnoclGetStringType ( objv[i+1] );
							}
							break;
						case OffsetIdx:
							{
								sscanf ( Tcl_GetString ( objv[i+1] ), "%d %d", &xoffset, &yoffset );
							}
							break;
					}

					i += 2;
				}


				switch ( type & ( GNOCL_STR_FILE | GNOCL_STR_STOCK | GNOCL_STR_BUFFER ) )
				{
						/* implement this feature first */
					case GNOCL_STR_FILE:
						{

							pixbuf = gdk_pixbuf_new_from_file ( imageName, NULL );

							if ( pixbuf == NULL )
							{
								Tcl_SetResult ( interp, "ERROR. Image file not found.\n", TCL_STATIC );
								return TCL_ERROR;
							}

							cursor = gdk_cursor_new_from_pixbuf ( display, pixbuf, xoffset, yoffset );
							gdk_window_set_cursor ( GTK_WIDGET ( widget )->window, cursor );

						}

						break;
					case GNOCL_STR_STOCK:
						{
							Tcl_SetResult ( interp, "ERROR: Cannot create cursor from stock item.\n" , TCL_STATIC );
							return TCL_ERROR;
						}

						break;
					case GNOCL_STR_BUFFER:
						{

							GdkPixbuf *pixbuf;
							pixbuf = gnoclGetPixBufFromName ( imageName, interp );

							if ( pixbuf == NULL )
							{
								Tcl_SetResult ( interp, "Pixbuf does not exist.", TCL_STATIC );
								return TCL_ERROR;
							}

							cursor = gdk_cursor_new_from_pixbuf ( display, pixbuf, xoffset, yoffset );
							gdk_window_set_cursor ( GTK_WIDGET ( widget )->window, cursor );

						}
						break;
					default:
						Tcl_AppendResult ( interp, "Unknown type for \"", imageName, "\" must be of type FILE (%/) or STOCK (%#)", NULL );
						return TCL_ERROR;
				}


// GdkPixbuf *gnoclGetPixBufFromName ( const char *id, Tcl_Interp *interp );
// gdk_pixbuf_render_pixmap_and_mask   (GdkPixbuf *pixbuf, GdkPixmap **pixmap_return, GdkBitmap **mask_return, int alpha_threshold);
// void  gdk_window_set_cursor (GdkWindow *window, GdkCursor *cursor);


			}
			break;

		default:
			{
				/* do nothing yet */
			}

	}

	return TCL_OK;
}




/**
\brief      Perform an effective break, let the script decide what to do next.
**/
int gnoclSignalStopCmd ( ClientData data, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[] )
{

	/*--------------------------------
		NOT USED AT THE MOMENT
	--------------------------------*/
	static GnoclOption options[] =
	{
		{ "-widget", GNOCL_STRING, NULL },
		{ "-signal", GNOCL_STRING, NULL },
		{ NULL },
	};

	static const char *cmds[] =  { NULL, NULL};

	if ( gnoclGetCmdsAndOpts ( interp, cmds, options, objv, objc ) == TCL_OK )
	{
		return TCL_OK;
	}

	/*--------------------------------*/

	if ( objc != 3 )
	{
		Tcl_WrongNumArgs ( interp, 1, objv, "widget signal-type" );
		return TCL_ERROR;
	}

	GtkWidget *widget = gnoclGetWidgetFromName (  Tcl_GetString ( objv[1] ), interp );


	if ( strcmp ( Tcl_GetString ( objv[2] ), "motionNotify" ) == 0 )
	{
		g_signal_stop_emission_by_name ( widget , "motion-notify-event" );
		return TCL_OK;
	}

	if ( strcmp ( Tcl_GetString ( objv[2] ), "button2Press" ) == 0 )
	{
		g_signal_stop_emission_by_name ( widget , "button_press_event" );
		return TCL_OK;
	}

	if ( strcmp ( Tcl_GetString ( objv[2] ), "button3Press" ) == 0 )
	{
		g_signal_stop_emission_by_name ( widget , "button_press_event" );
		return TCL_OK;
	}


	g_signal_stop_emission_by_name ( widget, Tcl_GetString ( objv[2] ) );
	return TCL_OK;
}

/**
\brief
**/
int gnoclSignalEmitCmd ( ClientData data, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[] )
{
	/*--------------------------------
		NOT USED AT THE MOMENT
	--------------------------------*/
	static GnoclOption options[] =
	{
		{ "-widget", GNOCL_STRING, NULL },
		{ "-signal", GNOCL_STRING, NULL },
		{ NULL },
	};

	static const char *cmds[] =  { NULL, NULL};

	if ( gnoclGetCmdsAndOpts ( interp, cmds, options, objv, objc ) == TCL_OK )
	{
		return TCL_OK;
	}

	/*--------------------------------*/
	if ( objc != 3 )
	{
		Tcl_WrongNumArgs ( interp, 1, objv, "widget signal-type" );
		return TCL_ERROR;
	}

	GtkWidget *widget = gnoclGetWidgetFromName (  Tcl_GetString ( objv[1] ), interp );

	g_signal_emit_by_name ( widget , Tcl_GetString ( objv[2] ), NULL );
	return TCL_OK;
}



/**
\brief      Provide gnocl parallel to the Tk bind command
**/
int gnoclBindCmd ( ClientData data, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[] )
{

#ifdef DEBUG_COMMANDS
	g_print ( "gnoclBingCmd STUB ONLY\n" );
#endif
	g_print ( "gnoclBingCmd STUB ONLY\n" );
	return TCL_OK;
}


/**
\brief      Provide access to glib string manipulation commands.
\notes		http://developer.gnome.org/glib/stable/glib-Unicode-Manipulation.html#g-unichar-to-utf8
**/
int gnoclStringCmd ( ClientData data, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[] )
{

#if 1
	g_print ( "%s %s %s\n", __FUNCTION__,  Tcl_GetString ( objv[1] ) , Tcl_GetString ( objv[2] ) );
#endif


	static const char *cmd[] =
	{
		"unichar_to_utf8",
		NULL
	};
	enum optIdx
	{
		unichar_to_utf8_Idx
	};

	int idx;

	if ( objc != 3 )
	{
		Tcl_WrongNumArgs ( interp, 1, objv, "--- NEED AN ERROR MESSAGE ---" );
		return TCL_ERROR;
	}

	if ( Tcl_GetIndexFromObj ( interp, objv[1], cmd, "option", TCL_EXACT, &idx ) != TCL_OK )
	{
		return TCL_ERROR;
	}

	/*
	    g_print ( "opt = %s  id = %s\n",  Tcl_GetString ( objv[1] ),  Tcl_GetString ( objv[2] ) );
	*/
	switch ( idx )
	{

		case unichar_to_utf8_Idx:
			{
				gchar outbuf[6];

				// gint g_unichar_to_utf8 (gunichar c, gchar *outbuf);
				gunichar c;
				gint res;

				sscanf ( Tcl_GetString ( objv[2] ), "U+%06"G_GINT32_FORMAT"X", &c );

				res = g_unichar_validate ( Tcl_GetString ( objv[2] ) );

				g_print ( "valid = %d\n", res );

				res = g_unichar_to_utf8 ( c	, outbuf );

				g_print ( "res = %d %s\n", res, outbuf );

				Tcl_SetObjResult ( interp, Tcl_NewStringObj ( outbuf, -1 ) );

			}
		default: {}
	}




	return TCL_OK;
}

/**
\brief      Set custom widget tooltip window or triggers a new tooltip query on display,
\note       Used in order to update the contents of a visible tooltip.
**/
int gnoclToolTip ( ClientData data, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[] )
{

	/* these are not used within the actual code, only for feeback purposes! */
	static GnoclOption options[] =
	{
		{ "-window", GNOCL_STRING, NULL },
		{ "-position", GNOCL_STRING, NULL },
		{ NULL },
	};

	static const char *cmds[] =  { "trigger", NULL};

	if ( gnoclGetCmdsAndOpts ( interp, cmds, options, objv, objc ) == TCL_OK )
	{
		return TCL_OK;
	}

	/*------------------*/

	static const char *opts[] = { "-window", "-position", NULL};

#ifdef DEBUG_COMMANDS
	g_print ( "%s : objc = %d\n", __FUNCTION__, objc );
#endif

	int idx;
	int i = 2;
	GtkWidget *widget, *window;
	GdkDisplay *display;


	enum optsIdx { WindowIdx, PositionIdx };


	if ( objc < 1 )
	{
		Tcl_WrongNumArgs ( interp, 1, objv, "Wrong number of arguments. Must be trigger or widget-id -window tooltip-id" );
		return TCL_ERROR;
	}

	if ( strcmp ( Tcl_GetString ( objv[1] ), "trigger" ) == 0 )
	{
		display = gdk_display_get_default ();
		gtk_tooltip_trigger_tooltip_query ( display );

		return TCL_OK;
	}

	widget = gnoclGetWidgetFromName ( Tcl_GetString ( objv[1] ), interp );


	/* parse all the options */
	while ( i <  objc )
	{

#ifdef DEBUG_COMMANDS
		g_print ( "\t%d\n", i );
#endif
		getIdx ( opts, Tcl_GetString ( objv[i] ), &idx );

		switch ( idx )
		{
			case WindowIdx:
				{
					window = gnoclGetWidgetFromName ( Tcl_GetString ( objv[i+1] ), interp );
					gtk_widget_set_tooltip_window ( widget, window );
					//return TCL_OK;
				}

				/* this options will not work properly, perhaps the positioning is controlled by Gtk itself */
			case PositionIdx:
				{
					GtkWidget *parent, *child;
					gint x, y;

					sscanf ( Tcl_GetString ( objv[i+1] ), "%d %d", &x, &y );

					child = gtk_bin_get_child ( GTK_BIN ( widget ) );
					parent = gtk_widget_get_parent ( GTK_BIN ( widget ) );

					//window = gtk_widget_get_tooltip_window ( parent );
					//window = gtk_widget_get_tooltip_window ( child );
					window = gtk_widget_get_tooltip_window ( widget );

					if ( window == NULL )
					{
						g_print ( "NO WINDOW!\n" );
					}

					gtk_window_move ( GTK_WINDOW ( window ) , x, y );

					//return TCL_OK;
				}
		}

		i += 2;

	}

	display = gdk_display_get_default ();
	gtk_tooltip_trigger_tooltip_query ( display );

	return TCL_OK;
}

/**
\brief      Convert HSV values to RGB
\note
**/
int gnoclHsv2RgbCmd ( ClientData data, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[] )
{

	/*--------------------------------------*/
	static GnoclOption options[] =
	{
		{ NULL, GNOCL_STRING, NULL },
		{ NULL },
	};

	static const char *cmds[] =  { NULL, NULL};

	if ( gnoclGetCmdsAndOpts ( interp, cmds, options, objv, objc ) == TCL_OK )
	{
		return TCL_OK;
	}

	/*--------------------------------------*/


#ifdef DEBUG_COMMANDS
	g_print ( "%s %s\n", __FUNCTION__, Tcl_GetString ( objv[1] ) );
#endif
	g_print ( "%s %s\n", __FUNCTION__, Tcl_GetString ( objv[1] ) );

	gdouble h, s, v;
	gdouble r , g, b;

	gchar str[32];

	sscanf ( Tcl_GetString ( objv[1] ), "%f %f %f", &h, &s, &v );

	printf ( "h %f s %f v %f\n", h, s, v );

	gtk_hsv_to_rgb ( h, s, v, &r, &g, &b );

	sprintf ( str, "%d %d %d", ( int ) r * 255, ( int ) g * 255, ( int ) b * 255 ) ;

	Tcl_SetObjResult ( interp, Tcl_NewStringObj ( str, -1 ) );

	return TCL_OK;
}

/**
\brief      Emits a short beep.
\note       Gdk Documentation for the implemented features.
            http://library.gnome.org/devel/gdk/stable/gdk-General.html
**/
int gnoclBeepCmd ( ClientData data, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[] )
{

#ifdef DEBUG_COMMANDS
	g_print ( "%s\n", __FUNCTION__ );
#endif

	static GnoclOption options[] =
	{
		{ NULL, GNOCL_STRING, NULL },
		{ NULL },
	};

	static const char *cmds[] =  { NULL, NULL};

	if ( gnoclGetCmdsAndOpts ( interp, cmds, options, objv, objc ) == TCL_OK )
	{
		return TCL_OK;
	}

	gdk_beep;
	return TCL_OK;
}

/**
\brief     Returns a list of all package object and commands names.
\note
**/
int gnoclCommandsCmd ( ClientData data, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[] )
{

#ifdef DEBUG_COMMANDS
	g_print ( "%s\n", __FUNCTION__ );
#endif

	getCommandsNames ( interp );
	return TCL_OK;
}


/**
\brief      Control redirection input events to window.
\note       Gdk Documentation for the implemented features.
            http://library.gnome.org/devel/gdk/stable/gdk-General.html
**/
int gnoclGrabCmd ( ClientData data, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[] )
{
	static const char *cmd[] = { "pointer", "keyboard", "release", "status", NULL };
	enum optIdx { PointerIdx, KeyboardIdx, ReleaseIdx, StatusIdx };
	int idx;

	if ( objc != 3 )
	{
		Tcl_WrongNumArgs ( interp, 1, objv, "option widgetid " );
		return TCL_ERROR;
	}

	if ( Tcl_GetIndexFromObj ( interp, objv[1], cmd, "option", TCL_EXACT, &idx ) != TCL_OK )
	{
		return TCL_ERROR;
	}

	/*
	    g_print ( "opt = %s  id = %s\n",  Tcl_GetString ( objv[1] ),  Tcl_GetString ( objv[2] ) );
	*/
	switch ( idx )
	{
			/* deny any pointer events, ie. grab all pointer inputs */
		case PointerIdx:
			{
				GtkWidget *widget;
				guint32 activate_time;

				widget = gnoclGetWidgetFromName (  Tcl_GetString ( objv[2] ), interp );

				g_print ( "grab on %s\n", Tcl_GetString ( objv[2] ) );

				gdk_pointer_grab ( GTK_WIDGET ( widget )->window ,
								   TRUE,
								   GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK,
								   NULL, NULL, GDK_CURRENT_TIME );
			}
			break;
			/* deny any keyboard events, ie. grab all keyboard inputs */
		case KeyboardIdx:
			{
				GtkWidget *widget;
				guint32 activate_time;

				widget = gnoclGetWidgetFromName (  Tcl_GetString ( objv[2] ), interp );
				gdk_keyboard_grab ( GTK_WIDGET ( widget )->window , TRUE, GDK_CURRENT_TIME );

			}
			break;
			/* relase all grabs applied to the current display */
		case ReleaseIdx:
			{
				GtkWidget *widget;
				guint32 activate_time;

				widget = gnoclGetWidgetFromName (  Tcl_GetString ( objv[2] ), interp );
				gdk_display_pointer_ungrab ( gdk_drawable_get_display ( widget->window ), GDK_CURRENT_TIME );
				gdk_keyboard_ungrab ( GDK_CURRENT_TIME );
			}

			break;

			/* give status on pointer input grab, no equivalent for the keyboard */
		case StatusIdx:
			{
				char buffer[5];

				sprintf ( buffer, "%s", gdk_pointer_is_grabbed );
				Tcl_SetObjResult ( interp, Tcl_NewStringObj ( buffer, -1 ) );
			}
			break;

	}

	return TCL_OK;
}


/**
\brief      Screen and Display
            size - returns the physical screen size in millimeters
            geometry -returns the logicaL screen size in pixels
**/
int gnoclScreenCmd ( ClientData data, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[] )
{



	/*--------------------------------------*/
	static GnoclOption options[] =
	{
		{ NULL, GNOCL_STRING, NULL },
		{ NULL },
	};

	static const char *cmds[] = { "size", "geometry", "name", "screens", "composited", "windowList", "toplevels", NULL };

	if ( gnoclGetCmdsAndOpts ( interp, cmds, options, objv, objc ) == TCL_OK )
	{
		return TCL_OK;
	}

	/*--------------------------------------*/



	enum optIdx { SizeIdx, GeometryIdx, NameIdx, ScreensIdx, CompositedIdx, WindowListIdx, TopLevelsIdx };
	int idx;


	if ( objc != 2 )
	{
		Tcl_WrongNumArgs ( interp, 1, objv, "option " );
		return TCL_ERROR;
	}

	if ( Tcl_GetIndexFromObj ( interp, objv[1], cmds, "option", TCL_EXACT, &idx ) != TCL_OK )
	{
		return TCL_ERROR;
	}

	/*
	display = gdk_display_get_default ();
	num_screen = gdk_display_get_n_screens (display);
	displayname = gdk_display_get_name (display);

	*/
	switch ( idx )
	{
		case TopLevelsIdx:
		case WindowListIdx:
			{
				gchar buffer[16];
				GdkScreen *screen;
				GSList *toplevels, *p;
				gchar *widgetName;

				screen = gdk_screen_get_default();

				toplevels = gdk_screen_get_toplevel_windows ( screen );

				/* check to see if the windows are registered with gnocl */
				for ( p = toplevels; p != NULL ; p = p->next )
				{

					/* this returns GDKWindow, not Gtk */
					widgetName = gnoclGetNameFromWidget ( GTK_WIDGET ( p->data ) );
					printf ( "toplevel %s\n", widgetName );

					if ( widgetName != NULL )
					{
						printf ( "toplevel %s\n", widgetName );
					}

					else
					{
						printf ( "Missed this one! List length = %d\n", g_slist_length ( toplevels ) );
						printf ( "toplevel %s\n", widgetName );
					}
				}

				gboolean state = 0;

				sprintf ( &buffer, "%d", state );
				Tcl_SetObjResult ( interp, Tcl_NewIntObj ( state ) );
			}
			break;
		case CompositedIdx:
			{
				gchar buffer[16];
				GdkScreen *screen;
				gboolean state;

				screen = gdk_screen_get_default();
				state =  gdk_screen_is_composited ( screen );

				sprintf ( &buffer, "%d", state );
				Tcl_SetObjResult ( interp, Tcl_NewIntObj ( state ) );
			}
			break;
		case ScreensIdx:
			{
				gchar buffer[16];
				GdkDisplay *display;

				display = gdk_display_get_default ();
				gint   screens =  gdk_display_get_n_screens ( display );

				sprintf ( &buffer, "%d", screens );
				Tcl_SetObjResult ( interp, Tcl_NewStringObj ( buffer, -1 ) );
			}
			break;
		case NameIdx:
			{
				gchar buffer[16];
				GdkDisplay *display;
				const gchar *name;

				display  = gdk_display_get_default ();
				name = gdk_display_get_name ( display );

				sprintf ( &buffer, "%s", name );
				Tcl_SetObjResult ( interp, Tcl_NewStringObj ( buffer, -1 ) );
			}
			break;
		case SizeIdx:
			{
				gchar buffer[16];
				gint width_mm = 0;
				gint height_mm = 0;
				GdkScreen *screen;

				screen = gdk_screen_get_default();
				width_mm = gdk_screen_get_width_mm ( screen );
				height_mm = gdk_screen_get_height_mm ( screen );

				sprintf ( &buffer, "%d %d", width_mm, height_mm );
				Tcl_SetObjResult ( interp, Tcl_NewStringObj ( buffer, -1 ) );
			}

			break;
		case GeometryIdx:
			{
				gchar buffer[16];

				gint width = 0;
				gint height = 0;
				GdkScreen *screen;
				screen = gdk_screen_get_default();

				width = gdk_screen_get_width ( screen );
				height = gdk_screen_get_height ( screen );

				sprintf ( &buffer, "%d %d", width, height );

				Tcl_SetObjResult ( interp, Tcl_NewStringObj ( buffer, -1 ) );
			}

			break;
	}

	return TCL_OK;
}

/**
\brief      Return window/widget-related information
**/
int gnoclWinfoCmd ( ClientData data, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[] )
{

	static GnoclOption options[] =
	{
		{ NULL, GNOCL_STRING, NULL },
		{ NULL },
	};

	static const char *cmds[] =
	{
		"path", "parent", "toplevel", "geometry",
		"style", "pointer", "notify", "exists",
		"hasFocus", "isFocus",
		NULL
	};

	if ( gnoclGetCmdsAndOpts ( interp, cmds, options, objv, objc ) == TCL_OK )
	{
		return TCL_OK;
	}


	enum optIdx
	{
		PathIdx, ParentIdx, ToplevelIdx, GeometryIdx,
		StyleIdx, PointerIdx, NotifyIdx, ExistsIdx,
		HasFocusIdx, IsFocusIdx
	};

	int idx;

	if ( objc < 3 )
	{
		Tcl_WrongNumArgs ( interp, 1, objv, "option widgetid " );
		return TCL_ERROR;
	}

	if ( Tcl_GetIndexFromObj ( interp, objv[1], cmds, "option", TCL_EXACT, &idx ) != TCL_OK )
	{
		return TCL_ERROR;
	}


	// g_print ( "opt = %s  id = %s\n",  Tcl_GetString ( objv[1] ),  Tcl_GetString ( objv[2] ) );

	switch ( idx )
	{
		case IsFocusIdx:
			{

				GtkWidget *widget = NULL;
				widget =  gnoclGetWidgetFromName ( Tcl_GetString ( objv[2] ), interp );

				Tcl_SetObjResult ( interp, Tcl_NewIntObj ( gtk_widget_is_focus ( widget ) ) );
			}
			break;

		case HasFocusIdx:
			{

				GtkWidget *widget = NULL;
				widget =  gnoclGetWidgetFromName ( Tcl_GetString ( objv[2] ), interp );

				Tcl_SetObjResult ( interp, Tcl_NewIntObj ( gtk_widget_has_focus ( widget ) ) );
			}
			break;
		case ExistsIdx:
			{

				GtkWidget *widget = NULL;
				widget =  gnoclGetWidgetFromName ( Tcl_GetString ( objv[2] ), interp );

				if ( widget == NULL )
				{
					Tcl_SetObjResult ( interp, Tcl_NewStringObj ( "0", -1 ) );
				}

				else
				{
					Tcl_SetObjResult ( interp, Tcl_NewStringObj ( "1", -1 ) );
				}
			}
			break;
			//gnocl:winfo notify widget-id opts
		case NotifyIdx:
			{
				g_print ( "widget id = %s\n", Tcl_GetString ( objv[2] ) );

				static char *notifyOptions[] =
				{
					"-property", "-script",
					NULL
				};

				static enum notifyIdx
				{
					PropertyIdx, ScriptIdx
				};

				gint i = 3;
				gchar *property;
				gchar *command;
				GtkWidget *widget;

				widget =  gnoclGetWidgetFromName ( Tcl_GetString ( objv[2] ), interp );

				/* parse all the options */
				while ( i <  objc )
				{

					gint nidx;

					getIdx ( notifyOptions, Tcl_GetString ( objv[i] ), &nidx );

					switch ( nidx )
					{
						case PropertyIdx:
							{
								property = Tcl_GetString ( objv[i+1] );
							} break;
						case ScriptIdx:
							{
								command = Tcl_GetString ( objv[i+1] );

							}
							break;
						default: {}
					}

					i += 2;
				}

				return gnoclWinfoNotify ( widget, command );

			}

			break;
		case PathIdx:
			{
				GtkWidget *widget;
				guint path_length;
				char buffer[128];
				gchar *path;
				gchar *path_reversed;

				widget = gnoclGetWidgetFromName (  Tcl_GetString ( objv[2] ), interp );

				if ( objc == 5 )
				{
					if ( strcmp ( Tcl_GetString ( objv[3] ), "-classOnly" ) == 0 && strcmp ( Tcl_GetString ( objv[4] ), "1" ) == 0 )
					{
						/* return a reverse list */
						gtk_widget_class_path ( widget, &path_length, &path, &path_reversed );

					}

					else
					{
						/* ie, -classOnly 0 */
						gtk_widget_path ( widget, &path_length, &path, &path_reversed );
					}
				}

				else
				{
					/* return 'normal' list (default) */
					gtk_widget_path ( widget, &path_length, &path, &path_reversed );
				}

				Tcl_SetObjResult ( interp, Tcl_NewStringObj ( path, -1 ) );
			}
			break;
		case PointerIdx:
			{
				gint x, ox;
				gint y, oy;
				GtkWidget *widget;
				char buffer[128];

				GdkWindow *window_root;
				window_root = gdk_get_default_root_window ();
				gdk_window_get_pointer ( window_root, &ox, &oy, NULL );

				if ( strcmp ( Tcl_GetString ( objv[2] ), "root" ) == 0 )
				{
					sprintf ( buffer, "%d %d", ox, oy );
				}

				else
				{
					widget = gnoclGetWidgetFromName (  Tcl_GetString ( objv[2] ), interp );

					gdk_window_get_pointer ( widget->window, &x, &y, NULL );
					sprintf ( buffer, "%d %d %d %d", x, y, ox, oy );
				}


				Tcl_SetObjResult ( interp, Tcl_NewStringObj ( buffer, -1 ) );

			}
			break;
		case ParentIdx:
			{
				char buffer[128];
				GtkWidget *widget = gnoclGetWidgetFromName (  Tcl_GetString ( objv[2] ), interp );
				GtkWidget *parent = gtk_widget_get_parent ( widget );

				sprintf ( buffer, "%s", gnoclGetNameFromWidget ( parent ) );
				Tcl_SetObjResult ( interp, Tcl_NewStringObj ( buffer, -1 ) );
			}

			break;
		case ToplevelIdx:
			{
				GtkWidget *widget = gnoclGetWidgetFromName (  Tcl_GetString ( objv[2] ), interp );
				GtkWidget *toplevel = gtk_widget_get_toplevel ( widget );
				Tcl_SetObjResult ( interp, Tcl_NewStringObj ( gnoclGetNameFromWidget ( toplevel ), -1 ) );
			}

			break;
			/* probably best leave this to the widget commands themselves */
		case GeometryIdx:
			{
				char buffer[128];
				GtkWidget *widget;
				GtkRequisition req;
				gint x, y, w, h;

				x = 0; y = 0; w = 0; h = 0;

				widget = gnoclGetWidgetFromName (  Tcl_GetString ( objv[2] ), interp );

				gtk_widget_realize ( widget );

				gdk_window_get_origin ( widget->window, &x, &y );

				x += widget->allocation.x;
				x -= widget->style->xthickness;

				y += widget->allocation.y;
				y -= widget->style->ythickness;

				w = widget->allocation.width;
				h = widget->allocation.height;

				sprintf ( buffer, "%d %d %d %d", x, y, w, h );

				Tcl_SetObjResult ( interp, Tcl_NewStringObj ( buffer, -1 ) );
			}

			break;
		case StyleIdx:
			{
				char buffer[128];
				sprintf ( buffer, "%s", "Style" );
				Tcl_SetObjResult ( interp, Tcl_NewStringObj ( buffer, -1 ) );
			}

			break;
	}

	return TCL_OK;
}

/**
\brief
**/
static gint cmp_families ( const void * a, const void * b )
{
	const char *a_name = pango_font_family_get_name ( * ( PangoFontFamily ** ) a );
	const char *b_name = pango_font_family_get_name ( * ( PangoFontFamily ** ) b );

	return g_utf8_collate ( a_name, b_name );
}

/**
\brief Match supplied colour values for a named match.
**/
int gnoclParseColorCmd ( ClientData data, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[] )
{

	if ( objc != 2 )
	{
		Tcl_WrongNumArgs ( interp, 1, objv, "option" );
		return TCL_ERROR;
	}

	GtkSettings *settings = gtk_settings_get_default();

	GParamSpec *pspec;
	GValue *color;
	GString *gstring;

	color = Tcl_GetString ( objv[1] );

	//gboolean gtk_rc_property_parse_color (const GParamSpec *pspec, const GString *gstring, GValue *property_value);

	gtk_rc_property_parse_color ( pspec, gstring, color );

	char buffer[128];
	sprintf ( buffer, "%s", gstring->str );
	Tcl_SetObjResult ( interp, Tcl_NewStringObj ( buffer, -1 ) );

	return TCL_OK;
}

/**
\brief      Retrieve different information.
**/
int gnoclInfoCmd ( ClientData data, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[] )
{

	/*--------------------------------------*/
	static GnoclOption options[] =
	{
		{ NULL, GNOCL_STRING, NULL },
		{ NULL },
	};

	static const char *cmds[] =
	{
		"version", "gtkVersion", "hasGnomeSupport",
		"allStockItems", "breakpoint", "fonts",
		"allIconThemes",
		NULL
	};

	if ( gnoclGetCmdsAndOpts ( interp, cmds, options, objv, objc ) == TCL_OK )
	{
		return TCL_OK;
	}

	/*--------------------------------------*/


	enum optIdx
	{
		VersionIdx, GtkVersionIdx, HasGnomeIdx,
		AllStockItemsIdx, BreakpointIdx, FontsIdx,
		AllIconThemesIdx
	};
	int idx;

	if ( objc != 2 )
	{
		Tcl_WrongNumArgs ( interp, 1, objv, "option" );
		return TCL_ERROR;
	}

	if ( Tcl_GetIndexFromObj ( interp, objv[1], cmds, "option", TCL_EXACT, &idx ) != TCL_OK )
	{
		return TCL_ERROR;
	}

	switch ( idx )
	{
		case FontsIdx:
			{

				Tcl_Obj *res = Tcl_NewListObj ( 0, NULL );

				PangoContext *context;

				context = gdk_pango_context_get();

				PangoFontFamily **families;
				PangoFontFace **faces;
				gint n_families, n_faces, i;

				n_families = 0;
				families = NULL;
				pango_context_list_families ( context, &families, &n_families );

				qsort ( families, n_families, sizeof ( PangoFontFamily * ), cmp_families );

				for ( idx = 0;  idx < n_families;  idx++ )
				{
					char * name_tmp =  pango_font_family_get_name ( families[idx] );
					//g_print ( "font %d name = %s\n", idx, name_tmp );
					/* add the item to the list */
					Tcl_ListObjAppendElement ( interp, res, Tcl_NewStringObj ( name_tmp, -1 ) );
				}

				g_free ( families );

				//g_print ( "Return a list of available fonts on the system: total = %d\n", n_families );

				Tcl_SetObjResult ( interp, res );
			}
			break;
		case VersionIdx:
			{
				Tcl_SetObjResult ( interp, Tcl_NewStringObj ( VERSION, -1 ) );
			}
			break;
		case GtkVersionIdx:
			{
				char buffer[128];
				sprintf ( buffer, "%d.%d.%d", gtk_major_version,
						  gtk_minor_version, gtk_micro_version );
				Tcl_SetObjResult ( interp, Tcl_NewStringObj ( buffer, -1 ) );
			}

			break;
		case HasGnomeIdx:
			Tcl_SetObjResult ( interp, Tcl_NewBooleanObj (
#ifdef GNOCL_USE_GNOME
								   1
#else
								   0
#endif
							   ) );
			break;
		case AllStockItemsIdx:
		case AllIconThemesIdx:
			{
				GSList *p;
				GSList *ids;
				GtkIconTheme *icon_theme;
				Tcl_Obj *res = Tcl_NewListObj ( 0, NULL );

				if ( idx == AllIconThemesIdx )
				{
					icon_theme = gtk_icon_theme_get_default();

					ids = gtk_icon_theme_list_icons ( icon_theme, NULL );
				}
				else
				{
					ids = gtk_stock_list_ids();
				}

				for ( p = ids; p != NULL; p = p->next )
				{
					char *txt = p->data;
					int skip = 0;
					/* FIXME: gtk-missing-image, gtk-dnd-multiple and gtk-dnd
					          fail lookup, why?
					{
					   GtkStockItem sp;
					   printf( "%s lookup: %d\n", txt,
					         gtk_stock_lookup( txt, &sp ) );
					}
					**/

					/* see createStockName and gnoclGetStockName */

					if ( strncmp ( txt, "gtk", 3 ) == 0 )
						skip = 3;

#ifdef GNOCL_USE_GNOME
					else if ( strncmp ( txt, "gnome-stock", 11 ) == 0 )
						skip = 11;

#endif

					if ( skip > 0 )
					{
						GString *name = g_string_new ( NULL );
						char *tp = txt + skip;

						for ( ; *tp; ++tp )
						{
							if ( *tp == '-' )
							{
								++tp;
								g_string_append_c ( name, toupper ( *tp ) );
							}

							else
								g_string_append_c ( name, *tp );
						}

						Tcl_ListObjAppendElement ( interp, res,

												   Tcl_NewStringObj ( name->str, -1 ) );
						/* printf( "%s -> %s\n", (char *)p->data, name->str ); */
						g_string_free ( name, 1 );
					}

					else
						Tcl_ListObjAppendElement ( interp, res, Tcl_NewStringObj ( txt, -1 ) );

					g_free ( p->data );
				}

				g_slist_free ( ids );

				Tcl_SetObjResult ( interp, res );
			}

			break;
		case BreakpointIdx:
			/* this is only for debugging */
			G_BREAKPOINT();
			break;
	}

	return TCL_OK;
}

/**
\brief
**/
int gnoclUpdateCmd ( ClientData data, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[] )
{


	static GnoclOption options[] =
	{
		{ NULL, GNOCL_STRING, NULL },
		{ NULL },
	};

	static const char *cmds[] =  { NULL, NULL};

	if ( gnoclGetCmdsAndOpts ( interp, cmds, options, objv, objc ) == TCL_OK )
	{
		return TCL_OK;
	}

	int nMax = 500;
	int n;

	if ( objc != 1 )
	{
		Tcl_WrongNumArgs ( interp, 1, objv, NULL );
		return TCL_ERROR;
	}

	for ( n = 0; n < nMax && gtk_events_pending(); ++n )
	{
		gtk_main_iteration_do ( 0 );
	}

	Tcl_SetObjResult ( interp, Tcl_NewIntObj ( n ) );

	return TCL_OK;
}

/**
\brief	Create and manipulate system stock items.

typedef struct {
  gchar *stock_id;
  gchar *label;
  GdkModifierType modifier;
  guint keyval;
  gchar *translation_domain;
} GtkStockItem;

{ GEANY_STOCK_SAVE_ALL, _("Save All"), 0, 0, GETTEXT_PACKAGE },


-stockId
-label
-modifier
-key
-domain
-file
-pixbuf

syntax:
[0]              [1] [2...]
gnocl::stockItem add <option val ...>

**/


/**
\brief	Create stock item from image file.
\note	This code could possibly benefit from some tidying up.
**/
static int createStockItem ( Tcl_Interp * interp, GnoclStringType type, gchar * stockName, gchar * label, gchar * icon )
{
#if 1
	g_print ( "%s\n", __FUNCTION__ );
#endif

	GtkIconFactory *factory;
	GtkIconSet *iconset;
	GtkIconSource *source;
	GdkPixbuf *pixbuf;

	PixbufParams *para;

	GtkStockItem stock_items[] =
	{
		{stockName, label, 0, 0, "gnocl"}
	};

	/* add new stock items */
	gtk_stock_add ( stock_items, G_N_ELEMENTS ( stock_items ) );

	/* init icon factory & set source */
	factory = gtk_icon_factory_new();
	gtk_icon_factory_add_default ( factory );
	source = gtk_icon_source_new();
	iconset = gtk_icon_set_new();

	/* create icon from graphic sources */
	switch ( type )
	{
		case GNOCL_STR_STR:
		case GNOCL_STR_FILE:
			{
				/* create stock icon from file */
				gtk_icon_source_set_filename ( source, icon );
			}
			break;
		case GNOCL_STR_BUFFER:
			{
				/* create stock icon from pixbuf */
				para = gnoclGetPixBufFromName ( icon, interp );
				iconset = gtk_icon_set_new_from_pixbuf ( para->pixbuf );
			}
			break;
		default: {}
	}

	/* assign and clean up */
	gtk_icon_set_add_source ( iconset, source );
	gtk_icon_source_free ( source );
	gtk_icon_factory_add ( factory, stockName, iconset );
	gtk_icon_set_unref ( iconset );

}

/**
\brief	Add new items to the list of available stock icons.
**/
int gnoclStockItemCmd ( ClientData data, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[] )
{

	static GnoclOption options[] =
	{
		{ "-label", GNOCL_STRING, NULL },
		{ "-icon", GNOCL_STRING, NULL },
		{ NULL },
	};

	static const char *cmds[] =  { NULL, NULL};

	if ( gnoclGetCmdsAndOpts ( interp, cmds, options, objv, objc ) == TCL_OK )
	{
		return TCL_OK;
	}

#if 1
	listParameters ( objc, objv, "gnoclStockItemCmd" );
#endif

	gchar *label;
	gchar *icon;
	GString *gtkName;
	GnoclStringType type;

	label = NULL;
	icon = NULL;

	static const char *opts[] =
	{
		"-label", "-icon",
		NULL
	};

	enum optIdx
	{
		LabelIdx, IconIdx
	};

	int idx;
	int i;

	i = 2;

	/* parse and set all options */
	while ( i <  objc )
	{

		getIdx ( opts, Tcl_GetString ( objv[i] ), &idx );

		switch ( idx )
		{

			case LabelIdx:
				{
					/* the stock id can be constructed from the label */
					label = Tcl_GetString ( objv[i+1] ); // OK
					gtkName = createStockName ( "gtk", objv[i+1] );

					g_print ( "name = %s %s\n", Tcl_GetString ( objv[i+1] ), gtkName );

				}
				break;
			case IconIdx:
				{
					icon = Tcl_GetString ( objv[i+1] ) ;

					type = gnoclGetStringType ( objv[i+1] );


					switch ( type )
					{
						case GNOCL_STR_STR:
						case GNOCL_STR_FILE:
							{
								g_print ( "file\n" );
								icon = gnoclGetString ( objv[i+1] );

							} break;
						case GNOCL_STR_BUFFER:
							{
								g_print ( "buffer\n" );
								icon = gnoclGetString ( objv[i+1] );
							} break;

						default:
							{
								gchar str[128];
								sprintf ( str, "Invalid Markup String: %s.\n", icon );
								Tcl_SetResult ( interp, str, TCL_STATIC );
								return TCL_ERROR;
							};
					}

				}
				break;
			default: {}
		}

		i += 2;
	}

	createStockItem ( interp, type, gtkName->str, label, icon );

	return TCL_OK;
}


/**
\brief 	Set style for specified widget.
	usage: gnocl::setStyle class style value
	notes: must be declared before widget shown
	Added pseudo-styles for GtkTextWidget, "rollover-fg" and "rollover-bg".
**/

int gnoclSetStyleCmd ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{
	/*--------------------------------------*/
	static GnoclOption options[] =
	{
		{ NULL, GNOCL_STRING, NULL },
		{ NULL },
	};

	static const char *cmds[] =  { NULL, NULL};

	if ( gnoclGetCmdsAndOpts ( interp, cmds, options, objv, objc ) == TCL_OK )
	{
		return TCL_OK;
	}

	/*--------------------------------------*/

	char *id = Tcl_GetString ( objv[1] );
	char *style = Tcl_GetString ( objv[2] );
	char *val = Tcl_GetString ( objv[3] );

	if ( strcmp ( id, "GtkTextView" ) == 0 )
	{
		extern rollOverTagFgClr;
		extern rollOverTagBgClr;
		extern rollOverMode;

		if ( strcmp ( style, "rollover-mode" ) == 0 )
		{
			if ( strcmp ( val, "none" ) == 0 )
			{
				rollOverMode = GNOCL_ROLLOVER_NONE;
			}

			if ( strcmp ( val, "foreground" ) == 0  || strcmp ( val, "fg" ) == 0 )
			{
				rollOverMode = GNOCL_ROLLOVER_FG;
			}

			if ( strcmp ( val, "background" ) == 0  || strcmp ( val, "bg" ) == 0 )
			{
				rollOverMode = GNOCL_ROLLOVER_BG;
			}

			if ( strcmp ( val, "none" ) == 0 )
			{
				rollOverMode = GNOCL_ROLLOVER_BOTH;
			}

		}


		if ( strcmp ( style, "rollover-fg" ) == 0 )
		{
			gdk_color_parse ( val, &rollOverTagFgClr );
			return TCL_OK;
		}

		if ( strcmp ( style, "rollover-bg" ) == 0 )
		{
			gdk_color_parse ( val, &rollOverTagBgClr );
			return TCL_OK;
		}
	}


	GtkWidget *widget = gnoclGetWidgetFromName ( id, interp ) ;

	char str[512];

	//sprintf ( str, "style \"%s_style\"\n{\n%s = \"%s\"\n}\nwidget \"*.%s\" style \"%s_style\"" , id, style, val, id, id );

	sprintf ( str,
			  "style \"myStyle\" {\n \
	%s::%s = %s\n}\n \
	class \"%s\" style \"myStyle\"\n",
			  id, style, val, id );

	gtk_rc_parse_string ( str );

	return TCL_OK;

}


/**
\brief 	Get style for specified widget. Use application main window.
\note	This command might prove useful for some form of automated widget state dump.
**/

/*
typedef struct {
  GdkColor fg[5];
  GdkColor bg[5];
  GdkColor light[5];
  GdkColor dark[5];
  GdkColor mid[5];
  GdkColor text[5];
  GdkColor base[5];
  GdkColor text_aa[5];		// Halfway between text/base

  GdkColor black;
  GdkColor white;
  PangoFontDescription *font_desc;

  gint xthickness;
  gint ythickness;

  GdkGC *fg_gc[5];
  GdkGC *bg_gc[5];
  GdkGC *light_gc[5];
  GdkGC *dark_gc[5];
  GdkGC *mid_gc[5];
  GdkGC *text_gc[5];
  GdkGC *base_gc[5];
  GdkGC *text_aa_gc[5];
  GdkGC *black_gc;
  GdkGC *white_gc;

  GdkPixmap *bg_pixmap[5];
} GtkStyle;
*/

int gnoclGetStyleCmd ( ClientData data, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[] )
{
#ifdef DEBUG_COMMANDS
	g_print ( "gnoclGetStyleCmd\n" ) ;
#endif

	GtkStyle *style;
	GtkWidget *widget;
	gchar buffer[512];
	gint idx;

	static const char *cmd[] =
	{
		"foreground", "background",
		"light", "dark",
		"mid", "text",
		"base", "text_aa",
		"xthickness", "ythickness",
		NULL
	};

	enum
	{
		ForegroundIdx, BackgroundIdx,
		LightIdx, DarkIdx,
		MidIdx, TextIdx,
		BaseIdx, Text_aaIdx,
		XthicknessIdx, YthicknessIdx
	};

	/* simply return a list of supported style options */
	if ( objc == 1 )
	{
		Tcl_SetObjResult ( interp, Tcl_NewStringObj ( cmds2list  ( cmd ), -1 ) );
		return TCL_OK;
	}

	/* obtain specific values */
	if ( objc != 3 )
	{
		Tcl_WrongNumArgs ( interp, 1, objv, NULL );
		return TCL_ERROR;
	}

	char *id = Tcl_GetString ( objv[2] );


	if ( Tcl_GetIndexFromObj ( interp, objv[1], cmd, "option", TCL_EXACT, &idx ) != TCL_OK )
	{
		return TCL_ERROR;
	}


	widget = gnoclGetWidgetFromName ( id, interp ) ;

	style = gtk_widget_get_style ( widget );


	switch ( idx )
	{
		case ForegroundIdx:
			{
				sprintf ( buffer, "%d %d %d", style->fg->red, style->fg->green, style->fg->blue );
			}
			break;
		case BackgroundIdx:
			{
				sprintf ( buffer, "%d %d %d", style->bg->red, style->bg->green, style->bg->blue );
			}
			break;
		case LightIdx:
			{
				sprintf ( buffer, "%d %d %d", style->light->red, style->light->green, style->light->blue );
			}
			break;
		case DarkIdx:
			{
				sprintf ( buffer, "%d %d %d", style->dark->red, style->dark->green, style->dark->blue );
			}
			break;
		case MidIdx:
			{
				sprintf ( buffer, "%d %d %d", style->mid->red, style->mid->green, style->mid->blue );
			}
			break;
		case TextIdx:
			{
				sprintf ( buffer, "%d %d %d", style->text->red, style->text->green, style->text->blue );
			}
			break;
		case BaseIdx:
			{
				sprintf ( buffer, "%d %d %d", style->base->red, style->base->green, style->base->blue );
			}
			break;
		case Text_aaIdx:
			{
				sprintf ( buffer, "%d %d %d", style->text_aa->red, style->text_aa->green, style->text_aa->blue );
			}
			break;
		case XthicknessIdx:
			{
				sprintf ( buffer, "%d", style->xthickness );
			}
			break;
		case YthicknessIdx:
			{
				sprintf ( buffer, "%d", style->ythickness );
			}
			break;
	}

	Tcl_SetObjResult ( interp, Tcl_NewStringObj ( buffer, -1 ) );


	return TCL_OK;
}

/**
\brief	Load and configure widget styles using a resource (.rsc) file.
**/
int gnoclResourceFileCmd ( ClientData data, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[] )
{
	/*--------------------------------------*/
	static GnoclOption options[] =
	{
		{ NULL, GNOCL_STRING, NULL },
		{ NULL },
	};

	static const char *cmds[] =  { NULL, NULL};

	if ( gnoclGetCmdsAndOpts ( interp, cmds, options, objv, objc ) == TCL_OK )
	{
		return TCL_OK;
	}

	/*--------------------------------------*/

	int nMax = 500;
	int n;

	if ( objc != 2 )
	{
		Tcl_WrongNumArgs ( interp, 1, objv, NULL );
		return TCL_ERROR;
	}

	char * str = Tcl_GetString ( objv[1] );

	gtk_rc_parse ( str );

	Tcl_SetObjResult ( interp, Tcl_NewIntObj ( n ) );

	return TCL_OK;
}

/**
\break
**/
int gnoclConfigureCmd (	ClientData data, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[] )
{



	/*--------------------------------------*/
	GnoclOption options[] =
	{
		{ "-tooltip", GNOCL_BOOL, NULL },
		{ "-defaultIcon", GNOCL_OBJ, NULL },
		{ NULL }
	};

	static const char *cmds[] =  { NULL, NULL};

	if ( gnoclGetCmdsAndOpts ( interp, cmds, options, objv, objc ) == TCL_OK )
	{
		return TCL_OK;
	}

	/*--------------------------------------*/


	if ( gnoclGetCmdsAndOpts ( interp, NULL, options, objv, objc ) == TCL_OK )
	{
		return TCL_OK;
	}

	const int tooltipIdx     = 0;

	const int defaultIconIdx = 1;

	int ret = TCL_ERROR;

	if ( gnoclParseOptions ( interp, objc, objv, options ) != TCL_OK )
		goto cleanExit;

	if ( options[defaultIconIdx].status == GNOCL_STATUS_CHANGED )
	{
		GnoclStringType type = gnoclGetStringType ( options[defaultIconIdx].val.obj );

		switch ( type )
		{
			case GNOCL_STR_EMPTY:
				gtk_window_set_default_icon_list ( NULL );
				break;
			case GNOCL_STR_FILE:
				{
					GdkPixbuf *pix = gnoclPixbufFromObj ( interp, options + defaultIconIdx );
					GList *list = NULL;

					if ( pix == NULL )
						goto cleanExit;

					list = g_list_append ( list, pix );

					gtk_window_set_default_icon_list ( list );

					g_list_free ( list );
				}

				break;
			default:
				Tcl_AppendResult ( interp, "Unknown type for \"",
								   Tcl_GetString ( options[defaultIconIdx].val.obj ),
								   "\" must be of type FILE (%/) or empty", NULL );
				goto cleanExit;
		}
	}

	if ( options[tooltipIdx].status == GNOCL_STATUS_CHANGED )
	{
		if ( options[tooltipIdx].val.b )
		{
			gtk_tooltips_enable ( gnoclGetTooltips() );
		}

		else
		{
			gtk_tooltips_disable ( gnoclGetTooltips() );
		}
	}

	ret = TCL_OK;

cleanExit:
	gnoclClearOptions ( options );
	return ret;
}

/**
\author     Peter G. Baum
**/
int gnoclClipboardCmd ( ClientData data, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[] )
{
	GnoclOption options[] =
	{
		{ "-primary", GNOCL_BOOL, NULL },
		{ NULL }
	};
	static const char *cmds[] = { "hasText", "setText", "getText", "clear", NULL };

	if ( gnoclGetCmdsAndOpts ( interp, cmds, options, objv, objc ) == TCL_OK )
	{
		return TCL_OK;
	}

	const int usePrimaryIdx = 0;

	enum optIdx { HasTextIdx, SetTextIdx, GetTextIdx, ClearIdx };

	int idx;

	int optNum;

	GtkClipboard *clip;

	int usePrimary = 0;

	if ( objc < 2 )
	{
		Tcl_WrongNumArgs ( interp, 1, objv, "option" );
		return TCL_ERROR;
	}

	if ( Tcl_GetIndexFromObj ( interp, objv[1], cmds, "option", TCL_EXACT, &idx ) != TCL_OK )
	{
		return TCL_ERROR;
	}

	if ( idx == SetTextIdx )
	{
		optNum = 2;

		if ( objc < 3 )
		{
			Tcl_WrongNumArgs ( interp, 1, objv, "text ?option value?" );
			return TCL_ERROR;
		}
	}

	else
	{
		optNum = 1;

		if ( objc < 2 )
		{
			Tcl_WrongNumArgs ( interp, 1, objv, NULL );
			return TCL_ERROR;
		}
	}

	if ( gnoclParseOptions ( interp, objc - optNum, objv + optNum, options ) != TCL_OK )
	{
		return TCL_ERROR;
	}

	if ( options[usePrimaryIdx].status == GNOCL_STATUS_CHANGED )
	{
		usePrimary = options[usePrimaryIdx].val.b;
	}

	clip = gtk_clipboard_get ( usePrimary ? gdk_atom_intern ( "PRIMARY", 1 )  : GDK_NONE );

	switch ( idx )
	{
		case HasTextIdx:
			{
				int ret = gtk_clipboard_wait_is_text_available ( clip );
				Tcl_SetObjResult ( interp, Tcl_NewBooleanObj ( ret ) );
			}

			break;
		case SetTextIdx:
			{
				gtk_clipboard_set_text ( clip, Tcl_GetString ( objv[2] ), -1 );
			}
			break;
		case GetTextIdx:
			{
				char *txt = gtk_clipboard_wait_for_text ( clip );

				if ( txt )
				{
					Tcl_SetObjResult ( interp, Tcl_NewStringObj ( txt, -1 ) );
					g_free ( txt );
				}

				/* FIXME? else error? */
			}

			break;
		case ClearIdx:
			gtk_clipboard_clear ( clip );
			break;
	}

	return TCL_OK;
}


