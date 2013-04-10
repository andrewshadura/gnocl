/*
 * $Id: parseOptions.c,v 1.34 2005/01/01 15:27:54 baum Exp $
 *
 * This file implements tcl parsing and setting
 *
 * Copyright (c) 2001 - 2005 Peter G. Baum  http://www.dr-baum.net
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

/*
   History:
   2013-11: renamed doOnButtonClicked to doOnToolButtonClicked
   2011-11: -onIconPress %t now returns entry text content. Added %b to review mouse button info.
   2009-12: added %g to those callback with %w substitutions, returns 'glade name'
        08: added padding big, small, default
        03: don't use Tcl_GetIndexFromObjStruct any more, since it is
            broken for cached values in 8.4
   2003-02: new functions for dndTarget, onDropData and onDragData
        11: new gnoclOptGeneric: this removes redundancies
   2002-07-01: Begin of developement
*/

#include "gnocl.h"
#include "gnoclparams.h"

/* global used to store a pointer to the text/entry popupmenu */
GtkMenu *popupMenu;

/**
\brief	Replaces default label with custom label widget.
**/
int gnoclOptToolButtonLabelWidget ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	GtkToolButton *button;
	GtkWidget *label_widget;
	const char *txt;

	button = obj;
	txt = Tcl_GetString ( opt->val.obj );

#ifdef DEBUG_PARSEOPTIONS
	g_print ( "%s = %s\n", __FUNCTION__, txt );
#endif

	label_widget = gnoclGetWidgetFromName ( txt, interp );
	//gtk_frame_set_label_widget ( frame, label_widget );

	gtk_tool_button_set_label_widget ( button, label_widget );

	return TCL_OK;
}

/**
\brief	Replaces default icon custom icon widget.
**/
int gnoclOptToolButtonIconWidget ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	GtkToolButton *button;
	GtkWidget *icon_widget;
	const char *txt;

	button = obj;
	txt = Tcl_GetString ( opt->val.obj );

#ifdef DEBUG_PARSEOPTIONS
	g_print ( "%s = %s\n", __FUNCTION__, txt );
#endif

	icon_widget = gnoclGetWidgetFromName ( txt, interp );

	gtk_tool_button_set_icon_widget ( button, icon_widget );

	return TCL_OK;
}

/**
\brief	Replaces default label with custom label widget.
**/
int gnoclOptFrameLabelWidget ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	GtkFrame *frame;
	GtkWidget *label_widget;
	const char *txt;

	frame = obj;
	txt = Tcl_GetString ( opt->val.obj );

#ifdef DEBUG_PARSEOPTIONS
	g_print ( "%s = %s\n", __FUNCTION__, txt );
#endif

	label_widget = gnoclGetWidgetFromName ( txt, interp );
	gtk_frame_set_label_widget ( frame, label_widget );

	return TCL_OK;
}

/**
\brief      Utility function to determine which events types have occured.
**/
static int _eventType_ ( int type )
{
	char *str;

	switch ( type )
	{
		case GDK_NOTHING:
			str = "nothing"; break;
		case GDK_DELETE:
			str = "delete_event"; break;
		case GDK_DESTROY:
			str = "destroy_event"; break;
		case GDK_EXPOSE:
			str = "expose_event"; break;
		case GDK_MOTION_NOTIFY:
			str = "motion_notify_event"; break;
		case GDK_BUTTON_PRESS:
			str = "button_press_event"; break;
		case GDK_2BUTTON_PRESS:
			str = "button2press"; break;
		case GDK_3BUTTON_PRESS:
			str = "button3press"; break;
		case GDK_BUTTON_RELEASE:
			str = "button_release_event"; break;
		case GDK_KEY_PRESS:
			str = "key_press_event"; break;
		case GDK_KEY_RELEASE:
			str = "key_release_event"; break;
		case GDK_ENTER_NOTIFY:
			str = "enter_notify_event"; break;
		case GDK_LEAVE_NOTIFY:
			str = "leave_notify_event"; break;
		case GDK_FOCUS_CHANGE:
			str = "focus_event"; break;
		case GDK_CONFIGURE:
			str = "configure_event"; break;
		case GDK_MAP:
			str = "map_event"; break;
		case GDK_UNMAP:
			str = "unmap_event"; break;
		case GDK_PROPERTY_NOTIFY:
			str = "property_notify_event"; break;
		case GDK_SELECTION_CLEAR:
			str = "selection_clear_event"; break;
		case GDK_SELECTION_REQUEST:
			str = "selection_request_event"; break;
		case GDK_SELECTION_NOTIFY:
			str = "selection_notify_event"; break;
		case GDK_PROXIMITY_IN:
			str = "proximity_in_event"; break;
		case GDK_PROXIMITY_OUT:
			str = "proximity_out_event"; break;
		case GDK_DRAG_ENTER:
			str = "drag_enter"; break;
		case GDK_DRAG_LEAVE:
			str = "drag_leave"; break;
		case GDK_DRAG_MOTION:
			str = "drag_motion"; break;
		case GDK_DRAG_STATUS:
			str = "drag_status"; break;
		case GDK_DROP_START:
			str = "drop_start"; break;
		case GDK_DROP_FINISHED:
			str = "drop-finished"; break;
		case GDK_CLIENT_EVENT:
			str = "client_event"; break;
		case GDK_VISIBILITY_NOTIFY:
			str = "visibility_notify_event"; break;
		case GDK_NO_EXPOSE:
			str = "no_expose_event"; break;
		case GDK_SCROLL:
			str = "scroll_event"; break;
		case GDK_WINDOW_STATE:
			str = "window_state_event"; break;
		case GDK_SETTING:
			str = "setting"; break;
		case GDK_OWNER_CHANGE:
			str = "owner_change"; break;
		case GDK_GRAB_BROKEN:
			str = "grab_broken_event"; break;
		default:
			str = "unknown";  break;
	}

#ifdef DEBUG_PARSEOPTIONS
	g_print ( "Event Type = %s %d\n", str, type );
#endif
}


static GHashTable *sizeGroupTables[3] = { NULL, NULL, NULL };

/**
\brief	This is a copy from Tcl_GetIndexFromObjStruct, but without cached values, since this is broken in Tcl
\note  	This function is copyrighted by the Regents of the University of
    	California, Sun Microsystems, Inc., Scriptics Corporation, ActiveState
    	Corporation and other parties.
**/
/* this is a copy from Tcl_GetIndexFromObjStruct, but without
   cached values, since this is broken in Tcl

   This function is copyrighted by the Regents of the University of
   California, Sun Microsystems, Inc., Scriptics Corporation, ActiveState
   Corporation and other parties.
*/
int gnoclGetIndexFromObjStruct ( Tcl_Interp *interp, Tcl_Obj *objPtr, char **tablePtr, int offset, char *msg, int flags, int *indexPtr )
#if 0
Tcl_Interp *interp;     /* Used for error reporting if not NULL. */
Tcl_Obj *objPtr;        /* Object containing the string to lookup. */
char **tablePtr;        /* The first string in the table. The second
                             * string will be at this address plus the
                             * offset, the third plus the offset again,
                             * etc. The last entry must be NULL
                             * and there must not be duplicate entries. */
int offset;             /* The number of bytes between entries */
char *msg;              /* Identifying word to use in error messages. */
int flags;              /* 0 or TCL_EXACT */
int *indexPtr;          /* Place to store resulting integer index. */
#endif
{
	int index, length, i, numAbbrev;
	char *key, *p1, *p2, **entryPtr;
	Tcl_Obj *resultPtr;

	/*
	 * Lookup the value of the object in the table.  Accept unique
	 * abbreviations unless TCL_EXACT is set in flags.
	 */

	key = Tcl_GetStringFromObj ( objPtr, &length );
	index = -1;
	numAbbrev = 0;

	/*
	 * The key should not be empty, otherwise it's not a match.
	 */

	if ( key[0] == '\0' )
	{
		goto error;
	}

	for ( entryPtr = tablePtr, i = 0; *entryPtr != NULL;
			entryPtr = ( char ** ) ( ( long ) entryPtr + offset ), i++ )
	{
		for ( p1 = key, p2 = *entryPtr; *p1 == *p2; p1++, p2++ )
		{
			if ( *p1 == 0 )
			{
				index = i;
				goto done;
			}
		}

		if ( *p1 == 0 )
		{
			/*
			 * The value is an abbreviation for this entry.  Continue
			 * checking other entries to make sure it's unique.  If we
			 * get more than one unique abbreviation, keep searching to
			 * see if there is an exact match, but remember the number
			 * of unique abbreviations and don't allow either.
			 */

			numAbbrev++;
			index = i;
		}
	}

	if ( ( flags & TCL_EXACT ) || ( numAbbrev != 1 ) )
	{
		goto error;
	}

done:

	*indexPtr = index;

	return TCL_OK;

error:

	if ( interp != NULL )
	{
		int count;
		resultPtr = Tcl_GetObjResult ( interp );
		Tcl_AppendStringsToObj ( resultPtr,
								 ( numAbbrev > 1 ) ? "ambiguous " : "bad ", msg, " \"",
								 key, "\": must be ", *tablePtr, ( char * ) NULL );

		for ( entryPtr = ( char ** ) ( ( long ) tablePtr + offset ), count = 0; *entryPtr != NULL;
				entryPtr = ( char ** ) ( ( long ) entryPtr + offset ), count++ )
		{
			if ( ( * ( ( char ** ) ( ( long ) entryPtr + offset ) ) ) == NULL )
			{
				Tcl_AppendStringsToObj ( resultPtr,
										 ( count > 0 ) ? ", or " : " or ", *entryPtr,
										 ( char * ) NULL );
			}

			else
			{
				Tcl_AppendStringsToObj ( resultPtr, ", ", *entryPtr, ( char * ) NULL );
			}
		}
	}

	return TCL_ERROR;
}



/**
\brief	Sets widget properties.
**/
int gnoclOptGeneric ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, const char *optName, const char *txt[], const int types[], Tcl_Obj **ret )
{
	if ( ret == NULL ) /* set value */
	{
		int idx;

		if ( Tcl_GetIndexFromObj ( interp, opt->val.obj, txt, optName, TCL_EXACT, &idx ) != TCL_OK )
		{
			return TCL_ERROR;
		}

		g_object_set ( obj, opt->propName, types[idx], NULL );
	}

	else /* get value */
	{
		gint val;
		int  k;
		g_object_get ( obj, opt->propName, &val, NULL );

		for ( k = 0; txt[k]; ++k )
		{
			if ( types[k] == val )
			{
				*ret = Tcl_NewStringObj ( txt[k], -1 );
				return TCL_OK;
			}
		}

		Tcl_SetResult ( interp, "Unknown setting for parameter", TCL_STATIC );

		return TCL_ERROR;
	}

	return TCL_OK;
}


/**
\brief
**/
int gnoclOptAnchor ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{

	const char *txt[] = { "center",
						  "N", "NW", "NE",
						  "S", "SW", "SE",
						  "W", "E", NULL
						};
	const int types[] = { GTK_ANCHOR_CENTER,
						  GTK_ANCHOR_NORTH, GTK_ANCHOR_NORTH_WEST, GTK_ANCHOR_NORTH_EAST,
						  GTK_ANCHOR_SOUTH, GTK_ANCHOR_SOUTH_WEST, GTK_ANCHOR_SOUTH_EAST,
						  GTK_ANCHOR_WEST, GTK_ANCHOR_EAST
						};

	assert ( sizeof ( GTK_SHADOW_NONE ) == sizeof ( int ) );

	return gnoclOptGeneric ( interp, opt, obj, "anchor", txt, types, ret );
}

/**
\brief  Set current folder for file/folder chooser dialogs buttons
**/
int gnoclOptCurrentFolder ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{

	gtk_file_chooser_set_current_folder ( GTK_FILE_CHOOSER ( obj ), Tcl_GetString ( opt->val.obj ) );

	return TCL_OK;
}

/**
\brief  Checks the shadow type requested against the list of supported types.
**/
int gnoclOptShadow ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{

	const char *txt[] = { "none", "in", "out", "etchedIn", "etchedOut", NULL };
	const int types[] = { GTK_SHADOW_NONE, GTK_SHADOW_IN, GTK_SHADOW_OUT, GTK_SHADOW_ETCHED_IN, GTK_SHADOW_OUT };

	assert ( sizeof ( GTK_SHADOW_NONE ) == sizeof ( int ) );

	return gnoclOptGeneric ( interp, opt, obj, "shadow-type", txt, types, ret );
}

/**
\brief  This is simply returning, so the arrows are being set elsewhere!
**/
int gnoclOptAngle (	Tcl_Interp *interp,	GnoclOption *opt,	GObject *obj,	Tcl_Obj **ret )
{

	return ;

	const char *txt[] = { "up", "down", "left", "right", "none", NULL };
	const int types[] = { GTK_ARROW_UP, GTK_ARROW_DOWN, GTK_ARROW_LEFT, GTK_ARROW_RIGHT, GTK_ARROW_NONE};

	assert ( sizeof ( GTK_ARROW_RIGHT ) == sizeof ( int ) );

	return gnoclOptGeneric ( interp, opt, obj, "arrow", txt, types, ret );
}


/**
\brief  Set the default widget for a toplevel window.
**/
int gnoclOptDefaultWidget ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{

	GtkWidget *widget = gnoclGetWidgetFromName ( Tcl_GetString ( opt->val.obj ), interp );

	gtk_window_set_default ( obj, widget );

	return TCL_OK;
}

/**
\brief      This is simply returning, so the arrows are being set elsewhere!
**/
int gnoclOptArrow ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{

	return ;

	const char *txt[] = { "up", "down", "left", "right", "none", NULL };
	const int types[] = { GTK_ARROW_UP, GTK_ARROW_DOWN, GTK_ARROW_LEFT, GTK_ARROW_RIGHT, GTK_ARROW_NONE};

	assert ( sizeof ( GTK_ARROW_RIGHT ) == sizeof ( int ) );

	return gnoclOptGeneric ( interp, opt, obj, "arrow", txt, types, ret );
}


/**
\brief	Set relief for  button widgets etc.
**/
int gnoclOptRelief ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	const char *txt[] = { "normal", "half", "none", NULL };
	const int types[] = { GTK_RELIEF_NORMAL, GTK_RELIEF_HALF, GTK_RELIEF_NONE };

	assert ( sizeof ( GTK_RELIEF_NORMAL ) == sizeof ( int ) );

	return gnoclOptGeneric ( interp, opt, obj, "relief", txt, types, ret );
}

/**
\brief	Set relief for toolbar buttons.
**/

int gnoclOptButtonRelief ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	const char *txt[] = { "normal", "half", "none", NULL };
	const int types[] = { GTK_RELIEF_NORMAL, GTK_RELIEF_HALF, GTK_RELIEF_NONE };


	static const gchar *rc_string =
	{
		"style \"normal\"\n"
		"{\n"
		"   GtkToolbar::button-relief = GTK_RELIEF_NORMAL \n"
		"}\n"
		"\n"
		"class \"GtkToolbar\" style \"normal\"\n"
	};

	gtk_rc_parse_string ( rc_string );

	return TCL_OK;

}

/**
\brief
**/
int gnoclOptPangoStretch ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	const char *txt[] = { "ultraCondensed", "extraCondensed", "condensed",
						  "semiCondensed", "normal", "semiExpanded", "expanded",
						  "extraExpanded", "ultraExpanded", NULL
						};
	const int types[] = { PANGO_STRETCH_ULTRA_CONDENSED,
						  PANGO_STRETCH_EXTRA_CONDENSED, PANGO_STRETCH_CONDENSED,
						  PANGO_STRETCH_SEMI_CONDENSED, PANGO_STRETCH_NORMAL,
						  PANGO_STRETCH_SEMI_EXPANDED, PANGO_STRETCH_EXPANDED,
						  PANGO_STRETCH_EXTRA_EXPANDED, PANGO_STRETCH_ULTRA_EXPANDED
						};

	assert ( sizeof ( PANGO_STRETCH_ULTRA_CONDENSED ) == sizeof ( int ) );

	return gnoclOptGeneric ( interp, opt, obj, "stretch", txt, types, ret );
}


/**
\brief
**/
int gnoclOptPangoWeight ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	const char *txt[] = { "ultralight", "light", "normal", "bold", "ultrabold",
						  "heavy", NULL
						};
	const int types[] = { PANGO_WEIGHT_ULTRALIGHT,
						  PANGO_WEIGHT_LIGHT, PANGO_WEIGHT_NORMAL, PANGO_WEIGHT_BOLD,
						  PANGO_WEIGHT_ULTRABOLD, PANGO_WEIGHT_HEAVY
						};

	assert ( sizeof ( PANGO_WEIGHT_ULTRALIGHT ) == sizeof ( int ) );

	return gnoclOptGeneric ( interp, opt, obj, "weight", txt, types, ret );
}


/**
\brief
**/
int gnoclOptPangoVariant ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	const char *txt[] = { "normal", "smallCaps", NULL };
	const int types[] = { PANGO_VARIANT_NORMAL, PANGO_VARIANT_SMALL_CAPS };

	assert ( sizeof ( PANGO_VARIANT_SMALL_CAPS ) == sizeof ( int ) );

	return gnoclOptGeneric ( interp, opt, obj, "variant", txt, types, ret );
}


/**
\brief
**/
int gnoclOptPangoStyle ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	const char *txt[] = { "normal", "oblique", "italic", NULL };
	const int types[] = { PANGO_STYLE_NORMAL, PANGO_STYLE_OBLIQUE, PANGO_STYLE_ITALIC };

	assert ( sizeof ( PANGO_STYLE_NORMAL ) == sizeof ( int ) );

	return gnoclOptGeneric ( interp, opt, obj, "style", txt, types, ret );
}


/**
\brief	Set wrapmode options for rendering treelist text cells
**/
int gnoclOptPangoWrapMode ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{

	const char *mode[] = { "word", "char", "wordChar", NULL };
	const int types[] = {  PANGO_WRAP_WORD, PANGO_WRAP_CHAR, PANGO_WRAP_WORD_CHAR};

	assert ( sizeof ( PANGO_WRAP_WORD ) == sizeof ( int ) );

	return gnoclOptGeneric ( interp, opt, obj, "wrap-mode", mode, types, ret );

}

/**
\brief
\author     Peter G Baum
\date
\since
**/
int gnoclOptPangoScaledInt ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	int i;

	if ( Tcl_GetIntFromObj ( interp, opt->val.obj, &i ) != TCL_OK )
	{
		return TCL_ERROR;
	}

	g_object_set ( obj, opt->propName, i * PANGO_SCALE, NULL );

	return TCL_OK;
}

/*
typedef enum {
  PANGO_ELLIPSIZE_NONE,
  PANGO_ELLIPSIZE_START,
  PANGO_ELLIPSIZE_MIDDLE,
  PANGO_ELLIPSIZE_END
} PangoEllipsizeMode;
*/
/**
\brief      Set ellipsize property for label widgets
\author     William J Giddings
\date       14/Jan/2010
**/
int gnoclOptEllipsize ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	const char *txt[] = { "none", "start", "middle", "end", NULL };
	const int types[] = { PANGO_ELLIPSIZE_NONE, PANGO_ELLIPSIZE_START, PANGO_ELLIPSIZE_MIDDLE, PANGO_ELLIPSIZE_END };

	assert ( sizeof ( PANGO_ELLIPSIZE_END ) == sizeof ( int ) );

	return gnoclOptGeneric ( interp, opt, obj, "ellipsize", txt, types,  ret );
}

/**
\brief
\author     Peter G Baum
\date
\since
**/
int gnoclOptJustification ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	const char *txt[] = { "left", "right", "center", "fill", NULL };
	const int types[] = { GTK_JUSTIFY_LEFT, GTK_JUSTIFY_RIGHT, GTK_JUSTIFY_CENTER, GTK_JUSTIFY_FILL };

	assert ( sizeof ( GTK_JUSTIFY_FILL ) == sizeof ( int ) );

	return gnoclOptGeneric ( interp, opt, obj, "justification", txt, types,  ret );
}

/**
\brief
\author     Peter G Baum
\date
\since
**/
int gnoclOptScale ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	double d;

	if ( Tcl_GetDoubleFromObj ( NULL, opt->val.obj, &d ) != TCL_OK )
	{
		const char *txt[] = { "xx-small", "x-small", "small", "medium",
							  "large", "x-large", "xx-large", NULL
							};
		const double types[] = { PANGO_SCALE_XX_SMALL, PANGO_SCALE_X_SMALL,
								 PANGO_SCALE_SMALL, PANGO_SCALE_MEDIUM, PANGO_SCALE_LARGE,
								 PANGO_SCALE_X_LARGE, PANGO_SCALE_XX_LARGE
							   };

		int idx;

		if ( Tcl_GetIndexFromObj ( NULL, opt->val.obj, txt, NULL,
								   TCL_EXACT, &idx ) != TCL_OK )
		{
			Tcl_AppendResult ( interp, "Unknown scale \"",
							   Tcl_GetString ( opt->val.obj ),
							   "\", must be a floating point value or one of xx-small, "
							   "x-small, small, medium, large, x-large, or xx-large.", NULL );
			return TCL_ERROR;
		}

		d = types[idx];
	}

	g_object_set ( obj, opt->propName, d, NULL );

	return TCL_OK;
}

/**
\brief
\author     Peter G Baum
\date
\since
**/
int gnoclGetPadding ( Tcl_Interp *interp, Tcl_Obj *obj, int *pad )
{
	if ( Tcl_GetIntFromObj ( NULL, obj, pad ) != TCL_OK )
	{
		const char *txt[] = { "small", "normal", "big", NULL };
		const int vals[] = { GNOCL_PAD_SMALL, GNOCL_PAD, GNOCL_PAD_BIG };

		int idx;

		if ( Tcl_GetIndexFromObj ( NULL, obj, txt, NULL,
								   TCL_EXACT, &idx ) != TCL_OK )
		{
			Tcl_AppendResult ( interp, "Unknown padding \"",
							   Tcl_GetString ( obj ),
							   "\", must be an integer or one small, normal or big", NULL );
			return TCL_ERROR;
		}

		*pad = vals[idx];
	}

	return TCL_OK;
}

/**
\brief
\author     Peter G Baum
\date
\since
**/
int gnoclOptPadding ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	int pad;

	if ( ret == NULL ) /* set value */
	{
		if ( gnoclGetPadding ( interp, opt->val.obj, &pad ) != TCL_OK )
		{
			return TCL_ERROR;
		}

		g_object_set ( obj, opt->propName, pad, NULL );
	}

	else /* get value */
	{
		g_object_get ( obj, opt->propName, &pad, NULL );
		*ret = Tcl_NewIntObj ( pad );
	}

	return TCL_OK;
}

