/**
\brief	Sample example for loading, reading and writing a configuration file with GKeyFile
\todo	Probelms with <kf-id> clear command. The current way of deleting files relies upon
		a data association with g-objects. Perhaps the creation of a para file? The same
		problem lies in deleting a pixbuf.
**/

/**
\page page_keyfile gnocl::keyFile
\htmlinclude keyfile.html
**/

#include "gnocl.h"


/* module function prototypes */
int gnoclForgetKeyFileFromName ( KeyFileParams * para  );
char *gnoclGetAutoKeyFileId ( void );
GKeyFile *gnoclGetKeyFileFromName ( const char * id, Tcl_Interp * interp );
const char *gnoclGetNameFromKeyFile (  GKeyFile *keyFile );
int gnoclKeyFileCmd ( ClientData data, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[] );
int gnoclMemNameAndKeyFile ( const char * path, GKeyFile *keyFile );
int gnoclRegisterKeyFile ( Tcl_Interp * interp, GKeyFile *keyFile, Tcl_ObjCmdProc * proc );
int keyFileFunc ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] );

/* hash table declarations */
static GHashTable *name2KeyFileList;
static const char idPrefix[] = "::gnocl::_KF";

static GnoclOption options[] =
{
	{ NULL, GNOCL_STRING, NULL },
	{ NULL },
};

static const char *cmds[] =
{
	"new", "load", "write", NULL
};


/**
\brief
\note       Gdk Documentation for the implemented features.
            http://library.gnome.org/devel/glib/stable/glib-Key-value-file-parser.html
**/
int gnoclKeyFileCmd ( ClientData data, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[] )
{

	if ( gnoclGetCmdsAndOpts ( interp, cmds, options, objv, objc ) == TCL_OK )
	{
		return TCL_OK;
	}




#ifdef DEBUG_KEYFILE
	listParameters ( objc, objv, "gnoclKeyFileCmd" );
#endif


	KeyFileParams *para;

	//GKeyFile *  keyFile = NULL;
	GError   *  p_error = NULL;
	gchar *iniFileName = "app.ini";

	enum optIdx
	{
		NewIdx, LoadIdx, WriteIdx
	};
	int idx;



	if ( objc < 2 )
	{
		Tcl_WrongNumArgs ( interp, 1, objv, "option" );
		return TCL_ERROR;
	}


	para = g_new ( KeyFileParams, 1 );
	para->interp = interp;


	if ( Tcl_GetIndexFromObj ( interp, objv[1], cmds, "option", TCL_EXACT, &idx ) != TCL_OK )
	{
		return TCL_ERROR;
	}

	switch ( idx )
	{
		case NewIdx:
			{

				gchar str[256];

				para->keyFile = g_key_file_new();

			}
			break;
		case LoadIdx:
			{
#ifdef DEBUG_KEYFILE
				g_print ( "\tLoadIdx\n" );
#endif

				/* create a new keyfile object */
				para->keyFile = g_key_file_new();

				if ( objc == 3 )
				{
					g_key_file_load_from_file ( para->keyFile, Tcl_GetString ( objv[2] ), G_KEY_FILE_KEEP_COMMENTS, NULL );
				}

				else
				{
					gchar str[256];
					sprintf ( str, "%s.ini", gnoclGetAppName ( interp ) );
					g_key_file_load_from_file ( para->keyFile, str, G_KEY_FILE_KEEP_COMMENTS, NULL );
				}
			}
			break;
		default: {}

	}

	if ( para->keyFile != NULL )
	{
		//return gnoclRegisterKeyFile ( interp, para->keyFile, keyFileFunc );
	}

	para->name = gnoclGetAutoKeyFileId();

	gnoclMemNameAndKeyFile ( para->name, para->keyFile );

	Tcl_CreateObjCommand ( interp, para->name, keyFileFunc, para, NULL );

	Tcl_SetObjResult ( interp, Tcl_NewStringObj ( para->name, -1 ) );


	return TCL_OK;
}


