/*
 * $Id: entry.c,v 1.11 2005/01/01 15:27:54 baum Exp $
 *
 * This file implements the entry widget
 *
 * Copyright (c) 2001 - 2005 Peter G. Baum  http://www.dr-baum.net
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

/*
   History:
   2011-07-03: cget -data now works properly
   2011-06-30: added options
   					-dropTargets, -dragTargets, -onDropData, -onDragData
   2011-04-03: added options
   					-active -buffer -capsLockWarning -cursorPos -hasFrame
					-inputMethod -innerBorder -invisibleChar -setInvisibleChar -overwriteMode
					-activatePrimaryIcon -pulseStep -scrollOffset
					-activateSecondaryIcon -selectionBound -shadow -text -textLength -truncate
					-primaryIconSensitive -secondaryIconSensitive
				added commands
					pulse
					progress <val>
   2011-04-02: added options -primaryIcon -secondaryIcon -onIconPress -onIconRelease
   2010-05-25: added command wordList
   2009-06-13: added -align
   2009-04-27: added command setPosition
   2009-04-26: added -onInsertText -onKeyPress -onKeyRelease
   2009-03-06: added commands clear, get and set
   2008-11: added -baseFont
   2008-10: added command, class
   2004-02: added -data
        09: added cget
            added -widthChars
            removed getValue
   2003-01: fixed uninitialized variables
        12: added -value and -onChanged
        08: switched from GnoclWidgetOptions to GnoclOption
            gtk+ 2.0 related cleanups
   2002-04: updates for gtk 2.0
        09: added GnomeEntry, GnomeFileEntry, GnomeNumberEntry,
            GnomePixmapEntry and GnomeIconEntry
   2001-03: Begin of developement
*/

/**
\page page_entry gnocl::entry
\htmlinclude entry.html
**/

#include "gnocl.h"
#include "gnoclparams.h"

#include <string.h>
#include <assert.h>

/* function declarations */
static int gnoclOptCompletion ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret );
static int gnoclOptOnActivate ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret );

/**
\brief
*/
static const int variableIdx      = 0;
static const int onChangedIdx     = 1;
static const int valueIdx         = 2;
static const int cursorIdx        = 3;
static const int primaryIconIdx   = 4;
static const int secondaryIconIdx = 5;
static const int cursorPosIdx     = 6;
static const int dataIdx          = 7;


