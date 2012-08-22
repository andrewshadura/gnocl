/**image.c
\brief		Implement binding to the GtkImage widget.
\history	2011-06-11	Implemented changes to support PixbufParams
**/


/*
* $Id: image.c,v 1.9 2005/08/16 20:57:45 baum Exp $
 *
* This file implements the image widget
 *
* Copyright (c) 2001 - 2005 Peter G. Baum  http://www.dr-baum.net
 *
* See the file "license.terms" for information on usage and redistribution
* of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

/**
\page page_image gnocl::image
\htmlinclude image.html
**/

#include "gnocl.h"
#include "gnoclparams.h"
#include <string.h>
#include <assert.h>

static const int imageIdx = 0;
static const int stockSizeIdx  = 1;
static const int sizeIdx = 2;
static const int rotateIdx = 3;

GnoclOption imageOptions[] =
{
	{ "-image", GNOCL_OBJ, NULL },
	{ "-stockSize", GNOCL_OBJ, NULL },
	{ "-size", GNOCL_OBJ, NULL },
	{ "-rotate", GNOCL_OBJ, NULL },
	{ "-align", GNOCL_OBJ, "?align", gnoclOptBothAlign },
	{ "-xPad", GNOCL_OBJ, "xpad", gnoclOptPadding },
	{ "-yPad", GNOCL_OBJ, "ypad", gnoclOptPadding },
	{ "-name", GNOCL_STRING, "name" },
	{ "-visible", GNOCL_BOOL, "visible" },
	{ "-sensitive", GNOCL_BOOL, "sensitive" },
	{ "-tooltip", GNOCL_OBJ, "", gnoclOptTooltip },
	{ NULL }
};

/**
\brief
**/
int getIconSize ( Tcl_Interp *interp, Tcl_Obj *obj, GtkIconSize *size )
{
	const char *txt[] = { "menu", "smallToolBar",
						  "largeToolBar", "button", "dnd", "dialog", NULL
						};
	GtkIconSize modes[] = { GTK_ICON_SIZE_MENU, GTK_ICON_SIZE_SMALL_TOOLBAR,
							GTK_ICON_SIZE_LARGE_TOOLBAR, GTK_ICON_SIZE_BUTTON,
							GTK_ICON_SIZE_DND, GTK_ICON_SIZE_DIALOG
						  };

	int idx;

	if ( Tcl_GetIndexFromObj ( interp, obj, txt, "icon size",
							   TCL_EXACT, &idx ) != TCL_OK )
		return TCL_ERROR;

	*size = modes[idx];

	return TCL_OK;
}