/**
\brief
**/
int keyFileFunc ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{

#ifdef DEBUG_KEYFILE
	listParameters ( objc, objv, "keyFileFunc" );
#endif

	KeyFileParams *para = ( KeyFileParams * ) data;


	/* values take from the sample app */
	GError   *  p_error     = NULL;
	gchar    *  s_data;
	gsize       size;
	gboolean    b_ret;

	gchar *str = NULL;

	static const char *cmds[] =
	{
		"get", "set", "remove",
		"write", "has", "clear",
		"class", "delete", "free",
		NULL
	};

	enum cmdIdx
	{
		GetIdx, SetIdx, RemoveIdx,
		WriteIdx, HasIdx, ClearIdx,
		ClassIdx, DeleteIdx, FreeIdx
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
		case ClassIdx:
			{
				Tcl_SetResult ( interp, "keyfile", TCL_STATIC );
				return TCL_OK;
			}
		case ClearIdx:
			{
#ifdef DEBUG_KEYFILE
				g_print ( "\tclear\n" );
#endif
				/* remove all groups and keys from the keyfFile */
				g_key_file_free ( para->keyFile );
				para->keyFile =  g_key_file_new();

				return TCL_OK;
			}
			break;
		case FreeIdx:
		case DeleteIdx:
			{
#ifdef DEBUG_KEYFILE
				g_print ( "\tdelete\n" );
#endif
				/* delete the keyfile object */
				gnoclForgetKeyFileFromName ( para );

				return TCL_OK;
			}
			break;
		case HasIdx:
			{

				if ( objc < 3 )
				{
					Tcl_SetResult ( interp, "ERROR: Item to be checked not specified.", TCL_STATIC );
					return TCL_ERROR;
				}

				gchar *group = NULL;
				gchar *key = NULL;

				gchar res[2];
				int i, optIdx;

				static const char *getSwitches[] =
				{
					"-group", "-key",
					NULL
				};

				enum getSwitchesIdx
				{
					GroupIdx, KeyIdx
				};

				/* determine what switches have been set */
				for ( i = 2; i < objc; i += 2 )
				{

					getIdx ( getSwitches, Tcl_GetString ( objv[i] ), &optIdx );

					switch ( optIdx )
					{
						case GroupIdx:
							{
								group = Tcl_GetString ( objv[i+1] );
							}
							break;
						case KeyIdx:
							{
								key = Tcl_GetString ( objv[i+1] );
							}
							break;
						default:
							{
								Tcl_SetResult ( interp, "ERROR: Invalid switch.", TCL_STATIC );
								return TCL_ERROR;
							}
					}

				}

				if ( group != NULL && key == NULL )
				{

					i = g_key_file_has_group ( para->keyFile, group );

				}

				if ( group != NULL && key != NULL )
				{

					i = g_key_file_has_key ( para->keyFile, group, key, NULL );

				}


				sprintf ( res, "%d", i );
				Tcl_SetResult ( interp, res, TCL_STATIC );
				return TCL_OK;
			}
			break;
		case WriteIdx:
			{
				gsize size;
				gchar *data;
				gint ret;

				data = g_key_file_to_data ( para->keyFile, &size, NULL );

				if ( objc == 3 )
				{
					ret = g_file_set_contents (  Tcl_GetString ( objv[2] ), data, size, NULL );

				}

				else
				{
					gchar str[256];
					sprintf ( str, "%s.ini", gnoclGetAppName ( interp ) );

					ret = g_file_set_contents ( str, data, size, NULL );

				}

				return TCL_OK;
				//Tcl_SetResult ( interp, data, TCL_STATIC );

			}
			break;
		case GetIdx:
			{
#ifdef DEBUG_KEYFILE
				g_print ( "\tGetIdx\n" );
#endif

				int i;
				gchar *group = NULL;
				gchar *key = NULL;
				gchar *locale = NULL;
				gchar *str = NULL;
				gint list = 0;

				int optIdx;


				if ( objc == 2 )
				{
					gchar *data;
					gsize size;

					data = g_key_file_to_data ( para->keyFile, &size, NULL );
					Tcl_SetResult ( interp, data, TCL_STATIC );
					return TCL_OK;
				}





				static const char *getSwitches[] =
				{
					"-group", "-key", "-locale", "-list",
					NULL
				};

				enum getSwitchesIdx
				{
					GroupIdx, KeyIdx, LocaleIdx, ListIdx
				};

				/* determine what switches have been set */
				for ( i = 3; i < objc; i += 2 )
				{
					{

						getIdx ( getSwitches, Tcl_GetString ( objv[i] ), &optIdx );

						switch ( optIdx )
						{
							case GroupIdx:
								{
									group = Tcl_GetString ( objv[i+1] );
								}
								break;
							case KeyIdx:
								{
									key = Tcl_GetString ( objv[i+1] );
								}
								break;
							case LocaleIdx:
								{
									locale = Tcl_GetString ( objv[i+1] );
								}
								break;
							case ListIdx:
								{
									Tcl_GetIntFromObj ( NULL, objv[i+1], &list );
								}
								break;
							default:
								{
									Tcl_SetResult ( interp, "ERROR: Wrong switch.", TCL_STATIC );
									return TCL_ERROR;
								}
						}

					}
				}

				/* effetively, only value and comment used */
				static const char *getOpts[] =
				{
					"double", "integer", "boolean",
					"string", "value", "comment",
					"groups", "keys", "startgroup",
					NULL
				};

				enum getOptsIdx
				{
					DoubleIdx, IntegerIdx, BooleanIdx,
					StringIdx, ValueIdx, CommentIdx,
					GroupsIdx, KeysIdx, StartGroupIdx
				};

				getIdx ( getOpts, Tcl_GetString ( objv[2] ), &optIdx );

				switch ( optIdx )
				{

					case StartGroupIdx:
						{
							str = g_key_file_get_start_group ( para->keyFile );
						}
						break;
						/*----- WORKING HERE -----*/
					case KeysIdx:
						{

							gint len = -1;
							gchar **keylist;
							gchar **key;
							Tcl_Obj *resList;

							/* extract group name */
							if ( strcmp ( Tcl_GetString ( objv[3] ), "-group" ) != 0 && objc != 5 )
							{
								Tcl_SetResult ( interp, "ERROR: Option -group not specified.", TCL_STATIC );
								return TCL_ERROR;
							}

							keylist =  g_key_file_get_keys ( para->keyFile, Tcl_GetString ( objv[4] ), &len, NULL );

							if ( len == -1 )
							{
								Tcl_SetResult ( interp, "ERROR: Group not found.", TCL_STATIC );
								return TCL_ERROR;
							}

							key = keylist;

							resList = Tcl_NewListObj ( 0, NULL );

							while ( *key != NULL )
							{
								Tcl_ListObjAppendElement ( interp, resList, Tcl_NewStringObj ( *key, -1 ) );
								key++;
							}

							g_strfreev ( keylist );

							Tcl_SetObjResult ( interp, resList );

							return TCL_OK;
						}
						break;
					case GroupsIdx:
						{

							gint len;

							gchar **grouplist;
							gchar **group;
							Tcl_Obj *resList;


							grouplist =  g_key_file_get_groups ( para->keyFile, &len );
							group = grouplist;

							resList = Tcl_NewListObj ( 0, NULL );

							while ( *group != NULL )
							{
								Tcl_ListObjAppendElement ( interp, resList, Tcl_NewStringObj ( *group, -1 ) );
								group++;
							}

							g_strfreev ( grouplist );

							Tcl_SetObjResult ( interp, resList );

							return TCL_OK;
						}
						break;
						/* on the TCL side, everything is a string, i.e. a value */
					case DoubleIdx:
						{
							gchar tmp[32];
							sprintf ( tmp, "%f", g_key_file_get_double ( para->keyFile, group, key, NULL ) );
							str = tmp;
						}
						break;
					case IntegerIdx:
						{
							gchar tmp[32];
							sprintf ( tmp, "%d", g_key_file_get_integer ( para->keyFile, group, key, NULL ) );
							str = tmp;
						}
						break;
					case BooleanIdx:
						{
							gchar tmp[32];
							sprintf ( tmp, "%d", g_key_file_get_boolean ( para->keyFile, group, key, NULL ) );
							str = tmp;
						}
						break;
					case StringIdx:
						{
							if ( locale )
							{
								str = g_key_file_get_locale_string ( para->keyFile, group, key, locale, NULL );
							}

							else
							{
								str = g_key_file_get_string ( para->keyFile, group, key, NULL );
							}
						}
						break;
					case ValueIdx:
						{
							str = g_key_file_get_value ( para->keyFile, group, key, NULL );
						}
						break;
					case CommentIdx:
						{
							str = g_key_file_get_comment ( para->keyFile, group, key, NULL );
						}

						break;
					default:
						{
							Tcl_SetResult ( interp, ">>>>>>>ERROR: Wrong option.", TCL_STATIC );
							return TCL_ERROR;
						}
				}


				if ( str == NULL )
				{
					Tcl_SetResult ( interp, "ERROR: Item or comment requested not found.", TCL_STATIC );
					return TCL_ERROR;
				}

				Tcl_SetResult ( interp, str, TCL_STATIC );

			}

			break;
		case SetIdx:
			{
#ifdef DEBUG_KEYFILE
				g_print ( "\tSetIdx\n" );
#endif

				int i;
				gchar *group = NULL;
				gchar *key = NULL;
				gchar *locale = NULL;
				gchar *str = NULL;
				gchar *value = NULL;
				gint list = 0;

				int optIdx;

				static const char *getSwitches[] =
				{
					"-group", "-key", "-locale",
					"-list", "-value",
					NULL
				};

				enum getSwitchesIdx
				{
					GroupIdx, KeyIdx, LocaleIdx,
					ListIdx, ValIdx
				};

				/* determine what switches have been set */
				for ( i = 3; i < objc; i += 2 )
				{
					{

						getIdx ( getSwitches, Tcl_GetString ( objv[i] ), &optIdx );

						switch ( optIdx )
						{
							case GroupIdx:
								{
									group = Tcl_GetString ( objv[i+1] );
								}
								break;
							case KeyIdx:
								{
									key = Tcl_GetString ( objv[i+1] );
								}
								break;
							case LocaleIdx:
								{
									locale = Tcl_GetString ( objv[i+1] );
								}
								break;
							case ListIdx:
								{
									Tcl_GetIntFromObj ( NULL, objv[i+1], &list );
								}
								break;
							case ValIdx:
								{
									value = Tcl_GetString ( objv[i+1] );
								}
								break;
							default:
								{
									/* commented out because it causes problems with set separator */
									//Tcl_SetResult ( interp, "ERROR: Invalid switch.", TCL_STATIC );
									//return TCL_ERROR;
								}
						}

					}
				}

				/* effetively, only value and comment used */
				static const char *getOpts[] =
				{
					"double", "integer", "boolean",
					"string", "value", "comment",
					"separator",
					NULL
				};

				enum getOptsIdx
				{
					DoubleIdx, IntegerIdx, BooleanIdx,
					StringIdx, ValueIdx, CommentIdx,
					SeparatorIdx
				};

				getIdx ( getOpts, Tcl_GetString ( objv[2] ), &optIdx );

				switch ( optIdx )
				{
					case SeparatorIdx:
						{
							if ( objc < 4 )
							{
								Tcl_SetResult ( interp, "ERROR: Separator not specified.", TCL_STATIC );
								return TCL_ERROR;
							}

							g_key_file_set_list_separator ( para->keyFile, Tcl_GetString ( objv[3] ) );
							return TCL_OK;
						}
						break;
						/* on the TCL side, everything is a string, i.e. a value */
					case DoubleIdx:
						{
							/* change the value of key to get right data type */

							g_key_file_set_double ( para->keyFile, group, key, atof ( value ) ) ;

						}
						break;
					case BooleanIdx:
					case IntegerIdx:
						{
							/* change the value of key to get right data type */
							/* string to int */
							g_key_file_set_integer ( para->keyFile, group, key, atoi ( value ) ) ;
						}
						break;
					case StringIdx:
						{
							if ( locale )
							{
								g_key_file_set_locale_string ( para->keyFile, group, key, locale, value );
							}

							else
							{
								g_key_file_set_string ( para->keyFile, group, key, value );
							}
						}
						break;
					case ValueIdx:
						{
							g_key_file_set_value ( para->keyFile, group, key, value );
						}
						break;
					case CommentIdx:
						{
							g_key_file_set_comment ( para->keyFile, group, key, value, NULL );
						}

						break;
					default:
						{
							Tcl_SetResult ( interp, "ERROR: Wrong option.", TCL_STATIC );
							return TCL_ERROR;
						}
				}


			}

			break;
		case RemoveIdx:
			{

#ifdef DEBUG_KEYFILE
				g_print ( "\tRemoveIdx\n" );
#endif

				gchar *group = NULL;
				gchar *key = NULL;

				static const char *getOpts[] =
				{
					"group", "comment",
					NULL
				};

				enum getOptsIdx
				{
					GroupIdx, CommentIdx
				};

				int optIdx;
				int ret;

				getIdx ( getOpts, Tcl_GetString ( objv[2] ), &optIdx );

				group = Tcl_GetString ( objv[3] );

				switch ( optIdx )
				{
						/* FORMAT: id remove group <group> -key <key> */
					case GroupIdx:
						{

							/* get optional key */
							if ( objc == 6 && strcmp ( Tcl_GetString ( objv[4] ), "-key" ) == 0 )
							{
								/* remove key */
								ret = g_key_file_remove_key ( para->keyFile, group, Tcl_GetString ( objv[5] ), NULL );
							}

							else
							{
								/* remove group */
								ret = g_key_file_remove_group ( para->keyFile, group, NULL );
							}

						}
						break;
						/* FORMAT: id remove comment <group> -key <key> */
					case CommentIdx:
						{

							/* get optional key */
							if ( objc == 6 && strcmp ( Tcl_GetString ( objv[4] ), "-key" ) == 0 )
							{
								/* remove group and key comment*/
								ret = g_key_file_remove_comment ( para->keyFile, group, Tcl_GetString ( objv[5] ), NULL );
							}

							else
							{
								/* remove group comment*/
								ret = g_key_file_remove_comment ( para->keyFile, group, NULL, NULL );
							}
						}
						break;
					default:
						{}
				}
			}
			break;
		default: {}
	}

	return TCL_OK;
}



