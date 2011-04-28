/*
 *  $Id: text.c,v 1.7 2005-01-01 15:27:54 baum Exp $
 *
 *  This file implements the text widget
 *
 *  Copyright (c) 2001 - 2005 Peter G. Baum  http://www.dr-baum.net
 *
 *  See the file "license.terms" for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 *  Modification of the the GtkSourceView undo/redo provided by Ross Burton (ross@burtonini.com)
 */

/*
   History:
   2011-04  added -inputMethod, -onPreeditChanged
   2011-02  added undo/redo functionality
   2010-05	added tag option -paragraph
   2010-04	added -onDeleteFromCursor, -onInsertAtCursor, -onMoveCursor, -onMoveViewport
			added lorem - insert some dummy text for preview purposes
   2010-03	added getIndex, return line / char info at location x y in textview
   2010-01	added -buffer
   2010-01	added -doOnCutClipboard, -doOnCopyClipboard, -doOnPasteClipboard
   2009-01	added -text option to allow plain text creation at startup
   2008-04	renamed function gnoclOptOnInsertText to  gnoclOptOnTextInsert
   2009-04	added commands save, load, (aka serialize, deserialize)

   2009-02	added -onScroll and -widthRequest
   			added commands search and replace
   2009-01	added -heightRequest


   2008-06	added new options -onEntry and -onLeave
   				(required modification of the text widget signal mask)

   2008-06  added   <id> getSelectionBounds, returns range currently selected
                    <id> -onInsertText script


   [not finnished] 2008-03: XYgetCursor -retrive row/col position under window coordinates, XY
   [not finnished] 2008-03: cget retrieve current setting for specified option, based upon entry.c

   2008-03: added tag delete <tagname> command
   2008-03: added command 'class', return class of widget, ie. text
   2007-11: extened tag command to include apply / remove
   2007-11: added extended list of tag options to include
      -onEvent
   2007-10: added new options to the text widget
      -baseFont
          -baseColor
          -onButtonPress
          -onButtonRelease
          -onKeyPress
          -onKeyRelease
          -onMotion
          -dropTargets -test this option
          -dragTargets -test this option
   2003-03: added scrollToPosition
        11: switched from GnoclWidgetOptions to GnoclOption
   2002-05: transition to gtk 2.0: move from gtkText to gtkTextView
   2001-06: Begin of developement

   TODO: marks
 */

/**
\page page_text gnocl::text
\htmlinclude text.html
**/

#include "gnocl.h"
#include <string.h>
#include <assert.h>


static int textFunc ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj *  const objv[] );

/**
\brief     Search through the GtkTextBuffer from startPos to endPos.
           If a match is found, add it to a list of row col indices.
           When search complete, return the list to the calling function.
**/
static int searchWord ( GtkTextBuffer *buffer, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[], int cmdNo, int isTextWidget )
{

	listParameters ( objc,  objv, __FUNCTION__ );

	int res, row1, col1, row2, col2;
	GtkTextIter start, begin, end;
	Tcl_Obj *resList;

	resList = Tcl_NewListObj ( 0, NULL );

	/*  default with the start of the buffer */
	gtk_text_buffer_get_start_iter ( buffer, &start );

	while ( gtk_text_iter_forward_search ( &start, ( gchar*  ) Tcl_GetString ( objv[cmdNo+2] ), 0, &begin, &end, NULL ) != NULL )
	{
#ifdef DEBUG_TEXT
		g_print ( "*  search forwards %s\n", ( gchar*  ) Tcl_GetString ( objv[cmdNo+2] ) );
#endif
		/*  return the index of the found location */
		row1 = gtk_text_iter_get_line ( &begin );
		col1 = gtk_text_iter_get_line_offset ( &begin );
		row2 = gtk_text_iter_get_line ( &end );
		col2 = gtk_text_iter_get_line_offset ( &end );

		Tcl_ListObjAppendElement ( interp, resList, Tcl_NewIntObj ( row1 ) );
		Tcl_ListObjAppendElement ( interp, resList, Tcl_NewIntObj ( col1 ) );
		Tcl_ListObjAppendElement ( interp, resList, Tcl_NewIntObj ( row2 ) );
		Tcl_ListObjAppendElement ( interp, resList, Tcl_NewIntObj ( col2 ) );

		start = end;

	}

	Tcl_SetObjResult ( interp, resList );

	/*  return the total number of items returned in the list */
	return TCL_OK;
}

/**
\brief
**/
static int gnoclOptText ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	GtkWidget *container;
	container =  gtk_widget_get_parent ( obj );

#ifdef DEBUG_TEXT
	printf ( "INSERT SOME INITIAL TEXT\n" );
#endif

	return TCL_OK;
}

/**
\brief
**/
static int gnoclOptTabs ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{

#ifdef DEBUG_TEXT
	g_print ( "SET SOME INITIAL TABS\n" );
#endif

	PangoTabArray *ptaNameList = pango_tab_array_new_with_positions ( 2, TRUE, PANGO_TAB_LEFT, 100, PANGO_TAB_LEFT, 200 );
	gtk_text_view_set_tabs ( GTK_TEXT_VIEW ( obj ), ptaNameList );


	return TCL_OK;
}

/**
\brief
*/
static gboolean doOnTextEnterLeave ( GtkWidget *widget, GdkEventMotion *event, 	gpointer data )
{
	GnoclCommandData *cs = ( GnoclCommandData *  ) data;

	GnoclPercSubst ps[] =
	{
		{ 'w', GNOCL_STRING },  /*  widget */
		{ 0 }
	};

	ps[0].val.str = gnoclGetNameFromWidget ( widget );

	/*  TODO: gnocl::buttonStateToList -> {MOD1 MOD3 BUTTON2...} */
	gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 );
	return 0;
}

/**
\brief
**/
static int gnoclOptTextOnEnterLeave ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	GtkWidget *container;
	container =  gtk_widget_get_parent ( obj );

	assert ( strcmp ( opt->optName, "-onEnter" ) == 0 || strcmp ( opt->optName, "-onLeave" ) == 0 );
	return gnoclConnectOptCmd ( interp, obj, opt->optName[3] == 'E' ? "enter-notify-event" : "leave-notify-event", G_CALLBACK ( doOnTextEnterLeave ), opt, NULL, ret );
}

/**
\brief
**/
static GdkBitmap *getBitmapMask ( gchar *filename )
{
	GdkPixbuf *pbuf;
	GdkBitmap *ret;
	GError *err = NULL;

	g_return_val_if_fail ( filename != NULL, NULL );

	pbuf = gdk_pixbuf_new_from_file ( filename, &err );

	if ( err != NULL )
	{
		g_warning ( "%s", err->message );
		g_error_free ( err );
		return NULL;
	}

	/* you may want to change the threshold, depending on your image */
	gdk_pixbuf_render_pixmap_and_mask ( pbuf, NULL, &ret, 1 );

	//g_object_unref ( pbuf );

	return ret;
}


/**
\brief
**/
static void doOnInsertPixbuf ( GtkTextBuffer *textbuffer, GtkTextIter *location, GdkPixbuf *pixbuf, gpointer user_data )
{
	GnoclCommandData *cs = ( GnoclCommandData * ) user_data;

	GnoclPercSubst ps[] =
	{
		{ 'w', GNOCL_STRING },  /* widget */
		{ 'r', GNOCL_INT },     /* row */
		{ 'c', GNOCL_INT },     /* column */
		{ 'p', GNOCL_INT },		/* pixbuf */
		{ 0 }
	};

	ps[0].val.str = gnoclGetNameFromWidget ( textbuffer );
	ps[1].val.i     = gtk_text_iter_get_line ( location );
	ps[2].val.i     = gtk_text_iter_get_line_offset ( location );
	ps[3].val.str = gnoclGetNameFromPixBuf ( pixbuf );

	gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 );
}


/**
\brief
\author     William J Giddings
\date       30-Apr-09
**/
int gnoclOptOnInsertPB ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{

	/* check the name of the signal is correct for this function */
	assert ( strcmp ( opt->optName, "-onInsertPixBuf" ) == 0 );

	/* connect the signal with its callback function */

	return gnoclConnectOptCmd ( interp, GTK_ENTRY  ( obj ), "insert-pixbuf", G_CALLBACK ( doOnInsertPixbuf ), opt, NULL, ret );

}

/**
\brief     Handles the "toggle-cursor-visible" signal.
\author    William J Giddings
\date      30/04/2010
\since     0.9.95
\note      Used by: gnome::text, Default Binding F7.
\**/
static void doOnToggleCursorVisible ( GtkTextView *text_view, gpointer user_data )
{

	GnoclCommandData *cs = ( GnoclCommandData * ) user_data;

	GnoclPercSubst ps[] =
	{
		{ 'w', GNOCL_STRING },  /* widget name */
		{ 'g', GNOCL_STRING },  /* glade name */
		{ 'v', GNOCL_INT },  	/* visibility, boolean */
		{ 0 }
	};

	ps[0].val.str = gnoclGetNameFromWidget ( text_view );
	ps[1].val.str = gtk_widget_get_name ( text_view );
	ps[2].val.i = gtk_text_view_get_cursor_visible ( text_view );

	gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 );
}

/**
\brief
\author		William J Giddings
\date		30/04/2010
\note
**/
int gnoclOptOnToggleCursorVisible ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	/* check the name of the signal is correct for this function */
	assert ( strcmp ( opt->optName, "-onToggleCursorVisible" ) == 0 );

	/* connect the signal with its callback function */
	return gnoclConnectOptCmd ( interp, obj, "toggle-cursor-visible" , G_CALLBACK ( doOnToggleCursorVisible ), opt, NULL, ret );
}

/**
\brief
    Description yet to be added.
\note
    As long as the options for the GtkScrolledWindow are not set
    automatically, we don't need any special handling in gnoclSetOptions.
*/
static const int scrollBarIdx = 0;
static const int textIdx = 1;
static const int bufferIdx = 2;
static const int useUndoIdx = 3;

