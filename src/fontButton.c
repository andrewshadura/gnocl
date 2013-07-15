/*
 * $Id: fontButton.c,v 1.2 2005/01/01 15:27:54 baum Exp $
 *
 * This file implements the fontButton widget
 *
 * Copyright (c) 2001 - 2005 Peter G. Baum  http://www.dr-baum.net
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

/*
   History:
   2013-07:	added commands, options, commands
   2008-10: added command, class
   2004-12: Begin of developement
*/


/**
\page page_fontButton gnocl::fontButton
\htmlinclude fontButton.html
**/

#include "gnocl.h"

static const int baseFontIdx = 0;

static GnoclOption fontButtonOptions[] =
{

	{ "-baseFont", GNOCL_OBJ, NULL }, /* 4 */

	/* GtkFontButton properties */
	{ "-fontName", GNOCL_STRING, "font-name" },
	{ "-showSize", GNOCL_BOOL, "show-size" },
	{ "-showStyle", GNOCL_BOOL, "show-style" },
	{ "-title", GNOCL_STRING, "title" },
	{ "-useFont", GNOCL_BOOL, "use-font" },
	{ "-useSize", GNOCL_BOOL, "use-size" },

	/* GtkFontButton signals */
	{"-onFontSet", GNOCL_OBJ, "", gnoclOptOnFontSet },

	/* other properties */
	{ "-data", GNOCL_OBJ, "", gnoclOptData },
	{ "-name", GNOCL_STRING, "name" },
	{ "-tooltip", GNOCL_OBJ, "", gnoclOptTooltip },
	{ NULL },
};

/**
\brief
**/
static int configure (  Tcl_Interp *interp, GtkButton *button,  GnoclOption options[] )
{

#ifdef DEBUG
	printf ( "fontButton configure - no special requirements.\n" );
#endif

	if ( options[baseFontIdx].status == GNOCL_STATUS_CHANGED )
	{

		char *fnt;
		GtkWidget *label;

		fnt = Tcl_GetString ( options[baseFontIdx].val.obj );
		label = gnoclFindChild ( GTK_WIDGET ( button ), GTK_TYPE_LABEL );

		PangoFontDescription *font_desc = pango_font_description_from_string ( fnt );

		gtk_widget_modify_font ( GTK_WIDGET ( label ), font_desc );
		pango_font_description_free ( font_desc );

	}


	return TCL_OK;

}

static const char *cmds[] = { "delete", "configure", "cget", "onClicked", "class",  NULL };

/**
\brief      This function is only concerned with the reconfiguration of the button, not the dialog.
**/
int fontButtonFunc ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{

	enum cmdIdx { DeleteIdx, ConfigureIdx, CgetIdx, OnClickedIdx, ClassIdx };

	GtkFontButton *button = GTK_FONT_BUTTON ( data );
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
				Tcl_SetObjResult ( interp, Tcl_NewStringObj ( "fontButton", -1 ) );
			}
			break;
		case DeleteIdx:
			{
				return gnoclDelete ( interp, GTK_WIDGET ( button ), objc, objv );
			}

		case ConfigureIdx:
			{
				int ret = TCL_ERROR;

				if ( gnoclParseAndSetOptions ( interp, objc - 1, objv + 1, fontButtonOptions, G_OBJECT ( button ) ) == TCL_OK )
				{
					ret = configure ( interp, button, fontButtonOptions );
				}

				gnoclClearOptions ( fontButtonOptions );

				return ret;
			}

			break;
		case OnClickedIdx:
			{
#ifdef DEBUG
				printf ( "fontButtonFunc - onClicked \n" );
#endif

				if ( objc != 2 )
				{
					Tcl_WrongNumArgs ( interp, 2, objv, NULL );
					return TCL_ERROR;
				}

				if ( GTK_WIDGET_IS_SENSITIVE ( GTK_WIDGET ( button ) ) )
				{
					gtk_button_clicked ( GTK_BUTTON ( button ) );
				}



			}
			break;

		case CgetIdx:
			{
				int     idx;

				switch ( gnoclCget ( interp, objc, objv, G_OBJECT ( button ), fontButtonOptions, &idx ) )
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
							return gnoclCgetNotImplemented ( interp, fontButtonOptions + idx );
						}
				}
			}
	}

	return TCL_OK;
}

/**
\brief
**/
int gnoclFontButtonCmd ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{

	if ( gnoclGetCmdsAndOpts ( interp, cmds, fontButtonOptions, objv, objc ) == TCL_OK )
	{
		return TCL_OK;
	}


	int       ret;
	GtkFontButton *button;

	if ( gnoclParseOptions ( interp, objc, objv, fontButtonOptions ) != TCL_OK )
	{
		gnoclClearOptions ( fontButtonOptions );
		return TCL_ERROR;
	}

	button = GTK_FONT_BUTTON ( gtk_font_button_new( ) );

	gtk_widget_show ( GTK_WIDGET ( button ) );

	ret = gnoclSetOptions ( interp, fontButtonOptions, G_OBJECT ( button ), -1 );
	gnoclClearOptions ( fontButtonOptions );

	if ( ret != TCL_OK )
	{
		gtk_widget_destroy ( GTK_WIDGET ( button ) );
		return TCL_ERROR;
	}

	return gnoclRegisterWidget ( interp, GTK_WIDGET ( button ), fontButtonFunc );
}
