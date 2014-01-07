/* button.c
authors	Peter G. Baum, William J Giddings
date 	2001-03:
*/

/**
\page page_button gnocl::button
\htmlinclude button.html
**/

/**
 \par Modification History
 \verbatim
 2013-07: added commands, options, commands
 2013-04: added -baseFont
 2012-11: added -align
 2012-09: switched to the use of ButtonParams
 2009-02: added -widthReuest, -heightRequest
 2009-01: added geometry
      12: added parent
 2008-10: added class
 2004-02: added -data
      09: added cget
          renamed subcommand "invoke" to "onClicked"
 2003-03: added -icon
      08: switched from GnoclWidgetOptions to GnoclOption
          many cleanups, e.g. no more associated parameters
          renamed -command to -onClicked
      04: update for gtk 2.0
 2002-01: new command "invoke"
      09: underlined accelerators
 2001-03: Begin of developement
 \endverbatim

 \todo
 button - BLAH, BLAH
**/

#include "gnocl.h"
// #include "gnoclparams.h"


static const int textIdx 	= 0;
static const int iconIdx 	= 1;
static const int alignIdx 	= 2;
static const int baseFontIdx = 3;

//static const int dataIdx 	= 3;

static GnoclOption buttonOptions[] =
{
	/* GtkWidget specific options */
	{ "-text", GNOCL_OBJ, NULL },    /* 0 */
	{ "-icon", GNOCL_OBJ, NULL },    /* 1 */
	{ "-align", GNOCL_OBJ, NULL },   /* 2 */
	{ "-baseFont", GNOCL_OBJ, NULL }, /*  */

	//{ "-data", GNOCL_STRING, NULL }, /* 3 */



	{ "-activeBackgroundColor", GNOCL_OBJ, "active", gnoclOptGdkColorBg }, // 2

	/* GtkContainer Properties */
	{ "-borderWidth", GNOCL_OBJ, "border-width", gnoclOptPadding }, //3

	/* GtkObject Properties */

	//{ "-data", GNOCL_OBJ, "", gnoclOptData },

	{ "-hasFocus", GNOCL_BOOL, "has-focus" }, //6
	{ "-heightGroup", GNOCL_OBJ, "h", gnoclOptSizeGroup }, //7
	{ "-name", GNOCL_STRING, "name" },
	{ "-normalBackgroundColor", GNOCL_OBJ, "normal", gnoclOptGdkColorBg },

	/* GtkButton specific signals, "activate" is not used */
	{ "-onEnter", GNOCL_OBJ, "E", gnoclOptOnEnterLeave },
	{ "-onLeave", GNOCL_OBJ, "L", gnoclOptOnEnterLeave },
	{ "-onClicked", GNOCL_OBJ, "clicked", gnoclOptOnClicked }, //gnoclOptOnButtonClicked }
	{ "-onButtonPress", GNOCL_OBJ, "P", gnoclOptOnButton },
	{ "-onButtonRelease", GNOCL_OBJ, "R", gnoclOptOnButton },

	/* GtkWidget signals */
	{ "-onPopupMenu", GNOCL_OBJ, "popup-menu", gnoclOptCommand },
	{ "-onRealize", GNOCL_OBJ, "realize", gnoclOptCommand },
	{ "-onShowHelp", GNOCL_OBJ, "", gnoclOptOnShowHelp },
	{ "-prelightBackgroundColor", GNOCL_OBJ, "prelight", gnoclOptGdkColorBg },
	{ "-relief", GNOCL_OBJ, "relief", gnoclOptRelief },
	{ "-sensitive", GNOCL_BOOL, "sensitive" },
	{ "-sizeGroup", GNOCL_OBJ, "s", gnoclOptSizeGroup },
	{ "-tooltip", GNOCL_OBJ, "", gnoclOptTooltip },
	{ "-visible", GNOCL_BOOL, "visible" },
	{ "-widthGroup", GNOCL_OBJ, "w", gnoclOptSizeGroup },
	{ "-backgroundImage", GNOCL_OBJ, "", gnoclOptBackgroundImage },

	/* inherited GtkWidget properties */
	{ "-heightRequest", GNOCL_INT, "height-request" },
	{ "-widthRequest", GNOCL_INT, "width-request" },

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

	ButtonParams *para = ( ButtonParams * ) data;

	gnoclForgetWidgetFromName ( para->name );
	Tcl_DeleteCommand ( para->interp, para->name );

	g_free ( para->name );
	g_free ( para );
}

