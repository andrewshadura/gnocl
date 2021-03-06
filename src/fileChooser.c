/*
 * $Id: fileChooser.c,v 1.4 2005/02/22 23:16:10 baum Exp $
 *
 * This file implements the file chooser dialog
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
   2011-11: completed support for file filters, now accepts multiple filters.
   2010-07: merged contents of fileChooserDialog.c with fileChooser.c
   2010-01: deprecated -action option "openFolder" in favor of Gtk compliant "selectFolder"
   2009-12: module renamed from fileChooser,c to fileChooserDialog.c
   2004-12: start of development
*/

/**
\page page_fileChooser gnocl::fileChooser
\htmlinclude fileChooser.html

\page page_fileChooserDialog gnocl::fileChooserDialog
\htmlinclude fileChooserDialog.html
**/

#include "gnocl.h"
//#include "gnoclparams.h"


/* module function declarations */
int gnoclOptCurrentName ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret );
static int GnoclOptMiscFp ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret );

/**
\brief
**/
static int configure ( Tcl_Interp *interp, GtkWidget *widget, GnoclOption options[] )
{
	return gnoclCgetNotImplemented ( interp, options );
}

/**
\brief
**/
static int fc_cget ( Tcl_Interp *interp, GtkWidget *widget, GnoclOption options[], int idx )
{
	return gnoclCgetNotImplemented ( interp, options + idx );
}



/**
\brief
**/
void addFilterPatterns ( Tcl_Interp *interp, GtkFileFilter *filter, gchar *pattern )
{
	int argc, code, i;
	char *string;
	char **argv;

	code = Tcl_SplitList ( interp, pattern, &argc, &argv );

	for ( i = 0; i < argc; i++ )
	{
		/* process patterns */
		//gtk_file_filter_add_pattern (GtkFileFilter *filter, const gchar *pattern);
		gtk_file_filter_add_pattern ( filter, argv[i] );

	}

	Tcl_Free ( ( char * ) argv );

}

/**
\brief
**/
int gnoclOptFileFilters (	Tcl_Interp *interp,	GnoclOption *opt,	GObject *obj,	Tcl_Obj **ret )
{
#ifdef DEBUG_FILECHOOSER
	g_print ( "$s\n", __FUNCTION__ );
#endif
	int argc, code, i;
	char *string;
	char **argv;

	code = Tcl_SplitList ( interp, Tcl_GetString ( opt->val.obj ), &argc, &argv );

	GtkFileFilter *filter;

	/* read pairs */
	for ( i = 0; i < argc; i += 2 )
	{
		//filter = gnoclFileFilterFromName ( argv[i] );
		/* filter name */
		filter = gtk_file_filter_new ();
		gtk_file_filter_set_name ( filter, argv[i] );

		addFilterPatterns ( interp, filter, argv[i+1] );

		/* add filter to the dialog */
		gtk_file_chooser_add_filter ( GTK_FILE_CHOOSER ( obj ), filter );

	}

	Tcl_Free ( ( char * ) argv );

	return TCL_OK;
}

/**
\brief
**/
static void fileFilter ( GtkWidget *widget )
{
	g_print ( "%s\n", __FUNCTION__ );
	GtkFileFilter *filter;
	filter = gtk_file_filter_new ();
	gtk_file_filter_set_name ( filter, "Document Files" );
	//gtk_file_filter_add_pattern ( filter, "*.*" );
	gtk_file_filter_add_pattern ( filter, "*.jml" );
	gtk_file_filter_add_pattern ( filter, "*.txt" );
	gtk_file_filter_add_pattern ( filter, "*.odt" );
	gtk_file_filter_add_pattern ( filter, "*.rtf" );
	gtk_file_chooser_add_filter ( GTK_FILE_CHOOSER ( widget ), filter );
}

/**
\brief
**/
static int doFileSelectionChanged ( GtkFileChooser *chooser, gpointer user_data )
{
	GnoclCommandData *cs = ( GnoclCommandData * ) user_data;

	GnoclPercSubst ps[] =
	{
		{ 'f', GNOCL_STRING },  /* name of selected file */
		{ 0 }
	};

	ps[0].val.str = gtk_file_chooser_get_filename ( chooser );

	gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 );
}

