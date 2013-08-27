/*
 *  parseMarkup.c
 *
 *  See the file "license.terms" for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

/*
    The GtkTextBuffer has no default tags, whereas the Pango description requires
    a conventionalized set of descriptors. In order to guareentee correct display
    and editing of markupstrings, a protected set of markup tags need to created.
    Following this, any markup strings to be inserted can be added with safety.


*/


/*
    History:

    2011-09  began module development

*/

#include "gnocl.h"

/* list to hold names of applied tags */
static GSList *tagList = NULL;


/* remove range of characters from string */
char * strcremove ( char *str, char *dest, int first, int last )
{

	g_print ( "%s 1 first = %d ; last = %d\n", __FUNCTION__, first, last );

	int j = 0;
	char *ptr;
	ptr = str;

	while ( j < strlen ( str ) )
	{
		g_print ( "j = %d\n", j );

		if ( j >= first && j <= last )
		{
			g_print ( "remove %c\n", *str + j );
			//*str++;
		}

		else
		{
			//*dest++ =
			//*ptr++;
		}

		j++;

	}

	*dest = '/0';

	return dest;
}


/* insert character in string, return new string */
char * strcinsert ( char *str, char *dest, char *c, int i )
{

	//g_print ( "%s 1\n", __FUNCTION__ );

	int j = 0;

	while ( j < i )
	{
		*dest++ = *str++;
		j++;
	}

	//g_print ( "%s 2\n", __FUNCTION__ );

	*dest++ = c;

	//g_print ( "%s 3\n", __FUNCTION__ );

	while ( j < strlen ( str ) )
	{
		*dest++ = *str++;
		j++;
	}

	//g_print ( "%s 4\n", __FUNCTION__ );

	*dest = '/0';

	//g_print ( "%s 5\n", __FUNCTION__ );

	return dest;
}


/**
\brief  delete substring
        http://www.timeflash.net/tutorials/cc/16-how-to-remove-a-substring-from-a-string-in-c-and-c.html
**/
char* strdstr ( char* str, char* substr )
{
#ifdef DEBUG_PANGO_MARKUP
	g_print ( "FUNC: %s\n", __FUNCTION__ );
#endif


	int i, j, p, match;

	char newstr [strlen ( str ) ];

	for ( i = 0 ; i < strlen ( str ) ; ++i )
	{
		if ( str[i] == substr[j] )
		{
			match = 1;

			for ( j = 1; j < strlen ( substr ); ++j )
			{
				if ( str[i+j] != substr[j] )
				{
					j = 0;
					match = 0;
				}
			}

			if ( match )
			{
				i += strlen ( substr ) - 1;
				j = 0;
			}
		}

		else
		{
			newstr[p] = str[i];
			++p;
		}
	}

	newstr[p] = '\0';

	return newstr;
}


/**
    String concatenation
    http://rosettacode.org/wiki/String_concatenation#C
**/
static char *sconcat ( const char *s1, const char *s2 )
{
	char *s0 = ( char * ) malloc ( strlen ( s1 ) + strlen ( s2 ) + 1 );
	strcpy ( s0, s1 );
	strcat ( s0, s2 );
	return s0;
}

/**
    Strip a set of characters from a string
    http://rosettacode.org/wiki/Strip_a_set_of_characters_from_a_string#C
    done
**/
/* checks if character exists in list */
static int contains ( char character, char * list )
{
	while ( *list )
	{
		if ( character == *list )
			return 1;

		++list;
	}

	return 0;
}



char * removeChar ( char *str, char garbage )
{

#if 1
	g_print ( "FUNC: %s\n", __FUNCTION__ );
#endif

	char *src, *dst;

	for ( src = dst = str; *src != '\0'; src++ )
	{
		*dst = *src;

		if ( *dst != garbage ) dst++;
	}

	*dst = '\0';

	g_print ( "%s\n", dst );

	return dst;
}


/**
\brief
**/
static int strip_chars ( char * string, char * chars )
{

	int i = 0;
	int j = strlen ( string );
	int l = strlen ( chars );

	char newstr[strlen ( string ) ];
	int c = 0;
	int k = 0;

	strcpy ( newstr, string );

	char *f;
	f = string;

	/* search through string till match found */
	while ( i < j )
	{

		/* check for chars, in string */
		if ( strncmp ( f + i, chars, l ) != -1 )
		{
			g_print ( "got tag %s\n", chars );
			i += l - 1;
			/* remove matching tag */
		}

		else
		{
			/*
			newstr[c] = string[i];
			g_print ( "plain text %c\n", newstr[c] );
			newstr[c+1] = '\0';
			g_print ( "      string = %s\n", newstr );
			*/
			c++;
			i++;

		}

	}

	g_print ( "string = %s : newstr = %s\n", string, newstr );

	strcpy ( string, newstr );

	string[j] = '\0';

	return c;
}

/**
    search for first occurance of p in s, starting from i
    done
**/
static int strnfrst ( char *s, char *p, int i )
{
	char *f;
	int l;

	l = strlen ( p ); /* length of search string */
	f = s + i;

	/* search through string till match found */
	while ( *f != '\0' )
	{
		if ( !strncmp ( f, p, l ) )
			return f - s;

		f++;
	}

	return -1;
}




/**
    extract a range of characters from string s starting from position a to position b
    done
**/
static char *strrng ( char *dest, const char *src, int a, int b )
{
	unsigned i, j;

	j = 0;

	for ( i = a; i < b; i++ )
	{
		dest[j++] = src[i];
	}

	dest[j] = '\0';
	return dest;

}

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
    gettag
    get name of next tag in string, starting at position i
    return updated value of i

    test-string = "<b>bold</b>   <i>italic</i>   <u>underline</u>   <s>strikethrough</s>"

