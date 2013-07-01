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

#include "gnocl.h"exist

/* list to hold names of applied tags */
static GSList *tagList = NULL;


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

/* removes all chars from string */
static char * _strip_chars ( const char * string, const char * chars )
{

	char * newstr = malloc ( strlen ( string ) );
	int counter = 0;

	while ( *string )
	{
		if ( contains ( *string, chars ) != 1 )
		{
			newstr[ counter ] = *string;
			++ counter;
		}

		++ string;
	}

	return newstr;
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
		dest[j++] = src[i];

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
**/
static int _getTag ( char *str, char *tag, int i )
{

	int a, b;

	a = strnfrst ( str, "</", i );

	if ( a )
		a = strnfrst ( str, "<", i );

	b = strnfrst ( str, ">", a + 1 );

	strrng ( tag, str, a, b + 1 );

	return a;
}


/**
    gettag
    get name of next tag in string, starting at position i
    return updated value of i

    test-string = "<b>bold</b>   <i>italic</i>   <u>underline</u>   <s>strikethrough</s>"

**/
static int getTag ( char *str, char *tag, int i )
{

	int j;

	if ( strcmp ( str + i, "<" ) )
	{
		/* handle a tag */
		j = strnfrst ( str, ">", i );
		//strrng ( tag, str, ++i, j );
		strrng ( tag, str, i, ++j );
		return j;
	}

	return ++i;
}


/**
\brief
**/
static int getText ( char *str, char *txt, int i )
{

	int j;

	j = strnfrst ( str, "<", i );
	strrng ( txt, str, i, j );
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
\brief	Remove tag from list of active tags.
**/
void removeTag ( const char *tag )
{
	GSList *p = NULL;

#ifdef DEBUG_PANGO_MARKUP
	g_print ( "FUNC: %s\n", __FUNCTION__ );
#endif

	int i, j, k;
	i = 0;
	j = 0;
	k = strlen ( tag );

	gchar str[k];

	g_print ( "\t1 %d\n", k );

	//for ( i = 0; i < k; i++ )
	while ( i < k )
	{
		if ( tag[i] != '/' )
		{
			str[j++] = tag[i++];
		}

		else
		{
			i++;
		}

	}

	str[j] = '\0';

	g_print ( "\ttag = %s\n", str );

	g_print ( "\t2\n" );



	/* iterate through tagList */

	for ( p = ( GSList * ) tagList; p != NULL; p = p->next )
	{
		g_print ( "\t iterate tag = %s\n", p->data );

	}


}




/**
\brief
**/
int _gnoclInsertMarkup ( GtkTextBuffer *buffer, GtkTextIter *iter, char *markup_text )
{
#ifdef DEBUG_PANGO_MARKUP
	g_print ( "FUNC: %s\n", __FUNCTION__ );
#endif

	/* interate through pango string, determine, text or markup */
	int i, start, end;
	start = 0;
	end = strlen ( markup_text );

	const char tag[256];
	char txt[end];

	const char *tags = " ";

	i = start;

	while ( *markup_text != '\0' )
	{
		g_print ( "========== %c\n" , *markup_text );

		if ( *markup_text == '<' )
		{
			g_print ( "tag <\n" , *markup_text );
			//getTag ( char *str, char *tag, int i )
		}

		if ( *markup_text == '>' )
		{
			g_print ( "tag >\n" , *markup_text );
		}

		++markup_text;
	}

	g_print ( "==========\n" );
	return 0;
}



/**
\brief
\todo	test the legality of the markup string.
		swap &gtl; for ">" etc..
**/
int gnoclInsertMarkup ( GtkTextBuffer *buffer, GtkTextIter *iter, char *markup_text )
{
#ifdef DEBUG_PANGO_MARKUP
	g_print ( "FUNC: %s\n", __FUNCTION__ );
#endif

	/* interate through pango string, determine, text or markup */
	int i, j, start, end;
	start = 0;


	end = strlen ( markup_text );


	const char tag[256];
	char txt[end];

	const char *tags = " ";

	i = start;

	while ( i < end )
	{
		i = getTag ( markup_text, tag, i );

		g_print ( "i = %d ; onTag = %s\n", i, tag );


#if 1

		/* add or remove tag from taglist */
		if ( strstr ( tag, "/" ) != NULL  )
		{
			g_print ( "\ttagOff %s\n", tag );


			/* remove matching tag from the list */
			//strip_chars ( tags, tag );
			removeTag ( tag );

		}

		else
		{
			g_print ( "\ttagOn %s\n", tag );
			/* add new tag to the list */
			//tags = sconcat ( tags, tag );
			//tags = sconcat ( tags, " " );

			/* Notice that these are initialized to the empty list. */

			if ( g_slist_find ( tagList, tag ) != NULL )
			{
				g_print ( "tag %s already exists!\n", tag );
			}

			else
			{
				g_print ( "adding tag %sn", tag );
				/* This is a list of strings. */
				tagList = g_slist_prepend ( tagList, tag );
			}

		}

#endif

		if ( i == end )
			break; /* prevent error */

		i = getText ( markup_text, txt, i );
		g_print ( "\ttext = %s\n", txt );

		GtkTextMark *tagStart, *tagEnd;
		GtkTextIter start, end;

		tagStart  = gtk_text_buffer_create_mark ( buffer, "tagStart", iter, 1 );

		gtk_text_buffer_insert  ( buffer, iter, txt, -1 );

		tagEnd = gtk_text_buffer_get_insert ( buffer );

		applyTags ( buffer, tagStart, tagEnd );


	}

	//g_print ( "taglist= %s\n", tags );	gchar tmp[32];
	//strcinsert ( "helloword", tmp, "~~~", 4 );
	//g_print ( "mytest %s\n", tmp );


	/* free memory */
	//g_slist_free (tagList);

	g_print ( "==========\n" );
	return 0;
}

/**
**/
void applyTags ( GtkTextBuffer *buffer, GtkTextMark *tagStart, GtkTextMark *tagEnd )
{
#ifdef DEBUG_PANGO_MARKUP
	g_print ( "FUNC: %s\n", __FUNCTION__ );
#endif
	GtkTextIter start, end;
	GSList *p = NULL;
	gint i;

	GtkTextTagTable *table;

	table = gtk_text_buffer_get_tag_table ( buffer );

	i = 1;

	gtk_text_buffer_get_iter_at_mark ( buffer, &start, tagStart );
	gtk_text_buffer_get_iter_at_mark ( buffer, &end, tagEnd );
	//gtk_text_buffer_apply_tag_by_name  (buffer,tag, &start,&end);

	/* iterate through tagList */
	for ( p = ( GSList * ) tagList; p != NULL; p = p->next )
	{
		g_print ( "\ttag = %s (%d)\n", p->data, i++ );

		if ( gtk_text_tag_table_lookup ( table, p->data ) != NULL )
		{
			gtk_text_buffer_apply_tag_by_name  ( buffer, p->data, &start, &end );
		}
	}

	g_print ( "----------\n" );
}