static GnoclOption textOptions[] =
{
	/*  textView */

	/*  gnocl-specific options */
	{ "-scrollbar", GNOCL_OBJ, NULL },
	{ "-text", GNOCL_STRING, NULL},
	{ "-buffer", GNOCL_STRING, NULL},
	{ "-useUndo", GNOCL_STRING, NULL},

	/* GtkTextView properties
	"accepts-tab"              gboolean              : Read / Write
	"buffer"                   GtkTextBuffer*        : Read / Write
	"cursor-visible"           gboolean              X
	"editable"                 gboolean              X
	"indent"                   gint                  X
	"justification"            GtkJustification      X
	"left-margin"              gint                  X
	"overwrite"                gboolean              : Read / Write
	"pixels-above-lines"       gint                  X
	"pixels-below-lines"       gint                  X
	"pixels-inside-wrap"       gint                  X
	"right-margin"             gint                  : Read / Write
	"tabs"                     PangoTabArray*        : Read / Write
	"wrap-mode"                GtkWrapMode           : Read / Write
	*/

	{ "-accepttTab", GNOCL_BOOL, "accepts-tab" },
	{ "-cursorVisible", GNOCL_BOOL, "cursor_visible" },
	{ "-editable", GNOCL_BOOL, "editable" },
	{ "-indent", GNOCL_INT, "indent" },
	{ "-justify", GNOCL_OBJ, "justification", gnoclOptJustification },
	{ "-leftMargin", GNOCL_INT, "left_margin" },
	{ "-inputMethod", GNOCL_STRING, "im-module" },

	{ "-overwrite", GNOCL_BOOL, "overwrite" },
	{ "-pixelsBelowLines", GNOCL_INT, "pixels_below_lines" },
	{ "-pixelsAboveLines", GNOCL_INT, "pixels_above_lines" },
	{ "-pixelsInsideWrap", GNOCL_INT, "pixels_inside_wrap" },
	{ "-rightMargin", GNOCL_INT, "right_margin" },

	{ "-tabs", GNOCL_STRING, "tabs", gnoclOptTabs}, /* "tabs" */
	{ "-wrapMode", GNOCL_OBJ, "wrap_mode", gnoclOptWrapmode },

	/* GtkTextBuffer properties
	"copy-target-list"         GtkTargetList*        : Read
	"cursor-position"          gint                  : Read
	"has-selection"            gboolean              : Read
	"paste-target-list"        GtkTargetList*        : Read
	"tag-table"                GtkTextTagTable*      : Read / Write / Construct Only
	"text"                     gchar*                : Read / Write
	*/

	{ "-hasFocus", GNOCL_BOOL, "has-focus" },
	{ "-tooltip", GNOCL_OBJ, "", gnoclOptTooltip },
	{ "-onShowHelp", GNOCL_OBJ, "", gnoclOptOnShowHelp },
	{ "-name", GNOCL_STRING, "name" },
	{ "-visible", GNOCL_BOOL, "visible" },
	{ "-sensitive", GNOCL_BOOL, "sensitive" },
	{ "-baseFont", GNOCL_OBJ, "Sans 14", gnoclOptGdkBaseFont },
	{ "-baseColor", GNOCL_OBJ, "normal", gnoclOptGdkColorBase },

	/* -------- GtkTextView signals --------*/

	/*
	  "backspace"                                      : Run Last / Action
	  "copy-clipboard"                                 : Run Last / Action
	  "cut-clipboard"                                  : Run Last / Action
	  "delete-from-cursor"                             : Run Last / Action
	  "insert-at-cursor"                               : Run Last / Action
	  "move-cursor"                                    : Run Last / Action
	  "move-viewport"                                  : Run Last / Action
	  "page-horizontally"                              : Run Last / Action
	  "paste-clipboard"                                : Run Last / Action
	  "populate-popup"                                 : Run Last
	  "select-all"                                     : Run Last / Action
	  "set-anchor"                                     : Run Last / Action
	  "set-scroll-adjustments"                         : Run Last / Action
	  "toggle-cursor-visible"                          : Run Last / Action
	  "toggle-overwrite"                               : Run Last / Action

	*/

	{ "-onBackspace", GNOCL_OBJ, "", gnoclOptOnBackspace},
	{ "-onCopyClipboard", GNOCL_OBJ, "C", gnoclOptOnClipboard},
	{ "-onCutClipboard", GNOCL_OBJ, "X", gnoclOptOnClipboard},

	{ "-onUndo", GNOCL_OBJ, "U", gnoclOptOnUndoRedo},
	{ "-onRedo", GNOCL_OBJ, "R", gnoclOptOnUndoRedo},

	/* added 29/Apr/2010 */
	{ "-onDeleteFromCursor", GNOCL_OBJ, "", gnoclOptOnDeleteFromCursor},
	{ "-onInsertAtCursor", GNOCL_OBJ, "", gnoclOptOnInsertAtCursor},
	{ "-onMoveCursor", GNOCL_OBJ, "", gnoclOptOnMoveCursor},
	{ "-onMoveViewport", GNOCL_OBJ, "", gnoclOptOnMoveViewport},
	{ "-onPageHorizontally", GNOCL_OBJ, "", gnoclOptOnPageHorizontally},
	{ "-onPasteClipboard", GNOCL_OBJ, "P", gnoclOptOnClipboard},
	{ "-onSelectAll", GNOCL_OBJ, "", gnoclOptOnSelectAll},
	{ "-onPreeditChanged", GNOCL_OBJ, "", gnoclOptOnClipboard},


	/* added 30/Apr/2010 */
	{ "-onSetAnchor", GNOCL_OBJ, "", gnoclOptOnSetAnchor},
	{ "-onSetScrollAdjustments", GNOCL_OBJ, "", gnoclOptOnScrollAdjustments},

	/* check parse*.c code, for errors, or re-enter */
	{ "-onToggleCursorVisible", GNOCL_OBJ, "", gnoclOptOnToggleCursorVisible },
	{ "-onToggleOverWrite", GNOCL_OBJ, "", gnoclOptOnToggleOverwrite},

	/* -------- end of GtkTexView signals */

	{ "-onButtonPress", GNOCL_OBJ, "P", gnoclOptOnButton },
	{ "-onButtonRelease", GNOCL_OBJ, "R", gnoclOptOnButton },
	{ "-onKeyPress", GNOCL_OBJ, "", gnoclOptOnKeyPress },
	{ "-onKeyRelease", GNOCL_OBJ, "", gnoclOptOnKeyRelease },
	{ "-onMotion", GNOCL_OBJ, "", gnoclOptOnMotion },

	{ "-onFocusIn", GNOCL_OBJ, "I", gnoclOptOnFocus },
	{ "-onFocusOut", GNOCL_OBJ, "O", gnoclOptOnFocus },
	{ "-onEnter", GNOCL_OBJ, "E", gnoclOptTextOnEnterLeave },
	{ "-onLeave", GNOCL_OBJ, "L", gnoclOptTextOnEnterLeave },
	{ "-onPopulatePopup", GNOCL_OBJ, "", gnoclOptOnPopulatePopup },

	{ "-heightRequest", GNOCL_INT, "height-request" },
	{ "-widthRequest", GNOCL_INT, "width-request" },

	/* -------- GtkTextBuffer signals -------- */
	{ "-onApplyTag", GNOCL_OBJ, "", gnoclOptOnApplyTag},
	{ "-onBeginUserAction", GNOCL_OBJ, "", gnoclOptOnBeginUserAction},
	{ "-onChanged", GNOCL_OBJ, "", gnoclOptOnChanged},
	{ "-onDeleteRange", GNOCL_OBJ, "", gnoclOptOnDeleteRange},
	{ "-onEndUserAction", GNOCL_OBJ, "", gnoclOptOnEndUserAction},
	{ "-onInsertChildAnchor", GNOCL_OBJ, "", gnoclOptOnInsertChildAnchor},
	{ "-onInsertPixBuf", GNOCL_OBJ, "", gnoclOptOnInsertPB},
	{ "-onInsertText", GNOCL_OBJ, "", gnoclOptOnTextInsert},
	{ "-onMarkDelete", GNOCL_OBJ, "", gnoclOptOnMarkDelete},
	{ "-onMarkSet", GNOCL_OBJ, "", gnoclOptOnMarkSet},
	{ "-onModified", GNOCL_OBJ, "", gnoclOptOnModified}, /* ie. modified-changed */
	{ "-onPasteDone", GNOCL_OBJ, "", gnoclOptOnPasteDone}, /* since Gtk+ 2.15 */
	{ "-onRemoveTag", GNOCL_OBJ, "", gnoclOptOnRemoveTag},

	/*  inherited GtkWidget features */
	{ "-onScroll", GNOCL_OBJ, "", gnoclOptOnScroll },

	/* drag and drop fucntionality taken from box.c */
	{ "-dropTargets", GNOCL_LIST, "t", gnoclOptDnDTargets },
	{ "-dragTargets", GNOCL_LIST, "s", gnoclOptDnDTargets },
	{ "-onDropData", GNOCL_OBJ, "", gnoclOptOnDropData },
	{ "-onDragData", GNOCL_OBJ, "", gnoclOptOnDragData },
	{ "-data", GNOCL_OBJ, "", gnoclOptData },

	{ NULL }
};



/**
\brief      Convert at text index in the form of {row col} into a GtkTextBuffer iter(ator).
\author     Peter G Baum
\date       2001-06:
\bug        (text_cursor_keyWords_test.tcl:13461): Gtk-WARNING **: Invalid text buffer iterator: either the iterator is uninitialized,
            or the characters/pixbufs/widgets in the buffer have been modified since the iterator was created.
            You must use marks, character numbers, or line numbers to preserve a position across buffer modifications.
            You can apply tags and insert marks without invalidating your iterators,
            but any mutation that affects 'indexable' buffer contents (contents that can be referred to by character offset)
            will invalidate all outstanding iterators
            Causes: moving the inter does not act on the buffer, once the iter has been moved, then it n
\todo       Include new keywords
                sentenceStart
                sentenceEnd
                paragraphStart
                paragraphEnd
                wordStart
                wordEnd
                lineEnd
\history
    2008-06-27  Began implementation of new keywords for text position. See TODO.
*/
int posToIter ( Tcl_Interp *interp, Tcl_Obj *obj, GtkTextBuffer *buffer, GtkTextIter *iter )
{
	char errMsg[] = "Position must be either a list of row and column "
					"or a keyword plus offset";
	char errEndOffset[] = "offset to \"end\" must be negative";

	int len;

	/*  error check the arguments passed to the function */

	if ( Tcl_ListObjLength ( interp, obj, &len ) != TCL_OK || len < 1 || len > 2 )
	{
		Tcl_SetResult ( interp, errMsg, TCL_STATIC );
		return TCL_ERROR;
	}

	/*  this is right */

	if ( len == 2 )
	{
		int idx[2];
		int isEnd[2] = { 0, 0 };
		int k;

		for ( k = 0; k < 2; ++k )
		{
			Tcl_Obj *tp;

			if ( Tcl_ListObjIndex ( interp, obj, k, &tp ) != TCL_OK )
			{
				Tcl_SetResult ( interp, errMsg, TCL_STATIC );
				return TCL_ERROR;
			}

			if ( Tcl_GetIntFromObj ( NULL, tp, idx + k ) != TCL_OK )
			{
				char *txt = Tcl_GetString ( tp );

				if ( strncmp ( txt, "end", 3 ) == 0 )
				{
					if ( gnoclPosOffset ( interp, txt + 3, idx + k ) != TCL_OK )
						return TCL_ERROR;

					if ( idx[k] > 0 )
					{
						Tcl_SetResult ( interp, errEndOffset, TCL_STATIC );
						return TCL_ERROR;
					}

					isEnd[k] = 1;
				}

				else
				{
					Tcl_AppendResult ( interp, "unknown row or column index \"", txt, "\" must be integer or end plus offset" );
					return TCL_ERROR;
				}

			}
		}

		gtk_text_buffer_get_start_iter ( buffer, iter );

		if ( isEnd[0] )
		{
			gtk_text_iter_set_line ( iter, -1 );
			gtk_text_iter_backward_lines ( iter, -idx[0] );
		}

		else
			gtk_text_iter_set_line ( iter, idx[0] );

		if ( isEnd[0] )
		{
			gtk_text_iter_forward_to_line_end ( iter );
			gtk_text_iter_backward_chars ( iter, -idx[1] );
		}

		else
			gtk_text_iter_forward_chars ( iter, idx[1] );
	}

	else if ( Tcl_GetIntFromObj ( NULL, obj, &len ) == TCL_OK )
	{
		if ( len < 0 )
		{
			Tcl_SetResult ( interp, "character offset must be greater zero.", TCL_STATIC );
			return TCL_ERROR;
		}

		gtk_text_buffer_get_iter_at_offset ( buffer, iter, len );
	}

	else
	{
		const char *txt = Tcl_GetString ( obj );
		const char *last;
		int offset;

		/*  get a fresh iterator, it may have already been altered due to a previous call */
		gtk_text_buffer_get_iter_at_mark ( buffer, iter, gtk_text_buffer_get_insert ( buffer ) );


		if ( strncmp ( txt, "start", 5 ) == 0 )
		{
#ifdef DEBUG_TEXT
			g_print ( "checking start\n" );
#endif
			gtk_text_buffer_get_start_iter ( buffer, iter );
			last = txt + 5;
		}

		else if ( strncmp ( txt, "end", 3 ) == 0 )
		{
#ifdef DEBUG_TEXT
			g_print ( "checking end\n" );
#endif
			gtk_text_buffer_get_end_iter ( buffer, iter );
			last = txt + 3;
		}

		else if ( strncmp ( txt, "cursor", 6 ) == 0 )
		{
#ifdef DEBUG_TEXT
			g_print ( "checking cursor\n" );
#endif
			last = txt + 6;
			gtk_text_buffer_get_iter_at_mark ( buffer, iter, gtk_text_buffer_get_insert ( buffer ) );
		}

		else if ( strncmp ( txt, "selectionStart", 14 ) == 0 )
		{
#ifdef DEBUG_TEXT
			g_print ( "checking selectionStart\n" );
#endif
			GtkTextIter end;
			gtk_text_buffer_get_selection_bounds ( buffer, iter, &end );
			last = txt + 14;
		}

		else if ( strncmp ( txt, "selectionEnd", 12 ) == 0 )
		{
#ifdef DEBUG_TEXT
			g_print ( "checking selectionEnd\n" );
#endif
			GtkTextIter start;
			gtk_text_buffer_get_selection_bounds ( buffer, &start, iter );
			last = txt + 12;
		}

		else if ( strncmp ( txt, "wordStart", 9 ) == 0 )
		{
#ifdef DEBUG_TEXT
			g_print ( "checking wordStart\n" );
#endif
			/*  get a fresh iterator, it may have already been altered */
			//gtk_text_buffer_get_iter_at_mark( buffer, iter, gtk_text_buffer_get_insert( buffer ) );
			gtk_text_iter_backward_word_start ( iter );
			last = txt + 9;
		}

		else if ( strncmp ( txt, "wordEnd", 7 ) == 0 )
		{
#ifdef DEBUG_TEXT
			g_print ( "checking wordEnd\n" );
#endif
			/*  get a fresh iterator, it may have already been altered */
			//gtk_text_buffer_get_iter_at_mark( buffer, iter, gtk_text_buffer_get_insert( buffer ) );
			gtk_text_iter_forward_word_end ( iter );
			last = txt + 7;
		}

		/*  WJG CURRENTLY WORKING HERE. Nothing happening, also text insert now kaput! */

		else if ( strncmp ( txt, "sentenceStart", 13 ) == 0 )
		{
#ifdef DEBUG_TEXT
			printf ( "checking sentenceStart\n" );
#endif
			/*  get a fresh iterator, it may have already been altered */
			//gtk_text_buffer_get_iter_at_mark( buffer, iter, gtk_text_buffer_get_insert( buffer ) );
			gtk_text_iter_backward_sentence_start ( iter );
			last = txt + 13;
		}

		else if ( strncmp ( txt, "sentenceEnd", 11 ) == 0 )
		{
#ifdef DEBUG_TEXT
			printf ( "checking sentenceEnd\n" );
#endif
			/*  get a fresh iterator, it may have already been altered */
			//gtk_text_buffer_get_iter_at_mark( buffer, iter, gtk_text_buffer_get_insert( buffer ) );
			gtk_text_iter_forward_sentence_end ( iter );
			last = txt + 11;
		}

		else if ( strncmp ( txt, "lineStart", 9 ) == 0 )
		{
#ifdef DEBUG_TEXT
			printf ( "checking lineStart\n" );
#endif
			/*  move iterator to an offset of 0 */
			/*  get a fresh iterator, it may have already been altered */
			//gtk_text_buffer_get_iter_at_mark( buffer, iter, gtk_text_buffer_get_insert( buffer ) );
			gtk_text_iter_backward_visible_line ( iter );
			last = txt + 9;
		}

		else if ( strncmp ( txt, "lineEnd", 7 ) == 0 )
		{
#ifdef DEBUG_TEXT
			g_print ( "checking lineEnd\n" );
#endif
			/*  move iterator to the start of the next line, then move back one offset */
			/*  get a fresh iterator, it may have already been altered */
			//gtk_text_buffer_get_iter_at_mark( buffer, iter, gtk_text_buffer_get_insert( buffer ) );
			gtk_text_iter_forward_visible_line ( iter );
			last = txt + 7;
		}

		else
		{
			Tcl_AppendResult ( interp, "unknown index \"", txt,
							   "\", must be a list of row and column, "
							   "an integer as character offset, "
							   "or one of start, end, cursor, wordStart, wordEnd, sentenceStart, sentenceEnd, lineStart, lineEnd, selectionStart, or selectionEnd",
							   NULL );
			return TCL_ERROR;
		}

		if ( gnoclPosOffset ( interp, last, &offset ) != TCL_OK )
			return TCL_ERROR;

		if ( offset > 0 )
			gtk_text_iter_forward_chars ( iter, offset );
		else if ( offset < 0 )
			gtk_text_iter_backward_chars ( iter, -offset );
	}

	return TCL_OK;
}

