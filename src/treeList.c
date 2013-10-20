/**
\brief
\todo	add getColumn and getRow widget commands
\history
   2013-07: added commands, options, commands
   2013-01: added insert
   2012-12: fixed problem with -wrapMode failing to set to option word
   2012-09: new option
				-data
			began work on cget
   2012-07: columnConfigure
            added option: -widget (embed widget within column header)
   2011-12: insertRow
   2011-06: getFullList
   2011-05: added options: -onInteractiveSearch
   2011-02: added options: -wrapMode, -wrapWidth, -gridLines
   			subcommand: resize columns rows
   2009-02: added -baseFont -baseColor
   2008-10: added class
   2004-02: added stock pixmap
        12: added references
        09: added -onButtonPress and -onButtonRelease
        08: changed -single to -singleRow, added -singleColumn
        07: added pixbuf
        05: add -onCellData
            some cleanups
   2003-03: added collapse, expand, scrollToPosition, setSelection,
            onSelectionChanged, option -single to add a single row
        12: switched from GnoclWidgetOptions to GnoclOptionFgetWtt
        10: new command erase and getSelection
        09: renamed -command to -onToggled, -onEdited resp.
            renamed -selectionChanged to -onSelectionChanged
   2002-06: update to gtk 2.0: use of GtkTreeView and GtkListStore
            renamed back to list, should be no problem with tcl list
                  command, since we use namespaces
        11: renamed list to clist
            new commands sort, getCellText
            new option columnClicked
            removed option titleActive: now coupled with columnClicked
   2001-07: Begin of developement
**/
/*
 * $Id: treeList.c,v 1.32 2005/02/22 23:16:10 baum Exp $
 *
 * This file implements the list widget
 *
 * Copyright (c) 2001 - 2005 Peter G. Baum  http://www.dr-baum.net
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

/**
\page page_list gnocl::list
\htmlinclude list.html

\page page_tree gnocl::tree
\htmlinclude tree.html
**/

#include "gnocl.h"
#include "gnoclparams.h"
#include "drag_n_drop.h" /* keep separate until all issues ironed out */

/* static function declarations */
static int optSizing ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret );
static int ruleHint ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret );
static int treeLinePattern ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret );
static int gnoclOptSearchEntryWidget ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret );

static gnoclOptColorEvenRow ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret ); /* stubb */

static void setCellBgClr ( GtkTreeModel *model, GtkTreeIter *iter, gchar *clr );

/**
 */
static const char refPrefix[] = "ref";


/**
 */
enum ConfigType
{
	CONFIG_VALUE, CONFIG_VISIBLE,
	CONFIG_FGCLR, CONFIG_BGCLR
};

/*
  "columns-changed"                                : Run Last
  "cursor-changed"                                 : Run Last
  "expand-collapse-cursor-row"                     : Run Last / Action
  "move-cursor"                                    : Run Last / Action
  "row-activated"                                  : Run Last / Action
* "row-collapsed"                                  : Run Last
* "row-expanded"                                   : Run Last
  "select-all"                                     : Run Last / Action
  "select-cursor-parent"                           : Run Last / Action
  "select-cursor-row"                              : Run Last / Action
  "set-scroll-adjustments"                         : Run Last / Action
* "start-interactive-search"                       : Run Last / Action
  "test-collapse-row"                              : Run Last
  "test-expand-row"                                : Run Last
  "toggle-cursor-row"                              : Run Last / Action
  "unselect-all"                                   : Run Last / Action
 */



/**
 */
static const int columnsIdx            = 0;
static const int selectionModeIdx      = 1;
static const int childrenIdx           = 2;
static const int titlesIdx             = 3;
static const int typesIdx              = 4;
static const int scrollbarIdx          = 5;
static const int visibleIdx            = 6;
static const int onSelectionChangedIdx = 7;
static const int onRowExpandedIdx      = 8;
static const int onRowCollapsedIdx     = 9;
static const int treeLinePatternIdx    = 10;
static const int dataIdx			   = 11;
//static const int reorderableIdx        = 11;

static GnoclOption treeListOptions[] =
{
	{ "-columns", GNOCL_INT, NULL },             /* 0 */
	{ "-selectionMode", GNOCL_OBJ, NULL },       /* 1 */
	{ "-children", GNOCL_LIST, NULL },           /* 2 */
	{ "-titles", GNOCL_LIST, NULL },             /* 3 */
	{ "-types", GNOCL_LIST, NULL },              /* 4 */
	{ "-scrollbar", GNOCL_OBJ, NULL },           /* 5 */
	{ "-visible", GNOCL_BOOL, NULL },            /* 6 */
	{ "-onSelectionChanged", GNOCL_OBJ, NULL },  /* 7 */
	{ "-onRowExpanded", GNOCL_OBJ, NULL },       /* 8 */
	{ "-onRowCollapsed", GNOCL_OBJ, NULL },      /* 9 */
	{ "-treeLinePattern", GNOCL_STRING, NULL },  /* 10 */
	/* This is a problematic one. The parameter set via styles,
	   rather than properties.
	*/

	{ "-data", GNOCL_STRING, NULL }, /* 11 */
	{"-onInteractiveSearch", GNOCL_OBJ, "", gnoclOptOnInteractiveSearch},
	{"-onMoveCursor", GNOCL_OBJ, "", gnoclOptOnMoveCursor},

	{ "-tooltip", GNOCL_OBJ, "", gnoclOptTooltip },
	{ "-hasFocus", GNOCL_BOOL, "has-focus" },
	{ "-headersClickable", GNOCL_BOOL, "headers-clickable" },
	{ "-headersVisible", GNOCL_BOOL, "headers-visible" },
	{ "-heightGroup", GNOCL_OBJ, "h", gnoclOptSizeGroup },
	{ "-heightRequest", GNOCL_INT, "height-request" },
	{ "-name", GNOCL_STRING, "name" },
	{ "-onButtonPress", GNOCL_OBJ, "P", gnoclOptOnButton },
	{ "-onButtonRelease", GNOCL_OBJ, "R", gnoclOptOnButton },
	{ "-onPopupMenu", GNOCL_OBJ, "popup-menu", gnoclOptCommand },
	{ "-sizeGroup", GNOCL_OBJ, "s", gnoclOptSizeGroup },
	{ "-widthGroup", GNOCL_OBJ, "w", gnoclOptSizeGroup },
	{ "-widthRequest", GNOCL_INT, "width-request" },

	{ "-baseFont", GNOCL_OBJ, "Sans 14", gnoclOptGdkBaseFont },
	{ "-baseColor", GNOCL_OBJ, "normal", gnoclOptGdkColorBase },

	{ "-evenRowColor", GNOCL_OBJ, "", gnoclOptColorEvenRow }, /* still a code stub */

	/* GtkTreeList properties, actual colours set by theme engine based
	   upon other settings,
	   19/03/09
	*/
	{ "-ruleHint", GNOCL_BOOL, "rules-hint"},
	{ "-treeLines", GNOCL_BOOL, "enable-tree-lines" },
	{ "-gridLines", GNOCL_BOOL, "enable-grid-lines" },
	{ "-expanders",  GNOCL_BOOL, "show-expanders" },
	{ "-enableSearch", GNOCL_BOOL, "enable-search" },
	{ "-fixedHeight", GNOCL_BOOL, "fixed-height-mode" },

	{ "-hoverExpand", GNOCL_BOOL, "hover-expand" },
	{ "-hoverSelection",  GNOCL_BOOL, "hover-selection" },
	{ "-reorderable" , GNOCL_BOOL, "reorderable" },
	{ "-rubberBand", GNOCL_BOOL, "rubber-banding" },

	{ "-searchColumn",  GNOCL_INT, "search-column"},
	{ "-searchEntry",  GNOCL_OBJ, "", gnoclOptSearchEntryWidget },
	{ "-tooltipColumn",  GNOCL_INT, "tooltip-column"},



	{ NULL }
};

/*
 * "enable-grid-lines"        GtkTreeViewGridLines  : Read / Write
 * "enable-search"            gboolean              : Read / Write
 * "enable-tree-lines"        gboolean              : Read / Write
   "expander-column"          GtkTreeViewColumn*    : Read / Write
 * "fixed-height-mode"        gboolean              : Read / Write
   "hadjustment"              GtkAdjustment*        : Read / Write
 * "headers-clickable"        gboolean              : Read / Write
 * "headers-visible"          gboolean              : Read / Write
 * "hover-expand"             gboolean              : Read / Write
   "hover-selection"          gboolean              : Read / Write
   "level-indentation"        gint                  : Read / Write
   "model"                    GtkTreeModel*         : Read / Write
 * "reorderable"              gboolean              : Read / Write
 * "rubber-banding"           gboolean              : Read / Write
 * "rules-hint"               gboolean              : Read / Write
 * "search-column"            gint                  : Read / Write
 * "show-expanders"           gboolean              : Read / Write
 * "tooltip-column"           gint                  : Read / Write
   "vadjustment"              GtkAdjustment*        : Read / Write
*/

static const int widthIdx = 0;
static const int widgetIdx = 1;
static const int wrapModeIdx = 1;
/**
\brief	Column Options
**/
static GnoclOption colOptions[] =
{
	{ "-width", GNOCL_INT, NULL },               /* 0 */
	{ "-widget", GNOCL_STRING, NULL},
	{ "-clickable", GNOCL_BOOL, "clickable" },
	{ "-maxWidth", GNOCL_INT, "max-width" },
	{ "-minWidth", GNOCL_INT, "min-width" },
	{ "-reorderable", GNOCL_BOOL, "reorderable" },
	{ "-resizable", GNOCL_BOOL, "resizable" },
	{ "-sizing", GNOCL_OBJ, "sizing", optSizing },
	{ "-titleAlign", GNOCL_OBJ, "alignment", gnoclOptHalign },
	{ "-title", GNOCL_STRING, "title" },
	{ "-visible", GNOCL_BOOL, "visible" },
	{ "-onWidthChange", GNOCL_OBJ, "", gnoclOptNotify },
	{ "-onClicked", GNOCL_OBJ, "", gnoclOptOnColumnClicked },
	/* expiremental */
	{ "-widthGroup", GNOCL_OBJ, "w", gnoclOptSizeGroup },
	{ NULL }
};

/**
**/
static GnoclOption cellRenderOptions[] =
{
	{ "-onCellData", GNOCL_STRING, NULL },    /* 0 */
	{ "-align", GNOCL_OBJ, "?align", gnoclOptBothAlign },
	{ "-visible", GNOCL_BOOL, "visible" },
	{ "-xPad", GNOCL_INT, "xpad" },
	{ "-yPad", GNOCL_INT, "ypad" },
	{ "-renderWidth", GNOCL_INT, "width" },
	{ "-renderHeight", GNOCL_INT, "height" },
	{ NULL }
};

/**
**/
static GnoclOption textRenderOptions[] =
{
	{ "-value", GNOCL_STRING, "text" },
	{ "-background", GNOCL_OBJ, "background-gdk", gnoclOptGdkColor },
	{ "-foreground", GNOCL_OBJ, "foreground-gdk", gnoclOptGdkColor },
	{ "-editable", GNOCL_BOOL, "editable" },
	{ "-font", GNOCL_STRING, "font" },
	{ "-fontFamily", GNOCL_STRING, "family" },
	{ "-fontRise", GNOCL_OBJ, "rise", gnoclOptPangoScaledInt },
	{ "-fontScale", GNOCL_OBJ, "scale", gnoclOptScale },
	{ "-fontSize", GNOCL_OBJ, "size", gnoclOptPangoScaledInt },
	{ "-fontStretch", GNOCL_OBJ, "stretch", gnoclOptPangoStretch },
	{ "-fontStyle", GNOCL_OBJ, "style", gnoclOptPangoStyle },
	{ "-fontVariant", GNOCL_OBJ, "variant", gnoclOptPangoVariant },
	{ "-fontWeight", GNOCL_OBJ, "weight", gnoclOptPangoWeight },
	{ "-strikethrough", GNOCL_BOOL, "strikethrough" },
	{ "-underline", GNOCL_OBJ, "underline", gnoclOptUnderline },
	{ "-wrapMode", GNOCL_OBJ, "wrap-mode", gnoclOptPangoWrapMode },
	{ "-wrapWidth", GNOCL_INT, "wrap-width"  },
	{ NULL }
};

/**
**/
static GnoclOption toggleRenderOptions[] =
{
	{ "-value", GNOCL_BOOL, "active" },
	{ "-sensitive", GNOCL_BOOL, "activatable" },
	{ NULL }
};

/**
**/
static const int noCellRenderOptions = sizeof ( cellRenderOptions )  / sizeof ( GnoclOption );
static const int noTextRenderOptions = sizeof ( textRenderOptions )  / sizeof ( GnoclOption );
static const int noToggleRenderOptions = sizeof ( toggleRenderOptions ) / sizeof ( GnoclOption );

/**
**/
/* moved to gnocl.h */
/*
typedef struct
{
    Tcl_Interp          *interp;
    char                *name;
    GtkTreeView        *view;
    GtkScrolledWindow *scrollWin;
    int               noColumns;
    int               isTree;
    GHashTable        *idToIter;
} TreeListParams;
*/

/**
\brief	Set the treeview "even-row-color" propterty
\note	Still a code stub.
**/
int gnoclOptColorEvenRow ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{

#if 0
	g_print ( "%s\n", __FUNCTION__ );
#endif

	return TCL_OK;

	//return modifyWidgetGdkColor ( interp, opt, obj, gtk_widget_modify_fg, G_STRUCT_OFFSET ( GtkStyle, fg ), ret );
}

/**
**/
gboolean  search_foreach_func ( GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer data )
{
#if 0
	g_print ( "%s\n", __FUNCTION__ );
#endif

	gchar *str;
	gint column = 2;

	gtk_tree_model_get ( model, iter, column, &str, -1 );

#if 0
	g_print ( "%s data = %s ; %s\n", __FUNCTION__, data, str );
#endif
	return TCL_OK;
}