/**
\brief
**/
static GtkFileChooserConfirmation doOverwriteConfirm ( GtkFileChooser *chooser, gpointer user_data )
{
	char *uri;
	uri = gtk_file_chooser_get_uri ( chooser );

	GnoclCommandData *cs = ( GnoclCommandData * ) user_data;

	GnoclPercSubst ps[] =
	{
		{ 'f', GNOCL_STRING },  /* filename */
		{ 'a', GNOCL_STRING },  /* action */
		{ 0 }
	};

	ps[0].val.str = gtk_file_chooser_get_uri ( chooser );
	ps[1].val.str = "action";
	gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 );

	/*
	if (is_uri_read_only (uri))
	    {
	      if (user_wants_to_replace_read_only_file (uri))
	        return GTK_FILE_CHOOSER_CONFIRMATION_ACCEPT_FILENAME;
	      else
	        return GTK_FILE_CHOOSER_CONFIRMATION_SELECT_AGAIN;
	    } else
	*/
	return GTK_FILE_CHOOSER_CONFIRMATION_CONFIRM; // fall back to the default dialog


	/*
	  if (is_uri_read_only (uri))
	    {
	      if (user_wants_to_replace_read_only_file (uri))
	        return GTK_FILE_CHOOSER_CONFIRMATION_ACCEPT_FILENAME;
	      else
	        return GTK_FILE_CHOOSER_CONFIRMATION_SELECT_AGAIN;
	    } else
	      return GTK_FILE_CHOOSER_CONFIRMATION_CONFIRM; // fall back to the default dialog
	*/

}


/**
\brief
**/
int gnoclOptOverwriteConfirm  ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
#ifdef DEBUG_FILECHOOSER
	g_print ( "$s\n", __FUNCTION__ );
#endif

	assert ( strcmp ( opt->optName, "-onConfirmOverwrite" ) == 0 );

	return gnoclConnectOptCmd ( interp, obj, "confirm-overwrite", G_CALLBACK ( doOverwriteConfirm ), opt, NULL, ret );
}


/**
\brief
**/
int gnoclOptFileSelectionChanged  ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
#ifdef DEBUG_FILECHOOSER
	g_print ( "$s\n", __FUNCTION__ );
#endif

	assert ( strcmp ( opt->optName, "-onSelectionChanged" ) == 0 );

	return gnoclConnectOptCmd ( interp, obj, "selection-changed", G_CALLBACK ( doFileSelectionChanged ), opt, NULL, ret );
}


static GnoclOption options[] =
{

	/* widget specific options */
	{ "-action", GNOCL_OBJ, NULL },            /* 0 */
	{ "-onUpdatePreview", GNOCL_OBJ, NULL },   /* 1 */
	{ "-getURIs", GNOCL_BOOL, NULL },          /* 2 */

	{ "-currentName", GNOCL_OBJ, "", gnoclOptCurrentName },
	{ "-currentFolder", GNOCL_OBJ, "f", GnoclOptMiscFp },
	{ "-extraWidget", GNOCL_OBJ, "extra-widget", gnoclOptWidget },
	{ "-filename", GNOCL_OBJ, "n", GnoclOptMiscFp },
	{ "-localOnly", GNOCL_BOOL, "local-only" },

// new 05/07/10
	{ "-createFolders", GNOCL_BOOL, "local-only" },
	{ "-overwriteConfirm", GNOCL_BOOL, "do-overwrite-confirmation" },
	{ "-previewLabel", GNOCL_BOOL,   "use-preview-label"},
	{ "-onConfirmOverwrite", GNOCL_OBJ, "", gnoclOptOverwriteConfirm },
	{ "-onFolderChanged", GNOCL_OBJ, "", NULL },
	{ "-onFileActivated", GNOCL_OBJ, "", NULL },
	{ "-onSelectionChanged", GNOCL_OBJ, "", gnoclOptFileSelectionChanged },
	{ "-onUpdatePreview", GNOCL_OBJ, "", NULL },

	{ "-name", GNOCL_STRING, "name" },
	{ "-onDestroy", GNOCL_OBJ, "destroy", gnoclOptCommand },
	{ "-previewWidget", GNOCL_OBJ, "preview-widget", gnoclOptWidget },
	{ "-previewWidgetActive", GNOCL_BOOL, "preview-widget-active" },
	{ "-selectMultiple", GNOCL_BOOL, "select-multiple" },
	{ "-selectFilename", GNOCL_OBJ, "s", GnoclOptMiscFp },
	{ "-showHidden", GNOCL_BOOL, "show-hidden" },
	{ "-title", GNOCL_STRING, "title" },

	{ "-fileFilters", GNOCL_OBJ, "", gnoclOptFileFilters},

	/* variation specific options */
	/* Dialog */
	/* Widget */
	{ "-visible", GNOCL_BOOL, "visible" },

	{ NULL }

};

