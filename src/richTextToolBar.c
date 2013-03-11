/*
   History:
   2013-02: Begin of developement
 */

/**
\page page_tbar gnocl::gtkrichTextToolBar
\htmlinclude RTtbar.html
**/

#include "gnocl.h"
#include "gnoclparams.h"

static const int textIdx = 0;

static GnoclOption richTextToolBarOptions[] =
{
	/* widget specific options */
	{ "-text", GNOCL_OBJ, NULL },

	/* general options */
	{ "-name", GNOCL_STRING, "name" },
	{ "-visible", GNOCL_BOOL, "visible" },
	{ "-tooltip", GNOCL_OBJ, "", gnoclOptTooltip },
	{ "-data", GNOCL_OBJ, "", gnoclOptData },
	{ NULL },
};


/**
\brief
**/
static void destroyFunc ( GtkWidget *widget, gpointer data )
{
#ifdef DEBUG_BUTTON
	printf ( "%s\n", __FUNCTION__ );
#endif

	RichTextToolbarParams *para = ( RichTextToolbarParams * ) data;

	gnoclForgetWidgetFromName ( para->name );
	Tcl_DeleteCommand ( para->interp, para->name );

	g_free ( para->name );
	g_free ( para );
}



/**
\brief	obtain current GtkTextBuffer from specified object
**/
static GtkTextBuffer *getTextBuffer ( gpointer user_data )
{
	TextParams *para = ( TextParams * ) user_data;

	GtkScrolledWindow   *scrolled = para->scrolled;
	GtkTextView     *text = GTK_TEXT_VIEW ( gtk_bin_get_child ( GTK_BIN ( scrolled ) ) );
	GtkTextBuffer  *buffer = gtk_text_view_get_buffer ( text );

	return buffer;
}

static void applyTag ( GtkTextBuffer *buffer, gchar *tagName )
{
	GtkTextIter start;
	GtkTextIter end;

	gtk_text_buffer_get_selection_bounds ( buffer, &start, &end );

	gtk_text_buffer_apply_tag_by_name ( buffer, tagName, &start, &end );
}

/**
\brief
**/
static void doClear ( GtkToolButton *toolbutton, gpointer user_data )
{
	gtk_text_buffer_insert_at_cursor ( getTextBuffer ( user_data ), "clear", 5 );
}


/**
\brief
**/
static void doBold ( GtkToolButton *toolbutton, gpointer user_data )
{
	GtkTextBuffer *buffer = getTextBuffer ( user_data );
	applyTag ( buffer, "<b>" );

}

/**
\brief
**/
static void doItalic ( GtkToolButton *toolbutton, gpointer user_data )
{
	GtkTextBuffer *buffer = getTextBuffer ( user_data );
	applyTag ( buffer, "<i>" );
}

/**
\brief
**/
static void doUnderline ( GtkToolButton *toolbutton, gpointer user_data )
{
	GtkTextBuffer *buffer = getTextBuffer ( user_data );
	applyTag ( buffer, "<u>" );
}


/**
\brief
**/
static void doStrikethrough ( GtkToolButton *toolbutton, gpointer user_data )
{
	//gtk_text_buffer_insert_at_cursor ( getTextBuffer ( user_data ), "strikethrough", 13 );
	GtkTextBuffer *buffer = getTextBuffer ( user_data );
	applyTag ( buffer, "<s>" );
}

/**
\brief
**/
static void doSuperscript ( GtkToolButton *toolbutton, gpointer user_data )
{
	GtkTextBuffer *buffer = getTextBuffer ( user_data );
	applyTag ( buffer, "<sup>" );
}

/**
\brief
**/
static void doSubscript ( GtkToolButton *toolbutton, gpointer user_data )
{
	GtkTextBuffer *buffer = getTextBuffer ( user_data );
	applyTag ( buffer, "<sub>" );
}