/**
**/
gboolean search_equal_func ( GtkTreeModel *model, gint column, const gchar *key, GtkTreeIter *iter, gpointer search_data )
{
#if 0
	g_print ( "%s\n", __FUNCTION__ );
#endif

	gchar *str;
	gchar *pos;
	gboolean res;

	GtkTreeIter row_iter;

	/* this function is set prior to the creation of any entries */
	gtk_tree_model_get_iter_first ( model, &row_iter );

	gtk_tree_model_get ( model, &row_iter, column, &str, -1 );
	pos = gtk_tree_model_get_string_from_iter ( model, &row_iter );
#if 0
	g_print ( "%s (%s,%d) content = %s\n", __FUNCTION__, pos, column, str );
#endif

	/* remaining rows */
	while ( gtk_tree_model_iter_next ( GTK_LIST_STORE ( model ), &row_iter ) )
	{

		gtk_tree_model_get ( model, &row_iter, column, &str, -1 );
		pos = gtk_tree_model_get_string_from_iter ( model, &row_iter );

#if 0
		g_print ( "%s (%s,%d) content = %s\n", __FUNCTION__, pos, column, str );
		g_print ( " Seaching for %s\n", str );
#endif
	}

	//gtk_tree_model_foreach (GtkTreeModel *model,search_foreach_func func,gpointer user_data);
	//gtk_container_foreach ( GTK_CONTAINER ( box ), removeChild, box );

	return TCL_OK;
}

/**
\brief
**/
static int gnoclOptSearchEntryWidget ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{

#if 0
	g_print ( "%s: search entry widget = %s\n", __FUNCTION__, Tcl_GetString ( opt->val.obj ) );
#endif

	GtkWidget *widget;

	widget = gnoclGetWidgetFromName ( Tcl_GetString ( opt->val.obj ), interp );

	gtk_tree_view_set_search_entry ( ( GTK_WIDGET ( obj ) ) , widget );

	return TCL_OK;

}

/**
\brief      Switch alternate row colouring option on or off for GtkTreeView widget. Default: 0.
\note       Actual shadings are derived from the current background settings.
**/
static int ruleHint ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
#if 0
	g_print ( "%s: option = %s\n", __FUNCTION__, opt->optName );
#endif

	/* obtain value passed to the option, then convert to an int */
	char *fnt = Tcl_GetStringFromObj ( opt->val.obj, NULL );

	if ( atoi ( fnt ) )
	{
		gtk_tree_view_set_rules_hint ( GTK_TREE_VIEW ( obj ), 1 );
	}

	else
	{
		gtk_tree_view_set_rules_hint ( GTK_TREE_VIEW ( obj ), 0 );
	}

	return TCL_OK;
}

/**
\brief      Modify the line pattern template used to render tree heirarchy lines.
\note       Maybe use
             void gtk_rc_parse_string (const gchar *rc_string);
             http://gtk2-engines-cleanice.sourcearchive.com/documentation/2.4.1-0ubuntu2/cleanice-style_8c-source.html
**/
static int treeLinePattern ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{

#if 0
	g_print ( "treeLinePattern -this option currently under development.\n" );
#endif
	static const gchar *rc_string =
	{
		"style \"solidTreeLines\"\n"
		"{\n"
		"   GtkTreeView::tree-line-pattern = \"\111\111\"\n"
		"}\n"
		"\n"
		"class \"GtkTreeView\" style \"solidTreeLines\"\n"
	};

	gtk_rc_parse_string ( rc_string );

	return TCL_OK;
}

/**
\brief
**/
static int optSizing ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	const char *txt[] = { "growOnly", "autosize", "fixed", NULL };
	const int types[] = { GTK_TREE_VIEW_COLUMN_GROW_ONLY,
						  GTK_TREE_VIEW_COLUMN_AUTOSIZE, GTK_TREE_VIEW_COLUMN_FIXED
						};

	assert ( sizeof ( GTK_TREE_VIEW_COLUMN_GROW_ONLY ) == sizeof ( int ) );

	return gnoclOptGeneric ( interp, opt, obj, "relief", txt, types, ret );
}

/**
\brief
\note 	The problem is that no new column can be added during the lifetime of a model.
		We therefore cannot add options in the model
**/
static int getOffset ( TreeListParams *para, enum ConfigType type, int col )
{
#if 0
	g_print ( "%s\n", __FUNCTION__ );
#endif

	/*
	   0 .. (N-1)                value
	   N .. (2*N-1)              visible         bool
	*/
	switch ( type )
	{
		case CONFIG_VALUE:        return col;
		case CONFIG_VISIBLE:      return col + para->noColumns;
		default:                  assert ( 0 );
	}

	return 0;
}

/**
\brief
**/
static int getColumn ( GtkCellRenderer *renderer )
{
#if 0
	g_print ( "%s\n", __FUNCTION__ );
#endif

	return GPOINTER_TO_INT ( g_object_get_data ( G_OBJECT ( renderer ), "gnoclColumn" ) );
}

/**
\brief
**/
static void destroyFunc ( GtkWidget *widget, gpointer data )
{
#if 0
	g_print ( "%s\n", __FUNCTION__ );
#endif

	TreeListParams *para = ( TreeListParams * ) data;

	gnoclForgetWidgetFromName ( para->name );
	Tcl_DeleteCommand ( para->interp, para->name );

	g_hash_table_destroy ( para->idToIter );
	g_free ( para->name );
	g_free ( para );
}

/**
\brief
**/
static int addIterator ( TreeListParams *para, GtkTreeIter *iter )
{
#if 0
	g_print ( "%s\n", __FUNCTION__ );
#endif

	static int no;

	GtkTreeIter *p = g_new ( GtkTreeIter, 1 );
	*p = *iter;

	++no;

	g_hash_table_insert ( para->idToIter, GINT_TO_POINTER ( no ), p );

	return no;
}

/**
\brief
**/
static int removeIterFromId ( TreeListParams *para, int id )
{
	void *p = g_hash_table_lookup ( para->idToIter, GINT_TO_POINTER ( id ) );

	if ( p )
	{
		g_hash_table_remove ( para->idToIter, GINT_TO_POINTER ( id ) );
		return 1;
	}

	return 0;
}

/**
\brief
**/
static void deleteCellData ( gpointer data )
{
#if 0
	g_print ( "%s\n", __FUNCTION__ );
#endif


	GnoclCommandData *cs = ( GnoclCommandData * ) data;
	g_free ( cs->command );
	g_free ( cs );
}

/**
\brief	Return value from a column for given row iterator.
**/
static Tcl_Obj *getObjFromCol ( GtkTreeModel *model, int colNo, GtkTreeIter *iter )
{
#if 0
	g_print ( "%s\n", __FUNCTION__ );
#endif

	Tcl_Obj *res;

	switch ( gtk_tree_model_get_column_type ( model, colNo ) )
	{
		case G_TYPE_BOOLEAN:
		case G_TYPE_UINT:
		case G_TYPE_INT:
			{
				int n;
				gtk_tree_model_get ( model, iter, colNo, &n, -1 );
				res = Tcl_NewIntObj ( n );
			}

			break;
		case G_TYPE_DOUBLE:
			{
				double n;
				gtk_tree_model_get ( model, iter, colNo, &n, -1 );
				res = Tcl_NewDoubleObj ( n );
			}

			break;
		case G_TYPE_OBJECT:
			{
				res = NULL;
			}
			break;
		default:
			{
				char *txt;
				gtk_tree_model_get ( model, iter, colNo, &txt, -1 );
				res = Tcl_NewStringObj ( txt, -1 );
				g_free ( txt );
			}
	}

	return res;
}

/**
\brief
**/
static Tcl_Obj *tclPathFromPath ( GtkTreePath *path )
{
#if 0
	g_print ( "%s\n", __FUNCTION__ );
#endif
	Tcl_Obj     *ret     = Tcl_NewListObj ( 0, NULL );
	int         *indices = gtk_tree_path_get_indices ( path );
	int         no       = gtk_tree_path_get_depth ( path );
	int         k;

	for ( k = 0; k < no; ++k )
	{
		Tcl_ListObjAppendElement ( NULL, ret, Tcl_NewIntObj ( indices[k] ) );
	}

	gtk_tree_path_free ( path );

	return ret;
}

/**
\brief
**/
static Tcl_Obj *tclPathFromIter ( GtkTreeModel *model, GtkTreeIter *iter )
{
#if 0
	g_print ( "%s\n", __FUNCTION__ );
#endif

	GtkTreePath *path = gtk_tree_model_get_path ( model, iter );
	return tclPathFromPath ( path );
}

/**
\brief
**/
/* "1:2:3" -> "1 2 3" */
static char *stringPathToTclPath ( char *path )
{
#if 0
	g_print ( "%s\n", __FUNCTION__ );
#endif

	char *ret = g_strdup ( path );
	char *p;

	for ( p = strchr ( ret, ':' ); p; p = strchr ( p, ':' ) )
		* p = ' ';

	return ret;
}

/**
\brief
**/
static GtkTreeIter getLastVisibleChild ( GtkTreeView *view, GtkTreeModel *model, GtkTreeIter *parent )
{
#if 0
	g_print ( "%s\n", __FUNCTION__ );
#endif


	GtkTreeIter ret;
	GtkTreeIter next;

	/* we look only at expanded trees */
	GtkTreePath *path = gtk_tree_model_get_path ( model, parent );

	if ( !gtk_tree_view_row_expanded ( view, path ) )
	{
		gtk_tree_path_free ( path );
		return *parent;
	}

	gtk_tree_path_free ( path );

	/* no children any more -> we are done */

	if ( gtk_tree_model_iter_children ( model, &next, parent ) == 0 )
		return *parent;

	/* find last child */
	do
		ret = next;

	while ( gtk_tree_model_iter_next ( model, &next ) );

	/* look for child of last child */
	return getLastVisibleChild ( view, model, &ret );
}

/**
\brief
**/
static int getLastVisibleIter ( GtkTreeView *view, GtkTreeModel *model,	GtkTreeIter *ret )
{
#if 0
	g_print ( "%s\n", __FUNCTION__ );
#endif

	GtkTreeIter iter, next;

	if ( gtk_tree_model_get_iter_first ( model, &next ) == 0 )
	{
		return TCL_ERROR;
	}

	do
	{
		iter = next;

	}
	while ( gtk_tree_model_iter_next ( model, &next ) );

	*ret = getLastVisibleChild ( view, model, &iter );

	return TCL_OK;
}

/**
\brief
**/
static GtkTreePath *tclPathToPath ( Tcl_Interp *interp, Tcl_Obj *obj, GtkTreeModel *model )
{
#if 0
	g_print ( "%s\n", __FUNCTION__ );
#endif
	int k, len;
	GtkTreePath *path;

	if ( Tcl_ListObjLength ( interp, obj, &len ) != TCL_OK )
	{
		return NULL;
	}

	path = gtk_tree_path_new( );

	/* TODO? last can be "end"? */
	for ( k = 0; k < len; ++k )
	{
		Tcl_Obj *tp;
		int     index;

		if ( Tcl_ListObjIndex ( interp, obj, k, &tp ) != TCL_OK )
		{
			goto cleanExit;
		}

		if ( Tcl_GetIntFromObj ( NULL, tp, &index ) != TCL_OK )
		{
			if ( strcmp ( Tcl_GetString ( tp ), "end" ) != 0 )
			{
				Tcl_AppendResult ( interp, "Expected integer or \"end\" but got \"",
								   Tcl_GetString ( tp ), "\"", ( char * ) NULL );
				goto cleanExit;
			}

			if ( k == 0 )
			{
				index = gtk_tree_model_iter_n_children ( model, NULL );
			}

			else
			{
				GtkTreeIter  iter;

				if ( gtk_tree_model_get_iter ( model, &iter, path ) == 0 )
				{
					Tcl_AppendResult ( interp, "Path \"", Tcl_GetString ( obj ), "\" not valid.", ( char * ) NULL );
					goto cleanExit;
				}

				index = gtk_tree_model_iter_n_children ( model, &iter );
			}

			--index;
		}

		gtk_tree_path_append_index ( path, index );
	}

	return path;

cleanExit:
	gtk_tree_path_free ( path );
	return NULL;
}

/**
\brief
**/
static int iterIsNull ( const GtkTreeIter *iter )
{
#if 0
	g_print ( "%s\n", __FUNCTION__ );
#endif

	/* see setIterNull */
	return iter->stamp == 0 && iter->user_data == NULL;
}

/**
\brief
**/
static void setIterNull ( GtkTreeIter *iter )
{
#if 0
	g_print ( "%s\n", __FUNCTION__ );
#endif

	/* must conform to iterIsNull */
	iter->stamp = 0;
	iter->user_data = NULL;
}

/**
\brief
**/
static int iterFromTclPath ( Tcl_Interp *interp, Tcl_Obj *obj, GtkTreeModel *model, int allowEmpty, GtkTreeIter *iter )
{
#if 0
	g_print ( "%s\n", __FUNCTION__ );
#endif

	int len;
	/* use list function here to avoid conversion to string and back to list */

	if ( obj == NULL ||
			( Tcl_ListObjLength ( NULL, obj, &len ) == TCL_OK && len == 0 ) )
	{
		if ( allowEmpty )
		{
			setIterNull ( iter );
		}

		else
		{
			Tcl_SetResult ( interp, "Path must not be empty.", TCL_STATIC );
			return TCL_ERROR;
		}
	}

	else
	{
		GtkTreePath *path = tclPathToPath ( interp, obj, model );

		if ( path == NULL )
		{
			return TCL_ERROR;
		}

		if ( gtk_tree_model_get_iter ( model, iter, path ) == 0 )
		{
			gtk_tree_path_free ( path );
			Tcl_AppendResult ( interp, "Path \"", Tcl_GetString ( obj ), "\" is not valid.", ( char * ) NULL );
			return TCL_ERROR;
		}

		gtk_tree_path_free ( path );
	}

	return TCL_OK;
}

/**
\brief
**/
static int getRefFromTcl ( Tcl_Interp *interp, Tcl_Obj *obj, GHashTable *idToIter, GtkTreeIter *iter )
{
#if 0
	g_print ( "%s\n", __FUNCTION__ );
#endif

	/* use list function here to avoid conversion to string
	   and back to list if it is a path */
	int len;

	if ( obj != NULL
			&& Tcl_ListObjLength ( NULL, obj, &len ) == TCL_OK && len == 1 )
	{
		char *str = Tcl_GetString ( obj );

		if ( str[0] == refPrefix[0] &&  strncmp ( str + 1, refPrefix + 1, sizeof ( refPrefix ) - 2 ) == 0 )
		{
			int no = atoi ( str + sizeof ( refPrefix ) - 1 );
			gpointer *ret = g_hash_table_lookup ( idToIter, GINT_TO_POINTER ( no ) );

			if ( ret )
			{
				*iter = * ( GtkTreeIter * ) ret;
				return TCL_OK;
			}
		}
	}

	if ( interp )
	{
		Tcl_AppendResult ( interp, "Reference \"", Tcl_GetString ( obj ), "\" is not valid.", ( char * ) NULL );
	}

	return TCL_ERROR;
}