/**
\brief	Set orientation of widget based upon new API
**/
int gnoclOptOrientation ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{

	if ( ret == NULL ) /* set value */
	{

		if ( strcmp ( Tcl_GetString ( opt->val.obj ), "horizontal" ) == 0 )
		{

			gtk_orientable_set_orientation ( GTK_ORIENTABLE ( obj ), GTK_ORIENTATION_HORIZONTAL );

		}

		else if ( strcmp ( Tcl_GetString ( opt->val.obj ), "vertical" ) == 0 )
		{

			gtk_orientable_set_orientation ( GTK_ORIENTABLE ( obj ), GTK_ORIENTATION_VERTICAL );
		}

		else
		{
			return TCL_ERROR;
		}

	}

	return TCL_OK;
}

/**
\brief
\author     Peter G Baum
\date
\since
**/
int gnoclOptUnderline ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	const char *txt[] = { "none", "single", "double", "low", "error", NULL };
	const int types[] = { PANGO_UNDERLINE_NONE, PANGO_UNDERLINE_SINGLE,
						  PANGO_UNDERLINE_DOUBLE, PANGO_UNDERLINE_LOW, PANGO_UNDERLINE_ERROR
						};

	assert ( sizeof ( PANGO_UNDERLINE_LOW ) == sizeof ( int ) );

	return gnoclOptGeneric ( interp, opt, obj, "underline", txt, types, ret );
}

/**
\brief
\author     Peter G Baum
\date
\since
**/
int gnoclOptWrapmode ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	const char *txt[] = { "none", "char", "word", NULL };
	const int types[] = { GTK_WRAP_NONE, GTK_WRAP_CHAR, GTK_WRAP_WORD };

	assert ( sizeof ( PANGO_VARIANT_SMALL_CAPS ) == sizeof ( int ) );

	return gnoclOptGeneric ( interp, opt, obj, "wrap mode", txt, types, ret );
}

/**
\brief
\author     Peter G Baum
\date
\since
**/
int gnoclOptPosition ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	const char *txt[] = { "left", "right", "top", "bottom", NULL };
	const int types[] = { GTK_POS_LEFT, GTK_POS_RIGHT, GTK_POS_TOP, GTK_POS_BOTTOM };

	assert ( sizeof ( PANGO_VARIANT_SMALL_CAPS ) == sizeof ( int ) );

	return gnoclOptGeneric ( interp, opt, obj, "position", txt, types, ret );
}



/**
\bug
    With window this produces:
    (gnocl:2370): Gtk-CRITICAL **: file gtkwidget.c: line 4891
    (gtk_widget_get_parent_window): assertion `widget->parent != NULL' failed

    (gnocl:2370): Gdk-CRITICAL **: file gdkwindow-x11.c: line 2363
    (gdk_window_set_cursor): assertion `window != NULL' failed

    with eventBox the cursor is set for the whole window, not only for the
    eventBox.
*/
#if 1

/* FIXME:

with window this produces:
   (gnocl:2370): Gtk-CRITICAL **: file gtkwidget.c: line 4891
   (gtk_widget_get_parent_window): assertion `widget->parent != NULL' failed

   (gnocl:2370): Gdk-CRITICAL **: file gdkwindow-x11.c: line 2363
   (gdk_window_set_cursor): assertion `window != NULL' failed

with eventBox the cursor is set for the whole window, not only for the
eventBox.
*/

int gnoclOptCursor ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
#ifdef DEBUG_PARSEOPTIONS
	g_print ( "gnoclOptCursor\n" );
#endif

	typedef struct
	{
		const char *name;
		GdkCursorType id;
	} CursorNameToId;

	const CursorNameToId cursors[] =
	{
		{ "xCursor", GDK_X_CURSOR },
		{ "arrow", GDK_ARROW },
		{ "basedArrowDown", GDK_BASED_ARROW_DOWN },
		{ "basedArrowUp", GDK_BASED_ARROW_UP },
		{ "boat", GDK_BOAT },
		{ "bogosity", GDK_BOGOSITY },
		{ "bottomLeftCorner", GDK_BOTTOM_LEFT_CORNER },
		{ "bottomRightCorner", GDK_BOTTOM_RIGHT_CORNER },
		{ "bottomSide", GDK_BOTTOM_SIDE },
		{ "bottomTee", GDK_BOTTOM_TEE },
		{ "boxSpiral", GDK_BOX_SPIRAL },
		{ "centerPtr", GDK_CENTER_PTR },
		{ "circle", GDK_CIRCLE },
		{ "clock", GDK_CLOCK },
		{ "coffeeMug", GDK_COFFEE_MUG },
		{ "cross", GDK_CROSS },
		{ "crossReverse", GDK_CROSS_REVERSE },
		{ "crosshair", GDK_CROSSHAIR },
		{ "diamondCross", GDK_DIAMOND_CROSS },
		{ "dot", GDK_DOT },
		{ "dotbox", GDK_DOTBOX },
		{ "doubleArrow", GDK_DOUBLE_ARROW },
		{ "draftLarge", GDK_DRAFT_LARGE },
		{ "draftSmall", GDK_DRAFT_SMALL },
		{ "drapedBox", GDK_DRAPED_BOX },
		{ "exchange", GDK_EXCHANGE },
		{ "fleur", GDK_FLEUR },
		{ "gobbler", GDK_GOBBLER },
		{ "gumby", GDK_GUMBY },
		{ "hand1", GDK_HAND1 },
		{ "hand2", GDK_HAND2 },
		{ "heart", GDK_HEART },
		{ "icon", GDK_ICON },
		{ "ironCross", GDK_IRON_CROSS },
		{ "leftPtr", GDK_LEFT_PTR },
		{ "leftSide", GDK_LEFT_SIDE },
		{ "leftTee", GDK_LEFT_TEE },
		{ "leftbutton", GDK_LEFTBUTTON },
		{ "llAngle", GDK_LL_ANGLE },
		{ "lrAngle", GDK_LR_ANGLE },
		{ "man", GDK_MAN },
		{ "middlebutton", GDK_MIDDLEBUTTON },
		{ "mouse", GDK_MOUSE },
		{ "pencil", GDK_PENCIL },
		{ "pirate", GDK_PIRATE },
		{ "plus", GDK_PLUS },
		{ "questionArrow", GDK_QUESTION_ARROW },
		{ "rightPtr", GDK_RIGHT_PTR },
		{ "rightSide", GDK_RIGHT_SIDE },
		{ "rightTee", GDK_RIGHT_TEE },
		{ "rightbutton", GDK_RIGHTBUTTON },
		{ "rtlLogo", GDK_RTL_LOGO },
		{ "sailboat", GDK_SAILBOAT },
		{ "sbDownArrow", GDK_SB_DOWN_ARROW },
		{ "sbHDoubleArrow", GDK_SB_H_DOUBLE_ARROW },
		{ "sbLeftArrow", GDK_SB_LEFT_ARROW },
		{ "sbRightArrow", GDK_SB_RIGHT_ARROW },
		{ "sbUpArrow", GDK_SB_UP_ARROW },
		{ "sbVDoubleArrow", GDK_SB_V_DOUBLE_ARROW },
		{ "shuttle", GDK_SHUTTLE },
		{ "sizing", GDK_SIZING },
		{ "spider", GDK_SPIDER },
		{ "spraycan", GDK_SPRAYCAN },
		{ "star", GDK_STAR },
		{ "target", GDK_TARGET },
		{ "tcross", GDK_TCROSS },
		{ "topLeftArrow", GDK_TOP_LEFT_ARROW },
		{ "topLeftCorner", GDK_TOP_LEFT_CORNER },
		{ "topRightCorner", GDK_TOP_RIGHT_CORNER },
		{ "topSide", GDK_TOP_SIDE },
		{ "topTee", GDK_TOP_TEE },
		{ "trek", GDK_TREK },
		{ "ulAngle", GDK_UL_ANGLE },
		{ "umbrella", GDK_UMBRELLA },
		{ "urAngle", GDK_UR_ANGLE },
		{ "watch", GDK_WATCH },
		{ "xterm", GDK_XTERM },
		{ "last", GDK_LAST_CURSOR},
		{ "blank", GDK_BLANK_CURSOR},

		/* type of cursors constructed with gdk_cursor_new_from_pixmap() or gdk_cursor_new_from_pixbuf() */
		{ "pixmap", GDK_CURSOR_IS_PIXMAP}
	};

	int idx;


	if ( gnoclGetIndexFromObjStruct ( interp, opt->val.obj, ( char ** ) &cursors[0].name, sizeof ( CursorNameToId ), "cursor", TCL_EXACT, &idx ) != TCL_OK )
	{
		return TCL_ERROR;
	}

	if ( 1 )
	{
		/* this fails for some reason */
		//gdk_window_set_cursor ( gtk_widget_get_parent_window ( GTK_WIDGET ( obj ) ), gdk_cursor_new ( cursors[idx].id ) );
		gdk_window_set_cursor ( GTK_WIDGET ( obj )->window , gdk_cursor_new ( cursors[idx].id ) );
	}

	return TCL_OK;
}

#endif


/**
 */
static const char *keyvalToString ( guint keyval )
{

	static GHashTable *keysyms = NULL;

	if ( keysyms == NULL )
	{

		const struct
		{
			const char *name; guint key;
		} syms[] =

		{
#include "keysyms.h"
		};
		unsigned int k;
		keysyms = g_hash_table_new ( g_direct_hash, g_direct_equal );

		for ( k = 0; k < sizeof ( syms ) / sizeof ( *syms ); ++k )
			g_hash_table_insert ( keysyms, GUINT_TO_POINTER ( syms[k].key ),
								  ( gpointer ) syms[k].name );
	}

	return g_hash_table_lookup ( keysyms, GUINT_TO_POINTER ( keyval ) );
}



/**
\brief
\author     Peter G Baum
\date
**/
static int getShortValue ( Tcl_Interp *interp,	Tcl_Obj *list,	int idx, int *p )
{
	int val;
	Tcl_Obj *tp;

	if ( Tcl_ListObjIndex ( interp, list, idx, &tp ) != TCL_OK  )
	{
		return TCL_ERROR;
	}

	if ( Tcl_GetIntFromObj ( NULL, tp, &val ) != TCL_OK )
	{
		double d;

		if ( Tcl_GetDoubleFromObj ( NULL, tp, &d ) != TCL_OK )
		{
			Tcl_AppendResult ( interp,
							   "expected integer or double, but got \"",
							   Tcl_GetString ( tp ), "\"", NULL );
			return TCL_ERROR;
		}

		val = d * 0xFFFF;
	}

	if ( val < .0 || val > 0xFFFF )
	{
		Tcl_SetResult ( interp, "color value must be between 0 and 65535", TCL_STATIC );
		return TCL_ERROR;
	}

	*p = val;

	return TCL_OK;
}



/**
\brief
\author
\date
**/
static int getRGBA ( Tcl_Interp *interp, Tcl_Obj *obj, int *r, int *g, int *b, int *a )
{
	int no;

	if ( Tcl_ListObjLength ( interp, obj, &no ) != TCL_OK  || no < 0 || no > 4 )
	{
		Tcl_SetResult ( interp, "color must be either \"name\" or a list "
						"consisting of \"name alpha\", \"r g b\", or \"r g b alpha\"",
						TCL_STATIC );
		{
			return TCL_ERROR;
		}
	}

	if ( no == 0 ) /* transparent */
	{
		*r = *g = *b = *a = 0;
	}

	else if ( no < 3 )
	{
		Tcl_Obj *tp = obj;
		GdkColor color;

		if ( no == 2 )
		{
			if ( Tcl_ListObjIndex ( interp, obj, 0, &tp ) != TCL_OK )
				return TCL_ERROR;
		}

		/* take as string and reformat as a GdkColor structure */

		if ( gdk_color_parse ( Tcl_GetString ( tp ), &color ) == 0 )
		{
			Tcl_AppendResult ( interp, "unknown color \"", Tcl_GetString ( obj ), "\".", ( char * ) NULL );
			return TCL_ERROR;
		}

		*r = color.red;
		*g = color.green;
		*b = color.blue;

		if ( no == 2 )
		{
			if ( getShortValue ( interp, obj, 1, a ) != TCL_OK )
				return TCL_ERROR;
		}

		else
			*a = 0xFFFF;
	}

	else
	{
		if ( getShortValue ( interp, obj, 0, r ) != TCL_OK
				|| getShortValue ( interp, obj, 1, g ) != TCL_OK
				|| getShortValue ( interp, obj, 2, b ) != TCL_OK )
			return TCL_ERROR;

		if ( no == 4 )
		{
			if ( getShortValue ( interp, obj, 3, a ) != TCL_OK )
				return TCL_ERROR;
		}

		else
			*a = 0xFFFF;
	}

	return TCL_OK;
}

/**
\brief Take colour values from a string format and assign them to location
        addressed by pointer *color.
\author
\date
**/
int getGdkColor ( Tcl_Interp *interp, Tcl_Obj *obj, GdkColor *color )
{
	int r, g, b, a;

	if ( getRGBA ( interp, obj, &r, &g, &b, &a ) != TCL_OK )
	{
		return TCL_ERROR;
	}

	/* TODO? if a != 0xFFFF: alpha not supported? */
	color->red = r;

	color->green = g;

	color->blue = b;

	return TCL_OK;
}

/**
\brief  Set the colour attribute of an object. Prior to setting, the format
        of the colour parameter if checked by the function getGdkColor.
\author
\date
**/
int gnoclOptGdkColor ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	GdkColor color;

	if ( getGdkColor ( interp, opt->val.obj, &color ) == TCL_OK )
	{
		g_object_set ( obj, opt->propName, &color, NULL );
		return TCL_OK;
	}

	return TCL_ERROR;
}

/**
 */
int gnoclOptRGBAColor ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	int r, g, b, a;

	if ( getRGBA ( interp, opt->val.obj, &r, &g, &b, &a ) == TCL_OK )
	{
		guint col = ( guint ) ( ( ( r & 0xFF00 ) << 16 ) |
								( ( g & 0xFF00 ) << 8 ) | ( b & 0xFF00 ) | ( a >> 8 ) );
		g_object_set ( obj, opt->propName, col, NULL );
		return TCL_OK;
	}

	return TCL_ERROR;
}

/**
\brief
**/
int gnoclOptArrowTooltip ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{

	const char *text = Tcl_GetString ( opt->val.obj );

	gtk_menu_tool_button_set_arrow_tooltip_text ( GTK_WIDGET ( obj ), text );

	return TCL_OK;
}


/**
\brief
**/
int gnoclOptIconTooltip ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
#ifdef DEBUG_PARSEOPTIONS
	g_print ( "%s %s %s\n", __FUNCTION__, opt->propName, Tcl_GetString ( opt->val.obj ) );
#endif

	assert ( *opt->propName == 'P' || *opt->propName == 'S' );

	const char *txt = Tcl_GetString ( opt->val.obj );

	if ( *opt->propName == 'P' )
	{
		gtk_entry_set_icon_tooltip_markup   ( GTK_ENTRY ( obj ), GTK_ENTRY_ICON_PRIMARY, txt );
		return  TCL_OK;
	}

	if ( *opt->propName == 'S' )
	{
		gtk_entry_set_icon_tooltip_text   ( GTK_ENTRY ( obj ), GTK_ENTRY_ICON_SECONDARY, txt );
		return  TCL_OK;
	}


	return TCL_ERROR;

}


/**
/brief      Set tooltip for specified widget.
/author     Peter G Baum
/note       http://developer.gnome.org/gtk/stable/GtkTooltip.html
/todo		parse markup string for various options
*/
int gnoclOptTooltip ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	assert ( opt->propName == NULL || *opt->propName  == '\0' );
	assert ( strcmp ( opt->optName, "-tooltip" ) == 0 );

	if ( ret == NULL ) /* set value */
	{
		const char *txt = Tcl_GetString ( opt->val.obj );
		GtkTooltips *tt = gnoclGetTooltips();

		/* #ifdef GNOCL_USE_GNOME
		   if( GTK_CHECK_TYPE( widget, GTK_TYPE_ITEM ) )
		   {
		      g_signal_connect ( obj, "select",
		            G_CALLBACK( putHintInAppBar ),
		            g_memdup( txt, strlen( txt ) + 1 ) );

		      g_signal_connect( obj, "deselect",
		            G_CALLBACK( removeHintFromAppBar), obj );
		   }
		   else
		#endif */

		if ( *txt )
		{
			//gtk_tooltips_set_tip ( tt, GTK_WIDGET ( obj ), txt, NULL );
			//gtk_widget_set_tooltip_text ( GTK_WIDGET ( obj ), txt );
			gtk_widget_set_tooltip_markup ( GTK_WIDGET ( obj ), txt );
		}

		else
		{
			//gtk_tooltips_set_tip ( tt, GTK_WIDGET ( obj ), NULL, NULL );
			//gtk_widget_set_tooltip_text ( GTK_WIDGET ( obj ), NULL );
			gtk_widget_set_tooltip_markup ( GTK_WIDGET ( obj ), NULL );
		}
	}

	else /* get value */
	{
		GtkTooltipsData *td = gtk_tooltips_data_get ( GTK_WIDGET ( obj ) );

		if ( td && td->tip_text )
			*ret = Tcl_NewStringObj ( td->tip_text, -1 );
		else
			*ret = Tcl_NewStringObj ( "", 0 );
	}

	return TCL_OK;
}



/**
**/
int modifyWidgetGdkColor ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, void ( *func ) ( GtkWidget *, GtkStateType, const GdkColor * ), glong offset, Tcl_Obj **ret )
{

#ifdef DEBUG_PARSEOPTIONS
	g_print ( "parseOptions/modifyWidgetGdkColor\n" );
#endif

	GtkStateType type;

	switch ( *opt->propName )
	{
		case 'n': type = GTK_STATE_NORMAL; break;
		case 'a': type = GTK_STATE_ACTIVE; break;
		case 'p': type = GTK_STATE_PRELIGHT; break;
		case 's': type = GTK_STATE_SELECTED; break;
		case 'i': type = GTK_STATE_INSENSITIVE; break;
		default:     assert ( 0 );
	}

	if ( ret == NULL ) /* set value */
	{
		GdkColor color;

		if ( getGdkColor ( interp, opt->val.obj, &color ) != TCL_OK )
		{
			return TCL_ERROR;
		}

		( *func ) ( GTK_WIDGET ( obj ), type, &color );
	}

	else /* get value */
	{
		/* gtk_widget_get_modifier_style() FIXME: where are the differences?
		   gtk_widget_get_style( )
		   gtk_rc_get_style */
		/*
		GtkRcStyle *style = gtk_widget_get_modifier_style( GTK_WIDGET( obj ) );
		GtkStyle *style = gtk_widget_get_style( GTK_WIDGET( obj ) );
		*/
		GtkStyle *style = gtk_rc_get_style ( GTK_WIDGET ( obj ) );
		GdkColor *cp = ( GdkColor * ) G_STRUCT_MEMBER_P ( style, offset );
		GdkColor color = cp[type];
		*ret = Tcl_NewListObj ( 0, NULL );
		Tcl_ListObjAppendElement ( NULL, *ret, Tcl_NewIntObj ( color.red ) );
		Tcl_ListObjAppendElement ( NULL, *ret, Tcl_NewIntObj ( color.green ) );
		Tcl_ListObjAppendElement ( NULL, *ret, Tcl_NewIntObj ( color.blue ) );
	}

	return TCL_OK;
}


/**
 */
int gnoclOptGdkColorBg ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	return modifyWidgetGdkColor ( interp, opt, obj, gtk_widget_modify_bg, G_STRUCT_OFFSET ( GtkStyle, bg ), ret );
}

/**
**/
int gnoclOptGdkColorFg ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	return modifyWidgetGdkColor ( interp, opt, obj, gtk_widget_modify_fg, G_STRUCT_OFFSET ( GtkStyle, fg ), ret );
}

/**
 */
int gnoclOptGdkColorText ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	return modifyWidgetGdkColor ( interp, opt, obj, gtk_widget_modify_text, G_STRUCT_OFFSET ( GtkStyle, text ), ret );
}

/**
 */
int gnoclOptGdkColorBase ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	return modifyWidgetGdkColor ( interp, opt, obj, gtk_widget_modify_base, G_STRUCT_OFFSET ( GtkStyle, base ), ret );
}

/**
 */
int gnoclOptGdkBaseFont ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{

	char *fnt = Tcl_GetStringFromObj ( opt->val.obj, NULL );
	PangoFontDescription *font_desc = pango_font_description_from_string ( fnt );

	gtk_widget_modify_font ( GTK_WIDGET ( obj ), font_desc );
	pango_font_description_free ( font_desc );

	return TCL_OK;
}

/**
 */
char *gnoclGetGdkBaseFont ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{

	char *fnt = Tcl_GetStringFromObj ( opt->val.obj, NULL );
	PangoFontDescription *font_desc = pango_font_description_from_string ( fnt );

	gtk_widget_modify_font ( GTK_WIDGET ( obj ), font_desc );
	pango_font_description_free ( font_desc );

	return fnt;
}


/**
 */
/* -----------------
   group <-> sizeGroup mapping
-------------------- */
static int groupToIdx ( GtkSizeGroupMode mode )
{
	switch ( mode )
	{
		case GTK_SIZE_GROUP_HORIZONTAL: return 1;
		case GTK_SIZE_GROUP_VERTICAL:   return 2;
		default: assert ( mode == GTK_SIZE_GROUP_BOTH );
	}

	return 0;
}


/**
 */
static void destroySizeGroup (  gpointer data,  GObject *obj )
{
	const char   *name = data;
	assert ( groupToIdx ( GTK_SIZE_GROUP_BOTH ) == 0 );
	g_hash_table_remove ( sizeGroupTables[0], name );
}

/**
 */
static void destroyWidthGroup ( gpointer data,  GObject *obj )
{
	const char   *name = data;
	assert ( groupToIdx ( GTK_SIZE_GROUP_HORIZONTAL ) == 1 );
	g_hash_table_remove ( sizeGroupTables[1], name );
}

/**
\brief
\author
\date
\note
**/
static void destroyHeightGroup (    gpointer data,  GObject *obj )
{
	const char   *name = data;
	assert ( groupToIdx ( GTK_SIZE_GROUP_VERTICAL ) == 2 );
	g_hash_table_remove ( sizeGroupTables[2], name );
}

