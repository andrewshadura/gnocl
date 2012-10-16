/* labelEntry.c
authors	William J Giddings
date	20012
*/

/**
\page page_labelEntry gnocl::labelEntry
\htmlinclude labelEntry.html
\note
**/

/**
\par Modification History
\verbatim
	2012-09: new options
				-data
				-labelWidthGroup
				-labelHeightGroup
				-onKeyPress
				-onKeyRelease
				-onChanged
				-sensitive
				-editable
				-baseFont
				-labelBaseFont
				-useMarkup <---- NOT WORKING PROPERLY
				-justify <----- NOT WORKING PROPERLY
	2012-08: new options
				-hasFocus
				-onActivate
				-variable
	2012-01: began development
\endverbatim
**/


#include "gnocl.h"
#include "gnoclparams.h"
#include <string.h>
#include <assert.h>
#include <gdk/gdkkeysyms.h>

#include <glib.h>
#include <glib/gprintf.h>
/*
#include "gnocl.h"
#include "string.h"
#include "math.h"

#include <gdk/gdkkeysyms.h>
#include <assert.h>
#include <gdk/gdk.h>
*/


typedef struct
{
	Tcl_Interp  *interp;
	char        *name;
	GtkBox	    *hbox;
	GtkWidget   *label;
	GtkWidget   *entry;
	char        *variable;
	char        *focus;
	char        *onChanged;
	int         inSetVar;
	gboolean	check;
	char		*data;
} LabelEntryParams;

static const char *keyvalToString ( guint keyval );
static void changedFunc ( GtkWidget *widget, gpointer data );
static int doCommand ( LabelEntryParams *para, const char *val, int background );
static int setVariable ( LabelEntryParams *para, const char *val );
static char *traceFunc ( ClientData data, Tcl_Interp *interp,   const char *name1,  const char *name2, int flags );
static int setVal ( GtkEntry *entry, const char *txt );

static const int spacingIdx = 0;
static const int shadowIdx      = 1;
static const int alignIdx       = 2;
static const int justifyIdx     = 3;
static const int widthCharsIdx  = 4;
static const int textIdx        = 5;
static const int tooltipIdx     = 6;
static const int labelBaseFontIdx  = 7;
static const int labelWidthGroupIdx = 8;
static const int entryWidthCharsIdx = 9;
static const int valueIdx = 10;
static const int variableIdx = 11;
static const int onChangedIdx = 12;
static const int onActivateIdx = 13;
static const int onKeyPressIdx = 14;
static const int onKeyReleaseIdx = 15;
static const int hasFocusIdx = 16;
static const int entryWidthGroupIdx = 17;
static const int sensitiveIdx = 18;
static const int editableIdx = 19;
static const int baseFontIdx = 20;
static const int useMarkupIdx = 21;
static const int dataIdx = 22;

//static enum  optsIdx { CollapsedIdx, EllipsizeIdx, ReliefIdx, LabelIdx, LabelWidgetIdx };

static GnoclOption labelEntryOptions[] =
{
	/* hbox */
	{ "-spacing", GNOCL_INT, NULL },          // 0
	{ "-shadow", GNOCL_STRING, NULL },        // 1

	/* label */
	{ "-align", GNOCL_STRING, NULL },         // 2
	{ "-justify", GNOCL_STRING, NULL },       // 3
	{ "-widthChars", GNOCL_INT, NULL },       // 4
	{ "-text", GNOCL_STRING, NULL },          // 5
	{ "-tooltip", GNOCL_STRING, NULL },       // 6
	{ "-labelBaseFont", GNOCL_STRING, NULL }, // 7
	{ "-labelWidthGroup", GNOCL_OBJ, NULL },  // 8

	/* entry */
	{ "-entryWidthChars", GNOCL_STRING, NULL }, // 9
	{ "-value", GNOCL_STRING, NULL },          // 10
	{ "-variable", GNOCL_STRING, NULL },       // 11
	{ "-onChanged", GNOCL_STRING, NULL },      // 12
	{ "-onActivate", GNOCL_OBJ, NULL},         // 13
	{ "-onKeyPress", GNOCL_OBJ, NULL},         // 14
	{ "-onKeyRelease", GNOCL_OBJ, NULL},       // 15
	{ "-hasFocus", GNOCL_BOOL, NULL },         // 16
	{ "-entryWidthGroup", GNOCL_OBJ, NULL },   // 17  ***** IS THIS NECESSARY? *****
	{ "-sensitive", GNOCL_BOOL, NULL },		   // 18
	{ "-editable", GNOCL_BOOL, NULL },		   // 19
	{ "-baseFont", GNOCL_STRING, NULL },		  // 20

	{ "-useMarkup", GNOCL_BOOL, NULL},		   // 21 ***** not working properly *****


	//{ "-widthChars", GNOCL_INT, "width-chars" },
	//{ "-data", GNOCL_OBJ, "", gnoclOptData }, // 22
	{ "-data", GNOCL_STRING, NULL },

	{ "-baseColor", GNOCL_OBJ, "normal", gnoclOptGdkColorBase }, //23
	{ "-background", GNOCL_OBJ, "normal", gnoclOptGdkColorBg }, //24

	{ NULL },
};