/**
\brief
**/
static int configure ( Tcl_Interp *interp, GtkImage *image, GnoclOption options[] )
{

	/*
	   TODO:
	   GdkPixbuf*  gdk_pixbuf_new_from_data        (const guchar *data,
	                                             GDK_COLORSPACE_RGB,
	                                             gboolean has_alpha,
	                                             int bits_per_sample,
	                                             int width,
	                                             int height,
	                                             int rowstride,
	                                             GdkPixbufDestroyNotify destroy_fn,
	                                             gpointer destroy_fn_data);
	         %m25x10x8
	         %i-RGB-25x10x10-
	         %i-RGBA-25x10x16

	GdkPixbuf*  gdk_pixbuf_new_from_xpm_data    (const char **data);


	*/
	if ( options[imageIdx].status == GNOCL_STATUS_CHANGED )
	{
		GnoclStringType type = gnoclGetStringType ( options[imageIdx].val.obj );

		switch ( type & ( GNOCL_STR_FILE | GNOCL_STR_STOCK | GNOCL_STR_BUFFER ) )
		{
			case GNOCL_STR_FILE:
				{
					GError *error = NULL;
					char *txt = gnoclGetString ( options[imageIdx].val.obj );
#ifdef DEBUG_PIXBUF
					g_printf ( "loading from file = %s\n",  txt );
#endif
					GdkPixbufAnimation *ani = gdk_pixbuf_animation_new_from_file ( txt, &error );

					if ( ani == NULL )
					{
						Tcl_SetResult ( interp, error->message, TCL_VOLATILE );
						g_error_free ( error );
						return TCL_ERROR;
					}

					if ( gdk_pixbuf_animation_is_static_image ( ani ) )
					{
						GdkPixbuf *pix = gdk_pixbuf_animation_get_static_image ( ani );
						gtk_image_set_from_pixbuf ( image, pix );
					}

					else
						gtk_image_set_from_animation ( image, ani );

					g_object_unref ( ani );
				}

				break;
			case GNOCL_STR_STOCK:
				{
					GtkIconSize size = GTK_ICON_SIZE_BUTTON;
					GtkStockItem item;

					if ( gnoclGetStockItem ( options[imageIdx].val.obj, interp, &item ) != TCL_OK )
					{
						return TCL_ERROR;
					}

					if ( options[stockSizeIdx].status == GNOCL_STATUS_CHANGED )
					{
						if ( getIconSize ( interp, options[stockSizeIdx].val.obj, &size ) != TCL_OK )
						{
							return TCL_ERROR;
						}
					}

					else if ( gtk_image_get_storage_type ( image ) == GTK_IMAGE_STOCK )
					{
						gtk_image_get_stock ( image, NULL, &size );
					}

					gtk_image_set_from_stock ( image, item.stock_id, size );
				}

				break;
			case GNOCL_STR_BUFFER:
				{
					char *txt = gnoclGetString ( options[imageIdx].val.obj );
#ifdef DEBUG_PIXBUF
					g_printf ( "loading from pixbuf = %s\n",  txt );
#endif
					//GdkPixbuf *pixbuf;
					//pixbuf = gnoclGetPixBufFromName ( txt, interp );

					PixbufParams *para = gnoclGetPixBufFromName ( txt, interp );

					if ( para->pixbuf == NULL )
					{
						Tcl_SetResult ( interp, "Pixbuf does not exist.", TCL_STATIC );
						return TCL_ERROR;
					}

					gtk_image_set_from_pixbuf ( image, GDK_PIXBUF ( para->pixbuf ) );

				}
				break;
			default:
				Tcl_AppendResult ( interp, "Unknown type for \"",
								   Tcl_GetString ( options[imageIdx].val.obj ),
								   "\" must be of type FILE (%/) or STOCK (%#)", NULL );
				return TCL_ERROR;
		}
	}

	else if ( options[stockSizeIdx].status == GNOCL_STATUS_CHANGED )
	{
		char        *id;
		GtkIconSize size;

		if ( gtk_image_get_storage_type ( image ) != GTK_IMAGE_STOCK )
		{
			Tcl_SetResult ( interp, "Size can only be changed for stock images.", TCL_STATIC );
			return TCL_ERROR;
		}

		gtk_image_get_stock ( image, &id, &size );

		if ( getIconSize ( interp, options[stockSizeIdx].val.obj, &size )
				!= TCL_OK )
			return TCL_ERROR;

		gtk_image_set_from_stock ( image, id, size );
	}

	if ( options[sizeIdx].status == GNOCL_STATUS_CHANGED )
	{
		GdkPixbuf *src, *dest;
		int       width, height;
		g_print ( "%s 1\n", __FUNCTION__ );

		if ( gtk_image_get_storage_type ( image ) != GTK_IMAGE_PIXBUF )
		{
			Tcl_SetResult ( interp, "Only pixbuf images can be sized.", TCL_STATIC );
			return TCL_ERROR;
		}

		g_print ( "%s 2\n", __FUNCTION__ );

		if ( gnoclGet2Int ( interp, options[sizeIdx].val.obj, &width, &height ) != TCL_OK )
		{
			return TCL_ERROR;
		}

		g_print ( "%s 3\n", __FUNCTION__ );

		if ( width <= 0  || height <= 0 )
		{
			Tcl_SetResult ( interp, "Size must be greater zero.", TCL_STATIC );
			return TCL_ERROR;
		}

		g_print ( "%s 4\n", __FUNCTION__ );
		src = gtk_image_get_pixbuf ( image );
		g_print ( "%s 5\n", __FUNCTION__ );
		dest = gdk_pixbuf_scale_simple ( src, width, height, GDK_INTERP_BILINEAR );
		g_print ( "%s 6\n", __FUNCTION__ );

		if ( dest == NULL )
		{
			Tcl_SetResult ( interp, "Error in scaling. Not enough memory?",	TCL_STATIC );
			return TCL_ERROR;
		}

		g_print ( "%s 7\n", __FUNCTION__ );
		gtk_image_set_from_pixbuf ( image, dest );
		g_print ( "%s 8\n", __FUNCTION__ );
		g_object_unref ( dest );
	}

	return TCL_OK;
}

