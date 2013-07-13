/*
   History:
   2013-07: added commands, options, commands
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
static void removeBgTags ( GtkTextBuffer *buffer )
{

	GtkTextIter start;
	GtkTextIter end;

	gtk_text_buffer_get_selection_bounds ( buffer, &start, &end );

	/* remove all bg tags */
	gtk_text_buffer_remove_tag_by_name  ( buffer, "<span background='cyan'>", &start, &end );
	gtk_text_buffer_remove_tag_by_name  ( buffer, "<span background='magenta'>", &start, &end );
	gtk_text_buffer_remove_tag_by_name  ( buffer, "<span background='yellow'>", &start, &end );
	gtk_text_buffer_remove_tag_by_name  ( buffer, "<span background='orange'>", &start, &end );
}

/**
\brief
**/
static void removeFgTags ( GtkTextBuffer *buffer )
{

	GtkTextIter start;
	GtkTextIter end;

	gtk_text_buffer_get_selection_bounds ( buffer, &start, &end );

	/* remove all fg tags */
	gtk_text_buffer_remove_tag_by_name  ( buffer, "<span foreground='red'>", &start, &end );
	gtk_text_buffer_remove_tag_by_name  ( buffer, "<span foreground='green'>", &start, &end );
	gtk_text_buffer_remove_tag_by_name  ( buffer, "<span foreground='blue'>", &start, &end );
	gtk_text_buffer_remove_tag_by_name  ( buffer, "<span foreground='gray'>", &start, &end );
}

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

/**
\brief
**/
static void removeTag ( GtkTextBuffer *buffer, gchar *tagName )
{
	GtkTextIter start;
	GtkTextIter end;

	gtk_text_buffer_get_selection_bounds ( buffer, &start, &end );

	gtk_text_buffer_remove_tag_by_name ( buffer, tagName, &start, &end );
}

/**
\brief
**/
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

	RichTextToolbarParams *para = ( RichTextToolbarParams * ) user_data;
	GtkTextBuffer  *buffer = gtk_text_view_get_buffer ( GTK_TEXT_VIEW ( para->textView ) );

	GtkTextIter start;
	GtkTextIter end;

	gtk_text_buffer_get_selection_bounds ( buffer, &start, &end );

	gtk_text_buffer_remove_tag_by_name  ( buffer, "<b>", &start, &end );
	gtk_text_buffer_remove_tag_by_name  ( buffer, "<u>", &start, &end );
	gtk_text_buffer_remove_tag_by_name  ( buffer, "<i>", &start, &end );
	gtk_text_buffer_remove_tag_by_name  ( buffer, "<s>", &start, &end );

	gtk_text_buffer_remove_tag_by_name  ( buffer, "<sub>", &start, &end );
	gtk_text_buffer_remove_tag_by_name  ( buffer, "<sup>", &start, &end );
	gtk_text_buffer_remove_tag_by_name  ( buffer, "<big>", &start, &end );
	gtk_text_buffer_remove_tag_by_name  ( buffer, "<small>", &start, &end );

}


/**
\brief
**/
static void doClearBg ( GtkToolButton *toolbutton, gpointer user_data )
{
	RichTextToolbarParams *para = ( RichTextToolbarParams * ) user_data;

	GtkTextBuffer *buffer = getTextBuffer ( user_data );
	removeBgTags ( buffer );
}


/**
\brief
**/
static void doClearFg ( GtkToolButton *toolbutton, gpointer user_data )
{
	RichTextToolbarParams *para = ( RichTextToolbarParams * ) user_data;

	GtkTextBuffer *buffer = getTextBuffer ( user_data );
	removeFgTags ( buffer );
}

/**
\brief
**/
static void doYellowBg ( GtkToolButton *toolbutton, gpointer user_data )
{
	RichTextToolbarParams *para = ( RichTextToolbarParams * ) user_data;
	para->bgClr = "<span background='yellow'>";
	gtk_widget_destroy ( para->bgImg );
	para->bgImg = gtk_image_new_from_file ( "./yellowBg.png"  );
	gtk_widget_show ( para->bgImg );
	gtk_tool_button_set_icon_widget ( para->bg, para->bgImg );
}

