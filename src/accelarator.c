
/**
\page page_accelarator gnocl::accelarator
\htmlinclude accelarator.html
**/

/**
\par Modification History
\verbatim
	2013-07:	added commands, options, commands
\endverbatim
**/

#include "gnocl.h"
#include "gnoclparams.h"


AccGrpParams *gnoclGetAccGrpFromName ( const char *id, Tcl_Interp *interp );
const char *gnoclGetNameFromAccGrp ( GtkAccelGroup *group );
int gnoclRegisterAccGrp ( Tcl_Interp *interp, GtkAccelGroup *group, Tcl_ObjCmdProc *proc );

/* needs to be public to be accessed by gnocl::inventory */
static GHashTable *name2accGrpList;
static const char idPrefix[] = "::gnocl::_AGRP";

static const int windowIdx = 0;
static const int widgetIdx = 1;

static GnoclOption accGrpOptions[] =
{
	{ "-window", GNOCL_STRING, NULL },            /* 0 */
	{ "-widget", GNOCL_STRING, NULL },            /* 1 */
	{ NULL }
};

/**
\brief
\note      "char *" and not "const char *" because of a not very strict
            handling of "const char *" in Tcl e.g. Tcl_CreateObjCommand
*/
char *gnoclGetAccGrpId ( void )
{
#ifdef DEBUG_ACCELGROUP
	g_print ( "%s\n", __FUNCTION__ );
#endif
	static int no = 0;
	/*
	static char buffer[30];
	*/

	char *buffer = g_new ( char, sizeof ( idPrefix ) + 15 );
	strcpy ( buffer, idPrefix );

	/* with namespace, since the Id is also the widget command */
	sprintf ( buffer + sizeof ( idPrefix ) - 1, "%d", ++no );

	return buffer;
}


/**
\brief
**/
int gnoclMemNameAndAccGrp ( const char *path, GtkAccelGroup *group )
{
#ifdef DEBUG_ACCELGROUP
	g_print ( "%s\n", __FUNCTION__ );
#endif
	int n ;

	n = atoi ( path + sizeof ( idPrefix ) - 1 );

	assert ( n > 0 );
	assert ( g_hash_table_lookup ( name2accGrpList, GINT_TO_POINTER ( n ) ) == NULL );
	assert ( strncmp ( path, idPrefix, sizeof ( idPrefix ) - 1 ) == 0 );

	/* memorize the name of the widget in the widget */
	g_object_set_data ( G_OBJECT ( group ), "gnocl::name", ( char * ) path );
	g_hash_table_insert ( name2accGrpList, GINT_TO_POINTER ( n ), group );

	return 0;
}


/**
\brief
**/
int gnoclRegisterAccGrp ( Tcl_Interp *interp, GtkAccelGroup *group, Tcl_ObjCmdProc *proc )
{
#ifdef DEBUG_ACCELGROUP
	g_print ( "%s\n", __FUNCTION__ );
#endif

	const char *name;

	name = gnoclGetAccGrpId();

	//gnoclMemNameAndAccGrp ( name, group ); //<--- problems here

	//g_signal_connect_after ( G_OBJECT ( pixbuf ), "destroy", G_CALLBACK ( simpleDestroyFunc ), interp );

	if ( proc != NULL )
	{

		Tcl_CreateObjCommand ( interp, ( char * ) name, proc, group, NULL );
	}

	Tcl_SetObjResult ( interp, Tcl_NewStringObj ( name, -1 ) );

	return TCL_OK;
}

/**
\brief      Returns the widget name associated with pointer
**/
const char *gnoclGetNameFromAccGrp ( GtkAccelGroup *group )
{
#ifdef DEBUG_ACCELGROUP
	g_print ( "%s\n", __FUNCTION__ );
#endif
	const char *name = g_object_get_data ( group , "gnocl::name" );

	return name;
}

/**
\brief	Returns pointer to the accGrp
**/

AccGrpParams *gnoclGetAccGrpFromName ( const char * id, Tcl_Interp * interp )
{
#ifdef DEBUG_ACCELGROUP
	g_print ( "%s\n", __FUNCTION__ );
#endif

	AccGrpParams *para = NULL;

	int n;

	if ( strncmp ( id, idPrefix, sizeof ( idPrefix ) - 1 ) == 0
			&& ( n = atoi ( id + sizeof ( idPrefix ) - 1 ) ) > 0 )
	{
		para = g_hash_table_lookup ( name2accGrpList, GINT_TO_POINTER ( n ) );
	}

	if ( para == NULL && interp != NULL )
	{
		Tcl_AppendResult ( interp, "Unknown accelarator group \"", id, "\".", ( char * ) NULL );
	}

	return para;
}

