/**
\brief This file implements the file and folder chooser buttons.
**/

/**
\page page_fileChooserButton gnocl::fileChooserButton
\htmlinclude fileChooserButton.html
**/

/**
\page page_folderChooserButton gnocl::folderChooserButton
\htmlinclude folderChooserButton.html
**/

/**
 \par Modification History
 \verbatim
 *  2011/03/28	  added -currentFolder
 *  2011/03/20    completed cget
 *  2011/03/20    added extra options
 *  2008/10/08    added class
 *  2008/07/13    Begin development
 \endverbatim
**/


#include "gnocl.h"

static GnoclOption chooserButtonOptions[] =
{

	{ "-onFileSet", GNOCL_OBJ, "file-set", gnoclOptOnFileSet },

	/*-------*/
	{ "-title", GNOCL_STRING, "title" },
	{ "-widthChars", GNOCL_INT, "width-chars" },
	{ "-focusOnClick", GNOCL_BOOL, "focus-on-click" },

	{ "-dialog", GNOCL_OBJ, NULL},				/* uncertain how to handle this one 18/03/11*/

	/* GtkContainer Properties */
	{ "-borderWidth", GNOCL_OBJ, "border-width", gnoclOptPadding },

	/* GtkObject Properties */
	{ "-data", GNOCL_OBJ, "", gnoclOptData },
	{ "-hasFocus", GNOCL_BOOL, "has-focus" },
	{ "-heightGroup", GNOCL_OBJ, "h", gnoclOptSizeGroup },
	{ "-name", GNOCL_STRING, "name" },

	{ "-currentFolder", GNOCL_OBJ, "", gnoclOptCurrentFolder},

	{ "-sensitive", GNOCL_BOOL, "sensitive" },
	{ "-sizeGroup", GNOCL_OBJ, "s", gnoclOptSizeGroup },
	{ "-tooltip", GNOCL_OBJ, "", gnoclOptTooltip },
	{ "-visible", GNOCL_BOOL, "visible" },
	{ "-widthGroup", GNOCL_OBJ, "w", gnoclOptSizeGroup },

	{ NULL},
};






/**
\brief
**/
static int configure ( Tcl_Interp *interp, GtkWidget *chooserButton, GnoclOption options[] )
{
	GtkFileFilter *filter1, *filter2, *filter3;

	/* set the location to the user's home director */
	// gtk_file_chooser_set_current_folder( GTK_FILE_CHOOSER(chooserButton), g_get_home_dir() );
#ifdef DEBUG_FILECHOOSER_BUTTON
	printf ( "Element 0 contains %s \n", options[0] );
	printf ( "Element 1 contains %s \n", options[1] );
	printf ( "Element 2 contains %s \n", options[2] );
#endif
	/* NOTE: once perfected, this switch also needs to be applied to the fileChooser selector widget */

	/* this is the fileChooser, so some filters needed */
	filter1 = gtk_file_filter_new ();
	gtk_file_filter_set_name ( filter1, "All Files" );
	gtk_file_filter_add_pattern ( filter1, "*" );
	gtk_file_chooser_add_filter ( GTK_FILE_CHOOSER ( chooserButton ), filter1 );

	filter2 = gtk_file_filter_new ();
	gtk_file_filter_set_name ( filter2, "Image Files" );
	gtk_file_filter_add_pattern ( filter2, "*.png" );
	gtk_file_filter_add_pattern ( filter2, "*.jpg" );
	gtk_file_filter_add_pattern ( filter2, "*.gif" );
	gtk_file_filter_add_pattern ( filter2, "*.tif" );
	gtk_file_chooser_add_filter ( GTK_FILE_CHOOSER ( chooserButton ), filter2 );

	filter3 = gtk_file_filter_new ();
	gtk_file_filter_set_name ( filter3, "Document Files" );
	gtk_file_filter_add_pattern ( filter3, "*.jml" );
	gtk_file_filter_add_pattern ( filter3, "*.txt" );
	gtk_file_filter_add_pattern ( filter3, "*.odt" );
	gtk_file_filter_add_pattern ( filter3, "*.rtf" );
	gtk_file_chooser_add_filter ( GTK_FILE_CHOOSER ( chooserButton ), filter3 );

	return TCL_OK;
}

/**
\brief
**/
static int cget ( Tcl_Interp *interp, GtkButton *chooserButton, GnoclOption options[], int idx )
{

	g_print ( "option = %s\n", options[idx] );


	return gnoclCgetNotImplemented ( interp, options + idx );
}

