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
	to do
		document
			-acceptTab
			-tabs
*/
/*
   History:
   2013-04: resolved problem with tag sub-command "ranges"
   2013-02: serialize command now returns buffer data as string
   2013-01: added subcommands isToplevelFocus and hasGlobalFocus
   2012-12	corrected -accepttTab to -acceptsTab
   2012-11  added -data option to text tag
			fixed problems with:
				getIndex subcommand
				tag get  subcommand
			added %r(ow) %c(col) substitution string parameters for
			-onMotion signal handler
   2012-10	added -onDestroy
   2012-09	cget -baseFont implemented
			resetUndo
   2012-08  migrated to the use of TextParams to hold widget details
			implemented -variable option
   2012-07	added grabFocus, same effect as setting the -hasFocus option
   2012-06  added selectionBounds to get widget command.
   2011-07  added tag sub-commands
				names, raise, lower
    		insert command now detects pango markup text
			added -markupTags
   2011-06  added tag -underline option 'error'
   2011-06  added -hasToolTip, -onQueryTooltip,
   			added tag options
   				-data
   2011-04  added tag options
				-marginAccumulate -backgroundFullHeight	-backgroundStipple
				-direction -editable -foregroundStipple -indent -language
				-leftMargin -name -pixelsInsideWrap -rightMargin -rise
				-scale -tabs -variant -weight
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

   TODO:
    * set a clear pango mode, ie no other tags a can be created, non-deleted
    * marks
 */

/**
\page page_text gnocl::text
\htmlinclude text.html
**/

#include "gnocl.h"
#include "gnoclparams.h"
#include "./textUndo/undo_manager.h"

static int textFunc ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj *  const objv[] );
static int cget ( Tcl_Interp *interp, GtkTextView *text, GnoclOption options[], int idx );
static void gnoclGetTagRanges ( Tcl_Interp * interp, GtkTextBuffer * buffer, gchar * tagName );
static void getTagName ( GtkTextTag *tag, gpointer data );
static void gnoclGetTagProperties ( GtkTextTag * tag, Tcl_Obj *resList );

static int setTextVariable ( TextParams *para, const char *val );
static void changedFunc ( GtkWidget *widget, gpointer data );
static void destroyFunc ( GtkWidget *widget, gpointer data );
static int setVal ( GtkTextBuffer *buffer, const char *txt );
static char *traceFunc ( ClientData data, Tcl_Interp *interp, const char *name1, const char *name2,	int flags );
static int doCommand ( TextParams *para, const char *val, int background );

static gint usemarkup = 0;


/***********************************************************************
 * trace funcs
***********************************************************************/

/**
\brief
**/
static void destroyFunc ( GtkWidget *widget, gpointer data )
{
#ifdef DEBUG_TEXT
	printf ( "%s\n", __FUNCTION__ );
#endif


	TextParams *para = ( TextParams * ) data;

	gnoclForgetWidgetFromName ( para->name );
	Tcl_DeleteCommand ( para->interp, para->name );

	gnoclAttachOptCmdAndVar (
		NULL, &para->onChanged,
		NULL, &para->textVariable,
		"changed", G_OBJECT ( para->textView ),
		G_CALLBACK ( changedFunc ), para->interp, traceFunc, para );

	g_free ( para->textVariable );
	g_free ( para->name );
	g_free ( para );
}

/**
\brief
**/
static void changedFunc ( GtkWidget *widget, gpointer data )
{
#ifdef DEBUG_TEXT
	printf ( "%s\n", __FUNCTION__ );
#endif

	TextParams *para = ( TextParams * ) data;

	GtkScrolledWindow *scrolled = para->scrolled;
	GtkTextView *text = GTK_TEXT_VIEW ( gtk_bin_get_child ( GTK_BIN ( scrolled ) ) );
	GtkTextBuffer *buffer = gtk_text_view_get_buffer ( text );

	GtkTextIter start, end;
	gtk_text_buffer_get_bounds ( buffer, &start, &end );
	const char *val = gtk_text_buffer_get_text ( buffer, &start, &end, FALSE );

	g_print ( "....%s\n", val );


	//const char *val = gtk_label_get_text ( para->label );
	setTextVariable ( para, val );
	//doCommand ( para, val, 1 );
}

/**
\brief
**/
static int setTextVariable ( TextParams *para,	const char *val )
{
#ifdef DEBUG_TEXT
	printf ( "%s\n", __FUNCTION__ );;
#endif


	if ( para->textVariable && para->inSetVar == 0 )
	{
		const char *ret;
		para->inSetVar = 1;
		ret = Tcl_SetVar ( para->interp, para->textVariable, val, TCL_GLOBAL_ONLY );
		para->inSetVar = 0;
		return ret == NULL ? TCL_ERROR : TCL_OK;
	}

	return TCL_OK;
}

/**
\brief
**/
static int setVal ( GtkTextBuffer *buffer, const char *txt )
{
	int blocked;
	blocked = g_signal_handlers_block_matched ( G_OBJECT ( buffer ), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, ( gpointer * ) changedFunc, NULL );
	//gtk_label_set_text ( label, txt );

	gtk_text_buffer_set_text ( buffer, txt, -1 );

	//gtk_label_set_markup ( label, txt );

	//OptLabelFull ( label, txt );

	if ( blocked )
	{
		g_signal_handlers_unblock_matched ( G_OBJECT ( buffer ), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, ( gpointer * ) changedFunc, NULL );
	}

	return TCL_OK;
}

/**
\brief
**/
static char *traceFunc ( ClientData data, Tcl_Interp *interp, const char *name1, const char *name2,	int flags )
{
	TextParams *para = ( TextParams * ) data;

	GtkScrolledWindow *scrolled = para->scrolled;
	GtkTextView *text = GTK_TEXT_VIEW ( gtk_bin_get_child ( GTK_BIN ( scrolled ) ) );
	GtkTextBuffer *buffer = gtk_text_view_get_buffer ( text );


	if ( para->inSetVar == 0 && name1 )
	{
		const char *txt = name1 ? Tcl_GetVar2 ( interp, name1, name2, 0 ) : NULL;

		if ( txt )
		{
			setVal ( buffer, txt );
			doCommand ( para, txt, 1 );
		}
	}

	return NULL;
}

/**
\brief
**/
static int doCommand ( TextParams *para, const char *val, int background )
{
	if ( para->onChanged )
	{
		GnoclPercSubst ps[] =
		{
			{ 'w', GNOCL_STRING },  /* widget */
			{ 'v', GNOCL_STRING },  /* value */
			{ 0 }
		};

		ps[0].val.str = para->name;
		ps[1].val.str = val;

		return gnoclPercentSubstAndEval ( para->interp, ps, para->onChanged, background );
	}

	return TCL_OK;
}


/***********************************************************************/

/**
\brief	Return text with Pango markup
**/
Tcl_Obj *getMarkUpString ( Tcl_Interp *interp, GtkTextBuffer *buffer, GtkTextIter *start, GtkTextIter *end )
{
#ifdef DEBUG_TEXT
	g_print ( "%s usemarkup = %d\n", __FUNCTION__, usemarkup );
#endif

	g_print ( "%s\n", __FUNCTION__ );


	Tcl_Obj *res;
	//gchar *txt = NULL;

	GtkTextIter *iter;
	gunichar ch;
	GSList *p, *onList, *offList;

	gchar *tagName = NULL;
	gchar *onTag = NULL;
	gchar *offTag = NULL;

	//txt = gtk_text_buffer_get_text ( buffer, start, end, 1 );

	//res = Tcl_NewStringObj ( txt, -1 );
	//g_free ( txt );

	res = Tcl_NewStringObj ( "", 0 );

	iter = gtk_text_iter_copy ( start );

	/* parse each position in the selection */
	while ( gtk_text_iter_equal ( iter, end ) == 0 )
	{

		/* process tagOff before any subsequent tagOn */
		offList = gtk_text_iter_get_toggled_tags ( iter, 0 );

		for ( p = offList; p != NULL; p = p->next )
		{
			tagName = ( GTK_TEXT_TAG ( p->data )->name );

			/* handle overlapping tags, pango only allows nesting -WORKING HERE*/
			if ( onTag != tagName  )
			{
				Tcl_AppendStringsToObj ( res, "</", onTag, ">", "</", tagName, ">", "<", onTag, ">", ( char * ) NULL );
			}

			else
			{
				Tcl_AppendStringsToObj ( res, "</", tagName, ">", ( char * ) NULL );
				onTag = NULL;
				offTag = NULL;
			}

			//offTag = tagName;

		}

		/* process tagOn */
		onList = gtk_text_iter_get_toggled_tags ( iter, 1 );

		for ( p = onList; p != NULL; p = p->next )
		{
			tagName = ( GTK_TEXT_TAG ( p->data )->name );

			/* handle overlapping tags, pango only allows nesting -OK*/
			if ( onTag != NULL && strcmp ( onTag, tagName ) == 0 )
			{
				Tcl_AppendStringsToObj ( res, "</", onTag, ">", "<", onTag, ">", "<", tagName, ">", ( char * ) NULL );
			}

			onTag = tagName;
		}

		/* get character */
		ch = gtk_text_iter_get_char ( iter );
		Tcl_AppendStringsToObj ( res, &ch, ( char * ) NULL );

		/* turnOff span? */
		gtk_text_iter_forward_char ( iter );

		/* ensure all open markup tags are properly closed */
		if  ( gtk_text_iter_equal ( iter, end ) )
		{
			offList = gtk_text_iter_get_toggled_tags ( iter, 0 );

			for ( p = offList; p != NULL; p = p->next )
			{
				tagName = ( GTK_TEXT_TAG ( p->data )->name );

				if ( strstr ( tagName, "=" ) )
				{
					Tcl_AppendStringsToObj ( res, "</span>", ( char * ) NULL );
				}

				else
				{
					Tcl_AppendStringsToObj ( res, "</", tagName, ">", ( char * ) NULL );
				}


			}

		}

	}

	gtk_text_iter_free ( iter );


#ifdef DEBUG_TEXT
	g_print ( "done!\n" );
#endif

	return res;
}


/**
\brief  Serialize the content of the textbuffer in an ascii format
\note   Currently supports tags only.
**/
static Tcl_Obj *dumpAll ( Tcl_Interp *interp, GtkTextBuffer *buffer, GtkTextIter *start, GtkTextIter *end )
{

#ifdef DEBUG_TEXT
	g_print ( "%s\n", __FUNCTION__ );
#endif

	Tcl_Obj *res;

	GtkTextIter *iter;
	gunichar ch;
	GSList *p, *onList, *offList;

	gchar *tagName = NULL;
	gchar *onTag = NULL;
	gchar *offTag = NULL;


	res = Tcl_NewStringObj ( "", 0 );

	iter = gtk_text_iter_copy ( start );

	while ( gtk_text_iter_equal ( iter, end ) == 0 )
	{
		/* process tagOff before any subsequent tagOn */
		offList = gtk_text_iter_get_toggled_tags ( iter, 0 );

		for ( p = offList; p != NULL; p = p->next )
		{
			tagName = ( GTK_TEXT_TAG ( p->data )->name );
			Tcl_AppendStringsToObj ( res, "</", tagName, ">", ( char * ) NULL );
		}

		/* process tagOn */
		onList = gtk_text_iter_get_toggled_tags ( iter, 1 );

		for ( p = onList; p != NULL; p = p->next )
		{
			tagName = ( GTK_TEXT_TAG ( p->data )->name );
			Tcl_AppendStringsToObj ( res, "<", tagName, ">", ( char * ) NULL );
		}

		/* get character */
		ch = gtk_text_iter_get_char ( iter );
		Tcl_AppendStringsToObj ( res, &ch, ( char * ) NULL );

		/* turnOff span? */
		gtk_text_iter_forward_char ( iter );
	}

	gtk_text_iter_free ( iter );


#ifdef DEBUG_TEXT
	g_print ( "done!\n" );
#endif

	return res;
}