/**
\brief
**/
void buttonAlign ( ButtonParams *para )
{

#ifdef DEBUG_BUTTON
	g_print ( "%s\n", __FUNCTION__ );
#endif

	if ( strcmp ( para->align, "left" ) == 0 )
	{
		gtk_alignment_set ( GTK_ALIGNMENT ( para->alignment ), 0.0, 0.0, 0.0, 0.0 );
	}

	if ( strcmp ( para->align, "right" ) == 0 )
	{
		gtk_alignment_set ( GTK_ALIGNMENT ( para->alignment ), 1.0, 1.0, 0.0, 0.0 );
	}

	if ( strcmp ( para->align, "center" ) == 0 || strcmp ( para->align, "centre" ) == 0 )
	{
		gtk_alignment_set ( GTK_ALIGNMENT ( para->alignment ), 0.5, 0.5, 0.0, 0.0 );
	}

}


/**
\brief  Configure the named widget using -option parameter passed either
        during the creation of the widget or following a configure command.
**/
static int configure (  Tcl_Interp *interp, ButtonParams *para,  GnoclOption options[] )
{
	if ( options[textIdx].status == GNOCL_STATUS_CHANGED && gnoclConfigButtonText ( interp, GTK_BUTTON ( para->button ), options[textIdx].val.obj ) != TCL_OK )
	{


		return TCL_ERROR;
	}

	/*
		if ( options[dataIdx].status == GNOCL_STATUS_CHANGED )
		{
			para->data = strdup ( options[dataIdx].val.str );
		}
	*/
	if ( options[alignIdx].status == GNOCL_STATUS_CHANGED )
	{

		para->align = Tcl_GetString ( options[alignIdx].val.obj );

		// gtk_toolbar_insert  (GtkToolbar *toolbar, GtkToolItem *item, gint pos);

		buttonAlign ( para );

	}

	if ( options[baseFontIdx].status == GNOCL_STATUS_CHANGED )
	{

		GtkWidget *label;

		para->baseFont = Tcl_GetString ( options[baseFontIdx].val.obj );
		label = gnoclFindChild ( GTK_WIDGET ( para->button ), GTK_TYPE_LABEL );

		PangoFontDescription *font_desc = pango_font_description_from_string ( para->baseFont );

		gtk_widget_modify_font ( GTK_WIDGET ( label ), font_desc );
		pango_font_description_free ( font_desc );

	}

	if ( options[iconIdx].status == GNOCL_STATUS_CHANGED )
	{
		GnoclStringType type = gnoclGetStringType ( options[iconIdx].val.obj );
		GtkWidget *label = gnoclFindChild ( GTK_WIDGET ( para->button ), GTK_TYPE_LABEL );

		if ( type == GNOCL_STR_EMPTY )
		{
			/* remove all children apart from label */
			GtkWidget *child = gtk_bin_get_child ( GTK_BIN ( para->button ) );

			if ( child && ( child != label ) )
			{
				gtk_widget_ref ( label );
				gtk_container_remove ( GTK_CONTAINER ( para->button ), child );
				gtk_container_add ( GTK_CONTAINER ( para->button ), label );
				gtk_widget_unref ( label );
				gtk_widget_show ( label );
			}
		}

		else
		{
			GtkWidget *image = gnoclFindChild ( GTK_WIDGET ( para->button ), GTK_TYPE_IMAGE );

			if ( label == NULL )
			{
				gtk_button_set_label ( para->button, "" );
				label = gnoclFindChild ( GTK_WIDGET ( para->button ), GTK_TYPE_LABEL );
			}

			else if ( ( type & ( GNOCL_STR_STOCK | GNOCL_STR_FILE ) ) == 0 )
			{
				Tcl_AppendResult ( interp, "Unknown type for \"",
								   Tcl_GetString ( options[iconIdx].val.obj ),
								   "\" must be of type FILE (%/) or STOCK (%#)", NULL );
				return TCL_ERROR;

			}

			para->iconName = Tcl_GetString ( options[iconIdx].val.obj );

			if ( image == NULL )
			{
				/* this should match gtkbutton.c */
				GtkWidget *hbox = gtk_hbox_new ( 0, 2 );
				GtkAlignment *align = gtk_alignment_new ( 0.5, 0.5, 0.0, 0.0 );

				para->alignment = align;

				buttonAlign ( para );

				image = gtk_image_new();

				gtk_box_pack_start ( GTK_BOX ( hbox ), image, 0, 0, 0 );

				gtk_widget_ref ( label );
				gtk_container_remove ( GTK_CONTAINER ( para->button ), label );
				gtk_box_pack_end ( GTK_BOX ( hbox ), label, 0, 0, 0 );
				gtk_widget_unref ( label );

				gtk_container_add ( GTK_CONTAINER ( para->button ), align );
				gtk_container_add ( GTK_CONTAINER ( align ), hbox );
				gtk_widget_show_all ( align );
			}

			if ( type & GNOCL_STR_STOCK )
			{
				GtkStockItem item;

				if ( gnoclGetStockItem ( options[iconIdx].val.obj, interp, &item ) != TCL_OK )
				{
					return TCL_ERROR;
				}

				gtk_image_set_from_stock ( GTK_IMAGE ( image ), item.stock_id, GTK_ICON_SIZE_BUTTON );
			}

			else if ( type & GNOCL_STR_FILE )
			{
				GdkPixbuf *pix = gnoclPixbufFromObj ( interp, options + iconIdx );

				if ( pix == NULL )
				{
					return TCL_ERROR;
				}

				gtk_image_set_from_pixbuf ( GTK_IMAGE ( image ), pix );
			}
		}

	}

	return TCL_OK;
}