/**
\brief
**/
char *gnoclGetAutoKeyFileId ( void )
{
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
\note	Name of widget stored in para structure.
**/
int gnoclMemNameAndKeyFile ( const char * path, GKeyFile * keyFile )
{
	int n ;

	n = atoi ( path + sizeof ( idPrefix ) - 1 );

	assert ( n > 0 );
	//assert ( g_hash_table_lookup ( name2KeyFileList, GINT_TO_POINTER ( n ) ) == NULL );

	g_hash_table_lookup ( name2KeyFileList, GINT_TO_POINTER ( n ) );

	assert ( strncmp ( path, idPrefix, sizeof ( idPrefix ) - 1 ) == 0 );

	g_hash_table_insert ( name2KeyFileList, GINT_TO_POINTER ( n ), keyFile );

	return 0;
}

/**
\brief
**/
int gnoclForgetKeyFileFromName ( KeyFileParams * para  )
{
#ifdef DEBUG_KEYFILE
	g_print ( "gnoclForgetKeyFileFromName %s\n", para->name );
#endif

	int n;

	g_key_file_free ( para->keyFile );

	n = atoi ( para->name + sizeof ( idPrefix ) - 1 );

	assert ( strncmp ( para->name, idPrefix, sizeof ( idPrefix ) - 1 ) == 0 );
	assert ( n > 0 );

	g_hash_table_remove ( name2KeyFileList, GINT_TO_POINTER ( n ) );
	Tcl_DeleteCommand ( para->interp, para->name );

	g_free ( para );

	return TCL_OK;
}


/**
\brief
**/
const char *gnoclGetNameFromKeyFile (  GKeyFile * keyFile )
{
	const char *name = g_object_get_data ( G_OBJECT ( keyFile ), "gnocl::name" );

	return name;
}

/**
\brief
**/
GKeyFile *gnoclGetKeyFileFromName ( const char * id, Tcl_Interp * interp )
{
	GKeyFile *keyFile = NULL;
	int       n;

	if ( strncmp ( id, idPrefix, sizeof ( idPrefix ) - 1 ) == 0
			&& ( n = atoi ( id + sizeof ( idPrefix ) - 1 ) ) > 0 )
	{
		keyFile = g_hash_table_lookup ( name2KeyFileList, GINT_TO_POINTER ( n ) );
	}

	if ( keyFile == NULL && interp != NULL )
	{
		Tcl_AppendResult ( interp, "Unknown keyfile \"", id, "\".", ( char * ) NULL );
	}

	return keyFile;
}

/**
\brief
**/
int gnoclRegisterKeyFile ( Tcl_Interp * interp, GKeyFile * keyFile, Tcl_ObjCmdProc * proc )
{

	const char *name;

	name = gnoclGetAutoKeyFileId();

	gnoclMemNameAndKeyFile ( name, keyFile ); //<--- problems here

	//g_signal_connect_after ( G_OBJECT ( pixbuf ), "destroy", G_CALLBACK ( simpleDestroyFunc ), interp );

	if ( proc != NULL )
	{

		Tcl_CreateObjCommand ( interp, ( char * ) name, proc, keyFile, NULL );
	}

	Tcl_SetObjResult ( interp, Tcl_NewStringObj ( name, -1 ) );

	return TCL_OK;
}