/**
\brief
**/
static int getIterFromTcl ( Tcl_Interp *interp, Tcl_Obj *obj, GtkTreeModel *model, GHashTable *idToIter, int allowEmpty, GtkTreeIter *iter )
{
#if 0
	g_print ( "%s\n", __FUNCTION__ );
#endif


	if ( getRefFromTcl ( NULL, obj, idToIter, iter ) != TCL_OK && iterFromTclPath ( interp, obj, model, allowEmpty, iter ) != TCL_OK )
	{
		Tcl_AppendResult ( interp, " \"", Tcl_GetString ( obj ), "\" is neiter a valid path nor a valid iterator.", ( char * ) NULL );
		return TCL_ERROR;
	}

	return TCL_OK;
}

typedef struct
{
	Tcl_Interp *interp;
	Tcl_Obj    *list;
} SelectionStruct;

/**
\brief
**/
static void getSelectionForeach ( GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer data )
{
#if 0
	g_print ( "%s\n", __FUNCTION__ );
#endif

	SelectionStruct *ss = ( SelectionStruct * ) data;
	Tcl_ListObjAppendElement ( ss->interp, ss->list, tclPathFromIter ( model, iter ) );
}

/**
\brief
**/
static Tcl_Obj *getSelectionList ( Tcl_Interp *interp, GtkTreeSelection *selection )
{
#if 0
	g_print ( "%s\n", __FUNCTION__ );
#endif

	SelectionStruct  ss;

	ss.interp = interp;
	ss.list = Tcl_NewListObj ( 0, NULL );
	Tcl_IncrRefCount ( ss.list );

	gtk_tree_selection_selected_foreach ( selection, getSelectionForeach, &ss );

	return ss.list;
}

/**
\brief
**/
static int doOnSelectionChanged ( Tcl_Interp *interp, TreeListParams *para, GtkTreeSelection *selection, const char *command, int background )
{
#if 0
	g_print ( "%s\n", __FUNCTION__ );
#endif

	GnoclPercSubst ps[] =
	{
		{ 'w', GNOCL_STRING },  /* widget */
		{ 'p', GNOCL_OBJ },     /* path */
		{ 0 }
	};
	int ret;

	ps[0].val.str = para->name;
	ps[1].val.obj = getSelectionList ( interp, selection );

	ret = gnoclPercentSubstAndEval ( interp, ps, command, background );

	Tcl_DecrRefCount ( ps[1].val.obj );

	return ret;
}

/**
\brief
**/
static void onSelectionChanged ( GtkTreeSelection *selection, gpointer data )
{
#if 0
	g_print ( "%s\n", __FUNCTION__ );
#endif

	GnoclCommandData *cs = ( GnoclCommandData * ) data;
	TreeListParams *para = ( TreeListParams * ) cs->data;

	doOnSelectionChanged ( cs->interp, para, selection, cs->command, 1 );
}

/**
\brief
**/
static void rowExpandCollapseFunc ( GtkTreeView *treeview, GtkTreeIter *iter, GtkTreePath *path, gpointer data )
{
	GnoclCommandData *cs = ( GnoclCommandData * ) data;
	TreeListParams *para = ( TreeListParams * ) cs->data;
	GtkTreeModel *model = gtk_tree_view_get_model ( para->view );

	GnoclPercSubst ps[] =
	{
		{ 'w', GNOCL_STRING },  /* widget */
		{ 'p', GNOCL_OBJ },     /* path */
		{ 0 }
	};

	assert ( para->view == treeview );

	ps[0].val.str = para->name;
	ps[1].val.obj = tclPathFromIter ( model, iter );
	gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 );
}

/**
\brief
**/
static GtkTreeViewColumn *getValidColumn ( GtkTreeView *view, Tcl_Interp *interp, Tcl_Obj *obj, int *pCol )
{
#if 0
	g_print ( "%s\n", __FUNCTION__ );
#endif

	GtkTreeViewColumn *column;
	int col;

	/* TODO? allow "end-x", range "1-4", list "1 3 4" */

	if ( Tcl_GetIntFromObj ( interp, obj, &col ) != TCL_OK )
		return NULL;

	if ( pCol )
		*pCol = col;

	column = gtk_tree_view_get_column ( view, col );

	if ( column == NULL )
		Tcl_SetResult ( interp, "column not in a valid range", TCL_STATIC );

	return column;
}

/**
\brief
**/
static int columnToNo ( GtkTreeView *view, GtkTreeViewColumn *column )
{
#if 0
	g_print ( "%s\n", __FUNCTION__ );
#endif
	int n = 0;
	GtkTreeViewColumn *col;

	do
	{
		col = gtk_tree_view_get_column ( view, n );

		if ( col == column )
			return n;

		++n;
	}

	while ( col != NULL );

	assert ( 0 );

	return 0;
}

/**
\brief
**/
static GdkPixbuf *getPixbuf ( Tcl_Interp *interp, GtkTreeView *view, Tcl_Obj *obj )
{
#if 0
	g_print ( "%s\n", __FUNCTION__ );
#endif

	GdkPixbuf *pixbuf = NULL;

	GnoclStringType type = gnoclGetStringType ( obj );

	switch ( type & ~GNOCL_STR_UNDERLINE )
	{
		case GNOCL_STR_FILE:
			{
				char *txt = gnoclGetString ( obj );
				GError *error = NULL;
				pixbuf = gdk_pixbuf_new_from_file ( txt, &error );

				if ( pixbuf == NULL )
				{
					Tcl_SetResult ( interp, error->message, TCL_VOLATILE );
					g_error_free ( error );
				}
			}

			break;
		case GNOCL_STR_STOCK:
			{
				/* TODO: how to change the size? */
				GtkIconSize size = GTK_ICON_SIZE_BUTTON;
				GtkStockItem item;

				if ( gnoclGetStockItem ( obj, interp, &item ) == TCL_OK )
					pixbuf = gtk_widget_render_icon ( GTK_WIDGET ( view ), item.stock_id, size, NULL );
			}

			break;
		default:
			Tcl_SetResult ( interp, "String must be either "
							"a file (\"%/\") or a stock (\"%#\") percent string",
							TCL_STATIC );
	}

	return pixbuf;
}

/**
\brief
**/
static int setCell ( Tcl_Interp *interp, GtkTreeView *view, GtkTreeIter *iter, int col, Tcl_Obj *obj, int isTree )
{
#if 0
	g_print ( "%s\n", __FUNCTION__ );
#endif

	GtkTreeModel *model = gtk_tree_view_get_model ( view );
	GType type = gtk_tree_model_get_column_type ( model, col );

	GValue value = { 0 };

	switch ( type )
	{
		case G_TYPE_BOOLEAN:
			{
				int n;

				if ( Tcl_GetBooleanFromObj ( interp, obj, &n ) != TCL_OK )
				{
					return TCL_ERROR;
				}

				g_value_init ( &value, G_TYPE_BOOLEAN );

				g_value_set_boolean ( &value, n );
			}

			break;
		case G_TYPE_UINT:
			{
				int n;

				if ( Tcl_GetBooleanFromObj ( interp, obj, &n ) != TCL_OK )
				{
					return TCL_ERROR;
				}

				g_value_init ( &value, G_TYPE_UINT );

				g_value_set_uint ( &value, n );
			}

			break;
		case G_TYPE_DOUBLE:
			{
				double n;

				if ( Tcl_GetDoubleFromObj ( interp, obj, &n ) != TCL_OK )
				{
					return TCL_ERROR;
				}

				g_value_init ( &value, G_TYPE_DOUBLE );

				g_value_set_double ( &value, n );
			}

			break;
		case G_TYPE_INT:
			{
				int n;

				if ( Tcl_GetIntFromObj ( interp, obj, &n ) != TCL_OK )
				{
					return TCL_ERROR;
				}

				g_value_init ( &value, G_TYPE_INT );

				g_value_set_int ( &value, n );
			}

			break;
		case G_TYPE_OBJECT: /* pixbuf */
			{
				GdkPixbuf *pixbuf = getPixbuf ( interp, view, obj );

				if ( pixbuf == NULL )
				{
					return TCL_ERROR;
				}

				g_value_init ( &value, G_TYPE_OBJECT );

				g_value_set_object ( &value, G_OBJECT ( pixbuf ) );
			}

			break;
		default:
			{
				g_value_init ( &value, G_TYPE_STRING );
				g_value_set_string ( &value, gnoclGetString ( obj ) );
			}
	}

	if ( isTree )
	{
		gtk_tree_store_set_value ( GTK_TREE_STORE ( model ), iter, col, &value );
	}

	else
	{

#if 0
		GtkCellRenderer *renderer;

		renderer = gtk_cell_renderer_text_new ();
		gtk_tree_model_get ( model, iter, COLOR, &text, -1 );
		/* WORKING HERE! change cell colour? */
		g_object_set ( renderer, "background", "#FF0000", "background-set", TRUE, NULL );
#endif

#if 0
		setCellBgClr ( GTK_LIST_STORE ( model ), iter, "#FF00FF" );
#endif

		gtk_list_store_set_value ( GTK_LIST_STORE ( model ), iter, col, &value );

	}

#if 0
	/* this is not much use */
	enum
	{
		COL_NAME = 0,
		COL_AGE,
		COL_COLOR,
		NUM_COLS
	};
	GtkCellRenderer *renderer;

	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes ( GTK_TREE_VIEW ( view ), -1, "Name", renderer, "text", COL_NAME, "foreground", COL_COLOR, NULL );
	gtk_list_store_set ( GTK_LIST_STORE ( model ), &iter, COL_NAME, "Jane Doe", col, 23, COL_COLOR, "red", -1 );
#endif

	g_value_unset ( &value );

	return TCL_OK;
}

/**
\brief
**/
static void setCellBgClr ( GtkTreeModel *model, GtkTreeIter *iter, gchar *clr )
{
#if 1
	g_print ( "%s %s\n", __FUNCTION__, clr );
#endif
	GtkCellRenderer *renderer;

	enum
	{
		COL_NAME = 0,
		COL_AGE,
		COL_COLOR,
		NUM_COLS
	};

	renderer = gtk_cell_renderer_text_new ();
	//gtk_tree_model_get (model, iter, COLOR, &text, -1);
	/* WORKING HERE! change cell colour? */
	g_object_set ( renderer, "background", clr, "background-set", TRUE, NULL );
	gtk_list_store_set ( GTK_LIST_STORE ( model ), iter, COL_COLOR, clr, -1 );

}

/**
\brief
\arguments
	TreeListParams *para
	Tcl_Interp *interp
	Tcl_Obj *child
	GtkTreeIter *parentIter
	int singleCol
	int insertPos
**/
static Tcl_Obj *insertRow ( TreeListParams *para, Tcl_Interp *interp, Tcl_Obj *child, GtkTreeIter *parentIter, int singleCol, int insertPos )
{
#if 0
	g_print ( "%s %d\n", __FUNCTION__, insertPos );
#endif

	GValue       value = { 0 };
	GtkTreeIter  iter;
	GtkTreeModel *model = gtk_tree_view_get_model ( para->view );
	int          nCol, col;

	switch ( insertPos )
	{
		case 0:
			{
				if ( para->isTree )
				{
					gtk_tree_store_prepend ( GTK_TREE_STORE ( model ), &iter, parentIter );
				}

				else
				{
					gtk_list_store_prepend ( GTK_LIST_STORE ( model ), &iter );
				}
			}
			break;
		case -1:
			{
				if ( para->isTree )
				{
					gtk_tree_store_append ( GTK_TREE_STORE ( model ), &iter, parentIter );
				}

				else
				{
					gtk_list_store_append ( GTK_LIST_STORE ( model ), &iter );
				}
			}
			break;
		default:
			{
				if ( para->isTree )
				{
					gtk_tree_store_append ( GTK_TREE_STORE ( model ), &iter, parentIter );
				}

				else
				{
					gtk_list_store_insert ( GTK_LIST_STORE ( model ), &iter, insertPos );
				}
			}
	}

	/*
		if ( insertPos )
		{
			if ( para->isTree )
			{
				gtk_tree_store_prepend ( GTK_TREE_STORE ( model ), &iter, parentIter );
			}

			else
			{
				gtk_list_store_prepend ( GTK_LIST_STORE ( model ), &iter );
			}
		}

		else
		{
			if ( para->isTree )
			{
				gtk_tree_store_append ( GTK_TREE_STORE ( model ), &iter, parentIter );
			}

			else
			{
				gtk_list_store_append ( GTK_LIST_STORE ( model ), &iter );

			}
		}
	*/

	g_value_init ( &value, G_TYPE_BOOLEAN );

	g_value_set_boolean ( &value, 1 );

	if ( singleCol )
	{
		nCol = 1;
	}

	else
	{
		if ( Tcl_ListObjLength ( interp, child, &nCol ) != TCL_OK  )
		{
			Tcl_SetResult ( interp, "row entry must be proper list", TCL_STATIC );
			return NULL;
		}

		if ( nCol > para->noColumns )
		{
			char buffer[1024];
			sprintf ( buffer,  "too many columns (%d) in row entry, max is %d", nCol, para->noColumns );
			Tcl_SetResult ( interp, buffer, TCL_VOLATILE );
			return NULL;
		}
	}

	for ( col = 0; col < nCol; ++col )
	{
		Tcl_Obj *tp;
		int offset;

		if ( singleCol )
		{
			tp = child;
		}

		else
		{
			if ( Tcl_ListObjIndex ( interp, child, col, &tp ) != TCL_OK )
			{
				return NULL;
			}
		}

		/* TODO: skip empty values? But what to do with string cols? */

		if ( setCell ( interp, para->view, &iter, col, tp, para->isTree ) != TCL_OK )
		{
			return NULL;
		}

		offset = getOffset ( para, CONFIG_VISIBLE, col );

		if ( para->isTree )
		{
			gtk_tree_store_set_value ( GTK_TREE_STORE ( model ), &iter, offset, &value );
		}

		else
		{
			gtk_list_store_set_value ( GTK_LIST_STORE ( model ), &iter, offset, &value );
		}
	}

	/* cells which value are not set are not visible */
	g_value_set_boolean ( &value, 0 );

	for ( ; col < para->noColumns; ++col )
	{
		int offset = getOffset ( para, CONFIG_VISIBLE, col );

		if ( para->isTree )
		{
			gtk_tree_store_set_value ( GTK_TREE_STORE ( model ), &iter, offset, &value );
		}

		else
		{
			gtk_list_store_set_value ( GTK_LIST_STORE ( model ), &iter, offset, &value );
		}
	}

	g_value_unset ( &value );

	return tclPathFromIter ( model, &iter );
}

