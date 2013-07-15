/*
fixed.c
*/


/**
\page page_fixed gnocl::fixed
\htmlinclude fixed.html
**/
/**
\par Modification History
\verbatim
	2013-07:	added commands, options, commands
\endverbatim
**/

#include "gnocl.h"

/* function declarations */
static int gnoclOptRange ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret );

/*
  "lower"                    gdouble               : Read / Write
  "max-size"                 gdouble               : Read / Write
  "metric"                   GtkMetricType         : Read / Write
  "position"                 gdouble               : Read / Write
  "upper"                    gdouble               : Read / Write
*/
static GnoclOption fixedOptions[] =
{
	/* options based upon widget properties */
	{ "-x", GNOCL_INT, NULL },    			/* 0 */
	{ "-y", GNOCL_INT, "NULL"},			/* 1 */
	{ "-child", GNOCL_STRING, "NULL"},	/* 2 */
	{ NULL },
};

static const int xIdx = 0;
static const int yIdx = 1;
static const int childIdx  = 2;

/*
static void
gtk_fixed_remove (GtkContainer *container,
		  GtkWidget    *widget)
{
  GtkFixed *fixed;
  GtkFixedChild *child;
  GList *children;

  fixed = GTK_FIXED (container);

  children = fixed->children;
  while (children)
    {
      child = children->data;

      if (child->widget == widget)
	{
	  gboolean was_visible = GTK_WIDGET_VISIBLE (widget);

	  gtk_widget_unparent (widget);

	  fixed->children = g_list_remove_link (fixed->children, children);
	  g_list_free (children);
	  g_free (child);

	  if (was_visible && GTK_WIDGET_VISIBLE (container))
	    gtk_widget_queue_resize (GTK_WIDGET (container));

	  break;
	}

      children = children->next;
    }
}
*/

/**
\brief	Reposition child location within the GtkFixed widget stack
\note	Derived from function gtk_fixed_remove in gtkfixed.c
**/
static void restack ( GtkFixed *fixed, GtkWidget *widget, gchar *operation )
{

	GtkFixedChild *child;
	GtkFixedChild *child_info;
	GList *children;
	gint i, x, y;
	gboolean was_visible;

	children = fixed->children;
	i = 0;

	gchar action[16];
	gint level;

	sscanf ( operation, "%s %d", &action, &level );


	while ( children )
	{
		child = children->data;

		if ( child->widget == widget )
		{

			/* get a copy of the widget pointer and poisition */
			child_info = g_new ( GtkFixedChild, 1 );
			child_info->widget = child->widget;
			child_info->x = child->x;
			child_info->y = child->y;

			/* determine the visibility of the widget */
			was_visible = GTK_WIDGET_VISIBLE ( widget );

			fixed->children = g_list_remove_link ( fixed->children, children );

			/* move widget down up level in the z-stack */
			if ( strcmp ( "raise", action ) == 0 )
			{
				fixed->children = g_list_insert  ( fixed->children, child_info, ++i );
			}

			/* move widget down one level in the z-stack */
			else if ( strcmp ( "lower", action ) == 0 )
			{
				fixed->children = g_list_insert  ( fixed->children, child_info, --i );
			}

			/* move the widget to the bottom of the z-stack */
			else if ( strcmp ( "toBottom", action ) == 0 )
			{
				fixed->children = g_list_prepend ( fixed->children, child_info );
			}

			/* move the widget to the bottom of the z-stack */
			else if ( strcmp ( "stackLevel", action ) == 0 )
			{
				fixed->children =  g_list_insert ( fixed->children, child_info, level );
			}

			/* move the widget to the top of the z-stack, default */
			else
			{
				fixed->children = g_list_append ( fixed->children, child_info );
			}

			g_list_free ( children );
			g_free ( child );

			/* determine if the container needs resizing */
			if ( was_visible && GTK_WIDGET_VISIBLE ( fixed ) )
			{
				gtk_widget_queue_resize ( GTK_WIDGET ( fixed ) );
			}

			break;
		}

		i++;
		children = children->next;
	}

}