/**
\brief	Return a Tcl list of text attributes.
GtkJustification justification;
GtkTextDirection direction;
PangoFontDescription *font;
gdouble font_scale;
gint left_margin;
gint indent;
gint right_margin;
gint pixels_above_lines;
gint pixels_below_lines;
gint pixels_inside_wrap;
PangoTabArray *tabs;
GtkWrapMode wrap_mode;
PangoLanguage *language;
guint invisible : 1;
guint bg_full_height : 1;
guint editable : 1;
guint realized : 1;

**/
static getAttributes ( Tcl_Interp *interp, GtkTextAttributes *values )
{
#ifdef DEBUG_TEXT
	g_print ( " % s\n", __FUNCTION__ );
#endif


	Tcl_Obj *resList;

	resList = Tcl_NewListObj ( 0, NULL );

	gchar *justify;
	gchar *direction;
	gchar *font;

	justify = "left";
	direction = "none";
	font = "";

	font = pango_font_description_to_string ( values->font );

	switch ( values->justification )
	{
		case GTK_JUSTIFY_LEFT:
			{
				justify = "left";
			}
			break;
		case GTK_JUSTIFY_RIGHT:
			{
				justify = "right";
			}			break;
		case GTK_JUSTIFY_CENTER:
			{
				justify = "center";
			}			break;
		case GTK_JUSTIFY_FILL:
			{
				justify = "fill";
			}			break;
		default:
			{

#ifdef DEBUG_TEXT
				g_print ( "no justify %s\n", __FUNCTION__ );
#endif

			}
	}

	switch ( values->direction )
	{
		case GTK_TEXT_DIR_NONE:
			{
				direction = "none";
			}
			break;
		case GTK_TEXT_DIR_LTR:
			{
				direction = "left - to - right";
			}
			break;
		case GTK_TEXT_DIR_RTL:
			{
				direction = "right - to - left";
			}
			break;
		default:
			{
#ifdef DEBUG_TEXT
				g_print ( "no direction %s\n", __FUNCTION__ );
#endif

			}
	}


	Tcl_ListObjAppendElement ( interp, resList, Tcl_NewStringObj ( "justification", -1 ) );
	Tcl_ListObjAppendElement ( interp, resList, Tcl_NewStringObj ( justify, -1 ) );

	Tcl_ListObjAppendElement ( interp, resList, Tcl_NewStringObj ( "direction", -1  ) );
	Tcl_ListObjAppendElement ( interp, resList, Tcl_NewStringObj ( direction, -1 ) );

	Tcl_ListObjAppendElement ( interp, resList, Tcl_NewStringObj ( "font", -1  ) );
	Tcl_ListObjAppendElement ( interp, resList, Tcl_NewStringObj ( font, -1 ) );

	Tcl_ListObjAppendElement ( interp, resList, Tcl_NewStringObj ( "font_scale", -1  ) );
	Tcl_ListObjAppendElement ( interp, resList, Tcl_NewDoubleObj ( values->font_scale ) );

	Tcl_ListObjAppendElement ( interp, resList, Tcl_NewStringObj ( "left_margin", -1  ) );
	Tcl_ListObjAppendElement ( interp, resList, Tcl_NewIntObj ( values->left_margin ) );

	Tcl_ListObjAppendElement ( interp, resList, Tcl_NewStringObj ( "indent", -1  ) );
	Tcl_ListObjAppendElement ( interp, resList, Tcl_NewIntObj ( values->indent ) );

	Tcl_ListObjAppendElement ( interp, resList, Tcl_NewStringObj ( "right_margin", -1  ) );
	Tcl_ListObjAppendElement ( interp, resList, Tcl_NewIntObj ( values->right_margin ) );

	Tcl_ListObjAppendElement ( interp, resList, Tcl_NewStringObj ( "pixels_above_lines", -1  ) );
	Tcl_ListObjAppendElement ( interp, resList, Tcl_NewIntObj ( values-> pixels_above_lines ) );

	Tcl_ListObjAppendElement ( interp, resList, Tcl_NewStringObj ( "pixels_below_lines", -1  ) );
	Tcl_ListObjAppendElement ( interp, resList, Tcl_NewIntObj ( values->pixels_below_lines ) );

	Tcl_ListObjAppendElement ( interp, resList, Tcl_NewStringObj ( "pixels_inside_wrap", -1  ) );
	Tcl_ListObjAppendElement ( interp, resList, Tcl_NewIntObj ( values->pixels_inside_wrap ) );

	Tcl_ListObjAppendElement ( interp, resList, Tcl_NewStringObj ( "tabs", -1  ) );
	Tcl_ListObjAppendElement ( interp, resList, Tcl_NewStringObj ( "tab - array", -1 ) );

	Tcl_ListObjAppendElement ( interp, resList, Tcl_NewStringObj ( "wrap_mode", -1  ) );
	Tcl_ListObjAppendElement ( interp, resList, Tcl_NewStringObj ( "wrap", -1 ) );

	Tcl_ListObjAppendElement ( interp, resList, Tcl_NewStringObj ( "language", -1  ) );
	Tcl_ListObjAppendElement ( interp, resList, Tcl_NewStringObj ( "text - language", -1 ) );

	Tcl_ListObjAppendElement ( interp, resList, Tcl_NewStringObj ( "invisible", -1  ) );
	Tcl_ListObjAppendElement ( interp, resList, Tcl_NewIntObj ( values->invisible ) );

	Tcl_ListObjAppendElement ( interp, resList, Tcl_NewStringObj ( "bg_full_height", -1  ) );
	Tcl_ListObjAppendElement ( interp, resList, Tcl_NewIntObj ( values->bg_full_height ) );

	Tcl_ListObjAppendElement ( interp, resList, Tcl_NewStringObj ( "editable", -1  ) );
	Tcl_ListObjAppendElement ( interp, resList, Tcl_NewIntObj ( 1 ) );

	Tcl_ListObjAppendElement ( interp, resList, Tcl_NewStringObj ( "realized", -1  ) );
	Tcl_ListObjAppendElement ( interp, resList, Tcl_NewIntObj ( values->realized ) );

	Tcl_SetObjResult ( interp, resList );

}


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
		g_print ( "*  search forwards % s\n", ( gchar*  ) Tcl_GetString ( objv[cmdNo+2] ) );
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
	g_print ( "INSERT SOME INITIAL TEXT\n" );
#endif

	return TCL_OK;
}

/**
\brief	Set equidistant tab stops meausure in pixels.
http://developer.gnome.org/pango/stable/pango-Tab-Stops.html
**/
static int gnoclOptTabs ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{

#ifdef DEBUG_TEXT
	g_print ( "%s SET SOME INITIAL TABS\n", __FUNCTION__ );
#endif

	gint pos;

	Tcl_GetIntFromObj ( interp, opt->val.obj, &pos );

	PangoTabArray *tab_array = pango_tab_array_new_with_positions ( 1, TRUE, PANGO_TAB_LEFT, pos );
	gtk_text_view_set_tabs ( GTK_TEXT_VIEW ( obj ), tab_array );

	/*
	 * An alternative way...
	*/
	/*
	PangoTabArray *tab_array;
	gint initial_size,tab_index, location;
	PangoTabAlign alignment;

	initial_size = 2;

	tab_array = pango_tab_array_new (initial_size,1);

	pango_tab_array_set_tab (tab_array,0,PANGO_TAB_LEFT, 100);
	pango_tab_array_set_tab (tab_array,1,PANGO_TAB_LEFT, 200);

	gtk_text_view_set_tabs ( GTK_TEXT_VIEW ( obj ), tab_array );

	pango_tab_array_free (tab_array);
	*/
	return TCL_OK;
}

/**
\brief	Set text tag attribute.
\notes  Keywords:
        raise
        lower
        top
        bottom
**/
static int gnoclOptTextTagPriority ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{

#ifdef DEBUG_TEXT
	debugStep ( __FUNCTION__, 1.0 );
#endif

	static char *opts[] =
	{
		"raise", "lower", "top", "bottom",
		NULL
	};

	static enum  popupOptionsIdx
	{
		raiseIdx, lowerIdx, topIdx, bottomIdx
	};

	gint idx;

	gint priority;

	gint max;

	GtkTextTag *tag;

	tag = GTK_TEXT_TAG ( obj );

	max = gtk_text_tag_table_get_size ( tag->table );

	priority = gtk_text_tag_get_priority ( tag );

	getIdx ( opts,  Tcl_GetStringFromObj ( opt->val.obj, NULL ), &idx );

	switch ( idx )
	{
		case raiseIdx:
			{
				priority++;
			}
			break;
		case lowerIdx:
			{
				priority--;
			}
			break;
		case topIdx:
			{
				priority = max - 1;
			}
			break;
		case bottomIdx:
			{
				priority = 0;
			}
			break;
		default:
			{
				Tcl_GetIntFromObj ( interp, opt->val.obj, &priority );
			}
	}

	/* check limits */
	if ( priority < 0 )
	{
		priority = 0;
	}

	if ( priority >= max )
	{
		priority--;
	}

	gtk_text_tag_set_priority ( tag, priority );

	return TCL_OK;
}

/**
\brief	Set text tag attribute.
**/
static int gnoclOptTagBackgroundStipple ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	GtkWidget *container;
	container =  gtk_widget_get_parent ( obj );

#ifdef DEBUG_TEXT
	debugStep ( __FUNCTION__, 1.0 );
	g_print ( "Feature not yet implemented\n" );
#endif

	return TCL_OK;
}

/**
\brief	Set text tag attribute.
**/
static int gnoclOptTagTextDirection ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{

#ifdef DEBUG_TEXT
	debugStep ( __FUNCTION__, 1.0 );
#endif

	/* options: leftRight | rightLeft | none */

	/*
	typedef enum
	{
	GTK_TEXT_DIR_NONE,
	GTK_TEXT_DIR_LTR,
	GTK_TEXT_DIR_RTL,
	} GtkTextDirection;
	*/

	int idx;

	const char *txt[] = { "none", "leftRight", "rightLeft", NULL };

	if ( Tcl_GetIndexFromObj ( NULL, opt->val.obj, txt, NULL, TCL_EXACT, &idx ) != TCL_OK )
	{
		Tcl_AppendResult ( interp, "Unknown direction \"", Tcl_GetString ( opt->val.obj ), "\". Must be one of none, leftRight or rightLeft.", NULL );
		return TCL_ERROR;
	}

	g_object_set ( obj, opt->propName, idx, NULL );

	return TCL_OK;
}

/**
\brief	Set text tag attribute.
**/
static int gnoclOptTextTagForegroundStipple ( Tcl_Interp * interp, GnoclOption * opt, GObject * obj, Tcl_Obj **ret )
{
	GtkWidget *container;
	container =  gtk_widget_get_parent ( obj );

#ifdef DEBUG_TEXT
	debugStep ( __FUNCTION__, 1.0 );
	g_print ( "Feature not yet implemented\n" );
#endif

	return TCL_OK;
}

/**
\brief	Set text tag attribute.
**/
static int gnocOptTextTagLanguage ( Tcl_Interp * interp, GnoclOption * opt, GObject * obj, Tcl_Obj **ret )
{

	gchar *lang;
	gint i;

	lang = Tcl_GetStringFromObj ( opt->val.obj, NULL );

#ifdef DEBUG_TEXT
	debugStep ( __FUNCTION__, 1.0 );
	g_print ( "Language = %s\n", lang );
#endif

	i = getLanguage ( lang );

#ifdef DEBUG_TEXT
	g_print ( "idx = %d\n", i );
#endif



	return TCL_OK;
}

/**
\brief	Set text tag attribute.
**/
static int gnoclOptTextTagTabs ( Tcl_Interp * interp, GnoclOption * opt, GObject * obj, Tcl_Obj **ret )
{


#ifdef DEBUG_TEXT
	debugStep ( __FUNCTION__, 1.0 );
	g_print ( "Feature not yet implemented\n" );
#endif

	return TCL_OK;
}

/**
\brief	Set text tag attribute.
**/
static int gnoclOptTextTagVariant ( Tcl_Interp * interp, GnoclOption * opt, GObject * obj, Tcl_Obj **ret )
{

#ifdef DEBUG_TEXT
	debugStep ( __FUNCTION__, 1.0 );
	g_print ( "Feature not yet implemented\n" );
#endif

	return TCL_OK;
}