/**
\brief
\author
\date
\note
**/
static int addSizeGroup (   GtkWidget *widget,  GtkSizeGroupMode mode,  const char *name )
{
	int new = 0;
	GHashTable   *table = sizeGroupTables[groupToIdx ( mode ) ];
	GtkSizeGroup *group = g_hash_table_lookup ( table, name );

	if ( group == NULL )
	{
		GWeakNotify destroyFunc[3] = { destroySizeGroup,
									   destroyWidthGroup, destroyHeightGroup
									 };
		char *str = g_strdup ( name );
		group = gtk_size_group_new ( mode );
		g_hash_table_insert ( table, str, ( gpointer ) group );
		g_object_weak_ref ( G_OBJECT ( group ),
							destroyFunc[groupToIdx ( mode ) ], str );
		new = 1;
	}

	gtk_size_group_add_widget ( group, widget );

	g_object_set_data_full ( G_OBJECT ( widget ), "gnocl::sizeGroup",
							 g_strdup ( name ), g_free );

	if ( new )
		g_object_unref ( group );

	return 0;
}

/**
\brief
\author
\date
\note
**/
static const char *getSizeGroup (   GtkWidget *widget,  GtkSizeGroupMode mode )
{
	return g_object_get_data ( G_OBJECT ( widget ), "gnocl::sizeGroup" );
}

/**
\brief
\author
\date
\note
**/
static int removeSizeGroup ( GtkWidget *widget, GtkSizeGroupMode mode )
{
	const char *name = getSizeGroup ( widget, mode );
	GtkSizeGroup *group;

	if ( name == NULL )
		return 0;

	group = g_hash_table_lookup ( sizeGroupTables[groupToIdx ( mode ) ], name );

	if ( group == NULL )
		return 0;

	gtk_size_group_remove_widget ( group, widget );

	g_object_set_data ( G_OBJECT ( widget ), "gnocl::sizeGroup", NULL );

	return 1;
}

/**
\brief
\author
\date
\note
**/
int gnoclOptSizeGroup ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	GtkSizeGroupMode mode = GTK_SIZE_GROUP_BOTH;

	switch ( opt->optName[1] )
	{
		case 'w':   mode = GTK_SIZE_GROUP_HORIZONTAL; break;
		case 'h':   mode = GTK_SIZE_GROUP_VERTICAL;   break;
		default: assert ( opt->optName[1] == 's' );
	}

	if ( sizeGroupTables[groupToIdx ( mode ) ] == NULL )
	{
		sizeGroupTables[groupToIdx ( mode ) ] =
			g_hash_table_new_full ( g_str_hash, g_str_equal, g_free, NULL );
	}

	if ( ret == NULL ) /* set value */
	{
		const char *group = Tcl_GetString ( opt->val.obj );

		if ( *group )
		{
			addSizeGroup ( GTK_WIDGET ( obj ), mode, group  );
		}

		else
		{
			removeSizeGroup ( GTK_WIDGET ( obj ), mode );
		}
	}

	else              /* get value */
	{
		const char *group = getSizeGroup ( GTK_WIDGET ( obj ), mode );

		if ( group )
		{
			*ret = Tcl_NewStringObj ( group, -1 );
		}

		else
		{
			*ret = Tcl_NewStringObj ( "", 0 );
		}
	}

	return TCL_OK;
}

/**
\brief
\author
\date
\note	used in megawigets such as labelEntry
**/
int gnoclOptWidthGroup ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	GtkSizeGroupMode mode = GTK_SIZE_GROUP_HORIZONTAL;

	if ( sizeGroupTables[groupToIdx ( mode ) ] == NULL )
	{
		sizeGroupTables[groupToIdx ( mode ) ] =
			g_hash_table_new_full ( g_str_hash, g_str_equal, g_free, NULL );
	}

	if ( ret == NULL ) /* set value */
	{
		const char *group = Tcl_GetString ( opt->val.obj );

		if ( *group )
		{
			addSizeGroup ( GTK_WIDGET ( obj ), mode, group  );
		}

		else
		{
			removeSizeGroup ( GTK_WIDGET ( obj ), mode );
		}
	}

	else              /* get value */
	{
		const char *group = getSizeGroup ( GTK_WIDGET ( obj ), mode );

		if ( group )
		{
			*ret = Tcl_NewStringObj ( group, -1 );
		}

		else
		{
			*ret = Tcl_NewStringObj ( "", 0 );
		}
	}

	return TCL_OK;
}

/**
\brief
\author
\date
\note
**/
static int optAlign ( Tcl_Interp *interp, Tcl_Obj *obj, int isHor,  gfloat *f )
{
	double d = -1;

	if ( Tcl_GetDoubleFromObj ( NULL, obj, &d ) != TCL_OK )
	{
		char *txt = Tcl_GetString ( obj );

		if ( strcmp ( txt, isHor ? "left" : "top" ) == 0 )
			d = .0;
		else if ( strcmp ( txt, "center" ) == 0 )
			d = 0.5;
		else if ( strcmp ( txt, isHor ? "right" : "bottom" ) == 0 )
			d = 1.;
	}

	if ( d < .0 || d > 1. )
	{
		if ( interp )
		{
			if ( isHor )
				Tcl_SetResult ( interp, "Horizontal alignement must be either "
								"a double value between 0 and 1 or "
								"\"left\", \"center\", or \"right\"",
								TCL_STATIC );
			else
				Tcl_SetResult ( interp, "Horizontal alignement must be either "
								"a double value between 0 and 1 or "
								"\"top\", \"center\", or \"bottom\"",
								TCL_STATIC );
		}

		return TCL_ERROR;
	}

	*f = d;

	return TCL_OK;
}

/**
\brief
\author
\date
\note       - a list with one element for x and one for y
            or - a double
            or - one of the predifined strings
**/
int gnoclGetBothAlign ( Tcl_Interp *interp, Tcl_Obj *obj,   gfloat *xAlign, gfloat *yAlign )
{
	int    len = 0;
	double d;

	if ( Tcl_GetDoubleFromObj ( NULL, obj, &d ) == TCL_OK )
	{
		*xAlign = d;
		*yAlign = d;
	}

	else if ( Tcl_ListObjLength ( NULL, obj, &len ) == TCL_OK && len == 2 )
	{
		Tcl_Obj *tp;

		if ( Tcl_ListObjIndex ( NULL, obj, 0, &tp ) != TCL_OK )
		{
			goto cleanExit;
		}

		if ( optAlign ( interp, tp, 0, xAlign ) != TCL_OK )
		{
			goto cleanExit;
		}

		if ( Tcl_ListObjIndex ( NULL, obj, 1, &tp ) != TCL_OK )
		{
			goto cleanExit;
		}

		if ( optAlign ( NULL, tp, 1, yAlign ) != TCL_OK )
		{
			goto cleanExit;
		}
	}

	else if ( len == 1 )
	{
		const char *txt[] = { "topLeft", "top", "topRight",
							  "left", "center", "right",
							  "bottomLeft", "bottom", "bottomRight",
							  NULL
							};

		int idx;

		if ( Tcl_GetIndexFromObj ( NULL, obj, txt, NULL, TCL_EXACT, &idx ) != TCL_OK )
		{
			goto cleanExit;
		}

		*xAlign = ( idx % 3 ) * 0.5;

		*yAlign = ( idx / 3 ) * 0.5;
	}

	return TCL_OK;

cleanExit:
	Tcl_SetResult ( interp, "Alignment must be either a list with "
					"the vertical and horizontal alignment or one of "
					"\"topLeft\", \"top\", \"topRight\", "
					"\"left\", \"center\", \"right\", "
					"\"bottomLeft\", \"bottom\", or \"bottomRight\"", TCL_STATIC );
	return TCL_ERROR;

}

/**
\brief
\author
\date
\note
**/
int gnoclOptBothAlign ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	gfloat xAlign, yAlign;
	char *p;
	char buffer[32];
	strcpy ( buffer, opt->propName );
	p = strchr ( buffer, '?' );

	if ( ret == NULL ) /* set value */
	{
		if ( gnoclGetBothAlign ( interp, opt->val.obj, &xAlign, &yAlign ) == TCL_OK )
		{
			*p = 'x';
			g_object_set ( obj, buffer, xAlign, NULL );
			*p = 'y';
			g_object_set ( obj, buffer, yAlign, NULL );

			return TCL_OK;
		}
	}

	else /* get value */
	{
		const double eps = 0.00001;
		const char *vert = NULL;
		const char *hor = NULL;
		*p = 'x';
		g_object_get ( obj, buffer, &xAlign, NULL );
		*p = 'y';
		g_object_get ( obj, buffer, &yAlign, NULL );

		if ( fabs ( yAlign ) < eps )
			vert = "top";
		else if ( fabs ( yAlign - 0.5 ) < eps )
			vert = "";
		else if ( fabs ( yAlign - 1.0 ) < eps )
			vert = "bottom";

		if ( vert != NULL )
		{
			if ( fabs ( xAlign ) < eps )
				hor = *vert ? "Left" : "left";
			else if ( fabs ( xAlign - 0.5 ) < eps )
				hor = *vert ? "" : "center";
			else if ( fabs ( xAlign - 1.0 ) < eps )
				hor = *vert ? "Right" : "right";
		}

		if ( vert != NULL  && hor != NULL )
		{
			*ret = Tcl_NewStringObj ( vert, -1 );
			Tcl_AppendToObj ( *ret, hor, -1 );
		}

		else
		{
			*ret = Tcl_NewListObj ( 0, NULL );
			Tcl_ListObjAppendElement ( NULL, *ret, Tcl_NewDoubleObj ( xAlign ) );
			Tcl_ListObjAppendElement ( NULL, *ret, Tcl_NewDoubleObj ( yAlign ) );
		}

		return TCL_OK;
	}

	return TCL_ERROR;
}

/**
\brief
\author
\date
\note
**/
int gnoclOptWidget ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	GtkWidget *widget = NULL;

	if ( ret == NULL ) /* set value */
	{
		const char *str = Tcl_GetString ( opt->val.obj );

		if ( *str != 0 )
		{
			widget = gnoclChildNotPacked ( str, interp );

			if ( widget == NULL )
			{
				return TCL_ERROR;
			}
		}

		g_object_set ( obj, opt->propName, widget, NULL );
	}

	else /* get value */
	{
		g_object_get ( obj, opt->propName, &widget, NULL );

		if ( widget )
		{
			*ret = Tcl_NewStringObj ( gnoclGetNameFromWidget ( widget ), -1 );
		}

		else
		{
			*ret = Tcl_NewStringObj ( "", 0 );
		}
	}

	return TCL_OK;
}

/**
\brief      Returns the name of the objects parent widget.
\author     WJG
\date       06/12/08
\note       Function isn't working properly. Code originally sorted in button.c, transfered to text.c
*/
int gnoclGetParent ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{

	GtkWidget * parent;

	parent = gtk_widget_get_ancestor ( GTK_WIDGET ( obj ), GTK_TYPE_BOX );
	obj = Tcl_NewStringObj ( gnoclGetNameFromWidget ( parent ), -1 );
	Tcl_SetObjResult ( interp, obj );
	return TCL_OK;
}

/**
\brief      Set the specified widget width to the require size in pixels based
            upon the current style of the widget. This is called to set those
            Gtk+ widgets which do not have a default
            "width-chars" property: e.g GtkComboBox.
\author     William J Giddings
\date       18/Feb/09
\see        http://www.gtkforums.com/viewtopic.php?t=3495&highlight=width
*/
int gnoclOptCharWidth ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
#ifdef DEBUG_PARSEOPTIONS
	g_print ( "gnoclOptCharWidth\n" );
#endif
	PangoLayout *layout;
	gint w, h, l;

	gchar text[2] = "X";

	// char *len = Tcl_GetStringFromObj ( opt->val.obj, NULL );
	l = atoi ( Tcl_GetStringFromObj ( opt->val.obj, NULL ) );
#ifdef DEBUG_PARSEOPTIONS
	g_print ( "l = %d\n", l );
#endif

	//layout = gtk_widget_create_pango_layout ( GTK_WIDGET ( obj ) ,text);

	layout = gtk_widget_create_pango_layout ( GTK_WIDGET ( gtk_label_new ( "" ) ), text );

	pango_layout_get_pixel_size ( layout, &w, &h );
	g_object_unref ( layout );

	l = l * w + 30;
#ifdef DEBUG_PARSEOPTIONS
	printf ( "w = %d ; l = %d\n", w );
#endif


	if ( 1 )
	{
		gtk_widget_set_size_request ( GTK_WIDGET ( obj ), l, -1 );
	}

	return TCL_OK;
}

/**
\brief
\author
\date
\note
**/
int gnoclOptHeightRequest ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{

	gint width, height;


	if ( Tcl_GetIntFromObj ( interp, opt->val.obj, &height ) != TCL_OK )
	{
		return TCL_ERROR;
	}

	gtk_widget_set_size_request ( GTK_WIDGET ( obj ), -1, height );


}

/**
\brief
\author
\date
\note
**/
int gnoclOptChild ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	if ( ret == NULL ) /* set value */
	{
		const char *str = Tcl_GetString ( opt->val.obj );

		if ( *str == 0 )
		{
			GtkWidget *widget = gtk_bin_get_child ( GTK_BIN ( obj ) );

			if ( widget != NULL )
			{
				gtk_container_remove ( GTK_CONTAINER ( obj ), widget );
			}
		}

		else
		{
			GtkWidget *widget = gnoclChildNotPacked ( str, interp );

			if ( widget == NULL )
			{
				return TCL_ERROR;
			}

			gtk_container_add ( GTK_CONTAINER ( obj ), widget );
		}

	}

	else /* get value */
	{
		GtkWidget *child = gtk_bin_get_child ( GTK_BIN ( obj ) );

		if ( child )
		{
			*ret = Tcl_NewStringObj ( gnoclGetNameFromWidget ( child ), -1 );
		}

		else
		{
			*ret = Tcl_NewStringObj ( "", 0 );
		}
	}

	return TCL_OK;
}

/**
\brief
\author
\date
\note
**/
int gnoclOptHalign ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	if ( ret == NULL ) /* set value */
	{
		gfloat d;

		if ( optAlign ( interp, opt->val.obj, 1, &d ) != TCL_OK )
			return TCL_ERROR;

		g_object_set ( obj, opt->propName, d, NULL );
	}

	else /* get value */
	{
		const double eps = 0.00001;
		gfloat align;
		g_object_get ( obj, opt->propName, &align, NULL );

		if ( fabs ( align ) < eps )
		{
			*ret = Tcl_NewStringObj ( "left", -1 );
		}

		else if ( fabs ( align - 0.5 ) < eps )
		{
			*ret = Tcl_NewStringObj ( "center", -1 );
		}

		else if ( fabs ( align - 1.0 ) < eps )
		{
			*ret = Tcl_NewStringObj ( "right", -1 );
		}

		else
		{
			*ret = Tcl_NewDoubleObj ( align );
		}
	}

	return TCL_OK;
}

/**
\brief
\author
\date
\note
**/
static void destroyCmdData ( gpointer data, GClosure *closure )
{
	GnoclCommandData *cs = ( GnoclCommandData * ) data;

	g_free ( cs->command );
	g_free ( cs );
}

/**
\brief
\author
\date
\note      07-Dec-09   Added %g option to return glade name as part of mark-up string.
**/
static void doCommand ( GtkWidget *widget, gpointer data )
{
	GnoclCommandData *cs = ( GnoclCommandData * ) data;

	/* if we have set the result, we are in the middle of
	   error handling. In this case don't call any callbacks
	   (especially onDestroy!) because this overrides the result. */

	//if ( *cs->interp->result == '\0' )
	//if ( *Tcl_GetStringResult(cs->interp) == '\0' )
	//{
	GnoclPercSubst ps[] =
	{
		{ 'w', GNOCL_STRING },  /* widget */
		{ 'g', GNOCL_STRING },  /* gladeName */
		{ 0 }
	};

	ps[0].val.str = gnoclGetNameFromWidget ( widget );
	ps[1].val.str = gtk_widget_get_name ( GTK_WIDGET ( widget ) );
	gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 );
	//}
}

/**
\brief
\author
\date
\note
**/
const char *gnoclGetOptCmd ( GObject *obj, const char *signal )
{
	/* there really should be a GTK+ command
	   cs = g_signal_get_data( obj, signal ); */
	return g_object_get_data ( obj, signal );
}

/**
\brief      Destroys all connections for a particular object, with the given
            user-data. Apparently this uses deprecated library functions, see the
            following for more info.
\see        http://library.gnome.org/devel/gtk/stable/gtk-Signals.html
**/
int gnoclDisconnect ( GObject *obj, const char *signal, GCallback handler )
{
	/* we must match func and ID since more than one signal can call
	   the same func if they use the same percent substitutions */
	g_signal_handlers_disconnect_matched ( obj,
										   ( GSignalMatchType ) ( G_SIGNAL_MATCH_FUNC | G_SIGNAL_MATCH_ID ),
										   g_signal_lookup ( signal, G_OBJECT_TYPE ( obj ) ),
										   0, NULL, ( gpointer * ) handler, NULL );
	g_object_set_data ( obj, signal, NULL );

	return 0;
}


/**
\brief
\author
\date
\note       The signals connected here are those of the GtkWidget base class.
\see        See http://library.gnome.org/devel/gtk/stable/GtkWidget.html
\note
            Signals specific to particular wigdets, e.g. textBuffer insert-at-cursor
            need different handlers. These, however, are triggered not by user
            interaction, but under program control. e.g. after a menu operation
            to paste, the relevant signal would be emitted a callback function
            then needs to be added, with a func similar to gnoclConnectOptCmd and
            gnoclConnectSignalCmd. It looks as though gnoclConnectOptCmd will
            connect the signal with a command... -onPaste .... ensure that
            textBuffer pointer and not textView is passed....
**/
int gnoclConnectOptCmd ( Tcl_Interp *interp, GObject *obj, const char *signal, GCallback handler, GnoclOption *opt, void *data, Tcl_Obj **ret )
{
#ifdef DEBUG_PARSEOPTIONS
	g_print ( "%s %s %s\n", __FUNCTION__, gnoclGetNameFromWidget ( GTK_WIDGET ( obj ) ), signal );
#endif

	if ( ret == NULL ) /* set value */
	{
		assert ( opt->type == GNOCL_OBJ );

		if ( opt->status == GNOCL_STATUS_CHANGED )
		{
			const char *cmd = Tcl_GetString ( opt->val.obj );

			gnoclDisconnect ( obj, signal, handler );

			if ( cmd && *cmd )
			{
				/* TODO? remember here already the name of the widget.
				         We then don't have to look it up in every event.
				         Problem: name must be registered before first
				         configure, so gnoclRegisterWidget does not work any more.
				*/

				GnoclCommandData *cs = g_new ( GnoclCommandData, 1 );
				cs->command = g_strdup ( cmd );
				cs->interp = interp;
				cs->data = data;
				g_signal_connect_data ( obj, signal, handler, cs, destroyCmdData, ( GConnectFlags ) 0 );
				g_object_set_data ( obj, signal, ( gpointer ) cs->command );
			}

			else
				g_object_set_data ( obj, signal, NULL );
		}
	}

	else /* get value */
	{
		const char *cmd = gnoclGetOptCmd ( obj, signal );

		if ( cmd )
			*ret = Tcl_NewStringObj ( cmd, -1 );
		else
			*ret = Tcl_NewStringObj ( "", 0 );
	}

	return TCL_OK;
}

/**
\brief This is for all callbacks which have only %w as substitution
**/
int gnoclOptCommand ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	assert ( opt->type == GNOCL_OBJ );

	return gnoclConnectOptCmd ( interp, obj, opt->propName, G_CALLBACK ( doCommand ), opt, NULL, ret );
}


/**
\brief
**/
static void doOnMoveHandle ( GtkWidget *widget, GtkScrollType scroll_type, gpointer user_data )
{
#ifdef DEBUG_PARSEOPTIONS
	g_print ( "%s\n", __FUNCTION__ );
#endif

	// g_print ("%s\n",__FUNCTION__);

	GnoclCommandData *cs = ( GnoclCommandData * ) user_data;

	GnoclPercSubst ps[] =
	{
		{ 'w', GNOCL_STRING },  /* widget */
		{ 'g', GNOCL_STRING },  /* glade name */
		{ 'p', GNOCL_INT}, 		/* handle position */
		{ 0 }
	};

	ps[0].val.str = gnoclGetNameFromWidget ( widget );
	ps[1].val.str = gtk_widget_get_name ( widget );
	ps[2].val.i = gtk_paned_get_position ( widget );

	gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 );
}



/**
\brief React to movement of the paned window handle throught key bindings
**/
int gnoclOptMoveHandle ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	// g_print ("%s\n",__FUNCTION__);
	assert ( strcmp ( opt->optName, "-onHandleMoved" ) == 0 );
	return gnoclConnectOptCmd ( interp, obj, "move-handle", G_CALLBACK ( doOnMoveHandle ), opt, NULL, ret );

}

/**
\brief
**/
int gnoclOptData ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	const char *dataID = "gnocl::data";

	assert ( opt->type == GNOCL_OBJ );

	if ( ret == NULL ) /* set value */
	{
		const char *data = Tcl_GetString ( opt->val.obj );

		if ( data && *data )
		{
			g_object_set_data_full ( obj, dataID, g_strdup ( data ), g_free );
		}

		else
		{
			g_object_set_data ( obj, dataID, NULL );
		}
	}

	else /* get value */
	{
		const char *data = g_object_get_data ( obj, dataID );

		if ( data )
		{
			*ret = Tcl_NewStringObj ( data, -1 );
		}

		else
		{
			*ret = Tcl_NewStringObj ( "", 0 );
		}
	}

	return TCL_OK;
}

/**
\brief
**/
static gboolean doOnScroll ( GtkWidget *widget, GdkEventScroll *event, gpointer data )
{
	GnoclCommandData *cs = ( GnoclCommandData * ) data;

	GnoclPercSubst ps[] =
	{
		{ 'w', GNOCL_STRING },  /* widget */
		{ 't', GNOCL_STRING },  /* event type */
		{ 'x', GNOCL_INT },     /* pointer x-pos in window */
		{ 'y', GNOCL_INT },     /* pointer y-pos in window */
		{ 'X', GNOCL_INT },     /* pointer x-pos on screen */
		{ 'Y', GNOCL_INT },     /* pointer y-pos on screen */
		{ 's', GNOCL_INT },     /* state */
		{ 'd', GNOCL_STRING },  /* direction */
		{ 'g', GNOCL_STRING },  /* widget glade name */
		{ 0 }
	};

	ps[0].val.str = gnoclGetNameFromWidget ( widget );
	ps[1].val.str = "scroll";
	ps[2].val.i = event->x;
	ps[3].val.i = event->y;
	ps[4].val.i = event->x_root;
	ps[5].val.i = event->y_root;
	ps[6].val.i = event->state;
	ps[7].val.str = gtk_widget_get_name ( widget );

	switch ( event->direction )
	{
		case GDK_SCROLL_UP:
			{
				ps[7].val.str = "up";
				break;
			}

		case GDK_SCROLL_DOWN:
			{
				ps[7].val.str = "down";
				break;
			}

		case GDK_SCROLL_LEFT:
			{
				ps[7].val.str = "left";
				break;
			}

		case GDK_SCROLL_RIGHT:
			{
				ps[7].val.str = "right";
				break;
			}
	}


	if ( gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 ) == TCL_OK )
	{
		int del;

		if ( Tcl_GetBooleanFromObj ( NULL, Tcl_GetObjResult ( cs->interp ), &del ) == TCL_OK && del == 0 )
		{
			return TRUE;
		}
	}

	return FALSE;
}

