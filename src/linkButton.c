/* linkButton.c
authors	Peter G Baum	William J Giddings
date	2008-06
*/

/**
\par Modification History
\verbatim
	2013-07:	added commands, options, commands
\endverbatim
**/

/*
 * The main functions a
GtkWidget*          gtk_link_button_new                 (const gchar *uri);
GtkWidget*          gtk_link_button_new_with_label      (const gchar *uri, const gchar *label);
const gchar*        gtk_link_button_get_uri             (GtkLinkButton *link_button);
void                gtk_link_button_set_uri             (GtkLinkButton *link_button, const gchar *uri);
void                (*GtkLinkButtonUriFunc)             (GtkLinkButton *button, const gchar *link_, gpointer user_data);
GtkLinkButtonUriFunc gtk_link_button_set_uri_hook       (GtkLinkButtonUriFunc func, gpointer data, GDestroyNotify destroy);
*/

#include "gnocl.h"

/**
\page page_linkButton gnocl::linkButton
\htmlinclude linkButton.html
**/

GnoclOption linkButtonOptions[] =
{
	{ "-uri", GNOCL_OBJ, NULL },        /* 0 */
	{ "-text", GNOCL_OBJ, NULL },
	{ "-activeBackgroundColor", GNOCL_OBJ, "active", gnoclOptGdkColorBg },
	{ "-data", GNOCL_OBJ, "", gnoclOptData },
	{ "-hasFocus", GNOCL_BOOL, "has-focus" },
	{ "-heightGroup", GNOCL_OBJ, "h", gnoclOptSizeGroup },
	{ "-name", GNOCL_STRING, "name" },
	{ "-normalBackgroundColor", GNOCL_OBJ, "normal", gnoclOptGdkColorBg },
	{ "-onClicked", GNOCL_OBJ, "clicked", gnoclOptOnLinkButton },
	{ "-onRealize", GNOCL_OBJ, "realize", gnoclOptCommand },
	{ "-onShowHelp", GNOCL_OBJ, "", gnoclOptOnShowHelp },
	{ "-prelightBackgroundColor", GNOCL_OBJ, "prelight", gnoclOptGdkColorBg },
	{ "-relief", GNOCL_OBJ, "relief", gnoclOptRelief },
	{ "-sensitive", GNOCL_BOOL, "sensitive" },
	{ "-sizeGroup", GNOCL_OBJ, "s", gnoclOptSizeGroup },
	{ "-tooltip", GNOCL_OBJ, "", gnoclOptTooltip },
	{ "-visible", GNOCL_BOOL, "visible" },
	{ "-widthGroup", GNOCL_OBJ, "w", gnoclOptSizeGroup },
	{ NULL },
};

static const int textIdx = 0;
static const int iconIdx = 1;

/**
\brief
**/
static int configure ( Tcl_Interp *interp, GtkWidget *widget, GnoclOption options[] )
{
#ifdef DEBUG
	printf ( "linkButton configure not fully implemented\n" );
#endif
	return TCL_OK;
}

/**
\brief
**/
int gnoclConfigLinkButtonText ( Tcl_Interp *interp, GtkWidget *widget, Tcl_Obj *txtObj )
{
	GnoclStringType type = gnoclGetStringType ( txtObj );

	if ( type & GNOCL_STR_STOCK )
	{
		GtkStockItem sp;

		if ( gnoclGetStockItem ( txtObj, interp, &sp ) != TCL_OK )
			return TCL_ERROR;

		gtk_button_set_label ( widget, sp.stock_id );

		gtk_button_set_use_stock ( widget, 1 );
	}

	else
	{
		GtkLabel *label;
		char *txt = gnoclGetString ( txtObj );

		gtk_button_set_label ( widget, txt );
		gtk_button_set_use_stock ( widget, 0 );

		label = GTK_LABEL ( gnoclFindChild ( GTK_WIDGET ( widget ),  GTK_TYPE_LABEL ) );
		assert ( label );
		/* TODO? pango_parse_markup for error message */
		gtk_label_set_use_markup ( label, ( type & GNOCL_STR_MARKUP ) != 0 );
		gtk_label_set_use_underline ( label,  ( type & GNOCL_STR_UNDERLINE ) != 0 );
	}

	return TCL_OK;
}

/**
\brief
**/
Tcl_Obj *gnoclCgetLinkButtonText ( Tcl_Interp *interp, GtkWidget *widget )
{
	Tcl_Obj *obj = NULL;

	if ( gtk_button_get_use_stock ( widget ) )
	{
		const char *st = gtk_button_get_label ( widget );

		if ( st == NULL )
			obj = Tcl_NewStringObj ( "", 0 );
		else
		{
			obj = Tcl_NewStringObj ( "%#", 2 );
			Tcl_AppendObjToObj ( obj, gnoclGtkToStockName ( st ) );
		}
	}

	else
	{
		GtkLabel *label = GTK_LABEL ( gnoclFindChild ( GTK_WIDGET ( widget ), GTK_TYPE_LABEL ) );
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
			obj = old;
	}

	return obj;
}

/**
\brief
**/
static int cget ( Tcl_Interp *interp, GtkWidget *widget, GnoclOption options[], int idx )
{
	Tcl_Obj *obj = NULL;

	if ( idx == textIdx )
		obj = gnoclCgetLinkButtonText ( interp, widget );
	else if ( idx == iconIdx )
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

	return gnoclCgetNotImplemented ( interp, options + idx );
}