/**
\brief	Add child widget to the GtkFIxed container.
**/
static int addChildren ( GtkFixed *fixed, Tcl_Interp *interp, Tcl_Obj *children, GnoclOption *options )
{
	g_print ( "addChildren %s\n", options[childIdx].val.str );

	GtkWidget *child;
	gint x, y;

	/* add the widget to the frame! */
	child = gnoclGetWidgetFromName ( options[childIdx].val.str, interp );

	/* check for -x and -y options, otherwise place at 0,0 */
	if ( options[xIdx].status == GNOCL_STATUS_CHANGED && options[yIdx].status == GNOCL_STATUS_CHANGED )
	{
		g_print ( "addChildren -3 x = %d  y = %d\n", options[xIdx].val.i, options[yIdx].val.i );

		x = options[xIdx].val.i;
		y = options[yIdx].val.i;

		gtk_fixed_put ( GTK_FIXED ( fixed ), child, x, y );

	}

	/* check for -x and -y options, otherwise place at 0,0 */
	if ( options[xIdx].status == GNOCL_STATUS_CHANGED && options[yIdx].status == GNOCL_STATUS_CHANGED )
	{
		g_print ( "addChildren -3 x = %d  y = %d\n", options[xIdx].val.i, options[yIdx].val.i );

		x = options[xIdx].val.i;
		y = options[yIdx].val.i;

		gtk_fixed_put ( GTK_FIXED ( fixed ), child, x, y );

	}

	else
	{
		gtk_fixed_put ( GTK_FIXED ( fixed ), child, 0, 0 );
	}

	return TCL_OK;
}

/**
\brief	Remove child widget from GtkFixed container.
**/
static void removeChild ( GtkWidget *widget, gpointer data )
{
	GtkBox *box = GTK_BOX ( data );
	/* FIXME: is this correct? where to put the g_object_unref? */
	g_object_ref ( widget );
	gtk_container_remove ( GTK_CONTAINER ( box ), widget );

	if ( GTK_IS_ALIGNMENT ( widget ) )
	{
		GtkWidget *child = gtk_bin_get_child ( GTK_BIN ( widget ) );
		g_object_ref ( child );
		gtk_container_remove ( GTK_CONTAINER ( widget ), child );
		g_object_unref ( widget );
	}
}

/**
\brief
\author
\date
**/
static int gnoclOptRange ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{

	char *str = Tcl_GetString ( opt->val.str );
	GtkWidget *ruler = GTK_WIDGET ( obj );
	gdouble lower, upper, position, max_size;

	/* why isn't this parsing the string correctly? */
	sscanf ( str, "%f %f %f %f", &lower, &upper, &position, &max_size );

	g_print ( "%s %f %f %f %f\n", str, lower, upper, position, max_size );

	/* Paramters for the set range library call
	lower :     the lower limit of the ruler
	upper :     the upper limit of the ruler
	position :  the mark on the ruler
	max_size :  the maximum size of the ruler used when calculating the space to leave for the text
	*/

	gtk_ruler_set_range ( GTK_RULER ( ruler ), lower, upper, position, max_size );

	return TCL_OK;
}

/**
\brief	Set widget options based upon recieved values.
**/
static int configure ( Tcl_Interp *interp, GtkFixed *fixed, GnoclOption options[] )
{

#ifdef DEBUG_FIXED
	g_print ( "configure -1\n" );
#endif


	if ( options[childIdx].status == GNOCL_STATUS_CHANGED )
	{
#ifdef DEBUG_FIXED
		g_print ( "configure -2 %s\n", options[childIdx].val.str );
#endif
		addChildren ( fixed, interp, NULL, options );
	}

	return TCL_OK;
}


/**
\brief	Obtain current values for widget options.
**/
static int cget ( Tcl_Interp *interp, GtkWidget *widget, GnoclOption options[], int idx )
{
#ifdef DEBUG_FIXED
	g_print ( "cget\n" );
#endif

	return gnoclCgetNotImplemented ( interp, options + idx );
}

static const char *cmds[] =
{
	"delete", "configure", "add", "move",
	"remove", "class", "raise", "lower",
	"children", "toTop", "toBottom", "position",
	"stackLevel", "size",
	NULL
};