/**
\brief
**/
static gboolean doOnDelete ( GtkWidget *widget, GdkEvent *event, gpointer data )
{
	GnoclCommandData *cs = ( GnoclCommandData * ) data;

	GnoclPercSubst ps[] =
	{
		{ 'w', GNOCL_STRING },  /* widget */
		{ 'g', GNOCL_STRING },  /* glade name */
		{ 0 }
	};

	ps[0].val.str = gnoclGetNameFromWidget ( widget );
	ps[1].val.str = gtk_widget_get_name ( widget );

	if ( gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 ) == TCL_OK )
	{
		int del;

		if ( Tcl_GetBooleanFromObj ( NULL, Tcl_GetObjResult ( cs->interp ), &del ) == TCL_OK && del == 0 )
		{
			return TRUE;  /* ignore delete */
		}
	}

	return FALSE; /* proceed: destroy widget */
}

/**
\brief
\note       based upon doOnDelete
**/
static gboolean doOnExpose ( GtkWidget *widget, GdkEvent *event, gpointer data )
{
	GnoclCommandData *cs = ( GnoclCommandData * ) data;

	GnoclPercSubst ps[] =
	{
		{ 'w', GNOCL_STRING },  /* widget */
		{ 'g', GNOCL_STRING },  /* glade name */
		{ 0 }
	};

	ps[0].val.str = gnoclGetNameFromWidget ( widget );
	ps[1].val.str = gtk_widget_get_name ( widget );

	if ( gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 ) == TCL_OK )
	{
		int del;

		if ( Tcl_GetBooleanFromObj ( NULL, Tcl_GetObjResult ( cs->interp ), &del ) == TCL_OK && del == 0 )
		{
			return TRUE;  /* ignore delete */
		}
	}

	return FALSE; /* proceed: destroy widget */
}

/**
\brief      Respond to expose-event signal.
\note       Developed for use with gnocl::openGL
**/
int gnoclOptOnExpose ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	assert ( strcmp ( opt->optName, "-onExpose" ) == 0 );
	return gnoclConnectOptCmd ( interp, obj, "expose-event", G_CALLBACK ( doOnExpose ), opt, NULL, ret );
}


/**
\brief
**/
int gnoclOptOnDelete ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	assert ( strcmp ( opt->optName, "-onDelete" ) == 0 );
	return gnoclConnectOptCmd ( interp, obj, "delete-event", G_CALLBACK ( doOnDelete ), opt, NULL, ret );
}

/**
\brief
**/
static void doOnQueryToolTip ( GtkWidget *widget, gint x, gint y, gboolean keyboard_mode, GtkTooltip *tooltip, gpointer user_data )
{
#ifdef DEBUG_PARSEOPTIONS
	g_print ( "%s\n", __FUNCTION__ );
#endif

	GnoclCommandData *cs = ( GnoclCommandData * ) user_data;
	GtkTextIter *iter;

	//GtkTextWindowType win;
	gint buffer_x;
	gint buffer_y;

	//gtk_text_view_window_to_buffer_coords ( GTK_TEXT_VIEW  ( widget ), GTK_TEXT_WINDOW_WIDGET, x, y, &buffer_x, &buffer_y );

	//gtk_text_view_get_iter_at_location  ( GTK_TEXT_VIEW  ( widget ), iter, buffer_x, buffer_y );

	GnoclPercSubst ps[] =
	{
		{ 'w', GNOCL_STRING },  /* widget */
		{ 'g', GNOCL_STRING },  /* glade name */
		{ 'x', GNOCL_INT },
		{ 'y', GNOCL_INT },
		{ 'm', GNOCL_INT },
		{ 'r', GNOCL_INT },
		{ 'c', GNOCL_INT },
		{ 0 }
	};

	ps[0].val.str = gnoclGetNameFromWidget ( widget );
	ps[1].val.str = gtk_widget_get_name ( widget );
	ps[2].val.i = x;
	ps[3].val.i = y;
	ps[4].val.i = keyboard_mode;

	//ps[5].val.i = gtk_text_iter_get_line ( iter );
	//ps[6].val.i = gtk_text_iter_get_line_offset ( iter );


	gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 );
}


/**
\brief
**/
int gnoclOptOnQueryToolTip ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	assert ( strcmp ( opt->optName, "-onQueryTooltip" ) == 0 );
	return gnoclConnectOptCmd ( interp, obj, "query-tooltip", G_CALLBACK ( doOnQueryToolTip ), opt, NULL, ret );
}

/**
\brief
**/
static void doOnShowHelp ( GtkWidget *widget, GtkWidgetHelpType arg1, gpointer data )
{
	GnoclCommandData *cs = ( GnoclCommandData * ) data;

	GnoclPercSubst ps[] =
	{
		{ 'w', GNOCL_STRING },  /* widget */
		{ 'h', GNOCL_STRING },
		{ 'g', GNOCL_STRING },  /* glade name */
		{ 0 }
	};

	ps[0].val.str = gnoclGetNameFromWidget ( widget );
	ps[1].val.str = ( arg1 == GTK_WIDGET_HELP_TOOLTIP ? "tooltip" : "whatsThis" );
	ps[2].val.str = gtk_widget_get_name ( widget );

	gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 );
}

/**
\brief
**/
int gnoclOptOnShowHelp ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	assert ( strcmp ( opt->optName, "-onShowHelp" ) == 0 );
	return gnoclConnectOptCmd ( interp, obj, "show-help", G_CALLBACK ( doOnShowHelp ), opt, NULL, ret );
}

/**
\brief
\todo    gnocl::buttonStateToList -> {MOD1 MOD3 BUTTON2...}
**/
static void doOnMotion ( GtkWidget *widget, GdkEventMotion *event, gpointer data )
{
	GnoclCommandData *cs = ( GnoclCommandData * ) data;

	GnoclPercSubst ps[] =
	{
		{ 'w', GNOCL_STRING },  /* widget */
		{ 'x', GNOCL_INT },
		{ 'y', GNOCL_INT },
		{ 'X', GNOCL_INT },
		{ 'Y', GNOCL_INT },
		{ 's', GNOCL_INT },
		{ 'g', GNOCL_STRING },  /* glade name */

		/* textview specific options */
		{ 'r', GNOCL_INT },
		{ 'c', GNOCL_INT },
		{ 0 }
	};

	ps[0].val.str = gnoclGetNameFromWidget ( widget );
	ps[1].val.i = event->x;
	ps[2].val.i = event->y;
	ps[3].val.i = event->x_root;
	ps[4].val.i = event->y_root;
	ps[5].val.i = event->state;
	ps[6].val.str = gtk_widget_get_name ( widget );
	ps[7].val.i = -1;
	ps[8].val.i = -1;

	gchar *type =  G_OBJECT_TYPE_NAME ( widget );

	//g_print ( "Widget type: %s\n", type );

	if ( strcmp ( type, "GtkUndoView" ) == 0 || strcmp ( type, "GtkTextView" ) == 0 )
	{
		GtkTextIter iter;
		gint bx, by; /* buffer coordinates */
		gint line, row;
		gtk_text_view_window_to_buffer_coords ( widget, GTK_TEXT_WINDOW_WIDGET, event->x, event->y, &bx, &by );
		gtk_text_view_get_iter_at_location ( widget, &iter, bx, by );
		ps[7].val.i = gtk_text_iter_get_line ( &iter );
		ps[8].val.i = gtk_text_iter_get_line_offset ( &iter );
	}

	/* TODO: gnocl::buttonStateToList -> {MOD1 MOD3 BUTTON2...} */
	gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 );
}

/**
\brief      Handler function for a GtkTextView "backspace" signal.
\note
**/
static void doOnBackSpace ( GtkWidget *widget, gpointer data )
{

	GnoclCommandData *cs = ( GnoclCommandData * ) data;

	GnoclPercSubst ps[] =
	{
		{ 'w', GNOCL_STRING },  /* widget */
		{ 'g', GNOCL_STRING },  /* glade name */
		{ 'e', GNOCL_STRING },  /* signal/event name */
		{ 0 }
	};

	ps[0].val.str = gnoclGetNameFromWidget ( widget );
	ps[1].val.str = gtk_widget_get_name ( widget );
	ps[2].val.str = "backspace";

	gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 );

}
/**
\brief      Handler function for a GtkEntry "preedit-changed" signal.
\note
**/
static void doOnPreEditChanged ( GtkWidget *widget, gchar *preedit, gpointer data )
{

	GnoclCommandData *cs = ( GnoclCommandData * ) data;

	GnoclPercSubst ps[] =
	{
		{ 'w', GNOCL_STRING },  /* widget */
		{ 'g', GNOCL_STRING },  /* glade name */
		{ 't', GNOCL_STRING },  /* signal/event name */
		{ 0 }
	};

	ps[0].val.str = gnoclGetNameFromWidget ( widget );
	ps[1].val.str = gtk_widget_get_name ( widget );
	ps[2].val.str = preedit;

	gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 );

}
/**
\brief      Connect GtkTextView "backspace" event to its handler.
\note
**/
int gnoclOptOnBackSpace ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	assert ( strcmp ( opt->optName, "-onBackSpace" ) == 0 );
	return gnoclConnectOptCmd ( interp, obj, "backspace", G_CALLBACK ( doOnBackSpace ), opt, NULL, ret );
}

/**
\brief
\author     William J Giddings
\date       15/Jan/2010
\note
\todo       Next step: How to get additional details from a signal?
            Substituation values would be:
                %w   widget name
                %t   type of event
                %s   state of modifiers
**/
static int doOnSelectAll ( GtkWidget *widget, gboolean select, gpointer data )
{

	GnoclCommandData *cs = ( GnoclCommandData * ) data;

	GnoclPercSubst ps[] =
	{
		{ 'w', GNOCL_STRING },  /* widget */
		{ 'g', GNOCL_STRING },  /* glade name */
		{ 's', GNOCL_INT },     /* either selected, or deselected */
		{ 'e', GNOCL_STRING },  /* signal/event name */
		{ 0 }
	};

	ps[0].val.str = gnoclGetNameFromWidget ( widget );
	ps[1].val.str = gtk_widget_get_name ( widget );
	ps[2].val.i = select;
	ps[3].val.str = "select-all";

	gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 );

}

/**
\brief
**/
int gnoclOptOnSelectAll ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
#ifdef DEBUG_PARSEOPTIONS
	g_print ( "gnoclOptOnSelectAll\n" );
#endif

	assert ( strcmp ( opt->optName, "-onSelectAll" ) == 0 );

	return gnoclConnectOptCmd ( interp, obj, "select-all", G_CALLBACK ( doOnSelectAll ), opt, NULL, ret );
}


/**
\brief
**/
static gboolean doOnDeleteFromCursor ( GtkTextView *text_view, GtkDeleteType type, gint count, gpointer user_data )
{
	GnoclCommandData *cs = ( GnoclCommandData * ) user_data;

	GnoclPercSubst ps[] =
	{
		{ 'w', GNOCL_STRING },  /* widget */
		{ 'g', GNOCL_STRING },  /* glade name */
		{ 'n', GNOCL_INT }, 	/* the number of type units to delete */
		{ 't', GNOCL_STRING }, 	/* the delete type, int */
		{ 'T', GNOCL_STRING }, 	/* the delete type, string */
		{ 0 }
	};

	ps[0].val.str = gnoclGetNameFromWidget ( text_view );
	ps[1].val.str = gtk_widget_get_name ( GTK_WIDGET ( text_view ) );
	ps[2].val.i = count;
	ps[3].val.i = type;

	switch ( type )
	{
		case GTK_DELETE_CHARS:
			ps[4].val.str = "chars";
			break;

		case GTK_DELETE_WORD_ENDS:
			ps[4].val.str = "word-ends";
			break;

		case GTK_DELETE_WORDS:
			ps[4].val.str = "words";
			break;

		case GTK_DELETE_DISPLAY_LINES:
			ps[4].val.str = "display-lines";
			break;

		case GTK_DELETE_PARAGRAPH_ENDS:
			ps[4].val.str = "paragraph-ends";
			break;

		case GTK_DELETE_PARAGRAPHS:
			ps[4].val.str = "paragraphs";
			break;

		case GTK_DELETE_WHITESPACE:
			ps[4].val.str = "whitespace";
			break;

		default: {}
	}

	if ( gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 ) == TCL_OK )
	{
		int del;

		if ( Tcl_GetBooleanFromObj ( NULL, Tcl_GetObjResult ( cs->interp ), &del ) == TCL_OK && del == 0 )
		{
			return TRUE;  /* ignore delete */
		}
	}

	return FALSE; /* proceed: destroy widget */
}


/**
\author     William J Giddings
\date       02/06/08
\note      Handle entry delete
**/
int gnoclOptOnDeleteFromCursor ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
#ifdef DEBUG_PARSEOPTIONS
	g_print ( "gnoclOptOnDeleteFromCursor\n" );
#endif

	assert ( strcmp ( opt->optName, "-onDeleteFromCursor" ) == 0 );

	return gnoclConnectOptCmd ( interp, obj, "delete-from-cursor", G_CALLBACK ( doOnDeleteFromCursor ), opt, NULL, ret );
}


/**
\brief
\author     William J Giddings
\date       15/Jan/2010
\note		Used by both text, entry and treeview widgets
\todo


  GTK_MOVEMENT_LOGICAL_POSITIONS, move by forw/back graphemes
  GTK_MOVEMENT_VISUAL_POSITIONS,  move by left/right graphemes
  GTK_MOVEMENT_WORDS,             move by forward/back words
  GTK_MOVEMENT_DISPLAY_LINES,     move up/down lines (wrapped lines)
  GTK_MOVEMENT_DISPLAY_LINE_ENDS, move up/down lines (wrapped lines)
  GTK_MOVEMENT_PARAGRAPHS,        move up/down paragraphs (newline-ended lines)
  GTK_MOVEMENT_PARAGRAPH_ENDS,    move to either end of a paragraph
  GTK_MOVEMENT_PAGES,	          move by pages
  GTK_MOVEMENT_BUFFER_ENDS,       move to ends of the buffer
  GTK_MOVEMENT_HORIZONTAL_PAGES   move horizontally by pages

**/
static int doOnMoveCursor ( GtkWidget *widget, GtkMovementStep step, gint count, gint extend_selection, gpointer data )
{

	GnoclCommandData *cs = ( GnoclCommandData * ) data;

	GnoclPercSubst ps[] =
	{

		{ 'w', GNOCL_STRING },  /* widget */
		{ 'g', GNOCL_STRING },  /* glade name */
		{ 'c', GNOCL_INT},		/* count */
		{ 's', GNOCL_STRING },  /* step */
		{ 'x', GNOCL_BOOL},		/* extended selection */
		{ 0 }
	};

	ps[0].val.str = gnoclGetNameFromWidget ( widget );
	ps[1].val.str = gtk_widget_get_name ( widget );

	switch ( step )
	{
		case GTK_MOVEMENT_LOGICAL_POSITIONS:
			{
				//move by forw/back graphemes
				ps[2].val.str = "gf/b";
			}
			break;

		case GTK_MOVEMENT_VISUAL_POSITIONS:
			{
				//move by left/right graphemes
				ps[2].val.str = "gl/r";
			}
			break;


		case GTK_MOVEMENT_WORDS:
			{
				//move by forward/back words
				ps[2].val.str = "wf/b";
			}
			break;

		case GTK_MOVEMENT_DISPLAY_LINES:
			{
				//move up/down lines (wrapped lines)
				ps[2].val.str = "dlu/d";
			}
			break;

		case GTK_MOVEMENT_DISPLAY_LINE_ENDS:
			{
				//move up/down lines (wrapped lines)
				ps[2].val.str = "dleu/d";
			}
			break;

		case GTK_MOVEMENT_PARAGRAPHS:
			{
				//move up/down paragraphs (newline-ended lines)
				ps[2].val.str = "pu/d";
			}
			break;

		case GTK_MOVEMENT_PARAGRAPH_ENDS:
			{
				//move to either end of a paragraph
				ps[2].val.str = "pend";
			}
			break;

		case GTK_MOVEMENT_PAGES:
			{
				//move by pages
				ps[2].val.str = "p";
			}
			break;
		case GTK_MOVEMENT_BUFFER_ENDS:
			{
				//move to ends of the buffer
				ps[2].val.str = "eob";
			}
			break;
		case GTK_MOVEMENT_HORIZONTAL_PAGES:
			{
				//move horizontally by pages
				ps[2].val.str = "hp";
			}
			break;

		default: {}
	}


	ps[3].val.i = count;
	ps[4].val.i = extend_selection;

	gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 );

}

/**
\author     William J Giddings
\date       02/06/08
\note		Used by both text, entry and treeview widgets, other widgets will have their own handler, e.g. iconview.
**/
int gnoclOptOnMoveCursor ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{

	return gnoclConnectOptCmd ( interp, obj,   "move-cursor" , G_CALLBACK ( doOnMoveCursor ), opt, NULL, ret );
}

/**
\brief
\author     William J Giddings
\date       15/Jan/2010
\note		Used by both text and entry widgets
\todo


  GTK_SCROLL_STEPS,
  GTK_SCROLL_PAGES,
  GTK_SCROLL_ENDS,
  GTK_SCROLL_HORIZONTAL_STEPS,
  GTK_SCROLL_HORIZONTAL_PAGES,
  GTK_SCROLL_HORIZONTAL_ENDS


**/
static int doOnMoveViewport ( GtkTextView  *text_view, GtkScrollStep step, gint count, gpointer user_data )
{

	GnoclCommandData *cs = ( GnoclCommandData * ) user_data;

	GnoclPercSubst ps[] =
	{

		{ 'w', GNOCL_STRING },  /* widget */
		{ 'g', GNOCL_STRING },  /* glade name */
		{ 'c', GNOCL_INT},		/* count */
		{ 't', GNOCL_INT },  	/* step, int */
		{ 'T', GNOCL_STRING},  	/* step, string */
		{ 0 }
	};

	ps[0].val.str = gnoclGetNameFromWidget ( text_view );
	ps[1].val.str = gtk_widget_get_name ( GTK_WIDGET ( text_view ) );
	ps[2].val.i = count;
	ps[3].val.i = step;

	switch ( step )
	{
		case GTK_SCROLL_STEPS:
			ps[4].val.str = "steps";
			break;

		case GTK_SCROLL_PAGES:
			ps[4].val.str = "pages";
			break;

		case GTK_SCROLL_ENDS:
			ps[4].val.str = "scroll-ends";
			break;

		case GTK_SCROLL_HORIZONTAL_STEPS:
			ps[4].val.str = "horizontal-steps";
			break;

		case GTK_SCROLL_HORIZONTAL_PAGES:
			ps[4].val.str = "horizontal-pages";
			break;

		case GTK_SCROLL_HORIZONTAL_ENDS:
			ps[4].val.str = "horizontal-ends";
			break;

		default: {}
	}


	gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 );

}

/**
\author     William J Giddings
\date       02/06/08
\note
**/
int gnoclOptOnMoveViewport ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	assert ( strcmp ( opt->optName, "-onMoveViewport" ) == 0 );

	return gnoclConnectOptCmd ( interp, obj,   "move-viewport" , G_CALLBACK ( doOnMoveViewport ), opt, NULL, ret );
}

/**
\author     William J Giddings
\date       02/06/08
\note      ????
**/
int gnoclOptOnPageHorizontally ( Tcl_Interp *interp, GnoclOption *opt,
								 GObject *obj, Tcl_Obj **ret )
{
	assert ( strcmp ( opt->optName, "-onPageHorizontally" ) == 0 );

	char warmMsg[] = "WARNING: Text widget option -onPageHorizontally."
					 "This signal should not be used anymore, instead use the \"move-cursor\""
					 "signal with the GTK_MOVEMENT_HORIZONTAL_PAGES granularity.";

	Tcl_SetResult ( interp, warmMsg, TCL_STATIC );
	return TCL_OK;
}

/**
\brief     Handles the 'set-anchor' signal.
\author    William J Giddings
\date      30/04/2010
\since     0.9.95
\note      Used by: gnome::text
\**/
static void doOnSetAnchor ( GtkTextView *text_view, gpointer user_data )
{

	GnoclCommandData *cs = ( GnoclCommandData * ) user_data;
	GtkTextIter iter;
	GtkTextBuffer *buffer;
	gint row, col;

	GnoclPercSubst ps[] =
	{
		{ 'w', GNOCL_STRING },  /* widget name */
		{ 'g', GNOCL_STRING },  /* glade name */
		{ 'r', GNOCL_INT},		/* row */
		{ 'c', GNOCL_INT},		/* col */
		{ 0 }
	};

	ps[0].val.str = gnoclGetNameFromWidget ( text_view );
	ps[1].val.str = gtk_widget_get_name ( GTK_WIDGET ( text_view ) );

	/* convenience call, get the position of the insertion */
	buffer = gtk_text_view_get_buffer ( text_view );
	gtk_text_buffer_get_iter_at_mark ( buffer, &iter, gtk_text_buffer_get_insert ( buffer ) );
	row = gtk_text_iter_get_line ( &iter );
	col = gtk_text_iter_get_line_offset ( &iter );

	ps[2].val.i = row;
	ps[3].val.i = col;

	gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 );
}

/**
\author     William J Giddings
\date       02/06/08
\note      ????
**/
int gnoclOptOnSetAnchor ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )  /* stub */
{
	/* check the name of the signal is correct for this function */
	assert ( strcmp ( opt->optName, "-onSetAnchor" ) == 0 );

	/* connect the signal with its callback function */
	return gnoclConnectOptCmd ( interp, obj, "set-anchor", G_CALLBACK ( doOnSetAnchor ), opt, NULL, ret );
}


