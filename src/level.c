/**
\page page_level gnocl::level
\htmlinclude level.html
**/

/**
\par Modification History
\verbatim
	2013-07: added commands, options, commands
	2013-02: Begin of developement
\endverbatim
**/

#include "gnocl.h"
#include "./level/gtklevel.h"

static GnoclOption levelOptions[] =
{
	/* widget specific options */
	{ "-active", GNOCL_BOOL, "active" },

	/* general options */
	{ "-name", GNOCL_STRING, "name" },
	{ "-visible", GNOCL_BOOL, "visible" },
	{ "-tooltip", GNOCL_OBJ, "", gnoclOptTooltip },
	{ NULL },
};



/**
\brief
**/
static int configure ( Tcl_Interp *interp, GtkLevel *level, GnoclOption options[] )
{
	return TCL_OK;
}

static const char *cmds[] =
{
	"delete", "configure",
	"class", "set",
	NULL
};


/**
\brief
**/
int levelFunc ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{

	enum cmdIdx
	{
		DeleteIdx, ConfigureIdx, ClassIdx, SetIdx
	};

	GtkLevel *level = GTK_WIDGET ( data );

	int idx;

	if ( Tcl_GetIndexFromObj ( interp, objv[1], cmds, "command", TCL_EXACT, &idx ) != TCL_OK )
	{
		return TCL_ERROR;
	}

	switch ( idx )
	{
		case SetIdx:
			{
				//g_print ("%s %s\n",__FUNCTION__,cmds[idx]);
				/* get value */
				//Tcl_GetString ( objv[2] );
				gint i;
				Tcl_GetIntFromObj ( interp, objv[2], &i );
				level->sel = i;

				GtkWidget *widget = GTK_WIDGET ( level );

				GdkRegion *region;
				region = gdk_drawable_get_clip_region ( widget->window );
				gdk_window_invalidate_region ( widget->window, region, TRUE );

				//gtk_level_paint (level);

			}
			break;

		case ClassIdx:
			{
				Tcl_SetObjResult ( interp, Tcl_NewStringObj ( "level", -1 ) );
			}
			break;
		case DeleteIdx:
			{
				return gnoclDelete ( interp, GTK_WIDGET ( level ), objc, objv );
			}
		case ConfigureIdx:
			{
				int ret = TCL_ERROR;

				if ( gnoclParseAndSetOptions ( interp, objc - 1, objv + 1,
											   levelOptions, G_OBJECT ( level ) ) == TCL_OK )
				{
					ret = configure ( interp, level, levelOptions );
				}

				gnoclClearOptions ( levelOptions );

				return ret;
			}

			break;
	}

	return TCL_OK;
}

/**
\brief
**/
int gnoclLevelCmd ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{

	if ( gnoclGetCmdsAndOpts ( interp, cmds, levelOptions, objv, objc ) == TCL_OK )
	{
		return TCL_OK;
	}


	int        ret;
	GtkWidget *level;
	GtkAdjustment *adjustment;

	if ( gnoclParseOptions ( interp, objc, objv, levelOptions ) != TCL_OK )
	{
		gnoclClearOptions ( levelOptions );
		return TCL_ERROR;
	}

	adjustment = GTK_ADJUSTMENT ( gtk_adjustment_new ( 0, 0, 100, 0.01, 0.1, 0 ) );

	level = gtk_level_new ( adjustment );

	ret = gnoclSetOptions ( interp, levelOptions, G_OBJECT ( level ), -1 );

	gnoclClearOptions ( levelOptions );

	if ( ret != TCL_OK )
	{
		gtk_widget_destroy ( GTK_WIDGET ( level ) );
		return TCL_ERROR;
	}

	/* TODO: if not -visible == 0 */
	gtk_widget_show ( GTK_WIDGET ( level ) );

	return gnoclRegisterWidget ( interp, GTK_WIDGET ( level ), levelFunc );
}