/**
\brief      Apply a lists of tags to the a specified range of text.
**/
static int applyTag ( GtkTextBuffer *buffer, Tcl_Interp *interp, int objc, Tcl_Obj *  const objv[], int cmdNo )
{
	/**
	Or, does it appear here?
	*/
	/*  declare some variables */
	// console error messaging
	GnoclOption insertOptions[] =
	{
		{ "-tags", GNOCL_LIST, NULL },
		{ NULL }
	};
	const int tagsIdx = 0;
	gint      startOffset;
	gint   endOffset;
	int       ret = TCL_ERROR;

	GtkTextIter   iter;
	GtkTextIter   iter2;

	/*  The arguments passed in the tcl script are in the objv array. These are:
	0:  {0 0}       fromIndex
	+1:     {0 end}   toIndex
	+2:       -tags
	+3:     bold cursor     taglist
	*/

	/*
	printf ( "-2: {%s} \n-1: {%s} \n0: {%s} \n+1: {%s} \n+2: %s \n+3: %s \n",
	 gnoclGetString ( objv[cmdNo-2] ),
	 gnoclGetString ( objv[cmdNo-1] ),
	 gnoclGetString ( objv[cmdNo] ),
	 gnoclGetString ( objv[cmdNo+1] ),
	 gnoclGetString ( objv[cmdNo+2] ),
	 gnoclGetString ( objv[cmdNo+3] ) );
	*/

	// "position text ?-option val ...?"

	if ( objc < cmdNo + 2 )
	{
		Tcl_WrongNumArgs ( interp, cmdNo, objv, "fromIndex toIndex -tags {tag1 tag2...}S" );
		return TCL_ERROR;
	}

	/*  determine some value */
	// get position of fromIndex within the buffer as in iterator

	if ( posToIter ( interp, objv[cmdNo], buffer, &iter ) != TCL_OK )
	{
		return TCL_ERROR;
	}

	// get position of toIndex within the buffer as in iterator

	if ( posToIter ( interp, objv[cmdNo+1], buffer, &iter2 ) != TCL_OK )
	{
		return TCL_ERROR;
	}

	/*  parse options, check to see if they are suitable */

	if ( gnoclParseOptions ( interp, objc - cmdNo - 1, objv + cmdNo + 1, insertOptions ) != TCL_OK )
	{
		goto clearExit;
	}

	startOffset = gtk_text_iter_get_offset ( &iter );

	endOffset = gtk_text_iter_get_offset ( &iter2 );

	/*  add the tags, gets a list and then works through them */

	if ( insertOptions[tagsIdx].status == GNOCL_STATUS_CHANGED )
	{
		GtkTextIter start;
		GtkTextIter end;
		int k, no;
		Tcl_Obj *obj = insertOptions[tagsIdx].val.obj;

		/*  get the offset position for the inserted text*/
		gtk_text_buffer_get_iter_at_offset ( buffer, &start, startOffset );
		gtk_text_buffer_get_iter_at_offset ( buffer, &end, endOffset );

		if ( Tcl_ListObjLength ( interp, obj, &no ) != TCL_OK )
		{
			goto clearExit;
		}

		/*  apply each tag in turn */

		for ( k = 0; k < no; ++k )
		{
			Tcl_Obj *tp;

			if ( Tcl_ListObjIndex ( interp, obj, k, &tp ) != TCL_OK )
			{
				Tcl_SetResult ( interp, "Could not read tag list", TCL_STATIC );
				goto clearExit;
			}

			gtk_text_buffer_apply_tag_by_name ( buffer, Tcl_GetString ( tp ), &start, &end );

		}
	}

	ret = TCL_OK;

clearExit:
	gnoclClearOptions ( insertOptions );

	return ret;
}

/**
\brief	Remove specific tag from a tag tagtable
**/
static void deleteTag  ( GtkTextTag *tag, gpointer data )
{
	gtk_text_tag_table_remove ( data, tag );
}


/**
\brief
\note
**/
static int removeTag ( GtkTextBuffer *buffer, Tcl_Interp *interp, int objc, Tcl_Obj *  const objv[], int cmdNo )
{
	/*  declare some variables */
	// console error messaging
	GnoclOption insertOptions[] =
	{
		{ "-tags", GNOCL_LIST, NULL },
		{ NULL }
	};
	const int tagsIdx = 0;
	gint      startOffset;
	gint   endOffset;
	int       ret = TCL_ERROR;

	GtkTextIter   iter;
	GtkTextIter   iter2;

	/*  The arguments passed in the tcl script are in the objv array. These are:
	0:  {0 0}       fromIndex
	+1:     {0 end}   toIndex
	+2:       -tags
	+3:     bold cursor     taglist
	*/
	/*
	printf ( "-2: {%s} \n-1: {%s} \n0: {%s} \n+1: {%s} \n+2: %s \n+3: %s \n",
	 gnoclGetString ( objv[cmdNo-2] ),
	 gnoclGetString ( objv[cmdNo-1] ),
	 gnoclGetString ( objv[cmdNo] ),
	 gnoclGetString ( objv[cmdNo+1] ),
	 gnoclGetString ( objv[cmdNo+2] ),
	 gnoclGetString ( objv[cmdNo+3] ) );
	*/
	// "position text ?-option val ...?"

	if ( objc < cmdNo + 2 )
	{
		Tcl_WrongNumArgs ( interp, cmdNo, objv, "fromIndex toIndex -tags {tag1 tag2...}S" );
		return TCL_ERROR;
	}

	/*  determine some value */
	// get position of fromIndex within the buffer as in iterator

	if ( posToIter ( interp, objv[cmdNo], buffer, &iter ) != TCL_OK ) return TCL_ERROR;

	// get position of toIndex within the buffer as in iterator
	if ( posToIter ( interp, objv[cmdNo+1], buffer, &iter2 ) != TCL_OK ) return TCL_ERROR;

	/*  parse options, check to see if they are suitable */
	if ( gnoclParseOptions ( interp, objc - cmdNo - 1, objv + cmdNo + 1, insertOptions ) != TCL_OK ) goto clearExit;

	startOffset = gtk_text_iter_get_offset ( &iter );

	endOffset = gtk_text_iter_get_offset ( &iter2 );

	/*  add the tags, gets a list and then works through them */
	if ( insertOptions[tagsIdx].status == GNOCL_STATUS_CHANGED )
	{
		GtkTextIter start;
		GtkTextIter end;
		int k, no;
		Tcl_Obj *obj = insertOptions[tagsIdx].val.obj;

		/*  get the offset position for the inserted text*/
		gtk_text_buffer_get_iter_at_offset ( buffer, &start, startOffset );
		gtk_text_buffer_get_iter_at_offset ( buffer, &end, endOffset );

		if ( Tcl_ListObjLength ( interp, obj, &no ) != TCL_OK ) goto clearExit;

		/*  apply each tag in turn */
		for ( k = 0; k < no; ++k )
		{
			Tcl_Obj *tp;

			if ( Tcl_ListObjIndex ( interp, obj, k, &tp ) != TCL_OK )
			{
				Tcl_SetResult ( interp, "Could not read tag list", TCL_STATIC );
				goto clearExit;
			}

			gtk_text_buffer_remove_tag_by_name ( buffer, Tcl_GetString ( tp ), &start, &end );

		}
	}

	ret = TCL_OK;

clearExit:
	gnoclClearOptions ( insertOptions );

	return ret;
}

/**
\brief      To implement working bindings to GtkTextMark functions.
\author     William J Giddings
\date       31/Jun/2008
\note
**/
static int markCmd ( GtkTextBuffer *buffer, Tcl_Interp *interp, int objc, Tcl_Obj *  const objv[], int cmdNo )
{
	const char  *cmds[] = { "create", "configure", "delete", "move", "names", "cget", "getIndex", NULL };
	enum   cmdIdx { CreateIdx, ConfigureIdx, DeleteIdx, MoveIdx, NamesIdx, CgetIdx, GetIndexIdx };
	int     idx, row, col;
	GtkTextIter iter;
	GtkTextMark *mark;
	Tcl_Obj  *resList;

	GnoclOption markOptions[] =
	{
		{ "-visible", GNOCL_BOOL, "invisible" },
		{ "-gravity", GNOCL_BOOL, "left-gravity" },
		{ NULL }
	};

	/*  check the script for errors */

	if ( objc < cmdNo + 1 )
	{
		Tcl_WrongNumArgs ( interp, cmdNo, objv, "subcommand ?option val ...?" );
		return TCL_ERROR;
	}

	if ( Tcl_GetIndexFromObj ( interp, objv[cmdNo], cmds, "subcommand", TCL_EXACT, &idx ) != TCL_OK )
	{
		return TCL_ERROR;
	}


	switch ( idx )
	{
		case CreateIdx:
			{
				char *str;
				Tcl_Obj       *resList;
#ifdef DEBUG_TEXT
				printf ( "markCmd 1>  create %sIdx mark: %s position: { %s } \n",
						 Tcl_GetString ( objv[cmdNo+0] ) ,
						 Tcl_GetString ( objv[cmdNo+1] ) ,
						 Tcl_GetString ( objv[cmdNo+2] ) );
#endif

				if ( objc < cmdNo + 2 )
				{
					Tcl_WrongNumArgs ( interp, cmdNo + 1, objv, "mark-name ?option val ...?" );
					return TCL_ERROR;
				}

				/*  convert position to inter */

				if ( posToIter ( interp, objv[cmdNo+2], buffer, &iter ) != TCL_OK )
				{
					/*  defaul to cursor position */
					return TCL_ERROR;
				}

				gtk_text_buffer_create_mark ( buffer, Tcl_GetString ( objv[cmdNo+1] ), &iter, 0 );

				/*  return the name of the mark created */
				resList = Tcl_NewListObj ( 0, NULL );
				str = Tcl_GetString ( objv[cmdNo+1] );
				Tcl_ListObjAppendElement ( interp, resList, Tcl_NewStringObj ( str, strlen ( str ) ) );
				Tcl_SetObjResult ( interp, resList );

				return TCL_OK ;

			}

		case ConfigureIdx:
			{

				if ( objc < cmdNo + 2 )
				{
					Tcl_WrongNumArgs ( interp, cmdNo + 1, objv, "mark-name ?option val ...?" );
					return TCL_ERROR;
				}

#ifdef DEBUG_TEXT
				printf ( "markCmd ConfigureIdx> %s %s %s %s} \n",
						 Tcl_GetString ( objv[cmdNo+0] ) ,
						 Tcl_GetString ( objv[cmdNo+1] ) ,
						 Tcl_GetString ( objv[cmdNo+2] ) ,
						 Tcl_GetString ( objv[cmdNo+3] ) );
#endif

				mark = gtk_text_buffer_get_mark ( buffer, Tcl_GetString ( objv[cmdNo+1] ) );

				/*  check the options flag */

				if  ( strcmp ( Tcl_GetString ( objv[cmdNo+2] ), "-visible" ) )
				{
#ifdef DEBUG_TEXT
					printf ( "markCmd 3>\n" );
#endif
					/*  apply the setting */

					if ( Tcl_GetString ( objv[cmdNo+3] ) )
					{
						gtk_text_mark_set_visible ( mark, 1 );
					}

					else
					{
						gtk_text_mark_set_visible ( mark, 0 );
					}

				}

				else if  ( strcmp ( Tcl_GetString ( objv[cmdNo+2] ), "-gravity" ) )
				{
					/*  NOTE:
					 *  There is no Gtk lib function to reset the gravity of a marker.
					 *  To achieve this, the marker first needs to be deleted and then
					 *  a new recreated with the same name and a complementary left-gravity
					 *  setting. Right-gravity is necessary for right-to-left written scripts
					 *  such as Hebrew and Arabic.
					 */
#ifdef DEBUG_TEXT
					printf ( "markCmd 4>\n" );
#endif
					/*  apply the setting */

					if ( strcmp ( Tcl_GetString ( objv[cmdNo+3] ), "left" ) )
					{
						gtk_text_mark_set_visible ( mark, 1 );
					}

					else
						/*  reverts to default, ie left-gravity 0 */
					{
						gtk_text_mark_set_visible ( mark, 1 );
					}

				}

				return TCL_OK;

			}

		case DeleteIdx:
			{
				gtk_text_buffer_delete_mark_by_name ( buffer, Tcl_GetString ( objv[cmdNo+1] ) );
				return TCL_OK;
			}

		case MoveIdx:
			{

				/*  convert the new position to a GtkTextIter */
				posToIter ( interp, objv[cmdNo+2] , buffer, &iter );

				/*  move names mark to a new location within the buffer */
				gtk_text_buffer_move_mark_by_name ( buffer, Tcl_GetString ( objv[cmdNo+1] ), &iter );

				return TCL_OK;

			}

		case NamesIdx:
			{
				// return a list of all the marks associated with the textBuffer, such a query is not supported within Gtk
			}

		case CgetIdx:
			{
				/*  if cget implmented, then to get the position of a mark
				*
				*  void gtk_text_buffer_get_iter_at_mark (GtkTextBuffer *buffer, GtkTextIter *iter, GtkTextMark *mark);
				*  gboolean gtk_text_mark_get_visible (GtkTextMark *mark);
				*  gboolean gtk_text_mark_get_left_gravity (GtkTextMark *mark);
				*/

			}

		case GetIndexIdx:
			{
#ifdef DEBUG_TEXT
				printf ( "GetIndex\n" );
#endif
				/*  return the index of the named marker */
				mark = gtk_text_buffer_get_mark ( buffer, Tcl_GetString ( objv[cmdNo+1] ) );
				gtk_text_buffer_get_iter_at_mark ( buffer, &iter, mark );

				/*  the following block is hacked from GetCursorIdx */
				row = gtk_text_iter_get_line ( &iter );
				col = gtk_text_iter_get_line_offset ( &iter );

				resList = Tcl_NewListObj ( 0, NULL );
				Tcl_ListObjAppendElement ( interp, resList, Tcl_NewIntObj ( row ) );
				Tcl_ListObjAppendElement ( interp, resList, Tcl_NewIntObj ( col ) );
				Tcl_SetObjResult ( interp, resList );

			}

		default:
			{
				//assert ( 0 );
				//return TCL_ERROR;
			}
	}

	return TCL_OK;

}