/**
\brief  Special function to set the text associated with this widget.
**/
int gnoclConfigButtonText ( Tcl_Interp *interp, GtkButton *button, Tcl_Obj *txtObj )
{
#ifdef DEBUG_BUTTON
	printf ( "%s\n", __FUNCTION__ );
#endif

	GnoclStringType type = gnoclGetStringType ( txtObj );

	if ( type & GNOCL_STR_STOCK )
	{
		GtkStockItem sp;

		if ( gnoclGetStockItem ( txtObj, interp, &sp ) != TCL_OK )
		{
			return TCL_ERROR;
		}

		gtk_button_set_label ( button, sp.stock_id );

#ifdef DEBUG_BUTTON
		printf ( "%s 1\n", __FUNCTION__ );
#endif
		gtk_button_set_use_stock ( button, 1 );
#ifdef DEBUG_BUTTON
		printf ( "%s 2\n", __FUNCTION__ );
#endif
	}

	else
	{
		GtkLabel *label;
		char *txt = gnoclGetString ( txtObj );
#ifdef DEBUG_BUTTON
		printf ( "%s 3\n", __FUNCTION__ );
#endif
		gtk_button_set_label ( button, txt );
#ifdef DEBUG_BUTTON
		printf ( "%s 4\n", __FUNCTION__ );
#endif
		gtk_button_set_use_stock ( button, 0 );
#ifdef DEBUG_BUTTON
		printf ( "%s 5\n", __FUNCTION__ );
#endif
		label = GTK_LABEL ( gnoclFindChild ( GTK_WIDGET ( button ), GTK_TYPE_LABEL ) );
#ifdef DEBUG_BUTTON
		printf ( "%s 6\n", __FUNCTION__ );
#endif
		assert ( label );

		/* TODO? pango_parse_markup for error message */
		gtk_label_set_use_markup ( label, ( type & GNOCL_STR_MARKUP ) != 0 );
		gtk_label_set_use_underline ( label, ( type & GNOCL_STR_UNDERLINE ) != 0 );
	}

	return TCL_OK;
}


