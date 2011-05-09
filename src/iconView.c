/**
\brief	This module implements the gnocl::iconView widget.
**/

/**
\page page_iconview gnocl::iconView
\htmlinclude iconview.html
**/

#include "gnocl.h"
#include "gnoclparams.h"
#include <string.h>
#include <assert.h>

/* some default settings for the default listStore model */

#define ITEM_WIDTH 75
#define ICON_WIDTH 50


/*
enum
{
	COL_DISPLAY_NAME,
	COL_PIXBUF,
	NUM_COLS
};
*/

enum
{
	COL_TNAIL = 0,  /* thumbnail */
	COL_LABEL,      /* text */
	COL_FNAME,		/* filename */
	COL_TOOLTIP,	/* tooltip text */
	N_COLS,         /* number of columns */
};

/**
\note	modify to suit iconview params.
**/
static void destroyFunc ( GtkWidget *widget, gpointer data )
{
#ifdef DEBUG_ICONVIEW
	printf ( "%s\n", __FUNCTION__ );
#endif

	IconViewParams *para = ( IconViewParams * ) data;
	gnoclForgetWidgetFromName ( para->name );
	Tcl_DeleteCommand ( para->interp, para->name );
	g_free ( para->name );
	g_free ( para );
}

/**
\brief
**/
static gboolean doOnActivateCursorItem ( GtkIconView *iconview, gpointer user_data )
{
	GnoclCommandData *cs = ( GnoclCommandData * ) user_data;

	//if ( *cs->interp->result == '\0' )
	//{
	GnoclPercSubst ps[] =
	{
		{ 'w', GNOCL_STRING },  /* printiconview */
		{ 0 }
	};

	ps[0].val.str = gnoclGetNameFromWidget ( iconview );
	gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 );
	//}
}

/**
\brief      This is for all callbacks which have only %w as substitution
**/
static int gnoclOptOnActivateCursorItem ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	assert ( opt->type == GNOCL_OBJ );

	return gnoclConnectOptCmd ( interp, obj, opt->propName, G_CALLBACK ( doOnActivateCursorItem ), opt, NULL, ret );
}

/**
\brief
**/
static void  doOnItemActivated ( GtkIconView *iconview, GtkTreePath *path, gpointer user_data )
{
	GnoclCommandData *cs = ( GnoclCommandData * ) user_data;

	//if ( *cs->interp->result == '\0' )
	//{
	GnoclPercSubst ps[] =
	{
		{ 'w', GNOCL_STRING },  /* widget name */
		{ 'l', GNOCL_STRING },  /* text */
		{ 'f', GNOCL_STRING },  /* filename */
		{ 't', GNOCL_STRING },  /* tooltip */
		{ 0 }
	};

	/*-------*/

	GtkTreeIter iter;
	GtkTreeModel *model;
	gchar *label, *fname, *tooltip;
	GdkPixbuf *pbuf, *tnail;

	model = gtk_icon_view_get_model ( iconview );

	gtk_tree_model_get_iter ( model, &iter, path );
	gtk_tree_model_get ( model, &iter, COL_TNAIL, &tnail, COL_LABEL, &label, COL_FNAME, &fname, COL_TOOLTIP, &tooltip, - 1 );

	/*-------*/

	//g_print ( "item = %s\n", text);

	ps[0].val.str = gnoclGetNameFromWidget ( gtk_widget_get_parent ( iconview ) );
	ps[1].val.str = label;
	ps[2].val.str = fname;
	ps[3].val.str = tooltip;
	gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 );
	//}
}

/**
\brief      This is for all callbacks which have only %w as substitution
**/
static int gnoclOptOnItemActivated ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	assert ( opt->type == GNOCL_OBJ );

	return gnoclConnectOptCmd ( interp, obj, opt->propName, G_CALLBACK ( doOnItemActivated ), opt, NULL, ret );
}