/**
\brief     Handles the 'set-anchor' signal.
\author    William J Giddings
\date      30/04/2010
\since     0.9.95
\note      Used by: gnome::text
\todo		NEEDS COMPLETION
\**/
static void doOnScrollAdjustments ( GtkTextView *horizontal, GtkAdjustment *vertical, GtkAdjustment *arg2, gpointer user_data )
{

	GnoclCommandData *cs = ( GnoclCommandData * ) user_data;
	GtkTextIter iter;
	GtkTextBuffer *buffer;
	gint row, col;

	GnoclPercSubst ps[] =
	{
		{ 'w', GNOCL_STRING },  /* widget name */
		{ 'g', GNOCL_STRING },  /* glade name */
		{ 0 }
	};

	ps[0].val.str = gnoclGetNameFromWidget ( horizontal );
	ps[1].val.str = gtk_widget_get_name ( GTK_WIDGET ( horizontal ) );


	gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 );
}


/**
\brief
\author		William J Giddings
\date		30/04/2010
\note
**/
int gnoclOptOnScrollAdjustments ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	/* check the name of the signal is correct for this function */
	assert ( strcmp ( opt->optName, "-onSetScrollAdjustments" ) == 0 );

	/* connect the signal with its callback function */
	return gnoclConnectOptCmd ( interp, obj, "set-scroll-adjustments" , G_CALLBACK ( doOnScrollAdjustments ), opt, NULL, ret );
}


/**
\brief     Handles the "toggle-overwrite" signal.
\author    William J Giddings
\date      30/04/2010
\since     0.9.95
\note      Used by: gnome::text
\**/
static void doOnToggleOverWrite ( GtkTextView *text_view, gpointer user_data )
{

	GnoclCommandData *cs = ( GnoclCommandData * ) user_data;

	GnoclPercSubst ps[] =
	{
		{ 'w', GNOCL_STRING },  /* widget name */
		{ 'g', GNOCL_STRING },  /* glade name */
		{ 's', GNOCL_INT }, 	/* overwritable?, boolean*/
		{ 0 }
	};

	ps[0].val.str = gnoclGetNameFromWidget ( text_view );
	ps[1].val.str = gtk_widget_get_name ( GTK_WIDGET ( text_view ) );
	ps[1].val.str = gtk_text_view_get_overwrite ( text_view );

	gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 );
}

/**
\brief
\author
\date
\note
**/
int gnoclOptOnToggleOverwrite ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	/* check the name of the signal is correct for this function */
	assert ( strcmp ( opt->optName, "-onToggleOverWrite" ) == 0 );

	/* connect the signal with its callback function */
	return gnoclConnectOptCmd ( interp, obj, "toggle-overwrite" , G_CALLBACK ( doOnToggleOverWrite ), opt, NULL, ret );
}

/**
\brief
\author
\date
\note
**/
int gnoclOptOnPreEditChanged ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	//return TCL_OK;
	assert ( strcmp ( opt->optName, "-onPreeditChanged" ) == 0 );
	return gnoclConnectOptCmd ( interp, obj,  "preedit-changed" , G_CALLBACK ( doOnPreEditChanged ), opt, NULL, ret );
}
/**
\brief
\author
\date
\note
**/
int gnoclOptOnBackspace ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	//return TCL_OK;
	assert ( strcmp ( opt->optName, "-onBackspace" ) == 0 );
	return gnoclConnectOptCmd ( interp, obj,  "backspace" , G_CALLBACK ( doOnBackSpace ), opt, NULL, ret );
}

/**
\brief
**/
static void doOnNotify ( GObject *gobject, GParamSpec *pspec, gpointer user_data )
{
#ifdef DEBUG_PARSEOPTIONS
	g_print ( "%s\n", __FUNCTION__ );
#endif

	GnoclCommandData *cs = ( GnoclCommandData * ) user_data;

	GnoclPercSubst ps[] =
	{
		{ 'w', GNOCL_STRING },  /* widget name */
		{ 'g', GNOCL_STRING },  /* glade name */

		{ 0 }
	};

	GtkWidget * treeView;

	treeView = gtk_tree_view_column_get_tree_view  ( GTK_TREE_VIEW_COLUMN ( gobject ) );

	ps[0].val.str = gnoclGetNameFromWidget ( GTK_WIDGET ( treeView ) );
	ps[1].val.str = gtk_widget_get_name ( GTK_WIDGET ( treeView ) );

	gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 );
}

/**
\brief	Implement notification event for object property changes
**/
int gnoclOptNotify ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
#ifdef DEBUG_PARSEOPTIONS
	g_print ( "%s\n", __FUNCTION__ );
#endif

	return gnoclConnectOptCmd ( interp, obj,  "notify", G_CALLBACK ( doOnNotify ), opt, NULL, ret );

}

/**
\brief
**/
int gnoclWinfoNotify ( Tcl_Interp *interp, GObject *obj, gchar *command )
{

	gnoclDisconnect ( obj, "notify", G_CALLBACK ( doOnNotify ) );

	command = g_strdup ( command );

	GnoclCommandData *cs = g_new ( GnoclCommandData, 1 );
	cs->command = g_strdup ( command );
	cs->interp = interp;

	g_signal_connect_data ( obj, "notify", G_CALLBACK ( doOnNotify ), cs, destroyCmdData, ( GConnectFlags ) 0 );
	g_object_set_data ( obj, signal, ( gpointer ) cs->command );

	return TCL_OK;

}


/**
\brief      Enable motion events for during any button press
\author     William J Giddings
\date       18/Feb/2010
\note
**/
int gnoclOptOnButtonMotion ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	assert ( strcmp ( opt->optName, "-onButtonMotion" ) == 0 );
	return gnoclConnectOptCmd ( interp, obj, "motion-notify-event", G_CALLBACK ( doOnMotion ), opt, NULL, ret );
}

/**
\brief
\author
\date
\note
**/
int gnoclOptOnMotion ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	assert ( strcmp ( opt->optName, "-onMotion" ) == 0 || strcmp ( opt->optName, "-onPointerMotion" ) == 0 );
	return gnoclConnectOptCmd ( interp, obj, "motion-notify-event", G_CALLBACK ( doOnMotion ), opt, NULL, ret );
}

/**
\brief      Yet to be annotated
\author     William J Giddings
\date       10/05/09
**/
static gboolean doOnMoveFocus  ( GtkWidget *widget, int direction, gpointer data )
{

	GnoclCommandData *cs = ( GnoclCommandData * ) data;

	GnoclPercSubst ps[] =
	{
		{ 'w', GNOCL_STRING },  /* widget */
		{ 'd', GNOCL_STRING },  /* widget */
		{ 'g', GNOCL_STRING },  /* glade name */
		{ 0 }
	};

	gchar *str;

	switch ( direction )
	{
		case GTK_DIR_TAB_FORWARD:
			{
				sprintf ( str, "tab-forward" );
				break;
			}

		case GTK_DIR_TAB_BACKWARD:
			{
				sprintf ( str, "tab-backward" );
				break;
			}

		case GTK_DIR_UP:
			{
				sprintf ( str, "dir-up" );
				break;
			}

		case GTK_DIR_DOWN:
			{
				sprintf ( str, "dir-down" );
				break;
			}

		case GTK_DIR_LEFT:
			{
				sprintf ( str, "dir-left" );
				break;
			}

		case GTK_DIR_RIGHT:
			{
				sprintf ( str, "dir-right" );
				break;
			}
	}

	ps[0].val.str = gnoclGetNameFromWidget ( widget );
	ps[1].val.str = str;
	ps[2].val.str = gtk_widget_get_name ( GTK_WIDGET ( widget ) );

	gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 );
	return 0;
}

/**
\brief      Yet to be annotated
\author     William J Giddings
\date       10/05/09
**/
int gnoclOptOnMoveFocus ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	assert ( strcmp ( opt->optName, "-onMoveFocus" ) == 0 );
	return gnoclConnectOptCmd ( interp, obj, "move-focus", G_CALLBACK ( doOnMoveFocus ), opt, NULL, ret );
}

static gboolean doOnEnterLeave ( GtkWidget *widget, GdkEventCrossing *event, gpointer data )
{

	/*
	typedef enum
	{
	  GTK_DIR_TAB_FORWARD,
	  GTK_DIR_TAB_BACKWARD,
	  GTK_DIR_UP,
	  GTK_DIR_DOWN,
	  GTK_DIR_LEFT,
	  GTK_DIR_RIGHT
	} GtkDirectionType;
	*/

	GnoclCommandData *cs = ( GnoclCommandData * ) data;

	GnoclPercSubst ps[] =
	{
		{ 'w', GNOCL_STRING },  /* widget */
		{ 'x', GNOCL_INT },
		{ 'y', GNOCL_INT },
		{ 'X', GNOCL_INT },
		{ 'Y', GNOCL_INT },
		{ 's', GNOCL_INT },
		{ 'g', GNOCL_STRING },  /* glade name */
		{ 0 }
	};


	ps[0].val.str = gnoclGetNameFromWidget ( widget );
	ps[1].val.i = event->x;
	ps[2].val.i = event->y;
	ps[3].val.i = event->x_root;
	ps[4].val.i = event->y_root;
	ps[5].val.i = event->state;
	ps[6].val.str = gtk_widget_get_name ( GTK_WIDGET ( widget ) );

	/* TODO: gnocl::buttonStateToList -> {MOD1 MOD3 BUTTON2...} */
	gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 );
	return 0;
}

/**
\brief      Callback function to focus change event.
\author     William J Giddings
\date       15/10/08
**/
static gboolean doOnFocus ( GtkWidget *widget, GdkEventFocus *event, gpointer data )
{

	GnoclCommandData *cs = ( GnoclCommandData * ) data;

	GnoclPercSubst ps[] =
	{
		{ 't', GNOCL_STRING },  /* toplevel */
		{ 'p', GNOCL_STRING },  /* parent */
		{ 'w', GNOCL_STRING },  /* widget */
		{ 'g', GNOCL_STRING },  /* glade name */
		{ 0 }
	};

	ps[0].val.str = gnoclGetNameFromWidget ( gtk_widget_get_toplevel ( widget ) );
	ps[1].val.str = gnoclGetNameFromWidget ( gtk_widget_get_parent ( widget ) );
	ps[2].val.str = gnoclGetNameFromWidget ( widget );
	ps[3].val.str = gtk_widget_get_name ( GTK_WIDGET ( widget ) );

	gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 );
	return 0;
}

/**
\brief
\author
\date
\note
**/
int gnoclOptOnFocus ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{

	switch  ( *opt->propName )
	{
		case 'I':
			{

				return gnoclConnectOptCmd ( interp, obj, "focus-in-event", G_CALLBACK ( doOnFocus ), opt, NULL, ret );
				break;
			}

		case 'O':
			{
				return gnoclConnectOptCmd ( interp, obj, "focus-out-event", G_CALLBACK ( doOnFocus ), opt, NULL, ret );
				break;
			}

		default:
			{
				// assert( 0 );
				return TCL_ERROR;
			}
	}
}

/*
int gnoclOptOnFocus ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
    assert ( *opt->propName == 'I' || *opt->propName == 'O' );
    return gnoclConnectOptCmd ( interp, obj, *opt->propName == 'I' ?  "focus-in-event" : "focus-out-event", G_CALLBACK ( doOnFocus ), opt, NULL, ret );
}
*/

/**
\brief
\author
\date
\note
**/
int gnoclOptOnEnterLeave ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	assert ( strcmp ( opt->optName, "-onEnter" ) == 0 || strcmp ( opt->optName, "-onLeave" ) == 0 );
	return gnoclConnectOptCmd ( interp, obj, opt->optName[3] == 'E' ? "enter-notify-event" : "leave-notify-event", G_CALLBACK ( doOnEnterLeave ), opt, NULL, ret );
}

/**
\brief
\author     William J Giddings
\date       10/05/09
**/
int gnoclOptOnProximityInOut ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	assert ( strcmp ( opt->optName, "-onProximityIn" ) == 0 || strcmp ( opt->optName, "-onProximityOut" ) == 0 );
	return gnoclConnectOptCmd ( interp, obj, opt->optName[3] == 'I' ? "proximity-in-event" : "proximity-out-event", G_CALLBACK ( doOnEnterLeave ), opt, NULL, ret );
}




/**
\brief	callback handler for GTK_BUTTON widgets
**/
static void doOnClicked   ( GtkWidget *button, gpointer user_data )
{
#ifdef DEBUG_PARSEOPTIONS
	g_print ( "%s\n", __FUNCTION__ );
#endif
	GnoclCommandData *cs = ( GnoclCommandData * ) user_data;

	GnoclPercSubst ps[] =
	{
		{ 'w', GNOCL_STRING },  /* widget */
		{ 'p', GNOCL_STRING },  /* parent */
		{ 'g', GNOCL_STRING },  /* glade name */
		{ 'd', GNOCL_STRING },  /* data */
		{ 0 }
	};

	const char *dataID = "gnocl::data";
	ButtonParams *para = g_object_get_data ( G_OBJECT ( button ), dataID );

	ps[0].val.str = gnoclGetNameFromWidget ( button );
	ps[1].val.str = gnoclGetNameFromWidget ( gtk_widget_get_parent ( button ) );
	ps[2].val.str = gtk_widget_get_name ( button );
	ps[3].val.str = g_object_get_data ( button, "gnocl::data" );


	gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 );

}

/**
\brief
	widget button, *** NOT ** mouse button
**/
static void doOnToolButtonClicked ( GtkToolButton *toolbutton, gpointer user_data )
{
#ifdef DEBUG_PARSEOPTIONS
	g_print ( "%s\n", __FUNCTION__ );
#endif
	GnoclCommandData *cs = ( GnoclCommandData * ) user_data;

	GnoclPercSubst ps[] =
	{
		{ 'w', GNOCL_STRING },  /* widget */
		{ 'g', GNOCL_STRING },  /* glade name */
		{ 'p', GNOCL_STRING },  /* parent */
		{ 'd', GNOCL_STRING },  /* data */
		{ 0 }
	};

	const char *dataID = "gnocl::para";
	ButtonParams *para = g_object_get_data ( G_OBJECT ( toolbutton ), dataID );

	ps[0].val.str = gnoclGetNameFromWidget ( toolbutton );
	ps[1].val.str = gtk_widget_get_name ( GTK_WIDGET ( toolbutton ) );
	ps[2].val.str = gtk_widget_get_parent ( GTK_WIDGET ( toolbutton ) );

	ps[3].val.str = para->data;

	gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 );

}


/**
\brief
**/
static void doOnInteractiveSearch   ( GtkTreeView *treeview, gpointer user_data )
{
#ifdef DEBUG_PARSEOPTIONS
	g_print ( "%s\n", __FUNCTION__ );
#endif
	GnoclCommandData *cs = ( GnoclCommandData * ) user_data;

	GtkEntry *entry = gtk_tree_view_get_search_entry ( treeview );


	GnoclPercSubst ps[] =
	{
		{ 'w', GNOCL_STRING },  /* widget */
		{ 'g', GNOCL_STRING },  /* glade name */
		{ 'c', GNOCL_INT},  	/* search column */
		{ 'e', GNOCL_STRING },  /* search entry widget */
		{ 0 }
	};



	ps[0].val.str = gnoclGetNameFromWidget ( treeview );
	ps[1].val.str = gtk_widget_get_name ( GTK_WIDGET ( treeview ) );
	ps[2].val.i = gtk_tree_view_get_search_column ( treeview );;

	if ( entry != NULL )
	{
		ps[3].val.str = gnoclGetNameFromWidget ( entry );
	}

	gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 );

}

/**
\brief
**/
static void doOnColumnClicked ( GtkTreeViewColumn *treeviewcolumn, gpointer user_data )
{
#ifdef DEBUG_PARSEOPTIONS
	g_print ( "%s\n", __FUNCTION__ );
#endif

	GtkWidget *tree;
	gint col;

	tree = gtk_tree_view_column_get_tree_view ( treeviewcolumn );

	g_object_get ( tree, "search-column", &col, NULL );

	GnoclCommandData *cs = ( GnoclCommandData * ) user_data;

	GnoclPercSubst ps[] =
	{
		{ 'w', GNOCL_STRING },  /* widget */
		{ 'c', GNOCL_INT },  	/* active column number */
		{ 0 }
	};

	ps[0].val.str = gnoclGetNameFromWidget ( tree );
	ps[1].val.i = col;
	gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 );

}

/**
\brief	Handle entry icon signals.
**/
static void doOnIconPress ( GtkWidget *entry, GtkEntryIconPosition icon_pos, GdkEvent  *event, gpointer user_data )
{
#ifdef DEBUG_PARSEOPTIONS
	g_print ( "%s %d\n", __FUNCTION__, icon_pos );
#endif
	GnoclCommandData *cs = ( GnoclCommandData * ) user_data;

	GnoclPercSubst ps[] =
	{
		{ 'w', GNOCL_STRING },  /* widget */
		{ 'b', GNOCL_STRING },
		{ 'p', GNOCL_STRING },  /* icon position */
		{ 'g', GNOCL_STRING },  /* glade name */
		{ 't', GNOCL_STRING },  /* glade name */
		{ 0 }
	};

	ps[0].val.str = gnoclGetNameFromWidget ( entry );

	switch ( event->type )
	{
		case GDK_BUTTON_PRESS:   ps[1].val.str = "buttonPress"; break;
		case GDK_2BUTTON_PRESS:  ps[1].val.str = "button2Press"; break;
		case GDK_3BUTTON_PRESS:  ps[1].val.str = "button3Press"; break;
		case GDK_BUTTON_RELEASE: ps[1].val.str = "buttonRelease"; break;
		default:  assert ( 0 ); break;
	}

	if ( icon_pos )
	{
		ps[2].val.str = "secondary";
	}

	else
	{
		ps[2].val.str = "primary";
	}


	ps[3].val.str = gtk_widget_get_name ( GTK_WIDGET ( entry ) );

	ps[4].val.str = gtk_entry_get_text  ( GTK_WIDGET ( entry ) );

	gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 );
}


/**
\brief
	Handle mouse button presses -not a button widget click.
**/
static void doOnMouseButton ( GtkWidget *widget, GdkEventButton *event, gpointer data )
{
#ifdef DEBUG_PARSEOPTIONS
	g_print ( "%s\n", __FUNCTION__ );
#endif
	GnoclCommandData *cs = ( GnoclCommandData * ) data;

	GnoclPercSubst ps[] =
	{
		{ 'w', GNOCL_STRING },  /* widget */
		{ 't', GNOCL_STRING },
		{ 'x', GNOCL_INT },
		{ 'y', GNOCL_INT },
		{ 'X', GNOCL_INT },
		{ 'Y', GNOCL_INT },
		{ 'b', GNOCL_INT },
		{ 's', GNOCL_INT },
		{ 'g', GNOCL_STRING },  /* glade name */
		{ 0 }
	};

	ps[0].val.str = gnoclGetNameFromWidget ( widget );
	ps[8].val.str = gtk_widget_get_name ( widget );

	switch ( event->type )
	{
		case GDK_BUTTON_PRESS:   ps[1].val.str = "buttonPress"; break;
		case GDK_2BUTTON_PRESS:  ps[1].val.str = "button2Press"; break;
		case GDK_3BUTTON_PRESS:  ps[1].val.str = "button3Press"; break;
		case GDK_BUTTON_RELEASE: ps[1].val.str = "buttonRelease"; break;
		default:  assert ( 0 ); break;
	}

	ps[2].val.i = event->x;
	ps[3].val.i = event->y;
	ps[4].val.i = event->x_root;
	ps[5].val.i = event->y_root;
	ps[6].val.i = event->button;
	ps[7].val.i = event->state;



	gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 );
}


/**
\brief
**/
static void doOnUndoRedo ( GtkWidget *widget, gpointer data )
{
#ifdef DEBUG_PARSEOPTIONS
	g_print ( "%s\n", __FUNCTION__ );
#endif
	GnoclCommandData *cs = ( GnoclCommandData * ) data;

	GnoclPercSubst ps[] =
	{
		{ 'w', GNOCL_STRING },  /* widget */
		{ 'g', GNOCL_STRING },  /* glade name */
		{ 'u', GNOCL_INT },  	/* flag denoting whether further undos possible */
		{ 'r', GNOCL_INT },  	/* flag denoting whether further redos possible */
		{ 0 }
	};


	ps[0].val.str = gnoclGetNameFromWidget ( widget );
	ps[1].val.str = gtk_widget_get_name ( widget );
	//ps[2].val.i = gtk_source_undo_manager_can_undo ( GTK_UNDO_VIEW ( widget )->undo_manager_ );
	//ps[3].val.i = gtk_source_undo_manager_can_redo ( GTK_UNDO_VIEW ( widget )->undo_manager_ );

	gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 );
}


/**
\brief
\author
\date
\note
**/
static void doOnFontSet ( GtkWidget *widget, gpointer data )
{
	GnoclCommandData *cs = ( GnoclCommandData * ) data;

	GnoclPercSubst ps[] =
	{
		{ 'w', GNOCL_STRING },  /* widget */
		{ 'f', GNOCL_STRING },  /* widget */
		{ 0 }
	};

	ps[0].val.str = gnoclGetNameFromWidget ( widget );
	ps[1].val.str = gtk_font_button_get_font_name ( widget );

	gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 );

}

/**
\brief
\author
\date
\note
**/
static void doOnFolderSet ( GtkWidget *widget, gpointer data )
{
#ifdef DEBUG_PARSEOPTIONS
	g_print ( "%s\n", __FUNCTION__ );
#endif

	GnoclCommandData *cs = ( GnoclCommandData * ) data;

	GnoclPercSubst ps[] =
	{
		{ 'w', GNOCL_STRING },  /* widget */
		{ 'f', GNOCL_STRING },  /* widget */
		{ 0 }
	};

	ps[0].val.str = gnoclGetNameFromWidget ( widget );
	ps[1].val.str = gtk_file_chooser_get_filename ( widget );

	gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 );

}

/**
\brief	Return name of item selected in a file/folder chooser button operation
**/
static void doOnFileSet ( GtkWidget *widget, gpointer data )
{

	GnoclCommandData *cs = ( GnoclCommandData * ) data;

	GnoclPercSubst ps[] =
	{
		{ 'w', GNOCL_STRING },  /* widget */
		{ 'f', GNOCL_STRING },  /* widget */
		{ 'g', GNOCL_STRING },  /* glade name */
		{ 0 }
	};

	ps[0].val.str = gnoclGetNameFromWidget ( widget );
	ps[1].val.str = gtk_file_chooser_get_filename ( widget );
	ps[2].val.str = gtk_widget_get_name ( widget );

	gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 );

}

