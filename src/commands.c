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
#include <string.h>
#include <ctype.h>

/**
\brief      Manipulate the screen pointer.

static int configure ( Tcl_Interp *interp, GtkImage *image, GnoclOption options[] )
int gnoclGetStockItem ( Tcl_Obj *obj, Tcl_Interp *interp, GtkStockItem *sp )

**/
int gnoclPointerCmd ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{

	static const char *cmd[] = { "warp", "set", NULL };
	enum optIdx { WarpIdx, SetIdx };
	int idx1;

	if ( objc <= 3 )
	{
		Tcl_WrongNumArgs ( interp, 1, objv, "option value " );
		return TCL_ERROR;
	}

	if ( Tcl_GetIndexFromObj ( interp, objv[1], cmd, "option", TCL_EXACT, &idx1 ) != TCL_OK )
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

	if ( objc != 3 )
	{
		Tcl_WrongNumArgs ( interp, 1, objv, "widget signal-type" );
		return TCL_ERROR;
	}

	GtkWidget *widget = gnoclGetWidgetFromName (  Tcl_GetString ( objv[1] ), interp );

	g_signal_stop_emission_by_name ( widget , Tcl_GetString ( objv[2] ) );
	return TCL_OK;
}

/**
\brief      Perform an effective break, let the script decide what to do next.
**/
int gnoclSignalEmitCmd ( ClientData data, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[] )
{

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
\brief      Emits a short beep.
\note       Gdk Documentation for the implemented features.
            http://library.gnome.org/devel/gdk/stable/gdk-General.html
**/
int gnoclBeepCmd ( ClientData data, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[] )
{

#ifdef DEBUG_COMMANDS
	g_print ( "gnoclBeepCmd\n" );
#endif

	gdk_beep;
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

				gdk_pointer_grab ( widget->window ,
								   TRUE,
								   GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK,
								   NULL, NULL, GDK_CURRENT_TIME );
				break;
			}

			/* deny any keyboard events, ie. grab all keyboard inputs */
		case KeyboardIdx:
			{
				GtkWidget *widget;
				guint32 activate_time;

				widget = gnoclGetWidgetFromName (  Tcl_GetString ( objv[2] ), interp );
				gdk_keyboard_grab ( widget->window , TRUE, GDK_CURRENT_TIME );
				break;
			}

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
	static const char *cmd[] = { "size", "geometry", "name", "screens", "composited", "windowList", "toplevels", NULL };
	enum optIdx { SizeIdx, GeometryIdx, NameIdx, ScreensIdx, CompositedIdx, WindowListIdx, TopLevelsIdx };
	int idx;


	if ( objc != 2 )
	{
		Tcl_WrongNumArgs ( interp, 1, objv, "option " );
		return TCL_ERROR;
	}

	if ( Tcl_GetIndexFromObj ( interp, objv[1], cmd, "option", TCL_EXACT, &idx ) != TCL_OK )
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
	static const char *cmd[] =
	{
		"path", "parent", "toplevel",
		"geometry", "style", "pointer",
		"notify",
		NULL
	};

	enum optIdx
	{
		PathIdx, ParentIdx, ToplevelIdx,
		GeometryIdx, StyleIdx, PointerIdx,
		NotifyIdx
	};
	int idx;


	if ( objc < 3 )
	{
		Tcl_WrongNumArgs ( interp, 1, objv, "option widgetid " );
		return TCL_ERROR;
	}

	if ( Tcl_GetIndexFromObj ( interp, objv[1], cmd, "option", TCL_EXACT, &idx ) != TCL_OK )
	{
		return TCL_ERROR;
	}


	// g_print ( "opt = %s  id = %s\n",  Tcl_GetString ( objv[1] ),  Tcl_GetString ( objv[2] ) );

	switch ( idx )
	{
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

				widget = gnoclGetWidgetFromName (  Tcl_GetString ( objv[2] ), interp );

				//gtk_widget_realize(widget);

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
	static const char *cmd[] =
	{
		"version", "gtkVersion", "hasGnomeSupport",
		"allStockItems", "breakpoint", "fonts", NULL
	};
	enum optIdx
	{
		VersionIdx, GtkVersionIdx, HasGnomeIdx,
		AllStockItems, BreakpointIdx, FontsIdx
	};
	int idx;

	if ( objc != 2 )
	{
		Tcl_WrongNumArgs ( interp, 1, objv, "option" );
		return TCL_ERROR;
	}

	if ( Tcl_GetIndexFromObj ( interp, objv[1], cmd, "option", TCL_EXACT, &idx ) != TCL_OK )
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
			Tcl_SetObjResult ( interp, Tcl_NewStringObj ( VERSION, -1 ) );
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
		case AllStockItems:
			{
				Tcl_Obj *res = Tcl_NewListObj ( 0, NULL );
				GSList *ids = gtk_stock_list_ids();
				GSList *p;

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
						Tcl_ListObjAppendElement ( interp, res,
												   Tcl_NewStringObj ( txt, -1 ) );

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

	GtkIconFactory *factory;
	GtkIconSet *iconset;
	GtkIconSource *source;
	GdkPixbuf *pixbuf;

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
				pixbuf = gnoclGetPixBufFromName ( icon, interp );
				iconset = gtk_icon_set_new_from_pixbuf ( pixbuf );
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
\brief
**/
int gnoclStockItemCmd ( ClientData data, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[] )
{

#ifdef DEBUG_COMMANDS
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
					label = Tcl_GetString ( objv[i+1] );
					gtkName = createStockName ( "gtk", objv[i+1] );

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
								//g_print ( "file\n" );
								icon = gnoclGetString ( objv[i+1] );

							} break;
						case GNOCL_STR_BUFFER:
							{
								//g_print ( "buffer\n" );
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
		"foreground", "background", "light",
		"dark", "mid", "text",
		"base", "text_aa",
		"xthickness", "ythickness",
		NULL
	};

	enum
	{
		ForegroundIdx, BackgroundIdx, LightIdx,
		DarkIdx, MidIdx, TextIdx,
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
int gnoclResourceFileCmd (
	ClientData data,
	Tcl_Interp * interp,
	int objc,
	Tcl_Obj * const objv[] )
{

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
	GnoclOption options[] =
	{
		{ "-tooltip", GNOCL_BOOL, NULL },
		{ "-defaultIcon", GNOCL_OBJ, NULL },
		{ NULL }
	};
	const int tooltipIdx     = 0;
	const int defaultIconIdx = 1;

	int ret = TCL_ERROR;

	if ( gnoclParseOptions ( interp, objc, objv, options ) != TCL_OK )
		goto cleanExit;

	if ( options[defaultIconIdx].status == GNOCL_STATUS_CHANGED )
	{
		GnoclStringType type = gnoclGetStringType (
								   options[defaultIconIdx].val.obj );

		switch ( type )
		{
			case GNOCL_STR_EMPTY:
				gtk_window_set_default_icon_list ( NULL );
				break;
			case GNOCL_STR_FILE:
				{
					GdkPixbuf *pix = gnoclPixbufFromObj ( interp,
														  options + defaultIconIdx );
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
			gtk_tooltips_enable ( gnoclGetTooltips() );
		else
			gtk_tooltips_disable ( gnoclGetTooltips() );
	}

	ret = TCL_OK;

cleanExit:
	gnoclClearOptions ( options );
	return ret;
}

/**
\author     Peter G. Baum
**/
int gnoclClipboardCmd (
	ClientData data,
	Tcl_Interp * interp,
	int objc,
	Tcl_Obj * const objv[] )
{
	GnoclOption options[] =
	{
		{ "-primary", GNOCL_BOOL, NULL },
		{ NULL }
	};
	const int usePrimaryIdx = 0;

	static const char *cmd[] = { "hasText", "setText", "getText", "clear",
								 NULL
							   };
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

	if ( Tcl_GetIndexFromObj ( interp, objv[1], cmd, "option", TCL_EXACT, &idx ) != TCL_OK )
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
			gtk_clipboard_set_text ( clip, Tcl_GetString ( objv[2] ), -1 );
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