static const int actionIdx          = 0;
static const int onUpdatePreviewIdx = 1;
static const int getURIsIdx         = 2;

/* moved to gnocl.h */
/*
typedef struct
{
    Tcl_Interp           *interp;
    char                 *name;
    int                  getURIs;
    GtkFileChooserDialog *fileDialog;
} FileSelDialogParams;
*/

/**
\brief	Create and configure filechooser dialog widget.
**/
static int GnoclOptMiscFp ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	if ( ret == NULL ) /* set value */
	{
		const char *str;
		gboolean ( *fp ) ( GtkFileChooser *, const gchar * );

		switch ( opt->propName[0] )
		{
			case 'f':
				{
					fp = gtk_file_chooser_set_current_folder;
				}
				break;
			case 'n':
				{
					fp = gtk_file_chooser_set_filename;
				}
				break;
			case 's':
				{
					fp = gtk_file_chooser_select_filename;
				}
				break;
			default: assert ( 0 );
		}

		str = Tcl_GetString ( opt->val.obj );

		if ( *str )
			( *fp ) ( GTK_FILE_CHOOSER ( obj ), str );
	}

	else /* get value */
	{
		char *str;
		char * ( *fp ) ( GtkFileChooser * );

		switch ( opt->propName[0] )
		{
			case 'f':
				{
					fp = gtk_file_chooser_get_current_folder;
				}
				break;
			case 'n':
				{
					fp = gtk_file_chooser_get_filename;
				}
				break;
			case 's':
				{
					Tcl_SetResult ( interp, "Option -selectFilename can only be set",	TCL_STATIC );
					return TCL_ERROR;
				}
				break;
			default: assert ( 0 );
		}

		str = ( *fp ) ( GTK_FILE_CHOOSER ( obj ) );

		*ret = Tcl_NewStringObj ( str, -1 );
		g_free ( str );
	}

	return TCL_OK;
}

/**
\brief
**/
int gnoclOptCurrentName ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	if ( ret == NULL ) /* set value */
	{
		const char *str = Tcl_GetString ( opt->val.obj );
		gtk_file_chooser_set_current_name ( GTK_FILE_CHOOSER ( obj ), str );
	}

	else /* get value */
	{
		char *str = gtk_file_chooser_get_filename ( GTK_FILE_CHOOSER ( obj ) );
		g_print ( "file = %s\n", str );
		*ret = Tcl_NewStringObj ( str, -1 );
		g_free ( str );
	}

	return TCL_OK;
}

/**
\brief
**/
static void destroyFunc ( GtkWidget *widget, gpointer data )
{
	FileSelDialogParams *para = ( FileSelDialogParams * ) data;

	gnoclForgetWidgetFromName ( para->name );
	Tcl_DeleteCommand ( para->interp, para->name );

	g_free ( para->name );
	g_free ( para );
}

/**
\brief
**/
static int getActionType ( Tcl_Interp *interp, Tcl_Obj *obj, GtkFileChooserAction *action )
{

	// deprecate openFolder in favour of selectFolder
	const char *txt[] = { "open", "save", "openFolder", "createFolder", "selectFolder", NULL };
	GtkFileChooserAction types[] = { GTK_FILE_CHOOSER_ACTION_OPEN,
									 GTK_FILE_CHOOSER_ACTION_SAVE,
									 GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
									 GTK_FILE_CHOOSER_ACTION_CREATE_FOLDER,
									 GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER
								   };

	int idx;

	if ( Tcl_GetIndexFromObj ( interp, obj, txt, "action", TCL_EXACT, &idx ) != TCL_OK )
	{
		return TCL_ERROR;
	}

	*action = types[idx];

	return TCL_OK;
}

/**
\brief
**/
static Tcl_Obj *getUTF8FileName ( const char *name, int *errFlag )
{
	Tcl_Obj *ret;

	*errFlag = 0;

	if ( name != NULL )
	{
		GError *error = NULL;
		gsize  bytesRead, bytesWritten;
		char   *txt = g_filename_to_utf8 ( name, -1, &bytesRead, &bytesWritten,
										   &error );

		if ( txt == NULL )
		{
			ret = Tcl_NewStringObj ( error->message, -1 );
			*errFlag = 1;
		}

		else
		{
			ret = Tcl_NewStringObj ( name, bytesWritten );
			g_free ( txt );
		}
	}

	else
		ret = Tcl_NewStringObj ( "", 0 );

	return ret;
}