/**
\brief
**/
static gboolean doOnMoveCursor ( GtkIconView *iconview, GtkMovementStep step, gint count, gpointer user_data )
{
	GnoclCommandData *cs = ( GnoclCommandData * ) user_data;
	GList *list;

	//if ( *cs->interp->result == '\0' )
	//{
	GnoclPercSubst ps[] =
	{
		{ 'w', GNOCL_STRING },  /* printiconview */
		{ 0 }
	};

	list = gtk_icon_view_get_selected_items ( iconview );
	/*
	      GtkTreePath *path = (GtkTreePath *) (list->data);


	      gchar *text_path;
	      image = eog_thumb_view_get_image_from_path (tb, path);
	      text_path = gtk_tree_path_to_string (path);
	      g_free (text_path);

	      g_list_foreach (list, (GFunc) gtk_tree_path_free , NULL);
	      g_list_free (list);
	*/

	/* return name of container */
	ps[0].val.str = gnoclGetNameFromWidget ( gtk_widget_get_parent ( iconview ) );
	gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 );
	//}
}


/**
\brief  This is for all callbacks which have only %w as substitution
\note	This handler is specific for this module.
**/
static int gnoclOptIVOnMoveCursor ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	//assert ( opt->type == GNOCL_OBJ );

	assert ( strcmp ( opt->optName, "-onCursor" ) == 0 || strcmp ( opt->optName, "-onMoveCursor" ) == 0 );

	return gnoclConnectOptCmd ( interp, obj, opt->propName, G_CALLBACK ( doOnMoveCursor ), opt, NULL, ret );
}

/**
\brief
**/
static void doOnSelectAll ( GtkIconView *iconview, gpointer user_data )
{
	GnoclCommandData *cs = ( GnoclCommandData * ) user_data;

	//if ( *cs->interp->result == '\0' )
	//{
	GnoclPercSubst ps[] =
	{
		{ 'w', GNOCL_STRING },  /* printiconview */
		{ 0 }
	};

	ps[0].val.str = gnoclGetNameFromWidget ( iconview );
	gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 );
	//}
}

/**
\brief      This is for all callbacks which have only %w as substitution
**/
static int _gnoclOptOnSelectAll ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	assert ( opt->type == GNOCL_OBJ );

	return gnoclConnectOptCmd ( interp, obj, opt->propName, G_CALLBACK ( doOnSelectAll ), opt, NULL, ret );
}

/**
\brief
**/
static void  doOnSelectCursorItem ( GtkIconView *iconview, gpointer user_data )
{
	GnoclCommandData *cs = ( GnoclCommandData * ) user_data;

	//if ( *cs->interp->result == '\0' )
	//{
	GnoclPercSubst ps[] =
	{
		{ 'w', GNOCL_STRING },  /* printiconview */
		{ 0 }
	};

	ps[0].val.str = gnoclGetNameFromWidget ( iconview );
	gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 );
	//}
}

/**
\brief      This is for all callbacks which have only %w as substitution
**/
static int gnoclOptOnSelectCursorItem ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	assert ( opt->type == GNOCL_OBJ );

	return gnoclConnectOptCmd ( interp, obj, opt->propName, G_CALLBACK ( doOnSelectCursorItem ), opt, NULL, ret );
}

