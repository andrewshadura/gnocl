/** attr-table.c **/
#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>

int main ( int argc, char *argv[] )
{
	gchar *str =
		"<u>car </u><span foreground=\"blue\"><u>is </u>THE CAR</span> in Arabic";
	gchar *text;
	PangoAttrList *attrs;
	PangoAttrIterator *iterator;
	gint start, end;
	PangoAttribute *attr;

	pango_parse_markup ( str, -1, 0, &attrs, &text, NULL, NULL );

	g_print ( "<html>\n<head>\n" );
	g_print ( "<meta http-equiv=\"Content-Type\""
			  "content=\"text/html; charset=\"UTF-8\">\n" );
	g_print ( "</head>\n<body bgcolor=\"white\">\n" );
	g_print ( "<table border=\"1\" bgcolor=\"white\">\n" );

	iterator = pango_attr_list_get_iterator ( attrs );

	g_print ( "<tr>\n<td>String:</td>" );

	do
	{
		pango_attr_iterator_range ( iterator, &start, &end );

		g_print ( "<td>%.*s</td>", MIN ( ( gint ) strlen ( text ), end - start ),
				  &text[start] );
	}
	while ( pango_attr_iterator_next ( iterator ) );

	g_print ( "</tr>\n" );

	iterator = pango_attr_list_get_iterator ( attrs );

	g_print ( "<tr>\n<td>Foreground:</td>" );

	do
	{
		pango_attr_iterator_range ( iterator, &start, &end );

		if ( ( attr = pango_attr_iterator_get ( iterator,
												PANGO_ATTR_FOREGROUND ) ) )
		{
			const PangoAttrColor *color_attr = ( const PangoAttrColor * ) attr;
			g_print ( "<td>#%04x%04x%04x</td>", color_attr->red,
					  color_attr->green, color_attr->blue );
		}

		else
		{
			g_print ( "<td>Â </td>" );
		}
	}
	while ( pango_attr_iterator_next ( iterator ) );

	g_print ( "</tr>\n" );

	iterator = pango_attr_list_get_iterator ( attrs );

	g_print ( "<tr>\n<td>Underline:</td>" );

	do
	{
		pango_attr_iterator_range ( iterator, &start, &end );

		attr = pango_attr_iterator_get ( iterator, PANGO_ATTR_UNDERLINE );

		if ( attr )
		{
			g_print ( "<td>Yes</td>" );
		}

		else
		{
			g_print ( "<td>Â </td>" );
		}
	}
	while ( pango_attr_iterator_next ( iterator ) );

	g_print ( "</tr>\n" );

	g_print ( "</table>\n</body>\n</html>\n" );

	pango_attr_iterator_destroy ( iterator );

	exit ( 0 );
}