/**
\brief
**/
static void doMagentaBg ( GtkToolButton *toolbutton, gpointer user_data )
{
	RichTextToolbarParams *para = ( RichTextToolbarParams * ) user_data;
	para->bgClr = "<span background='magenta'>";
	gtk_widget_destroy ( para->bgImg );
	para->bgImg = gtk_image_new_from_file ( "./magentaBg.png"  );
	gtk_widget_show ( para->bgImg );
	gtk_tool_button_set_icon_widget ( para->bg, para->bgImg );
}

/**
\brief
**/
static void doCyanBg ( GtkToolButton *toolbutton, gpointer user_data )
{
	RichTextToolbarParams *para = ( RichTextToolbarParams * ) user_data;
	para->bgClr = "<span background='cyan'>";
	gtk_widget_destroy ( para->bgImg );
	para->bgImg = gtk_image_new_from_file ( "./cyanBg.png"  );
	gtk_widget_show ( para->bgImg );
	gtk_tool_button_set_icon_widget ( para->bg, para->bgImg );
}

/**
\brief
**/
static void doOrangeBg ( GtkToolButton *toolbutton, gpointer user_data )
{
	RichTextToolbarParams *para = ( RichTextToolbarParams * ) user_data;
	para->bgClr = "<span background='orange'>";
	gtk_widget_destroy ( para->bgImg );
	para->bgImg = gtk_image_new_from_file ( "./orangeBg.png"  );
	gtk_widget_show ( para->bgImg );
	gtk_tool_button_set_icon_widget ( para->bg, para->bgImg );
}

/**
\brief	Configure menubutton settings
\note	This cannot be done directly. The menubutton needs to be replaced
		with a new item with the desired icon.
**/
static void doCyanBg_ ( GtkToolButton *toolbutton, gpointer user_data )
{

	RichTextToolbarParams *para = ( RichTextToolbarParams * ) user_data;
	para->bgClr = "<span background='cyan'>";

	/* get the position in the toolbar and a copy of the menu pointer */
	gint idx = gtk_toolbar_get_item_index ( para->toolBar, para->bg );
	GtkWidget *mnu = gtk_menu_tool_button_get_menu ( para->bg );

	/* release the sub-menu, detroy the menubutton */
	gtk_menu_tool_button_set_menu ( para->bg, NULL );
	gtk_widget_destroy ( para->bg );

	/* create a new button, assign menu and insert into toolbar */
	para->bgImg = gtk_image_new_from_file ( "./cyanBg.png"  );
	para->bg =  gtk_menu_tool_button_new ( para->bgImg, "" );
	gtk_menu_tool_button_set_menu ( para->bg, mnu );
	gtk_widget_show ( para->bg );
	gtk_tool_button_set_icon_widget ( para->bg, para->bgImg );
	gtk_toolbar_insert ( para->toolBar, para->bg, idx );

}

/**
\brief
**/
static void doRedFg ( GtkToolButton *toolbutton, gpointer user_data )
{
	RichTextToolbarParams *para = ( RichTextToolbarParams * ) user_data;
	para->fgClr = "<span foreground='red'>";
	gtk_widget_destroy ( para->fgImg );
	para->fgImg = gtk_image_new_from_file ( "./redFg.png"  );
	gtk_widget_show ( para->fgImg );
	gtk_tool_button_set_icon_widget ( para->fg, para->fgImg );
}

/**
\brief
**/
static void doGreenFg ( GtkToolButton *toolbutton, gpointer user_data )
{
	RichTextToolbarParams *para = ( RichTextToolbarParams * ) user_data;
	para->fgClr = "<span foreground='green'>";
	gtk_widget_destroy ( para->fgImg );
	para->fgImg = gtk_image_new_from_file ( "./greenFg.png"  );
	gtk_widget_show ( para->fgImg );
	gtk_tool_button_set_icon_widget ( para->fg, para->fgImg );
}

/**
\brief
**/
static void doBlueFg ( GtkToolButton *toolbutton, gpointer user_data )
{
	RichTextToolbarParams *para = ( RichTextToolbarParams * ) user_data;
	para->fgClr = "<span foreground='blue'>";
	gtk_widget_destroy ( para->fgImg );
	para->fgImg = gtk_image_new_from_file ( "./blueFg.png"  );
	gtk_widget_show ( para->fgImg );
	gtk_tool_button_set_icon_widget ( para->fg, para->fgImg );
}