/**
\brief
**/
static void doOnSelectionChanged ( GtkIconView *iconview, gpointer user_data )
{

#ifdef DEBUG_ICONVIEW
	g_print ( "%s\n", __FUNCTION__ );
#endif


	GnoclCommandData *cs = ( GnoclCommandData * ) user_data;

	//if ( *cs->interp->result == '\0' )
	//{
	GnoclPercSubst ps[] =
	{
		{ 'w', GNOCL_STRING },  /* widget name */
		{ 'l', GNOCL_STRING },  /* text */
		{ 'f', GNOCL_STRING },  /* filename */
		{ 't', GNOCL_STRING },  /* tooltip */
		{ 0 }
	};

	/* get default display and screen */

	gint x, y;
	GdkDisplay *display = NULL;
	GdkScreen *screen = NULL;

	GtkTreeIter iter;
	GtkTreeModel *model;
	gchar *label, *fname, *tooltip;
	GdkPixbuf *tnail;
	GtkTreePath *path;

	g_print ( "%s 1\n", __FUNCTION__ );

	display = gdk_display_get_default ();
	screen = gdk_display_get_default_screen ( display );

	/* get cursor position */
	gdk_display_get_pointer ( display, NULL, &x, &y, NULL );

	g_print ( "%s\n 2\n", __FUNCTION__ );

	path = gtk_icon_view_get_path_at_pos ( iconview, x, y );
	model = gtk_icon_view_get_model ( iconview );

	g_print ( "%s 3\n", __FUNCTION__ );

	gtk_tree_model_get_iter ( model, &iter, path );
	/* problems here with this one */
	//gtk_tree_model_get ( model, &iter, COL_TNAIL, &tnail, COL_LABEL, &label, COL_FNAME, &fname, COL_TOOLTIP, &tooltip, - 1 );


	g_print ( "%s 4\n", __FUNCTION__ );
	ps[0].val.str = gnoclGetNameFromWidget ( gtk_widget_get_parent ( iconview ) );
	ps[1].val.str = label;
	ps[2].val.str = fname;
	ps[3].val.str = tooltip;

	g_print ( "%s 5\n", __FUNCTION__ );
	gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 );
	//}
}

/**
\brief      This is for all callbacks which have only %w as substitution
**/
static int gnoclOptOnSelectionChanged ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	assert ( opt->type == GNOCL_OBJ );

	return gnoclConnectOptCmd ( interp, obj, opt->propName, G_CALLBACK ( doOnSelectionChanged ), opt, NULL, ret );
}

/**
\brief
**/
static void doOnScrollAdjustments ( GtkIconView *horizontal, GtkAdjustment *vertical, GtkAdjustment *arg2, gpointer user_data )
{
	GnoclCommandData *cs = ( GnoclCommandData * ) user_data;

	if ( *cs->interp->result == '\0' )
	{
		GnoclPercSubst ps[] =
		{
			{ 'w', GNOCL_STRING },  /* printiconview */
			{ 0 }
		};

		ps[0].val.str = gnoclGetNameFromWidget ( horizontal );
		gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 );
	}
}

/**
\brief
**/
static int _gnoclOptOnScrollAdjustments ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	assert ( opt->type == GNOCL_OBJ );

	return gnoclConnectOptCmd ( interp, obj, opt->propName, G_CALLBACK ( doOnScrollAdjustments ), opt, NULL, ret );
}

/**
\brief
**/
static void dotOnToggleCursorItem ( GtkIconView *iconview, gpointer user_data )
{
	GnoclCommandData *cs = ( GnoclCommandData * ) user_data;

	//if ( *cs->interp->result == '\0' )
	//{
	GnoclPercSubst ps[] =
	{
		{ 'w', GNOCL_STRING },  /* printiconview */
		{ 0 }
	};

	ps[0].val.str = gnoclGetNameFromWidget ( iconview );
	gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 );
	//}
}

/**
\brief      This is for all callbacks which have only %w as substitution
**/
static int gnoclOptOnToggleCursorItem ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	assert ( opt->type == GNOCL_OBJ );

	return gnoclConnectOptCmd ( interp, obj, opt->propName, G_CALLBACK ( dotOnToggleCursorItem ), opt, NULL, ret );
}

/**
\brief
**/
static void doOnUnselectAll ( GtkIconView *iconview, gpointer user_data )
{
	GnoclCommandData *cs = ( GnoclCommandData * ) user_data;

	//if ( *cs->interp->result == '\0' )
	//{
	GnoclPercSubst ps[] =
	{
		{ 'w', GNOCL_STRING },  /* printiconview */
		{ 0 }
	};

	ps[0].val.str = gnoclGetNameFromWidget ( iconview );
	gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 );
	//}
}

/**
\brief      This is for all callbacks which have only %w as substitution
**/
static int gnoclOnUnselectAll ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	assert ( opt->type == GNOCL_OBJ );

	return gnoclConnectOptCmd ( interp, obj, opt->propName, G_CALLBACK ( doOnUnselectAll ), opt, NULL, ret );
}