/**
\brief  Speical function to obtain the text assigned to the button widget.
**/
Tcl_Obj *gnoclCgetButtonText (  Tcl_Interp *interp, GtkButton *button )
{
#ifdef DEBUG_BUTTON
	printf ( "%s\n", __FUNCTION__ );
#endif
	Tcl_Obj *obj = NULL;

	if ( gtk_button_get_use_stock ( button ) )
	{
		const char *st = gtk_button_get_label ( button );

		if ( st == NULL )
		{
			obj = Tcl_NewStringObj ( "", 0 );
		}

		else
		{
			obj = Tcl_NewStringObj ( "%#", 2 );
			Tcl_AppendObjToObj ( obj, gnoclGtkToStockName ( st ) );
		}
	}

	else
	{
		GtkLabel *label = GTK_LABEL ( gnoclFindChild ( GTK_WIDGET ( button ), GTK_TYPE_LABEL ) );
		Tcl_Obj *old = Tcl_NewStringObj ( gtk_label_get_label ( label ), -1 );
		assert ( label );

		if ( gtk_label_get_use_markup ( label ) )
		{
			obj = Tcl_NewStringObj ( "%<", 2 );
			Tcl_AppendObjToObj ( obj, old );
		}

		else if ( gtk_label_get_use_underline ( label ) )
		{
			obj = Tcl_NewStringObj ( "%_", 2 );
			Tcl_AppendObjToObj ( obj, old );
		}

		else
		{
			obj = old;
		}
	}

	return obj;
}

/**
\brief  Obtain current -option values.
**/
static int cget (   Tcl_Interp *interp,  ButtonParams *para,  GnoclOption options[],  int idx )
{

	Tcl_Obj *obj = NULL;

	/*
		if ( idx == dataIdx )
		{
			obj = Tcl_NewStringObj ( para->data, -1 );
			gnoclOptParaData ( interp, para->data, &obj);
		}
	*/
	if ( idx == baseFontIdx )
	{
		obj = Tcl_NewStringObj ( para->baseFont, -1 );
	}

	if ( idx == alignIdx )
	{
		obj = Tcl_NewStringObj ( para->align, -1 );
	}


	if ( idx == textIdx )
	{
		obj = gnoclCgetButtonText ( interp, para->button );
	}

	else if ( idx == iconIdx )
	{
		GtkWidget *image = gnoclFindChild ( GTK_WIDGET ( para->button ), GTK_TYPE_IMAGE );

		if ( image == NULL )
		{
			obj = Tcl_NewStringObj ( "", 0 );
		}

		else
		{
			gchar  *st;
			g_object_get ( G_OBJECT ( image ), "stock", &st, NULL );

			if ( st )
			{
				obj = Tcl_NewStringObj ( "%#", 2 );
				Tcl_AppendObjToObj ( obj, gnoclGtkToStockName ( st ) );
				g_free ( st );
			}

			else
			{
				Tcl_SetResult ( interp, "Could not determine icon type.", TCL_STATIC );
				return TCL_ERROR;
			}
		}
	}

	if ( obj != NULL )
	{
		Tcl_SetObjResult ( interp, obj );
		return TCL_OK;
	}

	return gnoclCgetNotImplemented ( interp, options + idx );
}

static const char *cmds[] =
{
	"delete", "configure", "cget",
	"onClicked", "class", "parent",
	"geometry", "toplevel",
	"add",
	NULL
};