static GnoclOption entryOptions[] =
{
	/* gnocl unique and complex options handled through configure */
	{ "-variable", GNOCL_STRING, NULL },      /* 0 */
	{ "-onChanged", GNOCL_STRING, NULL },     /* 1 */
	{ "-value", GNOCL_STRING, NULL },         /* 2 */
	{ "-showCursor", GNOCL_BOOL, NULL},       /* 3 */
	{ "-primaryIcon", GNOCL_OBJ, NULL },      /* 4 */
	{ "-secondaryIcon", GNOCL_OBJ, NULL },    /* 5 */
	{ "-cursorPos", GNOCL_STRING, NULL }, /* 6 */


	{ "-data", GNOCL_OBJ, "", gnoclOptData }, /* 7 */


	{ "-name", GNOCL_STRING, "name" },

	{ "-hasFocus", GNOCL_BOOL, "has-focus" },
	{ "-heightGroup", GNOCL_OBJ, "h", gnoclOptSizeGroup },

	{ "-onRealize", GNOCL_OBJ, "realize", gnoclOptCommand },
	{ "-onShowHelp", GNOCL_OBJ, "", gnoclOptOnShowHelp },
	{ "-sensitive", GNOCL_BOOL, "sensitive" },
	{ "-sizeGroup", GNOCL_OBJ, "s", gnoclOptSizeGroup },

	{ "-tooltip", GNOCL_OBJ, "", gnoclOptTooltip },
	{ "-visible", GNOCL_BOOL, "visible" },

	{ "-widthGroup", GNOCL_OBJ, "w", gnoclOptSizeGroup },

	{ "-baseFont", GNOCL_OBJ, "Sans 14", gnoclOptGdkBaseFont },
	{ "-baseColor", GNOCL_OBJ, "normal", gnoclOptGdkColorBase },

	/* other entry properties Gtk+ 2.24 */
	{ "-activate", GNOCL_BOOL, "activates-default" },
	{ "-buffer", GNOCL_OBJ, "", NULL }, 							/* to be implemented */
	{ "-capsLockWarning", GNOCL_BOOL, "caps-lock-warning" },
	{ "-editable", GNOCL_BOOL, "editable" },
	{ "-activate", GNOCL_BOOL, "activates-default" },
	{ "-hasFrame", GNOCL_BOOL, "has-frame" },
	{ "-inputMethod", GNOCL_OBJ, "im-module", NULL }, 				/* to be implemented */
	{ "-innerBorder", GNOCL_OBJ, "inner-border", NULL }, 			/* to be implemented */
	{ "-invisibleChar", GNOCL_INT, "invisible-char"}, 				/* to be implemented */
	{ "-setInvisibleChar", GNOCL_BOOL,  "invisible-char-set" },
	{ "-maxLength", GNOCL_INT, "max-length" },
	{ "-overwriteMode", GNOCL_BOOL, "overwrite-mode" },
	{ "-activatePrimaryIcon", GNOCL_BOOL, "primary-icon-activatable" },
	{ "-primaryIconSensitive", GNOCL_BOOL, "primary-icon-sensitive" },
	//{ "-progressFraction", GNOCL_DOUBLE, "progress-fraction" },
	{ "-pulseStep", GNOCL_DOUBLE, "progress-pulse-step" },
	{ "-scrollOffset", GNOCL_INT, "scroll-offset" },
	{ "-activateSecondaryIcon", GNOCL_BOOL, "secondary-icon-activatable" },
	{ "-secondaryIconSensitive", GNOCL_BOOL, "secondary-icon-sensitive" },
	{ "-selectionBound", GNOCL_INT, "selection-bound" },
	{ "-shadow", GNOCL_OBJ, "shadow-type", gnoclOptShadow },
	{ "-text", GNOCL_STRING, "text"},
	{ "-textLength", GNOCL_INT, "text-length"},
	{ "-truncate", GNOCL_BOOL, "truncate-multiline"},
	{ "-textVisible", GNOCL_BOOL, "visibility" },
	{ "-widthChars", GNOCL_INT, "width-chars" },
	{ "-align", GNOCL_DOUBLE, "xalign" },

	{ "-primaryIconTooltip", GNOCL_OBJ, "P", gnoclOptIconTooltip },
	{ "-secondaryIconTooltip", GNOCL_OBJ, "S", gnoclOptIconTooltip },

	{ "-completion", GNOCL_OBJ, "", gnoclOptCompletion },

	/* GtkEntry Signals upto Gtk+ 2.24*/
	/* use the following for -onInsert for validation testing, implemented in Tcl */
	{ "-onActivate", GNOCL_OBJ, "activate", gnoclOptOnActivate },
	{ "-onBackSpace", GNOCL_OBJ, "backspace", gnoclOptOnBackspace },
	{ "-onCopy", GNOCL_OBJ, "C", gnoclOptOnClipboard },
	{ "-onCut", GNOCL_OBJ, "X", gnoclOptOnClipboard },
	{ "-onDel", GNOCL_OBJ, "delete-from-cursor", gnoclOptOnDeleteFromCursor},
	{ "-onIconPress", GNOCL_OBJ, "P", gnoclOptOnIconPress },
	{ "-onIconRelease", GNOCL_OBJ, "R", gnoclOptOnIconPress },
	{ "-onInsert", GNOCL_OBJ, "", gnoclOptOnInsertAtCursor},
	{ "-onCursor", GNOCL_OBJ, "move-cursor", gnoclOptOnMoveCursor},
	{ "-onPaste", GNOCL_OBJ, "V", gnoclOptOnClipboard},
	{ "-onPopulatePopup", GNOCL_OBJ, "populate-popup", gnoclOptOnPopulatePopup},
	{ "-onOverwrite", GNOCL_OBJ, "toggle-overwrite", gnoclOptOnToggleOverwrite},
	{ "-onPreeditChanged", GNOCL_OBJ, "preedit-changed", gnoclOptOnPreEditChanged },

	/* GtkEditable Signals */
	{ "-onKeyPress", GNOCL_OBJ, "", gnoclOptOnKeyPress },
	{ "-onKeyRelease", GNOCL_OBJ, "", gnoclOptOnKeyRelease },

	{ "-onFocusIn", GNOCL_OBJ, "I", gnoclOptOnFocus },
	{ "-onFocusOut", GNOCL_OBJ, "O", gnoclOptOnFocus },

	/* drag-n-drop */
	{ "-dropTargets", GNOCL_LIST, "t", gnoclOptDnDTargets },
	{ "-dragTargets", GNOCL_LIST, "s", gnoclOptDnDTargets },
	{ "-onDropData", GNOCL_OBJ, "", gnoclOptOnDropData },
	{ "-onDragData", GNOCL_OBJ, "", gnoclOptOnDragData },

	{ NULL }
};

