/** recentMgr.c
\author    William J Giddings email: wjgiddings@blueyonder.co.uk
\date      31-DEC-2011
\version   0.9.95
	History:
	2011-12: New module added.
**/

#include "gnocl.h"

static GHashTable *name2recentMgrList;
static const char idPrefix[] = "::gnocl::_RMGR";

/**
\brief
\note      "char *" and not "const char *" because of a not very strict
            handling of "const char *" in Tcl e.g. Tcl_CreateObjCommand
*/
char *gnoclGetAutoRecentMgrId ( void )
{
	g_print ( "%s\n", __FUNCTION__ );

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
int gnoclMemNameAndRecentManager ( const char * path,  GtkRecentManager *manager )
{
	g_print ( "%s\n", __FUNCTION__ );

	int n ;

	n = atoi ( path + sizeof ( idPrefix ) - 1 );

	assert ( n > 0 );
	assert ( g_hash_table_lookup ( name2recentMgrList, GINT_TO_POINTER ( n ) ) == NULL );
	assert ( strncmp ( path, idPrefix, sizeof ( idPrefix ) - 1 ) == 0 );

	/* memorize the name of the widget in the widget */
	g_object_set_data ( G_OBJECT ( manager ), "gnocl::name", ( char * ) path );
	g_hash_table_insert ( name2recentMgrList, GINT_TO_POINTER ( n ), manager );

	return 0;
}


/**
\brief	Register structure for pixbuf and cairo context
**/
int gnoclRegisterRecentMgr ( Tcl_Interp * interp, GtkRecentManager *manager, Tcl_ObjCmdProc * proc )
{
	g_print ( "%s\n", __FUNCTION__ );

	const char *name;

	name = gnoclGetAutoRecentMgrId();

	gnoclMemNameAndRecentManager ( name, manager ); //<--- problems here

	//g_signal_connect_after ( G_OBJECT ( pixbuf ), "destroy", G_CALLBACK ( simpleDestroyFunc ), interp );

	if ( proc != NULL )
	{

		Tcl_CreateObjCommand ( interp, ( char * ) name, proc, manager, NULL );
	}

	Tcl_SetObjResult ( interp, Tcl_NewStringObj ( name, -1 ) );

	return TCL_OK;
}


/**
\brief
**/
static int configure ( Tcl_Interp *interp, GtkPlug *plug, GnoclOption options[] )
{

	return TCL_OK;
}

/**
\brief
**/
static int recentMgrFunc ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{

	GtkRecentManager *manager = GTK_RECENT_MANAGER ( data );
	GtkRecentData *fileData;
	gchar *uri, *fileName;

	static gchar *groups[2] =
	{
		"testapp",
		NULL
	};

	g_print ( "%s\n", __FUNCTION__ );

	const char *cmds[] =
	{
		"add", "remove", "purge",
		"delete", "configure",
		NULL
	};

	enum cmdIdx
	{
		AddIdx, RemoveIdx, PurgeIdx,
		DeleteIdx, ConfigureIdx,
	};

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
		case AddIdx:
			{

				fileName = Tcl_GetString ( objv[2] );

				/* add a recently used file */
				fileData = g_slice_new ( GtkRecentData );
				fileData->display_name = NULL;
				fileData->description = NULL;
				fileData->mime_type = "text/plain";
				fileData->app_name = ( gchar* ) g_get_application_name ();
				fileData->app_exec = g_strjoin ( " ", g_get_prgname (), "%u", NULL );
				fileData->groups = groups;
				fileData->is_private = FALSE;
				uri = g_filename_to_uri ( fileName, NULL, NULL );

				/* add resource to the data manager */
				gtk_recent_manager_add_full ( manager, uri, fileData );

				g_free ( uri );
				g_free ( fileData->app_exec );
				g_slice_free ( GtkRecentData, fileData );

				return TCL_OK;

			}
		case RemoveIdx:
			{
				fileName = Tcl_GetString ( objv[2] );
				uri = g_filename_to_uri ( fileName, NULL, NULL );
				gtk_recent_manager_remove_item ( manager, uri, NULL );
			}
			break;
		case PurgeIdx:
			{
				g_print ( "Not yet implemented.\n" );
			}
			break;
		case DeleteIdx:
			{
				//return gnoclDelete ( interp, GTK_WIDGET ( plug ), objc, objv );
			}
			break;
		case ConfigureIdx:
			{
				/*
				int ret = TCL_ERROR;

				if ( recentMgrOptions[socketIDIdx].status == GNOCL_STATUS_CHANGED )
				{
					Tcl_SetResult ( interp, "Option -socketID cannot be changed after creation", TCL_STATIC );
					return TCL_ERROR;
				}

				if ( gnoclParseAndSetOptions ( interp, objc - 1, objv + 1,
											   recentMgrOptions, G_OBJECT ( plug ) ) == TCL_OK )
				{
					ret = configure ( interp, plug, recentMgrOptions );
				}

				gnoclClearOptions ( recentMgrOptions );

				return ret;
				*/
			}

			break;
		default:
			{
			}
	}

	return TCL_OK;
}

/**
\brief
**/
int gnoclRecentManagerCmd ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{
	g_print ( "%s\n", __FUNCTION__ );


	int ret;

	/* create new hash table if one does not exist */
	if ( name2recentMgrList == NULL )
	{
		name2recentMgrList = g_hash_table_new ( g_direct_hash, g_direct_equal );
	}


	static const char *cmd[] =
	{
		"getDefault",
		NULL
	};

	enum cmdIdx
	{
		GetDefaultIdx,
	};

	int cIdx;

	if ( Tcl_GetIndexFromObj ( interp, objv[1], cmd, "option", TCL_EXACT, &cIdx ) != TCL_OK )
	{
		return TCL_ERROR;
	}

	switch ( cIdx )
	{
		case GetDefaultIdx:
			{
				GtkRecentManager *manager;
				manager = gtk_recent_manager_get_default ();
				return gnoclRegisterRecentMgr ( interp, manager, recentMgrFunc );
			}
			break;
		default:
			{
			}
	}



	/*
		if ( gnoclParseOptions ( interp, objc, objv, recentMgrOptions ) != TCL_OK )
		{
			gnoclClearOptions ( recentMgrOptions );
			return TCL_ERROR;
		}
	*/

	//gnoclClearOptions ( recentMgrOptions );

	if ( ret != TCL_OK )
	{
		//gtk_widget_destroy ( GTK_WIDGET ( plug ) );
		return TCL_ERROR;
	}

	//return gnoclRegisterWidget ( interp, GTK_WIDGET ( plug ), plugFunc );

	return TCL_OK;
}