/**
\brief
**/
static Tcl_Obj *getFileList ( Tcl_Interp *interp, FileSelDialogParams *para )
{
	int     errFlag = 0;
	Tcl_Obj *err;
	Tcl_Obj *ret;
	GtkFileChooser *chooser = GTK_FILE_CHOOSER ( para->fileDialog );

	if ( gtk_file_chooser_get_select_multiple ( chooser ) )
	{
		GSList *files = para->getURIs
						? gtk_file_chooser_get_uris ( chooser )
						: gtk_file_chooser_get_filenames ( chooser );
		GSList *p = files;
		ret = Tcl_NewListObj ( 0, NULL  );

		for ( ; p != NULL; p = p->next )
		{
			if ( errFlag == 0 )
			{
				err = getUTF8FileName ( p->data, &errFlag );
				Tcl_ListObjAppendElement ( interp, ret, err );
			}

			g_free ( p->data );
		}

		g_slist_free ( files );

		if ( errFlag )
			Tcl_DecrRefCount ( ret );
	}

	else
	{
		char *txt = para->getURIs
					? gtk_file_chooser_get_uri ( chooser )
					: gtk_file_chooser_get_filename ( chooser );
		ret = err = getUTF8FileName ( txt, &errFlag );
		g_free ( txt );
	}

	if ( errFlag )
	{
		Tcl_SetObjResult ( interp, err );
		return NULL;
	}

	return ret;
}

/**
\brief
**/
#if 0
static void onButtonFunc ( FileSelDialogParams *para, int isOk )
{
	if ( para->onClicked )
	{
		GnoclPercSubst ps[] =
		{
			{ 'w', GNOCL_STRING },  /* widget */
			{ 'x', GNOCL_STRING },  /* exit button */
			{ 'f', GNOCL_OBJ },     /* files */
			{ 0 }
		};

		ps[0].val.str = para->name;
		ps[1].val.str = isOk ? "OK" : "CANCEL";
		ps[2].val.obj = isOk ? getFileList ( para ) : NULL;

		gnoclPercentSubstAndEval ( para->interp, ps, para->onClicked, 1 );
	}
}

/**
\brief
**/
static void onOkFunc ( GtkWidget *widget, gpointer data )
{
	onButtonFunc ( ( FileSelDialogParams * ) data, 1 );
}

/**
\brief
**/
static void onCancelFunc ( GtkWidget *widget, gpointer data )
{
	onButtonFunc ( ( FileSelDialogParams * ) data, 0 );
}

#endif

/**
\brief
**/
static void onUpdatePreview ( GtkFileChooser *chooser, gpointer data )
{
	GnoclCommandData *cs = ( GnoclCommandData * ) data;
	FileSelDialogParams *para = ( FileSelDialogParams * ) cs->data;

	GnoclPercSubst ps[] =
	{
		{ 'w', GNOCL_STRING },  /* widget */
		{ 'f', GNOCL_STRING },  /* file name */
		{ 0 }
	};
	int ret;

	assert ( chooser == GTK_FILE_CHOOSER ( para->fileDialog ) );
	assert ( cs->interp == para->interp );

	ps[0].val.str = para->name;
	ps[1].val.str = gtk_file_chooser_get_preview_filename ( chooser );

	ret = gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 );

	g_free ( ( char * ) ps[1].val.str );

	return;
}

/**
\brief
**/
static int cget ( FileSelDialogParams *para, GnoclOption options[], int idx )
{
	/*
	   Tcl_Obj *obj = NULL;

	   if( obj != NULL )
	   {
	      Tcl_SetObjResult( interp, obj );
	      return TCL_OK;
	   }
	*/

	return gnoclCgetNotImplemented ( para->interp, options + idx );
}


static const char *fc_cmds[] = { "delete", "configure", "cget", "onClicked", "class", NULL };