/**
\brief
*/

/* moved to gnocl.h */
/*
typedef struct
{
    GtkEntry    *entry;
    Tcl_Interp  *interp;
    char        *name;
    char        *variable;
    char        *onChanged;
    int         inSetVar;
} EntryParams;
*/

/* function declarations */
static int setVal ( GtkEntry *entry, const char *txt );
static void changedFunc ( GtkWidget *widget, gpointer data );


/**
\brief     Handles the 'activate' signal.
\author    William J Giddings
\date      25/04/2010
\since     0.9.95
\note      Used by: gnome::entry
\**/
static void doOnActivate (   GtkEntry *entry, gpointer user_data )
{


	GnoclCommandData *cs = ( GnoclCommandData * ) user_data;
	GtkTextIter *iter;

	GnoclPercSubst ps[] =
	{
		{ 'w', GNOCL_STRING },  /* widget name */
		{ 't', GNOCL_STRING },  /* text content */
		{ 0 }
	};

	ps[0].val.str = gnoclGetNameFromWidget ( entry );;
	ps[1].val.str = gtk_entry_get_text ( entry );

	gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 );
}

/**
\brief    Implements the 'activate' signal.
\author   William J Giddings
\date     25/04/2010
\since    0.9.95
**/
static int gnoclOptOnActivate ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{

	/* check the name of the signal is correct for this function */
	assert ( strcmp ( opt->optName, "-onActivate" ) == 0 );

	/* connect the signal with its callback function */
	return gnoclConnectOptCmd ( interp, obj, "activate", G_CALLBACK ( doOnActivate ), opt, NULL, ret );
}


/**
\brief
**/
//GtkEntryCompletion *create_completion(void)
int gnoclOptCompletion ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	assert ( strcmp ( opt->optName, "-completion" ) == 0 );

	GtkEntryCompletion *completion;
	GtkListStore *model;
	GtkTreeIter iter;

	const gchar *text[] =
	{
		"how", "now", "brown", "cow",
		"she", "sells", "sea", "shells", "by", "the", "shore",
		NULL,
	};
	gint i;

	/* Create a new completion... */
	completion = gtk_entry_completion_new();

	/* ...and set things up for the first column */
	gtk_entry_completion_set_text_column ( completion, 0 );

	/* Create a list store with one string column... */
	model = gtk_list_store_new ( 1, G_TYPE_STRING );

	/* ...and set it as the completion's model... */
	gtk_entry_completion_set_model ( completion, GTK_TREE_MODEL ( model ) );

	/* ...and drop our reference to the model */
	g_object_unref ( model );

	/* Add some text to be matched */
	for ( i = 0; text[i] != NULL; i++ )
	{
		gtk_list_store_append ( model, &iter );
		gtk_list_store_set ( model, &iter, 0, text[i], -1 );
	}

	gtk_entry_set_completion ( GTK_ENTRY ( obj ), completion );

	return TCL_OK;
}


