/*
 *  parseMarkup.c
 *
 *  See the file "license.terms" for information on usage and
 *  redistribution of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

/*
    The GtkTextBuffer has no default tags, whereas the Pango description
    requires a conventionalized set of descriptors. In order to
    guareentee correct display and editing of markupstrings, a protected
    set of markup tags need to created. Following this, any markup
    strings to be inserted can be added with safety.
*/


/*
    History:

    2011-09  began module development

*/

#include "gnocl.h"

static void applyTags ( GtkTextBuffer *buffer, GtkTextIter *iter );

/* list to hold names of applied tags */
static GList *tagList = NULL;


/*
char *strcpy(char *dest, const char *src)
{
  unsigned i;
  for (i=0; src[i] != '\0'; ++i)
    dest[i] = src[i];
  dest[i] = '\0';
  return dest;
}
*/




/**
    getMarkup
    get name of next tag in string, starting at position i
    return updated value of i

    test-string = "<b>bold</b>   <i>italic</i>   <u>underline</u>   <s>strikethrough</s>"

	CODE WORKS WELL.
**/
static int getMarkup ( char *str, char *tag, int i )
{
#if 0
	g_print ( "%s\n", __FUNCTION__ );
#endif

	int j;

	if ( strcmp ( str , "<" ) )
	{
		j = strnfrst ( str, ">", i );
		strrng ( tag, str, i, j + 1  );
		return j;
	}

	return i;
}




/**
\brief	Add tag from list of active tags.
		Last in, first out!
**/
static void addTag ( gchar *tag )
{
#if 1
	g_print ( "FUNC: %s; add %s\n", __FUNCTION__, tag );
#endif

	if ( g_slist_find ( tagList, tag ) == NULL && tag + 1 != '/' )
	{
		//g_print ( "\tAdded %s\n", tag );
		tagList = g_slist_prepend ( tagList, tag );
		//g_print ( "list length = %d\n", g_slist_length ( tagList ) );
	}
}

/**
\brief	Remove tag from list of active tags.
**/
static void removeTag ( gchar *tag )
{

#if 1
	g_print ( "FUNC: %s, remove %s\n", __FUNCTION__, tag );
#endif


	gchar *tmp;

	/* tag need to be checked
	 * remove "/" from tags
	 *
	 */
	tmp = str_replace ( tag, "/", "" );

	tagList = g_slist_remove  ( tagList, tmp );

	g_print ( "FUNC: %s, 1\n", __FUNCTION__, tag );

}

/**
\brief	This works.
**/
static void handleTag ( gchar tag[] )
{
#if 0
	g_print ( "FUNC: %s %s\n", __FUNCTION__, tag );
#endif


	if (  strncmp ( tag, "</", 2 ) == 0 )
	{
		//g_print ( "\tremove tag\n" );
		removeTag ( tag );
		return;
	}

	//g_print ( "\tadd tag\n" );
	addTag ( tag );

}

/**
\brief
\todo	test the legality of the markup string.
		swap &gtl; for ">" etc..
**/
int gnoclInsertMarkup ( GtkTextBuffer *buffer, GtkTextIter *iter, gchar str[] )
{
#if 0
	g_print ( "FUNC: %s\n", __FUNCTION__ );
#endif

	GList *lptr = NULL;


	/* interate through pango string, determine, text or markup */
	int i, j;
	gchar *ptr;
	const char tag[256];
	char txt[10];
	gchar *ch1, *ch2;
	GtkTextMark *mark;
	GtkTextIter *iter2;

	gchar *tags = "<b>";

	i = 0;

	/* initialize the text */
	while ( str[i] != NULL )
	{

		/* is this char the start of a tag? */
		if ( str[i] == '<' )
		{
			i = getMarkup ( str, tag, i );
			handleTag ( tag );

			g_print ( "tag = %s\n", tag );
		}
		else
		{
			g_print ( "c = '%c' ; tags = %s\n", str[i], tags );

			mark =  gtk_text_buffer_get_insert ( buffer );

			applyTags ( buffer, iter );

#if 1
			gtk_text_buffer_insert ( buffer, iter, &str[i], 1 );
#else
			gtk_text_buffer_get_iter_at_mark  ( buffer, iter, mark );
			gtk_text_buffer_insert_with_tags_by_name ( buffer, iter, &str[1], 1, tags, NULL );
#endif

			//gtk_text_buffer_get_iter_at_mark  (buffer, iter2, mark);



			//gtk_text_buffer_apply_tag_by_name (buffer,"<b>", iter2,iter2);

			//gtk_text_buffer_insert_at_cursor ( buffer, str[i],1);

			g_print ( "~~~~~\n", tag );
		}

		i++;
		//gtk_text_iter_forward_char (iter);

	}

	/* free memory */
	// g_list_free ( tagList );

	return 0;
}

/**
**/
//static GtkTextIter * applyTags ( GtkTextBuffer *buffer, GtkTextIter *iter )
static void applyTags ( GtkTextBuffer *buffer, GtkTextIter *iter )
{
#if 1
	g_print ( "FUNC: %s\n", __FUNCTION__ );
#endif
	GtkTextIter *iter2;
	GtkTextMark *mark;
	GList *p = NULL;


	GtkTextTagTable *table;

	table = gtk_text_buffer_get_tag_table ( GTK_TEXT_BUFFER ( buffer ) );

	/* iterate through tagList */
	for ( p = ( GList * ) tagList; p != NULL; p = p->next )
	{

		g_print ( "tag = %s\n", p->data );

		/*
				if ( gtk_text_tag_table_lookup ( table, p->data ) != NULL )
				{
					mark = gtk_text_buffer_get_insert ( GTK_TEXT_BUFFER ( buffer) );
					gtk_text_buffer_get_iter_at_mark ( GTK_TEXT_BUFFER ( buffer), iter2, mark );
					gtk_text_buffer_apply_tag_by_name  ( GTK_TEXT_BUFFER ( buffer), p->data, iter2, iter2);

				}
		*/

	}

}


