/*
 *    2008-10: added command, class
 */

/**
\page page_handleBox gnocl::handleBox
\htmlinclude handleBox.html
**/

#include "gnocl.h"

/* derive switches from the list of properties and signals related to this widget */
/* no need to change name, all widget handleBoxOptions are static within each code module */
static GnoclOption handleBoxOptions[] =
{
	{ "-shadow", GNOCL_OBJ, "shadow-type", gnoclOptShadow },                /* 0 */
	{ "-handlePosition", GNOCL_OBJ, "handle-position", gnoclOptPosition },  /* 1 */
	{ "-snapEdge", GNOCL_OBJ, "snap-edge", gnoclOptPosition },              /* 2 */
	{ "-child", GNOCL_OBJ, "", gnoclOptChild },                             /* 3 */
	{ "-onAttached", GNOCL_OBJ, "", gnoclOptOnChildAttached },              /* 4 */
	{ "-onDetached", GNOCL_OBJ, "", gnoclOptOnChildDetached },              /* 5 */
	{ "-tooltip", GNOCL_OBJ, "", gnoclOptTooltip },                         /* 6 */
	{ NULL }
};


/**
\brief
**/
static void removeChild ( GtkWidget *widget, gpointer data )
{
	GtkBox *box = GTK_BOX ( data );
	/* FIXME: is this correct? where to put the g_object_unref? */
	g_object_ref ( widget );
	gtk_container_remove ( GTK_CONTAINER ( box ), widget );

	if ( GTK_IS_ALIGNMENT ( widget ) )
	{
		GtkWidget *child = gtk_bin_get_child ( GTK_BIN ( widget ) );
		g_object_ref ( child );
		gtk_container_remove ( GTK_CONTAINER ( widget ), child );
		g_object_unref ( widget );
	}
}

/**
\brief
**/
static int configure ( Tcl_Interp *interp, GtkWidget *widget, GnoclOption handleBoxOptions[] )
{

	if ( 0 )
	{
		printf ( "\thandleBoxOptions %s\n", handleBoxOptions[0] );
		printf ( "\thandleBoxOptions %s\n", handleBoxOptions[3] );
		printf ( "\thandleBoxOptions %s\n", handleBoxOptions[5] );
	}

	return TCL_OK;
}

/**
\brief
**/
int handleBoxFunc ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[]  )
{
	static const char *cmds[] =
	{
		"delete", "configure", "cget",
		"class", "size",
		NULL
	};
	enum cmdIdx
	{
		DeleteIdx, ConfigureIdx, CgetIdx,
		ClassIdx, SizeIdx
	};
	int idx;

	GtkWidget *widget = GTK_WIDGET ( data );

	/* error check for formatting */

	/* Step 1) a plain widget id is not enough! */

	if ( objc < 2 )
	{
		Tcl_WrongNumArgs ( interp, 1, objv, "command" );
		return TCL_ERROR;
	}

	/* Step 2) get an idx value for the comman? */

	if ( Tcl_GetIndexFromObj ( interp, objv[1], cmds, "command", TCL_EXACT, &idx ) != TCL_OK )
	{
		return TCL_ERROR;
	}

	/* now that we have a valid command, determine what to do.. */

	switch ( idx )
	{
		case SizeIdx:
			{

			}
			break;
		case ClassIdx:
			Tcl_SetObjResult ( interp, Tcl_NewStringObj ( "handleBox", -1 ) );
			break;
		case ConfigureIdx:
			{
				int ret = TCL_ERROR;

				/* HANDLE SPECIAL CASES */
				/* check to see for reconfiguration of -child, this is not a parameter
				 * and needs some special handling.
				 * firstly, the existing child has to be removed, and a new one assigned
				 *
				 * Once the code for this special case has been developed, apply it to other simple containers,
				 * such as the toplevel gnocl::window widget.
				 */

				if ( strcmp ( &handleBoxOptions[objc], "-child" ) )
				{
					printf ( "Gotcha!\nSomething else needs to be done here...\n" );
					//return TCL_OK;
				}

				/* continue with configure as normal */
				printf ( "1) handleBox configure objc = %d  handleBoxOptions = %s\n", objc, handleBoxOptions[objc] );

				if ( gnoclParseAndSetOptions ( interp, objc - 1, objv + 1, handleBoxOptions, G_OBJECT ( widget ) ) == TCL_OK )
				{
					ret = configure ( interp, widget, handleBoxOptions );
				}

				gnoclClearOptions ( handleBoxOptions );

				return ret;
			}

			break;
		case DeleteIdx:
			return gnoclDelete ( interp, GTK_WIDGET ( widget ), objc, objv );
	}

	return TCL_OK;
}

/**
\brief
**/
static const int orientationIdx  = 0;
static const int buttonTypeIdx   = 1;
static const int childrenIdx     = 2;
static const int layoutIdx       = 3;

static const int startFrameOpts  = 7;
static const int startCommonOpts = 11;
static const int startPackOpts   = 24;
static const int paddingDiff     = 0;
static const int fillDiff        = 1;
static const int expandDiff      = 2;
static const int alignDiff       = 3;


/* some possible commands, see box.c */
/*
 *  static const char *cmds[] = {  "delete", "configure", "add", "addBegin", "addEnd", NULL
 */

/**
\brief
**/
int gnoclHandleBoxCmd ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{
	int       ret;
	GtkWidget *widget;
	GPtrArray *parray;


	/* STEP 1)  -check validity of switches */

	if ( gnoclParseOptions ( interp, objc, objv, handleBoxOptions ) != TCL_OK )
	{
		gnoclClearOptions ( handleBoxOptions );
		return TCL_ERROR;
	}

	/* STEP 2)  -create widget, configure it, then clear memory */

	widget = gtk_handle_box_new ();

	ret = gnoclSetOptions ( interp, handleBoxOptions, G_OBJECT ( widget ), -1 );

	if ( ret == TCL_OK )
	{

		ret = configure ( interp, widget, handleBoxOptions );
	}

	gnoclClearOptions ( handleBoxOptions );

	if ( ret != TCL_OK )
	{
		gtk_widget_destroy ( GTK_WIDGET ( widget ) );
		return TCL_ERROR;
	}

	/* STEP 3)  -show the widget */

	gtk_widget_show ( GTK_WIDGET ( widget ) );


	/* STEP 4)  -everything has worked, register the widget with the Tcl interpretor. */

	return gnoclRegisterWidget ( interp, GTK_WIDGET ( widget ), handleBoxFunc );

}