/**
\brief
**/
static int doCommand ( EntryParams *para, const char *val, int background )
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

/**
\brief
**/
static int setVal ( GtkEntry *entry, const char *txt )
{
#ifdef DEBUG_ENTRY
	printf ( "entry/staticFuncs/setVal\n" );
#endif

	int blocked = g_signal_handlers_block_matched (
					  G_OBJECT ( entry ), G_SIGNAL_MATCH_FUNC,
					  0, 0, NULL, ( gpointer * ) changedFunc, NULL );
	gtk_entry_set_text ( entry, txt );

	if ( blocked )
		g_signal_handlers_unblock_matched (
			G_OBJECT ( entry ), G_SIGNAL_MATCH_FUNC,
			0, 0, NULL, ( gpointer * ) changedFunc, NULL );

	return TCL_OK;
}

/**
\brief
**/
static char *traceFunc ( ClientData data, Tcl_Interp *interp,   const char *name1,  const char *name2, int flags )
{
	EntryParams *para = ( EntryParams * ) data;

	if ( para->inSetVar == 0 && name1 )
	{
		const char *txt = name1 ? Tcl_GetVar2 ( interp, name1, name2, 0 ) : NULL;

		if ( txt )
		{
			setVal ( para->entry, txt );
			doCommand ( para, txt, 1 );
		}
	}

	return NULL;
}

/**
\brief
**/
static int setVariable ( EntryParams *para, const char *val )
{
#ifdef DEBUG_ENTRY
	printf ( "entry/staticFuncs/setVariable\n" );
#endif

	if ( para->variable && para->inSetVar == 0 )
	{
		const char *ret;
		para->inSetVar = 1;
		ret = Tcl_SetVar ( para->interp, para->variable, val, TCL_GLOBAL_ONLY );
		para->inSetVar = 0;
		return ret == NULL ? TCL_ERROR : TCL_OK;
	}

	return TCL_OK;
}

/**
\brief
    Function associated with the widget.
*/
static void changedFunc ( GtkWidget *widget, gpointer data )
{
#ifdef DEBUG_ENTRY
	printf ( "entry/staticFuncs/changedFunc\n" );
#endif

	EntryParams *para = ( EntryParams * ) data;
	const char *val = gtk_entry_get_text ( para->entry );
	setVariable ( para, val );
	doCommand ( para, val, 1 );
}

/**
\brief
**/
static void destroyFunc (
	GtkWidget *widget,
	gpointer data )
{
#ifdef DEBUG_ENTRY
	printf ( "entry/staticFuncs/destroyFunc\n" );
#endif

	EntryParams *para = ( EntryParams * ) data;

	gnoclForgetWidgetFromName ( para->name );
	Tcl_DeleteCommand ( para->interp, para->name );

	gnoclAttachOptCmdAndVar (
		NULL, &para->onChanged,
		NULL, &para->variable,
		"changed", G_OBJECT ( para->entry ),
		G_CALLBACK ( changedFunc ), para->interp, traceFunc, para );

	g_free ( para->variable );
	g_free ( para->name );
	g_free ( para );
}

