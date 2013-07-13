/*
 * $Id: toolBar.c,v 1.13 2005/02/22 23:16:10 baum Exp $
 *
 * This file implements the toolbar widget
 *
 * Copyright (c) 2001 - 2005 Peter G. Baum  http://www.dr-baum.net
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

/*
   History:
   2013-07:	added commands options, commands
			cget -style now works
   2013-06: configure/cget -orientation now works
			added flip -an expiremental feature
   2013-05: added parent
   2012-05: toolbar cget -data now works
   2011-04: added -tooltips
   			nItems
   			re-wrote the code to use updated Toolbar API
   2010-09: added button, toggle, menu & radio as synonyms for item,
   			checkItem, menuButton & radioItem objects. Began updating this
   			code to remove deprecated Gtk+ function calls.
   2009-11: renamed static buttonFunc to non-static toolButtonFunc to
            enable use in parsing glade files
   2008-10: added command, class
   2007-12: propose addition of MenuToolButton
            http://www.gtk.org/api/2.6/gtk/GtkMenuToolButton.html
            change of term 'item' to button
            use command menuButton

   2004-02: added -data for radio and check item
   2003-09: changed -value to -onValue for radio item
            added cget to radio and check item
        12: switched from GnoclWidgetOptions to GnoclOption
        05: updates for gtk 2.0
   2002-02: percent subst. in command
            added STD_OPTIONS
        12: icon text qualified by "%#" or "%/"
   2001-03: Begin of developement
 */

/**
\page page_toolBar gnocl::toolBar
\htmlinclude toolBar.html
**/

#include "gnocl.h"
#include "gnoclparams.h"

/* static function declarations */
static int optOrientation ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret );
static int optVisibility ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret );
static int optStyle ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret );

/**
\brief	Set the menu to display when menuButton downarrow clicked.
**/
static int gnoclOptMenu ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{

	GtkWidget *menu;

	menu = gnoclGetWidgetFromName ( Tcl_GetString ( opt->val.obj ), interp );

	gtk_menu_tool_button_set_menu ( GTK_MENU_TOOL_BUTTON ( obj ), menu );

	return TCL_OK;
}

/* referenced in addButton */
static const int SpaceIdx      	= 0;
static const int WidgetIdx      = 1;
static const int ItemIdx      	= 2;
static const int MenuButtonIdx  = 3;
static const int CheckButtonIdx = 4;
static const int RadioItemIdx  	= 5;
static const int ToggleIdx     	= 6;
static const int ButtonIdx     	= 7;
static const int MenuIdx      	= 8;
static const int RadioIdx      	= 9;


/* */
static const int dataIdx = 0;
static const int orientationIdx = 1;
static const int tooolbarStyleIdx = 2;
static const int reliefIdx = 3;

/* toolBar options */
static GnoclOption toolBarOptions[] =
{
	{ "-data", GNOCL_OBJ, "", gnoclOptData }, /* 0 */
	{ "-orientation", GNOCL_STRING, NULL},
	{ "-style", GNOCL_OBJ, "toolbar-style", optStyle },
	//{ "-relief", GNOCL_OBJ, "button-relief", gnoclOptButtonRelief },

	/* widget specific options */
	{ "-iconSize", GNOCL_INT, "icon-size"},
	{ "-iconSizeSet", GNOCL_BOOL, "icon-size-set"},
	{ "-showArrow", GNOCL_BOOL, "show-arrow"},

	{ "-tooltips", GNOCL_BOOL, "tooltips" },

	/* inherited container properties */
	{ "-borderWidth", GNOCL_OBJ, "border-width", gnoclOptPadding },	/* 6 */

	/* general options */
	{ "-visible", GNOCL_BOOL, "visible" },
	{ "-sensitive", GNOCL_BOOL, "sensitive" },
	{ "-name", GNOCL_STRING, "name" },


	/* drag and drop functionality */
	/*
		{ "-dropTargets", GNOCL_LIST, "t", gnoclOptDnDTargets },
		{ "-dragTargets", GNOCL_LIST, "s", gnoclOptDnDTargets },
		{ "-onDropData", GNOCL_OBJ, "", gnoclOptOnDropData },
		{ "-onDragData", GNOCL_OBJ, "", gnoclOptOnDragData },
	*/
	{ NULL, 0, 0 }
};


/* menuButton options */
static const int menuButtonTextIdx      	= 0;
static const int menuButtonIconIdx 			= 1;
static const int menuButtonIconWidgetIdx    = 2;
static const int menuButtonMenuIdx 			= 3;

//static const int menuButtonArrowClickedIdx 	= 4;
GnoclOption menuButtonOptions[] =
{
	{ "-text", GNOCL_OBJ, NULL },
	{ "-icon", GNOCL_OBJ, NULL},
	{ "-iconWidget", GNOCL_STRING, NULL },
	{ "-menu", GNOCL_OBJ, "", gnoclOptMenu },

	// { "-onArrowClicked", GNOCL_STRING, NULL },	/* 4 */

	/* object signals */
	{ "-onShowMenu", GNOCL_OBJ, "show-menu", gnoclOptCommand },
	{ "-onClicked", GNOCL_OBJ, "clicked", gnoclOptOnClicked },

	/* inherited options */
	{ "-visible", GNOCL_BOOL, "visible" },
	{ "-sensitive", GNOCL_BOOL, "sensitive" },
	{ "-tooltip", GNOCL_OBJ, "", gnoclOptTooltip },
	{ "-arrowTooltip", GNOCL_OBJ, "", gnoclOptArrowTooltip },
	{ "-onShowHelp", GNOCL_OBJ, "", gnoclOptOnShowHelp },
	{ "-name", GNOCL_STRING, "name" },

	{ NULL, 0, 0 }
};


/* button options */
static const int buttonTextIdx      = 0;
static const int buttonIconIdx      = 1;
static const int buttonOnClickedIdx = 2;

GnoclOption buttonOptions[] =
{
	/* gnocl specific options */
	{ "-text", GNOCL_OBJ, NULL },
	{ "-icon", GNOCL_OBJ, NULL },

	{"-labelWidget", GNOCL_OBJ, "", gnoclOptToolButtonLabelWidget},	/* useful for bonobo text */
	{"-iconWidget", GNOCL_OBJ, "", gnoclOptToolButtonIconWidget},		/* odd option, use with caution */

	{ "-important", GNOCL_BOOL, "is-important" },

	/* widget properties */
	/* signal handling */
	/* general options */

	{ "-onClicked", GNOCL_OBJ, "clicked", gnoclOptCommand },
	{ "-visibility", GNOCL_OBJ, "", optVisibility },

	{ "-sensitive", GNOCL_BOOL, "sensitive" },
	{ "-tooltip", GNOCL_OBJ, "", gnoclOptTooltip },
	{ "-onShowHelp", GNOCL_OBJ, "", gnoclOptOnShowHelp },

	{ "-name", GNOCL_STRING, "name" },
	{ "-label", GNOCL_STRING, "label" },

	/* menu button option */
	{ "-menu", GNOCL_OBJ, "", gnoclOptMenu },

	{ NULL, 0, 0 }
};

/* Checkbutton Options */
static const int checkTextIdx      = 0;
static const int checkIconIdx      = 1;
static const int checkOnToggledIdx = 2;
static const int checkOnValueIdx   = 3;
static const int checkOffValueIdx  = 4;
static const int checkVariableIdx  = 5;
static const int checkActiveIdx    = 6;
static const int checkValueIdx     = 7;