/**
\brief
**/
void gnoclGetAccGrpList ( GList **list )
{
#ifdef DEBUG_ACCELGROUP
	g_print ( "%s\n", __FUNCTION__ );
#endif

	g_hash_table_foreach ( name2accGrpList, hash_to_list, list );
}



/**
\brief
**/
static int configure ( Tcl_Interp *interp, AccGrpParams *para, GnoclOption options[] )
{
#ifdef DEBUG_ACCELGROUP
	g_print ( "%s\n", __FUNCTION__ );
#endif


	if ( options[windowIdx].status == GNOCL_STATUS_CHANGED )
	{
		char *str = options[windowIdx].val.str;
		g_print ( "-window = %s\n", str );

		GtkWidget *widget = gnoclGetWidgetFromName ( options[windowIdx].val.str, interp );
		gtk_window_add_accel_group ( GTK_WINDOW ( widget ), para->group );

	}

	if ( options[widgetIdx].status == GNOCL_STATUS_CHANGED )
	{
		char *str = options[widgetIdx].val.str;
		g_print ( "-widget = %s\n", str );

		GtkWidget *widget = gnoclGetWidgetFromName ( options[widgetIdx].val.str, interp );
		//gtk_widget_add_accelarator( GTK_WIDGET (widget), "activate", para->group,GDK_P, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

	}


	return TCL_OK;
}

/**
\brief
**/
static int cget ( Tcl_Interp *interp, GtkWidget *accelarator, GnoclOption options[], int idx )
{
#ifdef DEBUG_ACCELGROUP
	g_print ( "%s\n", __FUNCTION__ );
#endif

	return gnoclCgetNotImplemented ( interp, options + idx );
}

/**
\brief
**/
int accelaratorFunc ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{

#ifdef DEBUG_ACCELGROUP
	g_print ( "%s\n", __FUNCTION__ );

#endif


	static const char *cmds[] =
	{
		"delete", "configure", "cget", "class", "options", "commands", NULL
	};

	enum cmdIdx
	{
		DeleteIdx, ConfigureIdx, CgetIdx, ClassIdx, OptionsIdx, CommandsIdx
	};

	GtkAccelGroup *accelarator = GTK_ACCEL_GROUP ( data );
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
				gnoclGetOptions ( interp, accGrpOptions );
			}
			break;
		case ClassIdx:
			Tcl_SetObjResult ( interp, Tcl_NewStringObj ( "accelarator", -1 ) );
			break;

		case DeleteIdx:
			return gnoclDelete ( interp, GTK_WIDGET ( accelarator ), objc, objv );

		case ConfigureIdx:
			{
				int ret = TCL_ERROR;

				if ( gnoclParseAndSetOptions ( interp, objc - 1, objv + 1, accGrpOptions, G_OBJECT ( accelarator ) ) == TCL_OK )
				{
					ret = configure ( interp, accelarator, accGrpOptions );
				}

				gnoclClearOptions ( accGrpOptions );

				return ret;
			}

			break;


		case CgetIdx:
			{
				int     idx;

				switch ( gnoclCget ( interp, objc, objv, G_OBJECT ( accelarator ), accGrpOptions, &idx ) )
				{
					case GNOCL_CGET_ERROR:
						return TCL_ERROR;
					case GNOCL_CGET_HANDLED:
						return TCL_OK;
					case GNOCL_CGET_NOTHANDLED:
						return cget ( interp, accelarator, accGrpOptions, idx );
				}
			}
	}

	return TCL_OK;
}

/**
\brief
**/
int gnoclAcceleratorCmd ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{

#ifdef DEBUG_ACCELERATOR
	g_print ( "gnoclAcceleratorCmd\n" );
	gint _i;

	for ( _i = 0; _i < objc; _i++ )
	{
		g_print ( "\targ %d = %s\n", _i,  Tcl_GetString ( objv[_i] ) );
	}

#endif

	if ( gnoclParseOptions ( interp, objc, objv, accGrpOptions ) != TCL_OK )
	{
		gnoclClearOptions ( accGrpOptions );
		return TCL_ERROR;
	}


	int ret;

	AccGrpParams *para;
	para = g_new ( AccGrpParams, 1 );

	// create new hash table if one does not exist
	if ( name2accGrpList == NULL )
	{
		name2accGrpList = g_hash_table_new ( g_direct_hash, g_direct_equal );
	}

	para->group = gtk_accel_group_new();

	ret = gnoclSetOptions ( interp, accGrpOptions, G_OBJECT ( para->group ), -1 );

	if ( ret == TCL_OK )
	{
		ret = configure ( interp, para, accGrpOptions );
	}

	gnoclClearOptions ( accGrpOptions );




	return gnoclRegisterAccGrp ( interp, para, accelaratorFunc );

}