/**
\brief
*/
static gboolean doOnTextEnterLeave ( GtkWidget * widget, GdkEventMotion * event, 	gpointer data )
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
\brief	Add default set of tag with pango compliant tagnames.
**/
static int gnoclOptMarkupTags ( Tcl_Interp * interp, GnoclOption * opt, GObject * obj, Tcl_Obj **ret )
{
#ifdef DEBUG_TEXT
	g_print ( "%s %d\n", __FUNCTION__, Tcl_GetString ( opt->val.obj ) );
#endif

	extern gint usemarkup;

	assert ( strcmp ( opt->optName, "-markupTags" ) == 0 );

	/* modify this to destroy tags */
	if ( strcmp ( Tcl_GetString ( opt->val.obj ), "1" ) == 0 )
	{
		usemarkup = 1;
		/* create default markup tag set */
	}

	else
	{
		return TCL_OK;
		usemarkup = 1;
		/* delete markup tags */
	}

#ifdef DEBUG_TEXT
	g_print ( "usemarkup = %d\n", usemarkup );
#endif

	GtkTextBuffer *buffer = gtk_text_view_get_buffer ( GTK_TEXT_VIEW ( obj ) );

	/* convenience tags */
	// 'b','i','s','u','tt','sub','sup','small','big'
	gtk_text_buffer_create_tag ( buffer, "<b>", "weight", PANGO_WEIGHT_BOLD, NULL );
	gtk_text_buffer_create_tag ( buffer, "<i>", "style", PANGO_STYLE_ITALIC, NULL );
	gtk_text_buffer_create_tag ( buffer, "<s>", "strikethrough", 1, NULL );
	gtk_text_buffer_create_tag ( buffer, "<u>", "underline", PANGO_UNDERLINE_SINGLE, NULL );
	gtk_text_buffer_create_tag ( buffer, "<tt>", "font", "Monospace", NULL );
	gtk_text_buffer_create_tag ( buffer, "<sub>", "scale", PANGO_SCALE_SMALL, "rise", -10, NULL );
	gtk_text_buffer_create_tag ( buffer, "<sup>", "scale", PANGO_SCALE_SMALL, "rise",  +10, NULL );
	gtk_text_buffer_create_tag ( buffer, "<small>", "scale", PANGO_SCALE_SMALL, NULL );
	gtk_text_buffer_create_tag ( buffer, "<big>", "scale", PANGO_SCALE_LARGE, NULL );

	/* foreground colours */
	// 'red', 'green', 'blue', 'cyan', 'magenta', 'yellow', 'black', 'gray', 'white'
	gtk_text_buffer_create_tag ( buffer, "<span foreground='red'>",   "foreground", "red", NULL );
	gtk_text_buffer_create_tag ( buffer, "<span foreground='green'>", "foreground", "green", NULL );
	gtk_text_buffer_create_tag ( buffer, "<span foreground='blue'>",  "foreground", "blue", NULL );

	gtk_text_buffer_create_tag ( buffer, "<span foreground='black'>", "foreground", "black", NULL );
	gtk_text_buffer_create_tag ( buffer, "<span foreground='gray'>",  "foreground", "gray", NULL );
	gtk_text_buffer_create_tag ( buffer, "<span foreground='white'>", "foreground", "white", NULL );

	/* background colours */
	// 'red', 'green', 'blue', 'cyan', 'magenta', 'yellow', 'black', 'gray', 'white'
	gtk_text_buffer_create_tag ( buffer, "<span background='cyan'>",    "background", "cyan", NULL );
	gtk_text_buffer_create_tag ( buffer, "<span background='magenta'>", "background", "magenta", NULL );
	gtk_text_buffer_create_tag ( buffer, "<span background='yellow'>",  "background", "yellow", NULL );
	gtk_text_buffer_create_tag ( buffer, "<span background='orange'>",  "background", "orange", NULL );


	gtk_text_buffer_create_tag ( buffer, "<span background='black'>", "background", "black", NULL );
	gtk_text_buffer_create_tag ( buffer, "<span background='gray'>",  "background", "gray", NULL );
	gtk_text_buffer_create_tag ( buffer, "<span background='white'>", "background", "white", NULL );

	/* default typefaces */
	// 'serif' or 'sans'
	gtk_text_buffer_create_tag ( buffer, "<span face='sans'>", "font", "serif", NULL );
	gtk_text_buffer_create_tag ( buffer, "<span face='serif'>", "font", "sans", NULL );

	/* font scaling */
	// 'xx-small', 'x-small', 'small', 'medium', 'large', 'x-large', 'xx-large'
	gtk_text_buffer_create_tag ( buffer, "<span size='xx-small'>", "scale", PANGO_SCALE_XX_SMALL, NULL );
	gtk_text_buffer_create_tag ( buffer, "<span size='x-small'>" , "scale", PANGO_SCALE_X_SMALL, NULL );
	gtk_text_buffer_create_tag ( buffer, "<span size='small'>"   , "scale", PANGO_SCALE_SMALL, NULL );
	gtk_text_buffer_create_tag ( buffer, "<span size='medium'>"  , "scale", PANGO_SCALE_MEDIUM, NULL );
	gtk_text_buffer_create_tag ( buffer, "<span size='large'>"   , "scale", PANGO_SCALE_LARGE, NULL );
	gtk_text_buffer_create_tag ( buffer, "<span size='x-large'>" , "scale", PANGO_SCALE_X_LARGE, NULL );
	gtk_text_buffer_create_tag ( buffer, "<span size='xx-large'>", "scale", PANGO_SCALE_XX_LARGE, NULL );

	/* font weight */
	//'ultralight', 'light', 'normal', 'bold', 'ultrabold', 'heavy'
	gtk_text_buffer_create_tag ( buffer, "<span weight='light'>"    , "weight", PANGO_WEIGHT_LIGHT, NULL );
	gtk_text_buffer_create_tag ( buffer, "<span weight='normal'>"   , "weight", PANGO_WEIGHT_NORMAL, NULL );
	gtk_text_buffer_create_tag ( buffer, "<span weight='bold'>"     , "weight", PANGO_WEIGHT_BOLD, NULL );
	gtk_text_buffer_create_tag ( buffer, "<span weight='ultrabold'>", "weight", PANGO_WEIGHT_ULTRABOLD, NULL );
	gtk_text_buffer_create_tag ( buffer, "<span weight='heavy'>"    , "weight", PANGO_WEIGHT_HEAVY, NULL );

	/* variant */
	// 'normal' or 'smallcaps'
	gtk_text_buffer_create_tag ( buffer, "<span variant='normal'>"    , "variant", PANGO_VARIANT_NORMAL, NULL );
	gtk_text_buffer_create_tag ( buffer, "<span variant='smallcaps'>" , "variant", PANGO_VARIANT_SMALL_CAPS, NULL );

	/* stretch */
	// 'ultracondensed', 'extracondensed', 'condensed', 'semicondensed', 'normal', 'semiexpanded', 'expanded', 'extraexpanded', 'ultraexpanded'
	gtk_text_buffer_create_tag ( buffer, "<span stretch='ultracondensed'>" , "stretch", PANGO_STRETCH_ULTRA_CONDENSED, NULL );
	gtk_text_buffer_create_tag ( buffer, "<span stretch='extracondensed'>" , "stretch", PANGO_STRETCH_EXTRA_CONDENSED, NULL );
	gtk_text_buffer_create_tag ( buffer, "<span stretch='condensed'>"      , "stretch", PANGO_STRETCH_CONDENSED, NULL );
	gtk_text_buffer_create_tag ( buffer, "<span stretch='normal'>"         , "stretch", PANGO_STRETCH_NORMAL, NULL );
	gtk_text_buffer_create_tag ( buffer, "<span stretch='semicondensed'>"  , "stretch", PANGO_STRETCH_SEMI_CONDENSED, NULL );
	gtk_text_buffer_create_tag ( buffer, "<span stretch='expanded'>"       , "stretch", PANGO_STRETCH_EXPANDED, NULL );
	gtk_text_buffer_create_tag ( buffer, "<span stretch='extraexpanded'>"  , "stretch", PANGO_STRETCH_EXTRA_EXPANDED, NULL );
	gtk_text_buffer_create_tag ( buffer, "<span stretch='ultraexpanded'>"  , "stretch", PANGO_STRETCH_ULTRA_EXPANDED, NULL );

	/* underline */
	// 'none', 'single', 'double', 'low', 'error'
	gtk_text_buffer_create_tag ( buffer, "<span underline='none'>"   , "underline", PANGO_UNDERLINE_NONE, NULL );
	gtk_text_buffer_create_tag ( buffer, "<span underline='single'>" , "underline", PANGO_UNDERLINE_SINGLE, NULL );
	gtk_text_buffer_create_tag ( buffer, "<span underline='double'>" , "underline", PANGO_UNDERLINE_DOUBLE, NULL );
	gtk_text_buffer_create_tag ( buffer, "<span underline='low'>"    , "underline", PANGO_UNDERLINE_LOW, NULL );
	gtk_text_buffer_create_tag ( buffer, "<span underline='error'>"  , "underline", PANGO_UNDERLINE_ERROR, NULL );

	return TCL_OK;
}

/**
\brief
**/
static int gnoclOptTextOnEnterLeave ( Tcl_Interp * interp, GnoclOption * opt, GObject * obj, Tcl_Obj **ret )
{
	GtkWidget *container;
	container =  gtk_widget_get_parent ( obj );

	assert ( strcmp ( opt->optName, "-onEnter" ) == 0 || strcmp ( opt->optName, "-onLeave" ) == 0 );
	return gnoclConnectOptCmd ( interp, obj, opt->optName[3] == 'E' ? "enter-notify-event" : "leave-notify-event", G_CALLBACK ( doOnTextEnterLeave ), opt, NULL, ret );
}

/**
\brief
**/
static GdkBitmap *getBitmapMask ( gchar * filename )
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
static void doOnInsertPixbuf ( GtkTextBuffer * textbuffer, GtkTextIter * location, GdkPixbuf * pixbuf, gpointer user_data )
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
	ps[1].val.i   = gtk_text_iter_get_line ( location );
	ps[2].val.i   = gtk_text_iter_get_line_offset ( location );
	ps[3].val.str = gnoclGetNameFromPixBuf ( pixbuf );

	gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 );
}


/**
\brief
\author     William J Giddings
\date       30-Apr-09
**/
int gnoclOptOnInsertPB ( Tcl_Interp * interp, GnoclOption * opt, GObject * obj, Tcl_Obj **ret )
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
static void doOnToggleCursorVisible ( GtkTextView * text_view, gpointer user_data )
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
	ps[1].val.str = gtk_widget_get_name ( GTK_WIDGET ( text_view ) );
	ps[2].val.i = gtk_text_view_get_cursor_visible ( text_view );

	gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 );
}

/**
\brief
\author		William J Giddings
\date		30/04/2010
\note
**/
int gnoclOptOnToggleCursorVisible ( Tcl_Interp * interp, GnoclOption * opt, GObject * obj, Tcl_Obj **ret )
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
static const int dataIdx = 4;
static const int baseColorIdx = 5;
static const int variableIdx = 6;
static const int onChangedIdx = 7;
static const int baseFontIdx = 8;
static const int tooltipIdx = 9;