static GnoclOption checkOptions[] =
{
	{ "-text", GNOCL_OBJ, NULL },           /* 0 */
	{ "-icon", GNOCL_OBJ, NULL },           /* 1 */
	{ "-onToggled", GNOCL_STRING, NULL },   /* 2 the order is important! */
	{ "-onValue", GNOCL_OBJ, NULL },        /* 3 */
	{ "-offValue", GNOCL_OBJ, NULL },       /* 4 */
	{ "-variable", GNOCL_STRING, NULL },    /* 5 */
	{ "-active", GNOCL_BOOL, NULL },        /* 6 */
	{ "-value", GNOCL_OBJ, NULL },          /* 7 */
	{ "-data", GNOCL_OBJ, "", gnoclOptData },
	{ "-name", GNOCL_STRING, "name" },
	{ "-onButtonPress", GNOCL_OBJ, "P", gnoclOptOnButton },
	{ "-onButtonRelease", GNOCL_OBJ, "R", gnoclOptOnButton },
	{ "-onShowHelp", GNOCL_OBJ, "", gnoclOptOnShowHelp },
	{ "-sensitive", GNOCL_BOOL, "sensitive" },
	{ "-tooltip", GNOCL_OBJ, "", gnoclOptTooltip },
	{ "-visible", GNOCL_BOOL, "visible" },
	{ NULL, 0, 0 }
};

/* radioButton options */
static const int radioTextIdx      = 0;
static const int radioIconIdx      = 1;
static const int radioOnToggledIdx = 2;
static const int radioOnValueIdx   = 3;
static const int radioOffValueIdx  = 4;
static const int radioVariableIdx  = 5;
static const int radioActiveIdx    = 6;
static const int radioValueIdx     = 7;

static GnoclOption radioOptions[] =
{
	/* gnocl specific options */
	{ "-text", GNOCL_OBJ, NULL },           /* 0 */
	{ "-icon", GNOCL_OBJ, NULL },           /* 1 */
	{ "-onToggled", GNOCL_STRING, NULL },   /* 2 the order is important! */
	{ "-onValue", GNOCL_OBJ, NULL },        /* 3 */
	{ "-offValue", GNOCL_OBJ, NULL },       /* 4 */
	{ "-variable", GNOCL_STRING, NULL },    /* 5 */
	{ "-active", GNOCL_BOOL, NULL },        /* 6 */
	{ "-value", GNOCL_OBJ, NULL },          /* 7 */
	{ "-data", GNOCL_OBJ, "", gnoclOptData },
	{ "-name", GNOCL_STRING, "name" },
	{ "-onButtonPress", GNOCL_OBJ, "P", gnoclOptOnButton },
	{ "-onButtonRelease", GNOCL_OBJ, "R", gnoclOptOnButton },
	{ "-onShowHelp", GNOCL_OBJ, "", gnoclOptOnShowHelp },
	{ "-sensitive", GNOCL_BOOL, "sensitive" },
	{ "-tooltip", GNOCL_OBJ, "", gnoclOptTooltip },
	{ "-visible", GNOCL_BOOL, "visible" },
	{ NULL, 0, 0 }
};




/* moved to gnocl.h */
/*
typedef struct
{
    GtkWidget  *item;
    char       *name;
    char       *onClicked;
    Tcl_Interp *interp;
} ToolButtonParams;
*/

/**
\brief
**/
static int optOrientation ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	const char *txt[] = { "horizontal", "vertical", NULL };
	const int types[] = { GTK_ORIENTATION_HORIZONTAL,
						  GTK_ORIENTATION_VERTICAL
						};

	assert ( sizeof ( GTK_ORIENTATION_VERTICAL ) == sizeof ( int ) );

	return gnoclOptGeneric ( interp, opt, obj, "orientation", txt, types, ret );
}


/**
\brief
**/
static int optVisibility ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
#ifdef DEBUG_TOOLBAR
	g_print ( "GNOCL MESSAGE: ToolBar option \"-visibility\" not yet implemented.\n", __FUNCTION__ );
#endif

	return TCL_OK;
}

/**
\brief
**/
static int optStyle ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	const char *txt[] =
	{
		"icons", "text", "both", "horizontal",
		NULL
	};
	const int types[] =
	{
		GTK_TOOLBAR_ICONS, GTK_TOOLBAR_TEXT, GTK_TOOLBAR_BOTH, GTK_TOOLBAR_BOTH_HORIZ
	};

	assert ( sizeof ( GTK_TOOLBAR_ICONS ) == sizeof ( int ) );

	return gnoclOptGeneric ( interp, opt, obj, "style", txt, types, ret );
}


/**
\brief	Modified version of getTextAndIcon to allow for new Toolbar API
\param
		Tcl_Interp *interp		pointer to Tcl interpreter
		GtkToolbar *toolbar		pointer to toolbar object that will receive new item
		GnoclOption *txtOpt		pointer to item label ??
		GnoclOption *iconOpt	pointer to item icon ??
		char **txt				handle on text pointer
		GtkWidget **icon		handle on image widget pointer
		int *isUnderline		pointer to int

\note	Either -text or -icon must be set.

**/
int getTextAndIcon ( Tcl_Interp *interp, GtkToolbar *toolbar,
					 GnoclOption *txtOpt, GnoclOption *iconOpt,
					 char **txt, GtkWidget **item, int *isUnderline )
{
	*item = NULL;
	*txt = NULL;
	*isUnderline = 0;

	GtkImage *image;
	gchar *name;
	Tcl_Obj *icon;
	GnoclStringType type;

	/* error check, must have an icon defined! */
	if ( iconOpt->status != GNOCL_STATUS_CHANGED && txtOpt->status != GNOCL_STATUS_CHANGED )
	{
		Tcl_SetObjResult ( interp, Tcl_NewStringObj ( "GNOCL ERROR! Either -icon and -text must be set.", -1 ) );
		return TCL_ERROR;
	}


	/* if only text set, must check for percent string */
	if ( txtOpt->status == GNOCL_STATUS_CHANGED )
	{
		/* assume text == icon */
		name = gnoclGetStringFromObj ( txtOpt->val.obj, NULL );
		icon = txtOpt->val.obj;
		type = gnoclGetStringType ( txtOpt->val.obj );

	}

	/* icon only */
	if ( iconOpt->status == GNOCL_STATUS_CHANGED )
	{
		icon = iconOpt->val.obj;

		if ( txtOpt->status != GNOCL_STATUS_CHANGED )
		{
			name = gnoclGetStringFromObj ( iconOpt->val.obj, NULL );
		}

	}

	type = gnoclGetStringType ( icon );

	/* stock item */
	if ( type & GNOCL_STR_STOCK )
	{
#ifdef DEBUG_TOOLBAR
		g_print ( "STOCK\n" );
#endif
		GtkStockItem stockItem;
		GtkIconSize sz;

		if ( gnoclGetStockItem ( icon, interp, &stockItem ) != TCL_OK )
		{
			return TCL_ERROR;
		}

		sz = gtk_toolbar_get_icon_size ( toolbar );
		image = gtk_image_new_from_stock ( stockItem.stock_id, sz );

	}

	/* file */
	else if ( type & GNOCL_STR_FILE )
	{
#ifdef DEBUG_TOOLBAR
		g_print ( "FILE\n" );
#endif
		GError *error = NULL;
		GdkPixbuf *pixbuf = NULL;

		pixbuf = gdk_pixbuf_new_from_file ( icon, NULL );
		image = gtk_image_new_from_pixbuf ( pixbuf );

	}

	/* buffer */
	else if ( type & GNOCL_STR_BUFFER )
	{
#ifdef DEBUG_TOOLBAR
		g_print ( "BUFFER\n" );
#endif
		GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file ( icon, NULL );
		GtkWidget *image = gtk_image_new_from_pixbuf ( pixbuf );

	}

	/* get the text string */
	else
	{
		GtkIconSize sz;

		sz = gtk_toolbar_get_icon_size ( toolbar );
		GtkWidget *image = gtk_image_new_from_stock ( GTK_STOCK_MISSING_IMAGE, sz );

	}

	/*
		uncertain what's happening here
		this isUnderline was
	*/
	if ( type & GNOCL_STR_UNDERLINE )
	{
		if ( txt == NULL )
		{
			*isUnderline = 0;
		}

		else
		{
			*isUnderline = 1;
		}
	}

	gtk_widget_show ( image );

	*item = image;

	*txt = g_strdup ( name );

	*isUnderline = 1;

	return TCL_OK;
}


