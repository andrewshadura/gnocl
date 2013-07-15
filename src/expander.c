/*
 * $Id: expander.c,v 1.2 2005/02/22 23:16:10 baum Exp $
 *
 * This file implements the expander widget
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
   2013-04: added options -onActivate
   2009-12: added options -spacing -expanderSize -underline
   2008-10: added command, class
   2005-01: Begin of developement
 */

/**
\page page_expander gnocl::expander
\htmlinclude expander.html
**/

#include "gnocl.h"

static int gnoclOptOnActivate ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret );


static GnoclOption expanderOptions[] =
{
	{ "-child", GNOCL_OBJ, "", gnoclOptChild },
	{ "-expand", GNOCL_BOOL, "expanded" },
	{ "-label", GNOCL_OBJ, "label", gnoclOptLabelFull },
	{ "-onDestroy", GNOCL_OBJ, "destroy", gnoclOptCommand },
	{ "-onActivate", GNOCL_OBJ, "activate", gnoclOptOnActivate },
	{ "-heightRequest", GNOCL_OBJ, "", gnoclOptHeightRequest},
	{ "-visible", GNOCL_BOOL, "visible" },
	{ "-spacing", GNOCL_INT, "spacing" },
	{ "-expanderSize", GNOCL_INT, "expander-size"},
	{ "-underline", GNOCL_BOOL, "use-underline" },
	{ "-useMarkup", GNOCL_BOOL, "use-markup" },
	{ NULL },
};


/**
\brief	callback handler for GTK_EXPANDER widgets
**/
static void doOnActivate   ( GtkExpander *expander, gpointer user_data )
{
#ifdef DEBUG_EXPANDER
	g_print ( "%s\n", __FUNCTION__ );
#endif
	GnoclCommandData *cs = ( GnoclCommandData * ) user_data;

	GnoclPercSubst ps[] =
	{
		{ 'w', GNOCL_STRING },  /* widget */
		{ 'g', GNOCL_STRING },  /* glade name */
		{ 'e', GNOCL_BOOL},     /* expanded */
		{ 'd', GNOCL_STRING },  /* data */
		{ 0 }
	};

	ps[0].val.str = gnoclGetNameFromWidget ( expander );
	ps[1].val.str = gtk_widget_get_name ( expander );
	ps[2].val.i	  = gtk_expander_get_expanded ( expander );
	ps[3].val.str = g_object_get_data ( expander, "gnocl::data" );

	gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 );

}

/**
\brief		Click on the expander up/down arrow
\author     William J Giddings
\date       09/Apr/13
**/
static int gnoclOptOnActivate ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	return gnoclConnectOptCmd ( interp, obj, "activate", G_CALLBACK ( doOnActivate ), opt, NULL, ret );
}



/**
\brief
**/
static int setExpanderSize ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{

#ifdef DEBUG_EXPANDER
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
static int setExpanderSpacing ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
#ifdef DEBUG_EXPANDER
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
static int configure ( Tcl_Interp *interp, GtkExpander *expander, GnoclOption options[] )
{
	return TCL_OK;
}


const char *cmds[] =
{
	"delete", "configure", "cget",
	"class",
	NULL
};


/**
\brief
**/
int expanderFunc ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{

	enum cmdIdx { DeleteIdx, ConfigureIdx, CgetIdx, ClassIdx };
	int idx;
	GtkExpander *expander = GTK_EXPANDER ( data );

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
				Tcl_SetObjResult ( interp, Tcl_NewStringObj ( "expander", -1 ) );
			}
			break;
		case DeleteIdx:
			{
				return gnoclDelete ( interp, GTK_WIDGET ( expander ), objc, objv );
			}
		case ConfigureIdx:
			{
				int ret = TCL_ERROR;

				if ( gnoclParseAndSetOptions ( interp, objc - 1, objv + 1, expanderOptions, G_OBJECT ( expander ) ) == TCL_OK )
				{
					ret = configure ( interp, expander, expanderOptions );
				}

				gnoclClearOptions ( expanderOptions );

				return ret;
			}

			break;
		case CgetIdx:
			{
				int idx;

				switch ( gnoclCget ( interp, objc, objv, G_OBJECT ( expander ), expanderOptions, &idx ) )
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
							assert ( 0 );
						}
				}

				assert ( 0 );
			}
	}

	return TCL_OK;
}

/**
\brief
**/
int gnoclExpanderCmd ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{
	if ( gnoclGetCmdsAndOpts ( interp, cmds, expanderOptions, objv, objc ) == TCL_OK )
	{
		return TCL_OK;
	}


	int ret;
	GtkExpander *expander;

	if ( gnoclParseOptions ( interp, objc, objv, expanderOptions ) != TCL_OK )
	{
		gnoclClearOptions ( expanderOptions );
		return TCL_ERROR;
	}

	expander = GTK_EXPANDER ( gtk_expander_new ( "" ) );

	gtk_widget_show ( GTK_WIDGET ( expander ) );

	ret = gnoclSetOptions ( interp, expanderOptions, G_OBJECT ( expander ), -1 );

	if ( ret == TCL_OK )
		ret = configure ( interp, expander, expanderOptions );

	gnoclClearOptions ( expanderOptions );

	if ( ret != TCL_OK )
	{
		gtk_widget_destroy ( GTK_WIDGET ( expander ) );
		return TCL_ERROR;
	}

	return gnoclRegisterWidget ( interp, GTK_WIDGET ( expander ), expanderFunc );
}