/**
\brief  Function associated with the widget.
\note	The add widget command is largely pointless, all input to it is first passed to the parent button.
**/
int buttonFunc ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{


	enum cmdIdx
	{
		DeleteIdx, ConfigureIdx, CgetIdx,
		OnClickedIdx, ClassIdx, ParentIdx,
		GeometryIdx, ToplevelIdx,
		AddIdx
	};

	ButtonParams *para = ( ButtonParams * ) data;

	GtkButton *button = para->button;
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

			/* button is a container, a pointless functionality */
		case AddIdx:
			{
				GtkWidget *child1 = gtk_bin_get_child ( GTK_BIN ( para->button ) );
				GtkWidget *child2 = gtk_bin_get_child ( GTK_CONTAINER ( child1 ) );
				GtkWidget *widget = gnoclGetWidgetFromName (  Tcl_GetString ( objv[2] ), interp );
				gtk_container_add ( GTK_CONTAINER ( child2 ), widget );
			}
			break;
		case ToplevelIdx:
			{
				//g_print ( "button ToplevelIdx\n" );
				GtkWidget *toplevel;
				Tcl_Obj *obj = NULL;
				toplevel = gtk_widget_get_toplevel ( para->button ) ;
				obj = Tcl_NewStringObj ( gnoclGetNameFromWidget ( toplevel ), -1 );
				Tcl_SetObjResult ( interp, obj );
				return TCL_OK;
			}

			break;
		case GeometryIdx:
			{
				//g_print ( "button GeometryIdx\n" );
				char *txt = gnoclGetWidgetGeometry ( para->button ) ;
				Tcl_SetObjResult ( interp, Tcl_NewStringObj ( txt , -1 ) );
				return TCL_OK;
			}

			break;
		case ParentIdx:
			{

				GtkWidget * parent;
				Tcl_Obj *obj = NULL;
				parent = gtk_widget_get_parent ( GTK_WIDGET ( para->button ) );
				obj = Tcl_NewStringObj ( gnoclGetNameFromWidget ( parent ), -1 );
				Tcl_SetObjResult ( interp, obj );

				/* this function not working too well! */
				/* return gnoclGetParent ( interp, data ); */
				return TCL_OK;
			}

			break;
		case ClassIdx:
			{
				Tcl_SetObjResult ( interp, Tcl_NewStringObj ( "button", -1 ) );

			}
			break;
		case DeleteIdx:
			{
				return gnoclDelete ( interp, GTK_WIDGET ( para->button ), objc, objv );
			}
			break;
		case ConfigureIdx:
			{
				int ret = TCL_ERROR;

				if ( gnoclParseAndSetOptions ( interp, objc - 1, objv + 1, buttonOptions, G_OBJECT ( para->button ) ) == TCL_OK )
				{
					ret = configure ( interp, para, buttonOptions );
				}

				gnoclClearOptions ( buttonOptions );

				return ret;
			}

			break;

		case OnClickedIdx:

			if ( objc != 2 )
			{
				Tcl_WrongNumArgs ( interp, 2, objv, NULL );
				return TCL_ERROR;
			}

			if ( GTK_WIDGET_IS_SENSITIVE ( GTK_WIDGET ( button ) ) )
			{
				gtk_button_clicked ( button );
			}

			break;

		case CgetIdx:
			{
				int idx;

				switch ( gnoclCget ( interp, objc, objv, G_OBJECT ( para->button ), buttonOptions, &idx ) )
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
							return cget ( interp, para, buttonOptions, idx );
						}
				}
			}
			break;
		default:
			{
			}
	}

	return TCL_OK;
}

/**
\brief     Function to create and configure an new instance of the button widget.
**/
int gnoclButtonCmd ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{
	if ( gnoclGetCmdsAndOpts ( interp, cmds, buttonOptions, objv, objc ) == TCL_OK )
	{
		return TCL_OK;
	}

	ButtonParams *para;
	para = g_new ( ButtonParams, 1 );

	int  ret;

	/* step 1) check validity of switches */
	if ( gnoclParseOptions ( interp, objc, objv, buttonOptions ) != TCL_OK )
	{
		gnoclClearOptions ( buttonOptions );
		return TCL_ERROR;
	}

	/* step 2) create an instance of the widget and 'show' it*/
	para->button = GTK_BUTTON ( gtk_button_new( ) );
	para->align = "center";

	const char *dataID = "gnocl::para";
	g_object_set_data ( G_OBJECT ( para->button ), dataID, para );
	para->interp = interp;
	para->alignment = gtk_alignment_new ( 0.5, 0.5, 0.0, 0.0 );;

	gtk_widget_show ( GTK_WIDGET ( para->button ) );

	/* step 3) check the options passed for the creation of the widget */
	ret = gnoclSetOptions ( interp, buttonOptions, G_OBJECT ( para->button ), -1 );

	/* step 4) if this is ok, then configure the new widget */
	if ( ret == TCL_OK )
	{
		ret = configure ( interp, para, buttonOptions );
	}

	/* step 5) clear the memory set assigned to the options */
	gnoclClearOptions ( buttonOptions );

	/* step 6) if the options passed were incorrect, then delete the widget */
	if ( ret != TCL_OK )
	{
		gtk_widget_destroy ( GTK_WIDGET ( para->button ) );
		g_free ( para );
		return TCL_ERROR;
	}

	para->name = gnoclGetAutoWidgetId();
	g_signal_connect ( G_OBJECT ( para->button ), "destroy", G_CALLBACK ( destroyFunc ), para );
	gnoclMemNameAndWidget ( para->name, GTK_WIDGET ( para->button ) );
	Tcl_CreateObjCommand ( interp, para->name, buttonFunc, para, NULL );
	Tcl_SetObjResult ( interp, Tcl_NewStringObj ( para->name, -1 ) );

	return TCL_OK;

}
