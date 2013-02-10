
#include "gnocl.h"


int toolItemGroupFunc ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] );

static GnoclOption options[] =
{
	/* widget specific options */
	{ "-collapsed", GNOCL_BOOL, "collapsed"},
	{ "-ellipsize", GNOCL_OBJ, "ellipsize", NULL },

	{ "-relief", GNOCL_OBJ, "header-relief", NULL },
	{ "-label", GNOCL_STRING, "label" },
	{ "-labelWidget", GNOCL_STRING, "label-widget", NULL },
	{ "-parent", GNOCL_OBJ, "", NULL},


	/* widget signals */
	{ "-onSetScrollAdjustments", GNOCL_OBJ, "", gnoclOptOnScrollAdjustments}, /* taken from text.c, perhaps custom handler needed, see layout.c */

	/* general options */
	{ "-name", GNOCL_STRING, "name" },
	{ "-visible", GNOCL_BOOL, "visible" },
	{ "-sensitive", GNOCL_BOOL, "sensitive" },
	{ "-data", GNOCL_OBJ, "", gnoclOptData },
	{ "-tooltip", GNOCL_OBJ, "", gnoclOptTooltip },
	{ NULL },
};


/**
\brief
**/
static int configure ( Tcl_Interp *interp, GtkWidget *toolPalette, GnoclOption options[] )
{

	return TCL_OK;
}