/**
\brief
**/
static int addTreeChildren ( TreeListParams *para, Tcl_Interp *interp, Tcl_Obj *path, Tcl_Obj *children, int singleRow, int singleCol, int insertPos )
{
#if 0
	g_print ( "%s\n", __FUNCTION__ );
#endif
	GtkTreeModel *model = gtk_tree_view_get_model ( para->view );
	int          n, noChilds = 1;
	GtkTreeIter  parentIter;
	GtkTreeIter  *pIter;
	Tcl_Obj      *ret = NULL;

	if ( getIterFromTcl ( interp, path, model, para->idToIter, 1, &parentIter ) != TCL_OK )
	{
		return TCL_ERROR;
	}

	if ( iterIsNull ( &parentIter ) )
	{
		pIter = NULL;
	}

	else
	{
		pIter = &parentIter;
	}

	if ( singleRow == 0 )
	{
		if ( Tcl_ListObjLength ( interp, children, &noChilds ) != TCL_OK  )
		{
			Tcl_SetResult ( interp, "row-list must be proper list", TCL_STATIC );
			return TCL_ERROR;
		}

		ret = Tcl_NewListObj ( 0, NULL );
	}

	for ( n = 0; n < noChilds; ++n )
	{
		Tcl_Obj *tp;

		if ( singleRow )
		{
			tp = children;
		}

		else if ( Tcl_ListObjIndex ( interp, children, n, &tp ) != TCL_OK )
		{
			goto errorExit;
		}

		tp = insertRow ( para, interp, tp, pIter, singleCol, insertPos );

		if ( tp == NULL )
		{
			goto errorExit;
		}

		if ( singleRow )
		{
			ret = tp;
		}

		else
		{
			Tcl_ListObjAppendElement ( interp, ret, tp );
		}
	}

	Tcl_SetObjResult ( interp, ret );

#if 0
	/* set optimal column width */

	for ( n = 0; n < noColumns; ++n )
	{
		gtk_list_set_column_width ( list, n, gtk_list_optimal_column_width ( list, n ) );
	}

#endif

	return TCL_OK;

errorExit:

	if ( ret )
	{
		Tcl_DecrRefCount ( ret );
	}

	return TCL_ERROR;
}

/**
\brief
\arguments
	TreeListParams *para
	Tcl_Interp *interp
	Tcl_Obj *children
	int singleRow
	int singleCol
	int insertPos					flag, 0 = beginning, -1 = end
**/
static int addListChildren ( TreeListParams *para, Tcl_Interp *interp, Tcl_Obj *children, int singleRow, int singleCol, int insertPos )
{
#if 0
	g_print ( "%s\n", __FUNCTION__ );
#endif

	int     n, noChilds = 1;
	Tcl_Obj *ret = NULL;

	if ( singleRow == 0 )
	{
		if ( Tcl_ListObjLength ( interp, children, &noChilds ) != TCL_OK  )
		{
			Tcl_SetResult ( interp, "row-list must be proper list", TCL_STATIC );
			return TCL_ERROR;
		}

		ret = Tcl_NewListObj ( 0, NULL );
	}

	for ( n = 0; n < noChilds; ++n )
	{
		Tcl_Obj *tp;

		if ( singleRow )
		{
			tp = children;
		}

		else if ( Tcl_ListObjIndex ( interp, children, n, &tp ) != TCL_OK )
		{
			goto errorExit;
		}

		tp = insertRow ( para, interp, tp, NULL, singleCol, insertPos );

		if ( tp == NULL )
		{
			goto errorExit;
		}

		if ( singleRow )
		{
			ret = tp;
		}

		else
		{
			Tcl_ListObjAppendElement ( interp, ret, tp );
		}
	}

#if 0

	/* set optimal column width */
	for ( n = 0; n < noColumns; ++n )
	{
		gtk_list_set_column_width ( list, n, gtk_list_optimal_column_width ( list, n ) );
	}

#endif
	Tcl_SetObjResult ( interp, ret );
	return TCL_OK;

errorExit:

	if ( ret )
	{
		Tcl_DecrRefCount ( ret );
	}

	return TCL_ERROR;
}