/**
\brief
**/
static void doGrayFg ( GtkToolButton *toolbutton, gpointer user_data )
{
	RichTextToolbarParams *para = ( RichTextToolbarParams * ) user_data;
	para->fgClr = "<span foreground='gray'>";
	gtk_widget_destroy ( para->fgImg );
	para->fgImg = gtk_image_new_from_file ( "./grayFg.png"  );
	gtk_widget_show ( para->fgImg );
	gtk_tool_button_set_icon_widget ( para->fg, para->fgImg );
}


/**
\brief
**/
static void doBold ( GtkToolButton *toolbutton, gpointer user_data )
{
	RichTextToolbarParams *para = ( RichTextToolbarParams * ) user_data;
	GtkTextBuffer  *buffer = gtk_text_view_get_buffer ( GTK_TEXT_VIEW ( para->textView ) );
	applyTag ( buffer, "<b>" );

}

/**
\brief
**/
static void doItalic ( GtkToolButton *toolbutton, gpointer user_data )
{
	RichTextToolbarParams *para = ( RichTextToolbarParams * ) user_data;
	GtkTextBuffer  *buffer = gtk_text_view_get_buffer ( GTK_TEXT_VIEW ( para->textView ) );
	applyTag ( buffer, "<i>" );
}

/**
\brief
**/
static void doUnderline ( GtkToolButton *toolbutton, gpointer user_data )
{
	RichTextToolbarParams *para = ( RichTextToolbarParams * ) user_data;
	GtkTextBuffer  *buffer = gtk_text_view_get_buffer ( GTK_TEXT_VIEW ( para->textView ) );
	applyTag ( buffer, "<u>" );
}


/**
\brief
**/
static void doStrikethrough ( GtkToolButton *toolbutton, gpointer user_data )
{
	RichTextToolbarParams *para = ( RichTextToolbarParams * ) user_data;
	GtkTextBuffer  *buffer = gtk_text_view_get_buffer ( GTK_TEXT_VIEW ( para->textView ) );
	applyTag ( buffer, "<s>" );
}

/**
\brief
**/
static void doSuperscript ( GtkToolButton *toolbutton, gpointer user_data )
{
	RichTextToolbarParams *para = ( RichTextToolbarParams * ) user_data;
	GtkTextBuffer  *buffer = gtk_text_view_get_buffer ( GTK_TEXT_VIEW ( para->textView ) );
	removeTag ( buffer , "<sub>" );
	removeTag ( buffer , "<sup>" );
	applyTag ( buffer, "<sup>" );
}

/**
\brief
**/
static void doSubscript ( GtkToolButton *toolbutton, gpointer user_data )
{
	RichTextToolbarParams *para = ( RichTextToolbarParams * ) user_data;
	GtkTextBuffer  *buffer = gtk_text_view_get_buffer ( GTK_TEXT_VIEW ( para->textView ) );
	removeTag ( buffer , "<sub>" );
	removeTag ( buffer , "<sup>" );
	applyTag ( buffer, "<sub>" );
}

/**
\brief
**/
static void doBigger ( GtkToolButton *toolbutton, gpointer user_data )
{
	RichTextToolbarParams *para = ( RichTextToolbarParams * ) user_data;
	GtkTextBuffer  *buffer = gtk_text_view_get_buffer ( GTK_TEXT_VIEW ( para->textView ) );
	removeTag ( buffer , "<big>" );
	removeTag ( buffer , "<small>" );
	applyTag ( buffer, "<big>" );
}

/**
\brief
**/
static void doSmaller ( GtkToolButton *toolbutton, gpointer user_data )
{
	RichTextToolbarParams *para = ( RichTextToolbarParams * ) user_data;
	GtkTextBuffer  *buffer = gtk_text_view_get_buffer ( GTK_TEXT_VIEW ( para->textView ) );
	removeTag ( buffer , "<big>" );
	removeTag ( buffer , "<small>" );
	applyTag ( buffer, "<small>" );
}


/**
\brief
**/
static void doBg ( GtkToolButton *toolbutton, gpointer user_data )
{

	RichTextToolbarParams *para = ( RichTextToolbarParams * ) user_data;

	if ( para->bgClr != "" )
	{
		GtkTextBuffer  *buffer = gtk_text_view_get_buffer ( GTK_TEXT_VIEW ( para->textView ) );
		removeBgTags ( buffer );
		applyTag ( buffer, para->bgClr );
	}

}