/**
\brief
**/
int addGroup ( GtkWidget *palette, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{

	debugStep ( __FUNCTION__, 1 );

	GtkWidget *group, *item;

	static char *groupOptions[] =
	{
		"-collapsed", "-ellipsize",	"-relief", "-label", "-labelWidget", NULL
	};

	static enum  optsIdx { CollapsedIdx, EllipsizeIdx, ReliefIdx, LabelIdx, LabelWidgetIdx };

	gint idx;
	gint i;

	/* create new group */

	group = gtk_tool_item_group_new ( "Test Category NEW" );

	for ( i = 2; i < objc; i += 2 )
	{

		getIdx ( groupOptions, Tcl_GetString ( objv[i] ), &idx );

		switch ( idx )
		{
			case CollapsedIdx:
				{
					gboolean collapsed;
					Tcl_GetIntFromObj ( NULL, objv[i+1], &collapsed );

					gtk_tool_item_group_set_collapsed   ( group, collapsed );

				} break;
			case EllipsizeIdx:
				{
					debugStep ( "EllipsizeIdx", 1 );

					static char *ellipsizeOptions[] =
					{
						"none", "start", "middle", "end", NULL
					};

					static enum  optsIdx { NoneIdx, StartIdx , MiddleIdx, EndIdx };

					gint idx;

					getIdx ( ellipsizeOptions, Tcl_GetString ( objv[i+1] ), &idx );

					switch ( idx )
					{
						case NoneIdx:
							{
								debugStep ( "NoneIdx", 0 );
								gtk_tool_item_group_set_ellipsize ( group, PANGO_ELLIPSIZE_NONE );
							} break;
						case StartIdx:
							{
								debugStep ( "StartIdx", 1 );
								gtk_tool_item_group_set_ellipsize ( group, PANGO_ELLIPSIZE_START );
							} break;
						case MiddleIdx:
							{
								debugStep ( "MiddleIdx", 2 );

								gtk_tool_item_group_set_ellipsize ( group, PANGO_ELLIPSIZE_MIDDLE );
							} break;
						case EndIdx:
							{
								debugStep ( "EndIdx", 3 );

								gtk_tool_item_group_set_ellipsize ( group, PANGO_ELLIPSIZE_END );
							} break;
						default:
							{

							};
					}


				} break;

			case ReliefIdx:
				{
					debugStep ( "ReliefIdx", 1 );
					static char *reliefOptions[] =
					{
						"normal", "half", "none", NULL
					};

					static enum  optsIdx { NormalIdx, HalfIdx, NoneIdx };

					gint idx;

					getIdx ( groupOptions, Tcl_GetString ( objv[i+1] ), &idx );


					switch ( idx )
					{
						case NormalIdx:
							{
								debugStep ( "ReliefIdx", 0 );
								gtk_tool_item_group_set_header_relief ( group, GTK_RELIEF_NORMAL );
							} break;
						case HalfIdx:
							{
								debugStep ( "ReliefIdx", 1 );
								gtk_tool_item_group_set_header_relief ( group, GTK_RELIEF_HALF );
							} break;
						case NoneIdx:
							{
								debugStep ( "ReliefIdx", 2 );

								gtk_tool_item_group_set_header_relief ( group, GTK_RELIEF_NONE );
							} break;

						default:
							{
								gtk_tool_item_group_set_header_relief ( group, GTK_RELIEF_NORMAL );
							}
					}



				} break;
			case LabelIdx:
				{
					gtk_tool_item_group_set_label ( group, Tcl_GetString ( objv[i+1] ) );

				} break;
			case LabelWidgetIdx:
				{
					GtkWidget *label_widget;

					label_widget = gnoclGetWidgetFromName ( Tcl_GetString ( objv[i+1] ), interp );

					gtk_tool_item_group_set_label_widget ( group, label_widget );

				} break;
			default: {}
		}

	}

	/*
		item = gtk_tool_button_new_from_stock ( GTK_STOCK_OK );

		gtk_tool_item_group_insert ( GTK_TOOL_ITEM_GROUP ( group ), item, -1 );

		item = gtk_tool_button_new_from_stock ( GTK_STOCK_OK );

		gtk_tool_item_group_insert ( GTK_TOOL_ITEM_GROUP ( group ), item, -1 );

		item = gtk_tool_button_new_from_stock ( GTK_STOCK_OK );

		gtk_tool_item_group_insert ( GTK_TOOL_ITEM_GROUP ( group ), item, -1 );
	*/
	debugStep ( __FUNCTION__, 2 );

	gtk_container_add ( GTK_CONTAINER ( palette ), group );


	gtk_widget_show_all ( gtk_widget_get_parent ( palette ) );

	return gnoclRegisterWidget ( interp, GTK_WIDGET ( group ), toolItemGroupFunc );

}

/*
  "icon-name"                gchar*                : Read / Write
  "icon-widget"              GtkWidget*            : Read / Write
  "label"                    gchar*                : Read / Write
  "label-widget"             GtkWidget*            : Read / Write
  "stock-id"                 gchar*                : Read / Write
  "use-underline"            gboolean              : Read / Write
*/
int addItem ( GtkWidget  *group, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{

	listParameters ( objc, objv, __FUNCTION__ );
	/*
	"-icon",
	"-widget",
	"-label",
	"-labelWidget",
	"-underLine",
	*/

	GtkToolItem *item = NULL;

	static const char *itemOpts[] =
	{
		"-icon", "-widget", "-label", "-labelWidget", "-underline"
		"-onClicked", "-tooltip",
		NULL
	};

	enum itemOptsIdx
	{
		IconIdx, WidgetIdx, LabelIdx, LabelWidgetIdx, UnderLineIdx,
		OnClickedIdx, TooltipIdx
	};


	gint i;
	int idx;

	//while (i < objc)
	for ( i = 2; i < objc; i += 2 )
	{
		getIdx ( itemOpts, Tcl_GetString ( objv[i] ), &idx );

		g_print ( " %d/%d --->%s = %s\n", i, objc, Tcl_GetString ( objv[i] ), Tcl_GetString ( objv[i+1] ) );

		switch ( idx )
		{
			case TooltipIdx:
				{
					debugStep ( "TooltipIdx", 1 );
					g_print ( "%s\n", Tcl_GetString ( objv[i+1] ) );
					gtk_widget_set_tooltip_text ( GTK_WIDGET ( item ), Tcl_GetString ( objv[i+1] ) );
				}
				break;
			case IconIdx:
				{

					debugStep ( "IconIdx", 1 );
					GtkStockItem stockItem;

					if ( gnoclGetStockItem ( objv[i+1], interp, &stockItem ) != TCL_OK )
					{
						debugStep ( "IconIdx", 2 );
						//item = gtk_tool_button_new_from_stock (  GTK_STOCK_OK  );
						item = gtk_tool_button_new_from_stock (  stockItem.stock_id  );
					}

				} break;
			case WidgetIdx: {} break;
			case LabelIdx: {} break;
			case LabelWidgetIdx: {} break;
			case UnderLineIdx: {} break;
			case OnClickedIdx:
				{
					//gnoclConnectOptCmd ( interp, item, "clicked", G_CALLBACK ( doOnDelete ), opt, NULL, ret );
				} break;
			default:
				{
					g_print ( "%d -missed this one!\n", i );
				}
		}

		//i+=2;
	}

	if ( item == NULL )
	{
		item = gtk_tool_button_new_from_stock (  GTK_STOCK_OK  );
	}

	gtk_tool_item_group_insert ( GTK_TOOL_ITEM_GROUP ( group ), item, -1 );
	gtk_widget_show_all ( GTK_WIDGET ( group ) );

	return TCL_OK;

}


/**
\brief
**/
int toolItemGroupFunc ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{
	GtkWidget  *group = ( GtkWidget * ) data;

	listParameters ( objc, objv, __FUNCTION__ );

	addItem ( group, interp, objc, objv );


	return TCL_OK;
}



/**
\brief
**/
int gnoclToolItemGroupCmd ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{
	int        ret;
	GtkWidget  *item_group, *item;


	item_group = gtk_tool_item_group_new ( "MY GROUP" );

	ret = gnoclSetOptions ( interp, options, G_OBJECT ( item_group ), -1 );

	if ( ret == TCL_OK )
	{
		ret = configure ( interp, item_group, options );
	}

	gnoclClearOptions ( options );

	if ( ret != TCL_OK )
	{
		gtk_widget_destroy ( GTK_WIDGET ( item_group ) );
		return TCL_ERROR;
	}

	item = gtk_tool_button_new_from_stock ( GTK_STOCK_OK );
	gtk_tool_item_group_insert ( GTK_TOOL_ITEM_GROUP ( item_group ), item, -1 );

	/* TODO: if not -visible == 0 */
	gtk_widget_show_all ( GTK_WIDGET ( item_group ) );

	return gnoclRegisterWidget ( interp, GTK_WIDGET ( item_group ), toolItemGroupFunc );
}
