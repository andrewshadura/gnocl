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
	2012-08:	new option
				-hasFocus
	2012-01:	began development
\endverbatim
**/

#include "gnocl.h"
#include "gnoclparams.h"

#include <string.h>
#include <assert.h>


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
} LabelEntryParams;


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
static const int entryWidthCharsIdx = 8;
static const int valueIdx = 9;
static const int variableIdx = 10;
static const int onChangedIdx = 11;
static const int onActivateIdx = 12;
static const int hasFocusIdx = 12;

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

	/* entry */
	{ "-entryWidthChars", GNOCL_STRING, NULL }, // 8
	{ "-value", GNOCL_STRING, NULL },           // 9

	{ "-variable", GNOCL_STRING, NULL },      /* 10 */
	{ "-onChanged", GNOCL_STRING, NULL },     /* 11 */
	{ "-onActivate", GNOCL_STRING, NULL},     /* 12 */
	
	{ "-hasFocus", GNOCL_BOOL, NULL },        /* 12 */

	//{ "-widthChars", GNOCL_INT, "width-chars" },
	{ "-data", GNOCL_OBJ, "", gnoclOptData },
	{ "-sensitive", GNOCL_BOOL, "sensitive" },
	{ "-baseFont", GNOCL_OBJ, "Sans 14", gnoclOptGdkBaseFont },
	{ "-baseColor", GNOCL_OBJ, "normal", gnoclOptGdkColorBase },
	{ "-background", GNOCL_OBJ, "normal", gnoclOptGdkColorBg },

	{ NULL },
};



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
static int configure ( Tcl_Interp *interp, LabelEntryParams *para, GnoclOption options[] )
{
	g_print ( "%s\n", __FUNCTION__ );


	gnoclAttachOptCmdAndVar (
		&options[onChangedIdx], &para->onChanged,
		&options[variableIdx], &para->variable,
		"changed", G_OBJECT ( para->entry ),
		G_CALLBACK ( changedFunc ), interp, traceFunc, para ); // see entry.c


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
		gtk_widget_grab_focus (para->entry);
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


	if ( options[alignIdx].status == GNOCL_STATUS_CHANGED )
	{
		g_print ( "%s alignIdx\n", __FUNCTION__ );
		char *str = options[alignIdx].val.str;

		const char *txt[] = { "left", "right", "center", "fill", NULL };
		const int types[] = { GTK_JUSTIFY_LEFT, GTK_JUSTIFY_RIGHT, GTK_JUSTIFY_CENTER, GTK_JUSTIFY_FILL };
		gint idx;

		getIdx ( txt, str, &idx );

		switch ( idx )
		{
			case GTK_JUSTIFY_LEFT:
				{
					gtk_label_set_justify ( para->label, GTK_JUSTIFY_LEFT );
				}
				break;
			case GTK_JUSTIFY_RIGHT:
				{
					gtk_label_set_justify ( para->label, GTK_JUSTIFY_RIGHT );
				}
				break;
			case GTK_JUSTIFY_CENTER:
				{
					gtk_label_set_justify ( para->label, GTK_JUSTIFY_CENTER );
				}
				break;
			case GTK_JUSTIFY_FILL:
				{
					gtk_label_set_justify ( para->label, GTK_JUSTIFY_FILL );
				}
				break;

		}
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

	return TCL_OK;
}



/**
\brief
**/
static int cget ( Tcl_Interp *interp, LabelEntryParams *para, GnoclOption options[], int idx )
{
#ifdef DEBUG_labelEntry
	g_print ( "%s %s \n", __FUNCTION__, options[1] );
#endif


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
			Tcl_SetObjResult ( interp, Tcl_NewStringObj ( "labelEntry", -1 ) );
			break;
		case DeleteIdx:
			return gnoclDelete ( interp, GTK_WIDGET ( para->hbox ), objc, objv );

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
				g_print ( "cget -not yet implemented\n" );
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
	g_free ( para );
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

	int pos = 0;

	GtkBox *hbox;
	GtkWidget *label;
	GtkWidget *entry;

	/* create widgets */
	/* 1) the container */
	gboolean homogeneous = 0;
	gint spacing = 5;
	para->hbox = gtk_hbox_new ( homogeneous, spacing );
	/* 2) the label */
	const gchar *str = "<blank>";
	para->label = gtk_label_new ( str );
	/* 3) the entry */
	para->entry = gtk_entry_new ();
	gtk_entry_set_text ( para->entry, "<blank>" );

	/* pack the components */
	// gtk_box_pack_end (GtkBox *box, GtkWidget *child, gboolean expand, gboolean fill, guint padding);
	gtk_box_pack_end ( para->hbox, para->entry, 0, 0, 2 );
	gtk_box_pack_end ( para->hbox, para->label, 0, 0, 2 );

	gtk_widget_show_all ( GTK_WIDGET ( para->hbox ) );

	g_print ( "%s 1\n", __FUNCTION__ );

	ret = gnoclSetOptions ( interp, labelEntryOptions, G_OBJECT ( para->entry ), -1 );

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