/**
\brief
**/
static void doFg ( GtkToolButton *toolbutton, gpointer user_data )
{
	RichTextToolbarParams *para = ( RichTextToolbarParams * ) user_data;

	if ( para->fgClr != "" )
	{
		GtkTextBuffer  *buffer = gtk_text_view_get_buffer ( GTK_TEXT_VIEW ( para->textView ) );
		removeFgTags ( buffer );
		applyTag ( buffer, para->fgClr );
	}
}




/**
\brief
**/
static int configure ( Tcl_Interp *interp, RichTextToolbarParams *para, GnoclOption options[] )
{

	if ( options[textIdx].status == GNOCL_STATUS_CHANGED )
	{
		GtkScrolledWindow *scrolled = gnoclGetWidgetFromName ( Tcl_GetString ( options[textIdx].val.obj ), interp );
		para->textView = GTK_TEXT_VIEW ( gtk_bin_get_child ( GTK_BIN ( scrolled ) ) );
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
		"class", "options", "commands",
		NULL
	};

	enum cmdIdx
	{
		DeleteIdx, ConfigureIdx, ClassIdx, SetIdx, OptionsIdx, CommandsIdx
	};

	RichTextToolbarParams *para = ( RichTextToolbarParams * ) data;


	int idx;

	if ( Tcl_GetIndexFromObj ( interp, objv[1], cmds, "command", TCL_EXACT, &idx ) != TCL_OK )
	{
		return TCL_ERROR;
	}

	switch ( idx )
	{
		case CommandsIdx:
			{
				gnoclGetOptions ( interp, cmds );
			}
			break;
		case OptionsIdx:
			{
				gnoclGetOptions ( interp, richTextToolBarOptions );
			}
			break;
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

	para->fgClr = "";
	para->bgClr = "<span background='yellow'>";


	GtkToolItem *clear, *bold, *italic, *underline, *strikethrough,
				*superscript, *subscript, *bigger, *smaller;

	GtkWidget *bgclrs, *fgclrs;
	GtkAccelGroup *group;

	GtkWidget *clearImg = gtk_image_new_from_file ( "./normal.png" );
	clear = gtk_tool_button_new  ( clearImg, "" );
	gtk_tool_button_set_icon_widget ( clear, clearImg );

	GtkWidget *boldImg = gtk_image_new_from_file ( "./bold.png" );
	bold = gtk_tool_button_new  ( boldImg, "" );
	gtk_tool_button_set_icon_widget ( bold, boldImg );

	/*
	typedef struct {
	  guint32 pixel;
	  guint16 red;
	  guint16 green;
	  guint16 blue;
	} GdkColor;
	*/

	/*
	 typedef enum {
	  GTK_STATE_NORMAL,
	  GTK_STATE_ACTIVE,
	  GTK_STATE_PRELIGHT,
	  GTK_STATE_SELECTED,
	  GTK_STATE_INSENSITIVE,
	  GTK_STATE_INCONSISTENT,
	  GTK_STATE_FOCUSED
	} GtkStateType;
	*/
	GdkColor color;
	color.red = 65535;
	color.green = 0;
	color.blue = 0;

	gtk_widget_modify_bg ( bold, GTK_STATE_NORMAL, &color );


	GtkWidget *italicImg = gtk_image_new_from_file ( "./italic.png" );
	italic = gtk_tool_button_new  ( italicImg, "" );
	gtk_tool_button_set_icon_widget ( italic, italicImg );

	GtkWidget *underlineImg = gtk_image_new_from_file ( "./underline.png" );
	underline = gtk_tool_button_new  ( underlineImg, "" );
	gtk_tool_button_set_icon_widget ( underline, underlineImg );

	GtkWidget *strikethroughImg = gtk_image_new_from_file ( "./strikethrough.png" );
	strikethrough = gtk_tool_button_new  ( strikethroughImg, "" );
	gtk_tool_button_set_icon_widget ( strikethrough, strikethroughImg );

	GtkWidget *superscriptImg = gtk_image_new_from_file ( "./superscript.png" );
	superscript = gtk_tool_button_new  ( superscriptImg, "" );
	gtk_tool_button_set_icon_widget ( superscript, superscriptImg );

	GtkWidget *subscriptImg = gtk_image_new_from_file ( "./subscript.png" );
	subscript = gtk_tool_button_new  ( subscriptImg, "" );
	gtk_tool_button_set_icon_widget ( subscript, subscriptImg );

	GtkWidget *biggerImg = gtk_image_new_from_file ( "./bigger.png" );
	bigger = gtk_tool_button_new  ( biggerImg, "" );
	gtk_tool_button_set_icon_widget ( bigger, biggerImg );

	GtkWidget *smallerImg = gtk_image_new_from_file ( "./smaller.png" );
	smaller = gtk_tool_button_new  ( smallerImg, "" );
	gtk_tool_button_set_icon_widget ( smaller, smallerImg );

	para->bgImg = gtk_image_new_from_file ( "./background.png" );
	para->bg =  gtk_menu_tool_button_new ( para->bgImg, "" );
	gtk_tool_button_set_icon_widget ( para->bg, para->bgImg );


	para->fgImg = gtk_image_new_from_file ( "./foreground.png" );
	para->fg =  gtk_menu_tool_button_new   ( para->fgImg, "" );
	gtk_tool_button_set_icon_widget ( para->fg, para->fgImg );


	group = gtk_accel_group_new();
	bgclrs = gtk_menu_new();
	fgclrs = gtk_menu_new();

	gtk_menu_tool_button_set_menu ( para->bg, bgclrs );
	gtk_menu_tool_button_set_menu ( para->fg, fgclrs );

	GtkWidget *n1 = gtk_image_menu_item_new_with_mnemonic ( "_None" );
	GtkWidget *a1 = gtk_image_menu_item_new_with_mnemonic ( "_Yellow" );
	GtkWidget *b1 = gtk_image_menu_item_new_with_mnemonic ( "_Cyan" );
	GtkWidget *c1 = gtk_image_menu_item_new_with_mnemonic ( "_Magenta" );
	GtkWidget *d1 = gtk_image_menu_item_new_with_mnemonic ( "_Orange" );

	gtk_menu_shell_append ( GTK_MENU_SHELL ( bgclrs ), n1 );
	gtk_menu_shell_append ( GTK_MENU_SHELL ( bgclrs ), a1 );
	gtk_menu_shell_append ( GTK_MENU_SHELL ( bgclrs ), b1 );
	gtk_menu_shell_append ( GTK_MENU_SHELL ( bgclrs ), c1 );
	gtk_menu_shell_append ( GTK_MENU_SHELL ( bgclrs ), d1 );


	g_signal_connect ( G_OBJECT ( n1 ), "activate", G_CALLBACK ( doClearBg ), ( gpointer ) para );

	/* load image for 'a' */
	GtkWidget *imageA = gtk_image_new_from_file ( "./yellow.png" );
	gtk_image_menu_item_set_image ( a1, imageA );
	gtk_image_menu_item_set_always_show_image ( a1, TRUE );
	g_signal_connect ( G_OBJECT ( a1 ), "activate", G_CALLBACK ( doYellowBg ), ( gpointer ) para );

	GtkWidget *imageB = gtk_image_new_from_file ( "./cyan.png" );
	gtk_image_menu_item_set_image ( b1, imageB );
	gtk_image_menu_item_set_always_show_image ( b1, TRUE );
	g_signal_connect ( G_OBJECT ( b1 ), "activate", G_CALLBACK ( doCyanBg ), ( gpointer ) para );

	GtkWidget *imageC = gtk_image_new_from_file ( "./magenta.png" );
	gtk_image_menu_item_set_image ( c1, imageC );
	gtk_image_menu_item_set_always_show_image ( c1, TRUE );
	g_signal_connect ( G_OBJECT ( c1 ), "activate", G_CALLBACK ( doMagentaBg ), ( gpointer ) para );

	GtkWidget *imageD = gtk_image_new_from_file ( "./orange.png" );
	gtk_image_menu_item_set_image ( d1, imageD );
	gtk_image_menu_item_set_always_show_image ( d1, TRUE );
	g_signal_connect ( G_OBJECT ( d1 ), "activate", G_CALLBACK ( doOrangeBg ), ( gpointer ) para );

	GtkWidget *n2 = gtk_image_menu_item_new_with_mnemonic ( "_None" );
	GtkWidget *a2 = gtk_image_menu_item_new_with_mnemonic ( "_Red" );
	GtkWidget *b2 = gtk_image_menu_item_new_with_mnemonic ( "_Green" );
	GtkWidget *c2 = gtk_image_menu_item_new_with_mnemonic ( "_Blue" );
	GtkWidget *d2 = gtk_image_menu_item_new_with_mnemonic ( "_Gray" );


	/*------------------------------------------------------------------*/

	g_signal_connect ( G_OBJECT ( n1 ), "activate", G_CALLBACK ( doClearFg ), ( gpointer ) para );

	/* load image for 'a' */
	GtkWidget *imageE = gtk_image_new_from_file ( "./red.png" );
	gtk_image_menu_item_set_image ( a2, imageE );
	gtk_image_menu_item_set_always_show_image ( a2, TRUE );
	g_signal_connect ( G_OBJECT ( a2 ), "activate", G_CALLBACK ( doRedFg ), ( gpointer ) para );

	GtkWidget *imageF = gtk_image_new_from_file ( "./green.png" );
	gtk_image_menu_item_set_image ( b2, imageF );
	gtk_image_menu_item_set_always_show_image ( b2, TRUE );
	g_signal_connect ( G_OBJECT ( b2 ), "activate", G_CALLBACK ( doGreenFg ), ( gpointer ) para );

	GtkWidget *imageG = gtk_image_new_from_file ( "./blue.png" );
	gtk_image_menu_item_set_image ( c2, imageG );
	gtk_image_menu_item_set_always_show_image ( c2, TRUE );
	g_signal_connect ( G_OBJECT ( c2 ), "activate", G_CALLBACK ( doBlueFg ), ( gpointer ) para );

	GtkWidget *imageH = gtk_image_new_from_file ( "./gray.png" );
	gtk_image_menu_item_set_image ( d2, imageH );
	gtk_image_menu_item_set_always_show_image ( d2, TRUE );
	g_signal_connect ( G_OBJECT ( d2 ), "activate", G_CALLBACK ( doGrayFg ), ( gpointer ) para );

	gtk_menu_shell_append ( GTK_MENU_SHELL ( fgclrs ), n2 );
	gtk_menu_shell_append ( GTK_MENU_SHELL ( fgclrs ), a2 );
	gtk_menu_shell_append ( GTK_MENU_SHELL ( fgclrs ), b2 );
	gtk_menu_shell_append ( GTK_MENU_SHELL ( fgclrs ), c2 );
	gtk_menu_shell_append ( GTK_MENU_SHELL ( fgclrs ), d2 );

	gtk_widget_show_all ( bgclrs );
	gtk_widget_show_all ( fgclrs );

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
	gtk_toolbar_insert ( GTK_TOOLBAR ( para->toolBar ), para->bg, -1 );
	gtk_toolbar_insert ( GTK_TOOLBAR ( para->toolBar ), para->fg, -1 );



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

	g_signal_connect ( G_OBJECT ( para->bg ), "clicked", G_CALLBACK ( doBg ), ( gpointer ) para );
	g_signal_connect ( G_OBJECT ( para->fg ), "clicked", G_CALLBACK ( doFg ), ( gpointer ) para );

	gtk_widget_show_all ( GTK_WIDGET ( para->toolBar ) );

	/* set tooltips */
	gtk_widget_set_tooltip_text ( GTK_WIDGET ( clear ) , "Clear formatting from selected text." );
	gtk_widget_set_tooltip_text ( GTK_WIDGET ( bold ) , "Enbolden selected text." );
	gtk_widget_set_tooltip_text ( GTK_WIDGET ( italic ) , "Italicize selected text." );
	gtk_widget_set_tooltip_text ( GTK_WIDGET ( underline ) , "Underline selected text." );
	gtk_widget_set_tooltip_text ( GTK_WIDGET ( strikethrough ) , "Strikethrough selected text." );

	gtk_widget_set_tooltip_text ( GTK_WIDGET ( superscript ) , "Convert selected text to superscript." );
	gtk_widget_set_tooltip_text ( GTK_WIDGET ( subscript ) , "Concert selected text to subscript." );

	gtk_widget_set_tooltip_text ( GTK_WIDGET ( bigger ) , "Increase the size of the selected text." );
	gtk_widget_set_tooltip_text ( GTK_WIDGET ( smaller ) , "Reduce the size of the selected text." );

	gtk_widget_set_tooltip_text ( GTK_WIDGET ( para->bg ) , "Set the background colour of the selected text." );
	gtk_widget_set_tooltip_text ( GTK_WIDGET ( para->fg ) , "Set the foreground colour of the selected text." );



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