/**
\brief
\author
\date
\note
 "accumulative-margin"      gboolean              : Read / Write
  "background"               gchar*                : Write
  "background-full-height"   gboolean              : Read / Write
  "background-full-height-set" gboolean              : Read / Write
  "background-gdk"           GdkColor*             : Read / Write
  "background-set"           gboolean              : Read / Write
  "background-stipple"       GdkPixmap*            : Read / Write
  "background-stipple-set"   gboolean              : Read / Write
  "direction"                GtkTextDirection      : Read / Write
  "editable"                 gboolean              : Read / Write
  "editable-set"             gboolean              : Read / Write
  "family"                   gchar*                : Read / Write
  "family-set"               gboolean              : Read / Write
  "font"                     gchar*                : Read / Write
  "font-desc"                PangoFontDescription*  : Read / Write
  "foreground"               gchar*                : Write
  "foreground-gdk"           GdkColor*             : Read / Write
  "foreground-set"           gboolean              : Read / Write
  "foreground-stipple"       GdkPixmap*            : Read / Write
  "foreground-stipple-set"   gboolean              : Read / Write
  "indent"                   gint                  : Read / Write
  "indent-set"               gboolean              : Read / Write
  "invisible"                gboolean              : Read / Write
  "invisible-set"            gboolean              : Read / Write
  "justification"            GtkJustification      : Read / Write
  "justification-set"        gboolean              : Read / Write
  "language"                 gchar*                : Read / Write
  "language-set"             gboolean              : Read / Write
  "left-margin"              gint                  : Read / Write
  "left-margin-set"          gboolean              : Read / Write
  "name"                     gchar*                : Read / Write / Construct Only
  "paragraph-background"     gchar*                : Write
  "paragraph-background-gdk" GdkColor*             : Read / Write
  "paragraph-background-set" gboolean              : Read / Write
  "pixels-above-lines"       gint                  : Read / Write
  "pixels-above-lines-set"   gboolean              : Read / Write
  "pixels-below-lines"       gint                  : Read / Write
  "pixels-below-lines-set"   gboolean              : Read / Write
  "pixels-inside-wrap"       gint                  : Read / Write
  "pixels-inside-wrap-set"   gboolean              : Read / Write
  "right-margin"             gint                  : Read / Write
  "right-margin-set"         gboolean              : Read / Write
  "rise"                     gint                  : Read / Write
  "rise-set"                 gboolean              : Read / Write
  "scale"                    gdouble               : Read / Write
  "scale-set"                gboolean              : Read / Write
  "size"                     gint                  : Read / Write
  "size-points"              gdouble               : Read / Write
  "size-set"                 gboolean              : Read / Write
  "stretch"                  PangoStretch          : Read / Write
  "stretch-set"              gboolean              : Read / Write
  "strikethrough"            gboolean              : Read / Write
  "strikethrough-set"        gboolean              : Read / Write
  "style"                    PangoStyle            : Read / Write
  "style-set"                gboolean              : Read / Write
  "tabs"                     PangoTabArray*        : Read / Write
  "tabs-set"                 gboolean              : Read / Write
  "underline"                PangoUnderline        : Read / Write
  "underline-set"            gboolean              : Read / Write
  "variant"                  PangoVariant          : Read / Write
  "variant-set"              gboolean              : Read / Write
  "weight"                   gint                  : Read / Write
  "weight-set"               gboolean              : Read / Write
  "wrap-mode"                GtkWrapMode           : Read / Write
  "wrap-mode-set"            gboolean              : Read / Write

**/
int tagCmd ( GtkTextBuffer *buffer, Tcl_Interp *interp, int objc, Tcl_Obj *  const objv[], int cmdNo )
{
	// widgetname tag add tagname start_index end_index
	// widgetname tag remove tagname start_index end_index

	// const char *cmds[] = { "create", "configure", "add", "delete", "remove", NULL };
	// enum cmdIdx { CreateIdx, ConfigureIdx, AddIdx, DeleteIdx};
	const char *cmds[] =
	{
		"cget", "create", "configure", "apply",
		"delete", "remove", "get", "clear",
		NULL
	};
	enum cmdIdx
	{
		CgetIdx, CreateIdx, ConfigureIdx, ApplyIdx,
		DeleteIdx, RemoveIdx, GetIdx, ClearIdx
	};

	/*  see also list.c */

	GnoclOption tagOptions[] =
	{
		{ "-paragraph", GNOCL_OBJ, "paragraph-background-gdk", gnoclOptGdkColor },
		{ "-background", GNOCL_OBJ, "background-gdk", gnoclOptGdkColor },
		{ "-foreground", GNOCL_OBJ, "foreground-gdk", gnoclOptGdkColor },
		{ "-font", GNOCL_STRING, "font" },
		{ "-fontFamily", GNOCL_STRING, "family" },
		{ "-fontStyle", GNOCL_OBJ, "style", gnoclOptPangoStyle },
		{ "-fontVariant", GNOCL_OBJ, "variant", gnoclOptPangoVariant },
		{ "-fontWeight", GNOCL_OBJ, "weight", gnoclOptPangoWeight },
		{ "-fontRise", GNOCL_OBJ, "rise", gnoclOptPangoScaledInt },
		{ "-fontStretch", GNOCL_OBJ, "stretch", gnoclOptPangoStretch },
		{ "-fontSize", GNOCL_OBJ, "size", gnoclOptPangoScaledInt },
		{ "-fontScale", GNOCL_OBJ, "scale", gnoclOptScale },
		{ "-wrapMode", GNOCL_OBJ, "wrap-mode", gnoclOptWrapmode },
		{ "-justification", GNOCL_OBJ, "justification", gnoclOptJustification },
		{ "-strikethrough", GNOCL_BOOL, "strikethrough" },
		{ "-underline", GNOCL_OBJ, "underline", gnoclOptUnderline },
		{ "-invisible", GNOCL_BOOL, "invisible" },
		{ "-editable", GNOCL_BOOL, "editable" },

		{ "-pixelsBelowLines", GNOCL_INT, "pixels-below-lines" },
		{ "-pixelsAboveLines", GNOCL_INT, "pixels-above-lines" },
		{ "-size", GNOCL_INT, "size" },
		{ "-sizePoints", GNOCL_DOUBLE, "size-points" },


		/*  GtkTextTag signal */
		{ "-onEvent", GNOCL_OBJ, "", gnoclOptOnEvent },
		{ NULL }
	};

	int idx;

	if ( objc < cmdNo + 1 )
	{
		Tcl_WrongNumArgs ( interp, cmdNo, objv, "subcommand ?option val ...?" );
		return TCL_ERROR;
	}

	if ( Tcl_GetIndexFromObj ( interp, objv[cmdNo], cmds, "subcommand", TCL_EXACT, &idx ) != TCL_OK )
	{
		return TCL_ERROR;
	}

	switch ( idx )
	{
		case ClearIdx:
			{
				GtkTextTagTable *tagtable = gtk_text_buffer_get_tag_table ( buffer );

				/*  get the settings of each tag in the buffer*/
				/*  note, pass the address of the pointer to the data assigned by the called function */
				gtk_text_tag_table_foreach ( tagtable, deleteTag , tagtable );

			}
			break;
		case CgetIdx:
			{
				printf ( "Cget not yet implemented\n" );
				break;
			}

			/*  return a list of tags applied to current iter */
		case GetIdx:
			{
				//int ret;
				GSList *p, *tagList;
				GtkTextIter iter;

				Tcl_Obj *resList;

				Tcl_Obj *err;
				Tcl_Obj *ret;

				ret = 0;

				/*  some DEBUG_TEXTging feedback */

#ifdef DEBUG_TEXT
				printf ( "Get tags for position: %s\n", Tcl_GetString ( objv[cmdNo+1] ) ) ;
#endif

				/*  check the arguments */

				if ( objc != cmdNo + 1 )
				{
					Tcl_WrongNumArgs ( interp, cmdNo + 1, objv, "position" );
					return TCL_ERROR;
				}

				/*  convert the position to a pointer */

				if ( posToIter ( interp, objv[cmdNo+1], buffer, &iter ) != TCL_OK )
				{
					return -1;
				}

				/*  Get the tags. But, remember, tagList is a list of tag structures! */
				tagList = gtk_text_iter_get_tags ( &iter );

				/*  initialise list for return to the inpreter */
				resList = Tcl_NewListObj ( 0, NULL );

				/*  build up a list of names */
				for ( p = tagList; p != NULL; p = p->next )
				{
					gchar *name = ( GTK_TEXT_TAG ( p->data )->name );

					Tcl_ListObjAppendElement ( interp, resList, Tcl_NewStringObj ( name, -1 ) );
				}

				/*  tidy up and return the answer */
				g_slist_free ( tagList );

				Tcl_SetObjResult ( interp, resList );

			}

			break ;
		case CreateIdx:
			{
				int ret;
				GtkTextTag *tag;
				/*  win tag create name */

				if ( objc < cmdNo + 2 )
				{
					Tcl_WrongNumArgs ( interp, cmdNo + 1, objv, "tag-name ?option val ...?" );
					return TCL_ERROR;
				}

				/*  tag = gtk_text_tag_new( Tcl_GetString( objv[3] ) ); */
				tag = gtk_text_buffer_create_tag ( buffer, Tcl_GetString ( objv[cmdNo+1] ), NULL );

				ret = gnoclParseAndSetOptions ( interp, objc - cmdNo - 1, objv + cmdNo + 1, tagOptions, G_OBJECT ( tag ) );

				gnoclClearOptions ( tagOptions );

				return ret;
			}

		case ConfigureIdx:
			{
				int        ret;
				GtkTextTag *tag;

				/*  win tag create name */

				if ( objc < cmdNo + 2 )
				{
					Tcl_WrongNumArgs ( interp, cmdNo + 1, objv, "tag-name ?option val ...?" );
					return TCL_ERROR;
				}

				/*  TODO? first reset options */
				tag = gtk_text_tag_table_lookup ( gtk_text_buffer_get_tag_table ( buffer ), Tcl_GetString ( objv[cmdNo+1] ) );

				if ( tag == NULL )
				{
					Tcl_AppendResult ( interp, "Unknown tag \"", Tcl_GetString ( objv[cmdNo+1] ), "\"", NULL );
					return TCL_ERROR;
				}

				ret = gnoclParseAndSetOptions ( interp, objc - cmdNo - 1, objv + cmdNo + 1, tagOptions, G_OBJECT ( tag ) );

				gnoclClearOptions ( tagOptions );

				return ret;
			}

		case ApplyIdx:
			{
				if ( applyTag ( buffer, interp, objc, objv, cmdNo + 1 ) != TCL_OK ) return -1;

				break;
			}

		case RemoveIdx:
			{
				if ( removeTag ( buffer, interp, objc, objv, cmdNo + 1 ) != TCL_OK ) return -1;

				break;
			}

		case DeleteIdx:
			{
				/*! WJG Added 28/03/08 */
				GtkTextTagTable *tagtable = gtk_text_buffer_get_tag_table ( buffer );
				GtkTextTag *tag = gtk_text_tag_table_lookup ( tagtable, Tcl_GetString ( objv[cmdNo+1] ) );

				if ( tag != NULL )
				{

					gtk_text_tag_table_remove ( tagtable, tag );
				}

				break;
			}

		default:
			{
				assert ( 0 );
				return TCL_ERROR;
			}
	}

	return TCL_OK;
}

