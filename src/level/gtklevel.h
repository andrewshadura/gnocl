/* level.h */

#ifndef __LEVEL_H
#define __LEVEL_H

#include <gtk/gtk.h>
#include <cairo.h>

G_BEGIN_DECLS


#define GTK_LEVEL(obj) GTK_CHECK_CAST(obj, gtk_level_get_type (), GtkLevel)
#define GTK_LEVEL_CLASS(klass) GTK_CHECK_CLASS_CAST(klass, gtk_level_get_type(), GtkLevelClass)
#define GTK_IS_LEVEL(obj) GTK_CHECK_TYPE(obj, gtk_level_get_type())


typedef struct _GtkLevel GtkLevel;
typedef struct _GtkLevelClass GtkLevelClass;


struct _GtkLevel
{
	GtkWidget widget;
	gint sel;
};

struct _GtkLevelClass
{
	GtkWidgetClass parent_class;
};


GtkType gtk_level_get_type ( void );
void gtk_level_set_sel ( GtkLevel *level, gint sel );
GtkWidget * gtk_level_new();


G_END_DECLS

#endif /* __LEVEL_H */