/**
\brief
**/
static void setUnderline ( GtkWidget *item )
{

	/* FIXME: is there really only one label? */
	GtkWidget *label = gnoclFindChild ( item, GTK_TYPE_LABEL );

	//assert ( label );
	if ( label !=  NULL )
	{
		gtk_label_set_use_underline ( GTK_LABEL ( label ), 1 );
	}
}





/**
\brief
**/
static int configure ( Tcl_Interp *interp, GtkWidget *widget, GnoclOption options[] )
{

	if ( options[orientationIdx].status == GNOCL_STATUS_CHANGED )
	{
		if ( strcmp ( options[orientationIdx].val.str, "horizontal" ) == 0 )
		{

			gtk_orientable_set_orientation ( GTK_ORIENTABLE ( widget ), GTK_ORIENTATION_HORIZONTAL );

		}

		else if ( strcmp ( options[orientationIdx].val.str, "vertical" ) == 0 )
		{

			gtk_orientable_set_orientation ( GTK_ORIENTABLE ( widget ), GTK_ORIENTATION_VERTICAL );
		}

		else
		{
			return TCL_ERROR;
		}


	}

	return TCL_OK;
}


/**
\brief  Obtain current -option values.
**/
static int toolBarCget (  Tcl_Interp *interp, GtkWidget *widget,  GnoclOption options[],  int idx )
{


	Tcl_Obj *obj = NULL;

	if ( idx == dataIdx )
	{
		obj = Tcl_NewStringObj ( g_object_get_data ( widget, "gnocl::data" ), -1 );
	}


	if ( idx == reliefIdx )
	{
		switch ( gtk_toolbar_get_relief_style ( widget ) )
		{
			case GTK_RELIEF_NORMAL:
				{
					obj = Tcl_NewStringObj ( "normal", -1 );
				} break;
			case GTK_RELIEF_HALF:
				{
					obj = Tcl_NewStringObj ( "half", -1 );
				} break;
			case GTK_RELIEF_NONE:
				{
					obj = Tcl_NewStringObj ( "none", -1 );
				} break;
			default:
				{
					return TCL_ERROR;
				}
		}
	}

	if ( idx == orientationIdx )
	{

		switch ( gtk_orientable_get_orientation ( widget ) )
		{
			case GTK_ORIENTATION_HORIZONTAL:
				{
					obj = Tcl_NewStringObj ( "horizontal", -1 );
				} break;
			case  GTK_ORIENTATION_VERTICAL:
				{
					obj = Tcl_NewStringObj ( "vertical", -1 );
				} break;
			default:
				{
					return TCL_ERROR;
				}
		}
	}

	if ( idx == tooolbarStyleIdx )
	{
		switch ( gtk_toolbar_get_style ( widget ) )
		{
			case GTK_TOOLBAR_ICONS:
				{
					obj = Tcl_NewStringObj ( "icons", -1 );
				} break;
			case  GTK_TOOLBAR_TEXT:
				{
					obj = Tcl_NewStringObj ( "text", -1 );
				} break;
			case  GTK_TOOLBAR_BOTH:
				{
					obj = Tcl_NewStringObj ( "both", -1 );
				} break;
			case  GTK_TOOLBAR_BOTH_HORIZ:
				{
					obj = Tcl_NewStringObj ( "horizontal", -1 );
				} break;
			default:
				{
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



/**
\brief
**/
static Tcl_Obj *cgetText ( GtkWidget *item )
{

	/* FIXME: is there really only one label? */
	GtkWidget *label = gnoclFindChild ( item, GTK_TYPE_LABEL );

	if ( label != NULL )
	{
		const char *txt = gtk_label_get_label ( GTK_LABEL ( label ) );

		if ( txt != NULL )
		{
			Tcl_Obj *obj = Tcl_NewStringObj ( txt, -1 );
			/* FIXME: that does not work
			if( gtk_button_get_use_stock( GTK_BUTTON( item ) ) )
			{
			   Tcl_Obj *old = obj;
			   obj = Tcl_NewStringObj( "%#", 2 );
			   Tcl_AppendObjToObj( obj, old );
			}
			else
			*/

			if ( gtk_label_get_use_underline ( GTK_LABEL ( label ) ) )
			{
				Tcl_Obj *old = obj;
				obj = Tcl_NewStringObj ( "%_", 2 );
				Tcl_AppendObjToObj ( obj, old );
			}

			return obj;
		}
	}

	return Tcl_NewStringObj ( "", 0 );;
}

/**
\brief
**/
/*
   -------------- check functions ----------------------------
*/
static void checkDestroyFunc ( GtkWidget *widget, gpointer data )
{
	GnoclToolBarCheckParams *para = ( GnoclToolBarCheckParams * ) data;

	gnoclAttachVariable ( NULL, &para->variable,
						  "toggled", G_OBJECT ( para->item ),
						  G_CALLBACK ( gnoclCheckToggledFunc ),
						  para->interp, gnoclCheckTraceFunc, para );

	gnoclForgetWidgetFromName ( para->name );
	Tcl_DeleteCommand ( para->interp, para->name );
	g_free ( para->name );

	Tcl_DecrRefCount ( para->onValue );
	Tcl_DecrRefCount ( para->offValue );
	g_free ( para );
}

/**
\brief
**/
static int checkConfigure ( Tcl_Interp *interp, GnoclToolBarCheckParams *para,
							GnoclOption options[] )
{
	if ( options[checkOnValueIdx].status == GNOCL_STATUS_CHANGED )
	{
		GNOCL_MOVE_OBJ ( options[checkOnValueIdx].val.obj, para->onValue );
	}

	if ( options[checkOffValueIdx].status == GNOCL_STATUS_CHANGED )
	{
		GNOCL_MOVE_OBJ ( options[checkOffValueIdx].val.obj, para->offValue );
	}

	if ( options[checkOnToggledIdx].status == GNOCL_STATUS_CHANGED )
	{
		GNOCL_MOVE_STRING ( options[checkOnToggledIdx].val.str, para->onToggled );
	}

	gnoclAttachVariable ( &options[checkVariableIdx], &para->variable,
						  "toggled", G_OBJECT ( para->item ),
						  G_CALLBACK ( gnoclCheckToggledFunc ), para->interp,
						  gnoclCheckTraceFunc, para );

	if ( gnoclCheckSetActive ( para, &options[checkActiveIdx] ) == 0
			&& ( options[checkVariableIdx].status == GNOCL_STATUS_CHANGED
				 || options[checkOnValueIdx].status == GNOCL_STATUS_CHANGED
				 || options[checkOffValueIdx].status == GNOCL_STATUS_CHANGED ) )
	{
		gnoclCheckVariableValueChanged ( para );
	}

	if ( options[checkValueIdx].status == GNOCL_STATUS_CHANGED )
	{
		if ( gnoclCheckSetValue ( para, options[checkValueIdx].val.obj ) != TCL_OK )
			return TCL_ERROR;
	}

	return TCL_OK;
}

/**
\brief
**/
static int checkCget ( Tcl_Interp *interp, GnoclToolBarCheckParams *para, GnoclOption options[], int idx )
{


	Tcl_Obj *obj = NULL;

	if ( idx == checkTextIdx )
	{
		obj = cgetText ( para->item );
	}

	else if ( idx == checkIconIdx )
	{
		; /* TODO */
	}

	else if ( idx == checkOnToggledIdx )
	{
		obj = Tcl_NewStringObj ( para->onToggled ? para->onToggled : "", -1 );
	}

	else if ( idx == checkOnValueIdx )
	{
		obj = para->onValue;
	}

	else if ( idx == checkOffValueIdx )
	{
		obj = para->offValue;
	}

	else if ( idx == checkVariableIdx )
	{
		obj = Tcl_NewStringObj ( para->variable, -1 );
	}

	else if ( idx == checkActiveIdx )
	{
		gboolean on;
		g_object_get ( G_OBJECT ( para->item ), "active", &on, NULL );
		obj = Tcl_NewBooleanObj ( on );
	}

	else if ( idx == checkValueIdx )
	{
		gboolean on;
		g_object_get ( G_OBJECT ( para->item ), "active", &on, NULL );
		obj = on ? para->onValue : para->offValue;
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
static int checktoolButtonFunc ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{
	static const char *cmds[] = { "delete", "configure", "cget", "onToggled", "class", NULL };
	enum cmdIdx { DeleteIdx, ConfigureIdx, CgetIdx, OnToggledIdx, ClassIdx };
	GnoclToolBarCheckParams *para = ( GnoclToolBarCheckParams * ) data;
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
		case ClassIdx:
			{
				Tcl_SetObjResult ( interp, Tcl_NewStringObj ( "toolBarCheckButton", -1 ) );
			}
			break;
		case DeleteIdx:
			{
				return gnoclDelete ( interp, para->item, objc, objv );
			}
		case ConfigureIdx:
			{
				int ret = TCL_ERROR;

				if ( gnoclParseAndSetOptions ( interp, objc - 1, objv + 1,
											   checkOptions, G_OBJECT ( para->item ) ) == TCL_OK )
				{
					int k;

					for ( k = 0; k < checkOnToggledIdx; ++k )
					{
						if ( checkOptions[k].status == GNOCL_STATUS_CHANGED )
						{
							Tcl_AppendResult ( interp, "Option \"",
											   checkOptions[k].optName,
											   "\" cannot be set after widget creation.",
											   NULL );
							gnoclClearOptions ( checkOptions );
							return TCL_ERROR;
						}
					}

					ret = checkConfigure ( interp, para, checkOptions );
				}

				gnoclClearOptions ( checkOptions );

				return ret;
			}

			break;

		case CgetIdx:
			{
				int     idx;

				switch ( gnoclCget ( interp, objc, objv, G_OBJECT ( para->item ),
									 checkOptions, &idx ) )
				{
					case GNOCL_CGET_ERROR:
						return TCL_ERROR;
					case GNOCL_CGET_HANDLED:
						return TCL_OK;
					case GNOCL_CGET_NOTHANDLED:
						return checkCget ( interp, para, checkOptions, idx );
				}
			}

		case OnToggledIdx:
			return gnoclCheckOnToggled ( interp, objc, objv, para );
	}

	return TCL_OK;
}

/**
\brief
**/
/*
   -------------- radio functions ----------------------------
*/
static int radioConfigure ( Tcl_Interp *interp, GnoclRadioParams *para, GnoclOption options[] )
{
	/* set an initial value for the radio button group */
	if ( gnoclRadioSetValueActive ( para, &options[radioOnValueIdx],
									&options[radioActiveIdx] ) != TCL_OK )
	{
		return TCL_ERROR;
	}

	/* when toggled */
	if ( options[radioOnToggledIdx].status == GNOCL_STATUS_CHANGED )
	{
		GNOCL_MOVE_STRING ( options[radioOnToggledIdx].val.str, para->onToggled );
	}

	if ( options[radioValueIdx].status == GNOCL_STATUS_CHANGED )
	{
		if ( gnoclRadioSetValue ( para, options[radioValueIdx].val.obj ) != TCL_OK )
		{
			return TCL_ERROR;
		}
	}

	gnoclAttachVariable (
		&options[radioVariableIdx], &para->group->variable,
		"toggled", G_OBJECT ( para->widget ),
		G_CALLBACK ( gnoclRadioToggledFunc ), para->group->interp,
		gnoclRadioTraceFunc, para );


	/*
			if ( gnoclRadioSetActive ( para, &options[radioActiveIdx] ) == 0
					&& ( options[radioVariableIdx].status == GNOCL_STATUS_CHANGED
						 || options[radioOnValueIdx].status == GNOCL_STATUS_CHANGED
						 || options[radioOffValueIdx].status == GNOCL_STATUS_CHANGED ) )
			{
				gnoclRadioVariableValueChanged ( para );
			}
	*/

	if ( options[radioValueIdx].status == GNOCL_STATUS_CHANGED )
	{
		if ( gnoclRadioSetValue ( para, options[radioValueIdx].val.obj ) != TCL_OK )
		{
			return TCL_ERROR;
		}
	}



	return TCL_OK;
}

/**
\brief
**/
static int radioCget ( Tcl_Interp *interp, GnoclRadioParams *para, GnoclOption options[], int idx )
{


	Tcl_Obj *obj = NULL;

	if ( idx == radioTextIdx )
	{
		obj = cgetText ( para->widget );
	}

	else if ( idx == radioIconIdx )
	{
		; /* TODO */
	}

	else if ( idx == radioOnToggledIdx )
	{
		obj = Tcl_NewStringObj ( para->onToggled ? para->onToggled : "", -1 );
	}

	else if ( idx == radioOnValueIdx )
	{
		GnoclRadioParams *p = gnoclRadioGetActivePara ( para->group );
		obj = p->onValue;
	}

	else if ( idx == radioVariableIdx )
	{
		obj = Tcl_NewStringObj ( para->group->variable, -1 );
	}

	else if ( idx == radioActiveIdx )
	{
		gboolean on;
		g_object_get ( G_OBJECT ( para->widget ), "active", &on, NULL );
		obj = Tcl_NewBooleanObj ( on );
	}

	else if ( idx == radioValueIdx )
	{
		obj = gnoclRadioGetValue ( para );
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
static int radiotoolButtonFunc ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{
	static const char *cmds[] = { "delete", "configure", "cget", "onToggled", "class", NULL };
	enum cmdIdx { DeleteIdx, ConfigureIdx, CgetIdx, OnToggledIdx, ClassIdx};
	GnoclRadioParams *para = ( GnoclRadioParams * ) data;
	int idx;

	if ( objc < 2 )
	{
		Tcl_WrongNumArgs ( interp, 1, objv, "command" );
		return TCL_ERROR;
	}

	if ( Tcl_GetIndexFromObj ( interp, objv[1], cmds, "command",
							   TCL_EXACT, &idx ) != TCL_OK )
		return TCL_ERROR;

	switch ( idx )
	{
		case ClassIdx:
			Tcl_SetObjResult ( interp, Tcl_NewStringObj ( "toolBarRadioButton", -1 ) );
			break;
		case DeleteIdx:
			return gnoclDelete ( interp, para->widget, objc, objv );

		case ConfigureIdx:
			{
				int ret = TCL_ERROR;

				if ( gnoclParseAndSetOptions ( interp, objc - 1, objv + 1,
											   radioOptions, G_OBJECT ( para->widget ) ) == TCL_OK )
				{
					int k;

					for ( k = 0; k < radioActiveIdx; ++k )
					{
						if ( radioOptions[k].status == GNOCL_STATUS_CHANGED )
						{
							Tcl_AppendResult ( interp, "Option \"",
											   radioOptions[k].optName,
											   "\" cannot be set after widget creation.",
											   NULL );
							gnoclClearOptions ( radioOptions );
							return TCL_ERROR;
						}
					}

					ret = radioConfigure ( interp, para, radioOptions );
				}

				gnoclClearOptions ( radioOptions );

				return ret;
			}

			break;

		case CgetIdx:
			{
				int     idx;

				switch ( gnoclCget ( interp, objc, objv, G_OBJECT ( para->widget ), radioOptions, &idx ) )
				{
					case GNOCL_CGET_ERROR:
						return TCL_ERROR;
					case GNOCL_CGET_HANDLED:
						return TCL_OK;
					case GNOCL_CGET_NOTHANDLED:
						return radioCget ( interp, para, radioOptions, idx );
				}
			}

		case OnToggledIdx:
			return gnoclRadioOnToggled ( interp, objc, objv, para );
	}

	return TCL_OK;
}

/**
\brief Description yet to be added.
\author Peter G Baum
*/

/*
   -------------- button functions ----------------------------
*/

/**
\brief
**/
static int menuButtonDoCommand ( ToolButtonMenuParams *para, int background )
{
#ifdef DEBUG_TOOLBAR
	printf ( "%s\n", __FUNCTION__ );
#endif

	if ( para->onClicked )
	{
		GnoclPercSubst ps[] =
		{
			{ 'w', GNOCL_STRING },  /* widget */
			{ 0 }
		};


		ps[0].val.str = para->name;

		return gnoclPercentSubstAndEval ( para->interp, ps, para->onClicked, background );
	}

	return TCL_OK;
}


/**
\brief
**/
static int buttonDoCommand ( ToolButtonParams *para, int background )
{
#ifdef DEBUG_TOOLBAR
	printf ( "%s\n", __FUNCTION__ );
#endif

	if ( para->onClicked )
	{
		GnoclPercSubst ps[] =
		{
			{ 'w', GNOCL_STRING },  /* widget */
			{ 0 }
		};


		ps[0].val.str = para->name;

		return gnoclPercentSubstAndEval ( para->interp, ps, para->onClicked, background );
	}

	return TCL_OK;
}


/**
\brief
**/
static void radioButtonCallback ( GtkWidget *widget, gpointer data )
{
#ifdef DEBUG_TOOLBAR
	printf ( "%s\n", __FUNCTION__ );
#endif
	ToolButtonParams *para = ( ToolButtonParams * ) data;


	//buttonDoCommand ( para, 1 );
}

/**
\brief
**/
static void buttonCallback ( GtkWidget *widget, gpointer data )
{
#ifdef DEBUG_TOOLBAR
	printf ( "%s\n", __FUNCTION__ );
#endif
	ToolButtonParams *para = ( ToolButtonParams * ) data;


	//buttonDoCommand ( para, 1 );
}

/**
\brief
**/
static void menuButtonCallback ( GtkWidget *widget, gpointer data )
{
#ifdef DEBUG_TOOLBAR
	printf ( "%s\n", __FUNCTION__ );
#endif
	ToolButtonMenuParams *para = ( ToolButtonMenuParams * ) data;


	menuButtonDoCommand ( para, 1 );

}


/**
\brief
**/
static void buttonDestroyFunc ( GtkWidget *widget, gpointer data )
{
	ToolButtonParams *p = ( ToolButtonParams * ) data;

	gnoclForgetWidgetFromName ( p->name );
	Tcl_DeleteCommand ( p->interp, p->name );

	g_free ( p->onClicked );
	g_free ( p->name );
	g_free ( p );
}

/**
\brief	Configure the menubutton item object
\note	There are two ways of creating this object using either:
			1) a pre-exisiting widget, or
			2) a toolbar button with a stock item. (default)
		This is a 'one-time' choice has to be made during widget creation.
		Problems redisplaying toolitem widget icon after changing.
		Resolved using call to gtk_widget_show (icon) after changing the icon.
**/
static int menuButtonConfigure ( Tcl_Interp *interp, ToolButtonMenuParams *para, GnoclOption options[] )
{

#ifdef DEBUG_TOOLBAR
	g_print ( "menuButtonConfigure\n" );
#endif

	gchar *label = NULL;
	GtkWidget *icon_widget = NULL;


	/* step 1) create the menu button itself one of two methods: */

	/* method 1) use a predefined button for the menu button itself */


	if ( options[menuButtonTextIdx].status == GNOCL_STATUS_CHANGED )
	{
		label = options[menuButtonTextIdx].val.str;
	}


	if ( options[menuButtonIconWidgetIdx].status == GNOCL_STATUS_CHANGED )
	{
		/* change the button clicked option */

		icon_widget = gnoclGetWidgetFromName ( options[menuButtonIconWidgetIdx].val.str, interp );

	}

	else
	{
		// para->item = gtk_menu_tool_button_new ( NULL, NULL );
		para->item = gtk_menu_tool_button_new_from_stock  ( GTK_STOCK_ABOUT );

	}

	para->item = gtk_menu_tool_button_new ( icon_widget, label );


	/* method 2) create a custom widget */

	if ( options[menuButtonIconIdx].status == GNOCL_STATUS_CHANGED )
	{

		para->item = gtk_menu_tool_button_new_from_stock  ( GTK_STOCK_COPY );
	}

	/*
		if ( options[menuButtonArrowClickedIdx].status == GNOCL_STATUS_CHANGED )
		{
			GNOCL_MOVE_STRING ( options[menuButtonArrowClickedIdx].val.str, para->onClicked );
		}
	*/

	/* step 2) attach a menu to this button */
	/*
		if ( options[menuButtonMenuIdx].status == GNOCL_STATUS_CHANGED )
		{
	#ifdef DEBUG_TOOLBAR
			g_print ( "menu = %s\n", options[menuButtonMenuIdx].val.str );
	#endif
			para->menu = gnoclGetWidgetFromName ( options[menuButtonMenuIdx].val.str, interp );

			gtk_menu_tool_button_set_menu ( para->item, para->menu );
		}
	*/
	return TCL_OK;
}


/**
\brief
**/
static int buttonConfigure ( Tcl_Interp *interp, ToolButtonParams *para, GnoclOption options[] )
{
	if ( options[buttonOnClickedIdx].status == GNOCL_STATUS_CHANGED )
	{
		GNOCL_MOVE_STRING ( options[buttonOnClickedIdx].val.str, para->onClicked );
	}

	return TCL_OK;
}

/**
\brief
**/
int toolButtonFunc ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{
	static const char *cmds[] = { "delete", "configure", "onClicked", "class", "cget", NULL };
	enum cmdIdx { DeleteIdx, ConfigureIdx, OnClickedIdx, ClassIdx, CgetIdx };
	ToolButtonParams *para = ( ToolButtonParams * ) data;
	int idx;

	if ( objc < 2 )
	{
		Tcl_WrongNumArgs ( interp, 1, objv, "command" );
		return TCL_ERROR;
	}

	if ( Tcl_GetIndexFromObj ( interp, objv[1], cmds, "command",
							   TCL_EXACT, &idx ) != TCL_OK )
		return TCL_ERROR;

	switch ( idx )
	{
		case CgetIdx:
			{

			}
		case ClassIdx:
			{
				Tcl_SetObjResult ( interp, Tcl_NewStringObj ( "toolBarButton", -1 ) );
				break;
			}
		case DeleteIdx:
			{
				return gnoclDelete ( interp, GTK_WIDGET ( para->item ), objc, objv );
			}
		case ConfigureIdx:
			{
#ifdef DEBUG_TOOLBAR
				printf ( "toolButtonFunc/configure\n" );
#endif
				int ret = TCL_ERROR;

				if ( gnoclParseAndSetOptions ( interp, objc - 1, objv + 1,
											   buttonOptions, G_OBJECT ( para->item ) ) == TCL_OK )
				{
					ret = buttonConfigure ( interp, para, buttonOptions );
				}

				gnoclClearOptions ( buttonOptions );

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

				return buttonDoCommand ( para, 0 );
			}
	}

	return TCL_OK;
}

/*
int gnoclRegisterWidget ( Tcl_Interp *interp, GtkWidget *widget, Tcl_ObjCmdProc *proc )
{
	const char *name = gnoclGetAutoWidgetId();
	gnoclMemNameAndWidget ( name, widget );

	g_signal_connect_after ( G_OBJECT ( widget ), "destroy", G_CALLBACK ( simpleDestroyFunc ), interp );

	if ( proc != NULL )
	{
		Tcl_CreateObjCommand ( interp, ( char * ) name, proc, widget, NULL );
	}

	Tcl_SetObjResult ( interp, Tcl_NewStringObj ( name, -1 ) );

	return TCL_OK;
}
*/



/**
\brief
**/
static int addCheckButton ( GtkToolbar *toolbar, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[], int atEnd )
{

	int              ret;
	GnoclToolBarCheckParams *para = NULL;
	char             *txt = NULL;
	int              isUnderline;
	GtkWidget        *icon;

	if ( gnoclParseOptions ( interp, objc - 2, objv + 2, checkOptions ) != TCL_OK )
	{
		gnoclClearOptions ( checkOptions );
		return TCL_ERROR;
	}


	if ( getTextAndIcon ( interp, toolbar,
						  checkOptions + checkTextIdx, checkOptions + checkIconIdx,
						  &txt, &icon, &isUnderline ) != TCL_OK )
	{
		return TCL_ERROR;
	}

	para = g_new ( GnoclToolBarCheckParams, 1 );

	para->onToggled = NULL;

	para->interp = interp;

	para->name = gnoclGetAutoWidgetId();

	para->variable = NULL;

	para->onValue = Tcl_NewIntObj ( 1 );

	Tcl_IncrRefCount ( para->onValue );

	para->offValue = Tcl_NewIntObj ( 0 );

	Tcl_IncrRefCount ( para->offValue );

	para->inSetVar = 0;

	/* this is the requirement after Gtk+ 2.4 */
	//para->item = gtk_tool_button_new_from_stock ( GTK_STOCK_COPY );
	//para->widget =   gtk_toggle_tool_button_new_from_stock ( GTK_STOCK_COPY );

	//GtkToggleAction *   gtk_toggle_action_new (const gchar *name, const gchar *label, const gchar *tooltip, const gchar *stock_id);

	/* create base object then add icon and label widgets */
	para->item = gtk_toggle_tool_button_new ();
	gtk_tool_button_set_icon_widget ( para->item, icon );
	gtk_widget_show ( icon );
	gtk_tool_button_set_label ( para->item, txt );

	//gtk_tool_button_set_label_widget (para->item, txt);
	//g_signal_connect ( para->item , "toggled", G_CALLBACK ( gnoclCheckToggledFunc ), para );

	gtk_toolbar_insert ( GTK_TOOLBAR ( toolbar ),  para->item, -1 );
	gtk_widget_show ( GTK_WIDGET (  para->item ) );

	if ( isUnderline )
	{
		setUnderline ( para->item );
	}

	ret = gnoclSetOptions ( interp, checkOptions, G_OBJECT ( para->item ), -1 );

	if ( ret == TCL_OK )
	{
		ret = checkConfigure ( interp, para, checkOptions );
	}

	if ( ret != TCL_OK )
	{
		g_free ( para );
	}

	else
	{
		g_signal_connect_after ( G_OBJECT ( para->item ), "destroy", G_CALLBACK ( checkDestroyFunc ), para );

		gnoclMemNameAndWidget ( para->name, para->item );

		Tcl_CreateObjCommand ( interp, para->name, checktoolButtonFunc, para, NULL );
		Tcl_SetObjResult ( interp, Tcl_NewStringObj ( para->name, -1 ) );
	}

	gnoclClearOptions ( checkOptions );

	return ret;
}

/**
\brief
\note	Using new Toolbar API. Gtk+ 2.16
**/
static int addButton ( gchar *type, GtkToolbar *toolbar, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[], int pos )
{
#ifdef DEBUG_TOOLBAR
	listParameters ( objc, objv, __FUNCTION__ );
#endif

	int          ret;
	ToolButtonParams *para = NULL;
	char         *txt = NULL;
	int          isUnderline;
	GtkWidget    *icon;				/* image created by getTextAndIcon to hold icon graphic*/

	if ( gnoclParseOptions ( interp, objc - 2, objv + 2, buttonOptions ) != TCL_OK )
	{
		gnoclClearOptions ( buttonOptions );
		return TCL_ERROR;
	}

	gint i;

	/* add label and icon widget */
	if ( getTextAndIcon (
				interp, toolbar,
				buttonOptions + buttonTextIdx,
				buttonOptions + buttonIconIdx,
				&txt, &icon, &isUnderline ) != TCL_OK )
	{
		return TCL_ERROR;
	}

	para = g_new ( ToolButtonParams, 1 );

	para->onClicked = NULL;

	para->interp = interp;

	para->name = gnoclGetAutoWidgetId();

	para->item =  gtk_tool_button_new ( icon, txt );

	//g_signal_connect ( para->item , "clicked", G_CALLBACK ( buttonCallback ), para );

	gtk_toolbar_insert ( GTK_TOOLBAR ( toolbar ),  para->item, -1 );

	gtk_widget_show ( GTK_WIDGET (  para->item ) );

	if ( isUnderline )
	{
		setUnderline ( para->item );
	}

	ret = gnoclSetOptions ( interp, buttonOptions, G_OBJECT ( para->item ), -1 );

	if ( ret == TCL_OK )
	{
		ret = buttonConfigure ( interp, para, buttonOptions );
	}

	if ( ret != TCL_OK )
	{
		g_free ( para );
	}

	else
	{
		g_signal_connect_after ( G_OBJECT ( para->item ), "destroy", G_CALLBACK ( buttonDestroyFunc ), para );

		gnoclMemNameAndWidget ( para->name, GTK_WIDGET ( para->item ) );

		Tcl_CreateObjCommand ( interp, para->name, toolButtonFunc, para, NULL );
		Tcl_SetObjResult ( interp, Tcl_NewStringObj ( para->name, -1 ) );
	}

	gnoclClearOptions ( buttonOptions );

	return ret;
}



/**
\brief
**/
static int addRadioButton ( GtkToolbar *toolbar, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[], int atEnd )
{
	int              ret;
	GnoclRadioParams *para = NULL;
	char             *txt = NULL;
	int              isUnderline;
	GtkWidget        *icon;
	GtkWidget        *firstWidget = NULL;

	GSList* list = NULL;



	/* check options */
	if ( gnoclParseOptions ( interp, objc - 2, objv + 2, radioOptions ) != TCL_OK )
	{
		gnoclClearOptions ( radioOptions );
		return TCL_ERROR;
	}



	/* Confirm that the mandatory -value or -variable options have been set. */
	if ( radioOptions[radioOnValueIdx].status != GNOCL_STATUS_CHANGED ||
			radioOptions[radioVariableIdx].status != GNOCL_STATUS_CHANGED )
	{
		gnoclClearOptions ( radioOptions );
		Tcl_SetResult ( interp, "Option \"-onValue\" and \"-variable\" are required.", TCL_STATIC );
		return TCL_ERROR;
	}



	/* Create the toobutton item and assign text label and icon. */
	if ( getTextAndIcon (
				interp, toolbar,
				radioOptions + radioTextIdx,
				radioOptions + radioIconIdx,
				&txt, &icon, &isUnderline ) != TCL_OK )
	{
		return TCL_ERROR;

	}



	/* Initialize object parameters. */
	para = g_new ( GnoclRadioParams, 1 );

	para->name = gnoclGetAutoWidgetId();

	para->onToggled = NULL;

	para->onValue = NULL;

	para->group = gnoclRadioGetGroupFromVariable ( radioOptions[radioVariableIdx].val.str );


	/* create new group if necessary */
	if ( para->group == NULL )
	{

		para->group = gnoclRadioGroupNewGroup ( radioOptions[radioVariableIdx].val.str, interp );

		para->widget = gtk_radio_tool_button_new ( NULL );

	}

	else
	{

		GnoclRadioParams *p = gnoclRadioGetParam ( para->group, 0 );

		firstWidget = p->widget;

		para->widget =  gtk_radio_tool_button_new_from_widget ( firstWidget );

	}



	/* should this be moved elsewhere? */
	g_signal_connect ( para->widget , "toggled", G_CALLBACK ( gnoclRadioToggledFunc ), para );

	/* use custom icon and label widgets */
	gtk_tool_button_set_icon_widget ( para->widget, icon );
	gtk_widget_show ( icon );
	gtk_tool_button_set_label ( para->widget, txt );

	/* add widget to toolbar and display */
	gtk_toolbar_insert ( GTK_TOOLBAR ( toolbar ),  para->widget, -1 );
	gtk_widget_show ( GTK_WIDGET (  para->widget ) );


	/*-----*/


	if ( isUnderline )
	{
		setUnderline ( para->widget );
	}

	/* add widget to group */
	gnoclRadioGroupAddWidgetToGroup ( para->group, para );


	/* configure the radiobutton */
	if ( gnoclSetOptions ( interp, radioOptions, G_OBJECT ( para->widget ), -1 ) == TCL_OK )
	{
		ret = radioConfigure ( interp, para, radioOptions );
	}



	if ( ret != TCL_OK )
	{
		g_free ( para );
	}

	else
	{
		g_signal_connect_after ( G_OBJECT ( para->widget ), "destroy", G_CALLBACK ( gnoclRadioDestroyFunc ), para );

		gnoclMemNameAndWidget ( para->name, para->widget );

		Tcl_CreateObjCommand ( interp, para->name, radiotoolButtonFunc, para, NULL );
		Tcl_SetObjResult ( interp, Tcl_NewStringObj ( para->name, -1 ) );

	}

	gnoclClearOptions ( radioOptions );

	return ret;
}

/**
\brief
\note	Using new Toolbar API. Gtk+ 2.16
**/
static int addMenuButton ( GtkToolbar *toolbar, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[], int pos )
{

#ifdef DEBUG_TOOLBAR
	listParameters ( objc, objv, __FUNCTION__ );
#endif


	int          ret;
	ToolButtonParams *para = NULL;
	char         *txt = NULL;
	int          isUnderline;
	GtkWidget    *icon;				/* image created by getTextAndIcon to hold icon graphic*/

	if ( gnoclParseOptions ( interp, objc - 2, objv + 2, menuButtonOptions ) != TCL_OK )
	{
		gnoclClearOptions ( menuButtonOptions );
		return TCL_ERROR;
	}

	gint i;

	/* add label and icon widget */
	if ( getTextAndIcon (
				interp, toolbar,
				menuButtonOptions + menuButtonTextIdx,
				menuButtonOptions + menuButtonIconIdx,
				&txt, &icon, &isUnderline ) != TCL_OK )
	{
		return TCL_ERROR;
	}

	para = g_new ( ToolButtonParams, 1 );

	para->onClicked = NULL;

	para->interp = interp;

	para->name = gnoclGetAutoWidgetId();

	//para->item =  gtk_tool_button_new ( icon, txt );

	para->item = gtk_menu_tool_button_new ( icon, txt );

	//g_signal_connect ( para->item , "clicked", G_CALLBACK ( buttonCallback ), para );

	gtk_toolbar_insert ( GTK_TOOLBAR ( toolbar ),  para->item, -1 );
	gtk_widget_show ( GTK_WIDGET (  para->item ) );
	/*-----*/

	if ( isUnderline )
	{
		setUnderline ( para->item );
	}

	ret = gnoclSetOptions ( interp, menuButtonOptions, G_OBJECT ( para->item ), -1 );

	if ( ret == TCL_OK )
	{
		ret = menuButtonConfigure ( interp, para, menuButtonOptions );
	}

	if ( ret != TCL_OK )
	{
		g_free ( para );
	}

	else
	{
		g_signal_connect_after ( G_OBJECT ( para->item ), "destroy", G_CALLBACK ( buttonDestroyFunc ), para );

		gnoclMemNameAndWidget ( para->name, GTK_WIDGET ( para->item ) );

		Tcl_CreateObjCommand ( interp, para->name, toolButtonFunc, para, NULL );
		Tcl_SetObjResult ( interp, Tcl_NewStringObj ( para->name, -1 ) );
	}

	gnoclClearOptions ( menuButtonOptions );

	return ret;
}

/**
\brief
**/
/* WJG added menuButton item 29/12/07 */

static int addItem ( GtkToolbar *toolbar, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[], int atEnd )
{

	const char *txt[] =
	{

		"button", "item",
		"toggleButton", "checkItem",
		"radioButton", "radioItem",
		"separator", "space",
		"menuButton",
		"widget",
		NULL
	};
	enum typeIdx
	{
		ButtonIdx, ItemIdx,
		ToggleButtonIdx, CheckItemIdx,
		RadioButtonIdx, RadioItemIdx,
		SpaceIdx, SeparatorIdx,
		MenuButtonIdx,
		WidgetIdx,
	};
	int idx;

	if ( objc < 3 )
	{
		Tcl_WrongNumArgs ( interp, 2, objv, "type ?option val ...?" );
		return TCL_ERROR;
	}

	if ( Tcl_GetIndexFromObj ( interp, objv[2], txt, "type", TCL_EXACT, &idx ) != TCL_OK )
	{
		return TCL_ERROR;
	}

	switch ( idx )
	{
		case SeparatorIdx:
		case SpaceIdx:
			{
				gint pos;
				gint n = -1;

				GtkToolItem *item = NULL;

				if ( objc < 3 )
				{
					Tcl_WrongNumArgs ( interp, 3, objv, NULL );
					return TCL_ERROR;
				}

				item = gtk_separator_tool_item_new ();

				gtk_widget_show ( item );

				if ( objc == 5 )
				{

					Tcl_GetIntFromObj ( NULL, objv[4], &pos );

					n = gtk_toolbar_get_n_items ( toolbar );

					gtk_toolbar_insert ( GTK_TOOLBAR ( toolbar ), item, n );

					return TCL_OK;

				}

				if ( atEnd )
				{
					gtk_toolbar_insert ( GTK_TOOLBAR ( toolbar ), item, -1 );
				}

				else
				{
					gtk_toolbar_insert ( GTK_TOOLBAR ( toolbar ), item, 0 );
				}
			}

			break;
		case WidgetIdx:
			{
				GtkWidget *child;

				if ( objc != 4 )
				{
					Tcl_WrongNumArgs ( interp, 3, objv, "widget-ID" );
					return TCL_ERROR;
				}

				child = gnoclGetWidgetFromName ( Tcl_GetString ( objv[3] ),	 interp );

				if ( child == NULL )
				{
					return TCL_ERROR;
				}

				// GtkToolItems are widgets that can appear on a toolbar.
				// To create a toolbar item that contain something else than a button, use gtk_tool_item_new().
				// Use gtk_container_add() to add a child widget to the tool item.

				GtkToolItem *item = gtk_tool_item_new ();

				gtk_container_add ( item, child );


				if ( atEnd )
				{
					gtk_toolbar_insert ( toolbar, item, -1 );
				}

				else
				{
					gtk_toolbar_insert ( toolbar, item, 0 );
				}

				gtk_widget_show_all ( item );

				Tcl_SetObjResult ( interp, objv[3] );

				return TCL_OK;
			}

		case ItemIdx:
		case ButtonIdx:
			{
				return addButton ( "button", toolbar, interp, objc, objv, atEnd );
			}
		case ToggleButtonIdx:
		case CheckItemIdx:
			{
				return addCheckButton ( toolbar, interp, objc, objv, atEnd );
			}
		case RadioButtonIdx:
		case RadioItemIdx:
			{
				return addRadioButton ( toolbar, interp, objc, objv, atEnd );
			}
		case MenuButtonIdx:
			{
				return addMenuButton ( toolbar, interp, objc, objv, atEnd );
			}
	}

	return TCL_OK;
}

/**
\brief
\note	Due to the deprecation of gtk_toolbar_prepend_item etc., the following command need to
		be recoded to reflect the use of gtk_toolbar_insert

		"add", "addBegin", "addEnd"

**/
int toolBarFunc ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{
	static const char *cmds[] =
	{
		"flip", "add", "addBegin", "addEnd",
		"class", "configure", "delete",
		"insert", "nItems", "cget",
		"parent", "options", "commands",
		NULL
	};

	enum cmdIdx
	{
		FlipIdx, AddIdx, BeginIdx, EndIdx,
		ClassIdx, ConfigureIdx, DeleteIdx,
		InsertIdx, NitemsIdx, CgetIdx,
		ParentIdx, OptionsIdx, CommandsIdx
	};

	GtkToolbar *toolBar = GTK_TOOLBAR ( data );
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
		case CommandsIdx:
			{
				gnoclGetOptions ( interp, cmds );
			}
			break;
		case OptionsIdx:
			{
				gnoclGetOptions ( interp, toolBarOptions );
			}
			break;
		case FlipIdx:
			{

				if ( gtk_toolbar_get_orientation ( toolBar ) == GTK_ORIENTATION_HORIZONTAL )
				{
					gtk_orientable_set_orientation ( GTK_ORIENTABLE ( toolBar ), GTK_ORIENTATION_VERTICAL );
				}

				else
				{
					gtk_orientable_set_orientation ( GTK_ORIENTABLE ( toolBar ), GTK_ORIENTATION_HORIZONTAL );
				}
			}
			break;
		case ParentIdx:
			{

				GtkWidget * parent;
				Tcl_Obj *obj = NULL;
				parent = gtk_widget_get_parent ( GTK_WIDGET ( toolBar ) );
				obj = Tcl_NewStringObj ( gnoclGetNameFromWidget ( parent ), -1 );
				Tcl_SetObjResult ( interp, obj );

				/* this function not working too well! */
				/* return gnoclGetParent ( interp, data ); */
				return TCL_OK;
			}

			break;
		case CgetIdx:
			{
				int idx;

				switch ( gnoclCget ( interp, objc, objv, G_OBJECT ( toolBar ), toolBarOptions, &idx ) )
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
							return toolBarCget ( interp, toolBar, toolBarOptions, idx );
						}
				}
			}
			break;
		case NitemsIdx:
			{

				gchar str[4];
				gint n;

				n = gtk_toolbar_get_n_items ( toolBar );

				sprintf ( str, "%d", n );

				Tcl_SetObjResult ( interp, Tcl_NewStringObj ( str, -1 ) );
			}
			break;
		case ClassIdx:
			{
				Tcl_SetObjResult ( interp, Tcl_NewStringObj ( "toolBar", -1 ) );
			}
			break;
		case DeleteIdx:
			{
				return gnoclDelete ( interp, GTK_WIDGET ( toolBar ), objc, objv );
			}
		case ConfigureIdx:
			{
				//int ret = gnoclParseAndSetOptions ( interp, objc - 1, objv + 1, toolBarOptions, G_OBJECT ( toolBar ) );
				//gnoclClearOptions ( toolBarOptions );
				//return ret;
				int ret = TCL_ERROR;

				if ( gnoclParseAndSetOptions ( interp, objc - 1, objv + 1, toolBarOptions, G_OBJECT ( toolBar ) ) == TCL_OK )
				{
					ret = configure ( interp, toolBar, toolBarOptions );
				}

				gnoclClearOptions ( toolBarOptions );

				return ret;



			}

			break;

		case AddIdx:      /* add is a shortcut for addEnd */
		case BeginIdx:
		case EndIdx:
			{
				return addItem ( toolBar, interp, objc, objv, idx != BeginIdx );
			}
	}

	return TCL_OK;
}

/**
\brief	Create an instance of a gnocl:toolBar widget.
**/
int gnoclToolBarCmd ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{
	int        ret;
	GtkToolbar *toolBar;

	if ( gnoclParseOptions ( interp, objc, objv, toolBarOptions ) != TCL_OK )
	{
		gnoclClearOptions ( toolBarOptions );
		return TCL_ERROR;
	}

	toolBar = GTK_TOOLBAR ( gtk_toolbar_new() );

	/* turn arrows off, this will prevent problems with toolbars in handleboxes */
	gtk_toolbar_set_show_arrow ( toolBar, 0 );

	gtk_widget_show ( GTK_WIDGET ( toolBar ) );

	ret = gnoclSetOptions ( interp, toolBarOptions, G_OBJECT ( toolBar ), -1 );

	if ( ret == TCL_OK )
	{
		ret = configure ( interp, toolBar, toolBarOptions );
	}

	gnoclClearOptions ( toolBarOptions );

	if ( ret != TCL_OK )
	{
		gtk_widget_destroy ( GTK_WIDGET ( toolBar ) );
		return TCL_ERROR;
	}

	return gnoclRegisterWidget ( interp, GTK_WIDGET ( toolBar ), toolBarFunc );
}


