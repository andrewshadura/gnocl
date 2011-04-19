/*
   History:
   2011-02: Begin developement
 */

#include "gnocl.h"
#include <string.h>
#include <assert.h>


/* Notice that these are initialized to the empty list. */
GSList *itemList = NULL;
Tcl_Interp *tcl_interp;
GtkMenu *popupMenu;



void _demoStub ( GtkMenu *menu )
{

	/* some menu appending stub */

	GtkWidget *m;
	GtkWidget *i;

	gint x;

	i = gtk_separator_menu_item_new();
	gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), i );
	gtk_widget_show ( i );

	i = gtk_separator_menu_item_new();
	gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), i );
	gtk_widget_show ( i );

	i = gtk_menu_item_new_with_label ( "Some item" );
	gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), i );
	gtk_widget_show ( i );

	m = gtk_menu_new();
	i = gtk_menu_item_new_with_label ( "Some menu" );
	gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), i );
	gtk_menu_item_set_submenu ( GTK_MENU_ITEM ( i ), m );
	gtk_widget_show ( i );


	const gchar *labels[] =
	{
		"Some menu item number 1",
		"This is a menu item too",
		"this is too",
		NULL,
	};


	for ( x = 0; labels[x] != NULL; x++ )
	{
		i = gtk_menu_item_new_with_label ( labels[x] );
		gtk_menu_shell_append ( GTK_MENU_SHELL ( m ), i );
		gtk_widget_show ( i );
	}


}

static void _doOnActivate ( GtkMenuItem *item, gchar *str )
{
	g_print ( "doActivtate -> %s", str );
}


/**
	called from popup-populate command
**/
int _gnoclAppendPopupMenuItems ( GtkMenu *menu )
{
	g_print ( "gnoclPopupMenuAppend\n" );

	//demoStub ( menu );

	gtk_menu_shell_append ( GTK_MENU_SHELL ( menu ), itemList );
	gtk_widget_show ( itemList );
}

/**************** key the functions below ******************/

/**
**/
void setPopupMenu ( GtkMenu *menu )
{
	popupMenu = menu;
}

/**
	called from text command to add popupitem
**/
void gnoclPopupMenuAddItem ( Tcl_Interp *interp, gchar *str )
{
	g_print ( "gnoclPopupMenuAddItem %s\n", str );

	GtkWidget *item;

	item = gnoclGetWidgetFromName ( str, interp );

	gtk_menu_shell_append ( GTK_MENU_SHELL ( popupMenu ), item );
	gtk_widget_show ( item );
}

/**
	called from text command to add popupitem
**/
void gnoclPopupMenuAddSubMenu ( Tcl_Interp *interp, gchar *str1, gchar *str2 )
{
	g_print ( "gnoclPopupMenuAddSubMenu %s %s\n", str1, str2 );

	GtkWidget *item, *submenu;
	GtkWidget *i;

	//demoStub ( popupMenu );

	/* get item to which submenu will be attached */
	item = gnoclGetWidgetFromName ( str1, interp );

	submenu = gnoclGetWidgetFromName ( str2, interp );

	//i = gtk_menu_item_new_with_label ( "TEST");
	//gtk_menu_shell_append ( GTK_MENU_SHELL ( popupMenu ), i );

	gtk_menu_item_set_submenu ( GTK_MENU_ITEM ( item ), submenu ) ;
	//gtk_widget_show ( i );

	//return gnoclRegisterWidget ( interp, GTK_WIDGET ( m ), menuFunc );

}

/**
	called from text command to add popupitem
**/
void gnoclPopupMenuAddSeparator ( Tcl_Interp *interp )
{
	g_print ( "gnoclPopupMenuAddSeparator\n" );

	GtkWidget *item;

	item = gtk_separator_menu_item_new();
	gtk_menu_shell_append ( GTK_MENU_SHELL ( popupMenu ), item );
	gtk_widget_show ( item );


}