/**
\brief
\author
\date
\note
**/
static void doOnLinkButton ( GtkWidget *widget, gpointer data )
{
	GnoclCommandData *cs = ( GnoclCommandData * ) data;

	GnoclPercSubst ps[] =
	{
		{ 'w', GNOCL_STRING },  /* widget */
		{ 'u', GNOCL_STRING },  /* widget */
		{ 'g', GNOCL_STRING },  /* glade name */
		{ 0 }
	};

	ps[0].val.str = gnoclGetNameFromWidget ( widget );
	ps[1].val.str =  gtk_link_button_get_uri ( widget );
	ps[2].val.str = gtk_widget_get_name ( widget );

#ifdef DEBUG_PARSEOPTIONS
	g_print ( "label = %s uri= %s\n", gnoclGetNameFromWidget ( widget ), gtk_link_button_get_uri ( widget ) );
#endif

	gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 );

}

/**
\brief      Called from gnoclOptOnEvent, this is handles how event signals are processed and
            values substituted into % values for further processing by Tcl scripts.
\author     William J Giddings
\date       01/11/2009
\note		Events only apply to windows and not object such as tags. <--- WRONG
**/
static void doOnEvent (	GtkTextTag *texttag, GObject *widget, GdkEvent *event, GtkTextIter *arg2, gpointer data )
{

	GnoclCommandData *cs = ( GnoclCommandData * ) data;

	GnoclPercSubst ps[] =
	{

		{ 'w', GNOCL_STRING },  /* 0 */
		{ 't', GNOCL_STRING },  /* 1 */
		{ 'x', GNOCL_INT },     /* 2 */
		{ 'y', GNOCL_INT },     /* 3 */
		{ 's', GNOCL_INT },     /* 4 */
		{ 'b', GNOCL_INT },     /* 5 */
		{ 'n', GNOCL_STRING },  /* 6 */
		{ 'X', GNOCL_INT },     /* 7 */
		{ 'Y', GNOCL_INT },     /* 8 */
		{ 'g', GNOCL_STRING },  /* 9 glade name */
		{ 'd', GNOCL_STRING },  /* 10 */
		{ 0 }
	};

	/* initialize with default values */
	ps[0].val.str = gnoclGetNameFromWidget ( widget );
	ps[9].val.str = gtk_widget_get_name ( GTK_WIDGET ( widget ) );
	ps[10].val.str = g_object_get_data ( texttag, "gnocl::data" );

	/* most of these events are not reported by the tag */

	/*
	 * _eventType_ ( event->type );
	 */

	/* set some initial values */
	switch ( event->type )
	{
			/* these are not reported, it would be useful if they were, could do roll-over effects */
		case GDK_ENTER_NOTIFY:
			{
				ps[1].val.str = "enterNotify";
			}
			break;
		case GDK_LEAVE_NOTIFY:
			{
				ps[1].val.str = "leaveNotify";
			}
			break;
			/* this is reported */
		case GDK_MOTION_NOTIFY:
			{
				ps[1].val.str = "motionNotify";
				ps[2].val.i = event->motion.x;
				ps[3].val.i = event->motion.y;
				ps[4].val.i = event->motion.state;
				ps[7].val.i = event->motion.x_root;
				ps[8].val.i = event->motion.y_root;
			}
			break;
		case GDK_BUTTON_PRESS:
		case GDK_2BUTTON_PRESS:
		case GDK_3BUTTON_PRESS:
		case GDK_BUTTON_RELEASE:
			{
				switch ( event->type )
				{
					case GDK_BUTTON_PRESS:      ps[1].val.str = "buttonPress"; break;
					case GDK_2BUTTON_PRESS:     ps[1].val.str = "button2Press"; break;
					case GDK_3BUTTON_PRESS:     ps[1].val.str = "button3Press"; break;
					case GDK_BUTTON_RELEASE:    ps[1].val.str = "buttonRelease"; break;
					default:                    assert ( 0 ); break;
				}

				ps[2].val.i = event->button.x;
				ps[3].val.i = event->button.y;
				ps[4].val.i = event->button.state;
				ps[5].val.i = event->button.button;
				ps[7].val.i = event->button.x_root;
				ps[8].val.i = event->button.y_root;
			}
			break;

		default:
			{
				// assert( 1 );
				ps[1].val.str = "unknownEvent";
			}

	}

	/* assign values to remaining elements of the array */
	ps[2].val.i = event->button.x;
	ps[3].val.i = event->button.y;
	ps[4].val.i = event->button.state;
	ps[5].val.i = event->button.button;
	ps[7].val.i = event->button.x_root;
	ps[8].val.i = event->button.y_root;

	ps[6].val.str = texttag->name;

	/* other settings, mouse pointer etc can be obtained from the window structure */
	gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 );
}

/**
\brief
\author
\date
\note
**/
int gnoclOptOnScroll ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	assert ( strcmp ( opt->optName, "-onScroll" ) == 0 );
	return gnoclConnectOptCmd ( interp, obj, "scroll-event", G_CALLBACK ( doOnScroll ), opt, NULL, ret );
}

/**
\brief
\author
\date
\note
**/
int gnoclOptOnEvent ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	assert ( strcmp ( opt->optName, "-onEvent" ) == 0 );
	return gnoclConnectOptCmd ( interp, obj, "event", G_CALLBACK ( doOnEvent ), opt, NULL, ret );
}

/**
\brief
\author     William J Giddings
\date
\note       This is no longer used but may prove a useful later.
            The function was originally developed as an event handler for
            text tag bingings. However, only a limited set of the events listed below
            are supported by the Gtk text tag.
**/
static void XXXXXdoOnEvent (
	GtkTextTag *texttag,
	GObject *arg1,
	GdkEvent *event,
	GtkTextIter *arg2,
	gpointer data )
{
	GnoclCommandData *cs = ( GnoclCommandData * ) data;

	GnoclPercSubst ps[] =
	{
		{ 't', GNOCL_STRING },
		{ 'w', GNOCL_STRING },
		{ 'x', GNOCL_INT },
		{ 'y', GNOCL_INT },
		{ 'b', GNOCL_INT },
		{ 'X', GNOCL_INT },
		{ 'Y', GNOCL_INT },
		{ 's', GNOCL_INT },
		{ 'g', GNOCL_STRING },  /* glade name */
		{ 0 }
	};

	/* most of these events are not reported by the tag */

	switch ( event->type )
	{

			/* these are reported */
		case GDK_MOTION_NOTIFY:
			ps[0].val.str = "motionNotify";
			// ps[1].val.str = ((GdkEventButton*)event)->window ;
			ps[2].val.i = ( ( GdkEventButton* ) event )->x;
			ps[3].val.i = ( ( GdkEventButton* ) event )->y;
			ps[4].val.i = ( ( GdkEventButton* ) event )->button;
			ps[5].val.i = ( ( GdkEventButton* ) event )->x_root;
			ps[6].val.i = ( ( GdkEventButton* ) event )->y_root;
			ps[7].val.i = ( ( GdkEventButton* ) event )->state;
			break;
		case GDK_BUTTON_PRESS:
		case GDK_2BUTTON_PRESS:
		case GDK_3BUTTON_PRESS:
		case GDK_BUTTON_RELEASE:

			switch ( event->type )
			{
				case GDK_BUTTON_PRESS:      ps[0].val.str = "buttonPress"; break;
				case GDK_2BUTTON_PRESS:     ps[0].val.str = "doubleButtonPress"; break;
				case GDK_3BUTTON_PRESS:     ps[0].val.str = "tripleButtonPress"; break;
				case GDK_BUTTON_RELEASE:    ps[0].val.str = "buttonRelease"; break;
				default:  assert ( 0 ); break;
			}

			// ps[1].val.str = ((GdkEventButton*)event)->window ;
			ps[2].val.i = ( ( GdkEventButton* ) event )->x;
			ps[3].val.i = ( ( GdkEventButton* ) event )->y;
			ps[4].val.i = ( ( GdkEventButton* ) event )->button;
			ps[5].val.i = ( ( GdkEventButton* ) event )->x_root;
			ps[6].val.i = ( ( GdkEventButton* ) event )->y_root;
			ps[7].val.i = ( ( GdkEventButton* ) event )->state;

			break;

			/* these are not */
		case GDK_NOTHING:           ps[0].val.str = "nothing"; break;

		case GDK_DELETE:            ps[0].val.str = "delete"; break;

		case GDK_DESTROY:           ps[0].val.str = "destroy"; break;

		case GDK_EXPOSE:            ps[0].val.str = "expose"; break;

		case GDK_KEY_PRESS:         ps[0].val.str = "keyPress"; break;

		case GDK_KEY_RELEASE:       ps[0].val.str = "keyRelease"; break;

		case GDK_ENTER_NOTIFY:      ps[0].val.str = "enterNotify"; break;

		case GDK_LEAVE_NOTIFY:      ps[0].val.str = "leaveNotify"; break;

		case GDK_FOCUS_CHANGE:      ps[0].val.str = "focusChange"; break;

		case GDK_CONFIGURE:         ps[0].val.str = "configure"; break;

		case GDK_MAP:               ps[0].val.str = "map"; break;

		case GDK_UNMAP:             ps[0].val.str = "unmap"; break;

		case GDK_PROPERTY_NOTIFY:   ps[0].val.str = "propertyNotify"; break;

		case GDK_SELECTION_CLEAR:   ps[0].val.str = "selectionClear"; break;

		case GDK_SELECTION_REQUEST: ps[0].val.str = "selectrionRequest"; break;

		case GDK_SELECTION_NOTIFY:  ps[0].val.str = "selectionNotify"; break;

		case GDK_PROXIMITY_IN:      ps[0].val.str = "proximityIn"; break;

		case GDK_PROXIMITY_OUT:     ps[0].val.str = "proximityOut"; break;

		case GDK_DRAG_ENTER:        ps[0].val.str = "dragEnter"; break;

		case GDK_DRAG_LEAVE:        ps[0].val.str = "dragLeave"; break;

		case GDK_DRAG_MOTION:       ps[0].val.str = "dragMotion"; break;

		case GDK_DRAG_STATUS:       ps[0].val.str = "dragStatus"; break;

		case GDK_DROP_START:        ps[0].val.str = "dropStart"; break;

		case GDK_DROP_FINISHED:     ps[0].val.str = "dropFinished"; break;

		case GDK_CLIENT_EVENT:      ps[0].val.str = "clientEvent"; break;

		case GDK_VISIBILITY_NOTIFY: ps[0].val.str = "visibilityNotify"; break;

		case GDK_NO_EXPOSE:         ps[0].val.str = "noExpose"; break;

		case GDK_SCROLL:            ps[0].val.str = "scroll"; break;

		case GDK_WINDOW_STATE:      ps[0].val.str = "windowState"; break;

		case GDK_SETTING:           ps[0].val.str = "setting"; break;

		case GDK_OWNER_CHANGE:      ps[0].val.str = "ownerChange"; break;

		case GDK_GRAB_BROKEN:       ps[0].val.str = "grabBroken"; break;

		default: assert ( 1 ); break;
	}



	/* other settings, mouse pointer etc can be obtained from the window structure */
	gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 );
}

/**
\brief
**/
int gnoclOptOnClicked ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	return gnoclConnectOptCmd ( interp, obj, "clicked", G_CALLBACK ( doOnClicked ), opt, NULL, ret );
}

/**
\brief
**/
int gnoclOptOnButtonClicked ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	return gnoclConnectOptCmd ( interp, obj, "clicked", G_CALLBACK ( doOnToolButtonClicked ), opt, NULL, ret );
}


/**
\brief
**/
int gnoclOptOnInteractiveSearch ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	return gnoclConnectOptCmd ( interp, obj,  "start-interactive-search", G_CALLBACK ( doOnInteractiveSearch ), opt, NULL, ret );
}


/**
\brief
**/
int gnoclOptOnColumnClicked ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	return gnoclConnectOptCmd ( interp, obj, "clicked", G_CALLBACK ( doOnColumnClicked ), opt, NULL, ret );
}


/**
\brief Respond to entry icon signals.
**/
int gnoclOptOnIconPress ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	assert ( *opt->propName == 'R' || *opt->propName == 'P' );
	return gnoclConnectOptCmd ( interp, obj, *opt->propName == 'P' ?  "icon-press" : "icon-release", G_CALLBACK ( doOnIconPress ), opt, NULL, ret );
}



/**
\brief
Handle mouse button event
**/
int gnoclOptOnButton ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	assert ( *opt->propName == 'R' || *opt->propName == 'P' );
	return gnoclConnectOptCmd ( interp, obj, *opt->propName == 'P' ?  "button-press-event" : "button-release-event", G_CALLBACK ( doOnMouseButton ), opt, NULL, ret );
}

/**
\brief
**/
int gnoclOptOnUndoRedo ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	assert ( *opt->propName == 'U' || *opt->propName == 'R' );
	return gnoclConnectOptCmd ( interp, obj, *opt->propName == 'U' ?  "undo" : "redo", G_CALLBACK ( doOnUndoRedo ), opt, NULL, ret );
}




/**
\brief
\author     William J Giddings
\date       29/Dec/09
**/
int gnoclOptOnFontSet ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	return gnoclConnectOptCmd ( interp, obj, "font-set", G_CALLBACK ( doOnFontSet ), opt, NULL, ret );
}

/**
\brief
\author     William J Giddings
\date       21/Jan/2010
**/
int gnoclOptOnFileSet ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{

	return gnoclConnectOptCmd ( interp, obj, "file-set", G_CALLBACK ( doOnFileSet ), opt, NULL, ret );
}

/**
\brief
\author     William J Giddings
\date       21/Jan/2010
**/
int gnoclOptOnFolderSet ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
#ifdef DEBUG_PARSEOPTIONS
	g_printf ( "gnoclOptOnFolderSet\n" );
#endif
	return gnoclConnectOptCmd ( interp, obj, "file-set", G_CALLBACK ( doOnFolderSet ), opt, NULL, ret );
}


/**
\brief
\author
\date
\note
**/
int gnoclOptOnLinkButton ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{

	/* check the name of the signal is correct for this function */
	assert ( strcmp ( opt->optName, "-onClicked" ) == 0 );

	return gnoclConnectOptCmd ( interp, obj, "clicked", G_CALLBACK ( doOnLinkButton ), opt, NULL, ret );
}

/*
    the following are handlers for GTKTextBuffer signals insert-text, delete-range
*/
/**
\brief
\author
\date
\note
**/
static void doOnCopyClipboard ( GtkTextBuffer *textbuffer, gpointer user_data )
{

	GnoclCommandData *cs = ( GnoclCommandData * ) user_data;

	GnoclPercSubst ps[] =
	{
		{ 't', GNOCL_STRING },  /* text inserted */
		{ 0 }
	};

	ps[0].val.str   = "copy";


	gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 );
}
/**
\brief
\author
\date
\note
**/
static void doOnCutClipboard ( GtkTextBuffer *textbuffer, gpointer user_data )
{

	GnoclCommandData *cs = ( GnoclCommandData * ) user_data;

	GnoclPercSubst ps[] =
	{
		{ 't', GNOCL_STRING },  /* text inserted */
		{ 0 }
	};

	ps[0].val.str   = "cut";


	gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 );
}

/**
\brief
\author
\date
\note
**/
static void doOnPasteClipboard ( GtkTextBuffer *textbuffer, gpointer user_data )
{

	GnoclCommandData *cs = ( GnoclCommandData * ) user_data;

	GnoclPercSubst ps[] =
	{
		{ 't', GNOCL_STRING },  /* text inserted */
		{ 0 }
	};

	ps[0].val.str   = "paste";


	gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 );
}




/**
\brief      The paste-done signal is emitted after paste operation has been completed.
\author     William J Giddings
\date       15/Jan/2010
\note       This is useful to properly scroll the view to the end of the pasted text.
**/
static void doOnPasteDone ( GtkTextBuffer *textbuffer, GtkClipboard *arg1, gpointer user_data )
{

	GnoclCommandData *cs = ( GnoclCommandData * ) user_data;

	GtkTextMark *mark;
	GtkTextIter *iter;

	GnoclPercSubst ps[] =
	{
		{ 'w', GNOCL_STRING },  /* widget */
		{ 'r', GNOCL_INT },     /* row */
		{ 'c', GNOCL_INT },     /* column */
		{ 0 }
	};

	/* obtain the new insert mark, ie the end of the pasted  block */
	mark = gtk_text_buffer_get_insert ( textbuffer );
	gtk_text_buffer_get_iter_at_mark ( textbuffer, iter, mark );

	ps[0].val.str = gnoclGetNameFromWidget ( textbuffer );
	ps[1].val.i     = gtk_text_iter_get_line ( iter );
	ps[2].val.i     = gtk_text_iter_get_line_offset ( iter );

	gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 );
}


/**
\brief
\author
\date
\note
**/
static void doOnTextInsert (
	GtkTextBuffer *textbuffer,
	GtkTextIter *location,
	gchar *text,
	gint len,
	gpointer user_data )
{

	GnoclCommandData *cs = ( GnoclCommandData * ) user_data;
	GtkTextIter *iter;

	GnoclPercSubst ps[] =
	{
		{ 'w', GNOCL_STRING },  /* widget */
		{ 't', GNOCL_STRING },  /* text inserted */
		{ 'r', GNOCL_INT },     /* row into which text inserted */
		{ 'c', GNOCL_INT },     /* column into which text inserted */
		{ 'l', GNOCL_INT },     /* row into which text inserted */
		{ 0 }
	};

	ps[0].val.str = gnoclGetNameFromWidget ( textbuffer );
	ps[1].val.str   = text;
	ps[2].val.i     = gtk_text_iter_get_line ( location );
	ps[3].val.i     = gtk_text_iter_get_line_offset ( location );
	ps[4].val.i     = len;

	gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 );
}

/**
\brief      Respond to the input to GtkEntry widget before updating widget contents.
\note       Use this signal to preprocess input for validation, undo/redo operations etc.
\author     William J Giddings
\date       26-Apr-09
**/
static void doOnInsertAtCursor ( GtkEntry *entry, gchar *arg1, gpointer user_data )
{
#ifdef DEBUG_PARSEOPTIONS
	g_print ( "doOnEntryInsertText 1 %s\n", arg1 );
#endif
	GnoclCommandData *cs = ( GnoclCommandData * ) user_data;

	GnoclPercSubst ps[] =
	{
		{ 'w', GNOCL_STRING },  /* widget */
		{ 'g', GNOCL_STRING },  /* glade name */
		{ 't', GNOCL_STRING },  /* text inserted */
		{ 0 }
	};

	ps[0].val.str = gnoclGetNameFromWidget ( GTK_WIDGET ( entry ) );
	ps[1].val.str = gtk_widget_get_name ( GTK_WIDGET ( entry ) );
	ps[2].val.str = arg1;

#ifdef DEBUG_PARSEOPTIONS
	g_print ( "doOnEntryInsertText 2\n" );
	g_print ( "doOnEntryInsertText 3 : %s %s\n", ps[0].val.str,  ps[1].val.str );
#endif
	/* this line here is the problem! causing a memory segmentation problem */

	if ( 1 )
	{
		gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 );
	}

}

/**
\brief
\author     William J Giddings
\date       26-Apr-09
**/
int gnoclOptOnInsertAtCursor ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
#ifdef DEBUG_PARSEOPTIONS
	g_print ( "gnoclOptOnInsertAtCursor 1\n" );
#endif
	/* check the name of the signal is correct for this function */
	assert ( strcmp ( opt->optName, "-onInsert" ) == 0 );
#ifdef DEBUG_PARSEOPTIONS
	g_print ( "gnoclOptOnInsertAtCursor 2\n" );
#endif

	/* connect the signal with its callback function */
	return gnoclConnectOptCmd ( interp, GTK_ENTRY  ( obj ), "insert-at-cursor", G_CALLBACK ( doOnInsertAtCursor ), opt, NULL, ret );

}

/**
\brief
\author
\date
\note
**/
static void doOnMarkSet (
	GtkTextBuffer *textbuffer,
	GtkTextIter *location,
	GtkTextMark *mark,
	gpointer user_data )
{

	GnoclCommandData *cs = ( GnoclCommandData * ) user_data;

	GnoclPercSubst ps[] =
	{
		{ 'r', GNOCL_INT },     /* length of text inserted */
		{ 'c', GNOCL_INT },     /* row into which text inserted */
		{ 'n', GNOCL_STRING },  /* column into which text inserted */
		{ 0 }
	};

	ps[0].val.i     = gtk_text_iter_get_line ( location );
	ps[1].val.i     = gtk_text_iter_get_line_offset ( location );
	ps[2].val.str   = gtk_text_mark_get_name ( mark );

	gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 );
}

/**
\brief
\author
\date
\note
**/
static void doOnChildAttached ( GtkHandleBox *handlebox, GtkWidget *widget, gpointer user_data )
{

	GnoclCommandData *cs = ( GnoclCommandData * ) user_data;

	GnoclPercSubst ps[] =
	{
		{ 'w', GNOCL_STRING },
		{ 'h', GNOCL_STRING },
		{ 'g', GNOCL_STRING },  /* glade name */
		{ 0 }
	};

	ps[0].val.str = gnoclGetNameFromWidget ( widget );
	ps[1].val.str = gnoclGetNameFromWidget ( handlebox );
	ps[2].val.str = gtk_widget_get_name ( widget );

	gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 );
}

/**
\brief
\author
\date
\note
**/
static void doOnChildDetached (	GtkHandleBox *handlebox, GtkWidget *widget,	gpointer user_data )
{

	GnoclCommandData *cs = ( GnoclCommandData * ) user_data;

	GnoclPercSubst ps[] =
	{
		{ 'w', GNOCL_STRING },
		{ 'h', GNOCL_STRING },
		{ 'g', GNOCL_STRING },  /* glade name */
		{ 0 }
	};

	ps[0].val.str = gnoclGetNameFromWidget ( widget );
	ps[1].val.str = gnoclGetNameFromWidget ( handlebox );
	ps[2].val.str = gtk_widget_get_name ( widget );

	gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 );
}

/**
\brief
\author
\date
\note
**/
static void doOnMarkDelete (	GtkTextBuffer *textbuffer,	GtkTextMark *mark,	gpointer user_data )
{

	GnoclCommandData *cs = ( GnoclCommandData * ) user_data;

	GnoclPercSubst ps[] =
	{
		{ 'n', GNOCL_STRING },  /* column into which text inserted */
		{ 0 }
	};

	ps[0].val.str   = gtk_text_mark_get_name ( mark );

	gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 );
}