/**
**/
static int scrollToMark (
	GtkTextView *view,
	GtkTextBuffer *buffer,
	Tcl_Interp *interp,
	int objc,
	Tcl_Obj *  const objv[] )
{
#ifdef DEBUG_TEXT
	printf ( "scrollToMark 1>  %s \n", Tcl_GetString ( objv[2] ) );
#endif

	GtkTextMark *mark;

	if ( objc < 3 )
	{
		Tcl_WrongNumArgs ( interp, 2, objv, "index ?-option val ...?" );
		return TCL_ERROR;
	}

	/*  check to see that the mark exists */
	mark = gtk_text_buffer_get_mark ( buffer, Tcl_GetString ( objv[2] ) );

	/*  reposition */
	if ( mark == NULL )
	{
		Tcl_SetResult ( interp, "This mark does not exist.", TCL_STATIC );
		return TCL_ERROR;
	}

	gtk_text_view_scroll_mark_onscreen  ( view, mark );

	return TCL_OK;

}

/**
\brief
\author
\date
\note
**/
static int scrollToPos (
	GtkTextView *view,
	GtkTextBuffer *buffer,
	Tcl_Interp *interp,
	int objc,
	Tcl_Obj *  const objv[] )
{

#ifdef DEBUG_TEXT
	printf ( "scrollToPos 1>  %s \n",  Tcl_GetString ( objv[2] ) );
#endif

	GnoclOption options[] =

	{
		{ "-margin", GNOCL_DOUBLE, NULL },    /*  0 */
		{ "-align", GNOCL_OBJ, NULL },        /*  1 */
		{ NULL }
	};
	const int marginIdx = 0;
	const int alignIdx  = 1;

	int   ret = TCL_ERROR;

	double      margin = .0;
	int         useAlign = 0;
	gfloat      xAlign = 0.5, yAlign = 0.5;
	GtkTextIter iter;
	GtkTextMark *mark;

	if ( objc < 3 )
	{
		Tcl_WrongNumArgs ( interp, 2, objv, "index ?-option val ...?" );
		return TCL_ERROR;
	}

	if ( posToIter ( interp, objv[2], buffer, &iter ) != TCL_OK )
	{
		return TCL_ERROR;
	}

	if ( gnoclParseOptions ( interp, objc - 2, objv + 2, options ) != TCL_OK )
	{
		goto clearExit;
	}

	if ( options[alignIdx].status == GNOCL_STATUS_CHANGED )
	{
		if ( gnoclGetBothAlign ( interp, options[alignIdx].val.obj, &xAlign, &yAlign ) != TCL_OK )
		{
			goto clearExit;
		}

		useAlign = 1;
	}

	if ( options[marginIdx].status == GNOCL_STATUS_CHANGED )
	{
		margin = options[marginIdx].val.d;

		if ( margin < 0.0 || margin >= 0.5 )
		{
			Tcl_SetResult ( interp, "-margin must be between 0 and 0.5", TCL_STATIC );
			goto clearExit;
		}
	}


	mark = gtk_text_buffer_create_mark ( buffer, "__gnoclScrollMark__", &iter, 0 );


	gtk_text_view_scroll_to_mark ( view, mark, margin, useAlign, xAlign, yAlign );

	gtk_text_buffer_delete_mark ( buffer, mark );

	ret = TCL_OK;

clearExit:
	gnoclClearOptions ( options );
	return ret;
}

/**
\brief      Insert formatted string into specified text widget at a given location.
\author     PGB
\date       2001-06:
\bugs      Recent changes to posToIter causing this function to crash.
           But, applyTag, which is hack of this function does not cause a crash!
\history    2008-06-27  Began implementation of new keywords for text position. See TODO.
**/
static int textInsert ( GtkTextBuffer *buffer, Tcl_Interp *interp, int objc, Tcl_Obj *  const objv[], int cmdNo )
{
	GnoclOption insertOptions[] =
	{
		{ "-tags", GNOCL_LIST, NULL },
		{ NULL }
	};
	const int tagsIdx = 0;
	gint      startOffset;
	int       ret = TCL_ERROR;

	GtkTextIter   iter;

	if ( objc < cmdNo + 2 )
	{
		Tcl_WrongNumArgs ( interp, cmdNo, objv, "position text ?-option val ...?" );
		return TCL_ERROR;
	}

	if ( posToIter ( interp, objv[cmdNo], buffer, &iter ) != TCL_OK )
	{
		return TCL_ERROR;
	}

	if ( gnoclParseOptions ( interp, objc - cmdNo - 1, objv + cmdNo + 1, insertOptions ) != TCL_OK )
	{
		goto clearExit;
	}

	startOffset = gtk_text_iter_get_offset ( &iter );

	gtk_text_buffer_insert ( buffer, &iter, gnoclGetString ( objv[cmdNo+1] ), -1 );

	if ( insertOptions[tagsIdx].status == GNOCL_STATUS_CHANGED )
	{
		GtkTextIter start;
		int k, no;
		Tcl_Obj *obj = insertOptions[tagsIdx].val.obj;

		gtk_text_buffer_get_iter_at_offset ( buffer, &start, startOffset );

		if ( Tcl_ListObjLength ( interp, obj, &no ) != TCL_OK )
		{
			goto clearExit;
		}

		for ( k = 0; k < no; ++k )
		{
			Tcl_Obj *tp;

			if ( Tcl_ListObjIndex ( interp, obj, k, &tp ) != TCL_OK )
			{
				Tcl_SetResult ( interp, "Could not read tag list", TCL_STATIC );
				goto clearExit;
			}

			gtk_text_buffer_apply_tag_by_name ( buffer, Tcl_GetString ( tp ), &start, &iter );
		}
	}

	ret = TCL_OK;

clearExit:
	gnoclClearOptions ( insertOptions );

	return ret;
}

/**
\brief
\note        04/12/09 WJG modified to allow for variable assignment
**/
static int configure ( Tcl_Interp *interp, GtkScrolledWindow *scrolled, GtkTextView *text, GnoclOption options[] )
{

	GtkTextBuffer *buffer;
	buffer = gtk_text_view_get_buffer ( text );

	/*
	    gnoclAttachOptCmdAndVar (
	        &options[onChangedIdx], &para->onChanged,
	        &options[variableIdx], &para->variable,
	        "changed", G_OBJECT ( para->text ),
	        G_CALLBACK ( changedFunc ), interp, traceFunc, para );
	*/

	if ( options[textIdx].status == GNOCL_STATUS_CHANGED )
	{
		//printf ( "INSERT SOME TEXT-b\n" );

		char *str = options[textIdx].val.str;
		gtk_text_buffer_set_text ( buffer, str, -1 );
	}

	if ( options[scrollBarIdx].status == GNOCL_STATUS_CHANGED )
	{
		GtkPolicyType hor, vert;

		if ( gnoclGetScrollbarPolicy ( interp, options[scrollBarIdx].val.obj, &hor, &vert ) != TCL_OK )
		{
			return TCL_ERROR;
		}

		gtk_scrolled_window_set_policy ( scrolled, hor, vert );
	}

	if ( options[bufferIdx].status == GNOCL_STATUS_CHANGED )
	{
		printf ( "APPLY NEW BUFFER-%s\n", options[bufferIdx].val.str );

		GtkTextBuffer *buffer;

		buffer = gnoclGetWidgetFromName ( options[bufferIdx].val.str, interp );

		gtk_text_view_set_buffer ( text, buffer );

	}


	return TCL_OK;
}

/**
\brief
**/
static int cget ( Tcl_Interp *interp, GtkTextView *text, GnoclOption options[], int idx )
{
	return gnoclCgetNotImplemented ( interp, options + idx );
}

/**
     ->   0: Ok
        1: delete chosen
        2: configure chosen
        3: scrollToPosition chosen
      < 0: ERROR
*/

/**
\brief      Return list of tag settings and their values in the format
            { tagName1 {attribute-list} tagName2 {attribute-list}..  }
\note       1) Only returns those attribute which have been set away from their default values.
            Getting a handle on the various tag parameters has been a process of trial and error.
            An alternative way of getting the colour values but this time in hex is:
            GdkColor *bg_color2  = &tag_appearance->bg_color;
            char *clr = gdk_color_to_string ( bg_color2 );
            sprintf ( tmp2, "clr = %s ",clr);
\todo       How do I get the results of this back to the calling function?
            Data is assigned to the memory block referenced by gpointer data.
**/
static void gnoclGetTagSettings ( GtkTextTag *tag, gpointer data )
{

	char **str = data;

	/*  modify the memory allocation here to use malloc and free to release it */

	static char tmp[300];
	static char tmp2[50];

	/*  if null arguments are sent, then clear buffers, and return */

	if ( tag == NULL || data == NULL )
	{
		sprintf ( tmp, "" );
		sprintf ( tmp2, "" );
		return ;
	}

	// GtkTextAttributes *tag_attributes=tag->values;
	GtkTextAppearance *tag_appearance;

	/*  get the info on the GtkTextTag union form gtktexttag.h */
	strcat ( tmp, "" );

	sprintf ( tmp2, "%s { ", tag->name );

	strcat ( tmp, tmp2 );

	strcat ( tmp, "priority " );

	sprintf ( tmp2, "%d ", tag->priority );

	strcat ( tmp, tmp2 );

	/*  test some of these settings */

	/*  Individual chunks of this can be set/unset as a group */
	PangoFontDescription *font;

	GtkTextDirection direction;

	guint realized;

	/*    -----------------------------------------------
	 *  get a handle on the tag appearance structure
	 *  it might be fair to say that these are 'options'
	 *  and take default values.
	 *  When values are returned following a dump tags call,
	 *  only return these values if they have been changed from
	 *  system defaults.
	 *    -----------------------------------------------*/
	tag_appearance = tag->values;

	/*  background colour */
	if ( tag->bg_color_set )
	{
		GdkColor bg_color  = tag_appearance->bg_color;
		/*  build up the output list */
		strcat ( tmp, "bg_color" );
		sprintf ( tmp2, " { %d %d %d } ", bg_color.red, bg_color.green, bg_color.blue );
		strcat ( tmp, tmp2 );
	}

	/*  background stipple */

	if ( tag->bg_stipple_set )
	{
		GdkBitmap *bg_stipple  = tag_appearance->bg_stipple;
	}

	/*  foreground colour */

	if ( tag->fg_color_set )
	{
		GdkColor fg_color  = tag_appearance->fg_color;
		/*  build up the output list */
		strcat ( tmp, "fg_color" );
		sprintf ( tmp2, " { %d %d %d } ", fg_color.red, fg_color.green, fg_color.blue );
		strcat ( tmp, tmp2 );
	}

	/*  font scaling */

	if ( tag->scale_set )
	{
		strcat ( tmp, "font_scale" );
		sprintf ( tmp2, " %d ", tag->values->font_scale );
		strcat ( tmp, tmp2 );
	}

	/*  foreground stipple */

	if ( tag->fg_stipple_set )
	{
		GdkBitmap *fg_stipple = tag_appearance->fg_stipple;
	}

	/*  text justfification */

	if ( tag->justification_set )
	{
		// GTK_JUSTIFY_LEFT | GTK_JUSTIFY_RIGHT | GTK_JUSTIFY_CENTER | GTK_JUSTIFY_FILL
		strcat ( tmp, "justification" );
		sprintf ( tmp2, " %d ", tag->values->justification );
		strcat ( tmp, tmp2 );
	}

	/*  default indent */

	if ( tag->indent_set )
	{
		strcat ( tmp, "indent" );
		sprintf ( tmp2, " %d ", tag->values->indent );
		strcat ( tmp, tmp2 );
	}

	/*  text rise, i.e. for superscript */

	if ( tag->rise_set )
	{
		gint rise = tag_appearance->rise;
	}

	/*  strikethrough or, overstrike */

	if ( tag->strikethrough_set )
	{
		guint strikethrough = tag_appearance->strikethrough;
	}

	if ( tag->right_margin_set )
	{
		strcat ( tmp, "right_margin" );
		sprintf ( tmp2, " %d ", tag->values->right_margin );
		strcat ( tmp, tmp2 );
	}

	if ( tag->right_margin_set )
	{
		//gint right_margin = tag->values->right_margin;
		//printf ( "right_margin\t%d\n", tag->values->right_margin );
	}

	if ( tag->pixels_above_lines_set )
	{
		strcat ( tmp, "pixels_above_lines" );
		sprintf ( tmp2, " %d ", tag->values->pixels_above_lines );
		strcat ( tmp, tmp2 );
	}

	if ( tag->pixels_below_lines_set )
	{
		strcat ( tmp, "pixels_below_lines" );
		sprintf ( tmp2, " %d ", tag->values->pixels_below_lines );
		strcat ( tmp, tmp2 );
	}

	if ( tag->pixels_inside_wrap_set )
	{
		//gint pixels_inside_wrap = tag_attributes->pixels_inside_wrap;
		//printf ( "pixels_inside_wrap\t%d\n", tag->values->pixels_inside_wrap );
	}

	if ( tag->tabs_set )
	{
		PangoTabArray *tabs = tag->values->tabs;
	}

	if ( tag->underline_set )
	{
		guint underline = tag_appearance->underline;          /*  PangoUnderline */
		//printf ( "underline\t%d\n", tag_appearance->underline );
	}

	if ( tag->wrap_mode_set )
	{
		GtkWrapMode wrap_mode;  /*  How to handle wrap-around for this tag.
                                 *  Must be GTK_WRAPMODE_CHAR,
                                 *  GTK_WRAPMODE_NONE, GTK_WRAPMODE_WORD
                                 */
	}

	if ( tag->bg_full_height_set )
	{
		/*  Background is fit to full line height rather than
		 *  baseline +/- ascent/descent (font height)
		*/
		guint bg_full_height;
	}

	if ( tag->invisible_set )
	{
		guint invisible = tag->values->invisible;
	}

	if ( tag->editable_set )
	{
		/*  can edit this text */
		guint editable = tag->values->editable;
	}

	if ( tag->language_set )
	{
		PangoLanguage *language;
	}

	if ( tag->pg_bg_color_set )
	{
		/*< public >*/
		GdkColor bg_color = tag_appearance->bg_color;
	}

	/*  Whether to use background-related values; this is irrelevant for
	 *  the values struct when in a tag, but is used for the composite
	 *  values struct; it's true if any of the tags being composited
	 *  had background stuff set.
	 */
	guint draw_bg = tag_appearance->draw_bg;

	/*  These are only used when we are actually laying out and rendering
	 *  a paragraph; not when a GtkTextAppearance is part of a
	 *  GtkTextAttributes.
	 */
	guint inside_selection = tag_appearance->inside_selection;

	guint is_text = tag_appearance->is_text;


	/*  update string */
	// sprintf ( tmp2, " %d ", tag->values->pixels_below_lines );
	strcat ( tmp, "} " );
#ifdef DEBUG_TEXT
	g_print ( "tmp >>> = %s\n", tmp );
#endif
	*str = tmp;
}