/**
\brief
**/
int fileChooserFunc ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{
	printf ( "widgetFunc\n" );


	enum cmdIdx { DeleteIdx, ConfigureIdx, CgetIdx, OnClickedIdx, ClassIdx };

	GtkWidget *widget = GTK_WIDGET ( data );
	int idx;

	if ( objc < 2 )
	{
		Tcl_WrongNumArgs ( interp, 1, objv, "command" );
		return TCL_ERROR;
	}

	if ( Tcl_GetIndexFromObj ( interp, objv[1], fc_cmds, "command", TCL_EXACT, &idx ) != TCL_OK )
	{
		return TCL_ERROR;
	}

	switch ( idx )
	{

		case ClassIdx:
			{
				Tcl_SetObjResult ( interp, Tcl_NewStringObj ( "fileChooser", -1 ) );
			}
			break;
		case DeleteIdx:
			{
				return gnoclDelete ( interp, GTK_WIDGET ( widget ), objc, objv );
			}
		case ConfigureIdx:
			{
				int ret = TCL_ERROR;

				//ret = configure ( interp, widget, GnoclOptMiscFp );

				if ( 1 )
				{
					if ( gnoclParseAndSetOptions ( interp, objc - 1, objv + 1, GnoclOptMiscFp, G_OBJECT ( widget ) ) == TCL_OK )
					{
						ret = configure ( interp, widget, GnoclOptMiscFp );
					}
				}

				gnoclClearOptions ( GnoclOptMiscFp );

				return ret;
			}

			break;
		case OnClickedIdx:

			if ( objc != 2 )
			{
				Tcl_WrongNumArgs ( interp, 2, objv, NULL );
				return TCL_ERROR;
			}

			if ( GTK_WIDGET_IS_SENSITIVE ( GTK_WIDGET ( widget ) ) )
			{
				gtk_button_clicked ( widget );
			}

			break;

		case CgetIdx:
			{
				int     idx;

				switch ( gnoclCget ( interp, objc, objv, G_OBJECT ( widget ), GnoclOptMiscFp, &idx ) )
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
							return fc_cget ( interp, widget, GnoclOptMiscFp, idx );
						}
				}
			}
	}

	return TCL_OK;
}

static const char *fd_cmds[] = { "delete", "configure", "cget", "hide", "show", NULL };

/**
\brief
**/
int fileDialogFunc ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{


	enum cmdIdx { DeleteIdx, ConfigureIdx, CgetIdx, HideIdx, ShowIdx };

	FileSelDialogParams *para = ( FileSelDialogParams * ) data;
	GtkWidget *widget = GTK_WIDGET ( para->fileDialog );
	int idx;

	if ( objc < 2 )
	{
		Tcl_WrongNumArgs ( interp, 1, objv, "command" );
		return TCL_ERROR;
	}

	if ( Tcl_GetIndexFromObj ( interp, objv[1], fd_cmds, "command", TCL_EXACT, &idx ) != TCL_OK )
		return TCL_ERROR;

	switch ( idx )
	{
		case HideIdx:
			{
				gtk_widget_hide ( widget );
			}
			break;
		case ShowIdx:
			{
				gtk_widget_show_all ( widget );
			}
			break;
		case DeleteIdx:
			return gnoclDelete ( interp, widget, objc, objv );

		case ConfigureIdx:

			if ( gnoclParseOptions ( interp, objc - 1, objv + 1, options )
					== TCL_OK )
			{
				return TCL_OK;
			}

			return TCL_ERROR;

		case CgetIdx:
			{
				int     idx;

				switch ( gnoclCget ( interp, objc, objv, G_OBJECT ( widget ),
									 options, &idx ) )
				{
					case GNOCL_CGET_ERROR:
						return TCL_ERROR;
					case GNOCL_CGET_HANDLED:
						return TCL_OK;
					case GNOCL_CGET_NOTHANDLED:
						return cget ( para, options, idx );
				}
			}
	}

	return TCL_OK;
}