/**
\brief
\author     Peter G Baum, William J Giddings
\date
**/
static int cget ( Tcl_Interp *interp, TreeListParams *para, GnoclOption options[], int idx )
{
#ifdef DEBUG_LABEL
	g_print ( "label/staticFuncs/cget\n" );
#endif

	Tcl_Obj *obj = NULL;

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
\brief      Configure the GtkTreeView widget.
**/
static int configure ( Tcl_Interp *interp,	TreeListParams *para,	GnoclOption options[] )
{
#if 0
	g_print ( "%s %s \n", __FUNCTION__, options[1] );
#endif


	if ( gnoclSetOptions ( interp, options, G_OBJECT ( para->view ), -1 ) != TCL_OK )
	{
		return TCL_ERROR;
	}

	if ( options[dataIdx].status == GNOCL_STATUS_CHANGED )
	{
#if 0
		g_print ( "set data =%s\n", options[dataIdx].val.str );
#endif
		para->data = strdup ( options[dataIdx].val.str );
	}

	if ( options[selectionModeIdx].status == GNOCL_STATUS_CHANGED )
	{
		GtkTreeSelection *select;
		GtkSelectionMode mode;

		if ( gnoclGetSelectionMode ( interp, options[selectionModeIdx].val.obj, &mode ) != TCL_OK )
		{
			return TCL_ERROR;
		}

		select = gtk_tree_view_get_selection ( para->view );

		gtk_tree_selection_set_mode ( select, mode );
	}

	if ( options[titlesIdx].status == GNOCL_STATUS_CHANGED )
	{
		Tcl_Obj *obj = options[titlesIdx].val.obj;
		int nCol, n;

		Tcl_ListObjLength ( interp, obj, &nCol );

		if ( nCol > para->noColumns )
		{
			char buffer[1024];
			sprintf ( buffer,  "too many titles (%d), max is %d", nCol, para->noColumns );
			Tcl_SetResult ( interp, buffer, TCL_VOLATILE );
			return TCL_ERROR;
		}

		for ( n = 0; n < nCol; ++n )
		{
			Tcl_Obj *tp;
			GtkTreeViewColumn *column;

			if ( Tcl_ListObjIndex ( interp, obj, n, &tp ) != TCL_OK )
			{
				return TCL_ERROR;
			}

			column = gtk_tree_view_get_column ( para->view, n );

			gtk_tree_view_column_set_title ( column, gnoclGetString ( tp ) );

		}
	}

	if ( options[childrenIdx].status == GNOCL_STATUS_CHANGED )
	{
		GtkTreeModel *model = gtk_tree_view_get_model ( para->view );

		if ( para->isTree )
		{
			gtk_tree_store_clear ( GTK_TREE_STORE ( model ) );

			if ( addTreeChildren ( para, interp, NULL, options[childrenIdx].val.obj, 0, 0, 0 ) != TCL_OK )
				return TCL_ERROR;
		}

		else
		{
			gtk_list_store_clear ( GTK_LIST_STORE ( model ) );

			if ( addListChildren ( para, interp, options[childrenIdx].val.obj, 0, 0, 0 ) != TCL_OK )
				return TCL_ERROR;
		}
	}

	if ( gnoclConnectOptCmd ( interp,
							  G_OBJECT ( gtk_tree_view_get_selection ( para->view ) ), "changed",
							  G_CALLBACK ( onSelectionChanged ), options + onSelectionChangedIdx,
							  para, NULL ) != TCL_OK )
	{
		return TCL_ERROR;
	}

	if ( gnoclConnectOptCmd ( interp, G_OBJECT ( para->view ), "row-expanded",
							  G_CALLBACK ( rowExpandCollapseFunc ), options + onRowExpandedIdx,
							  para, NULL ) != TCL_OK )
	{
		return TCL_ERROR;
	}

	if ( gnoclConnectOptCmd ( interp, G_OBJECT ( para->view ), "row-collapsed",
							  G_CALLBACK ( rowExpandCollapseFunc ), options + onRowCollapsedIdx,
							  para, NULL ) != TCL_OK )
	{
		return TCL_ERROR;
	}

	if ( options[scrollbarIdx].status == GNOCL_STATUS_CHANGED )
	{
		GtkPolicyType hor, vert;

		if ( gnoclGetScrollbarPolicy ( interp, options[scrollbarIdx].val.obj, &hor, &vert ) != TCL_OK )
		{
			return TCL_ERROR;
		}

		gtk_scrolled_window_set_policy ( para->scrollWin, hor, vert );
	}

	if ( options[visibleIdx].status == GNOCL_STATUS_CHANGED )
	{
		g_object_set ( G_OBJECT ( para->scrollWin ), "visible", options[visibleIdx].val.b, NULL );
	}

	/*
		if ( 1 )
		{
			if ( options[reorderableIdx].status == GNOCL_STATUS_CHANGED )
			{
				// this value can be toggled, get initial state
				gtk_tree_view_set_reorderable  ( GTK_TREE_VIEW ( para->view ), 1 ) ;
			}
		}
	*/
	return TCL_OK;
}

/**
\brief
**/
static int setSelection ( TreeListParams *para, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{
#if 0
	g_print ( "%s\n", __FUNCTION__ );
#endif

	GnoclOption options[] =
	{
		{ "-add", GNOCL_BOOL, NULL },       /* 0 */
		{ "-single", GNOCL_BOOL, NULL },    /* 1 */
		{ "-unselect", GNOCL_BOOL, NULL },  /* 2 */
		{ NULL }
	};
	const int addIdx = 0;
	const int singleIdx = 1;
	const int unselectIdx = 2;

	GtkTreeSelection *select;
	GtkTreeModel     *model = gtk_tree_view_get_model ( para->view );
	int              clear = 1;
	int              single = 0;
	int              unselect = 0;
	int              ret = TCL_ERROR;

	if ( objc < 3 )
	{
		Tcl_WrongNumArgs ( interp, 2, objv, "pathOrReference-list" );
		return TCL_ERROR;
	}

	if ( gnoclParseOptions ( interp, objc - 2, objv + 2, options ) != TCL_OK )
	{
		gnoclClearOptions ( options );
		{
			return TCL_ERROR;
		}
	}

	if ( options[addIdx].status == GNOCL_STATUS_CHANGED )
	{
		clear = ( options[addIdx].val.b == 0 );
	}

	if ( options[singleIdx].status == GNOCL_STATUS_CHANGED )
	{
		single = options[singleIdx].val.b;
	}

	if ( options[unselectIdx].status == GNOCL_STATUS_CHANGED )
	{
		unselect = options[unselectIdx].val.b;
	}

	gnoclClearOptions ( options );

	select = gtk_tree_view_get_selection ( para->view );

	g_signal_handlers_block_matched ( G_OBJECT ( select ), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, ( gpointer * ) onSelectionChanged, NULL );

	if ( clear )
	{
		if ( unselect )
		{
			gtk_tree_selection_select_all ( select );
		}

		else
		{
			gtk_tree_selection_unselect_all ( select );
		}
	}

	if ( single )
	{
		GtkTreeIter iter;

		if ( getIterFromTcl ( interp, objv[2], model, para->idToIter, 1, &iter ) != TCL_OK )
		{
			goto cleanExit;
		}

		if ( unselect )
		{
			gtk_tree_selection_unselect_iter ( select, &iter );
		}

		else
		{
			gtk_tree_selection_select_iter ( select, &iter );
		}
	}

	else
	{
		int  k, no;

		if ( Tcl_ListObjLength ( interp, objv[2], &no ) != TCL_OK )
		{
			goto cleanExit;
		}

		if ( no == 1 && strcmp ( Tcl_GetString ( objv[2] ), "all" ) == 0 )
		{
			if ( unselect )
			{
				gtk_tree_selection_unselect_all ( select );
			}

			else
			{
				gtk_tree_selection_select_all ( select );
			}
		}

		else
			for ( k = 0; k < no; ++k )
			{
				Tcl_Obj     *tp;
				GtkTreeIter iter;

				if ( Tcl_ListObjIndex ( interp, objv[2], k, &tp ) != TCL_OK  )
				{
					goto cleanExit;
				}

				if ( getIterFromTcl ( interp, tp, model, para->idToIter, 1, &iter ) != TCL_OK )
				{
					goto cleanExit;
				}

				if ( unselect )
				{
					gtk_tree_selection_unselect_iter ( select, &iter );
				}

				else
				{
					gtk_tree_selection_select_iter ( select, &iter );
				}
			}
	}

	ret = TCL_OK;

cleanExit:
	g_signal_handlers_unblock_matched ( G_OBJECT ( select ), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, ( gpointer * ) onSelectionChanged, NULL );

	return ret;
}

/**
\brief
**/
static int getSelection ( TreeListParams *pp, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{
#if 0
	g_print ( "%s\n", __FUNCTION__ );
#endif

	if ( objc != 2 )
	{
		Tcl_WrongNumArgs ( interp, 2, objv, "" );
		return TCL_ERROR;
	}

	Tcl_SetObjResult ( interp, getSelectionList ( interp, gtk_tree_view_get_selection ( pp->view ) ) );

	return TCL_OK;
}


/**
\brief
**/
static gboolean iterIsEqual ( GtkTreeIter *a, GtkTreeIter *b )
{
#if 0
	g_print ( "%s\n", __FUNCTION__ );
#endif

	return a->user_data == b->user_data &&
		   a->user_data2 == b->user_data2 &&
		   a->user_data3 == b->user_data3;
}

/**
\brief
**/
static gboolean listRemoveIter ( gpointer key, gpointer value, gpointer data )
{
#if 0
	g_print ( "%s\n", __FUNCTION__ );
#endif

	return iterIsEqual ( ( GtkTreeIter * ) value, ( GtkTreeIter * ) data );
}

typedef struct
{
	GtkTreeStore *model;
	GtkTreeIter  *iter;
} TreeRemovePara;

/**
\brief
**/
static gboolean treeRemoveIter ( gpointer key, gpointer value, gpointer data )
{
#if 0
	g_print ( "%s\n", __FUNCTION__ );
#endif

	TreeRemovePara *p = ( TreeRemovePara * ) data;
	GtkTreeIter    *v = ( GtkTreeIter * ) value;

	return iterIsEqual ( v, p->iter )
		   || gtk_tree_store_is_ancestor ( p->model, p->iter, v );
}

/**
\brief
**/
static int storeRemove ( TreeListParams *para, Tcl_Interp *interp, GtkTreeModel *model, GtkTreeIter *iter )
{
#if 0
	g_print ( "%s\n", __FUNCTION__ );
#endif

	/* since "row-deleted" is only called for the deleted row and not
	   for its children, we have to be very ugly */
	if ( para->isTree )
	{
		TreeRemovePara pp;
		pp.model = GTK_TREE_STORE ( model );
		pp.iter = iter;
		g_hash_table_foreach_remove ( para->idToIter, treeRemoveIter, &pp );
		gtk_tree_store_remove ( GTK_TREE_STORE ( model ), iter );
	}

	else
	{
		g_hash_table_foreach_remove ( para->idToIter, listRemoveIter, iter );
		gtk_list_store_remove ( GTK_LIST_STORE ( model ), iter );
	}

	return TCL_OK;
}

/**
\brief
**/
static int erase ( TreeListParams *para, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{
#if 0
	g_print ( "%s\n", __FUNCTION__ );
#endif

	GtkTreeModel *model = gtk_tree_view_get_model ( para->view );
	GtkTreeIter start;

	if ( objc < 3 || objc > 4 )
	{
		Tcl_WrongNumArgs ( interp, 2, objv,
						   "startPathOrReference ?endPathOrReference?" );
		return TCL_ERROR;
	}

	/* erasing anything in an empty model always succeeds */

	if ( gtk_tree_model_get_iter_first ( model, &start ) == 0 )
	{
		return TCL_OK;
	}

	if ( getIterFromTcl ( interp, objv[2], model, para->idToIter, 0, &start ) != TCL_OK )
	{
		return TCL_ERROR;
	}

	if ( objc > 3 )
	{
		GtkTreeIter iter;
		GtkTreeIter end;

		/* "end" is the last row with the same depth */

		if ( strcmp ( Tcl_GetString ( objv[3] ), "end" ) == 0 )
		{
			end = start;
			iter = start;

			while ( gtk_tree_model_iter_next ( model, &iter )  )
			{
				end = iter;
			}
		}

		else
		{
			GtkTreeIter startParent;
			GtkTreeIter endParent;
			gboolean    stp, endp;

			if ( getIterFromTcl ( interp, objv[3], model, para->idToIter, 0, &end ) != TCL_OK )
			{
				return TCL_ERROR;
			}

			stp = gtk_tree_model_iter_parent ( model, &startParent, &start );

			endp = gtk_tree_model_iter_parent ( model, &endParent, &end );

			if ( stp != endp || ( stp && endp && !iterIsEqual ( &startParent, &endParent ) ) )
			{
				Tcl_SetResult ( interp, "Paths or references must have the same depth.", TCL_STATIC );
				return TCL_ERROR;
			}

			iter = start;

			while ( !iterIsEqual ( &iter, &end ) )
			{
				if ( !gtk_tree_model_iter_next ( model, &iter ) )
				{
					Tcl_SetResult ( interp, "End reference or path is not before begin.", TCL_STATIC );
					return TCL_ERROR;
				}
			}
		}

		for ( iter = start; !iterIsEqual ( &iter, &end ); start = iter )
		{
			gtk_tree_model_iter_next ( model, &iter );

			if ( storeRemove ( para, interp, model, &start ) != TCL_OK )
			{
				return TCL_ERROR;
			}
		}

		start = iter;
	}

	if ( storeRemove ( para, interp, model, &start ) != TCL_OK )
	{
		return TCL_ERROR;
	}

	return TCL_OK;
}

/**
\brief
**/
static int scrollToPos ( TreeListParams *para, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{
#if 0
	g_print ( "%s\n", __FUNCTION__ );
#endif

	GnoclOption options[] =
	{
		{ "-align", GNOCL_OBJ, NULL },       /* 0 */
		{ "-path", GNOCL_OBJ, NULL },        /* 1 */
		{ "-column", GNOCL_OBJ, NULL },      /* 2 */
		{ NULL }
	};
	const int alignIdx  = 0;
	const int pathIdx   = 1;
	const int columnIdx = 2;

	GtkTreeModel      *model = gtk_tree_view_get_model ( para->view );
	int               useAlign = 0;
	gfloat            xAlign = 0.5,
					  yAlign = 0.5;
	GtkTreePath       *path = NULL;
	GtkTreeViewColumn *column = NULL;
	int               ret = TCL_ERROR;

	if ( gnoclParseOptions ( interp, objc - 1, objv + 1, options )  != TCL_OK )
		goto cleanExit;

	if ( options[alignIdx].status == GNOCL_STATUS_CHANGED )
	{
		if ( gnoclGetBothAlign ( interp, options[alignIdx].val.obj, &xAlign, &yAlign ) != TCL_OK )
			goto cleanExit;

		useAlign = 1;
	}

	if ( options[pathIdx].status == GNOCL_STATUS_CHANGED )
	{
		path = tclPathToPath ( interp, options[pathIdx].val.obj, model );

		if ( path == NULL )
			goto cleanExit;
	}

	if ( options[columnIdx].status == GNOCL_STATUS_CHANGED )
	{
		column = getValidColumn ( para->view, interp, options[columnIdx].val.obj, NULL );

		if ( column == NULL )
		{
			goto cleanExit;
		}
	}

	if ( path == NULL && column == NULL )
	{
		GtkTreeIter iter;

		if ( getLastVisibleIter ( para->view, model, &iter ) == TCL_OK )
		{
			path = gtk_tree_model_get_path ( model, &iter );
		}

		else
		{
			path = gtk_tree_path_new_first( );
		}
	}

	gtk_tree_view_scroll_to_cell ( para->view, path, column, useAlign, xAlign, yAlign );

	gtk_tree_path_free ( path );

	ret = TCL_OK;

cleanExit:
	gnoclClearOptions ( options );

	return ret;
}

/**
\brief
**/
static int collapse ( TreeListParams * para, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[], int doCollapse )
{
#if 0
	g_print ( "%s\n", __FUNCTION__ );
#endif
	GnoclOption options[] =
	{
		{ "-path", GNOCL_OBJ, NULL },       /* 0 */
		{ "-recursive", GNOCL_BOOL, NULL }, /* 1 */
		{ NULL }
	};
	const int pathIdx      = 0;
	const int recursiveIdx = 1;
	int   ret = TCL_ERROR;

	GtkTreeModel *model = gtk_tree_view_get_model ( para->view );
	GtkTreePath  *path;
	int          recursive = 1;

	if ( gnoclParseOptions ( interp, objc - 1, objv + 1, options )  != TCL_OK )
		goto cleanExit;

	if ( options[recursiveIdx].status == GNOCL_STATUS_CHANGED )
		recursive = options[recursiveIdx].val.b;

	if ( options[pathIdx].status == GNOCL_STATUS_CHANGED )
	{
		path = tclPathToPath ( interp, options[pathIdx].val.obj, model );

		if ( path == NULL )
			goto cleanExit;
	}

	else
		path = gtk_tree_path_new_first();

	if ( doCollapse )
		ret = gtk_tree_view_collapse_row ( para->view, path );
	else
		ret = gtk_tree_view_expand_row ( para->view, path, recursive );

	gtk_tree_path_free ( path );

	Tcl_SetObjResult ( interp, Tcl_NewIntObj ( ret ) );

	ret = TCL_OK;

cleanExit:
	gnoclClearOptions ( options );

	return ret;
}

/**
\brief
**/
static int getNumChildren ( TreeListParams * para, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[] )
{
#if 0
	g_print ( "%s\n", __FUNCTION__ );
#endif
	int n;
	GtkTreeModel *model = gtk_tree_view_get_model ( para->view );
	GtkTreeIter  iter;

	if ( objc != 2 && objc != 3 )
	{
		Tcl_WrongNumArgs ( interp, 2, objv, "?pathOrReference?" );
		return TCL_ERROR;
	}

	if ( objc == 3 )
	{
		if ( getIterFromTcl ( interp, objv[2], model, para->idToIter,  1, &iter ) != TCL_OK )
		{
			return TCL_ERROR;
		}
	}

	if ( objc == 2 || iterIsNull ( &iter ) )
	{
		n = gtk_tree_model_iter_n_children ( model, NULL );
	}

	else
	{
		n = gtk_tree_model_iter_n_children ( model, &iter );
	}

	Tcl_SetObjResult ( interp, Tcl_NewIntObj ( n ) );

	return TCL_OK;
}

/**
\brief
**/
static int coordsToPath ( TreeListParams * para, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[] )
{
#if 0
	g_print ( "%s\n", __FUNCTION__ );
#endif

	int               x, y;
	GtkTreePath       *path;
	GtkTreeViewColumn *column;
	gint              cellX, cellY;
	Tcl_Obj           *res;

	if ( objc != 4 )
	{
		Tcl_WrongNumArgs ( interp, 2, objv, "x y" );
		return TCL_ERROR;
	}

	if ( Tcl_GetIntFromObj ( interp, objv[2], &x ) != TCL_OK || Tcl_GetIntFromObj ( interp, objv[3], &y ) != TCL_OK )
	{
		return TCL_ERROR;
	}

	gtk_tree_view_get_path_at_pos ( para->view, x, y, &path, &column, &cellX, &cellY );

	/* return list:
	      - path
	      - colNumber
	      - cellX
	      - cellY
	*/
	res = Tcl_NewListObj ( 0, NULL );

	if ( path == NULL )
	{
		Tcl_ListObjAppendElement ( NULL, res, Tcl_NewStringObj ( "", 0 ) );
		Tcl_ListObjAppendElement ( NULL, res, Tcl_NewStringObj ( "", 0 ) );
		Tcl_ListObjAppendElement ( NULL, res, Tcl_NewStringObj ( "", 0 ) );
		Tcl_ListObjAppendElement ( NULL, res, Tcl_NewStringObj ( "", 0 ) );
	}

	else
	{
		Tcl_ListObjAppendElement ( NULL, res, tclPathFromPath ( path ) );
		Tcl_ListObjAppendElement ( NULL, res, Tcl_NewIntObj ( columnToNo ( para->view, column ) ) );
		Tcl_ListObjAppendElement ( NULL, res, Tcl_NewIntObj ( cellX ) );
		Tcl_ListObjAppendElement ( NULL, res, Tcl_NewIntObj ( cellY ) );
	}

	Tcl_SetObjResult ( interp, res );

	return TCL_OK;
}

/**
\brief
**/
static int setCursor ( TreeListParams * para, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[] )
{
#if 0
	g_print ( "%s\n", __FUNCTION__ );
#endif

	GnoclOption options[] =
	{
		{ "-column", GNOCL_INT, NULL },       /* 0 */
		{ "-startEdit", GNOCL_BOOL, NULL },   /* 1 */
		{ NULL }
	};

	const int columnIdx    = 0;
	const int startEditIdx = 1;

	int               ret = TCL_ERROR;
	GtkTreeModel      *model = gtk_tree_view_get_model ( para->view );
	GtkTreePath       *path = NULL;
	GtkTreeViewColumn *column = NULL;
	gboolean          startEdit = 0;

	if ( objc < 3 )
	{
		Tcl_WrongNumArgs ( interp, 2, objv, "path ?option val ...?" );
		return TCL_ERROR;
	}

	path = tclPathToPath ( interp, objv[2], model );

	if ( path == NULL )
	{
		return TCL_ERROR;
	}

	if ( gnoclParseOptions ( interp, objc - 2, objv + 2, options )  != TCL_OK )
	{
		goto cleanExit;
	}

	if ( options[startEditIdx].status == GNOCL_STATUS_CHANGED )
	{
		startEdit = options[startEditIdx].val.b;
	}

	if ( options[columnIdx].status == GNOCL_STATUS_CHANGED )
	{
		int col = options[columnIdx].val.i;
		column = gtk_tree_view_get_column ( para->view, col );
	}

	gtk_tree_view_set_cursor ( para->view, path, column, startEdit );

	ret = TCL_OK;

cleanExit:
	gnoclClearOptions ( options );
	gtk_tree_path_free ( path );

	return ret;
}

/**
\brief
**/
static void defaultEditedFunc ( GtkCellEditable * cell, gchar * arg1, gchar * arg2, gpointer data )
{
#if 0
	g_print ( "%s\n", __FUNCTION__ );
#endif

	/* Does this have to be really so complicated? */
	/* g_object_set( G_OBJECT( cell ), "text", arg2, NULL ); */
	GtkTreeModel *model = ( GtkTreeModel * ) data;
	GtkTreePath  *path = gtk_tree_path_new_from_string ( arg1 );
	int          column = getColumn ( GTK_CELL_RENDERER ( cell ) );
	GtkTreeIter  iter;

	gtk_tree_model_get_iter ( model, &iter, path );
	gtk_list_store_set ( GTK_LIST_STORE ( model ), &iter, column, arg2, -1 );
	gtk_tree_path_free ( path );
}

/**
\brief
**/
static void editedFunc ( GtkCellEditable * cell, gchar * arg1, gchar * arg2, gpointer data )
{
#if 0
	g_print ( "%s\n", __FUNCTION__ );
#endif

	GnoclCommandData *cs = ( GnoclCommandData * ) data;
	TreeListParams   *para = ( TreeListParams * ) cs->data;

	GnoclPercSubst ps[] =
	{
		{ 'w', GNOCL_STRING },  /* widget */
		{ 'p', GNOCL_STRING },  /* path */
		{ 'v', GNOCL_STRING },  /* new value */
		{ 'c', GNOCL_INT },     /* col no */
		{ 0 }
	};

	ps[0].val.str = para->name;
	ps[1].val.str = stringPathToTclPath ( arg1 );
	ps[2].val.str = arg2;
	ps[3].val.i = getColumn ( GTK_CELL_RENDERER ( cell ) );

	gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 );
	g_free ( ( char * ) ps[1].val.str );
}

/**
\brief
**/
static void defaultToggledFunc ( GtkCellRendererToggle * cell, gchar * arg1, gpointer data )
{
#if 0
	g_print ( "%s\n", __FUNCTION__ );
#endif

	/* Does this have to be really so complicated? */
	GtkTreeModel *model = ( GtkTreeModel * ) data;
	GtkTreePath  *path = gtk_tree_path_new_from_string ( arg1 );
	int          column = getColumn ( GTK_CELL_RENDERER ( cell ) );
	GtkTreeIter  iter;
	gboolean     val;

	gtk_tree_model_get_iter ( model, &iter, path );
	gtk_tree_model_get ( model, &iter, column, &val, -1 );

	gtk_list_store_set ( GTK_LIST_STORE ( model ), &iter, column, !val, -1 );

	gtk_tree_path_free ( path );
}

/**
\brief
**/
static void toggledFunc ( GtkCellRendererToggle * cell, gchar * arg1, gpointer data )
{
	GnoclCommandData *cs = ( GnoclCommandData * ) data;
	TreeListParams   *para = ( TreeListParams * ) cs->data;

	GnoclPercSubst ps[] =
	{
		{ 'w', GNOCL_STRING },  /* widget */
		{ 'p', GNOCL_STRING },  /* path */
		{ 'c', GNOCL_INT },     /* col no */
		{ 0 }
	};
	ps[0].val.str = para->name;
	ps[1].val.str = stringPathToTclPath ( arg1 );
	ps[2].val.i = getColumn ( GTK_CELL_RENDERER ( cell ) );

	gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 );
	g_free ( ( char * ) ps[1].val.str );
}

/**
\brief
**/
static int appendOptions ( GnoclOption * dest, GnoclOption * src )
{
#if 0
	g_print ( "%s\n", __FUNCTION__ );
#endif
	GnoclOption *dp = dest;
	GnoclOption *sp = src;

	while ( dp->optName )
	{
		++dp;
	}

	while ( sp->optName )
	{
		++sp;
	}

	/* copy also the final NULL */
	memcpy ( dp, src, ( sp - src + 1 ) * sizeof ( *src ) );

	return dp - dest + sp - src;
}

/**
\brief
**/
static int appendOneOption ( GnoclOption * dest, GnoclOption * src )
{
#if 0
	g_print ( "%s\n", __FUNCTION__ );
#endif
	GnoclOption *dp = dest;

	while ( dp->optName )
	{
		++dp;
	}

	*dp = *src;

	dp[1].optName = NULL;

	return dp - dest + 1;
}

/**
\brief
**/
static void cellDataFunc ( GtkTreeViewColumn * column, GtkCellRenderer * renderer, GtkTreeModel * model, GtkTreeIter * iter, gpointer data )
{
#if 0
	g_print ( "%s\n", __FUNCTION__ );
#endif

	GnoclCommandData *cs = ( GnoclCommandData * ) data;
	TreeListParams   *para = ( TreeListParams * ) cs->data;
	int              colNo = getColumn ( renderer );

	GnoclPercSubst ps[] =
	{
		{ 'w', GNOCL_STRING },    /* widget */
		{ 'v', GNOCL_OBJ },       /* value */
		{ 'p', GNOCL_OBJ },       /* path */
		{ 'c', GNOCL_INT },       /* column */
		{ 0 }
	};

	ps[0].val.str = para->name;
	ps[1].val.obj = getObjFromCol ( model, colNo, iter );
	ps[2].val.obj = tclPathFromIter ( model, iter );
	ps[3].val.i = colNo;

	if ( gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 ) == TCL_OK )
	{
		Tcl_Obj *res = Tcl_GetObjResult ( cs->interp );
		Tcl_Obj **objv;
		int k, no, ret;
		typedef Tcl_Obj *ObjPointer;

		int isText = GTK_IS_CELL_RENDERER_TEXT ( renderer );
		int noOpt = noCellRenderOptions + ( isText ?  noTextRenderOptions : noToggleRenderOptions ) + 1;
		GnoclOption *options;

		if ( Tcl_ListObjLength ( cs->interp, res, &no ) != TCL_OK )
		{
			Tcl_BackgroundError ( cs->interp );
		}

		options = g_new ( GnoclOption, noOpt );

		options->optName = NULL;

		appendOptions ( options, cellRenderOptions );

		if ( isText )
		{
			appendOptions ( options, textRenderOptions );
		}

		else
		{
			appendOptions ( options, toggleRenderOptions );
		}

		objv = g_new ( ObjPointer, no + 1 );

		objv[0] = NULL;

		for ( k = 0; k < no; ++k )
		{
			Tcl_ListObjIndex ( cs->interp, res, k, &objv[k+1] );
		}

		ret = gnoclParseOptions ( cs->interp, no + 1, objv, options );

		g_free ( objv );

		if ( ret != TCL_OK )
		{
			goto cleanExit;
		}

		if ( gnoclSetOptions ( cs->interp, options, G_OBJECT ( renderer ), -1 ) != TCL_OK )
		{
			goto cleanExit;
		}

		gnoclClearOptions ( options );

		g_free ( options );

		return;

cleanExit:
		gnoclClearOptions ( textRenderOptions );

		g_free ( options );

		Tcl_BackgroundError ( cs->interp );
	}
}

/**
\brief
**/
static int columnCget ( TreeListParams * para, Tcl_Interp * interp,	int objc, Tcl_Obj * const objv[] )
{
#if 0
	g_print ( "%s\n", __FUNCTION__ );
#endif
	GtkTreeViewColumn *column;
	int idx;

	if ( objc < 3 )
	{
		Tcl_WrongNumArgs ( interp, 2, objv, "column option" );
		return TCL_ERROR;
	}

	column = getValidColumn ( para->view, interp, objv[2], NULL );

	if ( column == NULL )
	{
		return TCL_ERROR;
	}

	switch ( gnoclCget ( interp, objc - 1, objv + 1, G_OBJECT ( column ), colOptions, &idx ) )
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

			if ( idx == widthIdx )
			{
				int width;
				g_object_get ( G_OBJECT ( column ), "width", &width, NULL );
				Tcl_SetObjResult ( interp, Tcl_NewIntObj ( width ) );
			}

			else
			{
				assert ( 0 );
				return gnoclCgetNotImplemented ( interp, colOptions + idx );
			}
	}

	return TCL_OK;
}

/**
\brief
**/
static int columnConfigure ( TreeListParams * para, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[] )
{
#if 0
	g_print ( "%s\n", __FUNCTION__ );
#endif

	const int noColOptions = sizeof ( colOptions ) / sizeof ( GnoclOption );
	int       onCmdIdx = -1;
	int       startRenderOptions;

	GtkTreeViewColumn *column;
	GList             *rendList;
	GtkCellRenderer   *renderer;
	int               colNo;
	GnoclOption       *options;
	int               ret = TCL_ERROR;

	if ( objc < 3 )
	{
		Tcl_WrongNumArgs ( interp, 2, objv, "column ?option val ...?" );
		return TCL_ERROR;
	}

	column = getValidColumn ( para->view, interp, objv[2], &colNo );

	if ( column == NULL )
	{
		return TCL_ERROR;
	}

	rendList = gtk_tree_view_column_get_cell_renderers ( column );

	renderer = ( GtkCellRenderer * ) rendList->data;

	assert ( rendList->next == NULL || rendList->next->data == NULL );

	g_list_free ( rendList );

	/* put the possible options together */
	if ( GTK_IS_CELL_RENDERER_TEXT ( renderer ) )
	{
		GnoclOption optTxt = { "-onEdited", GNOCL_OBJ, NULL };
		options = g_new ( GnoclOption, noColOptions + noCellRenderOptions + noTextRenderOptions + 2 );
		options->optName = NULL;
		appendOptions ( options, colOptions );
		startRenderOptions = appendOneOption ( options, &optTxt );
		onCmdIdx = startRenderOptions - 1;
		appendOptions ( options, cellRenderOptions );
		appendOptions ( options, textRenderOptions );
	}

	else if ( GTK_IS_CELL_RENDERER_TOGGLE ( renderer ) )
	{
		GnoclOption optTgl = { "-onToggled", GNOCL_OBJ, NULL };
		options = g_new ( GnoclOption, noColOptions + noCellRenderOptions
						  + noToggleRenderOptions + 2 );
		options->optName = NULL;
		appendOptions ( options, colOptions );
		startRenderOptions = appendOneOption ( options, &optTgl );
		onCmdIdx = startRenderOptions - 1;
		appendOptions ( options, cellRenderOptions );
		appendOptions ( options, toggleRenderOptions );
	}


	else
	{
		options = g_new ( GnoclOption, noColOptions + noCellRenderOptions + 1 );
		options->optName = NULL;
		startRenderOptions = appendOptions ( options, colOptions );
		appendOptions ( options, cellRenderOptions );
	}

	if ( gnoclParseOptions ( interp, objc - 2, objv + 2, options )  != TCL_OK )
	{
		goto cleanExit;
	}

	if ( gnoclSetOptions ( interp, options, G_OBJECT ( column ), startRenderOptions ) != TCL_OK )
	{
		goto cleanExit;
	}

	if ( gnoclSetOptions ( interp, options + startRenderOptions, G_OBJECT ( renderer ), -1 ) != TCL_OK )
	{
		goto cleanExit;
	}

	if ( options[widthIdx].status == GNOCL_STATUS_CHANGED )
	{
		gtk_tree_view_column_set_sizing ( column, GTK_TREE_VIEW_COLUMN_FIXED );
		gtk_tree_view_column_set_fixed_width ( column, options[widthIdx].val.i );

	}

	if ( options[widgetIdx].status == GNOCL_STATUS_CHANGED )
	{

		GtkWidget *widget = NULL;

		widget = gnoclGetWidgetFromName ( Tcl_GetString ( objv[4] ) , interp );

		if ( widget == NULL )
		{
			return TCL_ERROR;
		}

		else
		{
			gtk_tree_view_column_set_widget  ( column, widget );
		}
	}


	if ( options[startRenderOptions].status == GNOCL_STATUS_CHANGED )
	{
		GnoclCommandData *cs = g_new ( GnoclCommandData, 1 );
		cs->interp = interp;
		cs->data = para;
		cs->command = g_strdup ( options[startRenderOptions].val.str );
		gtk_tree_view_column_set_cell_data_func ( column, renderer, cellDataFunc, cs, deleteCellData );

	}

	if ( onCmdIdx > 0 && options[onCmdIdx].status == GNOCL_STATUS_CHANGED )
	{
		GtkTreeModel *model = gtk_tree_view_get_model ( para->view );
		const int isText = GTK_IS_CELL_RENDERER_TEXT ( renderer );
		GCallback  callback, defaultCallback;
		const char *signal;

		if ( isText )
		{
			assert ( strcmp ( options[onCmdIdx].optName, "-onEdited" ) == 0 );
			callback = G_CALLBACK ( editedFunc );
			defaultCallback = G_CALLBACK ( defaultEditedFunc );
			signal   = "edited";
		}

		else
		{
			assert ( strcmp ( options[onCmdIdx].optName, "-onToggled" ) == 0 );
			callback = G_CALLBACK ( toggledFunc );
			defaultCallback = G_CALLBACK ( defaultToggledFunc );
			signal   = "toggled";
		}

		gnoclDisconnect ( G_OBJECT ( renderer ), signal, defaultCallback );

		gnoclConnectOptCmd ( interp, G_OBJECT ( renderer ), signal, callback, &options[onCmdIdx], para, NULL );

		if ( gnoclGetOptCmd ( G_OBJECT ( renderer ), signal ) == NULL )
		{
			g_signal_connect ( G_OBJECT ( renderer ), signal, defaultCallback, model );
		}
	}

	ret = TCL_OK;

cleanExit:
	gnoclClearOptions ( options );
	g_free ( options );

	return ret;
}

/**
\brief	Obtain the value of a single cell.
**/
static int getValue ( TreeListParams * para, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[] )
{
#if 0
	g_print ( "%s\n", __FUNCTION__ );
#endif

	int          colNo;
	GtkTreeIter  iter;
	Tcl_Obj      *res;
	GtkTreeModel *model = gtk_tree_view_get_model ( para->view );

	if ( objc != 4 )
	{
		Tcl_WrongNumArgs ( interp, 2, objv, "pathOrReference column" );
		return TCL_ERROR;
	}

	if ( getIterFromTcl ( interp, objv[2], model, para->idToIter, 0, &iter ) != TCL_OK )
	{
		return TCL_ERROR;
	}

	if ( getValidColumn ( para->view, interp, objv[3], &colNo ) == NULL )
	{
		return TCL_ERROR;
	}

	res = getObjFromCol ( model, colNo, &iter );

	/*
	if( gtk_list_get_text( para->list, row, column, &text ) == 0 )
	{
	   Tcl_SetResult( interp, "text could not be retrieved.",
	         TCL_STATIC );
	   return TCL_ERROR;
	}
	*/

	if ( res != NULL )
	{
		Tcl_SetObjResult ( interp, res );
	}

	return TCL_OK;
}

/**
\brief	Obtain value from specific cell entry.
**/
/*
char * getCellValue (GtkTreeModel *model, GtkTreeIter *iter, int colNo)
{
	static gchar str[64];

	switch ( gtk_tree_model_get_column_type ( model, colNo ) )
	{
		case G_TYPE_BOOLEAN:
		case G_TYPE_UINT:
		case G_TYPE_INT:
			{
				int n;
				gtk_tree_model_get ( model, iter, colNo, &n, -1 );
				sprint(str,"%d",n);
				return str;
			}
		case G_TYPE_DOUBLE:
			{
				double n;
				gtk_tree_model_get ( model, iter, colNo, &n, -1 );
				sprint(str,"%f",n);
				return str;
			}
		case G_TYPE_OBJECT:
			{
				return NULL;
			}
		default:
			{
				char *txt;
				gint len;
				gtk_tree_model_get ( model, iter, colNo, txt, -1 );
				return txt;
			}
	}
	return "";
}
*/
/**
\brief	Return a list that contains all rows and columns.

	{
		{ {1} {2} {3} }
		{ {1} {2} {3} }
		{ {1} {2} {3} }
	}
\todo	Recursion of rested nodes
**/
static int getFullList ( TreeListParams * para, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[] )
{
#if 0
	g_print ( "%s\n", __FUNCTION__ );
#endif

	int          colNo;
	GtkTreeIter  iter;
	Tcl_Obj      *res;
	Tcl_Obj      *tmp1;
	Tcl_Obj      *tmp2;
	gint row, col;

	GtkTreeModel *model;

	row = 0; col = 0;

	model = gtk_tree_view_get_model ( para->view );

#if 0
	g_print ( "%s\n", __FUNCTION__ );
#endif

	/*
		if ( objc != 4 )
		{
			Tcl_WrongNumArgs ( interp, 2, objv, "pathOrReference column" );
			return TCL_ERROR;
		}
	*/

	/* total number of rows and columns */
	gint nrows = gtk_tree_model_iter_n_children ( model, NULL );
	gint ncols = gtk_tree_model_get_n_columns ( model );

#if 0
	g_print ( "rows %d: cols = %d\n", nrows, ncols );
#endif

	//col = gtk_tree_model_iter_n_children ( model, &iter );

	gtk_tree_model_get_iter_first ( model, &iter );

	//tmp2 = Tcl_NewStringObj ( 0, NULL );
	res = Tcl_NewStringObj ( 0, NULL );


	/* add headings and type */
	const gchar *title;
	GtkTreeViewColumn *column;

	/* add these values to two seperate lists */
	for ( col = 0; col < ncols / 2; col++ )
	{
// boolean, integer, float, string, markup or  image
		switch ( gtk_tree_model_get_column_type ( model, col ) )
		{
			case G_TYPE_BOOLEAN:
				{
#if 0
					g_print ( "col %d is an BOOLEAN\n", col );
#endif
				}
				break;
			case G_TYPE_UINT:
			case G_TYPE_INT:
				{
#if 0
					g_print ( "col %d is an INTEGER\n", col );
#endif
				}

				break;
			case G_TYPE_DOUBLE:
				{
#if 0
					g_print ( "col %d is an FLOAT\n", col );
#endif
				}

				break;
			case G_TYPE_OBJECT:
				{
#if 0
					g_print ( "col %d is an IMAGE\n", col );
#endif
				}
				break;
			default:
				{
#if 0
					g_print ( "col %d is an STRING\n", col );
#endif
				}
		}

		column = gtk_tree_view_get_column ( para->view, col );
		title = gtk_tree_view_column_get_title ( column );
#if 0
		g_print ( "col %d title = %s\n", col, title );
#endif

	}

	/* then add lists to the res object */

	do
	{
		tmp2 = Tcl_NewStringObj ( 0, NULL );

		tmp1 = Tcl_NewStringObj ( 0, NULL );

		for ( col = 0; col < ncols / 2; col++ )
		{

			tmp1 = getObjFromCol ( model, col, &iter );

			if ( Tcl_IsShared ( tmp2 ) )
			{
				tmp2 = Tcl_DuplicateObj ( tmp2 );
			}

			if ( Tcl_IsShared ( tmp1 ) )
			{
				tmp1 = Tcl_DuplicateObj ( tmp1 );
			}

			Tcl_ListObjAppendElement ( interp, tmp2, tmp1 );
#if 0
			g_print ( "item = %s\n", Tcl_GetString ( tmp1 ) );
#endif
		}

		//tmp =  Tcl_NewStringObj ( "}", -1 );
#if 0
		g_print ( "add list 1 %s\n", Tcl_GetString ( tmp2 ) );
#endif

		if ( Tcl_IsShared ( tmp2 ) )
		{
			tmp2 = Tcl_DuplicateObj ( tmp2 );
		}

		if ( Tcl_IsShared ( res ) )
		{
			res = Tcl_DuplicateObj ( res );
		}

#if 0
		g_print ( "add list 2\n" );
#endif
		Tcl_ListObjAppendElement ( interp, res, tmp2 );
#if 0
		g_print ( "add list 3\n" );
#endif
	}
	while ( gtk_tree_model_iter_next ( model, &iter ) );

	Tcl_SetObjResult ( interp, res );

	return TCL_OK;
}


/**
\brief
**/
static int cellConfigure ( TreeListParams * para, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[] )
{
#if 0
	g_print ( "%s %d\n", __FUNCTION__, objc );
#endif
	GnoclOption options[] =
	{
		{ "-value", GNOCL_OBJ, NULL },
		{ "-visible", GNOCL_OBJ, NULL },
		{ "-background", GNOCL_OBJ, NULL },
		/* TODO: all renderer options depending on type of renderer */
		{ NULL }
	};

	enum ConfigType types[] =
	{
		CONFIG_VALUE, CONFIG_VISIBLE, CONFIG_BGCLR
	};

	guint        k;
	int          col;
	GtkTreeIter  iter;
	GtkTreeModel *model = gtk_tree_view_get_model ( para->view );
	int          ret = TCL_ERROR;


	if ( objc < 4 )
	{
		Tcl_WrongNumArgs ( interp, 2, objv, "pathOrReference column ? option val ... ? " );
		return TCL_ERROR;
	}

	if ( getIterFromTcl ( interp, objv[2], model, para->idToIter, 0, &iter ) != TCL_OK )
	{
		return TCL_ERROR;
	}

	if ( getValidColumn ( para->view, interp, objv[3], &col ) == NULL )
	{
		return TCL_ERROR;
	}

	if ( gnoclParseOptions ( interp, objc - 3, objv + 3, options )  != TCL_OK )
	{
		goto cleanExit;
	}

	/* -value */
	for ( k = 0; k < sizeof ( types ) / sizeof ( *types ); ++k )
	{
		if ( options[k].status == GNOCL_STATUS_CHANGED )
		{
			int off = getOffset ( para, types[k], col );

			if ( setCell ( interp, para->view, &iter, off, options[k].val.obj, para->isTree ) != TCL_OK )
			{
				goto cleanExit;
			}
		}
	}

	ret = TCL_OK;

cleanExit:
	gnoclClearOptions ( options );

	return ret;
}


/**
\brief
**/
static int addRow ( TreeListParams * para, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[], int insertPos )
{
#if 0
	g_print ( "%s objc = %d\n", __FUNCTION__, objc );
#endif

	GnoclOption options[] =
	{
		{ "-singleRow", GNOCL_BOOL, NULL },    /* 0 */
		{ "-singleColumn", GNOCL_BOOL, NULL }, /* 1 */
		{ NULL }
	};
	const int singleRowIdx = 0;
	const int singleColIdx = 1;

	int offset = 2; 	// position of row data in command string

	/* make modifications backwards compatible with earlier releases where add = addEnd */
	if ( objc == 6 )
	{
		offset = 3;
	}


	int singleRow = 0;
	int singleCol = 0;

	if ( para->isTree )
	{
		if ( objc < 4 )
		{
			Tcl_WrongNumArgs ( interp, 2, objv, "path row - list ? option val ... ? " );
			return TCL_ERROR;
		}

		++offset;
	}

	else if ( objc < 3 )
	{
		Tcl_WrongNumArgs ( interp, 2, objv, "row - list" );
		return TCL_ERROR;
	}

	if ( gnoclParseOptions ( interp, objc - offset, objv + offset, options ) != TCL_OK )
	{
		gnoclClearOptions ( options );
		return TCL_ERROR;
	}

	if ( options[singleRowIdx].status == GNOCL_STATUS_CHANGED )
	{
		singleRow = options[singleRowIdx].val.b;
	}

	if ( options[singleColIdx].status == GNOCL_STATUS_CHANGED )
	{
		singleCol = options[singleColIdx].val.b;
	}

	gnoclClearOptions ( options );

	if ( para->isTree )
	{
		return addTreeChildren ( para, interp, objv[2], objv[3], singleRow, singleCol, insertPos );
	}


	return addListChildren ( para, interp, objv[offset], singleRow, singleCol, insertPos );

}

static	const char *cmds[] =
{
	"cget", "delete", "configure",
	"add", "addBegin", "addEnd",  "insert",
	"getSelection", "setSelection", "onSelectionChanged",
	"columnConfigure", "columnCget", "get",
	"getfulllist",
	"setRowColor",

	"cellConfigure", "erase", "scrollToPosition", "collapse",
	"expand", "getNumChildren", "coordsToPath", "setCursor",
	"getReference", "deleteReference", "referenceToPath", "class",
	"search", "resize", "columns", "rows", "redraw",
	NULL
};


/**
\brief
**/
int treeListFunc ( ClientData data, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[] )
{
#if 0
	g_print ( "%s\n", __FUNCTION__ );
#endif

	/* TODO "rowConfigure", "sort", */


	enum cmdIdx
	{
		CgetIdx, DeleteIdx, ConfigureIdx,
		AddIdx, AddBeginIdx, AddEndIdx, InsertIdx,
		GetSelectionIdx, SetSelectionIdx, OnSelectionChangedIdx,
		ColumnConfigureIdx, ColumnCgetIdx, GetIdx,

		GetFullListIdx, SetRowColorIdx,

		CellConfigureIdx, EraseIdx, ScrollToPosIdx, CollapseIdx,
		ExpandIdx, GetNumChildren, CoordsToPathIdx, SetCursorIdx,
		GetReferenceIdx, DeleteReferenceIdx, ReferenceToPathIdx, ClassIdx,
		SearchIdx, ResizeIdx, ColumnsIdx, RowsIdx, RedrawIdx
	};

	TreeListParams *para = ( TreeListParams * ) data;
	int idx;
	/* select: [list 1 2 4 "5 to 10"] all end end-5 "" */

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
			/* format
			0   1      2
			$id resize colId
			*/
		case SetRowColorIdx:
			{
				g_print ( "Row = %d : Color = %s\n", 5, "red" );
			}
			break;
		case CgetIdx:
			{
				int idx;

				switch ( gnoclCget ( interp, objc, objv, G_OBJECT ( para ), treeListOptions, &idx ) )
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
							return cget ( interp, G_OBJECT ( para ), treeListOptions, idx );
						}
				}

			}
			break;

		case RedrawIdx:
			{

				GdkRectangle visible_rect;
				/* using a path will only get one row, but if you specify NULL, then you'll
				need to set rect.height to GTK_WIDGET(view)->allocation.height,
				which will cause the whole column to be redrawn */

				//gtk_tree_view_get_cell_area(view, gtk_tree_path_new_from_string("1"), gtk_tree_view_get_column(view, 1), &rect);

				gtk_tree_view_get_visible_rect ( para->view, &visible_rect );
				gdk_window_invalidate_rect ( GTK_WIDGET ( para->view )->window, &visible_rect, TRUE );


			}
			break;
		case RowsIdx:
			{
				GList * rows;
				gint n;
				gchar buffer[16];

				GtkTreeModel *tree_model;
				tree_model = gtk_tree_view_get_model ( para->view );
				n = gtk_tree_model_iter_n_children ( tree_model, NULL );

				sprintf ( buffer, " % d", n );

				Tcl_SetObjResult ( interp, Tcl_NewStringObj ( buffer, -1 ) );
				return TCL_OK;
			}
			break;
		case ColumnsIdx:
			{
				GList * columns;
				gint n;
				gchar buffer[16];
				columns = gtk_tree_view_get_columns ( para->view );
				n = g_list_length ( columns );
				g_list_free ( columns );

				sprintf ( buffer, " % d", n );

				Tcl_SetObjResult ( interp, Tcl_NewStringObj ( buffer, -1 ) );
				return TCL_OK;
			}
			break;
		case ResizeIdx:
			{
				GtkTreeViewColumn *tree_column;
				int colNo;

				if ( getValidColumn ( para->view, interp, objv[2], &colNo ) == NULL )
				{
					return TCL_ERROR;
				}

				tree_column = gtk_tree_view_get_column ( para->view, colNo );

				gtk_tree_view_column_queue_resize   ( tree_column );
			}
			break;
		case SearchIdx:
			{

#if 0
				g_print ( "treeList search\n" );
#endif

				int          colNo;
				gchar		 *str;
				gchar		 *celStr;
				GtkTreeIter  iter;
				Tcl_Obj      *res;
				GtkTreeModel *model = gtk_tree_view_get_model ( para->view );

				if ( objc != 4 )
				{
					Tcl_WrongNumArgs ( interp, 2, objv, "column searchString" );
					return TCL_ERROR;
				}

				if ( getValidColumn ( para->view, interp, objv[2], &colNo ) == NULL )
				{
					return TCL_ERROR;
				}

				str = Tcl_GetString ( objv[3] );

#if 0
				g_print ( "Column = % d String = % s\n", colNo, str );
#endif
				//gtk_tree_model_get ( model, iter, colNo, &celStr, -1 );


				gtk_tree_model_foreach ( model, search_foreach_func, str );
				//gtk_text_tag_table_foreach ( tagtable, deleteTag , tagtable );

#if 0
				g_print ( "Column = % d String = % s\n", colNo, celStr );
#endif

			}
			break;
		case ClassIdx:
			{
				Tcl_SetObjResult ( interp, Tcl_NewStringObj ( "treeList", -1 ) );
			}
			break;
		case DeleteIdx:
			{
				return gnoclDelete ( interp, GTK_WIDGET ( para->scrollWin ), objc, objv );
			}
		case ConfigureIdx:
			{
				int ret = TCL_ERROR;

				if ( gnoclParseOptions ( interp, objc - 1, objv + 1, treeListOptions ) == TCL_OK )
				{
					ret = configure ( interp, para, treeListOptions );
				}

				gnoclClearOptions ( treeListOptions );

				return ret;
			}

			break;
		case InsertIdx:
			{
#if 0
				g_print ( "%s AddIdx objc = %d : >%s<\n", __FUNCTION__, objc,  Tcl_GetString ( objv[2] ) );
#endif
				gint row;

				/* insert */
				if  ( Tcl_GetIntFromObj ( NULL, objv[2], &row ) == TCL_OK )
				{
					return addRow ( para, interp, objc, objv, row );
				}

			}
			break;
		case AddIdx:
		case AddEndIdx:
			{
#if 0
				g_print ( "%s AddEndIdx\n", __FUNCTION__ );
#endif
				return addRow ( para, interp, objc, objv, -1 );
			}
			break;
		case AddBeginIdx:
			{
#if 0
				g_print ( "%s AddBeginIdx\n", __FUNCTION__ );
#endif

				return addRow ( para, interp, objc, objv, 0 );
			}
			break;

		case ColumnConfigureIdx:
			{
				return columnConfigure ( para, interp, objc, objv );
			}
		case ColumnCgetIdx:
			{
				return columnCget ( para, interp, objc, objv );
			}
		case GetIdx:
			{
				return getValue ( para, interp, objc, objv );
			}
		case GetFullListIdx:
			{
				return getFullList ( para, interp, objc, objv );
			}
		case CellConfigureIdx:
			{
				return cellConfigure ( para, interp, objc, objv );
			}
		case GetSelectionIdx:
			{
				return getSelection ( para, interp, objc, objv );
			}
		case SetSelectionIdx:
			{
				return setSelection ( para, interp, objc, objv );
			}
		case OnSelectionChangedIdx:
			{
				GtkTreeSelection *sel = gtk_tree_view_get_selection ( para->view );

				if ( objc != 2 )
				{
					Tcl_WrongNumArgs ( interp, 1, objv, NULL );
					return TCL_ERROR;
				}

				return doOnSelectionChanged ( interp, para, sel, gnoclGetOptCmd ( G_OBJECT ( sel ), "changed" ), 0 );
			}

		case EraseIdx:
			{
				return erase ( para, interp, objc, objv );
			}
		case ScrollToPosIdx:
			{
				return scrollToPos ( para, interp, objc, objv );
			}
		case CollapseIdx:
		case ExpandIdx:

			if ( !para->isTree )
			{

				Tcl_SetResult ( interp, "This command is only valid for a tree widget.", TCL_STATIC );
				return TCL_ERROR;
			}

			return collapse ( para, interp, objc, objv, idx == CollapseIdx );

		case GetNumChildren:
			{
				return getNumChildren ( para, interp, objc, objv );
			}
		case CoordsToPathIdx:
			{
				return coordsToPath ( para, interp, objc, objv );
			}
		case SetCursorIdx:
			{
				return setCursor ( para, interp, objc, objv );
			}
		case GetReferenceIdx:
			{
				int   no = 0;
				GtkTreeModel *model = gtk_tree_view_get_model ( para->view );
				GtkTreeIter  iter;
				char         buffer[256];

				if ( objc != 3 )
				{
					Tcl_WrongNumArgs ( interp, 2, objv, "path" );
					return TCL_ERROR;
				}

				if ( iterFromTclPath ( interp, objv[2], model, 0, &iter ) != TCL_OK )
				{
					return TCL_ERROR;
				}

				no = addIterator ( para, &iter );

				sprintf ( buffer, " % s % d", refPrefix, no );

				Tcl_SetObjResult ( interp, Tcl_NewStringObj ( buffer, -1 ) );

			}

			return TCL_OK;
		case DeleteReferenceIdx:
			{
				int  no;
				char *str;

				if ( objc != 3 )
				{
					Tcl_WrongNumArgs ( interp, 2, objv, "reference" );
					return TCL_ERROR;
				}

				/* G_BREAKPOINT(); */
				str = Tcl_GetString ( objv[2] );

				no = atoi ( str + sizeof ( refPrefix ) - 1 );

				if ( strncmp ( str, refPrefix, sizeof ( refPrefix ) - 1 ) || removeIterFromId ( para, no ) == 0 )
				{
					Tcl_AppendResult ( interp, "\"", str, "\" is not a valid reference.", ( char * ) NULL );

					return TCL_ERROR;
				}
			}

			return TCL_OK;
		case ReferenceToPathIdx:
			{
				GtkTreeModel *model = gtk_tree_view_get_model ( para->view );
				GtkTreeIter iter;

				if ( objc != 3 )
				{
					Tcl_WrongNumArgs ( interp, 2, objv, "reference" );
					return TCL_ERROR;
				}

				if ( getRefFromTcl ( interp, objv[2], para->idToIter, &iter ) != TCL_OK )
				{
					return TCL_ERROR;
				}

				Tcl_SetObjResult ( interp, tclPathFromIter ( model, &iter ) );
			}

			return TCL_OK;

	}

	return TCL_OK;
}