static const char *cmds[] = { "delete", "configure", "cget", "onClicked", "class",  NULL };

/**
\brief
**/
int linkButtonFunc ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{

	enum cmdIdx { DeleteIdx, ConfigureIdx, CgetIdx, OnClickedIdx, ClassIdx };

#ifdef DEBUG
	printf ( "linkButtonFunc 1\n" );
#endif
	GtkWidget *button = GTK_LINK_BUTTON ( data );
#ifdef DEBUG
	printf ( "linkButtonFunc 2\n" );
#endif
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
				Tcl_SetObjResult ( interp, Tcl_NewStringObj ( "linkButton", -1 ) );
			}
			break;
		case DeleteIdx:
			{
				return gnoclDelete ( interp, GTK_WIDGET ( button ), objc, objv );
			}

		case ConfigureIdx:
			{
#ifdef DEBUG
				printf ( "linkButtonFunc ConfigureIdx\n" );
#endif
				int ret = TCL_ERROR;

				if ( gnoclParseAndSetOptions ( interp, objc - 1, objv + 1, linkButtonOptions, button ) == TCL_OK )
				{
					ret = configure ( interp, button, linkButtonOptions );
				}

				gnoclClearOptions ( linkButtonOptions );

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
				int     idx;

				switch ( gnoclCget ( interp, objc, objv, G_OBJECT ( button ), linkButtonOptions, &idx ) )
				{
					case GNOCL_CGET_ERROR:
						return TCL_ERROR;
					case GNOCL_CGET_HANDLED:
						return TCL_OK;
					case GNOCL_CGET_NOTHANDLED:
						return cget ( interp, button, linkButtonOptions, idx );
				}
			}
	}

	return TCL_OK;
}

/**
\brief
**/
int gnoclLinkButtonCmd ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{

	if ( gnoclGetCmdsAndOpts ( interp, cmds, linkButtonOptions, objv, objc ) == TCL_OK )
	{
		return TCL_OK;
	}

	int         ret;
	GtkWidget   *widget;
	int         k, a, b;
	GnoclOption *pop;
	char        *str_label, *str_uri ;

	/* determine which type of linkButton to create... */
	/*
	 * Do some pre-checking on the options set.
	 * The gtk library has two forms of linkbutton, on with a label or one that simply shows the uri
	 * these values are set at initialisation and so the argument line needs to be parsed so
	 * that a decision can be made as to which type of widget to create.
	 *
	 */

	/*
	 * Parse all Options
	 * Picking out individual options from the active script command line.
	 */

	for ( k = 1; k < objc; k += 2 )
	{
		int idx;

		if ( gnoclGetIndexFromObjStruct (
					interp, objv[k],
					( char ** ) &linkButtonOptions[0].optName,
					sizeof ( GnoclOption ),
					"option",
					TCL_EXACT, &idx ) != TCL_OK )
		{
			return -1;
		}


		/* originally, commented out */
		/*
		printf( "parsing %s -> %s\n", Tcl_GetString( objv[k] ), linkButtonOptions[idx].optName);
		*/

		if ( strcmp ( Tcl_GetString ( objv[k] ), "-text" ) == 0 )
		{
			str_label = Tcl_GetString ( objv[k+1] );
			a = 1;
		}

		if ( strcmp ( Tcl_GetString ( objv[k] ), "-uri" ) == 0 )
		{
			str_uri = Tcl_GetString ( objv[k+1] );
			b = 1;
		}


	}   /* decide what to do based on above values in step 2*/


	/* STEP 1)  -check validity of switches */

	if ( gnoclParseOptions ( interp, objc, objv, linkButtonOptions ) != TCL_OK )
	{
		gnoclClearOptions ( linkButtonOptions );
		return TCL_ERROR;
	}

	/* STEP 2)  -create widget, configure it, then clear memory */

	/*
	 * a== 1 and b == 1, then create a button with a label.
	 * if a!= 1 and b == 1, then create a button with the URI as label
	 * if b != 1, then this is an error. -uri is a mandatory option.
	 */

	if ( a == 1 && b == 1 )
	{
		//printf ("label %s and URI %s\n", str_label, str_uri );
		widget = GTK_WIDGET ( gtk_link_button_new_with_label ( str_uri, str_label ) );
	}

	else if ( b == 1 )
	{
		//printf ("URI %s\n", str_uri );
		widget = gtk_link_button_new ( str_uri );
	}

	else
	{
		//printf ("error\n" );
	}

	a = 0; b = 0;


	gtk_widget_show ( GTK_WIDGET ( widget ) );

	ret = gnoclSetOptions ( interp, linkButtonOptions, G_OBJECT ( widget ), -1 );

	if ( ret == TCL_OK )
	{
		ret = configure ( interp, G_OBJECT ( widget ), linkButtonOptions );
	}

	gnoclClearOptions ( linkButtonOptions );

	/* STEP 3)  -show the widget */

	if ( ret != TCL_OK )
	{
		gtk_widget_destroy ( GTK_WIDGET ( widget ) );
		return TCL_ERROR;
	}

	/* STEP 4)  -everything has worked, register the widget with the Tcl interpretor. */
	return gnoclRegisterWidget ( interp, GTK_WIDGET ( widget ), linkButtonFunc );
}