/**
\brief
**/
static int configure (	Tcl_Interp *interp,	EntryParams *para,	GnoclOption options[] )
{

#ifdef DEBUG_ENTRY
	printf ( "entry/staticFuncs/configure\n" );
#endif


	gnoclAttachOptCmdAndVar (
		&options[onChangedIdx], &para->onChanged,
		&options[variableIdx], &para->variable,
		"changed", G_OBJECT ( para->entry ),
		G_CALLBACK ( changedFunc ), interp, traceFunc, para );

	if ( options[variableIdx].status == GNOCL_STATUS_CHANGED
			&& options[valueIdx].status == 0  /* value is handled below */
			&& para->variable != NULL )
	{
		/* if variable does not exist -> set it, else set widget state */
		const char *val = Tcl_GetVar ( interp, para->variable, TCL_GLOBAL_ONLY );

		if ( val == NULL )
		{
			val = gtk_entry_get_text ( para->entry );
			setVariable ( para, val );
		}

		else
		{
			setVal ( para->entry, val );
		}
	}

	if ( options[valueIdx].status == GNOCL_STATUS_CHANGED )
	{
		char *str = options[valueIdx].val.str;
		setVal ( para->entry, str );
		setVariable ( para, str );
	}

	if ( options[cursorIdx].status == GNOCL_STATUS_CHANGED )
	{

		gint *bool = options[cursorIdx].val.i;
		gdk_window_set_cursor ( GTK_WIDGET ( para->entry )->window, bool );

	}

	if ( options[cursorPosIdx].status == GNOCL_STATUS_CHANGED )
	{
		char *str = options[cursorPosIdx].val.str;

		gint len = strlen ( gtk_editable_get_chars ( para->entry, 0, -1 ) );
		gint pos = atoi ( str );


		if ( strcmp ( str, "end" ) == 0 )
		{
			gtk_editable_set_position ( para->entry, -1 );
			return TCL_OK;

		}

		else if ( pos == -1 )
		{
			gtk_editable_set_position ( para->entry, -1 );
		}

		else if ( pos <= len )
		{
			gtk_editable_set_position ( para->entry, pos );
		}

		else
		{
			gtk_editable_set_position ( para->entry, -1 );
		}

		return TCL_OK;

		//setVal ( para->entry, str );
		//setVariable ( para, str );
	}

	/* set entry icon */
	if ( options[primaryIconIdx].status == GNOCL_STATUS_CHANGED )
	{
		GnoclStringType type = gnoclGetStringType ( options[primaryIconIdx].val.obj );

		if ( type == GNOCL_STR_EMPTY )
		{

		}

		else
		{
			GtkWidget *image = gnoclFindChild ( GTK_WIDGET ( para->entry ), GTK_TYPE_IMAGE );


			if ( ( type & ( GNOCL_STR_STOCK | GNOCL_STR_FILE ) ) == 0 )
			{
				Tcl_AppendResult ( interp, "Unknown type for \"",
								   Tcl_GetString ( options[primaryIconIdx].val.obj ),
								   "\" must be of type FILE (%/) or STOCK (%#)", NULL );
				return TCL_ERROR;

			}

			if ( image == NULL )
			{

			}

			if ( type & GNOCL_STR_STOCK )
			{
				GtkStockItem item;

				if ( gnoclGetStockItem ( options[primaryIconIdx].val.obj, interp, &item ) != TCL_OK )
				{
					return TCL_ERROR;
				}

				//gtk_image_set_from_stock ( GTK_IMAGE ( image ), item.stock_id, GTK_ICON_SIZE_BUTTON );
				gtk_entry_set_icon_from_stock ( para->entry, GTK_ENTRY_ICON_PRIMARY, item.stock_id );

			}

			else if ( type & GNOCL_STR_FILE )
			{
				GdkPixbuf *pix = gnoclPixbufFromObj ( interp, options + primaryIconIdx );

				if ( pix == NULL )
				{
					return TCL_ERROR;
				}

				gtk_entry_set_icon_from_pixbuf ( para->entry, GTK_ENTRY_ICON_PRIMARY, pix );
			}
		}
	}

	/* set secondary icon */
	if ( options[secondaryIconIdx].status == GNOCL_STATUS_CHANGED )
	{
		GnoclStringType type = gnoclGetStringType ( options[secondaryIconIdx].val.obj );

		if ( type == GNOCL_STR_EMPTY )
		{

		}

		else
		{
			GtkWidget *image = gnoclFindChild ( GTK_WIDGET ( para->entry ), GTK_TYPE_IMAGE );


			if ( ( type & ( GNOCL_STR_STOCK | GNOCL_STR_FILE ) ) == 0 )
			{
				Tcl_AppendResult ( interp, "Unknown type for \"",
								   Tcl_GetString ( options[secondaryIconIdx].val.obj ),
								   "\" must be of type FILE (%/) or STOCK (%#)", NULL );
				return TCL_ERROR;

			}

			if ( image == NULL )
			{

			}

			if ( type & GNOCL_STR_STOCK )
			{
				GtkStockItem item;

				if ( gnoclGetStockItem ( options[secondaryIconIdx].val.obj, interp, &item ) != TCL_OK )
				{
					return TCL_ERROR;
				}

				//gtk_image_set_from_stock ( GTK_IMAGE ( image ), item.stock_id, GTK_ICON_SIZE_BUTTON );
				gtk_entry_set_icon_from_stock ( para->entry, GTK_ENTRY_ICON_SECONDARY, item.stock_id );

			}

			else if ( type & GNOCL_STR_FILE )
			{
				GdkPixbuf *pix = gnoclPixbufFromObj ( interp, options + secondaryIconIdx );

				if ( pix == NULL )
				{
					return TCL_ERROR;
				}

				gtk_entry_set_icon_from_pixbuf ( para->entry, GTK_ENTRY_ICON_SECONDARY, pix );
			}
		}
	}

	return TCL_OK;
}