/**
\brief
**/
static int signalEmit (
	Tcl_Interp *interp,
	Tcl_Obj *obj,
	int cmdNo,
	GtkTextBuffer *buffer )
{

	const char *signal = Tcl_GetString ( obj );

#ifdef DEBUG_TEXT
	printf ( ">> emitSignal %s\n", signal );
#endif

	/*  the problem now, is how to get the right object */
	gtk_signal_emit_by_name ( GTK_OBJECT ( GTK_WIDGET ( buffer ) ), signal, NULL );

	return TCL_OK;
}

/**
\brief
**/
int gnoclTextCommand (
	GtkTextBuffer *buffer,
	Tcl_Interp *interp,
	int objc,
	Tcl_Obj *  const objv[],
	int cmdNo,
	int isTextWidget )
{
#ifdef DEBUG_TEXT
	g_print ( "gnoclTextCommand %s %s\n", Tcl_GetString ( objv[cmdNo] ), Tcl_GetString ( objv[cmdNo+1] ) );
#endif

	const char *cmds[] =
	{
		"delete", "configure", "scrollToPosition",
		"scrollToMark", "set", "erase", "select",
		"get", "cut", "copy", "paste", "cget",
		"getLineCount", "getWordLength", "getLength",
		"getLineLength", "setCursor", "getCursor",
		"insert", "tag", "dump", "XYgetCursor",
		"signalEmit", "mark", "gotoWordStart",
		"gotoWordEnd", "search", "class", "spawn",
		"parent", "replace", "serialize",
		"deSerialize", "save", "load", "print",
		"getIndex", "getCoords", "getRect", "lorem",
		"clear", "popup", "undo", "redo",
		NULL
	};

	enum cmdIdx
	{
		DeleteIdx, ConfigureIdx, ScrollToPosIdx,
		ScrollToMarkIdx, SetIdx, EraseIdx, SelectIdx,
		GetIdx, CutIdx, CopyIdx, PasteIdx, CgetIdx,
		GetLineCountIdx, GetWordLengthIdx, GetLengthIdx,
		GetLineLengthIdx, SetCursorIdx, GetCursorIdx,
		InsertIdx, TagIdx, DumpIdx, XYgetCursorIdx,
		SignalEmitIdx, MarkIdx, GotoWordStartIdx,
		GotoWordEndIdx, SearchIdx, ClassIdx, SpawnIdx,
		ParentIdx, ReplaceIdx, SerializeIdx,
		DeSerializeIdx, SaveIdx, LoadIdx, PrintIdx,
		GetIndexIdx, GetCoordsIdx, GetRectIdx, LoremIdx,
		ClearIdx, PopupIdx, UndoIdx, RedoIdx
	};

	int   idx;
	GtkTextMark *mark;

	/*  WJG STUFF */
	char s[10];

	if ( objc < cmdNo + 1 )
	{
		Tcl_WrongNumArgs ( interp, cmdNo, objv, "command" );
		return -1;
	}

	if ( Tcl_GetIndexFromObj ( interp, objv[cmdNo], isTextWidget ? cmds : cmds + 3, "command", TCL_EXACT, &idx ) != TCL_OK )
	{
		return -1;
	}

	if ( !isTextWidget )
	{
		idx += 3;
	}

	switch ( idx )
	{
			/* item 1...n are GtkTextView operations, pass args onto textFunc*/
		case DeleteIdx:         return 1;
		case ConfigureIdx:      return 2;
		case ScrollToPosIdx:    return 3;
		case ScrollToMarkIdx:   return 4;
		case ParentIdx:         return 5;
		case GetIndexIdx:       return 6;
		case GetCoordsIdx:      return 7;
		case GetRectIdx:        return 8;
		case UndoIdx:  			return 9;
		case RedoIdx:  			return 10;

			/* these are GtkTextBuffer operation */

		case PrintIdx:
			{
				g_print ( "print\n" );

				/* reset parameters for this command and then call */
				gnoclPrintCmd ( buffer, interp, objc, objv );

			}

			break;
		case ClearIdx:
			{
				gtk_text_buffer_set_text ( buffer, "", 0 );
			}
			break;
		case LoremIdx:
			{

				gchar lorem[] = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Maecenas sed eleifend sem. Aenean at convallis ante. Etiam venenatis massa vitae nisl pretium sed pretium velit vehicula. Morbi vitae magna justo. Nullam ultricies rutrum felis rutrum tristique. Quisque orci mauris, cursus at dapibus quis, faucibus et nibh. Etiam posuere scelerisque libero eu rutrum. Nulla vel metus ut purus tempus adipiscing. Aenean lacus nunc, luctus sed tempor ac, semper vitae massa. Nunc et augue vitae ligula facilisis pulvinar a vestibulum magna. Quisque convallis rutrum vehicula. Morbi pulvinar nunc quis dui pharetra faucibus. Fusce sapien metus, varius eget fringilla quis, tristique vitae augue. Duis accumsan aliquet diam sed pretium. Nunc ipsum neque, auctor et convallis eget, molestie vitae libero. Integer eget velit at leo aliquam lobortis vel eu erat. Suspendisse sed orci neque. Vestibulum ante ipsum primis in faucibus orci luctus et ultrices posuere cubilia Curae; Ut lobortis arcu gravida sapien vestibulum venenatis. In quis diam tellus.";

				gtk_text_buffer_insert_at_cursor ( buffer, lorem, strlen ( lorem ) );

			}
			break;
		case PopupIdx:
			{
				// 0   1     2       3       4
				// $id popup item    <path>
				// $id popup subMenu <path1> <path2>
				g_print ( "PopupIdx %s\n", Tcl_GetString ( objv[cmdNo] ) );
				g_print ( "PopupIdx %s\n", Tcl_GetString ( objv[cmdNo +1] ) );
				g_print ( "PopupIdx %s\n", Tcl_GetString ( objv[cmdNo +2] ) );

				static char *popupOptions[] =
				{
					"item", "subMenu", "separator",
					NULL
				};

				static enum  popupOptionsIdx
				{
					ItemIdx, SubMenuIdx, SeparatorIdx
				};

				gint idx;

				getIdx ( popupOptions,  Tcl_GetString ( objv[cmdNo +1] ), &idx );

				switch ( idx )
				{
					case SeparatorIdx:
						{
							gnoclPopupMenuAddSeparator ( interp );
						}
						break;
					case ItemIdx:
						{
							gnoclPopupMenuAddItem ( interp, Tcl_GetString ( objv[cmdNo+2] ) );
						} break;
					case SubMenuIdx:
						{
							gnoclPopupMenuAddSubMenu ( interp, Tcl_GetString ( objv[cmdNo+2] ),  Tcl_GetString ( objv[cmdNo+3] ) );
						} break;
					default: {}
				}


			}

			break;
		case SaveIdx:
		case SerializeIdx:  /* WJG 29/04/09 */
			{

				FILE        *output;
				guint8      *data;
				gsize       length;
				GtkTextIter  start, end;
				GdkAtom se_format;

				se_format = gtk_text_buffer_register_serialize_tagset ( buffer, "default" );

				gtk_text_buffer_get_bounds ( buffer, &start, &end );
				data = gtk_text_buffer_serialize ( buffer, buffer, se_format, &start, &end, &length );

				output = fopen ( Tcl_GetString ( objv[cmdNo+1] ), "wb" );
				fwrite ( &length, sizeof ( gsize ), 1, output );
				fwrite ( data, sizeof ( guint8 ), length, output );
				fclose ( output );

				g_free ( data );
				return TCL_OK;
			}

			break;
		case LoadIdx:
		case DeSerializeIdx:  /* WJG 29/04/09 */
			{
				FILE        *input;
				guint8      *data;
				gsize        length;
				GtkTextIter  iter;
				GdkAtom        de_format;

				input = fopen ( Tcl_GetString ( objv[cmdNo+1] ), "rb" );

				/* Return with error message if the file is not found. */

				if ( input == NULL )
				{
					char str[128];
					sprintf ( str, "File %s not found.\n", Tcl_GetString ( objv[cmdNo+1] ) );
					Tcl_SetObjResult ( interp, Tcl_NewStringObj ( str, -1 ) );
					return TCL_OK;
				}


				fread ( &length, sizeof ( gsize ), 1, input );

				data = malloc ( sizeof ( guint8 ) * length );
				fread ( data, sizeof ( guint8 ), length, input );
				fclose ( input );

				de_format = gtk_text_buffer_register_deserialize_tagset ( buffer, "default" );
				gtk_text_buffer_get_iter_at_offset ( buffer, &iter, 0 );
				gtk_text_buffer_deserialize ( buffer, buffer, de_format, &iter, data, length, NULL );

				g_free ( data );
				return TCL_OK;
			}

			break;
		case ReplaceIdx:    /*  WJG 21/02/09 -Pretty much the same code as SearchIdx */
			{
				gint row1, col1, row2, col2;
				GtkTextIter start;
				GtkTextIter begin, end;
				GtkTextMark *replace;
				Tcl_Obj *resList;
				gint applyTags;
				gchar *  pch;
				gchar *  tagList;
				gint i;

				replace = NULL;

				if ( objc < cmdNo + 2 )
				{
					Tcl_WrongNumArgs ( interp, cmdNo + 1, objv, "<search-phrase> <swap-phrase>" );
					return -1;
				}

				resList = Tcl_NewListObj ( 0, NULL );

				/*  default with the start of the buffer */
				gtk_text_buffer_get_start_iter ( buffer, &start );

				i = 0;

				while ( gtk_text_iter_forward_search ( &start, Tcl_GetString ( objv[cmdNo+1] ), 0, &begin, &end, NULL ) != NULL )
				{

					if ( replace == NULL )
					{
						replace = gtk_text_buffer_create_mark ( buffer, "REPLACE_MARK", &begin, 1 );
					}

					else
					{
						gtk_text_buffer_move_mark  ( buffer, replace, &begin );
					}

					gtk_text_buffer_delete ( buffer, &begin, &end );

					gtk_text_buffer_get_iter_at_mark ( buffer, &end, replace );
					gtk_text_buffer_insert ( buffer, &end,  Tcl_GetString ( objv[cmdNo+2] ),  strlen (  Tcl_GetString ( objv[cmdNo+2] ) ) );

					start = end;
					i++;

				}

				gtk_text_buffer_delete_mark ( buffer, replace );

				Tcl_ListObjAppendElement ( interp, resList, Tcl_NewIntObj ( i ) );

				Tcl_SetObjResult ( interp, resList );

				return TCL_OK;
			}

			break;
		case SpawnIdx: /*  WJG began 06/12/08 */
			{
				/*  code taken from gnoclTextCmd */
				GtkTextView *spawn;
				GtkScrolledWindow *scrolled;

				/*  create a new text view with buffer GtkTextBuffer *buffer*/
				spawn = GTK_TEXT_VIEW ( gtk_text_view_new_with_buffer ( buffer ) );

				/*  add some extra signals to the default setting */
				gtk_widget_add_events ( spawn, GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK );

				scrolled =  GTK_SCROLLED_WINDOW ( gtk_scrolled_window_new ( NULL, NULL ) );
				gtk_scrolled_window_set_policy ( scrolled, GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC );
				gtk_container_add ( GTK_CONTAINER ( scrolled ), GTK_WIDGET ( spawn ) );
				gtk_widget_show_all ( GTK_WIDGET ( scrolled ) );

				if ( 0 )
				{
					/*  get properties of the parent widget */
					gint intval;
					gchar *strval;
					GObject *objval;

					g_object_get ( spawn,
								   "int-property", &intval,
								   "str-property", &strval,
								   "obj-property", &objval,
								   NULL );

					// Do something with intval, strval, objval

					g_free ( strval );
					g_object_unref ( objval );
				}

				/*  need to get options from the existing view */
				return gnoclRegisterWidget ( interp, GTK_WIDGET ( scrolled ), textFunc );

			}

			break;

		case SearchIdx: /*  WJG began 13/09/08 */
			{

				int row1, col1, row2, col2;
				GtkTextIter start;
				GtkTextIter begin, end;
				Tcl_Obj *resList;
				gint applyTags;
				char *  pch;
				char *  tagList;
				GtkTextTagTable *table;

				if ( objc < cmdNo + 1 )
				{
					Tcl_WrongNumArgs ( interp, cmdNo + 1, objv, "<search-phrase> -tags {taglist}" );
					return -1;
				}

				applyTags = 0;

				if ( objc == cmdNo + 4 )
				{
					if ( strcmp ( Tcl_GetString ( objv[cmdNo+2] ), "-tags" ) == 0 )
					{
						pch = strtok ( Tcl_GetString ( objv[cmdNo+3] ), " " );

						table = gtk_text_buffer_get_tag_table ( buffer );

						if ( gtk_text_tag_table_lookup ( table, pch ) == NULL )
						{



							Tcl_SetObjResult ( interp, Tcl_NewStringObj ( "GNOCL ERROR! Specified tag not found.", -1 ) );

							return TCL_ERROR;
						}

						applyTags = 1;
					}

					else
					{
						Tcl_SetObjResult ( interp, Tcl_NewStringObj ( "GNOCL ERROR! Invalid option given, it must be -tags <taglist>.", -1 ) );

						return TCL_ERROR;
					}
				}


				resList = Tcl_NewListObj ( 0, NULL );

				/*  default with the start of the buffer */
				gtk_text_buffer_get_start_iter ( buffer, &start );


				while ( gtk_text_iter_forward_search ( &start, Tcl_GetString ( objv[cmdNo+1] ), 0, &begin, &end, NULL ) != NULL )
				{

					/*  return the index of the found location */
					row1 = gtk_text_iter_get_line ( &begin );
					col1 = gtk_text_iter_get_line_offset ( &begin );
					row2 = gtk_text_iter_get_line ( &end );
					col2 = gtk_text_iter_get_line_offset ( &end );

					/*  check if there is a taglist to apply */
					if ( applyTags == 1 )
					{
						/* currently supporting only one tag */
						gtk_text_buffer_apply_tag_by_name ( buffer, pch, &begin, &end );

					}

					Tcl_ListObjAppendElement ( interp, resList, Tcl_NewIntObj ( row1 ) );
					Tcl_ListObjAppendElement ( interp, resList, Tcl_NewIntObj ( col1 ) );
					Tcl_ListObjAppendElement ( interp, resList, Tcl_NewIntObj ( row2 ) );
					Tcl_ListObjAppendElement ( interp, resList, Tcl_NewIntObj ( col2 ) );

					start = end;

				}


				Tcl_SetObjResult ( interp, resList );

				return TCL_OK;
			}

			break;
		case SignalEmitIdx:
			{
				/*  error check the command */
				if ( objc != cmdNo + 2 )
				{
					Tcl_WrongNumArgs ( interp, cmdNo + 1, objv, "<signal-name>" );

					return -1;
				}

				/*  DEBUG_TEXTging test line */
#ifdef DEBUG_TEXT
				printf ( "signalEmit %s\n", objv[cmdNo+1] );
#endif

				/*  attempt to emit the signal */
				if ( signalEmit ( interp, objv[cmdNo+1] , cmdNo, buffer ) != TCL_OK )
				{
					return -1;
				}
			}

			break;
		case XYgetCursorIdx:
			sprintf ( s, "Coordinates %s", objv[cmdNo+2] );
			Tcl_SetObjResult ( interp, Tcl_NewStringObj ( &s, -1 ) );
			break;
		case ClassIdx:
			{
				Tcl_SetObjResult ( interp, Tcl_NewStringObj ( "text", -1 ) );
			}
			break;
		case EraseIdx:
		case SelectIdx:
		case GetIdx:
			{
				GtkTextIter startIter, endIter;
				/*  text erase/select/getChars startIndex ?endIndex? */

				if ( objc < cmdNo + 2 || objc > cmdNo + 3 )
				{
					Tcl_WrongNumArgs ( interp, cmdNo + 1, objv, "startIndex ?endIndex?" );
					return TCL_ERROR;
				}

				if ( posToIter ( interp, objv[cmdNo+1], buffer, &startIter ) != TCL_OK )
				{
					return TCL_ERROR;
				}

				if ( objc >= 4 )
				{
					if ( posToIter ( interp, objv[cmdNo+2], buffer, &endIter ) != TCL_OK )
					{
						return TCL_ERROR;
					}
				}

				else
				{
					endIter = startIter;
					gtk_text_iter_backward_char ( &endIter );
				}

				switch ( idx )
				{
					case EraseIdx:
						gtk_text_buffer_delete ( buffer, &startIter, &endIter );
						break;
					case SelectIdx:
						gtk_text_buffer_place_cursor ( buffer, &startIter );
						gtk_text_buffer_move_mark_by_name ( buffer, "selection_bound", &endIter );
						break;
					case GetIdx:
						{
							/*  TODO: include_hidden_chars */
							char *txt = gtk_text_buffer_get_text ( buffer, &startIter, &endIter, 1 );
							Tcl_SetObjResult ( interp, Tcl_NewStringObj ( txt, -1 ) );
						}

						break;
				}
			}

			break;
		case CutIdx:
		case CopyIdx:
		case PasteIdx:
			{
				/*  TODO: option which clipboard */
				GtkClipboard *clipboard = gtk_clipboard_get ( GDK_NONE );

				if ( objc != cmdNo + 1 )
				{
					Tcl_WrongNumArgs ( interp, cmdNo + 1, objv, NULL );
					return TCL_ERROR;
				}

				switch ( idx )
				{
					case CutIdx:
						gtk_text_buffer_cut_clipboard ( buffer, clipboard, 1 );
						break;
					case CopyIdx:
						gtk_text_buffer_copy_clipboard ( buffer, clipboard );
						break;
					case PasteIdx:
						gtk_text_buffer_paste_clipboard ( buffer, clipboard, NULL, 1 );
						break;
				}
			}

			break;
		case CgetIdx:
			{
				/*  WJG started work on this one 19/03/08 */
				printf ( "text cget not yet implemented\n" );
				/*         int     idx;
				 *
				 *                    switch( gnoclCget( interp, objc, objv,
				 *                          G_OBJECT( para->text ), textOptions, &idx ) )
				 *                    {
				 *                       case GNOCL_CGET_ERROR:
				 *                                return TCL_ERROR;
				 *                       case GNOCL_CGET_HANDLED:
				 *                                return TCL_OK;
				 *                       case GNOCL_CGET_NOTHANDLED:
				 *                                return cget( interp, para, textOptions, idx );
				 *                    }
				 */
			}

			break;
		case GetLineCountIdx: /*  WJG Added 24/04/08 */
			{
				/*  editable getLength */
				if ( objc != cmdNo + 1 )
				{
					Tcl_WrongNumArgs ( interp, cmdNo + 1, objv, NULL );
					return -1;
				}

				Tcl_SetObjResult ( interp, Tcl_NewIntObj ( gtk_text_buffer_get_line_count ( buffer ) ) );
			}

			break;
		case GetLengthIdx: /*  TODO getByteCount */
			{
				/*  editable getLength */
				if ( objc != cmdNo + 1 )
				{
					Tcl_WrongNumArgs ( interp, cmdNo + 1, objv, NULL );
					return -1;
				}

				Tcl_SetObjResult ( interp, Tcl_NewIntObj ( gtk_text_buffer_get_char_count ( buffer ) ) );
			}

			break;
		case GetLineLengthIdx: /*  Added WJG 17/05/08 */
			{
				GtkTextIter iter;

				if ( objc != cmdNo + 2 )
				{
					Tcl_WrongNumArgs ( interp, cmdNo + 1, objv, "index" );
					return -1;
				}

				if ( posToIter ( interp, objv[cmdNo+1], buffer, &iter ) != TCL_OK )
				{
					return -1;
				}

				Tcl_SetObjResult ( interp, Tcl_NewIntObj ( gtk_text_iter_get_chars_in_line ( &iter ) ) );
			}

			break;
		case GetWordLengthIdx: /*  Added WJG 12/09/08 */
			{
				GtkTextIter iter, iter_a;
				int row_a, col_a, row_b, col_b, wordLength;

				/*  get the iter at the specified position */
				posToIter ( interp, objv[cmdNo+1], buffer, &iter );

				/*  move the iter to the start of the word */
				gtk_text_iter_backward_word_start ( &iter );

				/*  get row-offset values for the iter */
				row_a = gtk_text_iter_get_line ( &iter );
				col_a = gtk_text_iter_get_line_offset ( &iter );

				/*  move the iter to the end of the word */
				gtk_text_iter_forward_word_end ( &iter );

				/*  get the position... */
				row_b = gtk_text_iter_get_line ( &iter );
				col_b = gtk_text_iter_get_line_offset ( &iter );

				/*  subtract the larger from the smaller */
				wordLength = col_b - col_a;

				/*  return the value */
				Tcl_SetObjResult ( interp, Tcl_NewIntObj ( wordLength ) );
			}

			break;
		case GotoWordStartIdx: /*  Added WJG 12/09/08 */
			{
				GtkTextIter iter;

				/*  text setCursor index */

				if ( objc != cmdNo + 2 )
				{
					Tcl_WrongNumArgs ( interp, cmdNo + 1, objv, "index" );
					return -1;
				}

				gtk_text_buffer_get_iter_at_mark ( buffer, &iter, gtk_text_buffer_get_insert ( buffer ) );

				gtk_text_iter_backward_word_start ( &iter );

				gtk_text_buffer_place_cursor ( buffer, &iter );
			}

			break;
		case GotoWordEndIdx: /*  Added WJG 12/09/08 */
			{
				GtkTextIter iter;

				/*  text setCursor index */

				if ( objc != cmdNo + 2 )
				{
					Tcl_WrongNumArgs ( interp, cmdNo + 1, objv, "index" );
					return -1;
				}

				gtk_text_buffer_get_iter_at_mark ( buffer, &iter, gtk_text_buffer_get_insert ( buffer ) );

				gtk_text_iter_forward_word_end ( &iter );

				gtk_text_buffer_place_cursor ( buffer, &iter );
			}

			break;
		case SetIdx:
			{
				gtk_text_buffer_set_text ( buffer, gnoclGetString ( objv[cmdNo+1] ), -1 );
			}

			break;
		case SetCursorIdx:
			{
				GtkTextIter iter;

				/*  text setCursor index */

				if ( objc != cmdNo + 2 )
				{
					Tcl_WrongNumArgs ( interp, cmdNo + 1, objv, "index" );
					return -1;
				}

				if ( posToIter ( interp, objv[cmdNo+1], buffer, &iter ) != TCL_OK )
				{
					return -1;
				}

				gtk_text_buffer_place_cursor ( buffer, &iter );
			}

			break;
		case GetCursorIdx:
			{
				GtkTextIter   iter;
				int           row, col;
				Tcl_Obj       *resList;

				/*  text getCursor */

				if ( objc != cmdNo + 1 )
				{
					Tcl_WrongNumArgs ( interp, cmdNo + 1, objv, NULL );
					return -1;
				}

				gtk_text_buffer_get_iter_at_mark ( buffer, &iter, gtk_text_buffer_get_insert ( buffer ) );

				row = gtk_text_iter_get_line ( &iter );
				col = gtk_text_iter_get_line_offset ( &iter );

				resList = Tcl_NewListObj ( 0, NULL );
				Tcl_ListObjAppendElement ( interp, resList, Tcl_NewIntObj ( row ) );
				Tcl_ListObjAppendElement ( interp, resList, Tcl_NewIntObj ( col ) );
				Tcl_SetObjResult ( interp, resList );
			}

			break;
		case InsertIdx:

			if ( textInsert ( buffer, interp, objc, objv, cmdNo + 1 ) != TCL_OK )
			{
				return -1;
			}

			/*  this action needs to emit an insert text signal */

			break;

		case MarkIdx:

			if ( markCmd ( buffer, interp, objc, objv, cmdNo + 1 ) != TCL_OK )
			{
				return -1;
			}

			break;

		case TagIdx:

			if ( tagCmd ( buffer, interp, objc, objv, cmdNo + 1 ) != TCL_OK )
			{
				return -1;
			}

			break;

			/*  the widget dump has the format;
			 *  widgetId dump option startIndex endIndx
			 */
		case DumpIdx:
			{
				/*  some DEBUG_TEXTging feedback */

#ifdef DEBUG_TEXT
				g_print ( "dump: %d %d: %s %s %s %s \n",
						  objc, cmdNo + 4,
						  Tcl_GetString ( objv[cmdNo] ),
						  Tcl_GetString ( objv[cmdNo+1] ),
						  Tcl_GetString ( objv[cmdNo+2] ),
						  Tcl_GetString ( objv[cmdNo+3] ) ) ;
#endif

				GtkTextIter startIter, endIter;

				GtkTextTagTable *tagtable;
				char *txt;
				int idx;
				static const char *cmds[] = { "all", "tags", "text", "windows", "images", "marks", NULL };
				enum opts {AllIdx, TagsIdx, TextIdx, WindowsIdx, ImagesIdx, MarksIdx};


				/*  do some error checking */

				if ( objc < cmdNo + 4 )
				{
					Tcl_WrongNumArgs ( interp, cmdNo + 1, objv, "?all|text|tags|window|images|marks? startIndex ?endIndex?" );
					return TCL_ERROR;
				}

				/*  get iters for the required range */

				if ( posToIter ( interp, objv[cmdNo+2], buffer, &startIter ) != TCL_OK )
				{
					return TCL_ERROR;
				}

				if ( posToIter ( interp, objv[cmdNo+3], buffer, &endIter ) != TCL_OK )
				{
					return TCL_ERROR;
				}

				/*  get the sub-command, one of all, tags, text, images, marks */

				if ( Tcl_GetIndexFromObj ( interp, objv[cmdNo+1], cmds, "subcommand", TCL_EXACT, &idx ) != TCL_OK )
				{
					return TCL_ERROR;
				}

				/*
				 *  Respond to the second keyword..
				 */

				switch ( idx )
				{
					case AllIdx:
						{
							txt = "dump all... Feature not yet implmented\n";
							Tcl_SetObjResult ( interp, Tcl_NewStringObj ( txt, -1 ) );
							return TCL_OK;
						}

						break;
					case TagsIdx:
						{
#ifdef DEBUG_TEXT
							g_print ( "dump tags\n" );
#endif
							GtkTextTagTable *tagtable = gtk_text_buffer_get_tag_table ( buffer );

							/*  get the settings of each tag in the buffer*/
							/*  note, pass the address of the pointer to the data assigned by the called function */
							gtk_text_tag_table_foreach ( tagtable, gnoclGetTagSettings , &txt );

							Tcl_SetObjResult ( interp, Tcl_NewStringObj ( txt, -1 ) );
							/*  reset the outsput string by using null pointers */
							gnoclGetTagSettings ( NULL, NULL );
							return TCL_OK;
						}

						break;
					case TextIdx:
						{
							/*  TODO: include_hidden_chars */
							txt = gtk_text_buffer_get_text ( buffer, &startIter, &endIter, 1 );
							Tcl_SetObjResult ( interp, Tcl_NewStringObj ( txt, -1 ) );
							return TCL_OK;
						}

						break;
					case WindowsIdx:
						{
							txt = "dump windows... Feature not yet implmented\n";
							Tcl_SetObjResult ( interp, Tcl_NewStringObj ( txt, -1 ) );
							return TCL_OK;
						}

						break;
					case ImagesIdx:
						{
							txt = "dump images... Feature not yet implmented\n";
							Tcl_SetObjResult ( interp, Tcl_NewStringObj ( txt, -1 ) );
							return TCL_OK;
						}

						break;
					case MarksIdx:
						{
							txt = "dump marks... Feature not yet implmented\n";
							Tcl_SetObjResult ( interp, Tcl_NewStringObj ( txt, -1 ) );
							return TCL_OK;
						}

						break;
					default:
						{
							assert ( 0 );
						}
				}

				return TCL_OK;




				// gint gtk_text_tag_table_get_size (GtkTextTagTable *table);
				int tagTableSize = gtk_text_tag_table_get_size ( tagtable );

				printf ( "~tagTableSize~    %d\n", tagTableSize );

				// void gtk_text_tag_table_foreach (GtkTextTagTable *table, GtkTextTagTableForeach func, gpointer data);
				//gtk_text_tag_table_foreach ( tagtable, testPrint , NULL );

				//GtkTextTag*  gtk_text_tag_table_lookup (GtkTextTagTable *table, const gchar *name);
				GtkTextTag *tag = gtk_text_tag_table_lookup ( tagtable, "_tag_rollOver" );
#ifdef DEBUG_TEXT
				printf ( "~~tag~   %s\n", tag );
#endif
			}

			break;

		default:
			assert ( 0 );
			return -1;
	}

	return 0;
}