/**
\brief
**/
int imageFunc ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj* const objv[] )
{

#ifdef DEBUG_IMAGE
	g_printf ( "imageFunc\n" );
	gint _i;

	for ( _i = 0; _i < objc; _i++ )
	{
		g_printf ( "\targ %d = %s\n", _i,  Tcl_GetString ( objv[_i] ) );
	}

#endif

	static const char *cmds[] = { "turn", "flip", "delete", "configure", "class", NULL };
	enum cmdIdx { TurnIdx, FlipIdx, DeleteIdx, ConfigureIdx, ClassIdx };
	int idx;
	GtkImage *image = ( GtkImage* ) data;

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
		case FlipIdx:
			{
				GdkPixbuf *pixbuf;
				pixbuf = gtk_image_get_pixbuf ( image );

				if ( objc == 2 )
				{
					/* horizontal flip by default */
					pixbuf = gdk_pixbuf_flip ( pixbuf, TRUE );
					gtk_image_set_from_pixbuf ( image, pixbuf );
					break;
				}

				/* handle more complex arguments */
				if ( !strcmp ( Tcl_GetString ( objv[2] ) , "-orientation" ) )
				{

					if ( !strcmp ( Tcl_GetString ( objv[3] ) , "horizontal" ) )
					{
						pixbuf = gdk_pixbuf_flip ( pixbuf, TRUE );
						goto flipDone;
					}

					if ( !strcmp ( Tcl_GetString ( objv[3] ), "vertical" ) )
					{
						pixbuf = gdk_pixbuf_flip ( pixbuf, FALSE );
						goto flipDone;
					}

					/* must be something else, must be an error */
					Tcl_SetResult ( interp, "Invalid option,\n\tMust be -orientation horizontal | vertical", TCL_STATIC );
					return TCL_ERROR;
				}

flipDone:
				gtk_image_set_from_pixbuf ( image, pixbuf );
			}
			break;
		case TurnIdx:
			{
				/*
				typedef enum {
				    GDK_PIXBUF_ROTATE_NONE             =   0,
				    GDK_PIXBUF_ROTATE_COUNTERCLOCKWISE =  90,
				    GDK_PIXBUF_ROTATE_UPSIDEDOWN       = 180,
				    GDK_PIXBUF_ROTATE_CLOCKWISE        = 270
				} GdkPixbufRotation;
				*/

				static const char *turns[] =
				{
					"0", "90", "180", "270", "-90",
					"none", "counterclockwise", "upsideDown", "clockwise",
					"anticlockwise",
					NULL
				};

				enum turnsIdx
				{
					ZeroIdx, NinetyIdx, OneEightyIdx, TwoSeventyIdx, MinusNinetyIdx,
					NoneIdx, CounterClockwiseIdx, UpsideDownIdx, ClockwiseIdx,
					AntiClockwiseIdx
				};

				gint TurnIdx;
				gdouble angle;
				angle = 90;

				if ( Tcl_GetIndexFromObj ( interp, objv[2], turns, "command", TCL_EXACT, &TurnIdx ) != TCL_OK )
				{
					{
						Tcl_SetResult ( interp, "Invalid angle specified,", TCL_STATIC );
						return TCL_ERROR;
					}
				}


				switch ( TurnIdx )
				{
					case ZeroIdx:
					case NoneIdx:
						{
							angle = 90;
						} break;
					case NinetyIdx:
					case AntiClockwiseIdx:
					case CounterClockwiseIdx:
						{
							angle = 90;
						} break;
					case OneEightyIdx:
					case UpsideDownIdx:
						{
							angle = 180;
						} break;
					case MinusNinetyIdx:
					case TwoSeventyIdx:
					case ClockwiseIdx:
						{
							angle = 270;
						} break;
					default:
						{
							Tcl_SetResult ( interp, "Invalid angle specified,", TCL_STATIC );
							return TCL_ERROR;
						}
				}

				GdkPixbuf *pixbuf;
				pixbuf = gtk_image_get_pixbuf ( image );
				pixbuf = gdk_pixbuf_rotate_simple ( pixbuf, angle );

				gtk_image_set_from_pixbuf ( image, pixbuf );

			}

			break;

		case ClassIdx:
			{
				Tcl_SetObjResult ( interp, Tcl_NewStringObj ( "image", -1 ) );
			}
			break;
		case DeleteIdx:
			{
				return gnoclDelete ( interp, GTK_WIDGET ( image ), objc, objv );
			}
		case ConfigureIdx:
			{
				int ret = TCL_ERROR;

				if ( gnoclParseAndSetOptions ( interp, objc - 1, objv + 1, imageOptions, G_OBJECT ( image ) ) == TCL_OK )
				{
					ret = configure ( interp, image, imageOptions );
				}

				gnoclClearOptions ( imageOptions );

				return ret;
			}

			break;
	}

	return TCL_OK;
}

/**
\brief
**/
int gnoclImageCmd ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj* const objv[] )
{

#ifdef DEBUG_IMAGE
	g_printf ( "gnoclImageCmd\n" );
	gint _i;

	for ( _i = 0; _i < objc; _i++ )
	{
		g_printf ( "\targ %d = %s\n", _i,  Tcl_GetString ( objv[_i] ) );
	}

#endif


	GtkImage *image;
	int      ret;

	if ( gnoclParseOptions ( interp, objc, objv, imageOptions )
			!= TCL_OK )
	{
		gnoclClearOptions ( imageOptions );
		return TCL_ERROR;
	}

	image = GTK_IMAGE ( gtk_image_new( ) );

	ret = gnoclSetOptions ( interp, imageOptions, G_OBJECT ( image ), -1 );

	if ( ret == TCL_OK )
	{
		ret = configure ( interp, image, imageOptions );
	}

	gnoclClearOptions ( imageOptions );

	if ( ret != TCL_OK )
	{
		gtk_widget_destroy ( GTK_WIDGET ( image ) );
		return TCL_ERROR;
	}

	gtk_widget_show ( GTK_WIDGET ( image ) );

	return gnoclRegisterWidget ( interp, GTK_WIDGET ( image ), imageFunc );
}