/*
X  "column-spacing"           gint
X  "columns"                  gint
X  "item-padding"             gint
X  "item-width"               gint
X  "margin"                   gint
X  "markup-column"            gint
  "model"                    GtkTreeModel*
  "orientation"              GtkOrientation
X  "pixbuf-column"            gint
  "reorderable"              gboolean
  "row-spacing"              gint
  "selection-mode"           GtkSelectionMode
  "spacing"                  gint
  "text-column"              gint
  "tooltip-column"           gint

*/

/*
  "activate-cursor-item"                           : Run Last / Action
  "item-activated"                                 : Run Last
  "move-cursor"                                    : Run Last / Action
  "select-all"                                     : Run Last / Action
  "select-cursor-item"                             : Run Last / Action
  "selection-changed"                              : Run First
  "set-scroll-adjustments"                         : Run Last
  "toggle-cursor-item"                             : Run Last / Action
  "unselect-all"                                   : Run Last / Action
*/

/*
typedef enum
{
  GTK_ORIENTATION_HORIZONTAL,
  GTK_ORIENTATION_VERTICAL
}
*/

static const int modelIdx = 0;
static const int orientationIdx = 1;
static const int selectionModeIdx = 2;
static const int iconWidthIdx = 3;
static const int tooltipsIdx = 4;


static GnoclOption iconViewOptions[] =
{
	/* set in configure */
	{ "-model", GNOCL_STRING, NULL },
	{ "-orientation", GNOCL_STRING, NULL },
	{ "-selectionMode", GNOCL_STRING, NULL },
	{ "-iconWidth", GNOCL_INT, NULL },					/* not much use */
	{ "-tooltips", GNOCL_INT, NULL },

	/* widget properties */
	{ "-columnSpacing", GNOCL_INT, "column-spacing" },  /* space between icon columns */
	{ "-columns", GNOCL_INT, "columns" },
	{ "-itemPadding", GNOCL_INT, "item-padding" },
	{ "-itemWidth", GNOCL_INT, "item-width" },
	{ "-margin", GNOCL_INT, "margin" },                 /* spacing between icons and frame */
	{ "-markupColumn", GNOCL_INT, "markup-column" },
	//{ "-pixbufColumn", GNOCL_INT, "pixbuf-column" },
	{ "-reorderable", GNOCL_BOOL, "reorderable" },
	{ "-rowSpacing", GNOCL_INT, "row-spacing" },
	{ "-spacing", GNOCL_INT, "spacing" },
	//{ "-textColumn", GNOCL_INT, "text-column" },
	//{ "-tooltipColumn", GNOCL_INT, "tooltip-column" },

	/* widget signals */
	{ "-onActivateCursorItem", GNOCL_OBJ, "activate-cursor-item", gnoclOptOnActivateCursorItem },
	{ "-onItemActivated", GNOCL_OBJ,  "item-activated" , gnoclOptOnItemActivated },
	{ "-onMoveCursor", GNOCL_OBJ, "move-cursor", gnoclOptIVOnMoveCursor },
	{ "-onSelectAll", GNOCL_OBJ, "select-all", gnoclOptOnSelectAll },
	{ "-onSelectCursorItem", GNOCL_OBJ, "select-cursor-item", gnoclOptOnSelectCursorItem },
	{ "-onSelectionChanged", GNOCL_OBJ, "selection-changed", gnoclOptOnSelectionChanged },
	{ "-onSetScrollAdjustments", GNOCL_OBJ, "set-scroll-adjustments", gnoclOptOnScrollAdjustments },
	{ "-onToggleCursorItem", GNOCL_OBJ, "toggle-cursor-item", gnoclOptOnToggleCursorItem },
	{ "-onUnselectAll", GNOCL_OBJ, "unselect-all", gnoclOnUnselectAll },


	/* inherited properties */
	{ "-hasFocus", GNOCL_BOOL, "has-focus" },
	//{ "-tooltip", GNOCL_OBJ, "", gnoclOptTooltip },
	{ "-onShowHelp", GNOCL_OBJ, "", gnoclOptOnShowHelp },
	{ "-name", GNOCL_STRING, "name" },
	{ "-visible", GNOCL_BOOL, "visible" },
	{ "-sensitive", GNOCL_BOOL, "sensitive" },
	{ "-baseFont", GNOCL_OBJ, "Sans 14", gnoclOptGdkBaseFont },
	{ "-baseColor", GNOCL_OBJ, "normal", gnoclOptGdkColorBase },
	{ "-data", GNOCL_OBJ, "", gnoclOptData },

	{ NULL },
};