/**
\brief      Handler for gnocl created instances that take into account the scrolled window
\note        Unlike gnocl, Builder/Glade  not provide text objects within scrolled windows.
             Two handle functions are necessary, one for gnocl built widgets, and one for builder/glade widgets.
**/
static int textFunc ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj *  const objv[] )
{
	GtkScrolledWindow   *scrolled = GTK_SCROLLED_WINDOW ( data );
	GtkTextView     *text = GTK_TEXT_VIEW ( gtk_bin_get_child ( GTK_BIN ( scrolled ) ) );
	GtkTextBuffer  *buffer = gtk_text_view_get_buffer ( text );

#ifdef DEBUG_TEXT_TEXT
	g_printf ( "textFunc\n" );
	gint _i;

	for ( _i = 0; _i < objc; _i++ )
	{
		g_print ( "\targ %d = %s\n", _i,  Tcl_GetString ( objv[_i] ) );
	}

#endif


	if ( objc < 2 )
	{
		Tcl_WrongNumArgs ( interp, 1, objv, "command" );
		return TCL_ERROR;
	}

	switch ( gnoclTextCommand ( buffer, interp, objc, objv, 1, 1 ) )
	{
			/*  these are command which work upon the GtkTextView rather than the GtkTextBuffer */
		case 0:
			break;  /*  return TCL_OK */
		case 1:     /*  delete */
			return gnoclDelete ( interp, GTK_WIDGET ( scrolled ), objc, objv );
		case 2:     /*  configure */
			{
				int ret = TCL_ERROR;

				if ( gnoclParseAndSetOptions ( interp, objc - 1, objv + 1, textOptions, G_OBJECT ( text ) ) == TCL_OK )
				{
					//ret = configure ( interp, scrolled, text, para, textOptions );
					ret = configure ( interp, scrolled, text, textOptions );
				}

				gnoclClearOptions ( textOptions );

				return ret;
			}

			break;
		case 3: /*  scrollToPosition */
			return scrollToPos ( text, buffer, interp, objc, objv );
		case 4: /*  scrollToMark */
			return scrollToMark ( text, buffer, interp, objc, objv );
		case 5: /*  get parent, WJG added 06/12/08 */
			{
				GtkWidget *  parent;
				Tcl_Obj *obj = NULL;
				parent = gtk_widget_get_parent ( GTK_WIDGET ( data ) );
				obj = Tcl_NewStringObj ( gnoclGetNameFromWidget ( parent ), -1 );
				Tcl_SetObjResult ( interp, obj );

				/*  this function not working too well! */
				/*  return gnoclGetParent ( interp, data ); */
				return TCL_OK;
			}

			break;
		case 6: /* get line/row from coordinates */
			{

				GtkTextIter *iter;
				gint y, line_no;
				gint x, row_no;

				gint wx, wy; /* window coordinates */
				gint bx, by; /* buffer coordinates */
				gint line, row;

				sscanf ( Tcl_GetString ( objv[2] ), "%d %d", &wx, &wy );

				//Tcl_GetIntFromObj ( NULL, objv[2], &wx ) ;
				//Tcl_GetIntFromObj ( NULL, objv[3], &wy ) ;

				//gdk_window_get_pointer (TxT->window, &wx, &wy, NULL);
				gtk_text_view_window_to_buffer_coords ( text, GTK_TEXT_WINDOW_WIDGET, wx, wy, &bx, &by );
				gtk_text_view_get_iter_at_location ( text, &iter, bx, by );

				line = gtk_text_iter_get_line ( &iter );
				row = gtk_text_iter_get_line_offset ( &iter );

				gchar str[16];
				sprintf ( str, "%d %d", line, row );
				Tcl_SetObjResult ( interp, Tcl_NewStringObj ( str, -1 ) );

			}
			break;

			/* past line col position, return window location */
		case 7: /* window x/y coords from iter */
			{

				gint wx;
				gint wy;

				GtkTextIter iter;
				GdkRectangle rect;

				if ( posToIter ( interp, objv[2], buffer, &iter ) != TCL_OK )
				{
					return TCL_ERROR;
				}

				gtk_text_view_get_iter_location  ( text, &iter, &rect );

				// this line returns the buffer coordinates as window cooordinates!
				gtk_text_view_buffer_to_window_coords ( text, GTK_TEXT_WINDOW_WIDGET, rect.x, rect.y, &wx, &wy );

				gchar str[24];
				sprintf ( str, "%d %d %d %d\n", wx, wy, rect.width, rect.height );
				Tcl_SetObjResult ( interp, Tcl_NewStringObj ( str, -1 ) );

			}
			break;
		case 8: /* get visible rect */
			{
				GdkRectangle rect;

				gtk_text_view_get_visible_rect ( text, &rect );

				gchar str[24];
				sprintf ( str, "%d %d %d %d\n", rect.x, rect.y, rect.width, rect.height );
				Tcl_SetObjResult ( interp, Tcl_NewStringObj ( str, -1 ) );

			}
			break;
		case 9: /* undo action */
			{
				gtk_undo_view_undo ( text );
			}
			break;
		case 10: /* redo action */
			{
				gtk_undo_view_redo  ( text );
			}
		default:
			{
				return TCL_ERROR;
			}
	}

	return TCL_OK;
}

