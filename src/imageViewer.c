/**
	History:
	2011-06: Begin of developement

	Default bindings
		ScrollWheel:: Zoom in and out by 10%
		Mouse button 1:: Zoom in by a factor of 2 and center coordinate under mouse
		Mouse button 2:: Pan
		Mouse button 3:: Zoom out by a factor of 2
		*F*:: Fill image in window
		*H*:: Horizontal flip
		*V*:: Vertical flip
		*1* or *N*:: Show native size
		*>* or *+* or *=*:: Zoom in
		*<* or *-*:: Zoom out

**/

/**
\page page_imageViewer gnocl::imageViewer
\htmlinclude imageViewer.html
**/

#include "gnocl.h"
#include <gtkimageview/gtkimageview.h>
//#include "./imageViewer/gtk-image-viewer.h"


/*
  "mouse-wheel-scroll"                             : Run Last
  "pixbuf-changed"                                 : Run Last
  "scroll"                                         : Run Last / Action
  "set-fitting"                                    : Run Last / Action
  "set-scroll-adjustments"                         : Run Last
  "set-zoom"                                       : Run Last / Action
  "zoom-changed"                                   : Run Last
  "zoom-in"                                        : Run Last / Action
  "zoom-out"                                       : Run Last / Action
*/

static GnoclOption imageViewerOptions[] =
{
	/* widget specific options */
	{ "-active", GNOCL_BOOL, "active" },
	{ "-onMouseScroll", GNOCL_OBJ, "", gnoclOptOnMotion},


	/* general options */
	{ "-name", GNOCL_STRING, "name" },
	{ "-visible", GNOCL_BOOL, "visible" },
	{ "-tooltip", GNOCL_OBJ, "", gnoclOptTooltip },
	{ NULL },
};



/**
\brief
**/
static int configure ( Tcl_Interp *interp, GtkWidget *imageViewer, GnoclOption options[] )
{
	return TCL_OK;
}

/**
\brief
**/
int imageViewerFunc ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{
	static const char *cmds[] =
	{
		"delete", "configure", "class", "start", "stop",
		NULL
	};

	enum cmdIdx
	{
		DeleteIdx, ConfigureIdx, ClassIdx, ParentIdx, StartIdx, StopIdx
	};

	GtkWidget *imageViewer = GTK_WIDGET ( data );

	int idx;

	if ( Tcl_GetIndexFromObj ( interp, objv[1], cmds, "command", TCL_EXACT, &idx ) != TCL_OK )
	{
		return TCL_ERROR;
	}

	switch ( idx )
	{

		case ClassIdx:
			{
				Tcl_SetObjResult ( interp, Tcl_NewStringObj ( "imageViewer", -1 ) );
			}
			break;
		case DeleteIdx:
			{
				return gnoclDelete ( interp, GTK_WIDGET ( imageViewer ), objc, objv );
			}
		case ConfigureIdx:
			{
				int ret = TCL_ERROR;

				if ( gnoclParseAndSetOptions ( interp, objc - 1, objv + 1,
											   imageViewerOptions, G_OBJECT ( imageViewer ) ) == TCL_OK )
				{
					ret = configure ( interp, imageViewer, imageViewerOptions );
				}

				gnoclClearOptions ( imageViewerOptions );

				return ret;
			}

			break;
	}

	return TCL_OK;
}

/**
\brief
**/
int gnoclImageViewerCmd ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{

	int ret;
	GtkWidget *image_view;
	//GtkScrolledWindow *scrolled;

	gchar *fname = "./desklet_or_gadget.png";

	GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file ( fname, NULL );

	if ( gnoclParseOptions ( interp, objc, objv, imageViewerOptions ) != TCL_OK )
	{
		gnoclClearOptions ( imageViewerOptions );
		return TCL_ERROR;
	}

	image_view = gtk_image_view_new();

	gtk_image_view_set_pixbuf ( GTK_IMAGE_VIEW ( image_view ), pixbuf, TRUE );

//image_viewer = gtk_image_viewer_new_from_file(fname);

	//scrolled =  GTK_SCROLLED_WINDOW ( gtk_scrolled_window_new ( NULL, NULL ) );

	//gtk_scrolled_window_set_policy ( scrolled, GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC );

	//gtk_container_add ( GTK_CONTAINER ( scrolled ), GTK_WIDGET ( image_viewer ) );

	//gtk_widget_show_all ( GTK_WIDGET ( scrolled ) );

	// Set the scroll region and zoom range
	//gtk_image_viewer_set_scroll_region(GTK_IMAGE_VIEWER(image_viewer), 5,-5,5,5);
	//gtk_image_viewer_set_zoom_range(GTK_IMAGE_VIEWER(image_viewer), -HUGE, HUGE);

	// Need to do a manual zoom fit at creation because a bug when
	// not using an image.
	// gtk_image_viewer_zoom_fit(GTK_IMAGE_VIEWER(image_viewer));



	ret = gnoclSetOptions ( interp, imageViewerOptions, G_OBJECT ( image_view ), -1 );

	gnoclClearOptions ( imageViewerOptions );


	if ( ret != TCL_OK )
	{
		gtk_widget_destroy ( GTK_WIDGET ( image_view ) );
		return TCL_ERROR;
	}

	/* TODO: if not -visible == 0 */
	gtk_widget_show ( GTK_WIDGET ( image_view ) );

	return gnoclRegisterWidget ( interp, GTK_WIDGET ( image_view ), imageViewerFunc );
}