/**
\brief

static const int modelIdx = 0;
static const int orientationIdx = 1;
static const int selectionModeIdx = 2;

**/
static int configure ( Tcl_Interp *interp, IconViewParams *para, GnoclOption options[] )
{
#ifdef DEBUG_ICONVIEW
	g_print ( "CONFIGURE\n" );
#endif

	if ( options[modelIdx].status == GNOCL_STATUS_CHANGED )
	{
#ifdef DEBUG_ICONVIEW
		g_print ( "CONFIGURE -> model -> %s\n", options[modelIdx].val.str );
#endif
	}

	/* set orientation ie. vertical or horizontal */
	if ( options[orientationIdx].status == GNOCL_STATUS_CHANGED )
	{
#ifdef DEBUG_ICONVIEW
		g_print ( "CONFIGURE -> orientation\n" );
#endif
		const char *orientation[] = { "horizontal", "vertical", NULL };
		enum { horizontalIdx, verticalIdx };

		gint idx;

		getIdx ( orientation,  options[orientationIdx].val.str, &idx );

		switch ( idx )
		{

			case horizontalIdx:
				{
					gtk_icon_view_set_orientation ( para->iconView, GTK_ORIENTATION_HORIZONTAL );
#ifdef DEBUG_ICONVIEW
					g_print ( "CONFIGURE -> orientation -> horizontal\n" );
#endif
				}
				break;
			case verticalIdx:
				{

					gtk_icon_view_set_orientation ( para->iconView, GTK_ORIENTATION_VERTICAL );
#ifdef DEBUG_ICONVIEW
					g_print ( "CONFIGURE -> orientation -> vertical\n" );
#endif
				}
				break;
			default: {}
		}

	}

	if ( options[selectionModeIdx].status == GNOCL_STATUS_CHANGED )
	{
#ifdef DEBUG_ICONVIEW
		g_print ( "CONFIGURE -> selectionMode -> %s\n", options[selectionModeIdx].val.str );
#endif
	}

	if ( options[iconWidthIdx].status == GNOCL_STATUS_CHANGED )
	{
#ifdef DEBUG_ICONVIEW
		g_print ( "CONFIGURE -> iconWidth -> %d\n", options[iconWidthIdx].val.i );
#endif
		//para->icon_width = options[iconWidthIdx].val.i;

	}



	if ( options[tooltipsIdx].status == GNOCL_STATUS_CHANGED )
	{
#ifdef DEBUG_ICONVIEW
		g_print ( "CONFIGURE -> tooltips -> %d\n", options[tooltipsIdx].val.i );
#endif
		//para->icon_width = options[iconWidthIdx].val.i;


		if ( options[tooltipsIdx].val.i == 1 )
		{
			g_print ( "tooltips ON\n" );
			gtk_icon_view_set_tooltip_column ( para->iconView, COL_TOOLTIP );
		}

		else
		{
			g_print ( "tooltips OFF\n" );
			gtk_icon_view_set_tooltip_column ( para->iconView, -1 );

		}

	}


	return TCL_OK;
}