/**
\brief      Handler for glade created instances that have no scrolled window.
**/
int textViewFunc ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj *  const objv[] )
{

	GtkTextView     *text = GTK_TEXT_VIEW ( data );
	GtkTextBuffer  *buffer = gtk_text_view_get_buffer ( text );


	if ( objc < 2 )
	{
		Tcl_WrongNumArgs ( interp, 1, objv, "command" );
		return TCL_ERROR;
	}

	switch ( gnoclTextCommand ( buffer, interp, objc, objv, 1, 1 ) )
	{
			/*  these are command which work upon the GtkTextView rather than the GtkTextBuffer */
		case 0:
			break;  /*  return TCL_OK */
		case 1:     /*  delete */
			//return gnoclDelete ( interp, GTK_WIDGET ( scrolled ), objc, objv );
		case 2:     /*  configure */
			{
				int ret = TCL_ERROR;

				if ( gnoclParseAndSetOptions ( interp, objc - 1, objv + 1, textOptions, G_OBJECT ( text ) ) == TCL_OK )
				{
					//ret = configure ( interp, scrolled, text, para, textOptions );
					//ret = configure ( interp, scrolled, text, textOptions );
				}

				gnoclClearOptions ( textOptions );

				return ret;
			}

			break;
		case 3: /*  scrollToPosition */
			return scrollToPos ( text, buffer, interp, objc, objv );
		case 4: /*  scrollToMark */
			return scrollToMark ( text, buffer, interp, objc, objv );
		case 5: /*  get parent, WJG added 06/12/08 */
			{
				GtkWidget *  parent;
				Tcl_Obj *obj = NULL;
				parent = gtk_widget_get_parent ( GTK_WIDGET ( data ) );
				obj = Tcl_NewStringObj ( gnoclGetNameFromWidget ( parent ), -1 );
				Tcl_SetObjResult ( interp, obj );

				/*  this function not working too well! */
				/*  return gnoclGetParent ( interp, data ); */
				return TCL_OK;
			}

			break;
		default:
			return TCL_ERROR;
	}

	return TCL_OK;
}

/**
\brief
**/
int gnoclTextCmd ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj *  const objv[] )
{
	int               ret, k;
	GtkTextView       *textView;
	GtkTextView       *textBuffer;
	GtkScrolledWindow *scrolled;


	if ( gnoclParseOptions ( interp, objc, objv, textOptions ) != TCL_OK )
	{
		gnoclClearOptions ( textOptions );
		return TCL_ERROR;
	}

	textView = GTK_TEXT_VIEW ( gtk_text_view_new( ) );

	//GtkTextBuffer *     gtk_text_buffer_new  (NULL);

	//textBuffer = gtk_text_view_get_buffer (textView);

	/* implement new undo/redo buffer */
	textView = gtk_undo_view_new ( gtk_text_buffer_new  ( NULL ) );

	/*  add some extra signals to the default setting */
	gtk_widget_add_events ( textView, GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK );

	scrolled =  GTK_SCROLLED_WINDOW ( gtk_scrolled_window_new ( NULL, NULL ) );

	gtk_scrolled_window_set_policy ( scrolled, GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC );

	gtk_container_add ( GTK_CONTAINER ( scrolled ), GTK_WIDGET ( textView ) );

	gtk_widget_show_all ( GTK_WIDGET ( scrolled ) );

	ret = gnoclSetOptions ( interp, textOptions, G_OBJECT ( textView ), -1 );

	if ( ret == TCL_OK )
	{
		//ret = configure ( interp, scrolled, textView, para, textOptions );
		ret = configure ( interp, scrolled, textView, textOptions );
	}

	gnoclClearOptions ( textOptions );

	if ( ret != TCL_OK )
	{
		gtk_widget_destroy ( GTK_WIDGET ( scrolled ) );
		return TCL_ERROR;
	}

	return gnoclRegisterWidget ( interp, GTK_WIDGET ( scrolled ), textFunc );
}