/**
\brief
**/
int gnoclFileChooserDialogCmd ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{

	if ( gnoclGetCmdsAndOpts ( interp, fd_cmds, options, objv, objc ) == TCL_OK )
	{
		return TCL_OK;
	}

	FileSelDialogParams *para = NULL;
	int ret = TCL_ERROR;
	GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_OPEN;
	const char *okTxt;
	gint res;

	if ( gnoclParseOptions ( interp, objc, objv, options )  != TCL_OK )
	{
		goto cleanExit;
	}

	para = g_new ( FileSelDialogParams, 1 );

	para->interp = interp;

	para->getURIs = 0;

	if ( options[getURIsIdx].status == GNOCL_STATUS_CHANGED )
	{
		para->getURIs = options[getURIsIdx].val.b;
	}

	if ( options[actionIdx].status == GNOCL_STATUS_CHANGED )
	{
		if ( getActionType ( interp, options[actionIdx].val.obj, &action ) != TCL_OK )
		{
			goto cleanExit;
		}
	}

	switch ( action )
	{
		case GTK_FILE_CHOOSER_ACTION_SAVE:
			okTxt = GTK_STOCK_SAVE;
			break;
		default:
			okTxt = GTK_STOCK_OPEN;
			break;
	}

	if ( para->getURIs )
	{
		para->fileDialog = GTK_FILE_CHOOSER_DIALOG (
							   gtk_file_chooser_dialog_new_with_backend ( "", NULL, action, "gnome-vfs",
									   GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, okTxt, GTK_RESPONSE_ACCEPT, NULL ) );
	}

	else
	{
		para->fileDialog = GTK_FILE_CHOOSER_DIALOG (
							   gtk_file_chooser_dialog_new ( "", NULL, action,
									   GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, okTxt, GTK_RESPONSE_ACCEPT, NULL ) );
	}

	para->getURIs = 0;

	if ( gnoclSetOptions ( interp, options, G_OBJECT ( para->fileDialog ),
						   -1 ) != TCL_OK )
		goto cleanExit2;

	if ( gnoclConnectOptCmd ( interp,
							  G_OBJECT ( para->fileDialog ), "update-preview",
							  G_CALLBACK ( onUpdatePreview ), options + onUpdatePreviewIdx,
							  para, NULL ) != TCL_OK )
		goto cleanExit2;

	g_signal_connect ( G_OBJECT ( para->fileDialog ), "destroy",
					   G_CALLBACK ( destroyFunc ), para );

	para->name = gnoclGetAutoWidgetId();

	gnoclMemNameAndWidget ( para->name, GTK_WIDGET ( para->fileDialog ) );

	Tcl_CreateObjCommand ( interp, para->name, fileDialogFunc, para, NULL );

	res = gtk_dialog_run ( GTK_DIALOG ( para->fileDialog ) );

	gtk_widget_hide ( GTK_WIDGET ( para->fileDialog ) );

	if ( res == GTK_RESPONSE_ACCEPT )
	{
		Tcl_Obj *obj = getFileList ( interp, para );

		if ( obj == NULL )
			goto cleanExit2;

		Tcl_SetObjResult ( interp, obj );
	}

	ret = TCL_OK;

cleanExit2:
	gtk_widget_destroy ( GTK_WIDGET ( para->fileDialog ) );

cleanExit:
	gnoclClearOptions ( options );

	return ret;
}

/*---------------------------------------------------------------------*/


/**
\brief
**/
int gnoclFileChooserCmd ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{

	if ( gnoclGetCmdsAndOpts ( interp, fc_cmds, options, objv, objc ) == TCL_OK )
	{
		return TCL_OK;
	}


	int            ret = TCL_OK;
	GtkWidget      *widget;

	if ( 0 )
	{
		if ( gnoclParseOptions ( interp, objc, objv, GnoclOptMiscFp ) != TCL_OK )
		{
			gnoclClearOptions ( GnoclOptMiscFp );
			return TCL_ERROR;
		}
	}

	/*
	typedef enum
	{
	  GTK_FILE_CHOOSER_ACTION_OPEN,
	  GTK_FILE_CHOOSER_ACTION_SAVE,
	  GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
	  GTK_FILE_CHOOSER_ACTION_CREATE_FOLDER
	} GtkFileChooserAction;
	*/

	widget = gtk_file_chooser_widget_new ( GTK_FILE_CHOOSER_ACTION_SAVE ) ;


	fileFilter ( widget );

	gtk_widget_show ( GTK_WIDGET ( widget ) );


	if ( 0 )
	{
		ret = gnoclSetOptions ( interp, GnoclOptMiscFp, G_OBJECT ( widget ), -1 );

		if ( ret == TCL_OK )
		{
			ret = configure ( interp, G_OBJECT ( widget ), GnoclOptMiscFp );
		}

		gnoclClearOptions ( GnoclOptMiscFp );

		/* STEP 3)  -show the widget */

		if ( ret != TCL_OK )
		{
			gtk_widget_destroy ( GTK_WIDGET ( widget ) );
			return TCL_ERROR;
		}
	}

	/* STEP 4)  -everything has worked, register the widget with the Tcl interpretor. */
	return gnoclRegisterWidget ( interp, GTK_WIDGET ( widget ), fileChooserFunc );
}