/**
\brief     Handles the 'activate' signal.
\author    William J Giddings
\date      25/04/2010
\since     0.9.95
\note      Used by: gnome::entry
\**/
static void doOnActivate ( GtkEntry *entry, gpointer user_data )
{


	GnoclCommandData *cs = ( GnoclCommandData * ) user_data;
	//GtkTextIter *iter;

	GnoclPercSubst ps[] =
	{
		{ 'w', GNOCL_STRING },  /* widget name */
		{ 't', GNOCL_STRING },  /* entry content */
		{ 'd', GNOCL_STRING },  /* widget data */
		{ 0 }
	};

	ps[0].val.str = gnoclGetNameFromWidget ( gtk_widget_get_parent ( entry ) ); /* widget has no name */
	ps[1].val.str = gtk_entry_get_text ( entry );
	ps[2].val.str = g_object_get_data ( G_OBJECT ( entry ), "gnocl::data" );

	gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 );
}

/**
 * duplicate of parseoptions.c code
 */
static void doOnKey ( GtkWidget *entry, GdkEventKey *event, gpointer user_data )
{

	GnoclCommandData *cs = ( GnoclCommandData * ) user_data;

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

	ps[0].val.str   = gnoclGetNameFromWidget ( gtk_widget_get_parent ( entry ) );
	ps[1].val.i     = event->keyval;
	ps[2].val.str   = keyvalToString ( event->keyval );
	ps[3].val.obj   = unicode ?  Tcl_NewUnicodeObj ( ( Tcl_UniChar * ) & unicode, 1 ) : Tcl_NewStringObj ( "", 0 );
	ps[4].val.i     = event->state;
	ps[5].val.str   = gtk_widget_get_name ( gtk_widget_get_parent ( entry ) );

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
 * duplicate of parseoptions.c code
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
static char *traceFunc ( ClientData data, Tcl_Interp *interp,   const char *name1,  const char *name2, int flags )
{
	LabelEntryParams *para = ( LabelEntryParams * ) data;

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
static int setVariable ( LabelEntryParams *para, const char *val )
{

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
**/
static int setVal ( GtkEntry *entry, const char *txt )
{

	g_print ( "%s\n", __FUNCTION__ );


	int blocked = g_signal_handlers_block_matched ( G_OBJECT ( entry ), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, ( gpointer * ) changedFunc, NULL );
	gtk_entry_set_text ( entry, txt );

	if ( blocked )
	{
		g_signal_handlers_unblock_matched ( G_OBJECT ( entry ), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, ( gpointer * ) changedFunc, NULL );
	}

	return TCL_OK;
}

/**
\brief
    Function associated with the widget.
**/

static void changedFunc ( GtkWidget *widget, gpointer data )
{

	LabelEntryParams *para = ( LabelEntryParams * ) data;
	const char *val = gtk_entry_get_text ( para->entry );
	setVariable ( para, val );
	doCommand ( para, val, 1 );
}


/**
\brief
**/
static int doCommand ( LabelEntryParams *para, const char *val, int background )
{
	if ( para->onChanged )
	{
		GnoclPercSubst ps[] =
		{
			{ 'w', GNOCL_STRING },  /* widget */
			{ 'v', GNOCL_STRING },  /* value */
			{ 'd', GNOCL_STRING },  /* value */
			{ 0 }
		};

		ps[0].val.str = para->name;
		ps[1].val.str = val;
		ps[2].val.str = g_object_get_data ( G_OBJECT ( para->entry ), "gnocl::data" );

		return gnoclPercentSubstAndEval ( para->interp, ps, para->onChanged, background );
	}

	return TCL_OK;
}


/**
\brief
**/
static int configure ( Tcl_Interp *interp, LabelEntryParams *para, GnoclOption options[] )
{
	g_print ( "%s\n", __FUNCTION__ );


	gnoclAttachOptCmdAndVar (
		&options[onChangedIdx], &para->onChanged,
		&options[variableIdx], &para->variable,
		"changed", G_OBJECT ( para->entry ),
		G_CALLBACK ( changedFunc ), interp, traceFunc, para ); // see entry.c


	if ( options[dataIdx].status == GNOCL_STATUS_CHANGED )
	{
		para->data = strdup ( options[dataIdx].val.str );
	}

	if ( options[onActivateIdx].status == GNOCL_STATUS_CHANGED )
	{
		g_print ( "%s onActivateIdx\n", __FUNCTION__ );

		gnoclConnectOptCmd ( interp, G_OBJECT ( para->entry ), "activate", G_CALLBACK ( doOnActivate ), &options[onActivateIdx], NULL, NULL );

	}

	if ( options[onKeyPressIdx].status == GNOCL_STATUS_CHANGED )
	{
		g_print ( "%s onKeyPressIdx\n", __FUNCTION__ );

		gnoclConnectOptCmd ( interp, G_OBJECT ( para->entry ), "key-press-event", G_CALLBACK ( doOnKey ), &options[onKeyPressIdx], NULL, NULL );

	}

	if ( options[onKeyReleaseIdx].status == GNOCL_STATUS_CHANGED )
	{
		g_print ( "%s onKeyReleaseIdx\n", __FUNCTION__ );

		gnoclConnectOptCmd ( interp, G_OBJECT ( para->entry ), "key-release-event", G_CALLBACK ( doOnKey ), &options[onKeyReleaseIdx], NULL, NULL );

	}


	if ( options[variableIdx].status == GNOCL_STATUS_CHANGED
			&& options[valueIdx].status == 0  // value is handled below
			&& para->variable != NULL )
	{
		// if variable does not exist -> set it, else set widget state
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

	if ( options[hasFocusIdx].status == GNOCL_STATUS_CHANGED )
	{
		char *str = options[valueIdx].val.str;
		gtk_widget_grab_focus ( para->entry );
	}

	if ( options[sensitiveIdx].status == GNOCL_STATUS_CHANGED )
	{

		gtk_widget_set_sensitive ( para->entry, options[sensitiveIdx].val.i );

	}

	if ( options[editableIdx].status == GNOCL_STATUS_CHANGED )
	{

		gtk_entry_set_editable  ( para->entry, options[editableIdx].val.i );

	}

	if ( options[useMarkupIdx].status == GNOCL_STATUS_CHANGED )
	{
		gtk_label_set_use_markup ( para->label, options[useMarkupIdx].val.i );
	}


	/*--------------------_*/

	if ( options[spacingIdx].status == GNOCL_STATUS_CHANGED )
	{
		g_print ( "%s spacingIdx\n", __FUNCTION__ );

		gint spacing = options[spacingIdx].val.i;

		//g_object_set ( G_OBJECT(para->hbox),"border-width", pad, NULL );
		gtk_box_set_spacing ( para->hbox, spacing );
	}

	if ( options[textIdx].status == GNOCL_STATUS_CHANGED )
	{
		g_print ( "%s textIdx\n", __FUNCTION__ );
		char *str = options[textIdx].val.str;
		gtk_label_set_text ( para->label, str );
	}

	if ( options[valueIdx].status == GNOCL_STATUS_CHANGED )
	{
		g_print ( "%s valueIdx\n", __FUNCTION__ );
		char *str = options[valueIdx].val.str;
		gtk_entry_set_text ( para->entry, str );
	}

	if ( options[labelWidthGroupIdx].status == GNOCL_STATUS_CHANGED )
	{
		gnoclOptWidthGroup ( interp, &options[labelWidthGroupIdx], para->label, NULL );
	}

	if ( options[entryWidthGroupIdx].status == GNOCL_STATUS_CHANGED )
	{
		gnoclOptWidthGroup ( interp, &options[labelWidthGroupIdx], para->entry, NULL );
	}



	if ( options[alignIdx].status == GNOCL_STATUS_CHANGED )
	{
		g_print ( "%s alignIdx\n", __FUNCTION__ );

		//Tcl_Obj *ret =NULL;
		//gnoclOptBothAlign ( interp, &options[alignIdx], para->label, &ret );
	}

	if ( options[widthCharsIdx].status == GNOCL_STATUS_CHANGED )
	{
		g_print ( "%s widthCharsIdx\n", __FUNCTION__ );
		gint n_chars = options[widthCharsIdx].val.i;
		gtk_label_set_width_chars ( para->label, n_chars );
	}

	if ( options[entryWidthCharsIdx].status == GNOCL_STATUS_CHANGED )
	{
		g_print ( "%s entryWidthCharsIdx\n", __FUNCTION__ );
		gint n_chars = options[entryWidthCharsIdx].val.i;
		gtk_entry_set_width_chars ( para->entry, n_chars );
	}


	if ( options[tooltipIdx].status == GNOCL_STATUS_CHANGED )
	{
		g_print ( "%s tooltipIdx\n", __FUNCTION__ );

		char *txt = options[tooltipIdx].val.str;
		gtk_widget_set_tooltip_text ( GTK_WIDGET ( para->label ), txt );

		/* how to set it this way? */
		//int gnoclOptTooltip ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
		//gnoclOptTooltip ( interp, options[tooltipIdx], G_OBJECT ( para->label ),NULL );

	}

	if ( options[labelBaseFontIdx].status == GNOCL_STATUS_CHANGED )
	{
		g_print ( "%s labelBaseFontIdx\n", __FUNCTION__ );
		gchar *fnt = options[labelBaseFontIdx].val.str;

		PangoFontDescription *font_desc = pango_font_description_from_string ( fnt );
		gtk_widget_modify_font ( GTK_WIDGET ( para->label ), font_desc );
		pango_font_description_free ( font_desc );
	}

	if ( options[baseFontIdx].status == GNOCL_STATUS_CHANGED )
	{
		g_print ( "%s baseFontIdx\n", __FUNCTION__ );
		gchar *fnt = options[baseFontIdx].val.str;

		PangoFontDescription *font_desc = pango_font_description_from_string ( fnt );
		gtk_widget_modify_font ( GTK_WIDGET ( para->entry ), font_desc );
		pango_font_description_free ( font_desc );
	}

	return TCL_OK;
}



/**
\brief
**/
static int cget ( Tcl_Interp *interp, LabelEntryParams *para, GnoclOption options[], int idx )
{
#ifdef DEBUG_labelEntry
	g_print ( "%s %s %d\n", __FUNCTION__, options[idx], idx );
#endif

	Tcl_Obj *obj = NULL;

	/* get data */
	if ( idx == dataIdx )
	{
		obj = Tcl_NewStringObj ( para->data, -1 );
		//gnoclOptParaData ( interp, para->data, &obj);
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
int labelEntryFunc ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{
#ifdef DEBUG_labelEntry
	g_print ( "%s\n", __FUNCTION__, );
#endif

	LabelEntryParams *para = ( LabelEntryParams * ) data;

	static const char *cmds[] = { "delete", "configure", "cget", "class", NULL };
	enum cmdIdx { DeleteIdx, ConfigureIdx, CgetIdx, ClassIdx };

	gint idx;

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
		case ClassIdx:
			{
				Tcl_SetObjResult ( interp, Tcl_NewStringObj ( "labelEntry", -1 ) );
				break;
			}
		case DeleteIdx:
			{
				return gnoclDelete ( interp, GTK_WIDGET ( para->hbox ), objc, objv );
			}

		case ConfigureIdx:
			{
				int ret = TCL_ERROR;
				ret = configure ( interp, para, labelEntryOptions );

				if ( 1 )
				{
					if ( gnoclParseAndSetOptions ( interp, objc - 1, objv + 1, labelEntryOptions, G_OBJECT ( para->entry ) ) == TCL_OK )
					{
						ret = configure ( interp, para, labelEntryOptions );
					}
				}

				gnoclClearOptions ( labelEntryOptions );
				return ret;
			}

			break;

		case CgetIdx:
			{
				gint idx2;

				g_print ( "CgetIdx 1, %s\n", Tcl_GetString ( objv[2] ) );
				//getIdx ( labelEntryOptions, Tcl_GetString(objv[2]), &idx2 );
				g_print ( "CgetIdx 2\n" );
				cget ( interp, para, labelEntryOptions, 22 );
			}
	}

	return TCL_OK;
}


/**
\note	modify to suit labelentry params.
**/
static void destroyFunc ( GtkWidget *widget, gpointer data )
{
#ifdef DEBUG_labelentry
	printf ( "%s\n", __FUNCTION__ );
#endif

	LabelEntryParams *para = ( LabelEntryParams * ) data;
	gnoclForgetWidgetFromName ( para->name );
	Tcl_DeleteCommand ( para->interp, para->name );
	g_free ( para->name );
	g_free ( para->data );
	g_free ( para );
}


/**
\brief
**/
static void alignDestroyFunc ( GtkWidget *widget, gpointer data )
{
	gtk_widget_destroy ( GTK_WIDGET ( data ) );
}

/**
\brief
**/
int gnoclLabelEntryCmd ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{

	LabelEntryParams *para;
	int ret = TCL_OK;


	if ( gnoclParseOptions ( interp, objc, objv, labelEntryOptions ) != TCL_OK )
	{
		gnoclClearOptions ( labelEntryOptions );
		return TCL_ERROR;
	}

	para = g_new ( LabelEntryParams, 1 );
	para->interp = interp;
	para->variable = NULL;
	para->onChanged = NULL;
	para->inSetVar = 0;

	int needAlign = 0;

	/* create widgets */
	/* 2) the label */
	const gchar *str = "";
	para->label = gtk_label_new ( str );
	gtk_label_set_justify ( para->label, GTK_JUSTIFY_LEFT );
	para->entry = gtk_entry_new ();
	gtk_entry_set_text ( para->entry, "" );

	/* add to containers */
	if ( needAlign )
	{
		GtkWidget *align = gtk_alignment_new ( 1, 0, 0, 0 );
		gtk_container_add ( align, para->label );
		gtk_container_add ( para->hbox, align );
	}

	else
	{
		gboolean homogeneous = 0;
		gint spacing = 2;
		para->hbox = gtk_hbox_new ( homogeneous, spacing );
		gtk_box_pack_start ( para->hbox, para->label, 0, 0, 0 );
	}

	gtk_box_pack_end ( para->hbox, para->entry, 1, 1, 0 );

	gtk_widget_show_all ( GTK_WIDGET ( para->hbox ) );

	g_print ( "%s 1\n", __FUNCTION__ );

	if ( ret == TCL_OK )
	{
		ret = configure ( interp, para, labelEntryOptions );
	}

	g_print ( "%s 2\n", __FUNCTION__ );

	gnoclClearOptions ( labelEntryOptions );

	g_print ( "%s 3\n", __FUNCTION__ );

	/* STEP 3)  -show the widget */

	if ( ret != TCL_OK )
	{
		gtk_widget_destroy ( GTK_WIDGET ( para->hbox ) );
		return TCL_ERROR;
	}

	g_print ( "%s 4\n", __FUNCTION__ );

	/* STEP 4)  -everything has worked, register the widget with the Tcl interpretor. */
	para->name = gnoclGetAutoWidgetId();
	g_signal_connect ( G_OBJECT ( para->hbox ), "destroy", G_CALLBACK ( destroyFunc ), para );

	g_print ( "%s 5\n", __FUNCTION__ );
	gnoclMemNameAndWidget ( para->name, GTK_WIDGET ( para->hbox ) );
	Tcl_CreateObjCommand ( interp, para->name, labelEntryFunc, para, NULL );

	g_print ( "%s 6\n", __FUNCTION__ );
	Tcl_SetObjResult ( interp, Tcl_NewStringObj ( para->name, -1 ) );

	g_print ( "%s 7\n", __FUNCTION__ );
	return TCL_OK;

}