/**
\brief
**/
static int cget ( Tcl_Interp *interp, GtkWidget *widget, GnoclOption options[], int idx )
{
#ifdef DEBUG_ICONVIEW
	printf ( "%s\n", __FUNCTION__ );
#endif

	const char *dataIDa = "gnocl::data1";
	const char *dataIDb = "gnocl::data2";

	GtkWidget *arrow = gnoclFindChild ( GTK_WIDGET ( widget ), GTK_TYPE_BUTTON );
	char *str;

	/* this will enable us to obtain widget data for the arrow object */
	char *result = g_object_get_data ( G_OBJECT ( arrow ), dataIDa );
	result = g_object_get_data ( G_OBJECT ( arrow ), dataIDb );
	str = gnoclGetNameFromWidget ( arrow );


#ifdef DEBUG_ICONVIEW
	printf ( "%s\n", __FUNCTION__ );
#endif

	/*-----*/
	/*
	    Tcl_Obj *obj = NULL;

	    if ( idx == textIdx )
	    {
	        obj = gnoclCgetButtonText ( interp, widget );
	    }

	    else if ( idx == iconsIdx )
	    {
	        GtkWidget *image = gnoclFindChild ( GTK_WIDGET ( widget ), GTK_TYPE_IMAGE );

	        if ( image == NULL )
	            obj = Tcl_NewStringObj ( "", 0 );
	        else
	        {
	            gchar   *st;
	            g_object_get ( G_OBJECT ( image ), "stock", &st, NULL );

	            if ( st )
	            {
	                obj = Tcl_NewStringObj ( "%#", 2 );
	                Tcl_AppendObjToObj ( obj, gnoclGtkToStockName ( st ) );
	                g_free ( st );
	            }

	            else
	            {
	                Tcl_SetResult ( interp, "Could not determine icon type.",
	                                TCL_STATIC );
	                return TCL_ERROR;
	            }
	        }
	    }

	    if ( obj != NULL )
	    {
	        Tcl_SetObjResult ( interp, obj );
	        return TCL_OK;
	    }
	*/


	return gnoclCgetNotImplemented ( interp, options + idx );
}


