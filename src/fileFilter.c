/** fileFilter.c
\brief     Implement FileChooser file filtering.
**/

/**
\page page_fileFilter gnocl::fileFilter
\htmlinclude fileFilter.html
**/

/**
 \par Modification History
 \verbatim
 2011-02: Begin developement
 \endverbatim
**/

#include "gnocl.h"
#include "gnocl_logo.h"
#include "gnocl_pointer.h"
#include <string.h>
#include <assert.h>

/* needs to be public to be accessed by gnocl::inventory */
static GHashTable *name2fileFilterList;
static const char idPrefix[] = "::gnocl::_FF";


static const int nameIdx  = 0;
static const int patternIdx  = 1;

static int configure (  Tcl_Interp *interp, GtkFileFilter *fileFilter,  GnoclOption options[] )
{
	g_print ( "%s\n", __FUNCTION__ );

	if ( 0 )
	{
		if ( options[nameIdx].status == GNOCL_STATUS_CHANGED )
		{

			g_print ( "name = %s\n", Tcl_GetString ( options[nameIdx].val.obj ) );

			gtk_file_filter_set_name ( fileFilter, Tcl_GetString ( options[nameIdx].val.obj ) );
		}



		if ( options[patternIdx].status == GNOCL_STATUS_CHANGED )
		{
			g_print ( "pattern = %s\n", Tcl_GetString ( options[patternIdx].val.obj ) );
		}
	}

	return TCL_OK;

}

/**
\brief
**/
static int gnoclOptSetName ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	g_print ( "%s name = %s\n", __FUNCTION__, Tcl_GetString ( opt->val.obj ) );
	gtk_file_filter_set_name ( obj, Tcl_GetString ( opt->val.obj ) );

	return TCL_OK;
}

/**
\brief
\note	Only one pattern can be set at at time, parse the list and add singly
**/
static int gnoclOptSetPattern ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	g_print ( "%s pattern = = %s\n", __FUNCTION__, Tcl_GetString ( opt->val.obj ) );


	char * pch;
	printf ( "Splitting string \"%s\" into tokens:\n", Tcl_GetString ( opt->val.obj ) );
	pch = strtok ( Tcl_GetString ( opt->val.obj ), " " );

	while ( pch != NULL )
	{
		printf ( "%s\n", pch );
		gtk_file_filter_add_pattern  ( obj, pch );
		pch = strtok ( NULL, " " );
	}


	return TCL_OK;
}

/**
\brief
**/
static GnoclOption fileFilterOptions[] =
{
	{ "-name", GNOCL_OBJ, "", gnoclOptSetName },
	{ "-pattern", GNOCL_OBJ, "", gnoclOptSetPattern },
	{ NULL }
};


/**
\brief
**/
GtkFileFilter *gnoclFileFilterFromName ( const char * id, Tcl_Interp * interp )
{

	g_print ( "%s filter = %s\n", __FUNCTION__, id );

	int n;
	GtkFileFilter *fileFilter = NULL;


	if ( strncmp ( id, idPrefix, sizeof ( idPrefix ) - 1 ) == 0
			&& ( n = atoi ( id + sizeof ( idPrefix ) - 1 ) ) > 0 )
	{
		fileFilter = g_hash_table_lookup ( name2fileFilterList, GINT_TO_POINTER ( n ) );
	}

	if ( fileFilter == NULL && interp != NULL )
	{
		Tcl_AppendResult ( interp, "Unknown fileFilter \"", id, "\".", ( char * ) NULL );
	}

	return fileFilter;
}

/**
\brief
**/
int fileFilterFunc ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{
	g_print ( "%s\n", __FUNCTION__ );
	return TCL_OK;
}

/**
\brief
**/
char *gnoclGetAutoFileFilterId ( void )
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
int gnoclMemNameAndFileFilter ( const char * path,  GtkFileFilter *filter )
{
	g_print ( "%s\n", __FUNCTION__ );
	int n ;

	n = atoi ( path + sizeof ( idPrefix ) - 1 );

	assert ( n > 0 );
	assert ( g_hash_table_lookup ( name2fileFilterList, GINT_TO_POINTER ( n ) ) == NULL );
	assert ( strncmp ( path, idPrefix, sizeof ( idPrefix ) - 1 ) == 0 );

	/* memorize the name of the widget in the widget */
	g_object_set_data ( G_OBJECT ( filter ), "gnocl::name", ( char * ) path );
	g_hash_table_insert ( name2fileFilterList, GINT_TO_POINTER ( n ), filter );

	return 0;
}

/**
\brief
**/
int gnoclRegisterFileFilter ( Tcl_Interp * interp, GtkFileFilter *filter, Tcl_ObjCmdProc * proc )
{
	g_print ( "%s\n", __FUNCTION__ );
	const char *name;

	name = gnoclGetAutoFileFilterId();

	gnoclMemNameAndFileFilter ( name, filter );

	if ( proc != NULL )
	{

		Tcl_CreateObjCommand ( interp, ( char * ) name, proc, filter, NULL );
	}

	Tcl_SetObjResult ( interp, Tcl_NewStringObj ( name, -1 ) );

	return TCL_OK;
}

/**
\brief
**/
int gnoclFileFilterCmd ( ClientData data, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[] )
{

	g_print ( "%s\n", __FUNCTION__ );

	int ret;
	GtkFileFilter *fileFilter;

	/* create new hash table if one does not exist */
	if ( name2fileFilterList == NULL )
	{
		name2fileFilterList = g_hash_table_new ( g_direct_hash, g_direct_equal );
	}

	g_print ( "%s 1\n", __FUNCTION__ );

	if ( gnoclParseOptions ( interp, objc, objv, fileFilterOptions ) != TCL_OK )
	{
		gnoclClearOptions ( fileFilterOptions );
		return TCL_ERROR;
	}

	fileFilter = gtk_file_filter_new ();

	g_print ( "%s 2\n", __FUNCTION__ );
	/* this is not a widget with properties, so this function will fail */
	ret = gnoclSetOptions ( interp, fileFilterOptions, G_OBJECT ( fileFilter ), -1 );

	g_print ( "%s 3\n", __FUNCTION__ );

	configure ( interp, fileFilter, fileFilterOptions );


	g_print ( "%s 4\n", __FUNCTION__ );


	//gtk_file_filter_set_name ( fileFilter,"Document Files" );
	//gtk_file_filter_add_pattern ( filter, "*.*" );
	gtk_file_filter_add_pattern ( fileFilter, "*.jml" );
	gtk_file_filter_add_pattern ( fileFilter, "*.txt" );
	gtk_file_filter_add_pattern ( fileFilter, "*.odt" );
	gtk_file_filter_add_pattern ( fileFilter, "*.rtf" );
	//gtk_file_chooser_add_filter ( GTK_FILE_CHOOSER ( widget ), filter );

	g_print ( "%s 5\n", __FUNCTION__ );

	return gnoclRegisterFileFilter ( interp, fileFilter , fileFilterFunc );

}