/**
\brief
**/
/* a function to manage changes and events occuring to the widget during runtime */
int fileChooserButtonFunc ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{

	static const char *cmds[] =
	{
		"delete", "configure", "cget", "class", NULL
	};

	enum cmdIdx
	{
		DeleteIdx, ConfigureIdx, CgetIdx, ClassIdx
	};

	GtkButton *button = GTK_FILE_CHOOSER_BUTTON ( data );
	int idx;

	if ( Tcl_GetIndexFromObj ( interp, objv[1], cmds, "command", TCL_EXACT, &idx ) != TCL_OK )
	{
		return TCL_ERROR;
	}

	switch ( idx )
	{
		case DeleteIdx:
			{
			}
			break;
		case ConfigureIdx:
			{
				int ret = TCL_ERROR;

				if ( gnoclParseAndSetOptions ( interp, objc - 1, objv + 1, chooserButtonOptions, G_OBJECT ( button ) ) == TCL_OK )
				{
					ret = configure ( interp, button, chooserButtonOptions );
				}

				gnoclClearOptions ( chooserButtonOptions );

				return ret;

			} break;
		case CgetIdx:
			{
				int     idx;

				switch ( gnoclCget ( interp, objc, objv, button, chooserButtonOptions, &idx ) )
				{
					case GNOCL_CGET_ERROR:
						return TCL_ERROR;
					case GNOCL_CGET_HANDLED:
						return TCL_OK;
					case GNOCL_CGET_NOTHANDLED:
						return cget ( interp, button, chooserButtonOptions, idx );
				}
			} break;
		case ClassIdx:
			{
				Tcl_SetObjResult ( interp, Tcl_NewStringObj ( "fileChooseButton", -1 ) );
			} break;

	}

	return TCL_OK;
}

/**
\brief
**/
int gnoclFileChooserButtonCmd ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{
	int       ret;
	GtkWidget *chooserButton;

	/* check validity of switches */

	if ( gnoclParseOptions ( interp, objc, objv, chooserButtonOptions ) != TCL_OK )
	{
		gnoclClearOptions ( chooserButtonOptions );
		return TCL_ERROR;
	}

	/* create and show the chooserButton */
	chooserButton = gtk_file_chooser_button_new ( "Choose a file", GTK_FILE_CHOOSER_ACTION_OPEN );

	gtk_widget_show ( GTK_WIDGET ( chooserButton ) );

	/* intitalise options, and clear memory when done, return error initialisation fails */
	ret = gnoclSetOptions ( interp, chooserButtonOptions, G_OBJECT ( chooserButton ), -1 );

	if ( ret == TCL_OK )
	{
		ret = configure ( interp, chooserButton, chooserButtonOptions );
	}

	gnoclClearOptions ( chooserButtonOptions );

	if ( ret != TCL_OK )
	{
		gtk_widget_destroy ( GTK_WIDGET ( chooserButton ) );
		return TCL_ERROR;
	}

	/* monitor when the selected folder or file are changed */
//  g_signal_connect (G_OBJECT ( chooserButton ), "selection-changed", G_CALLBACK (folder_changed), NULL;


	/* register the new widget for use with the Tcl interpretor */
	return gnoclRegisterWidget ( interp, GTK_WIDGET ( chooserButton ), fileChooserButtonFunc );
}


/**
\brief
**/
int gnoclFolderChooserButtonCmd ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{
	int       ret;
	GtkWidget *chooserButton;

	/* check validity of switches */

	if ( gnoclParseOptions ( interp, objc, objv, chooserButtonOptions ) != TCL_OK )
	{
		gnoclClearOptions ( chooserButtonOptions );
		return TCL_ERROR;
	}

	/* create and show the chooserButton */
	chooserButton = gtk_file_chooser_button_new ( "Choose a folder", GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER );

	gtk_widget_show ( GTK_WIDGET ( chooserButton ) );

	/* intitalise options, and clear memory when done, return error initialisation fails */
	ret = gnoclSetOptions ( interp, chooserButtonOptions, G_OBJECT ( chooserButton ), -1 );

	if ( ret == TCL_OK )
	{
		ret = configure ( interp, chooserButton, chooserButtonOptions );
	}

	gnoclClearOptions ( chooserButtonOptions );

	if ( ret != TCL_OK )
	{
		gtk_widget_destroy ( GTK_WIDGET ( chooserButton ) );
		return TCL_ERROR;
	}

	/* monitor when the selected folder or file are changed */
//  g_signal_connect (G_OBJECT ( chooserButton ), "selection-changed", G_CALLBACK (folder_changed), NULL;


	/* register the new widget for use with the Tcl interpretor */
	return gnoclRegisterWidget ( interp, GTK_WIDGET ( chooserButton ), fileChooserButtonFunc );
}


/*****/