/**
\brief
**/
int iconViewFunc ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{
#ifdef DEBUG_ICONVIEW
	listParameters ( objc, objv, __FUNCTION__ );
#endif

	IconViewParams *para = ( IconViewParams * ) data;


	static const char *cmds[] =
	{
		"delete", "configure", "cget",
		"onClicked", "class", "add",
		NULL
	};
	enum cmdIdx
	{
		DeleteIdx, ConfigureIdx, CgetIdx,
		OnClickedIdx, ClassIdx, AddIdx
	};

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
		case AddIdx:
			{

				/* move this code into a distinct function */

				GdkPixbuf *pbuf, *tnail;
				GtkTreeIter iter;
				gchar *label, *filename, *tooltip;

				float dest_width = 50;
				float dest_height = 50;
				float ds;

				int src_width, src_height;
				int width, height;

				pbuf = NULL;
				tnail = NULL;

				label = NULL;

				width = gtk_icon_view_get_item_width ( para->iconView );
				//width = para->iconWidth;
#ifdef DEBUG_ICONVIEW
				g_print ( "----->item width %d\n", width );
				g_print ( "----->icon width %d\n", para->icon_width );
#endif
				GnoclStringType type = gnoclGetStringType ( objv[2] );

				filename = substring ( Tcl_GetString ( objv[2] ), 2, strlen ( Tcl_GetString ( objv[2] ) ) );

				switch ( type )
				{
					case GNOCL_STR_FILE:
						{
							g_print ( "file %s\n", filename );

							if ( exists ( filename ) )
							{
								pbuf = gdk_pixbuf_new_from_file ( filename, NULL ); /* No error checking is done here */
							}

							else
							{
								pbuf = NULL;
							}
						} break;
					case GNOCL_STR_BUFFER:
						{
							/* problems here */
							g_print ( "1) buffer %s<\n", filename );
							pbuf = gnoclGetPixBufFromName ( filename, interp );
							g_print ( "2) buffname = %s\n", gnoclGetNameFromPixBuf ( pbuf ) );

						} break;
					case GNOCL_STR_STOCK:
						{
							g_print ( "stock %s\n", filename );
						} break;
					default:
						{
							g_print ( "unknown item %s\n", Tcl_GetString ( objv[2] ) );
							//p1 = gtk_widget_render_icon ( para->iconView, GTK_STOCK_MISSING_IMAGE, GTK_ICON_SIZE_LARGE_TOOLBAR, NULL );

						}
				}

				g_print ( "3) buffer\n" );

				if ( pbuf == NULL )
				{
					tnail = gtk_widget_render_icon ( para->iconView, GTK_STOCK_MISSING_IMAGE, GTK_ICON_SIZE_LARGE_TOOLBAR, NULL );
				}

				g_print ( "4) buffer\n" );
				src_width =  gdk_pixbuf_get_width ( pbuf );
				src_height = gdk_pixbuf_get_height ( pbuf );
				g_print ( "5) buffer\n" );

				/* add resizing code to make display icon from any pixbuf */

				if ( src_width != width )
				{
					/* fit image width to item with */
					ds = width / ( float ) src_width;
					height = ( width * src_height ) / src_width;

				}

				else
				{
					width = src_width;
					height = src_height;
				}


#ifdef DEBUG_ICONVIEW
				g_print ( "scale me down %d %d %d %d %f %f %f\n", width, height, src_width, src_height, dest_width, dest_height, ds );
#endif
				tnail = gdk_pixbuf_scale_simple ( pbuf, width, height, GDK_INTERP_BILINEAR );

				/* get string and tooltip to display */
				if ( objc == 5 || objc == 7 )
				{

					if ( strcmp (  Tcl_GetString ( objv[3] ), "-label" ) == 0 )
					{
#ifdef DEBUG_ICONVIEW
						g_print ( "label = %s\n", Tcl_GetString ( objv[4] ) );
#endif
						label = Tcl_GetString ( objv[4] );

					}

					if ( strcmp (  Tcl_GetString ( objv[5] ), "-tooltip" ) == 0 )
					{
#ifdef DEBUG_ICONVIEW
						g_print ( "tooltip = %s\n", Tcl_GetString ( objv[6] ) );
#endif
						tooltip = Tcl_GetString ( objv[6] );

					}
				}

				gtk_list_store_append ( para->list_store, &iter );
				//gtk_list_store_set ( para->list_store, &iter, COL_DISPLAY_NAME, label, COL_PIXBUF, p2, -1 );

				gtk_list_store_set ( para->list_store, &iter, COL_TNAIL, tnail, COL_LABEL, label, COL_FNAME, filename, COL_TOOLTIP, tooltip, -1 );

			}
			break;
		case ClassIdx:
			{
				Tcl_SetObjResult ( interp, Tcl_NewStringObj ( "iconView", -1 ) );
			}
			break;
		case DeleteIdx:
			{
				return gnoclDelete ( interp, GTK_WIDGET ( para->iconView ), objc, objv );
			}
		case ConfigureIdx:
			{
				int ret = TCL_ERROR;

				if ( gnoclParseAndSetOptions ( interp, objc - 1, objv + 1, iconViewOptions, G_OBJECT ( para->iconView ) ) == TCL_OK )
				{
					ret = configure ( interp, para->iconView, iconViewOptions );
				}

				gnoclClearOptions ( iconViewOptions );

				return ret;
			}

			break;
		case OnClickedIdx:
			{
				if ( objc != 2 )
				{
					Tcl_WrongNumArgs ( interp, 2, objv, NULL );
					return TCL_ERROR;
				}

				if ( GTK_WIDGET_IS_SENSITIVE ( GTK_WIDGET ( para->iconView ) ) )
				{
					gtk_button_clicked ( para->iconView );
				}
			}
			break;

		case CgetIdx:
			{
				int     idx;

				switch ( gnoclCget ( interp, objc, objv, G_OBJECT ( para->iconView ), iconViewOptions, &idx ) )
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
							return cget ( interp, para->iconView, iconViewOptions, idx );
						}
				}
			}
	}

	return TCL_OK;
}