static GnoclOption textOptions[] =
{
	/*  textView */

	/*  gnocl-specific options - cget implemented */
	{ "-scrollbar", GNOCL_OBJ, NULL },
	{ "-text", GNOCL_STRING, NULL},
	{ "-buffer", GNOCL_STRING, NULL},
	{ "-useUndo", GNOCL_STRING, NULL},
	{ "-data", GNOCL_OBJ, "", gnoclOptData },
	{ "-baseColor", GNOCL_OBJ, "normal", gnoclOptGdkColorBase },
	{ "-variable", GNOCL_STRING, NULL },
	{ "-onChanged", GNOCL_STRING, NULL },
	{ "-baseFont", GNOCL_OBJ, "Sans 14", gnoclOptGdkBaseFont },
	{ "-tooltip", GNOCL_OBJ, "", gnoclOptTooltip },

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

	{ "-markupTags", GNOCL_OBJ, "", gnoclOptMarkupTags },
	{ "-acceptsTab", GNOCL_BOOL, "accepts-tab" },
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
	{ "-tabs", GNOCL_OBJ, "tabs", gnoclOptTabs}, /* "tabs" */
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

	{ "-onShowHelp", GNOCL_OBJ, "", gnoclOptOnShowHelp },
	{ "-name", GNOCL_STRING, "name" },
	{ "-visible", GNOCL_BOOL, "visible" },
	{ "-sensitive", GNOCL_BOOL, "sensitive" },
	{ "-baseFont", GNOCL_OBJ, "Sans 14", gnoclOptGdkBaseFont },
	//{ "-baseColor", GNOCL_OBJ, "normal", gnoclOptGdkColorBase },

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
	{ "-borderWidth", GNOCL_OBJ, "border-width", gnoclOptPadding },

	/* drag and drop functionality taken from box.c */
	{ "-dropTargets", GNOCL_LIST, "t", gnoclOptDnDTargets },
	{ "-dragTargets", GNOCL_LIST, "s", gnoclOptDnDTargets },
	{ "-onDropData", GNOCL_OBJ, "", gnoclOptOnDropData },
	{ "-onDragData", GNOCL_OBJ, "", gnoclOptOnDragData },
	{ "-hasTooltip", GNOCL_BOOL, "has-tooltip" },
	{ "-onQueryTooltip", GNOCL_OBJ, "", gnoclOptOnQueryToolTip },
	{ "-onDestroy", GNOCL_OBJ, "destroy", gnoclOptCommand },

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
int posToIter ( Tcl_Interp * interp, Tcl_Obj * obj, GtkTextBuffer * buffer, GtkTextIter * iter )
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
static int applyTag ( GtkTextBuffer * buffer, Tcl_Interp * interp, int objc, Tcl_Obj *  const objv[], int cmdNo )
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
static void deleteTag  ( GtkTextTag * tag, gpointer data )
{
	gtk_text_tag_table_remove ( data, tag );
}


/**
\brief
\note
**/
static int removeTag ( GtkTextBuffer * buffer, Tcl_Interp * interp, int objc, Tcl_Obj *  const objv[], int cmdNo )
{
	/*  declare some variables */
	// console error messaging
	GnoclOption insertOptions[] =
	{
		{ "-tags", GNOCL_LIST, NULL },
		{ NULL }
	};
	const int tagsIdx = 0;
	gint startOffset;
	gint endOffset;
	int ret = TCL_ERROR;

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


	/* check for keyword all */


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
static int markCmd ( GtkTextBuffer * buffer, Tcl_Interp * interp, int objc, Tcl_Obj *  const objv[], int cmdNo )
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
				Tcl_ListObjAppendElement ( interp, resList, Tcl_NewStringObj ( str, -1 ) );
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

		case CgetIdx:	// markCmd
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

    pathName tag add tagName index1 ?index2 index1 index2 ...?
    pathName tag bind tagName ?sequence? ?script?
    pathName tag cget tagName option
    pathName tag configure tagName ?option? ?value? ?option value ...?
    pathName tag delete tagName ?tagName ...?
    pathName tag lower tagName ?belowThis?
    pathName tag names ?index?
    pathName tag nextrange tagName index1 ?index2?
    pathName tag prevrange tagName index1 ?index2?
    pathName tag properties tagName
    pathName tag raise tagName ?aboveThis?
    pathName tag ranges tagName
    pathName tag remove tagName index1 ?index2 index1 index2 ...?

\todo
	some Tk-compatible command synonyms
		add + apply?
	allow list arguments for add, remove and delete

**/
int tagCmd ( GtkTextBuffer * buffer, Tcl_Interp * interp, int objc, Tcl_Obj *  const objv[], int cmdNo )
{
#ifdef DEBUG_TAGS
	g_print ( "%s\n", __FUNCTION__ );
#endif

	// widgetname tag add tagname start_index end_index
	// widgetname tag remove tagname start_index end_index

	// const char *cmds[] = { "create", "configure", "add", "delete", "remove", NULL };
	// enum cmdIdx { CreateIdx, ConfigureIdx, AddIdx, DeleteIdx};
	const char *cmds[] =
	{
		"cget", "create", "configure", "apply",
		"delete", "remove", "get", "clear", "set",
		"ranges", "names", "raise", "lower",
		"copy", "properties",
		NULL
	};
	enum cmdIdx
	{
		CgetIdx, CreateIdx, ConfigureIdx, ApplyIdx,
		DeleteIdx, RemoveIdx, GetIdx, ClearIdx, SetIdx,
		RangesIdx, NamesIdx, RaiseIdx, LowerIdx,
		CopyIdx, PropertiesIdx,
	};

	/*  see also list.c */

	GnoclOption tagOptions[] =
	{

		/* new options added 30/04/11, may need some revision */

		{"-marginAccumulate", GNOCL_BOOL, "accumulative-margin"},
		{"-backgroundFullHeight", GNOCL_BOOL, "background-full-height"},
		{"-backgroundStipple", GNOCL_OBJ, gnoclOptTagBackgroundStipple},
		{"-direction", GNOCL_OBJ, "direction", gnoclOptTagTextDirection},
		{"-foregroundStipple", GNOCL_OBJ, "foreground-stipple", gnoclOptTextTagForegroundStipple},
		{"-indent", GNOCL_INT, "indent"},
		{"-language", GNOCL_OBJ, "language", gnocOptTextTagLanguage},
		{"-leftMargin", GNOCL_INT, "left-margin"},
		{"-name", GNOCL_STRING, "name"},
		{"-pixelsInsideWrap", GNOCL_INT, "pixels-inside-wrap"},
		{"-priority", GNOCL_OBJ, "", gnoclOptTextTagPriority},
		{"-rightMargin", GNOCL_INT, "right-margin"},
		{"-rise", GNOCL_INT, "rise"},
		{"-scale", GNOCL_DOUBLE, "scale"},
		{"-tabs", GNOCL_OBJ, "tabs", gnoclOptTextTagTabs},
		{"-variant", GNOCL_OBJ, "variant", gnoclOptTextTagVariant},
		{"-weight", GNOCL_INT, "weight"},

		/*--------------- existing options ---------------*/

		{ "-background", GNOCL_OBJ, "background-gdk", gnoclOptGdkColor },
		{ "-editable", GNOCL_BOOL, "editable" },
		{ "-foreground", GNOCL_OBJ, "foreground-gdk", gnoclOptGdkColor },
		{ "-font", GNOCL_STRING, "font" },
		{ "-fontFamily", GNOCL_STRING, "family" },
		{ "-fontStyle", GNOCL_OBJ, "style", gnoclOptPangoStyle },
		{ "-fontVariant", GNOCL_OBJ, "variant", gnoclOptPangoVariant },
		{ "-fontWeight", GNOCL_OBJ, "weight", gnoclOptPangoWeight },
		{ "-fontRise", GNOCL_OBJ, "rise", gnoclOptPangoScaledInt },
		{ "-fontStretch", GNOCL_OBJ, "stretch", gnoclOptPangoStretch },
		{ "-fontSize", GNOCL_OBJ, "size", gnoclOptPangoScaledInt },
		//{ "-fontScale", GNOCL_OBJ, "scale", gnoclOptScale },
		{ "-invisible", GNOCL_BOOL, "invisible" },
		{ "-justification", GNOCL_OBJ, "justification", gnoclOptJustification },
		{ "-paragraph", GNOCL_OBJ, "paragraph-background-gdk", gnoclOptGdkColor },
		{ "-pixelsAboveLines", GNOCL_INT, "pixels-above-lines" },
		{ "-pixelsBelowLines", GNOCL_INT, "pixels-below-lines" },
		{ "-size", GNOCL_INT, "size" },
		{ "-strikethrough", GNOCL_BOOL, "strikethrough" },
		{ "-sizePoints", GNOCL_DOUBLE, "size-points" },
		{ "-underline", GNOCL_OBJ, "underline", gnoclOptUnderline },
		{ "-wrapMode", GNOCL_OBJ, "wrap-mode", gnoclOptWrapmode },

		{ "-data", GNOCL_OBJ, "", gnoclOptData },

		/* settings, what to do with these? */

		/*
				{"-backgroundFullHeightSet", GNOCL_BOOL, "background-full-height-set"},
				{"-backgroundSet", GNOCL_BOOL, "background-set"},
				{"-backgroundStippleSet", GNOCL_BOOL, "background-stipple-set"},
				{"-editableSet", GNOCL_BOOL, "editable-set"},
				{"-fontFamiltSet", GNOCL_BOOL, "family-set"},
				{"-foregroundStippleSet", GNOCL_BOOL, "foreground-stipple-set"},
				{"-indentSet", GNOCL_BOOL, "indent-set"},
				{"-invisibleSet", GNOCL_BOOL, "invisible-set"},
				{"-justificationSet", GNOCL_BOOL, "justification-set"},
				{"-languageSet", GNOCL_BOOL, "language-set"},
				{"-leftMarginSet", GNOCL_BOOL, "left-margin-set"},
				{"-paragraphBackgroundSet", GNOCL_BOOL, "paragraph-background-set"},
				{"-pixelsAboveLinesSet", GNOCL_BOOL, "pixels-above-lines-set"},
				{"-pixelsBelowLinesSet", GNOCL_BOOL, "pixels-below-lines-set"},
				{"-pixelsInsideWrapSet", GNOCL_BOOL, "pixels-inside-wrap-set"},
				{"-rightMarginSet", GNOCL_BOOL, "right-margin-set"},
				{"-riseSet", GNOCL_BOOL, "rise-set"},
				{"-scaleSet", GNOCL_BOOL, "scale-set"},
				{"-sizeSet", GNOCL_BOOL, "size-set"},
				{"-tabsSet", GNOCL_BOOL, "tabs-set"},
				{"-underlineSet", GNOCL_BOOL, "underline-set"},
				{"-variantSet", GNOCL_BOOL, "variant-set"},
				{"-weightSet", GNOCL_BOOL, "weight-set"},
				{"-wrapModeSet", GNOCL_BOOL, "wrap-mode-set"},
		*/

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

		case NamesIdx:	// return a list of all tag names
			{
#ifdef DEBUG_TAGS
				g_print ( "\ttag names\n" );
#endif
				GtkTextTagTable *tagtable = gtk_text_buffer_get_tag_table ( buffer );

				gchar names[512];

				Tcl_Obj *resList;
				resList = Tcl_NewListObj ( 0, NULL );
				/* create a tcl list */

				/*  get the settings of each tag in the buffer*/
				/*  note, pass the address of the pointer to the data assigned by the called function */
				gtk_text_tag_table_foreach ( tagtable, getTagName , &resList );

				Tcl_SetObjResult ( interp, resList );
				/*  reset the outsput string by using null pointers */
				//gnoclGetTagProperties ( NULL, NULL );

#ifdef  DEBUG_TAGS
				g_print ( "tag names\n" );
#endif

				return TCL_OK;
			}
			break;
		case PropertiesIdx:
			{
#ifdef DEBUG_TAGS
				g_print ( "\ttag properties %s\n", Tcl_GetString ( objv[3] ) );
#endif

				Tcl_Obj *resList;
				resList = Tcl_NewListObj ( 0, NULL );

				GtkTextTagTable *table = gtk_text_buffer_get_tag_table ( buffer );
				GtkTextTag *tag = gtk_text_tag_table_lookup ( table, Tcl_GetString ( objv[3] ) );

				gnoclGetTagProperties ( tag, resList );

				Tcl_SetObjResult ( interp, resList );

				return TCL_OK;

			}
			break;
		case CopyIdx:
			{
#ifdef DEBUG_TAGS
				g_print ( "\ttag copy\n" );
#endif
				listParameters ( objc, objv, __FUNCTION__ );

				GtkScrolledWindow *scrolled = gnoclGetWidgetFromName ( Tcl_GetString ( objv[3] ), interp );
				GtkTextView *text = GTK_TEXT_VIEW ( gtk_bin_get_child ( GTK_BIN ( scrolled ) ) );
				GtkTextBuffer *buffer2 = gtk_text_view_get_buffer ( text );
				GtkTextTagTable *table = gtk_text_buffer_get_tag_table ( buffer2 );

				buffer->tag_table = table;

				g_object_ref ( buffer->tag_table );

				table->buffers = g_slist_prepend ( table->buffers, buffer );

			}
			break;
		case RaiseIdx:
			{
#ifdef DEBUG_TAGS
				g_print ( "\ttag raise\n" );
#endif

				GtkTextTagTable *table = gtk_text_buffer_get_tag_table ( buffer );

				GtkTextTag *tag1;
				GtkTextTag *tag2;

				tag1 = gtk_text_tag_table_lookup ( table, Tcl_GetString ( objv[cmdNo+2] ) );
				tag2 = gtk_text_tag_table_lookup ( table, Tcl_GetString ( objv[cmdNo+3] ) );

				gint priority = gtk_text_tag_get_priority ( tag1 );
				priority++;
				gtk_text_tag_set_priority ( tag2, priority );

			}
			break;
		case LowerIdx:
			{
#ifdef DEBUG_TEXT
				g_print ( "tag lower\n" );
#endif

				GtkTextTagTable *table = gtk_text_buffer_get_tag_table ( buffer );

				GtkTextTag *tag1;
				GtkTextTag *tag2;

				tag1 = gtk_text_tag_table_lookup ( table, Tcl_GetString ( objv[cmdNo+2] ) );
				tag2 = gtk_text_tag_table_lookup ( table, Tcl_GetString ( objv[cmdNo+3] ) );

				gint priority = gtk_text_tag_get_priority ( tag1 );

				if ( priority >= 1 )
				{
					priority++;
				}

				gtk_text_tag_set_priority ( tag2, priority );
			}

		case RangesIdx:
			{
#ifdef DEBUG_TAGS
				g_print ( "\ttag ranges\n" );
#endif
				gnoclGetTagRanges ( interp, buffer, Tcl_GetString ( objv[cmdNo+1] ) );
			}
			break;
		case SetIdx:
			{
				/* control tag option application settings */
			}
			break;
		case ClearIdx:
			{
				GtkTextTagTable *tagtable = gtk_text_buffer_get_tag_table ( buffer );

				/*  get the settings of each tag in the buffer*/
				/*  note, pass the address of the pointer to the data assigned by the called function */
				gtk_text_tag_table_foreach ( tagtable, deleteTag , tagtable );

			}
			break;
		case CgetIdx:	// tagCmd
			{
				g_print ( "tag CgetIdx 1\n" );

				int     idx;
				Tcl_Obj *resList;

				GtkTextTagTable *tagtable = gtk_text_buffer_get_tag_table ( buffer );
				g_print ( "tag CgetIdx 2\n" );
				GtkTextTag *tag = gtk_text_tag_table_lookup ( tagtable, Tcl_GetString ( objv[cmdNo+1] ) );
				g_print ( "tag CgetIdx 3\n" );

				switch ( gnoclTagCget ( interp, objc, objv, G_OBJECT ( tag ), tagOptions, &idx ) )
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
							return gnoclCgetOne ( interp, objv, G_OBJECT ( tag ), tagOptions, &idx );
						}
				}

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

				static char *tagOpt[] =
				{
					"-on", "-off", "-all", NULL
				};

				static enum  tagOptIdx
				{
					OnIdx, OffIdx, AllIdx
				};

				gint idx;

				/*  check the number of arguments */
				if ( objc < 4 || objc > 5 )
				{
					Tcl_WrongNumArgs ( interp, cmdNo + 1, objv, "position opts" );
					return TCL_ERROR;
				}

				/*  convert the position to a pointer */
				if ( posToIter ( interp, objv[3], buffer, &iter ) != TCL_OK )
				{
					return -1;
				}


				if ( objc == 5 )
				{

					getIdx ( tagOpt, Tcl_GetString ( objv[4] ), &idx );

//g_print ("here I am... idx = %d\n", idx);

					switch ( idx )
					{
						case OnIdx:
							{
								/* build a list of tagOn changes */
								//g_print ("get onTags\n");
								tagList = gtk_text_iter_get_toggled_tags ( &iter, TRUE );
							}
							break;
						case OffIdx:
							{
								/* build a list of tagOff changes */
								//g_print ("get offTags\n");
								tagList = gtk_text_iter_get_toggled_tags ( &iter, FALSE );
							}
							break;
						default:
							{
								g_print ( "got everything!\n" );
								tagList = gtk_text_iter_get_tags ( &iter );
							}
					}

				}

				else
				{
					/* get a list of all applicable tags */
					tagList = gtk_text_iter_get_tags ( &iter );
				}

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
	GtkTextView * view,
	GtkTextBuffer * buffer,
	Tcl_Interp * interp,
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
	GtkTextView * view,
	GtkTextBuffer * buffer,
	Tcl_Interp * interp,
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
\bugs       Recent changes to posToIter causing this function to crash.
            But, applyTag, which is hack of this function does not cause a crash!
\history    2008-06-27  Began implementation of new keywords for text position. See TODO.
**/
static int textInsert ( GtkTextBuffer * buffer, Tcl_Interp * interp, int objc, Tcl_Obj *  const objv[], int cmdNo )
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


	/* handle pango strings first */
	gint type = gnoclGetStringType ( objv[cmdNo+1] );

	if ( type == 48 )
	{

		gnoclInsertMarkup ( buffer, &iter, gnoclGetString ( objv[cmdNo+1] ) );

		//gtk_text_buffer_insert_markup ( buffer, &iter, gnoclGetString ( objv[cmdNo+1] ) );
		//gnoclMarkupInsertTest ( buffer, &iter );

	}

	else
	{
		gtk_text_buffer_insert ( buffer, &iter, gnoclGetString ( objv[cmdNo+1] ), -1 );
	}

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
\brief	** USING ** TEXTPARAMS
**/
static int configure ( Tcl_Interp *interp, TextParams *para, GnoclOption options[] )
{

	GtkScrolledWindow *scrolled = para->scrolled;
	GtkTextView *text = GTK_TEXT_VIEW ( gtk_bin_get_child ( GTK_BIN ( scrolled ) ) );
	GtkTextBuffer *buffer = gtk_text_view_get_buffer ( text );


	/****************************/

	gnoclAttachOptCmdAndVar (
		&options[onChangedIdx], &para->onChanged,
		&options[variableIdx], &para->textVariable,
		"changed", G_OBJECT ( buffer ),
		G_CALLBACK ( changedFunc ), interp, traceFunc, para );

	if ( options[variableIdx].status == GNOCL_STATUS_CHANGED && para->textVariable != NULL )
	{
		// if variable does not exist -> set it, else set widget state
		const char *val = Tcl_GetVar ( interp, para->textVariable, TCL_GLOBAL_ONLY );

		if ( val == NULL )
		{

			GtkTextIter *start, *end;

			gtk_text_buffer_get_bounds ( buffer, start, end );

			val = gtk_text_buffer_get_text ( buffer, start, end, 0 );

			setTextVariable ( para, val );
		}

		else
		{
			//setVal ( para->label, val );
		}
	}

	/*****************************/


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
\brief	** USING ** TEXTPARAMS
**/
static int configure_textView ( Tcl_Interp *interp, GtkTextView *text, GnoclOption options[] )
{

	//GtkScrolledWindow *scrolled = para->scrolled;
	//GtkTextView *text = GTK_TEXT_VIEW ( gtk_bin_get_child ( GTK_BIN ( scrolled ) ) );
	GtkTextBuffer *buffer = gtk_text_view_get_buffer ( text );


	/****************************/

	/*
		gnoclAttachOptCmdAndVar (
			&options[onChangedIdx], &para->onChanged,
			&options[variableIdx], &para->textVariable,
			"changed", G_OBJECT ( buffer ),
			G_CALLBACK ( changedFunc ), interp, traceFunc, para );

		if ( options[variableIdx].status == GNOCL_STATUS_CHANGED && para->textVariable != NULL )
		{
			// if variable does not exist -> set it, else set widget state
			const char *val = Tcl_GetVar ( interp, para->textVariable, TCL_GLOBAL_ONLY );

			if ( val == NULL )
			{

				GtkTextIter *start, *end;

				gtk_text_buffer_get_bounds ( buffer, start, end );

				val = gtk_text_buffer_get_text ( buffer, start, end, 0 );

				setTextVariable ( para, val );
			}

			else
			{
				//setVal ( para->label, val );
			}
		}
	*/
	/*****************************/


	if ( options[textIdx].status == GNOCL_STATUS_CHANGED )
	{
		//printf ( "INSERT SOME TEXT-b\n" );

		char *str = options[textIdx].val.str;
		gtk_text_buffer_set_text ( buffer, str, -1 );
	}

	/*
		if ( options[scrollBarIdx].status == GNOCL_STATUS_CHANGED )
		{
			GtkPolicyType hor, vert;

			if ( gnoclGetScrollbarPolicy ( interp, options[scrollBarIdx].val.obj, &hor, &vert ) != TCL_OK )
			{
				return TCL_ERROR;
			}

			gtk_scrolled_window_set_policy ( scrolled, hor, vert );
		}
	*/
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
static int cget ( Tcl_Interp * interp, GtkTextView * text, GnoclOption options[], int idx )
{
	/* get the option from the array? */
	Tcl_Obj *obj = NULL;


	if ( idx == tooltipIdx )
	{
		obj = Tcl_NewStringObj ( gtk_widget_get_tooltip_markup ( GTK_WIDGET ( text ) ), -1 );
	}

	if ( idx == baseFontIdx )
	{
		g_print ( "basefont\n" );
		PangoContext *context = gtk_widget_get_pango_context ( GTK_WIDGET ( text ) );
		PangoFontDescription *desc = pango_context_get_font_description ( context );
		char *font = pango_font_description_to_string  ( desc );

		obj = Tcl_NewStringObj ( font, -1 );


	}

	if ( idx == dataIdx )
	{
		obj = Tcl_NewStringObj ( g_object_get_data ( text, "gnocl::data" ), -1 );
	}

	if ( idx == baseColorIdx )
	{
		g_print ( "basecolor\n" );

		Tcl_Obj *ret = Tcl_NewListObj ( 0, interp );
		GdkColor color;

		//modifyWidgetGdkColor ( interp, opts, G_OBJECT(text), gtk_widget_modify_base, G_STRUCT_OFFSET ( GtkStyle, base ), resList );

		GtkStyle *style = gtk_rc_get_style ( GTK_WIDGET ( text ) );
		GdkColor *cp = ( GdkColor * ) G_STRUCT_MEMBER_P ( style, G_STRUCT_OFFSET ( GtkStyle, base ) );
		//GdkColor color = cp[type];

		Tcl_ListObjAppendElement ( interp, ret, Tcl_NewIntObj ( color.red ) );
		Tcl_ListObjAppendElement ( interp, ret, Tcl_NewIntObj ( color.green ) );
		Tcl_ListObjAppendElement ( interp, ret, Tcl_NewIntObj ( color.blue ) );

		Tcl_SetObjResult ( interp, obj );

		return TCL_OK;
	}


	if ( obj != NULL )
	{
		Tcl_SetObjResult ( interp, obj );
		return TCL_OK;
	}


	return gnoclCgetNotImplemented ( interp, options + idx );
}

/**
\brief	Return a list of all occuranances of a named tag within a buffer
**/
static void gnoclGetTagRanges ( Tcl_Interp *interp, GtkTextBuffer *buffer, gchar *tagName )
{
#ifdef DEBUG_TEXT
	g_print ( "%s %s\n", __FUNCTION__, tagName );
#endif

	GtkTextIter iter;
	GtkTextTag *tag;
	GtkTextTagTable *table;
	gint row;
	gint col;

	Tcl_Obj *res;
	res = Tcl_NewStringObj ( "", 0 );

	static char s1[300];
	static char s2[10];

	gtk_text_buffer_get_start_iter ( buffer, &iter );

	table = gtk_text_buffer_get_tag_table ( buffer );
	tag = gtk_text_tag_table_lookup ( table, tagName );

	/* check to see if tag applied at start of text */
	if ( gtk_text_iter_begins_tag ( &iter, tag ) == TRUE )
	{

		row = gtk_text_iter_get_line ( &iter );
		col = gtk_text_iter_get_line_offset ( &iter );

		sprintf ( s2, "%d %d ", row, col );
		strcat ( s1, s2 );
	}

	while ( ( gtk_text_iter_forward_to_tag_toggle ( &iter, tag ) ) == TRUE )
	{
		row = gtk_text_iter_get_line ( &iter );
		col = gtk_text_iter_get_line_offset ( &iter );

		sprintf ( s2, "%d %d ", row, col );
		strcat ( s1, s2 );

	}

	Tcl_AppendStringsToObj ( res, trim ( s1 ), ( char * ) NULL );
	Tcl_SetObjResult ( interp, res );

	sprintf ( s1, "" );
	sprintf ( s2, "" );

	return TCL_OK;
}


/**
\brief	Add tagname to a Tcl list.
**/
static void getTagName ( GtkTextTag * tag, gpointer data )
{
#ifdef DEBUG_TAGS
	g_print ( "%s %s\n", __FUNCTION__, tag->name );
#endif

	Tcl_Obj **resList = data;

	Tcl_ListObjAppendElement ( NULL, *resList, Tcl_NewStringObj ( tag->name, -1 ) );

}

/**
     ->   0: Ok
        1: delete chosen
        2: configure chosen
        3: scrollToPosition chosen
      < 0: ERROR
*/

/**
\brief      Return list of tag properties and their values.
\note       Only returns those attribute which have been set away from their default values.

            Getting a handle on the various tag parameters has been a process of trial and error.

            An alternative way of getting the colour values but this time in hex is:
            GdkColor *bg_color2  = &tag_appearance->bg_color;
            char *clr = gdk_color_to_string ( bg_color2 );
            sprintf ( tmp2, "clr = %s ",clr);
\arguments	TAG POINTER, ptr to Tcl list
\returns	TCL_LIST
**/
static void gnoclGetTagProperties ( GtkTextTag *tag, Tcl_Obj *resList )
{
#ifdef DEBUG_TAGS
	g_print ( "%s start\n", __FUNCTION__ );
#endif

	if ( tag == NULL )
	{
		return;
	}

	/*  modify the memory allocation here to use malloc and free to release it */

	static char tmp[300];

	GtkTextAppearance *tag_appearance; // use tag->values->appearance.underline etc.

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

	gchar **fontName;
	gchar **fontFamily;
	gchar **fontDesc;

#ifdef DEBUG_TAGS
	g_print ( "%s 1\n", __FUNCTION__ );
#endif

	/* priority */
	Tcl_ListObjAppendElement ( NULL, resList, Tcl_NewStringObj ( "-priority", -1 ) );
	//sprintf ( tmp, "%d", tag->priority );
	//Tcl_ListObjAppendElement ( NULL, resList, Tcl_NewStringObj ( tmp, -1 ) );
	Tcl_ListObjAppendElement ( NULL, resList, Tcl_NewIntObj ( tag->priority ) );

#ifdef DEBUG_TAGS
	g_print ( "%s priority\n", __FUNCTION__ );
#endif

	/* font */

#ifdef DEBUG_TAGS
	g_print ( "%s font start\n", __FUNCTION__ );
#endif

	/* these need to be concatenated into a meaningfull string */
	g_object_get ( G_OBJECT ( tag ), "font", &fontName, NULL );
	g_object_get ( G_OBJECT ( tag ), "family", &fontFamily, NULL );
	g_object_get ( G_OBJECT ( tag ), "font-desc", &fontDesc, NULL );

#ifdef DEBUG_TAGS
	g_print ( "%s font middle\n", __FUNCTION__ );

	g_print ( "family = %s\n", fontFamily );
	g_print ( "desc   = %s\n", fontDesc );

#endif

	if ( fontName != NULL )
	{
		Tcl_ListObjAppendElement ( NULL, resList, Tcl_NewStringObj ( "-font", -1 ) );

	}

	sprintf ( tmp, "%s", fontName  );
	Tcl_ListObjAppendElement ( NULL, resList, Tcl_NewStringObj ( tmp, -1 ) );

	/*
		Tcl_ListObjAppendElement ( NULL, resList, Tcl_NewStringObj ( "-fontFamily", -1 ) );
		sprintf ( tmp, "%s", fontFamily );
		Tcl_ListObjAppendElement ( NULL, resList, Tcl_NewStringObj ( tmp,  -1 ) );
	*/

#ifdef DEBUG_TAGS
	g_print ( "%s font end\n", __FUNCTION__ );
#endif

	/*  background stipple */
	if ( tag->bg_stipple_set )
	{
		GdkBitmap *bg_stipple  = tag_appearance->bg_stipple;
	}

#ifdef DEBUG_TAGS
	g_print ( "%s stipple\n", __FUNCTION__ );
#endif


	/*  background colour */
	if ( tag->pg_bg_color_set )
	{
		GdkColor bg_color  = tag_appearance->bg_color;
		Tcl_ListObjAppendElement ( NULL, resList, Tcl_NewStringObj ( "-background", -1 ) );
		sprintf ( tmp, "%d %d %d", bg_color.red, bg_color.green, bg_color.blue );
		Tcl_ListObjAppendElement ( NULL, resList, Tcl_NewStringObj ( tmp,  -1 ) );
	}

#ifdef DEBUG_TAGS
	g_print ( "%s backgroun\n", __FUNCTION__ );
#endif

	/*  foreground colour */
	if ( tag->fg_color_set )
	{
		GdkColor fg_color  = tag_appearance->fg_color;
		Tcl_ListObjAppendElement ( NULL, resList, Tcl_NewStringObj ( "-foreground", -1 ) );
		sprintf ( tmp, "%d %d %d", fg_color.red, fg_color.green, fg_color.blue );
		Tcl_ListObjAppendElement ( NULL, resList, Tcl_NewStringObj ( tmp, -1 ) );
	}

#ifdef DEBUG_TAGS
	g_print ( "%s foreground\n", __FUNCTION__ );
#endif

	/*  font scaling */
	if ( tag->scale_set )
	{
		Tcl_ListObjAppendElement ( NULL, resList, Tcl_NewStringObj ( "-scale", -1 ) );
		sprintf ( tmp, "%f", tag->values->font_scale );
		Tcl_ListObjAppendElement ( NULL, resList, Tcl_NewStringObj ( tmp, -1 ) );
	}

	/*  foreground stipple */
	if ( tag->fg_stipple_set )
	{
		GdkBitmap *fg_stipple = tag_appearance->fg_stipple;
	}

	/*  text justfification */
	if ( tag->justification_set )
	{

		Tcl_ListObjAppendElement ( NULL, resList, Tcl_NewStringObj ( "-justification", -1 ) );

		// GTK_JUSTIFY_LEFT | GTK_JUSTIFY_RIGHT | GTK_JUSTIFY_CENTER | GTK_JUSTIFY_FILL
		switch ( tag->values->justification )
		{
			case GTK_JUSTIFY_LEFT:
				{
					sprintf ( tmp, "left" );
				}
				break;
			case GTK_JUSTIFY_RIGHT:
				{
					sprintf ( tmp, "right" );
				}
				break;
			case GTK_JUSTIFY_CENTER:
				{
					sprintf ( tmp, "center" );
				}
				break;
			case GTK_JUSTIFY_FILL:
				{
					sprintf ( tmp, "fill" );
				}
				break;
			default: {}
		}

		Tcl_ListObjAppendElement ( NULL, resList, Tcl_NewStringObj ( tmp, -1 ) );
	}

	/*  default indent */
	if ( tag->indent_set )
	{
		Tcl_ListObjAppendElement ( NULL, resList, Tcl_NewStringObj ( "-indent", -1 ) );
		sprintf ( tmp, "%d", tag->values->indent );
		Tcl_ListObjAppendElement ( NULL, resList, Tcl_NewIntObj ( tag->values->indent ) );
	}

	/*  text rise, i.e. for superscript */
	if ( tag->rise_set )
	{
		Tcl_ListObjAppendElement ( NULL, resList, Tcl_NewStringObj ( "-rise", -1 ) );
		Tcl_ListObjAppendElement ( NULL, resList, Tcl_NewIntObj (  tag_appearance->rise ) );
	}

	/*  strikethrough or, overstrike */
	if ( tag->strikethrough_set )
	{
		Tcl_ListObjAppendElement ( NULL, resList, Tcl_NewStringObj ( "-strikethrough", -1 ) );
		Tcl_ListObjAppendElement ( NULL, resList, Tcl_NewIntObj ( tag->values->appearance.strikethrough ) );
	}

	/* right margin */
	if ( tag->right_margin_set )
	{
		Tcl_ListObjAppendElement ( NULL, resList, Tcl_NewStringObj ( "-rightMargin", -1 ) );
		Tcl_ListObjAppendElement ( NULL, resList, Tcl_NewIntObj (  tag->values->right_margin ) );
	}

	/* pixels above lines */
	if ( tag->pixels_above_lines_set )
	{
		Tcl_ListObjAppendElement ( NULL, resList, Tcl_NewStringObj ( "-pixelsAboveLines", -1 ) );
		Tcl_ListObjAppendElement ( NULL, resList, Tcl_NewIntObj (  tag->values->pixels_above_lines ) );
	}

	/* pixels below lines */
	if ( tag->pixels_below_lines_set )
	{
		Tcl_ListObjAppendElement ( NULL, resList, Tcl_NewStringObj ( "-pixelsBelowLines", -1 ) );
		Tcl_ListObjAppendElement ( NULL, resList, Tcl_NewIntObj (  tag->values->pixels_below_lines ) );
	}

	/* wrapmode? */
	/*
		if ( tag->pixels_inside_wrap_set )
		{
			Tcl_ListObjAppendElement ( NULL, resList, Tcl_NewStringObj ( "-wrapMode", -1 ) );
			Tcl_ListObjAppendElement ( NULL, resList, Tcl_NewIntObj( tag->values->pixels_inside_wrap ) );
		}
	*/
	if ( tag->tabs_set )
	{
		PangoTabArray *tabs = tag->values->tabs;
	}

	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	/* underline */
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	if ( tag->underline_set )
	{
		Tcl_ListObjAppendElement ( NULL, resList, Tcl_NewStringObj ( "-underline", -1 ) );

		//switch ( tag_appearance->underline )
		switch ( tag->values->appearance.underline )
		{
			case PANGO_UNDERLINE_NONE:
				{
					Tcl_ListObjAppendElement ( NULL, resList, Tcl_NewStringObj ( "none", -1 ) );
				} break;
			case PANGO_UNDERLINE_SINGLE:
				{
					Tcl_ListObjAppendElement ( NULL, resList, Tcl_NewStringObj ( "single", -1 ) );
				} break;
			case PANGO_UNDERLINE_DOUBLE:
				{
					Tcl_ListObjAppendElement ( NULL, resList, Tcl_NewStringObj ( "double", -1 ) );
				} break;
			case PANGO_UNDERLINE_LOW:
				{
					Tcl_ListObjAppendElement ( NULL, resList, Tcl_NewStringObj ( "low", -1 ) );
				} break;
			case PANGO_UNDERLINE_ERROR:
				{
					Tcl_ListObjAppendElement ( NULL, resList, Tcl_NewStringObj ( "error", -1 ) );
				} break;
			default:
				{
					Tcl_ListObjAppendElement ( NULL, resList, Tcl_NewStringObj ( "single", -1 ) );
				}
		}

	}

	/* wrapmode */
	if ( tag->wrap_mode_set )
	{

		Tcl_ListObjAppendElement ( NULL, resList, Tcl_NewStringObj ( "-wrapMode", -1 ) );

		switch ( tag->values->wrap_mode )
		{
			case GTK_WRAP_CHAR:
				{
					Tcl_ListObjAppendElement ( NULL, resList, Tcl_NewStringObj ( "char", -1 ) );
				} break;
			case GTK_WRAP_NONE:
				{
					Tcl_ListObjAppendElement ( NULL, resList, Tcl_NewStringObj ( "none", -1 ) );
				} break;
			case GTK_WRAP_WORD:
				{
					Tcl_ListObjAppendElement ( NULL, resList, Tcl_NewStringObj ( "word", -1 ) );
				}
			case GTK_WRAP_WORD_CHAR:
				{
					Tcl_ListObjAppendElement ( NULL, resList, Tcl_NewStringObj ( "word", -1 ) );
				}
				break;
			default: {}
		}


	}

	if ( tag->bg_full_height_set )
	{
		/*  Background is fit to full line height rather than
		 *  baseline +/- ascent/descent (font height)
		*/
		guint bg_full_height;

		//sprintf ( tmp, "bg_full_height %d", tag->values->pixels_inside_wrap );
		//Tcl_ListObjAppendElement ( NULL, resList, Tcl_NewStringObj ( tmp, -1 ) );
	}

	/* invisible */
	if ( tag->invisible_set )
	{
		Tcl_ListObjAppendElement ( NULL, resList, Tcl_NewStringObj ( "-invisible", -1 ) );
		Tcl_ListObjAppendElement ( NULL, resList, Tcl_NewIntObj (  tag->values->invisible ) );

	}

	/* editable */
	if ( tag->editable_set )
	{
		Tcl_ListObjAppendElement ( NULL, resList, Tcl_NewStringObj ( "-editable", -1 ) );
		Tcl_ListObjAppendElement ( NULL, resList, Tcl_NewIntObj (  tag->values->editable ) );
	}

	if ( tag->language_set )
	{
		PangoLanguage *language;
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

#ifdef DEBUG_TAGS
	g_print ( "%s end\n", __FUNCTION__ );
#endif
}


/**
\brief      Return list of tag name
**/
static void gnoclGetTagNames ( GtkTextTag * tag, gpointer data )
{
#ifdef DEBUG_TEXT
	//g_print ( "%s 1 \n", __FUNCTION__ );
#endif

	if ( tag == NULL )
	{
		return;
	}

	char **str = data;

	/*  modify the memory allocation here to use malloc and free to release it */

	static char tmp[300];

	sprintf ( tmp, "%s", tag->name );

	*str = tmp;

}


/**
\brief
**/
static int signalEmit ( Tcl_Interp * interp, Tcl_Obj * obj, int cmdNo, GtkTextBuffer * buffer )
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
int gnoclTextCommand ( GtkTextView *textView, Tcl_Interp * interp, int objc, Tcl_Obj *  const objv[], int cmdNo, int isTextWidget )
{
#ifdef DEBUG_TEXT
	g_print ( "gnoclTextCommand %s %s\n", Tcl_GetString ( objv[cmdNo] ), Tcl_GetString ( objv[cmdNo+1] ) );
#endif

	GtkTextBuffer  *buffer = gtk_text_view_get_buffer ( textView );

	const char *cmds[] =
	{
		"delete", "configure", "scrollToPosition", "scrollToMark",
		"parent",
		"getIndex", "getCoords", "getRect",
		"undo", "redo", "grabFocus", "resetUndo", "getPos",



		"set", "erase", "select", "get", "cut", "copy", "paste",
		"cget", "getLineCount", "getWordLength", "getLength",
		"getLineLength", "setCursor", "getCursor", "insert", "tag",
		"dump", "signalEmit", "mark", "gotoWordStart",
		"gotoWordEnd", "search", "class", "spawn", "replace",
		"serialize", "deSerialize", "save", "load", "print", "lorem",
		"clear", "popup", "getSelectionBounds",
		"hasGlobalFocus", "isToplevelFocus",
		NULL
	};

	enum cmdIdx
	{
		DeleteIdx, ConfigureIdx, ScrollToPosIdx, ScrollToMarkIdx,
		ParentIdx,
		GetIndexIdx, GetCoordsIdx, GetRectIdx,
		UndoIdx, RedoIdx, GrabFocusIdx, ResetUndoIdx, GetPosIdx,

		SetIdx, EraseIdx, SelectIdx, GetIdx, CutIdx, CopyIdx, PasteIdx,
		CgetIdx, GetLineCountIdx, GetWordLengthIdx, GetLengthIdx,
		GetLineLengthIdx, SetCursorIdx, GetCursorIdx, InsertIdx, TagIdx,
		DumpIdx, SignalEmitIdx, MarkIdx, GotoWordStartIdx,
		GotoWordEndIdx, SearchIdx, ClassIdx, SpawnIdx, ReplaceIdx,
		SerializeIdx, DeSerializeIdx, SaveIdx, LoadIdx, PrintIdx, LoremIdx,
		ClearIdx, PopupIdx, GetSelectionBounds,
		HasGlobalFocusIdx, IsToplevelFocusIdx
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

//	g_print ( "1\n" );

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
		case GrabFocusIdx:  	return 11;
		case ResetUndoIdx:  	return 12;
		case GetPosIdx: 		return 13;
		case HasGlobalFocusIdx:	return 14;
		case IsToplevelFocusIdx:	return 15;
			/* these are GtkTextBuffer operations */

		case PrintIdx:
			{
#ifdef DEBUG_TEXT
				g_print ( "print\n" );
#endif
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
#ifdef DEBUG_TEXT
				g_print ( "PopupIdx %s\n", Tcl_GetString ( objv[cmdNo] ) );
				g_print ( "PopupIdx %s\n", Tcl_GetString ( objv[cmdNo +1] ) );
				g_print ( "PopupIdx %s\n", Tcl_GetString ( objv[cmdNo +2] ) );
#endif

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
				guint8		*data;
				gsize		length;
				GtkTextIter	start, end;
				GdkAtom		se_format;

				se_format = gtk_text_buffer_register_serialize_tagset ( buffer, NULL );

				gtk_text_buffer_get_bounds ( buffer, &start, &end );
				data = gtk_text_buffer_serialize ( buffer, buffer, se_format, &start, &end, &length );

#ifdef DEBUG_TEXT
				g_print ( "%s\n", data );
#endif


				output = fopen ( Tcl_GetString ( objv[cmdNo+1] ), "w" );

				fwrite ( &length, sizeof ( gsize ), 1, output );
				fwrite ( data, sizeof ( guint8 ), length, output );
				fclose ( output );

#if 0 // attempt to serialize returning a text string
				GString *buf;
				buf = g_string_sized_new ( length );

				int i;

				output = fopen ( "serialize.xml", "w" );

				/* ignore the first 32 characters */
				for ( i = 31; i < length ; i++ )
				{
					/* show the content of the data array */
					g_string_append_unichar ( buf, data[i] ); /* <--- problems here, but why? */
				}

				fclose ( output );

				Tcl_SetObjResult ( interp, Tcl_NewStringObj ( buf->str, -1 ) );

				g_string_free ( buf, 1 );

#endif

				return TCL_OK;

			}

			break;
		case LoadIdx:
		case DeSerializeIdx:  /* WJG 29/04/09 */
			{

				FILE *input;
				guint8 *data;
				gsize length;
				GtkTextIter iter;
				GdkAtom de_format;

#if 1 	// binary file
				input = fopen ( Tcl_GetString ( objv[cmdNo+1] ), "r" );

				// Return with error message if the file is not found.

				if ( input == NULL )
				{
					char str[128];
					sprintf ( str, "File %s not found.\n", Tcl_GetString ( objv[cmdNo+1] ) );
					Tcl_SetObjResult ( interp, Tcl_NewStringObj ( str, -1 ) );
					return TCL_OK;
				}

				// read data
				fread ( &length, sizeof ( gsize ), 1, input );
				data = malloc ( sizeof ( guint8 ) * length );
				fread ( data, sizeof ( guint8 ), length, input );
				fclose ( input );


				de_format = gtk_text_buffer_register_deserialize_tagset ( buffer, "default" );
				gtk_text_buffer_get_iter_at_offset ( buffer, &iter, 0 );
				gtk_text_buffer_deserialize ( buffer, buffer, de_format, &iter, data, length, NULL );

				g_free ( data );

#else  	// text file
				de_format = gtk_text_buffer_register_deserialize_tagset ( buffer, "default" );
				gtk_text_buffer_get_iter_at_offset ( buffer, &iter, 0 );
				gtk_text_buffer_deserialize ( buffer, buffer, de_format, &iter, Tcl_GetString ( objv[cmdNo+1] ), strlen ( Tcl_GetString ( objv[cmdNo+1] ) ), NULL );
#endif

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
				GtkTextBuffer *spawnBuffer;
				spawnBuffer = buffer;
#ifdef DEBUG_TEXT
				g_print ( "spawn -1\n" );
#endif
				/*  create a new text view with buffer GtkTextBuffer *buffer*/
				//g_object_ref (G_OBJECT(buffer));
				spawn = GTK_TEXT_VIEW ( gtk_text_view_new_with_buffer ( GTK_TEXT_BUFFER ( spawnBuffer ) ) );
				//spawn = GTK_TEXT_VIEW ( gtk_text_view_new_with_buffer ( NULL ) );
				//gtk_text_view_set_buffer (spawn, buffer);
#ifdef DEBUG_TEXT
				g_print ( "spawn -2\n" );
#endif
				/*  add some extra signals to the default setting */
				gtk_widget_add_events ( spawn, GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK );
				g_print ( "spawn -3\n" );
				scrolled =  gtk_scrolled_window_new ( NULL, NULL );
				gtk_scrolled_window_set_policy ( scrolled, GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC );
				gtk_container_add ( scrolled, GTK_WIDGET ( spawn ) );
				gtk_widget_show_all ( GTK_WIDGET ( scrolled ) );
#ifdef DEBUG_TEXT
				g_print ( "spawn -4\n" );
#endif

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
				gint get_markup = 0;

#ifdef DEBUG_TEXT
				g_print ( "2\n" );
#endif

				if ( objc == 6 )
				{
					if ( strcmp ( Tcl_GetString ( objv[cmdNo+4] ), "1" ) == 0 )
					{
						get_markup = 1;
						objc += 2;
					}
				}

#ifdef DEBUG_TEXT
				g_print ( "3\n" );
#endif

				if ( objc < cmdNo + 3 )
				{
					Tcl_WrongNumArgs ( interp, cmdNo + 2, objv, "{row col} -options" );
					return -1;
				}

#ifdef DEBUG_TEXT
				g_print ( "4\n" );
#endif
				/* get attributes */

				if ( strcmp ( Tcl_GetString ( objv[cmdNo+1] ), "attributes" ) == 0 )
				{

#ifdef DEBUG_TEXT
					g_print ( "attributes at %s\n", Tcl_GetString ( objv[cmdNo+2] ) );
#endif
					GtkTextIter iter;
					GtkTextAttributes values;

					if ( posToIter ( interp, objv[cmdNo+2], buffer, &iter ) != TCL_OK )
					{
						return TCL_ERROR;
					}

					if ( gtk_text_iter_get_attributes ( &iter, &values ) )
					{
#ifdef DEBUG_TEXT
						g_print ( "attributes at %s\n", Tcl_GetString ( objv[cmdNo+2] ) );
#endif
						/*
						GtkJustification justification;
						GtkTextDirection direction;
						PangoFontDescription *font;
						gdouble font_scale;
						gint left_margin;
						gint indent;
						gint right_margin;
						gint pixels_above_lines;
						gint pixels_below_lines;
						gint pixels_inside_wrap;
						PangoTabArray *tabs;
						GtkWrapMode wrap_mode;
						PangoLanguage *language;
						guint invisible : 1;
						guint bg_full_height : 1;
						guint editable : 1;
						guint realized : 1;
						*/

						getAttributes ( interp, &values );

					}

					return TCL_OK;
				}


				if ( strcmp ( Tcl_GetString ( objv[cmdNo+1] ), "markup" ) == 0 )
				{

#ifdef DEBUG_TEXT
					g_print ( "markup from %s to %s\n", Tcl_GetString ( objv[cmdNo+2] ), Tcl_GetString ( objv[cmdNo+3] ) );
					g_print ( "=====get as markup string\n" );
#endif

					posToIter ( interp, objv[cmdNo+2], buffer, &startIter );
					posToIter ( interp, objv[cmdNo+3], buffer, &endIter );

					Tcl_Obj *res = getMarkUpString ( interp, buffer, &startIter, &endIter );

					//char *txt = gtk_text_buffer_get_text ( buffer, &startIter, &endIter, 1 );
					Tcl_SetObjResult ( interp, res );
					return TCL_OK;


				}

#ifdef DEBUG_TEXT
				g_print ( "6\n" );
#endif

				if ( objc < cmdNo + 3 )
				{
					Tcl_WrongNumArgs ( interp, cmdNo + 1, objv, "startIndex ?endIndex?" );
					return TCL_ERROR;
				}

#ifdef DEBUG_TEXT
				g_print ( "7\n" );
#endif

				if ( posToIter ( interp, objv[cmdNo+1], buffer, &startIter ) != TCL_OK )
				{
					return TCL_ERROR;
				}

#ifdef DEBUG_TEXT
				g_print ( "8\n" );
#endif

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

#ifdef DEBUG_TEXT
				g_print ( "9\n" );
#endif

				switch ( idx )
				{
					case EraseIdx:
						{
							gtk_text_buffer_delete ( buffer, &startIter, &endIter );
						}
						break;
					case SelectIdx:
						{
							gtk_text_buffer_place_cursor ( buffer, &startIter );
							gtk_text_buffer_move_mark_by_name ( buffer, "selection_bound", &endIter );
						}
						break;
					case GetIdx:
						{
							/*  TODO: include_hidden_chars */
#ifdef DEBUG_TEXT
							g_print ( "10 ----\n" );
#endif

							if ( get_markup )
							{
#ifdef DEBUG_TEXT
								g_print ( "-----get as markup string\n" );
#endif
								Tcl_Obj *res = getMarkUpString ( interp, buffer, &startIter, &endIter );

								//char *txt = gtk_text_buffer_get_text ( buffer, &startIter, &endIter, 1 );
								Tcl_SetObjResult ( interp, res );
							}

							else
							{
								char *txt = gtk_text_buffer_get_text ( buffer, &startIter, &endIter, 1 );
								Tcl_SetObjResult ( interp, Tcl_NewStringObj ( txt, -1 ) );
							}
						}
						break;
				}
			}
#ifdef DEBUG_TEXT
			g_print ( "11\n" );
#endif
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
						{
							gtk_text_buffer_cut_clipboard ( buffer, clipboard, 1 );
						}
						break;
					case CopyIdx:
						{
							gtk_text_buffer_copy_clipboard ( buffer, clipboard );
						}
						break;
					case PasteIdx:
						{
							gtk_text_buffer_paste_clipboard ( buffer, clipboard, NULL, 1 );
						}
						break;
				}
			}

			break;
		case CgetIdx:	// gnoclTextCommand
			{
#ifdef DEBUG_TEXT
				g_print ( "%s CgetIdx\n", __FUNCTION__ );
#endif


				int     idx;

				switch ( gnoclCget ( interp, objc, objv, G_OBJECT ( textView ), textOptions, &idx ) )
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
							return cget ( interp, textView, textOptions, idx );
						}
				}

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

		case GetSelectionBounds:
			{
				GtkTextIter startIter, endIter;

				int           row, col;
				Tcl_Obj       *resList;

				if ( gtk_text_buffer_get_selection_bounds ( buffer, &startIter, &endIter ) )
				{

					resList = Tcl_NewListObj ( 0, NULL );

					row = gtk_text_iter_get_line ( &startIter );
					col = gtk_text_iter_get_line_offset ( &startIter );

					Tcl_ListObjAppendElement ( interp, resList, Tcl_NewIntObj ( row ) );
					Tcl_ListObjAppendElement ( interp, resList, Tcl_NewIntObj ( col ) );

					row = gtk_text_iter_get_line ( &endIter );
					col = gtk_text_iter_get_line_offset ( &endIter );

					Tcl_ListObjAppendElement ( interp, resList, Tcl_NewIntObj ( row ) );
					Tcl_ListObjAppendElement ( interp, resList, Tcl_NewIntObj ( col ) );

					Tcl_SetObjResult ( interp, resList );
					return TCL_OK;

				}

				/* return null string if there is no selection */
				Tcl_SetObjResult ( interp, Tcl_NewStringObj ( "", -1  ) );
				return TCL_OK;


			}
			break;


		case InsertIdx:
			{

				if ( textInsert ( buffer, interp, objc, objv, cmdNo + 1 ) != TCL_OK )
				{
					return -1;
				}
			}
			/*  this action needs to emit an insert text signal */

			break;

		case MarkIdx:
			{
				if ( markCmd ( buffer, interp, objc, objv, cmdNo + 1 ) != TCL_OK )
				{
					return -1;
				}
			}
			break;

		case TagIdx:
			{
				if ( tagCmd ( buffer, interp, objc, objv, cmdNo + 1 ) != TCL_OK )
				{
					return -1;
				}
			}
			break;

			/*  the widget dump has the format;
			 *  widgetId dump option startIndex endIndx
			 */
		case DumpIdx:
			{
				/*  some DEBUG_TEXTging feedback */

#ifdef DEBUG_TAGS
				g_print ( "\tdump: %d %d: %s %s %s %s\n",
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

				static const char *cmds[] =
				{
					"all", "tags", "text",
					"window", "images", "marks",
					NULL
				};

				enum opts
				{
					AllIdx, TagsIdx, TextIdx,
					WindowsIdx, ImagesIdx, MarksIdx
				};

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

#ifdef DEBUG_TAGS
				g_print ( "-----HERE\n" );
#endif

//getIdx ( cmds, objv[cmdNo+1], &idx );


				/*
				 *  Respond to the second keyword..
				 */

				switch ( idx )
				{
					case AllIdx:
						{
							Tcl_Obj *res;
							res = dumpAll ( interp,  buffer, &startIter, &endIter ) ;
							Tcl_SetObjResult ( interp, res );
							return TCL_OK;
						}

						break;
					case TagsIdx:	/* include tagOn/Off changes in the text */
						{
#ifdef DEBUG_TEXT
							g_print ( "dump text+tags start\n" );
#endif
							GtkTextTagTable *tagtable = gtk_text_buffer_get_tag_table ( buffer );

							/*  get the settings of each tag in the buffer*/
							/*  note, pass the address of the pointer to the data assigned by the called function */

							gtk_text_tag_table_foreach ( tagtable, gnoclGetTagProperties, &txt ); //

#ifdef DEBUG_TEXT
							g_print ( "dump text+tags end\n" );
#endif

							//Tcl_SetObjResult ( interp, Tcl_NewStringObj ( txt, -1 ) );
							/*  reset the outsput string by using null pointers */
							//gnoclGetTagProperties ( NULL, NULL );
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

			}

			break;

		default:
			assert ( 0 );
			return -1;
	}

	return 0;
}



/**
\brief      ** USING ** TEXTPARAMS
			Handler for gnocl created instances that take into account the scrolled window
\note       Unlike gnocl, Builder/Glade  not provide text objects within scrolled windows.
            Two handle functions are necessary, one for gnocl built widgets, and one for builder/glade widgets.
**/
static int textFunc ( ClientData data, Tcl_Interp * interp, int objc, Tcl_Obj *  const objv[] )
{
	TextParams *para = ( TextParams * ) data;

	GtkScrolledWindow   *scrolled = para->scrolled;
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

	/*
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
		case GrabFocusIdx:  	return 11;
		case ResetUndoIdx:  	return 12;
		case GetPosIdx: 		return 13;
		case HasFocus			return 14;
	*/

	switch ( gnoclTextCommand ( text, interp, objc, objv, 1, 1 ) )
	{
			/*  these are command which work upon the GtkTextView rather than the GtkTextBuffer */
		case 0: /*  return TCL_OK */
			{
				break;
			}
		case 1:  /*  delete */
			{
				return gnoclDelete ( interp, GTK_WIDGET ( scrolled ), objc, objv );
			}
		case 2:     /*  configure */
			{
				int ret = TCL_ERROR;

				if ( gnoclParseAndSetOptions ( interp, objc - 1, objv + 1, textOptions, G_OBJECT ( text ) ) == TCL_OK )
				{
					//ret = configure ( interp, scrolled, text, para, textOptions );
					ret = configure ( interp, para, textOptions );
				}

				gnoclClearOptions ( textOptions );

				return ret;
			}

			break;
		case 3: /*  scrollToPosition */
			{
				return scrollToPos ( text, buffer, interp, objc, objv );
			}
		case 4: /*  scrollToMark */
			{
				return scrollToMark ( text, buffer, interp, objc, objv );
			}
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
			// getIndex
		case 6: /* get line/row from root window coordinates, ie passed from an event */
			{

				g_print ( "tag getIndex\n" );

				GtkTextIter iter;
				gint y, line_no;
				gint x, row_no;

				gint wx, wy; /* window coordinates */
				gint bx, by; /* buffer coordinates */
				gint line, row;

				//sscanf ( Tcl_GetString ( objv[2] ), "%d %d", &wx, &wy );

				Tcl_GetIntFromObj ( NULL, objv[2], &wx ) ;
				Tcl_GetIntFromObj ( NULL, objv[3], &wy ) ;

				g_print ( "1\n" );

				//gdk_window_get_pointer (TxT->window, &wx, &wy, NULL);
				gtk_text_view_window_to_buffer_coords ( text, GTK_TEXT_WINDOW_WIDGET, wx, wy, &bx, &by );
				gtk_text_view_get_iter_at_location ( text, &iter, bx, by );

				//gtk_text_layout_get_iter_at_pixel (text->layout, &iter, x, y);

				g_print ( "2\n" );


				line = gtk_text_iter_get_line ( &iter );
				row = gtk_text_iter_get_line_offset ( &iter );

				g_print ( "3\n" );

				gchar str[16];
				sprintf ( str, "%d %d", line, row );

				g_print ( "4\n" );

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
		case 11: /* grab keyboard input */
			{
				gtk_widget_grab_focus ( text );
				return TCL_OK;
			}
		case 12: /* reset Undo/Redo buffer */
			{

				gtk_undo_view_reset ( text );

				return TCL_OK;
			}

		case 13: /* get iter at pixel */
			{

				//GtkTextIter iter;
				//gtk_text_layout_get_iter_at_pixel (text->layout, &iter, x, y);
				return TCL_OK;
			}

		case 14: /* does the widget have focus */
			{

				//GtkTextIter iter;
				//gtk_text_layout_get_iter_at_pixel (text->layout, &iter, x, y);
				Tcl_SetObjResult ( interp, Tcl_NewIntObj ( gtk_widget_has_focus ( text ) ) );
				return TCL_OK;
			}

		case 15: /* does the widget have focus */
			{

				//GtkTextIter iter;
				//gtk_text_layout_get_iter_at_pixel (text->layout, &iter, x, y);
				Tcl_SetObjResult ( interp, Tcl_NewIntObj ( gtk_widget_is_focus ( text ) ) );
				return TCL_OK;
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
int textViewFunc ( ClientData data, Tcl_Interp * interp, int objc, Tcl_Obj *  const objv[] )
{

	GtkTextView    *text = GTK_TEXT_VIEW ( data );
	GtkTextBuffer  *buffer = gtk_text_view_get_buffer ( text );

	if ( objc < 2 )
	{
		Tcl_WrongNumArgs ( interp, 1, objv, "command" );
		return TCL_ERROR;
	}

	switch ( gnoclTextCommand ( text, interp, objc, objv, 1, 1 ) )
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
		case 6: /* grab keyboard input */
			{
				gtk_widget_grab_focus ( text );
				return TCL_OK;
			}
			break;
		default:
			{
				return TCL_ERROR;
			}
	}

	return TCL_OK;
}


/**
\brief	Create a fully developed text megawidget.
\note	** USING ** TEXTPARAMS
**/
int gnoclTextCmd ( ClientData data, Tcl_Interp * interp, int objc, Tcl_Obj *  const objv[] )
{

	TextParams *para;

	int               ret, k;
	GtkTextView       *textView;
	GtkTextView       *textBuffer;
	GtkScrolledWindow *scrolled;

	para = g_new ( TextParams, 1 );

	para->interp = interp;
	para->textVariable = NULL;
	para->onChanged = NULL;
	para->inSetVar = 0;
	para->useMarkup = FALSE;


	if ( gnoclParseOptions ( interp, objc, objv, textOptions ) != TCL_OK )
	{
		gnoclClearOptions ( textOptions );
		return TCL_ERROR;
	}

	//textView = GTK_TEXT_VIEW ( gtk_text_view_new( ) );

	// implement new undo/redo buffer
	textView = gtk_undo_view_new ( gtk_text_buffer_new  ( NULL ) );

	para->scrolled =  GTK_SCROLLED_WINDOW ( gtk_scrolled_window_new ( NULL, NULL ) );

	gtk_scrolled_window_set_policy ( para->scrolled, GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC );

	gtk_container_add ( GTK_CONTAINER ( para->scrolled ), GTK_WIDGET ( textView ) );

	gtk_widget_show_all ( GTK_WIDGET ( para->scrolled ) );

	//add some extra signals to the default setting -these have no effect!!!
	gtk_widget_add_events ( textView, GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK );

	ret = gnoclSetOptions ( interp, textOptions, G_OBJECT ( textView ), -1 );


	if ( ret == TCL_OK )
	{
		ret = configure ( interp, para, textOptions );
	}

	gnoclClearOptions ( textOptions );

	if ( ret != TCL_OK )
	{
		gtk_widget_destroy ( GTK_WIDGET ( para->scrolled ) );
		return TCL_ERROR;
	}

	para->name = gnoclGetAutoWidgetId();

	g_signal_connect ( G_OBJECT ( para->scrolled ), "destroy", G_CALLBACK ( destroyFunc ), para );

	gnoclMemNameAndWidget ( para->name, GTK_WIDGET ( para->scrolled ) );

	Tcl_CreateObjCommand ( interp, para->name, textFunc, para, NULL );

	Tcl_SetObjResult ( interp, Tcl_NewStringObj ( para->name, -1 ) );

	return TCL_OK;

}


/**
\brief	Create a plain vanilla GtkTextView widget
\todo	Create modified version of the configure command
**/
int gnoclTextViewCmd ( ClientData data, Tcl_Interp * interp, int objc, Tcl_Obj *  const objv[] )
{

	TextParams *para;

	int               ret, k;
	GtkTextView       *textView;
	GtkTextView       *textBuffer;


	if ( gnoclParseOptions ( interp, objc, objv, textOptions ) != TCL_OK )
	{
		gnoclClearOptions ( textOptions );
		return TCL_ERROR;
	}

	// implement new undo/redo buffer
	textView = gtk_undo_view_new ( gtk_text_buffer_new  ( NULL ) );


	gtk_widget_show_all ( GTK_WIDGET ( textView ) );

	//add some extra signals to the default setting -these have no effect!!!
	gtk_widget_add_events ( textView, GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK );

	ret = gnoclSetOptions ( interp, textOptions, G_OBJECT ( textView ), -1 );


	if ( ret == TCL_OK )
	{
		ret = configure_textView ( interp, textView, textOptions );
	}

	gnoclClearOptions ( textOptions );

	if ( ret != TCL_OK )
	{
		gtk_widget_destroy ( GTK_WIDGET ( textView ) );
		return TCL_ERROR;
	}

	return gnoclRegisterWidget ( interp, textView, textViewFunc );

}
