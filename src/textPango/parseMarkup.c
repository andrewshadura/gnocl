/*
 *  parseMarkup.c
 *
 *  See the file "license.terms" for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

/*
    History:

    2011-09  began module development

*/

#include <gtk/gtk.h>
#include <stdio.h>
#include <string.h>

/**
\brief  delete substring
        http://www.timeflash.net/tutorials/cc/16-how-to-remove-a-substring-from-a-string-in-c-and-c.html
**/
char* strdstr ( char* str, char* substr )
{
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


		++ list;
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
int gnoclParseMarkup ( GtkTextBuffer *buffer, GtkTextIter *iter, char *markup_text )
{

	/* interate through pango string, determine, text or markup */
	int iter, start, end;
	start = 0;
	end = strlen ( markup_text );

	const char tag[256];
	char txt[end];

	const char *tags = " ";

	iter = start;

	while ( iter <= end )
	{
		iter = getTag ( markup_text, tag, iter );
		g_print ( "tag  = '%s'\n", tag );

		/* tagoff? */
		if ( strstr ( tag, "/" ) != NULL  )
		{
			g_print ( "tagOff %s\n", tag );
			/* remove matching tag from the list */
			//strip_chars ( tag, "/" );
			strip_chars ( tags, tag );
		}

		else
		{
			g_print ( "tagOn %s\n", tag );
			/* add new tag to the list */
			tags = sconcat ( tags, tag );
			tags = sconcat ( tags, " " );
		}

		if ( iter == end ) break; /* prevent error */

		iter = getText ( markup_text, txt, iter );
		g_print ( "text = '%s'\n", txt );

		gtk_text_buffer_insert_with_tags_by_name ( buffer, &iter, gnoclGetString ( objv[cmdNo+1] ), -1, "b i" );

	}

	g_print ( "taglist= %s\n", tags );

	return 0;
}