/**
\brief
**/
static int cget ( Tcl_Interp *interp, EntryParams *para, GnoclOption options[], int idx )
{
#ifdef DEBUG_ENTRY
	printf ( "entry/staticFuncs/cget\n" );
#endif

	Tcl_Obj *obj = NULL;

	if ( idx == dataIdx )
	{
		obj = Tcl_NewStringObj ( g_object_get_data ( para->entry, "gnocl::data" ), -1 );
	}

	else if ( idx == variableIdx )
	{
		obj = Tcl_NewStringObj ( para->variable, -1 );
	}

	else if ( idx == onChangedIdx )
	{
		obj = Tcl_NewStringObj ( para->onChanged ? para->onChanged : "", -1 );
	}

	else if ( idx == valueIdx )
	{
		obj = Tcl_NewStringObj ( gtk_entry_get_text ( para->entry ), -1 );
	}

	if ( obj != NULL )
	{
		Tcl_SetObjResult ( interp, obj );
		return TCL_OK;
	}

	return gnoclCgetNotImplemented ( interp, options + idx );
}

/**
\brief
**/
int entryFunc ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{

#ifdef DEBUG_ENTRY
	debugStep ( __FUNCTION__, 1 );
#endif


	static const char *cmds[] =
	{
		"delete", "configure", "cget", "onChanged",
		"class", "get", "clear", "set",  "setPosition",
		"wordList", "popup", "progress", "pulse",
		NULL
	};

	enum cmdIdx
	{
		DeleteIdx, ConfigureIdx, CgetIdx, OnChangedIdx,
		ClassIdx, GetIdx, ClearIdx, SetIdx, SetPositionIdx,
		WordListIdx, PopupIdx, ProgressIdx, PulseIdx
	};

	EntryParams *para = ( EntryParams * ) data;

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
		case PulseIdx:
			{
				gtk_entry_progress_pulse ( GTK_WIDGET ( para->entry ) );

				//gnoclUpdateCmd ( data, interp, objc, objv);

			}
			break;
		case ProgressIdx:
			{
				gdouble fraction;

				Tcl_GetDoubleFromObj ( NULL, objv[2], &fraction );

				g_print ( "PROGRESS %f\n", fraction );

				gtk_entry_set_progress_fraction ( GTK_WIDGET ( para->entry ), fraction );

				//gnoclUpdateCmd ( data, interp, objc, objv);

			}
			break;
		case PopupIdx:
			{
				// 0   1     2       3       4
				// $id popup item    <path>
				// $id popup subMenu <path1> <path2>
				g_print ( "PopupIdx %s\n", Tcl_GetString ( objv[1] ) );
				g_print ( "PopupIdx %s\n", Tcl_GetString ( objv[2] ) );
				g_print ( "PopupIdx %s\n", Tcl_GetString ( objv[3] ) );

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

				getIdx ( popupOptions,  Tcl_GetString ( objv[2] ), &idx );

				switch ( idx )
				{
					case SeparatorIdx:
						{
							gnoclPopupMenuAddSeparator ( interp );
						}
						break;
					case ItemIdx:
						{
							gnoclPopupMenuAddItem ( interp, Tcl_GetString ( objv[3] ) );
						} break;
					case SubMenuIdx:
						{
							gnoclPopupMenuAddSubMenu ( interp, Tcl_GetString ( objv[3] ),  Tcl_GetString ( objv[4] ) );
						} break;
					default: {}
				}


			}

			break;

			/* manipulate completion word list */
		case WordListIdx:
			{
				static const char *subCmds[] =
				{
					"add", "clear", "delete", "list",
					NULL
				};

				enum subCmdIdx
				{
					AddIdx, ClearIdx, DeleteIdx, ListIdx
				};

				int subIdx;

				if ( Tcl_GetIndexFromObj ( interp, objv[2], subCmds, "command", TCL_EXACT, &subIdx ) != TCL_OK )
				{
					return TCL_ERROR;
				}

				switch ( subIdx )
				{
					case AddIdx:
						{

							gchar **words;
							gint i;

							GtkTreeModel *model;
							GtkTreeIter iter;

							model = gtk_entry_completion_get_model ( para->completion );

							/* split a list into its parts */
							words = g_strsplit ( Tcl_GetString ( objv[3] ), " ", -1 );

							for ( i = 0; words[i] != NULL; i++ )
							{

								gtk_list_store_append ( model, &iter );
								gtk_list_store_set ( model, &iter, 0, words[i], -1 );

							}

							g_strfreev ( words );

						} break;
					case ClearIdx: {} break;
					case DeleteIdx: {} break;
					case ListIdx: {} break;
					default:
						{
							return TCL_ERROR;
						}
				}

			}
			break;
		case SetPositionIdx:
			{


				if ( 1 )
				{
					gtk_entry_set_position ( GTK_WIDGET ( para->entry ) , Tcl_GetString ( objv[2] ) );
				}

				else
				{
					gtk_editable_set_position ( GTK_EDITABLE ( GTK_WIDGET ( para->entry ) ) , Tcl_GetString ( objv[2] ) );
				}
			}

			break;
		case SetIdx:
			{
				/* simply set the text to nothing */
				gtk_entry_set_text ( para->entry, Tcl_GetString ( objv[2] ) );
			}

			break;
		case GetIdx:
			{
				/* equivalent to widget cget -value */
				Tcl_Obj *obj = NULL;

				obj = Tcl_NewStringObj ( gtk_entry_get_text ( para->entry  ), -1 );

				if ( obj != NULL )
				{
					Tcl_SetObjResult ( interp, obj );
					return TCL_OK;
				}
			}

			break;
		case ClearIdx:
			{
				/* simply set the text to nothing */
				gtk_entry_set_text ( para->entry, "" );
			}

			break;
		case ClassIdx:
			{

				Tcl_SetObjResult ( interp, Tcl_NewStringObj ( "entry", -1 ) );
			}
			break;
		case DeleteIdx:
			{
				return gnoclDelete ( interp, GTK_WIDGET ( para->entry ), objc, objv );
			}
		case ConfigureIdx:
			{
#ifdef DEBUG_ENTRY
				g_print ( "entryFunc ConfigureIdx\n" );
#endif
				int ret = TCL_ERROR;

				if ( gnoclParseAndSetOptions ( interp, objc - 1, objv + 1,
											   entryOptions, G_OBJECT ( para->entry ) ) == TCL_OK )
				{
					ret = configure ( interp, para, entryOptions );
				}

				gnoclClearOptions ( entryOptions );

				return ret;
			}

			break;

		case CgetIdx:
			{
				int     idx;

				switch ( gnoclCget ( interp, objc, objv, G_OBJECT ( para->entry ), entryOptions, &idx ) )
				{
					case GNOCL_CGET_ERROR:
						return TCL_ERROR;
					case GNOCL_CGET_HANDLED:
						return TCL_OK;
					case GNOCL_CGET_NOTHANDLED:
						return cget ( interp, para, entryOptions, idx );
				}
			}

		case OnChangedIdx:
			{
				const char *txt = gtk_entry_get_text ( para->entry );

				if ( objc != 2 )
				{
					Tcl_WrongNumArgs ( interp, 2, objv, NULL );
					return TCL_ERROR;
				}

				return doCommand ( para, txt, 0 );
			}
	}

	return TCL_OK;
}