**/
static int getTag ( char *str, char *tag, int i )
{
#if 1
	g_print ( "%s\n", __FUNCTION__ );
#endif
	int j;

	if ( strcmp ( str , "<" ) )
	{
		/* handle a tag */
		j = strnfrst ( str, ">", i );
		//strrng ( tag, str, ++i, j );
		strrng ( tag, str, i, j+1  );
		return j;
	}

	return i;
}


/**
\brief
**/
static int getText ( char *str, char *txt, int i )
{

	int j;
	g_print ( "%s 1 ", __FUNCTION__ );
	j = strnfrst ( str, "<", i );
	g_print ( "2 %d", j );

	if ( j < 0 )
	{
		j = strlen ( str );

		g_print ( "3 %d", j );
		strrng ( txt, str, i, strlen ( str ) );

	}

	else
	{
		strrng ( txt, str, i, j );
	}

	g_print ( "4 " );
	return j;
}

/**
\brief
**/
int gnoclMarkupInsertTest ( GtkTextBuffer *buffer, GtkTextIter *iter )
{
	g_print ( "FUNC: %s\n", __FUNCTION__ );

	gint i;

	for ( i = 0 ; i < 100; i++ )
		gtk_text_buffer_insert_with_tags_by_name ( buffer, iter, "H ", -1, "b", "i" );

}

/**
\brief	Add tag from list of active tags.
**/
void addTag ( gchar *tag )
{
#if 1
	g_print ( "FUNC: %s\n", __FUNCTION__ );
#endif

	if ( g_slist_find ( tagList, tag ) == NULL )
	{
		g_print ( "\tAdded %s\n", tag );
		tagList = g_slist_prepend ( tagList, tag );
		g_print ( "list length = %d\n", g_slist_length ( tagList ) );
	}
}

/**
\brief	Remove tag from list of active tags.
**/
void removeTag ( gchar *tag )
{
#if 1
	g_print ( "FUNC: %s\n", __FUNCTION__ );
#endif

	gchar *tmp;
	tmp = tag;

	tagList = g_slist_remove_all  ( tagList, tmp );
	tmp= str_replace ( tag, "/", "" );

	g_print ( "\tRemoved %s\n", tmp );
	tagList = g_slist_remove_all  ( tagList, tmp );
	g_print ( "list length = %d\n", g_slist_length ( tagList ) );

}




/**
\brief
**/
void handleTag ( gchar tag[] )
{
#if 1
	g_print ( "FUNC: %s %s\n", __FUNCTION__, tag );
#endif


	if ( tag[1] == '/' )
	{
		g_print ( "\tremove tag\n" );
		removeTag ( tag );
	}

	else
	{
		g_print ( "\tadd tag\n" );
		addTag ( tag );
	}


}

/**
\brief
\todo	test the legality of the markup string.
		swap &gtl; for ">" etc..
**/
int gnoclInsertMarkup ( GtkTextBuffer *buffer, GtkTextIter *iter, gchar markup_text[] )
{
#if 1
	g_print ( "FUNC: %s\n", __FUNCTION__ );
#endif

	GtkTextMark *tagStart, *tagEnd;
	GSList *lptr = NULL;
GtkTextIter *tagIter;

	/* interate through pango string, determine, text or markup */
	int i, j;


	const char tag[256];
	char txt[10];
	gchar *ch1, *ch2;

	//gchar tmp[strlen ( markup_text ) ];
	//sprintf ( tmp, "%s", markup_text );


	i = 0;

	/* initialize the text */
	while ( i < strlen ( markup_text ) )
	{
		//ch1 = tmp[i];
		//ch2 = tmp[i+1];

		if ( markup_text[i] == '<' )
		{
			g_print ("1-1 %d\n",i);
			i = getTag ( markup_text, tag, i );
			handleTag ( tag );
			g_print ("1-2 %d\n",i);
		}

		else
		{
			g_print ("2\n");
			//sprintf ( txt, "%c", ch1 );
			ch2 = &markup_text[i];
			tagIter = gtk_text_iter_copy (iter);
			
			//applyTags ( buffer,  iter );
			gtk_text_buffer_insert  ( buffer, iter, &markup_text[i], 1 );

			//gtk_text_iter_forward_char (iter);

			/* problems occuring here */
			applyTags ( buffer,  iter );

			g_print ( "ch = %c\n",  markup_text[i] );

		}

		i++;

	for ( lptr = ( GSList * ) tagList; lptr != NULL; lptr = lptr->next )
	{
		g_print ("tagList\n-----tag = %s\n-----\n",lptr->data);
	}



	}

	/* free memory */
	//g_slist_free ( tagList );

	


	g_print ( "==========\n" );
	return 0;
}

/**
**/
void applyTags ( GtkTextBuffer *buffer, GtkTextIter *iter )
{
#if 0
	g_print ( "FUNC: %s\n", __FUNCTION__ );
#endif
	GtkTextIter *end;
	GSList *p = NULL;
	gint i;

	GtkTextTagTable *table;

	table = gtk_text_buffer_get_tag_table ( buffer );

	end =  gtk_text_iter_copy ( iter );
	gtk_text_iter_set_offset  ( end, 1 );

	i = 1;

	/* iterate through tagList */
	for ( p = ( GSList * ) tagList; p != NULL; p = p->next )
	{


		if ( gtk_text_tag_table_lookup ( table, p->data ) != NULL )
		{

			if ( strncmp ( p->data, "</", 2 ) != 0 )
			{
				g_print ( "\t applying tag = %s (%d)\n", p->data, i++ );
				gtk_text_buffer_apply_tag_by_name  ( buffer, p->data, iter, end );
			}
		}
	}

}