/**
\brief
**/
static void doBigger ( GtkToolButton *toolbutton, gpointer user_data )
{
	GtkTextBuffer *buffer = getTextBuffer ( user_data );
	applyTag ( buffer, "<big>" );
}

/**
\brief
**/
static void doSmaller ( GtkToolButton *toolbutton, gpointer user_data )
{
	GtkTextBuffer *buffer = getTextBuffer ( user_data );
	applyTag ( buffer, "<small>" );
}


/**
\brief
**/
static void doBg ( GtkToolButton *toolbutton, gpointer user_data )
{
	gtk_text_buffer_insert_at_cursor ( getTextBuffer ( user_data ), "bg", 2 );
}

/**
\brief
**/
static void doFg ( GtkToolButton *toolbutton, gpointer user_data )
{
	gtk_text_buffer_insert_at_cursor ( getTextBuffer ( user_data ), "fg", 2 );
}


/**
\brief
**/
static int configure ( Tcl_Interp *interp, RichTextToolbarParams *para, GnoclOption options[] )
{

	g_print ( "%s\n", __FUNCTION__ );

	if ( options[textIdx].status == GNOCL_STATUS_CHANGED )
	{
		g_print ( "%s\n", Tcl_GetString ( options[textIdx].val.obj ) );

		para->textView = gnoclGetWidgetFromName ( Tcl_GetString ( options[textIdx].val.obj ), interp );
	}


	return TCL_OK;
}

/**
\brief
**/
int richTextToolBarFunc ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{
	static const char *cmds[] =
	{
		"delete", "configure",
		"class",
		NULL
	};

	enum cmdIdx
	{
		DeleteIdx, ConfigureIdx, ClassIdx, SetIdx
	};

	RichTextToolbarParams *para = ( RichTextToolbarParams * ) data;


	int idx;

	if ( Tcl_GetIndexFromObj ( interp, objv[1], cmds, "command", TCL_EXACT, &idx ) != TCL_OK )
	{
		return TCL_ERROR;
	}

	switch ( idx )
	{
		case ClassIdx:
			{
				Tcl_SetObjResult ( interp, Tcl_NewStringObj ( "richTextToolBar", -1 ) );
			}
			break;
		case DeleteIdx:
			{
				return gnoclDelete ( interp, GTK_WIDGET ( para->toolBar ), objc, objv );
			}
		case ConfigureIdx:
			{
				int ret = TCL_ERROR;

				if ( gnoclParseAndSetOptions ( interp, objc - 1, objv + 1,
											   richTextToolBarOptions, G_OBJECT ( para->toolBar ) ) == TCL_OK )
				{
					ret = configure ( interp, para, richTextToolBarOptions );
				}

				gnoclClearOptions ( richTextToolBarOptions );

				return ret;
			}

			break;
	}

	return TCL_OK;
}