/**
\brief
**/
static int getGType ( Tcl_Interp * interp, Tcl_Obj * obj, GType * type, int * isMarkup )
{
#if 0
	g_print ( "%s\n", __FUNCTION__ );
#endif


	/* since G_TYPE_BOOLEAN does not support sorting, we use G_TYPE_UINT */
	const char *txt[] =
	{
		"markup", "boolean", "integer",
		"float", "string", "image",
		NULL
	};

	GType types[] =
	{
		G_TYPE_STRING, G_TYPE_UINT, G_TYPE_INT,
		G_TYPE_DOUBLE, G_TYPE_STRING, G_TYPE_OBJECT
	};

	int idx;

	if ( Tcl_GetIndexFromObj ( interp, obj, txt, "type", TCL_EXACT, &idx ) != TCL_OK )
	{
		return TCL_ERROR;
	}

	*type = types[idx];

	*isMarkup = ( idx == 0 );

	return TCL_OK;
}

/**
\brief      Create the actual widget itself, configuring for either a tree or list.
**/
static int gnoclTreeListCmd ( ClientData data, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[], int isTree )
{

#if 0
	g_print ( "%s\n", __FUNCTION__ );
#endif

	TreeListParams   *para;
	int              ret, k;
	int              noColumns = -1;
	GtkWidget        *widget;
	GtkTreeModel     *model;
	GType            *types;
	int              *isMarkup;
	GnoclOption      * const columnsOpt = &treeListOptions[columnsIdx];
	GnoclOption      * const titlesOpt = &treeListOptions[titlesIdx];
	GnoclOption      * const typesOpt = &treeListOptions[typesIdx];

	/* check valid options */
	if ( gnoclParseOptions ( interp, objc, objv, treeListOptions ) != TCL_OK )
	{
		goto clearExit;
	}

	/* set style options */

	/* check 1 */
	if ( columnsOpt->status == 0 && titlesOpt->status == 0 && typesOpt->status == 0 )
	{
		Tcl_SetResult ( interp,
						"On creation the number of columns must be given either\n"
						"explicitly be the option -columns or implicitly by the length\n"
						"of -titles or -types.",
						TCL_STATIC );
		goto clearExit;
	}

	/* check 2 */
	if ( columnsOpt->status == GNOCL_STATUS_CHANGED )
	{
		noColumns = columnsOpt->val.i;
	}

	/* check 3 */
	if ( titlesOpt->status == GNOCL_STATUS_CHANGED )
	{
		int len;

		if ( Tcl_ListObjLength ( interp, titlesOpt->val.obj, &len ) != TCL_OK  )
		{
			Tcl_SetResult ( interp, "titles list must be proper list", TCL_STATIC );
			goto clearExit;
		}

		if ( noColumns > 0 && len != noColumns )
		{
			Tcl_SetResult ( interp, "titles list must be same length as given by -columns", TCL_STATIC );
			goto clearExit;
		}

		noColumns = len;
	}

	/* check 4 */
	if ( typesOpt->status == GNOCL_STATUS_CHANGED )
	{
		int len;

		if ( Tcl_ListObjLength ( interp, typesOpt->val.obj, &len ) != TCL_OK  )
		{
			Tcl_SetResult ( interp, "typesLen list must be proper list", TCL_STATIC );
			goto clearExit;
		}

		if ( noColumns > 0 && len != noColumns )
		{
			Tcl_SetResult ( interp, "types list must be same length as given by -columns", TCL_STATIC );
			goto clearExit;
		}

		noColumns = len;
	}

	/* check 5 */
	if ( noColumns < 1 )
	{
		Tcl_SetResult ( interp, "number of columns must be greater 0", TCL_STATIC );
		goto clearExit;
	}

	/* create a new parameters list for the GtkTreeView widget */
	para = g_new ( TreeListParams, 1 );
	para->noColumns = noColumns;
	types = g_new ( GType, 2 * noColumns );
	isMarkup = g_new ( int, noColumns );

	if ( typesOpt->status == GNOCL_STATUS_CHANGED )
	{
		int k;

		for ( k = 0; k < noColumns; ++k )
		{
			Tcl_Obj *tp;

			if ( Tcl_ListObjIndex ( interp, typesOpt->val.obj, k, &tp ) != TCL_OK || getGType ( interp, tp, &types[k], &isMarkup[k] ) != TCL_OK )
			{
				g_free ( types );
				g_free ( para );
				g_free ( isMarkup );
				goto clearExit;
			}
		}
	}
	else
	{
		for ( k = 0; k < noColumns; ++k )
		{
			types[k] = G_TYPE_STRING;
			isMarkup[k] = 0;
		}
	}

	for ( k = 0; k < noColumns; ++k )
	{
		types[noColumns+k] = G_TYPE_BOOLEAN; /* visible */
	}

	/* for every viewable column we need
	   - one column for the value
	   - one column for -visible
	*/
	if ( isTree )
	{
		model = GTK_TREE_MODEL ( gtk_tree_store_newv ( 2 * noColumns, types ) );
	}
	else
	{
		model = GTK_TREE_MODEL ( gtk_list_store_newv ( 2 * noColumns, types ) );
	}

	g_free ( types );


#if 0

	/* TODO: test, if label != "" and shadow != "flat" ? */
	if ( opt.label.val || opt.shadow.val )
	{
		para->frame = GTK_FRAME ( gtk_frame_new ( NULL ) );
	}
	else
	{
		para->frame = NULL;
	}

#endif

	para->noColumns = noColumns;
	para->isTree = isTree;
	para->name = gnoclGetAutoWidgetId( );
	para->interp = interp;
	para->view = GTK_TREE_VIEW ( gtk_tree_view_new_with_model ( model ) );

	/*-------------- add search function --------------*/
	//gtk_tree_view_set_search_equal_func ( para->view, search_equal_func, NULL, NULL );
	/*-------------------------------------------------*/
	/* new stuff */
	/*
	GtkTreeModel *model_sort;
	gtk_tree_model_sort_new_with_model(GTK_TREE_MODEL (model));
	gtk_tree_view_set_model (GTK_TREE_VIEW (treeview), model);
	g_object_unref (G_OBJECT (model));
	*/

	/*------------*/

	para->idToIter = g_hash_table_new_full ( g_direct_hash, g_direct_equal, NULL, g_free );

	/* para->columnClicked = NULL; */

	para->scrollWin = GTK_SCROLLED_WINDOW ( gtk_scrolled_window_new ( NULL, NULL ) );
	gtk_scrolled_window_set_policy ( para->scrollWin, GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC );
	gtk_container_add ( GTK_CONTAINER ( para->scrollWin ), GTK_WIDGET ( para->view ) );

#if 0

	if ( para->frame )
	{
		widget = GTK_WIDGET ( para->frame );
		gtk_container_add ( GTK_CONTAINER ( para->frame ), GTK_WIDGET ( para->list ) );
	}
	else
	{
		widget = GTK_WIDGET ( para->list );
	}

#endif

	/* widget = GTK_WIDGET( para->view ); */
	widget = GTK_WIDGET ( para->scrollWin );

	/* TODO: can types be changed after creation? */
	for ( k = 0; k < noColumns; ++k )
	{
		/* TODO: remember renderer, so that we can configure it afterwards? */
		GtkTreeViewColumn *column = gtk_tree_view_column_new();

		GType type = gtk_tree_model_get_column_type ( model, k );
		GtkCellRenderer *renderer;

		if ( type == G_TYPE_UINT )
		{
			renderer = gtk_cell_renderer_toggle_new( );
			gtk_tree_view_column_pack_start ( column, renderer, 1 );
			gtk_tree_view_column_set_attributes ( column, renderer, "active", k, "visible", getOffset ( para, CONFIG_VISIBLE, k ), NULL );
			g_signal_connect ( G_OBJECT ( renderer ), "toggled", G_CALLBACK ( defaultToggledFunc ), model );
		}

		else if ( type == G_TYPE_OBJECT )
		{
			renderer = gtk_cell_renderer_pixbuf_new( );

			gtk_tree_view_column_pack_start ( column, renderer, 1 );
			gtk_tree_view_column_set_attributes ( column, renderer, "pixbuf", k, "visible", getOffset ( para, CONFIG_VISIBLE, k ), NULL );
		}

		else
		{
			renderer = gtk_cell_renderer_text_new( );
			/* set pango wrapmode and wrap width */

			gtk_tree_view_column_pack_start ( column, renderer, 1 );

#if 0
			gtk_tree_view_column_set_attributes ( column, renderer, isMarkup[k] ? "markup" : "text", k, "visible", getOffset ( para, CONFIG_VISIBLE, k ), NULL );
#else

			if ( isMarkup[k] )
			{
				gtk_tree_view_column_set_attributes ( column, renderer, "markup", k, "visible", getOffset ( para, CONFIG_VISIBLE, k ), NULL );

			}

			else
			{
				gtk_tree_view_column_set_attributes ( column, renderer, "text", k, "visible", getOffset ( para, CONFIG_VISIBLE, k ), NULL );
			}

#endif

			//http://library.gnome.org/devel/gtk/2.18/GtkCellRendererText.html#GtkCellRendererText--width-chars
			//gtk_tree_view_column_set_attributes ( column, renderer,"wrap-mode",PANGO_WRAP_WORD_CHAR,"wrap-width",200, NULL );

			g_signal_connect ( G_OBJECT ( renderer ), "edited", G_CALLBACK ( defaultEditedFunc ), model );
		}

		g_object_set_data ( G_OBJECT ( renderer ), "gnoclColumn", GINT_TO_POINTER ( k ) );

		gtk_tree_view_column_set_resizable ( column, 1 );
		gtk_tree_view_append_column ( para->view, column );

		/* default: make clickable and sortable */

		if ( type != G_TYPE_OBJECT )
		{
			gtk_tree_view_column_set_sort_column_id ( column, k );
		}
	}

	g_free ( isMarkup );

	ret = configure ( interp, para, treeListOptions );
	gnoclClearOptions ( treeListOptions );

	if ( ret != TCL_OK )
	{
		g_free ( para );
		gtk_widget_destroy ( GTK_WIDGET ( para->view ) );
		return TCL_ERROR;
	}

	gnoclMemNameAndWidget ( para->name, widget );

	gtk_widget_show_all ( widget );

	Tcl_CreateObjCommand ( interp, para->name, treeListFunc, para, NULL );

	Tcl_SetObjResult ( interp, Tcl_NewStringObj ( para->name, -1 ) );

	g_signal_connect ( G_OBJECT ( widget ), "destroy", G_CALLBACK ( destroyFunc ), para );

	return TCL_OK;

clearExit:
	gnoclClearOptions ( treeListOptions );
	return TCL_ERROR;
}

/**
 \brief     Create gnocl::tree object.
**/
int gnoclTreeCmd ( ClientData data, Tcl_Interp * interp,   int objc, Tcl_Obj * const objv[] )
{
#if 0
	g_print ( "%s\n", __FUNCTION__ );
#endif

	if ( gnoclGetCmdsAndOpts ( interp, cmds, treeListOptions, objv, objc ) == TCL_OK )
	{
		return TCL_OK;
	}

	return gnoclTreeListCmd ( data, interp, objc, objv, 1 );
}

/**
 \brief     Create gnocl::list object.
**/
int gnoclListCmd ( ClientData data, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[] )
{
#if 0
	g_print ( "%s\n", __FUNCTION__ );
#endif

	if ( gnoclGetCmdsAndOpts ( interp, cmds, treeListOptions, objv, objc ) == TCL_OK )
	{
		return TCL_OK;
	}

	return gnoclTreeListCmd ( data, interp, objc, objv, 0 );
}