/**
\brief
**/
int gnoclIconViewCmd ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{

#ifdef DEBUG_ICONVIEW
	g_print ( "gnoclIconViewCmd\n" );
#endif

	IconViewParams *para;
	int            ret = TCL_OK;

	/* STEP 1) -test options */
	if ( gnoclParseOptions ( interp, objc, objv, iconViewOptions ) != TCL_OK )
	{
		gnoclClearOptions ( iconViewOptions );
		return TCL_ERROR;
	}

	para = g_new ( IconViewParams, 1 );
	para->interp = interp;

	/* STEP 2) -build widget */
	para->scrolledWindow = gtk_scrolled_window_new ( NULL, NULL );
	gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( para->scrolledWindow ), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC );
	gtk_scrolled_window_set_shadow_type ( GTK_SCROLLED_WINDOW ( para->scrolledWindow ), GTK_SHADOW_IN );

	/* create a widget with a pre-existing tree */
	//para->list_store = gtk_list_store_new ( NUM_COLS, G_TYPE_STRING, GDK_TYPE_PIXBUF );
	para->list_store = gtk_list_store_new ( N_COLS, GDK_TYPE_PIXBUF, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING );
	para->iconView = gtk_icon_view_new_with_model ( para->list_store );

	/* automatically size icon to fit */
	//gtk_icon_view_set_item_width (para->iconView,50);

	/* set some default settings */
	/* widget */
	gtk_icon_view_set_text_column ( GTK_ICON_VIEW ( para->iconView ), COL_LABEL );
	gtk_icon_view_set_pixbuf_column ( GTK_ICON_VIEW ( para->iconView ), COL_TNAIL );
	gtk_icon_view_set_text_column ( GTK_ICON_VIEW ( para->iconView ), COL_FNAME );
	gtk_icon_view_set_text_column ( GTK_ICON_VIEW ( para->iconView ), COL_TOOLTIP );

	/* show tooltips by default */
	gtk_icon_view_set_tooltip_column    ( GTK_ICON_VIEW ( para->iconView ), COL_TOOLTIP );

	/* permit label text markup */
	gtk_icon_view_set_markup_column  ( GTK_ICON_VIEW ( para->iconView ), COL_LABEL );

	/* set selection mode */
	gtk_icon_view_set_selection_mode ( GTK_ICON_VIEW ( para->iconView ), GTK_SELECTION_SINGLE );

	/* set default item width */
	gtk_icon_view_set_item_width ( GTK_ICON_VIEW ( para->iconView ), ITEM_WIDTH );

	/* pack the widget and display */
	gtk_container_add ( GTK_CONTAINER ( para->scrolledWindow ), para->iconView );
	gtk_widget_show_all ( para->scrolledWindow );

	/* parameters */
	para->icon_width = gtk_icon_view_get_item_width ( para->iconView );

	/* STEP 3) -reconfigure according to options */
	ret = gnoclSetOptions ( interp, iconViewOptions, G_OBJECT ( para->iconView ), -1 );

	if ( ret == TCL_OK )
	{
		ret = configure ( interp, G_OBJECT ( para->iconView ), iconViewOptions );
	}

	gnoclClearOptions ( iconViewOptions );

	if ( ret != TCL_OK )
	{
		gtk_widget_destroy ( GTK_WIDGET ( para->iconView ) );
		return TCL_ERROR;
	}

	/* STEP 4)  -everything has worked, register the widget with the Tcl interpretor. */
	para->name = gnoclGetAutoWidgetId();
	g_signal_connect ( G_OBJECT ( para->scrolledWindow ), "destroy", G_CALLBACK ( destroyFunc ), para );
	gnoclMemNameAndWidget ( para->name, GTK_WIDGET ( para->scrolledWindow ) );
	Tcl_CreateObjCommand ( interp, para->name, iconViewFunc, para, NULL );
	Tcl_SetObjResult ( interp, Tcl_NewStringObj ( para->name, -1 ) );

}
