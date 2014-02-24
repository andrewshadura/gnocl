/* gtklevel.c */

#include "gtklevel.h"

#define LEVEL_WIDTH 14
#define LEVEL_HEIGHT 83
#define LEVEL_RED_ZONE 5

static void gtk_level_class_init ( GtkLevelClass *klass );
static void gtk_level_init ( GtkLevel *level );
static void gtk_level_size_request ( GtkWidget *widget, GtkRequisition *requisition );
static void gtk_level_size_allocate ( GtkWidget *widget, GtkAllocation *allocation );
static void gtk_level_realize ( GtkWidget *widget );
static gboolean gtk_level_expose ( GtkWidget *widget, GdkEventExpose *event );
static void gtk_level_paint ( GtkWidget *widget );
static void gtk_level_destroy ( GtkObject *object );

/**
\brief
**/
GtkType gtk_level_get_type ( void )
{
	static GtkType gtk_level_type = 0;

	if ( !gtk_level_type )
	{
		static const GtkTypeInfo gtk_level_info =
		{
			"GtkLevel",
			sizeof ( GtkLevel ),
			sizeof ( GtkLevelClass ),
			( GtkClassInitFunc ) gtk_level_class_init,
			( GtkObjectInitFunc ) gtk_level_init,
			NULL,
			NULL,
			( GtkClassInitFunc ) NULL
		};
		gtk_level_type = gtk_type_unique ( GTK_TYPE_WIDGET, &gtk_level_info );
	}


	return gtk_level_type;
}

/**
\brief
**/
void gtk_level_set_state ( GtkLevel *level, gint num )
{
	level->sel = num;
	gtk_level_paint ( GTK_WIDGET ( level ) );
}

/**
\brief
**/
GtkWidget * gtk_level_new()
{
	return GTK_WIDGET ( gtk_type_new ( gtk_level_get_type() ) );
}

/**
\brief
**/
static void gtk_level_class_init ( GtkLevelClass *klass )
{
	GtkWidgetClass *widget_class;
	GtkObjectClass *object_class;


	widget_class = ( GtkWidgetClass * ) klass;
	object_class = ( GtkObjectClass * ) klass;

	widget_class->realize = gtk_level_realize;
	widget_class->size_request = gtk_level_size_request;
	widget_class->size_allocate = gtk_level_size_allocate;
	widget_class->expose_event = gtk_level_expose;

	object_class->destroy = gtk_level_destroy;
}

/**
\brief
**/
static void gtk_level_init ( GtkLevel *level )
{
	level->sel = 0;
}

/**
\brief
**/
static void gtk_level_size_request ( GtkWidget *widget,
									 GtkRequisition *requisition )
{
	g_return_if_fail ( widget != NULL );
	g_return_if_fail ( GTK_IS_LEVEL ( widget ) );
	g_return_if_fail ( requisition != NULL );

	requisition->width = LEVEL_WIDTH;
	requisition->height = LEVEL_HEIGHT;
}

/**
\brief
**/
static void gtk_level_size_allocate ( GtkWidget *widget,
									  GtkAllocation *allocation )
{
	g_return_if_fail ( widget != NULL );
	g_return_if_fail ( GTK_IS_LEVEL ( widget ) );
	g_return_if_fail ( allocation != NULL );

	widget->allocation = *allocation;

	if ( GTK_WIDGET_REALIZED ( widget ) )
	{
		gdk_window_move_resize (
			widget->window,
			allocation->x, allocation->y,
			allocation->width, allocation->height
		);
	}
}

/**
\brief
**/
static void gtk_level_realize ( GtkWidget *widget )
{
	GdkWindowAttr attributes;
	guint attributes_mask;

	g_return_if_fail ( widget != NULL );
	g_return_if_fail ( GTK_IS_LEVEL ( widget ) );

	GTK_WIDGET_SET_FLAGS ( widget, GTK_REALIZED );

	attributes.window_type = GDK_WINDOW_CHILD;
	attributes.x = widget->allocation.x;
	attributes.y = widget->allocation.y;
	attributes.width = LEVEL_WIDTH;
	attributes.height = LEVEL_HEIGHT;

	attributes.wclass = GDK_INPUT_OUTPUT;
	attributes.event_mask = gtk_widget_get_events ( widget ) | GDK_EXPOSURE_MASK;

	attributes_mask = GDK_WA_X | GDK_WA_Y;

	widget->window = gdk_window_new (
						 gtk_widget_get_parent_window ( widget ),
						 & attributes, attributes_mask
					 );

	gdk_window_set_user_data ( widget->window, widget );

	widget->style = gtk_style_attach ( widget->style, widget->window );
	gtk_style_set_background ( widget->style, widget->window, GTK_STATE_NORMAL );
}

/**
\brief
**/
static gboolean gtk_level_expose ( GtkWidget *widget, GdkEventExpose *event )
{
	g_return_val_if_fail ( widget != NULL, FALSE );
	g_return_val_if_fail ( GTK_IS_LEVEL ( widget ), FALSE );
	g_return_val_if_fail ( event != NULL, FALSE );

	gtk_level_paint ( widget );

	return FALSE;
}

/**
\brief
**/
static void gtk_level_paint ( GtkWidget *widget )
{
	cairo_t *cr;

	/* create a drawing context */
	cr = gdk_cairo_create ( widget->window );

	cairo_translate ( cr, 0, 2 );

	/* paint a black background */
	cairo_set_source_rgb ( cr, 0, 0, 0 );
	cairo_paint ( cr );

	gint pos = GTK_LEVEL ( widget )->sel;
	gint rect = pos / 5;

	cairo_set_source_rgb ( cr, 0.2, 0.4, 0 );

	gint i;

	for ( i = 20; i >= 0; i-- )
	{
		if ( i > 20 - rect )
		{
			if ( i > LEVEL_RED_ZONE )
			{
				cairo_set_source_rgb ( cr, 0.6, 1.0, 0 );
			}

			else
			{
				cairo_set_source_rgb ( cr, 1.0, 0, 0 );
			}
		}

		else
		{
			cairo_set_source_rgb ( cr, 0.2, 0.4, 0 );
		}

		cairo_rectangle ( cr, 2, i * 4 - 4, 10, 3 );
		cairo_fill ( cr );
	}

	cairo_destroy ( cr );
}

/**
\brief
**/
static void gtk_level_destroy ( GtkObject *object )
{
	GtkLevel *level;
	GtkLevelClass *klass;

	g_return_if_fail ( object != NULL );
	g_return_if_fail ( GTK_IS_LEVEL ( object ) );

	level = GTK_LEVEL ( object );

	klass = gtk_type_class ( gtk_widget_get_type() );

	if ( GTK_OBJECT_CLASS ( klass )->destroy )
	{
		( * GTK_OBJECT_CLASS ( klass )->destroy ) ( object );
	}
}