/**
\brief
**/
int gnoclEntryCmd (	ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{
#ifdef DEBUG_ENTRY
	printf ( "entry/staticFuncs/gnoclEntryCmd\n" );
#endif

	EntryParams *para;
	int ret;

	if ( gnoclParseOptions ( interp, objc, objv, entryOptions ) != TCL_OK )
	{
		gnoclClearOptions ( entryOptions );
		return TCL_ERROR;
	}

	para = g_new ( EntryParams, 1 );
	para->entry = GTK_ENTRY ( gtk_entry_new( ) );
	//para->entry = GTK_ENTRY ( gtk_undo_entry_new() );

	para->interp = interp;
	para->variable = NULL;
	para->onChanged = NULL;
	para->inSetVar = 0;

	gtk_entry_set_activates_default ( para->entry, TRUE );
	gtk_widget_show ( GTK_WIDGET ( para->entry ) );

	/* add completion */

	//GtkEntryCompletion *completion;
	GtkListStore *model;
	GtkTreeIter iter;
	gint i;

	const gchar *text[] =
	{
		NULL,
	};

	/* Create a new completion... */
	para->completion = gtk_entry_completion_new();

	/* ...and set things up for the first column */
	gtk_entry_completion_set_text_column ( para->completion, 0 );

	/* Create a list store with one string column... */
	model = gtk_list_store_new ( 1, G_TYPE_STRING );

	/* ...and set it as the completion's model... */
	gtk_entry_completion_set_model ( para->completion, GTK_TREE_MODEL ( model ) );

	/* ...and drop our reference to the model */
	g_object_unref ( model );

	/* Add some text to be matched */
	for ( i = 0; text[i] != NULL; i++ )
	{
		gtk_list_store_append ( model, &iter );
		gtk_list_store_set ( model, &iter, 0, text[i], -1 );
	}

	gtk_entry_set_completion ( GTK_ENTRY ( para->entry ), para->completion );

	/* set options */
	ret = gnoclSetOptions ( interp, entryOptions, G_OBJECT ( para->entry ), -1 );

	/* apply custom settigs */
	if ( ret == TCL_OK )
	{
		ret = configure ( interp, para, entryOptions );
	}

	gnoclClearOptions ( entryOptions );

	/* set cleanup handler */
	if ( ret != TCL_OK )
	{
		gtk_widget_destroy ( GTK_WIDGET ( para->entry ) );
		g_free ( para );
		return TCL_ERROR;
	}

	/* register the new widget */
	para->name = gnoclGetAutoWidgetId();

	g_signal_connect ( G_OBJECT ( para->entry ), "destroy", G_CALLBACK ( destroyFunc ), para );

	gnoclMemNameAndWidget ( para->name, GTK_WIDGET ( para->entry ) );

	Tcl_CreateObjCommand ( interp, para->name, entryFunc, para, NULL );

	Tcl_SetObjResult ( interp, Tcl_NewStringObj ( para->name, -1 ) );

	return TCL_OK;
}