/**
\brief	Function assigned to the names instances of the gnocl::fixed widget.
**/
static int fixedFunc ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{
#ifdef DEBUG_FIXED
	g_print ( "fixedFunc\n" );
#endif


	enum cmdIdx
	{
		DeleteIdx, ConfigureIdx, AddIdx, MoveIdx,
		RemoveIdx, ClassIdx, RaiseIdx, LowerIdx,
		ChildrenIdx, ToTopIdx, ToBottomIdx, PositionIdx,
		StackLevelIdx, SizeIdx
	};

	GtkWidget *widget = GTK_WIDGET ( data );
	GtkFrame *frame;
	GtkFixed *fixed;
	int idx;

	if ( GTK_IS_FRAME ( widget ) )
	{
		frame = GTK_FRAME ( widget );
		fixed = GTK_FIXED ( gtk_bin_get_child ( GTK_BIN ( frame ) ) );
	}

	else
	{
		frame = NULL;
		fixed = GTK_FIXED ( widget );
	}


	if ( Tcl_GetIndexFromObj ( interp, objv[1], cmds, "command",  TCL_EXACT, &idx ) != TCL_OK )
	{
		return TCL_ERROR;
	}

	switch ( idx )
	{

			/* return child widget position as {x y} or size (w h) */
		case SizeIdx:
		case PositionIdx:
			{

				char buffer[128];
				GtkWidget *widget;
				GtkRequisition req;

				widget = gnoclGetWidgetFromName (  Tcl_GetString ( objv[2] ), interp );

				GtkFixedChild *child;
				GList *children;
				children = fixed->children;

				while ( children )
				{
					child = children->data;

					if ( child->widget == widget )
					{

						gtk_widget_size_request ( widget, &req );

						if ( idx == SizeIdx )
						{
							sprintf ( buffer, "%d %d", child->x, child->y );
						}

						else
						{
							sprintf ( buffer, "%d %d", req.width, req.height );
						}

						//g_list_free ( children );
						//g_free ( child );
						break;
					}

					children = children->next;
				}

				Tcl_SetObjResult ( interp, Tcl_NewStringObj ( buffer, -1 ) );

			}
			break;
		case ChildrenIdx:
			{
#ifdef DEBUG_FIXED
				g_print ( "Children\n" );
#endif
				GList *childList, *p;
				Tcl_Obj *resList;
				gchar *name;

				childList = gtk_container_get_children ( GTK_CONTAINER ( fixed ) );

				resList = Tcl_NewListObj ( 0, NULL );

				for ( p = childList; p != NULL; p = p->next )
				{
					name = gnoclGetNameFromWidget ( p->data );
					Tcl_ListObjAppendElement ( interp, resList, Tcl_NewStringObj ( name, -1 ) );
				}

				/*  tidy up and return the answer */
				g_list_free ( childList );

				Tcl_SetObjResult ( interp, resList );
			}
			break;

		case ToTopIdx:
		case ToBottomIdx:
		case RaiseIdx:
		case LowerIdx:
		case StackLevelIdx:
			{
#ifdef DEBUG_FIXED
				g_print ( "Raise/Lower\n" );
#endif
				GtkWidget *child;
				char *name;
				char *operation;
				name = Tcl_GetString ( objv[2] );
				operation = Tcl_GetString ( objv[1] );
				child = gnoclGetWidgetFromName ( name, interp );

				if ( idx == StackLevelIdx )
				{
					sprintf ( operation, "level %d", Tcl_GetString ( objv[3] ) );
				}

				if ( GTK_IS_WIDGET ( child ) )
				{
					restack ( fixed, child, operation );
				}

				else
				{
					Tcl_SetResult ( interp, "GNOCL ERROR: Object is not a valid widget.", TCL_STATIC );
					return TCL_ERROR;
				}
			}
			break;
		case DeleteIdx:
			{
				return gnoclDelete ( interp, fixed, objc, objv );
			}
			break;
		case MoveIdx:
		case AddIdx:
			{
				/* place widget at position specified by -x / -y : default = 0,0 */
				GtkWidget *child;
				char *name;
				gint x;
				gint y;
				gint width;
				gint height;

				x = 0; y = 0;
				name = Tcl_GetString ( objv[2] );
				child = gnoclGetWidgetFromName ( name, interp );

				if ( GTK_IS_WIDGET ( child ) )
				{
				}
				else
				{
					Tcl_SetResult ( interp, "GNOCL ERROR: Object is not a valid widget.", TCL_STATIC );
					return TCL_ERROR;
				}

				/* add appropriate error message */
				if ( objc < 3 )
				{
					Tcl_WrongNumArgs ( interp, 1, objv, "add <widget-id> [optional -x <int> -y <int> -width <int> -height <int>]" );
					return TCL_ERROR;
				}

				name = Tcl_GetString ( objv[2] );

				static char *newOptions[] =
				{
					"-x", "-y", "-width", "-height", NULL
				};

				static enum  optsIdx
				{
					XIdx, YIdx, WidthIdx, HeightIdx
				};

				gint i;
				gint j;
				gint idx2;
				gchar *opt;

				for ( i = 3; i < objc; i += 2 )
				{
					j = i + 1;

					opt = Tcl_GetString ( objv[i] );

					if ( Tcl_GetIndexFromObj ( interp, objv[i], newOptions, "command", TCL_EXACT, &idx2 ) != TCL_OK )
					{
						return TCL_ERROR;
					}

					switch ( idx2 )
					{
						case XIdx:
							{
								Tcl_GetIntFromObj ( NULL, objv[j], &x );
							} break;
						case YIdx:
							{
								Tcl_GetIntFromObj ( NULL, objv[j], &y );
							} break;
						case WidthIdx:
							{
								Tcl_GetIntFromObj ( NULL, objv[j], &width );
							} break;
						case HeightIdx:
							{
								Tcl_GetIntFromObj ( NULL, objv[j], &height );
							} break;

						default: {}
					}

				}

				/* get widget */
				child = gnoclGetWidgetFromName ( name, interp );

				/* check, then insert */
				if ( GTK_IS_WIDGET ( child ) )
				{
					gtk_widget_set_usize ( child, width, height );

					if ( idx == AddIdx )
					{

						gtk_fixed_put ( GTK_FIXED ( fixed ), child, x, y );
					}

					else
					{
						gtk_fixed_move ( GTK_FIXED ( fixed ), child, x, y );
					}
				}

				else
				{
					Tcl_SetResult ( interp, "GNOCL ERROR: Object is not a valid widget.", TCL_STATIC );
					return TCL_ERROR;
				}

			}
			break;
		case RemoveIdx:
			{
				char *name;
				GtkWidget *child;
				name = Tcl_GetString ( objv[2] );
				child = gnoclGetWidgetFromName ( name, interp );

				/* check, then insert */
				if ( GTK_IS_WIDGET ( child ) )
				{
					g_object_ref ( child );
					gtk_container_remove ( fixed, child );
				}

				else
				{
					Tcl_SetResult ( interp, "GNOCL ERROR: Object is not a valid widget.", TCL_STATIC );
					return TCL_ERROR;
				}

			}
			break;
		case ClassIdx:
			{
			}
			break;
		default: {}

	}

	return TCL_OK;
}

/**
\brief	Create and instance of the gnocl::fixed container widget.
**/
int gnoclFixedCmd ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{
	if ( gnoclGetCmdsAndOpts ( interp, cmds, fixedOptions, objv, objc ) == TCL_OK )
	{
		return TCL_OK;
	}



	int ret;
	GtkFrame *frame = NULL;
	GtkWidget *widget;
	GtkFixed *fixed;


	if ( gnoclParseOptions ( interp, objc, objv, fixedOptions ) != TCL_OK )
	{
		gnoclClearOptions ( fixedOptions );
		return TCL_ERROR;
	}


	fixed = gtk_fixed_new();

	/* set default value */
	gtk_container_set_border_width ( GTK_CONTAINER ( fixed ), GNOCL_PAD_TINY );

	widget = GTK_WIDGET ( fixed );

	ret = configure ( interp, fixed, fixedOptions );

	gnoclClearOptions ( fixedOptions );

	if ( ret != TCL_OK )
	{
		gtk_widget_destroy ( widget );
		return TCL_ERROR;
	}

	gtk_widget_show_all ( widget );

	return gnoclRegisterWidget ( interp, widget, fixedFunc );
}