/**
\brief
\author
\date
\note
**/
static void doOnBeginUserAction (	GtkTextBuffer *textbuffer,	gpointer user_data )
{

	GnoclCommandData *cs = ( GnoclCommandData * ) user_data;

	GnoclPercSubst ps[] =
	{
		{ 'b', GNOCL_INT },     /* begin, return */
		{ 0 }
	};

	ps[0].val.i = 1;

	gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 );
}

/**
\brief
**/
static void doOnInsertChildAnchor ( GtkTextBuffer *textbuffer, GtkTextIter *location, GtkTextChildAnchor *anchor, gpointer user_data )
{

	GnoclCommandData *cs = ( GnoclCommandData * ) user_data;

	GnoclPercSubst ps[] =
	{
		{ 'a', GNOCL_STRING},   /* anchor applied */
		{ 'r', GNOCL_INT },     /* start (r)ow from which tag was applied */
		{ 'c', GNOCL_INT },     /* start (c)ol from which tag was applied */
		{ 'a', GNOCL_INT },     /*  */
		{ 0 }
	};

	// ps[0].val.str    = anchor->name;
	ps[0].val.i     = gtk_text_iter_get_line ( location );
	ps[1].val.i     = gtk_text_iter_get_line_offset ( location );


	gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 );
}

/**
\brief
**/
static void doOnEndUserAction ( GtkTextBuffer *textbuffer, gpointer user_data )
{

	GnoclCommandData *cs = ( GnoclCommandData * ) user_data;

	GnoclPercSubst ps[] =
	{
		{ 'e', GNOCL_INT },     /* end, return */
		{ 0 }
	};

	ps[0].val.i = 1;

	gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 );
}

/**
\brief
**/
static void doOnChanged ( GtkTextBuffer *textbuffer, gpointer user_data )
{

	GnoclCommandData *cs = ( GnoclCommandData * ) user_data;

	GnoclPercSubst ps[] =
	{
		{ 'c', GNOCL_INT },     /* changed, return */
		{ 0 }
	};

	ps[0].val.i = 1;

	gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 );
}

/**
\brief
**/
int gnoclOptOnChanged ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{

	/* check the name of the signal is correct for this function */
	assert ( strcmp ( opt->optName, "-onChanged" ) == 0 );

	/* get a pointer to the textbuffer for this textview widget */
	GtkTextBuffer *buffer;
	buffer =  gtk_text_view_get_buffer ( obj );

	/* connect the signal with its callback function */
	return gnoclConnectOptCmd ( interp, buffer, "changed", G_CALLBACK ( doOnChanged ), opt, NULL, ret );
}


/**
\brief
\author
\date
\note
**/
/* check this function format with the Gtk refeerence docs, the format is probabaly wrong */
static void doOnValueChanged ( GtkScaleButton *button, gdouble value, gpointer user_data )
{

	GnoclCommandData *cs = ( GnoclCommandData * ) user_data;

	GnoclPercSubst ps[] =
	{
		{ 'w', GNOCL_STRING }, /* widget */
		{ 'v', GNOCL_DOUBLE }, /* value */
		{ 'g', GNOCL_STRING }, /* glade name */
		{ 0 }
	};

	ps[0].val.str = gnoclGetNameFromWidget ( button );
	ps[1].val.d = value;
	ps[2].val.str = gtk_widget_get_name ( GTK_WIDGET ( button ) );

	gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 );

}

/**
\brief      Return formmated string to the Tcl interpreter.
\author     William J Giddingd
\date       11/05/09
**/
static gboolean doOnVisibility ( GtkWidget *widget, GdkEventVisibility *event, gpointer  data )
{
	GnoclCommandData *cs = ( GnoclCommandData * ) data;

	GnoclPercSubst ps[] =
	{
		{ 'w', GNOCL_STRING },  /* widget */
		{ 'v', GNOCL_STRING },  /* visibility  */
		{ 'g', GNOCL_STRING },  /* glade name */
		{ 'e', GNOCL_STRING },  /* signal/event name */
		{ 0 }
	};


	ps[0].val.str = gnoclGetNameFromWidget ( widget );
	ps[2].val.str = gtk_widget_get_name ( widget );

	switch ( event->state )
	{
		case GDK_VISIBILITY_FULLY_OBSCURED:
			{
				ps[1].val.str = "none";
				break;
			}

		case GDK_VISIBILITY_PARTIAL:
			{
				ps[1].val.str = "partial";
				break;
			}

		case GDK_VISIBILITY_UNOBSCURED:
			{
				ps[1].val.str = "all";
				break;
			}
	}

	ps[3].val.str = "visibility-notify-event";

	gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 );

}

/**
\brief
\author
\date
\note
**/
static void doOnModified (
	GtkTextBuffer *textbuffer,
	gpointer user_data )
{

	GnoclCommandData *cs = ( GnoclCommandData * ) user_data;

	GnoclPercSubst ps[] =
	{
		{ 'c', GNOCL_INT },     /* changed, return */
		{ 0 }
	};

	ps[0].val.i = 1;

	gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 );
}


/**
\brief
\author
\date
\note
**/
static void doOnApplyTag (	GtkTextBuffer *textbuffer,	GtkTextTag *tag,	GtkTextIter *start,	GtkTextIter *end,	gpointer user_data )
{

	GnoclCommandData *cs = ( GnoclCommandData * ) user_data;

	GnoclPercSubst ps[] =
	{
		{ 't', GNOCL_STRING },  /* tag applied */
		{ 'r', GNOCL_INT },     /* start (r)ow from which tag was applied */
		{ 'c', GNOCL_INT },     /* start (c)ol from which tag was applied */
		{ 'l', GNOCL_INT },     /* end (l)ine to which tag was applied */
		{ 'o', GNOCL_INT },     /* end (o)ffset to which tag was applied */
		{ 0 }
	};

	ps[0].val.str   = tag->name;
	ps[1].val.i     = gtk_text_iter_get_line ( start );
	ps[2].val.i     = gtk_text_iter_get_line_offset ( start );
	ps[3].val.i     = gtk_text_iter_get_line ( end );
	ps[4].val.i     = gtk_text_iter_get_line_offset ( end );

	gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 );
}

/**
\brief
\author
\date
\note
**/
static void doOnRemoveTag (	GtkTextBuffer *textbuffer,	GtkTextTag *tag,	GtkTextIter *start,	GtkTextIter *end,	gpointer user_data )
{

	GnoclCommandData *cs = ( GnoclCommandData * ) user_data;

	GnoclPercSubst ps[] =
	{
		{ 't', GNOCL_STRING },  /* tag removed */
		{ 'r', GNOCL_INT },     /* start (r)ow from which tag was applied */
		{ 'c', GNOCL_INT },     /* start (c)ol from which tag was applied */
		{ 'l', GNOCL_INT },     /* end (l)ine to which tag was applied */
		{ 'o', GNOCL_INT },     /* end (o)ffset to which tag was applied */
		{ 0 }
	};

	ps[0].val.str   = tag->name;
	ps[1].val.i     = gtk_text_iter_get_line ( start );
	ps[2].val.i     = gtk_text_iter_get_line_offset ( start );
	ps[3].val.i     = gtk_text_iter_get_line ( end );
	ps[4].val.i     = gtk_text_iter_get_line_offset ( end );

	gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 );
}

/**
\brief      Respond to a textBuffer delete-range signal.
 *  WJG 29/07/08
 *  void user_function (
 *      GtkTextBuffer *textbuffer,
 *      GtkTextIter   *start,
 *      GtkTextIter   *end,
 *      gpointer       user_data)
 */
static void doOnDeleteRange (
	GtkTextBuffer *textbuffer,
	GtkTextIter   *start,
	GtkTextIter   *end,
	gpointer       user_data )
{
#ifdef DEBUG_PARSEOPTIONS
	g_print ( "doOnDeleteRange >1\n" ) ;
#endif

	GnoclCommandData *cs = ( GnoclCommandData * ) user_data;

	GnoclPercSubst ps[] =
	{
		{ 'r', GNOCL_INT },     /* start (r)ow from which text was deleted */
		{ 'c', GNOCL_INT },     /* start (c)ol from which text was deleted */
		{ 'l', GNOCL_INT },     /* end (l)ine from which text was deleted */
		{ 'o', GNOCL_INT },     /* end (o)ffset from which text was deleted */
		{ 't', GNOCL_STRING },  /* the text deleted */
		{ 0 }
	};

	ps[0].val.i     = gtk_text_iter_get_line ( start );
	ps[1].val.i     = gtk_text_iter_get_line_offset ( start );
	ps[2].val.i     = gtk_text_iter_get_line ( end );
	ps[3].val.i     = gtk_text_iter_get_line_offset ( end );
	ps[4].val.str   = gtk_text_buffer_get_text ( textbuffer, start, end, 0 );

	gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 );
}

/**
\brief
\author
\date
\note
**/
static void doOnKey ( GtkWidget *widget, GdkEventKey *event, gpointer data )
{
	GnoclCommandData *cs = ( GnoclCommandData * ) data;

	GnoclPercSubst ps[] =
	{
		{ 'w', GNOCL_STRING },  /* widget */
		{ 'k', GNOCL_INT },     /* keycode */
		{ 'K', GNOCL_STRING },  /* keycode as symbol */
		{ 'a', GNOCL_OBJ },     /* character */
		{ 's', GNOCL_INT },     /* state of modifiers */
		{ 'g', GNOCL_STRING },  /* glade name */
		{ 'e', GNOCL_STRING },  /* event/signal name */
		{ 0 }
	};

	guint32 unicode = gdk_keyval_to_unicode ( event->keyval );

	ps[0].val.str   = gnoclGetNameFromWidget ( widget );
	ps[1].val.i     = event->keyval;
	ps[2].val.str   = keyvalToString ( event->keyval );
	ps[3].val.obj   = unicode ?  Tcl_NewUnicodeObj ( ( Tcl_UniChar * ) & unicode, 1 ) : Tcl_NewStringObj ( "", 0 );
	ps[4].val.i     = event->state;
	ps[5].val.str   = gtk_widget_get_name ( widget );

	if ( event->type == GDK_KEY_PRESS )
	{
		ps[6].val.str = "key_press_event";
	}

	if ( event->type == GDK_KEY_RELEASE )
	{
		ps[6].val.str = "key_release_event";
	}

	gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 );
}

/**
\brief
\author
\date
\note
**/
int gnoclOptOnKeyPress ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	assert ( strcmp ( opt->optName, "-onKeyPress" ) == 0 );
	return gnoclConnectOptCmd ( interp, obj, "key-press-event", G_CALLBACK ( doOnKey ), opt, NULL, ret );
}

/**
\brief
\author
\date
\note
**/
int gnoclOptOnKeyRelease ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	assert ( strcmp ( opt->optName, "-onKeyRelease" ) == 0 );
	return gnoclConnectOptCmd ( interp, obj, "key-release-event", G_CALLBACK ( doOnKey ), opt, NULL, ret );
}

/**
\brief      Respond to a textBuffer signal to handle inserted text.
\author
\date
\note
**/
int gnoclOptOnApplyTag ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{

	/* check the name of the signal is correct for this function */
	assert ( strcmp ( opt->optName, "-onApplyTag" ) == 0 );

	/* get a pointer to the textbuffer for this textview widget */
	GtkTextBuffer *buffer;
	buffer =  gtk_text_view_get_buffer ( obj );

	/* connect the signal with its callback function */
	return gnoclConnectOptCmd ( interp, buffer, "apply-tag", G_CALLBACK ( doOnApplyTag ), opt, NULL, ret );
}

/**
\brief      Respond to a textBuffer signal to handle inserted text.
\author     WJG
\date
**/
int gnoclOptOnClipboard ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{

	switch  ( *opt->propName )
	{
			/* these three are for the GtkTextView */
		case 'X':
			{

				return gnoclConnectOptCmd ( interp, obj, "cut-clipboard"  , G_CALLBACK ( doOnCutClipboard ), opt, NULL, ret );
			}
			break;
		case 'C':
			{
				return gnoclConnectOptCmd ( interp, obj, "copy-clipboard", G_CALLBACK ( doOnCopyClipboard ), opt, NULL, ret );
			}
			break;
		case 'P':
			{
				return gnoclConnectOptCmd ( interp, obj, "paste-clipboard", G_CALLBACK ( doOnPasteClipboard ), opt, NULL, ret );
			}
			break;
		default:
			{
				// assert( 0 );
				return TCL_ERROR;
			}
	}
}

/**
\brief      Respond to a textBuffer signal to handle inserted text.
\author     WJG
\date
**/
int gnoclOptOnPasteDone ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{

	/* check the name of the signal is correct for this function */
	assert ( strcmp ( opt->optName, "-onPasteDone" ) == 0 );

	/* get a pointer to the textbuffer for this textview widget */
	GtkTextBuffer *buffer;
	buffer =  gtk_text_view_get_buffer ( obj );

	return gnoclConnectOptCmd ( interp, buffer, "paste-done", G_CALLBACK ( doOnPasteDone ), opt, NULL, ret );
}

/**
\brief      Respond to a textBuffer signal to handle inserted text.
\author     WJG
 */
int gnoclOptOnTextInsert ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{

	/* check the name of the signal is correct for this function */
	assert ( strcmp ( opt->optName, "-onInsertText" ) == 0 );

	/* get a pointer to the textbuffer for this textview widget */
	GtkTextBuffer *buffer;
	buffer =  gtk_text_view_get_buffer ( obj );

	/* connect the signal with its callback function */
	return gnoclConnectOptCmd ( interp, buffer, "insert-text", G_CALLBACK ( doOnTextInsert ), opt, NULL, ret );
}

/**
\brief      Respond to a textBuffer signal to handle inserted text.
\author     WJG
\date
**/
int gnoclOptOnMarkSet ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{

	/* check the name of the signal is correct for this function */
	assert ( strcmp ( opt->optName, "-onMarkSet" ) == 0 );

	/* get a pointer to the textbuffer for this textview widget */
	GtkTextBuffer *buffer;
	buffer =  gtk_text_view_get_buffer ( obj );

	/* connect the signal with its callback function */
	return gnoclConnectOptCmd ( interp, buffer, "mark-set", G_CALLBACK ( doOnMarkSet ), opt, NULL, ret );
}

/**
\brief      Respond to a textBuffer signal to handle mark deletion.
**/
int gnoclOptOnMarkDelete ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{

	/* check the name of the signal is correct for this function */
	assert ( strcmp ( opt->optName, "-onMarkDelete" ) == 0 );

	/* get a pointer to the textbuffer for this textview widget */
	GtkTextBuffer *buffer;
	buffer =  gtk_text_view_get_buffer ( obj );

	/* connect the signal with its callback function */
	return gnoclConnectOptCmd ( interp, buffer, "mark-delete", G_CALLBACK ( doOnMarkDelete ), opt, NULL, ret );
}

/**
\brief      Respond to a textBuffer signal to handle inserted text.
**/
int gnoclOptOnBeginUserAction ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{

	/* check the name of the signal is correct for this function */
	assert ( strcmp ( opt->optName, "-onBeginUserAction" ) == 0 );

	/* get a pointer to the textbuffer for this textview widget */
	GtkTextBuffer *buffer;
	buffer =  gtk_text_view_get_buffer ( obj );

	/* connect the signal with its callback function */
	return gnoclConnectOptCmd ( interp, buffer, "begin-user-action", G_CALLBACK ( doOnBeginUserAction ), opt, NULL, ret );
}

/**
\brief  Respond to a textBuffer signal to handle inserted text.
**/
int gnoclOptOnEndUserAction ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{

	/* check the name of the signal is correct for this function */
	assert ( strcmp ( opt->optName, "-onEndUserAction" ) == 0 );

	/* get a pointer to the textbuffer for this textview widget */
	GtkTextBuffer *buffer;
	buffer =  gtk_text_view_get_buffer ( obj );

	/* connect the signal with its callback function */
	return gnoclConnectOptCmd ( interp, buffer, "end-user-action", G_CALLBACK ( doOnEndUserAction ), opt, NULL, ret );
}

/**
\brief Respond to a textBuffer signal to handle inserted text.
**/
int gnoclOptOnInsertChildAnchor ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{

	/* check the name of the signal is correct for this function */
	assert ( strcmp ( opt->optName, "-onInsertChildAnchor" ) == 0 );

	/* get a pointer to the textbuffer for this textview widget */
	GtkTextBuffer *buffer;
	buffer =  gtk_text_view_get_buffer ( obj );

	/* connect the signal with its callback function */
	return gnoclConnectOptCmd ( interp, buffer, "insert-child-anchor", G_CALLBACK ( doOnInsertChildAnchor ), opt, NULL, ret );
}

/**
\brief
\author
\date
\note
**/
int gnoclOptOnValueChanged ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{

	/* check the name of the signal is correct for this function */
	assert ( strcmp ( opt->optName, "-onValueChanged" ) == 0 );

	/* connect the signal with its callback function */
	return gnoclConnectOptCmd ( interp, obj, "value-changed", G_CALLBACK ( doOnValueChanged ), opt, NULL, ret );

}

/**
\brief      Implement signal handler for the "visibility-notify-event" event.
\note       This only applies to widget and not windows. It tests to see of the widget toplevel is obscured.
\author     William J Giddings
\date       11/05/09
**/
int gnoclOptOnVisibility ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
#ifdef DEBUG_PARSEOPTIONS
	g_print ( "gnoclOptOnVisibility\n" );
#endif
	assert ( strcmp ( opt->optName, "-onVisibility" ) == 0 );
	return gnoclConnectOptCmd ( interp, obj, "visibility-notify-event", G_CALLBACK ( doOnVisibility ), opt, NULL, ret );
}



/**
\brief
\author
\date
\note
**/
int gnoclOptOnModified ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{

	/* check the name of the signal is correct for this function */
	assert ( strcmp ( opt->optName, "-onModified" ) == 0 );

	/* get a pointer to the textbuffer for this textview widget */
	GtkTextBuffer *buffer;
	buffer =  gtk_text_view_get_buffer ( obj );

	/* connect the signal with its callback function */
	return gnoclConnectOptCmd ( interp, buffer, "modify-changed", G_CALLBACK ( doOnModified ), opt, NULL, ret );
}

/**
\brief
\author
\date
\note
**/
int gnoclOptApplyTag ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{

	/* check the name of the signal is correct for this function */
	assert ( strcmp ( opt->optName, "-onApplyTag" ) == 0 );

	/* get a pointer to the textbuffer for this textview widget */
	GtkTextBuffer *buffer;
	buffer =  gtk_text_view_get_buffer ( obj );

	/* connect the signal with its callback function */
	return gnoclConnectOptCmd ( interp, buffer, "apply-tag", G_CALLBACK ( doOnApplyTag ), opt, NULL, ret );
}

/**
\brief
\author
\date
\note
**/
int gnoclOptOnRemoveTag ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{

	/* check the name of the signal is correct for this function */
	assert ( strcmp ( opt->optName, "-onRemoveTag" ) == 0 );

	/* get a pointer to the textbuffer for this textview widget */
	GtkTextBuffer *buffer;
	buffer =  gtk_text_view_get_buffer ( obj );

	/* connect the signal with its callback function */
	return gnoclConnectOptCmd ( interp, buffer, "remove-tag", G_CALLBACK ( doOnRemoveTag ), opt, NULL, ret );
}

/**
\brief
\author
\date
\note
**/
int gnoclOptOnChildAttached ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{

	assert ( strcmp ( opt->optName, "-onAttached" ) == 0 );
	return gnoclConnectOptCmd ( interp, obj, "child-attached", G_CALLBACK ( doOnChildAttached ), opt, NULL, ret );
}

/**
\brief
\author
\date
\note
**/
int gnoclOptOnChildDetached ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	assert ( strcmp ( opt->optName, "-onDetached" ) == 0 );
	return gnoclConnectOptCmd ( interp, obj, "child-detached", G_CALLBACK ( doOnChildDetached ), opt, NULL, ret );
}

/**
\brief
\author
\date
\note
**/
int gnoclOptOnDeleteRange ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{

	GtkTextBuffer *buffer;

	buffer =  gtk_text_view_get_buffer ( obj );

	assert ( strcmp ( opt->optName, "-onDeleteRange" ) == 0 );
	return gnoclConnectOptCmd ( interp, buffer, "delete-range", G_CALLBACK ( doOnDeleteRange ), opt, NULL, ret );
}

/**
\brief
\author
\date
\note
**/
static void onDragBegin ( GtkWidget *widget, GdkDragContext *context, gpointer data )
{
	/* TODO: set custom icon */
	/* DragData *para = (DragData *)data; */
	gtk_drag_set_icon_default ( context );
}

/**
\brief
\author
\date
\note
**/
static void onDragDataGet ( GtkWidget *widget, GdkDragContext *context, GtkSelectionData *selectionData, guint info, guint32 time, gpointer data )
{
	GnoclCommandData *cs = ( GnoclCommandData * ) data;

	GnoclPercSubst ps[] =
	{
		{ 'T', GNOCL_OBJ },     /* 0 types of target */
		{ 't', GNOCL_INT },     /* 1 time */
		{ 'w', GNOCL_STRING },  /* 2 widget */
		{ 'g', GNOCL_STRING },  /* glade name */
		{ 0 }
	};

	gchar *name = gdk_atom_name ( selectionData->target );
	ps[0].val.obj = Tcl_NewStringObj ( name, -1 );
	ps[1].val.i = time;
	ps[2].val.str = gnoclGetNameFromWidget ( widget );
	ps[3].val.str = gtk_widget_get_name ( widget );
	/*
	   context->targets is a GList with all targets of the source.
	   The first one, which is present in target is used and copied
	   to selectionData->target
	*/

	if ( gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 ) == TCL_OK )
	{
		Tcl_Obj *res = Tcl_GetObjResult ( cs->interp );

		int len;
		unsigned char *pd = Tcl_GetByteArrayFromObj ( res, &len );
		/* FIXME: format (number of bits in a unit) is 16, why?,
		          what is a "unit"? */
		gtk_selection_data_set ( selectionData, gdk_atom_intern ( name, FALSE ), 16, ( guchar * ) pd, len );
	}

	g_free ( name );
}