/**
\brief
**/
int gnoclRichTextToolBarCmd ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{
	int ret;

	if ( gnoclParseOptions ( interp, objc, objv, richTextToolBarOptions ) != TCL_OK )
	{
		gnoclClearOptions ( richTextToolBarOptions );
		return TCL_ERROR;
	}

	RichTextToolbarParams *para;
	para = g_new ( RichTextToolbarParams, 1 );

	GtkToolItem *clear, *bold, *italic, *underline, *strikethrough,
				*superscript, *subscript, *bigger, *smaller, *bg, *fg;

	GtkWidget *bgclrs, *fgclrs;

	clear = gtk_tool_button_new_from_stock ( GTK_STOCK_INFO );
	bold = gtk_tool_button_new_from_stock ( GTK_STOCK_BOLD );
	italic = gtk_tool_button_new_from_stock ( GTK_STOCK_ITALIC );
	underline = gtk_tool_button_new_from_stock ( GTK_STOCK_UNDERLINE );
	strikethrough = gtk_tool_button_new_from_stock ( GTK_STOCK_STRIKETHROUGH );

	superscript = gtk_tool_button_new_from_stock ( GTK_STOCK_INFO );
	subscript = gtk_tool_button_new_from_stock ( GTK_STOCK_INFO );

	bigger = gtk_tool_button_new_from_stock ( GTK_STOCK_INFO );
	smaller = gtk_tool_button_new_from_stock ( GTK_STOCK_INFO );

	bg = gtk_menu_tool_button_new_from_stock ( GTK_STOCK_OPEN );
	fg = gtk_menu_tool_button_new_from_stock ( GTK_STOCK_CLOSE );


	gtk_widget_set_tooltip_text ( GTK_WIDGET ( clear ) , "Clear formatting from selected text." );
	gtk_widget_set_tooltip_text ( GTK_WIDGET ( bold ) , "Enbolden selected text." );
	gtk_widget_set_tooltip_text ( GTK_WIDGET ( italic ) , "Italicize selected text." );
	gtk_widget_set_tooltip_text ( GTK_WIDGET ( underline ) , "Underline selected text." );
	gtk_widget_set_tooltip_text ( GTK_WIDGET ( strikethrough ) , "Strikethrough selected text." );

	gtk_widget_set_tooltip_text ( GTK_WIDGET ( superscript ) , "Convert selected text to superscript." );
	gtk_widget_set_tooltip_text ( GTK_WIDGET ( subscript ) , "Concert selected text to subscript." );

	gtk_widget_set_tooltip_text ( GTK_WIDGET ( bigger ) , "Increase the size of the selected text." );
	gtk_widget_set_tooltip_text ( GTK_WIDGET ( smaller ) , "Reduce the size of the selected text." );

	gtk_widget_set_tooltip_text ( GTK_WIDGET ( bg ) , "Set the background colour of the selected text." );
	gtk_widget_set_tooltip_text ( GTK_WIDGET ( fg ) , "Set the foreground colour of the selected text." );


	bgclrs = gtk_menu_new();
	fgclrs = gtk_menu_new();


	/* create the toolbar, add the widgets */
	para->toolBar = GTK_TOOLBAR ( gtk_toolbar_new() );
	gtk_toolbar_set_show_arrow ( para->toolBar, TRUE );

	/*
	 GTK_TOOLBAR_ICONS,
	 GTK_TOOLBAR_TEXT,
	 GTK_TOOLBAR_BOTH,
	 GTK_TOOLBAR_BOTH_HORIZ
	*/
	gtk_toolbar_set_style ( GTK_TOOLBAR ( para->toolBar ), GTK_TOOLBAR_ICONS );

	/*
	 GTK_ICON_SIZE_INVALID
	 GTK_ICON_SIZE_MENU
	 GTK_ICON_SIZE_SMALL_TOOLBAR
	 GTK_ICON_SIZE_LARGE_TOOLBAR
	 GTK_ICON_SIZE_BUTTON
	 GTK_ICON_SIZE_DND
	 GTK_ICON_SIZE_DIALOG
	*/

	gtk_toolbar_set_icon_size ( GTK_TOOLBAR ( para->toolBar ), GTK_ICON_SIZE_MENU );

	gtk_toolbar_insert ( GTK_TOOLBAR ( para->toolBar ), clear, -1 );
	gtk_toolbar_insert ( GTK_TOOLBAR ( para->toolBar ), bold, -1 );
	gtk_toolbar_insert ( GTK_TOOLBAR ( para->toolBar ), italic, -1 );
	gtk_toolbar_insert ( GTK_TOOLBAR ( para->toolBar ), underline, -1 );
	gtk_toolbar_insert ( GTK_TOOLBAR ( para->toolBar ), strikethrough, -1 );
	gtk_toolbar_insert ( GTK_TOOLBAR ( para->toolBar ), gtk_separator_tool_item_new(), -1 );
	gtk_toolbar_insert ( GTK_TOOLBAR ( para->toolBar ), superscript, -1 );
	gtk_toolbar_insert ( GTK_TOOLBAR ( para->toolBar ), subscript, -1 );
	gtk_toolbar_insert ( GTK_TOOLBAR ( para->toolBar ), gtk_separator_tool_item_new(), -1 );
	gtk_toolbar_insert ( GTK_TOOLBAR ( para->toolBar ), bigger, -1 );
	gtk_toolbar_insert ( GTK_TOOLBAR ( para->toolBar ), smaller, -1 );

	gtk_toolbar_insert ( GTK_TOOLBAR ( para->toolBar ), gtk_separator_tool_item_new(), -1 );
	gtk_toolbar_insert ( GTK_TOOLBAR ( para->toolBar ), bg, -1 );
	gtk_toolbar_insert ( GTK_TOOLBAR ( para->toolBar ), fg, -1 );


	/* event handlers */
	g_signal_connect ( G_OBJECT ( clear ), "clicked", G_CALLBACK ( doClear ), ( gpointer ) para );
	g_signal_connect ( G_OBJECT ( bold ), "clicked", G_CALLBACK ( doBold ), ( gpointer ) para );
	g_signal_connect ( G_OBJECT ( italic ), "clicked", G_CALLBACK ( doItalic ), ( gpointer ) para );
	g_signal_connect ( G_OBJECT ( underline ), "clicked", G_CALLBACK ( doUnderline ), ( gpointer ) para );
	g_signal_connect ( G_OBJECT ( strikethrough ), "clicked", G_CALLBACK ( doStrikethrough ), ( gpointer ) para );

	g_signal_connect ( G_OBJECT ( superscript ), "clicked", G_CALLBACK ( doSuperscript ), ( gpointer ) para );
	g_signal_connect ( G_OBJECT ( subscript ), "clicked", G_CALLBACK ( doSubscript ), ( gpointer ) para );

	g_signal_connect ( G_OBJECT ( bigger ), "clicked", G_CALLBACK ( doBigger ), ( gpointer ) para );
	g_signal_connect ( G_OBJECT ( smaller ), "clicked", G_CALLBACK ( doSmaller ), ( gpointer ) para );

	g_signal_connect ( G_OBJECT ( bg ), "clicked", G_CALLBACK ( doBg ), ( gpointer ) para );
	g_signal_connect ( G_OBJECT ( fg ), "clicked", G_CALLBACK ( doFg ), ( gpointer ) para );

	gtk_widget_show_all ( GTK_WIDGET ( para->toolBar ) );



	/* step 3) check the options passed for the creation of the widget */
	ret = gnoclSetOptions ( interp, richTextToolBarOptions, G_OBJECT ( para->toolBar ), -1 );

	/* step 4) if this is ok, then configure the new widget */
	if ( ret == TCL_OK )
	{
		ret = configure ( interp, para, richTextToolBarOptions );
	}

	/* step 5) clear the memory set assigned to the options */
	gnoclClearOptions ( richTextToolBarOptions );

	if ( ret != TCL_OK )
	{
		gtk_widget_destroy ( GTK_WIDGET ( para->toolBar ) );
		return TCL_ERROR;
	}


	/* TODO: if not -visible == 0 */
	gtk_widget_show ( GTK_WIDGET ( para->toolBar ) );

//	return gnoclRegisterWidget ( interp, GTK_WIDGET ( para->toolBar ), richTextToolBarFunc );

	para->name = gnoclGetAutoWidgetId();
	g_signal_connect ( G_OBJECT ( para->toolBar ), "destroy", G_CALLBACK ( destroyFunc ), para );
	gnoclMemNameAndWidget ( para->name, GTK_WIDGET ( para->toolBar ) );
	Tcl_CreateObjCommand ( interp, para->name, buttonFunc, para, NULL );
	Tcl_SetObjResult ( interp, Tcl_NewStringObj ( para->name, -1 ) );
}