/**
\brief
\author
\date
\note
**/
static void onDragDataReceived ( GtkWidget *widget, GdkDragContext *context,
								 gint x, gint y, GtkSelectionData *selectionData, guint info,
								 guint32 time, gpointer data )
{
	GnoclCommandData *cs = ( GnoclCommandData * ) data;

	GnoclPercSubst ps[] =
	{
		{ 'd', GNOCL_OBJ },     /* 0 data */
		{ 'l', GNOCL_INT },     /* 1 length of data */
		{ 't', GNOCL_INT },     /* 2 time */
		{ 'T', GNOCL_STRING },  /* 3 type */
		{ 'w', GNOCL_STRING },  /* 4 widget */
		{ 'x', GNOCL_INT },     /* 5 x */
		{ 'y', GNOCL_INT },     /* 6 y */
		{ 'g', GNOCL_STRING },  /* 7 glade name */
		{ 0 }
	};

	int      ret;
	gboolean del = 0;

	ps[0].val.obj = Tcl_NewByteArrayObj ( selectionData->data, selectionData->length );
	ps[1].val.i = selectionData->length;
	ps[2].val.i = time;
	ps[3].val.str = gdk_atom_name ( selectionData->type );
	ps[4].val.str = gnoclGetNameFromWidget ( widget );
	ps[5].val.i = x;
	ps[6].val.i = y;
	ps[7].val.str = gtk_widget_get_name ( widget );

#ifdef DEBUG_PARSEOPTIONS
	g_print ( "selection: \"%s\" target: \"%s\" type: \"%s\" data: \"%s\"\n",
			  gdk_atom_name ( selectionData->selection ),
			  gdk_atom_name ( selectionData->target ),
			  gdk_atom_name ( selectionData->type ),
			  selectionData->data );
#endif

	ret = gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 );

	if ( ret == TCL_OK )
	{
		/* delete drag data if tcl function returns "delete" */
		Tcl_Obj *res = Tcl_GetObjResult ( cs->interp );
		char *str = Tcl_GetString ( res );

		if ( strcmp ( str, "delete" ) == 0 )
			del = 1;
	}

	gtk_drag_finish ( context, ret == TCL_OK, del, time );

	g_free ( ( char * ) ps[3].val.str );
}

/**
\brief
\author
\date
\note
**/
int gnoclOptOnDragData ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	int err = gnoclConnectOptCmd ( interp, obj, "drag_begin",
								   G_CALLBACK ( onDragBegin ), opt, NULL, ret );
	/*
	g_signal_connect( G_OBJECT( widget ), "drag_end",
	      G_CALLBACK( dragEnd ), para );
	*/

	if ( err == TCL_OK )
		err = gnoclConnectOptCmd ( interp, obj, "drag_data_get",
								   G_CALLBACK ( onDragDataGet ), opt, NULL, ret );

	return err;
}

/**
\brief
\author
\date
\note
**/
int gnoclOptOnDropData ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	return gnoclConnectOptCmd ( interp, obj, "drag_data_received", G_CALLBACK ( onDragDataReceived ), opt, NULL, ret );
}

/**
\brief
\author
\date
\note
**/
int gnoclOptIcons ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	const char *icons = Tcl_GetString ( opt->val.obj );

	/* set a default list of icons */
	static char *icons_[] =
	{
		GTK_STOCK_ZOOM_100
	};

#ifdef DEBUG_PARSEOPTIONS
	g_print ( "GnoclOptIcons, there are some problems here setting up the icons\n" );
	g_print ( "opt = %s val = %s \n", opt->optName, icons );
#endif

	if ( 1 )
	{
		return TCL_OK;
	}

	//gtk_scale_button_set_icons ( GTK_SCALE_BUTTON ( obj ), GTK_STOCK_ZOOM_IN );
	gtk_scale_button_set_icons ( GTK_SCALE_BUTTON ( obj ), icons_ );
}

/*
    const char *txt[] = { "left", "right", "center", "fill", NULL };
    const int types[] = { GTK_JUSTIFY_LEFT, GTK_JUSTIFY_RIGHT, GTK_JUSTIFY_CENTER, GTK_JUSTIFY_FILL };

    assert ( sizeof ( GTK_JUSTIFY_FILL ) == sizeof ( int ) );

    return gnoclOptGeneric ( interp, opt, obj, "justification", txt, types,  ret );
*/

/**
\brief
\author
\date
\note
**/
int gnoclOptIcon ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	GnoclStringType type = gnoclGetStringType ( opt->val.obj );

	switch ( type )
	{
		case GNOCL_STR_EMPTY:
			{
				gtk_window_set_icon ( GTK_WINDOW ( obj ), NULL );
			}

			break;
		case GNOCL_STR_FILE:
			{
				GdkPixbuf *pix = gnoclPixbufFromObj ( interp, opt );

				if ( pix == NULL )
				{
					return TCL_ERROR;
				}

				gtk_window_set_icon ( GTK_WINDOW ( obj ), pix );
			}

			break;
		default:
			{
				Tcl_AppendResult ( interp, "Unknown type for \"", Tcl_GetString ( opt->val.obj ), "\" must be of type FILE (%/) or empty", NULL );
			}

			return TCL_ERROR;
	}

	return TCL_OK;
}

/**
\brief
\author
\date
\note
**/
int gnoclOptLabelFull ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	if ( ret == NULL ) /* set value */
	{
		GnoclStringType type = gnoclGetStringType ( opt->val.obj );
		char *txt = gnoclGetString ( opt->val.obj );

		g_object_set ( obj, opt->propName, txt, NULL );

#ifdef DEBUG_PARSEOPTIONS
		g_print ( "opt->propName %s type %d\n", opt->propName, type );
#endif

		/* TODO? pango_parse_markup for error message */
		g_object_set ( obj, "use-markup", ( gboolean ) ( ( type & GNOCL_STR_MARKUP ) != 0 ), NULL );

		g_object_set ( obj, "use-underline", ( gboolean ) ( ( type & GNOCL_STR_UNDERLINE ) != 0 ), NULL );
	}

	else /* get value */
	{
		char *txt;
		gboolean markup, underline;
		Tcl_Obj *txtObj;
		g_object_get ( obj, opt->propName, &txt, "use-markup", &markup, "use-underline", &underline, NULL );
		txtObj = Tcl_NewStringObj ( txt, -1 );

		if ( markup )
		{
			*ret = Tcl_NewStringObj ( "%<", 2 );
			Tcl_AppendObjToObj ( *ret, txtObj );
		}

		else if ( underline )
		{
			*ret = Tcl_NewStringObj ( "%_", 2 );
			Tcl_AppendObjToObj ( *ret, txtObj );
		}

		else
			*ret = txtObj;
	}

	return TCL_OK;
}

/**
\brief
\author
\date
\note
**/
int gnoclOptDnDTargets ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	const int isSource = ( opt->propName[0] == 's' );

	if ( ret == NULL ) /* set value */
	{
		int no;

		Tcl_ListObjLength ( interp, opt->val.obj, &no );

		if ( no == 0 )
		{
			if ( isSource )
				gtk_drag_source_unset ( GTK_WIDGET ( obj ) );
			else
				gtk_drag_dest_unset ( GTK_WIDGET ( obj ) );
		}

		else
		{
			/* TODO: GDK_ACTION_COPY, GDK_ACTION_MOVE, GDK_ACTION_LINK
			         GDK_BUTTON2_MASK ...
			   for example:
			         { -action {COPY MOVE LINK} target1 target2 }
			         { -buttons {1 3 5} target1 target2 }
			*/
			int k;
			GtkTargetEntry *targets = g_new ( GtkTargetEntry, no );

			for ( k = 0; k < no; ++k )
			{
				Tcl_Obj *pobj;

				if ( Tcl_ListObjIndex ( interp, opt->val.obj, k, &pobj ) != TCL_OK )
				{
					g_free ( targets );
					return TCL_ERROR;
				}

				targets[k].target = Tcl_GetString ( pobj );

				/* TODO: GTK_TARGET_SAME_APP, GTK_TARGET_SAME_WIDGET */
				targets[k].flags = 0;
				targets[k].info = k;
			}

			if ( isSource )
				gtk_drag_source_set ( GTK_WIDGET ( obj ), GDK_BUTTON1_MASK,
									  targets, no, GDK_ACTION_COPY );
			else
				gtk_drag_dest_set ( GTK_WIDGET ( obj ), GTK_DEST_DEFAULT_ALL,
									targets, no, GDK_ACTION_COPY );

			g_free ( targets );
		}
	}

	else /* get value */
	{
		GtkTargetList *targets;

		if ( isSource )
			return TCL_OK;         /* FIXME: how to retrieve source targets? */
		else
			targets = gtk_drag_dest_get_target_list ( GTK_WIDGET ( obj ) );

		*ret = Tcl_NewListObj ( 0, NULL );

		if ( targets != NULL )
		{
			GList *p = targets->list;

			for ( ; p != NULL; p = p->next )
			{
				GtkTargetPair *pair = p->data;
				char *name = gdk_atom_name ( pair->target );
				Tcl_ListObjAppendElement ( interp, *ret,
										   Tcl_NewStringObj ( name, -1 ) );
				g_free ( name );
			}
		}
	}

	return TCL_OK;
}

/**
\brief	Parse strings of options and value pairs and assign those values
		to matching Gtk object properties specified in the options list.
\date
\note
\param	interp	pointer to Tcl Interpreter for the script\n
		opt		pointer to object options array\n
		obj		pointer to Gdk object for which properties are to be set (eg a Gtk widget)\n
		no		offset of object options within a consolidated list of options\n
**/
int gnoclSetOptions ( Tcl_Interp *interp, GnoclOption *opts, GObject *obj, int no )
{

	int k;
	GnoclOption *pop = opts;

	/* step through the options list */
	for ( k = 0; pop->optName && ( no == -1 || k < no ); ++k, ++pop )
	{

#ifdef DEBUG_PARSEOPTIONS
		g_print ( "testing for set %s %s\n", pop->optName, pop->propName );
#endif

		if ( pop->status == GNOCL_STATUS_CHANGED && pop->propName )
		{

#ifdef DEBUG_PARSEOPTIONS
			g_print ( "before setting %s\n", pop->optName );
#endif

			switch ( pop->type )
			{
				case GNOCL_BOOL:
					{
						g_object_set ( obj, pop->propName, pop->val.b, NULL );
					}
					break;
				case GNOCL_INT:
					{
						g_object_set ( obj, pop->propName, pop->val.i, NULL );
					}
					break;
				case GNOCL_DOUBLE:
					{
						g_object_set ( obj, pop->propName, pop->val.d, NULL );
					}
					break;
				case GNOCL_STRING:
					{
						g_object_set ( obj, pop->propName, pop->val.str, NULL );
					}
					break;
				case GNOCL_OBJ:
				case GNOCL_LIST:
					{
						if ( ( *pop->func ) ( interp, pop, obj, NULL ) != TCL_OK )
						{
#ifdef DEBUG_PARSEOPTIONS
							g_print ( "Error here... gnoclSetOptions/GNOCL_LIST\n" );
#endif

							pop->status = GNOCL_STATUS_SET_ERROR;

							return TCL_ERROR;

						}
					}
					break;

				default:    assert ( 0 );
			}

			pop->status = GNOCL_STATUS_SET;
		}
	}

	return TCL_OK;
}

/**
\brief
\author
\date
\note
**/
/* this is used for setting options in multiple canvas items via tags */
int gnoclResetSetOptions ( GnoclOption *opts )
{
	GnoclOption *pop;

	/* reset status */

	for ( pop = opts; pop->optName; ++pop )
	{
		if ( pop->status == GNOCL_STATUS_SET )
		{
			pop->status = GNOCL_STATUS_CHANGED;
		}
	}

	return 0;
}

/**
\brief      Parse the list of options passed from the interpreter script
			and assign these array accessible to module code.
**/
int gnoclParseOptions ( Tcl_Interp *interp, int objc, Tcl_Obj * const objv[], GnoclOption *opts )
{
	int k;
	GnoclOption *pop;

	/* reset status */
	for ( pop = opts; pop->optName; ++pop )
	{
		pop->status = 0;
	}

	/* Parse all Options */
	for ( k = 1; k < objc; k += 2 )
	{
		int idx;

		if ( gnoclGetIndexFromObjStruct ( interp, objv[k], ( char ** ) &opts[0].optName, sizeof ( GnoclOption ), "option", TCL_EXACT, &idx ) != TCL_OK )
		{
			return -1;
		}

#ifdef DEBUG_PARSEOPTIONS
		g_print ( "parsing %s -> %s %d\n", Tcl_GetString ( objv[k] ), opts[idx].optName, opts[idx].status );
#endif

		if ( k == objc - 1 )
		{
			Tcl_AppendResult ( interp, "no value given for parameter \"", Tcl_GetString ( objv[k] ) , "\"", ( char * ) NULL );
			return -1;
		}

		pop = opts + idx;

		if ( pop->status != 0 )
		{
			Tcl_AppendResult ( interp, "parameter \"", Tcl_GetString ( objv[k] ) , "\" defined more than once", ( char * ) NULL );
			pop->status = GNOCL_STATUS_CHANGED_ERROR;
			return TCL_ERROR;
		}

		/* parse types, assign to structure members */
		switch ( pop->type )
		{
			case GNOCL_STRING: /* option is a string */
				{
					int txtLen;
					const char *txt = gnoclGetStringFromObj ( objv[k+1], &txtLen );
					pop->val.str = g_memdup ( txt, txtLen + 1 );
				}

				break;
			case GNOCL_BOOL: /* option is a boolean */
				{
					int val;

					if ( Tcl_GetBooleanFromObj ( interp, objv[k+1], &val ) != TCL_OK )
					{
						pop->status = GNOCL_STATUS_CHANGED_ERROR;
						return TCL_ERROR;
					}

					pop->val.b = val;
				}

				break;
			case GNOCL_INT:	/* option is an integer */
				{
					if ( Tcl_GetIntFromObj ( interp, objv[k+1], &pop->val.i ) != TCL_OK )
					{
						pop->status = GNOCL_STATUS_CHANGED_ERROR;
						return TCL_ERROR;
					}
				}

				break;
			case GNOCL_DOUBLE: /* option is a float */
				{
					if ( Tcl_GetDoubleFromObj ( interp, objv[k+1], &pop->val.d ) != TCL_OK )
					{
						pop->status = GNOCL_STATUS_CHANGED_ERROR;
						return TCL_ERROR;
					}
				}

				break;
			case GNOCL_OBJ:  /* option is a ???? */
				{
					pop->val.obj = objv[k+1];
					Tcl_IncrRefCount ( pop->val.obj );
				}

				break;
			case GNOCL_LIST:  /* option is a list */
				{
					int len;

					if ( Tcl_ListObjLength ( interp, objv[k+1], &len ) != TCL_OK )
					{
						pop->status = GNOCL_STATUS_CHANGED_ERROR;
						return TCL_ERROR;
					}

					pop->val.obj = objv[k+1];

					Tcl_IncrRefCount ( pop->val.obj );
				}

				break;
			default:
				assert ( 0 );
		}

		pop->status = GNOCL_STATUS_CHANGED;
	}

	return TCL_OK;
}

/**
\brief   Check the validity options passed to the interpretor to either create or configure a widget.
\author
\date
\note
**/
int gnoclParseAndSetOptions ( Tcl_Interp *interp, int objc, Tcl_Obj * const objv[], GnoclOption *opts,	GObject *obj )
{
	if ( gnoclParseOptions ( interp, objc, objv, opts ) != TCL_OK )
	{
		return TCL_ERROR;
	}

	return gnoclSetOptions ( interp, opts, obj, -1 );
}

/**
\brief
\author
\date
\note
**/
int gnoclClearOptions ( GnoclOption *opts )
{
	for ( ; opts->optName; ++opts )
	{
		if ( opts->status > GNOCL_STATUS_CHANGED_ERROR )
		{
			switch ( opts->type )
			{
				case GNOCL_STRING:
					g_free ( opts->val.str );
					break;
				case GNOCL_LIST:
				case GNOCL_OBJ:

					if ( opts->val.obj != NULL )
						Tcl_DecrRefCount ( opts->val.obj );

					break;

				default:
					break;
			}
		}

		opts->status = 0;
	}

	return TCL_OK;
}

/**
\brief		Notify script that cget not available for specific option.
\author		Peter G Baum
\date
\note
**/
int gnoclCgetNotImplemented ( Tcl_Interp *interp, GnoclOption *opt )
{
	Tcl_AppendResult ( interp,
					   "Command \"cget\" for option \"",
					   opt->optName,
					   "\" is not yet implemented."
					   "\nPlease mail the author, if you need it.", NULL );
	return TCL_ERROR;
}

/**
\brief		Return value from pre-set options.
\author
\date
\note
**/
int gnoclCget (	Tcl_Interp *interp, int objc, Tcl_Obj * const objv[], GObject *gObj, GnoclOption *opts, int *idx )
{
	if ( objc != 3 )
	{
		Tcl_WrongNumArgs ( interp, 2, objv, "option" );
		return GNOCL_CGET_ERROR;
	}

	return gnoclCgetOne ( interp, objv[2], gObj, opts, idx );
}

/**
\note   Should this be move to the text.c module?
**/
int gnoclTagCget (	Tcl_Interp *interp, int objc, Tcl_Obj * const objv[], GObject *gObj, GnoclOption *opts, int *idx )
{

#ifdef DEBUG_TEXT
	g_print ( "%s 1) option = %s\n", __FUNCTION__, Tcl_GetString ( objv[4] ) );
#endif



	if ( objc != 5 )
	{
#ifdef DEBUG_TEXT
		g_print ( "%s 2)\n", __FUNCTION__ );
#endif
		Tcl_WrongNumArgs ( interp, 5, objv, "option" );
		return GNOCL_CGET_ERROR;
	}

#ifdef DEBUG_TEXT
	g_print ( "%s 3) %s %d\n", __FUNCTION__, Tcl_GetString ( objv[4] ), idx );
#endif
	return gnoclCgetOne ( interp, objv[4], gObj, opts, idx );
}


/**
\brief
\author
\date
\note
**/
int gnoclCgetOne ( 	Tcl_Interp *interp,	Tcl_Obj *obj, GObject *gObj, GnoclOption *opts, int *idx )
{

#ifdef DEBUG_PARSEOPTIONS
	g_print ( "%s 1)\n", __FUNCTION__ );
#endif

	GnoclOption *pop;

	if ( gnoclGetIndexFromObjStruct ( interp, obj, ( char ** ) &opts[0].optName, sizeof ( GnoclOption ), "option", TCL_EXACT, idx ) != TCL_OK )
	{

		return GNOCL_CGET_ERROR;
	}

	pop = opts + *idx;

	if ( pop->propName )
	{
		switch ( pop->type )
		{
			case GNOCL_STRING:
				{

					Tcl_Obj *obj;
					gchar   *val;
					g_object_get ( gObj, pop->propName, &val, NULL );
					obj = Tcl_NewStringObj ( val, -1 );
					g_free ( val );
					Tcl_SetObjResult ( interp, obj );
				}

				return GNOCL_CGET_HANDLED;
			case GNOCL_BOOL:
				{

					gboolean val;
					g_object_get ( gObj, pop->propName, &val, NULL );
					Tcl_SetObjResult ( interp, Tcl_NewBooleanObj ( val ) );
				}

				return GNOCL_CGET_HANDLED;
			case GNOCL_INT:
				{
					gint val;
					g_object_get ( gObj, pop->propName, &val, NULL );
					Tcl_SetObjResult ( interp, Tcl_NewIntObj ( val ) );
				}

				return GNOCL_CGET_HANDLED;
			case GNOCL_DOUBLE:
				{
					gdouble val;
					g_object_get ( gObj, pop->propName, &val, NULL );
					Tcl_SetObjResult ( interp, Tcl_NewDoubleObj ( val ) );
				}

				return GNOCL_CGET_HANDLED;

			case GNOCL_OBJ:
				{
#ifdef DEBUG_PARSEOPTIONS
					g_print ( "%s WE HAVE AN OBJECT\n", __FUNCTION__ );
#endif
				}
			default:
				{

					Tcl_Obj *obj = NULL;

					if ( ( *pop->func ) ( interp, pop, gObj, &obj ) != TCL_OK )
						return GNOCL_CGET_ERROR;

					if ( obj == NULL )
					{
						gnoclCgetNotImplemented ( interp, pop );
						return GNOCL_CGET_ERROR;
					}

					Tcl_SetObjResult ( interp, obj );
				}

				return GNOCL_CGET_HANDLED;
		}
	}

	return GNOCL_CGET_NOTHANDLED;
}


/*********************************************************************/

/**
\brief	"populate-popup" signal handler.
**/
static void doOnPopulatePopup ( GtkTextView *textview, GtkMenu *menu, gpointer user_data )
{
	GnoclCommandData *cs = ( GnoclCommandData * ) user_data;

	/* set global variable to hold location of the popup menu */
	popupMenu = menu;

	GnoclPercSubst ps[] =
	{
		{ 'w', GNOCL_STRING },  /* name of widget */
		{ 'g', GNOCL_STRING },  /* glade name */
		{ 0 }
	};

	ps[0].val.str = gnoclGetNameFromWidget ( textview );
	ps[1].val.str = gtk_widget_get_name ( GTK_WIDGET ( textview ) );

	gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 );

}

/**
\brief	Setup to populate-popup signal handler.
**/
int gnoclOptOnPopulatePopup (
	Tcl_Interp *interp,
	GnoclOption *opt,
	GObject *obj,
	Tcl_Obj **ret )
{

	return gnoclConnectOptCmd ( interp, obj, "populate-popup", G_CALLBACK ( doOnPopulatePopup ), opt, NULL, ret );

}


/**
\brief	Add menuitem to the popup menu.
**/
void gnoclPopupMenuAddItem ( Tcl_Interp *interp, gchar *str )
{
#ifdef DEBUG_TEXT
	g_print ( "%s\n", __FUNCTION__ );
#endif


	GtkWidget *item;

	item = gnoclGetWidgetFromName ( str, interp );

	gtk_menu_shell_append ( GTK_MENU_SHELL ( popupMenu ), item );
	gtk_widget_show ( item );
}

/**
\brief	Add a submenu to an existing popup menu item.
**/
void gnoclPopupMenuAddSubMenu ( Tcl_Interp *interp, gchar *str1, gchar *str2 )
{
#ifdef DEBUG_TEXT
	g_print ( "%s\n", __FUNCTION__ );
#endif


	GtkWidget *item, *submenu;
	GtkWidget *i;

	item = gnoclGetWidgetFromName ( str1, interp );

	submenu = gnoclGetWidgetFromName ( str2, interp );

	gtk_menu_item_set_submenu ( GTK_MENU_ITEM ( item ), submenu ) ;

}

/**
\brief	Add separator to the popup menu.
**/
void gnoclPopupMenuAddSeparator ( Tcl_Interp *interp )
{
#ifdef DEBUG_TEXT
	g_print ( "%s\n", __FUNCTION__ );
#endif

	GtkWidget *item;

	item = gtk_separator_menu_item_new();
	gtk_menu_shell_append ( GTK_MENU_SHELL ( popupMenu ), item );
	gtk_widget_show ( item );

}
