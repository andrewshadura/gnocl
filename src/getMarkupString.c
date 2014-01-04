#include "gnocl.h"
//#include "gnoclparams.h"

/**
\brief	Return text with Pango markup
\notes	Pango has a weight markup format compared to HTML or the textBuff
		Basically, it means there are problems when overlapping markup tags.
		Whatever is opened has to be closed, and then the overlap re-opened.

		eg: this will work
		The buffer will return this..
		<b>aaa<i> bbb</b> <u>ccc</u></i> ddd</u>
		But this is pango!
		<b>aaa<i> bbb</i></b><i> <u>ccc</u></i><u> ddd</u>
\date	23/06/13
\notes	Problems occur whenever 3 tags are active.
e.g.
Lorem <b><i><u>ips</b></i></u>u<u><i><b>m</u></i></b> dol
                  ^
Error on line 1 char 30: Element 'b' was closed, but the currently open element is 'u'
**/
Tcl_Obj *getMarkUpString ( Tcl_Interp *interp, GtkTextBuffer *buffer, GtkTextIter *start, GtkTextIter *end )
{
#if 1
	g_print ( "%s\n", __FUNCTION__ );
#endif

	Tcl_Obj *res;

	GtkTextIter *iter;
	gunichar ch;
	GList *q;
	GList *onList = NULL, *offList = NULL, *revList = NULL;
	gchar *tagName = NULL;

	res = Tcl_NewStringObj ( "", 0 );
	iter = gtk_text_iter_copy ( start );

	/* parse each position in the selection */
	while ( gtk_text_iter_equal ( iter, end ) == 0 )
	{

		/* process tagOff before any subsequent tagOn */

		offList = gtk_text_iter_get_toggled_tags ( iter, 0 );
		onList = gtk_text_iter_get_toggled_tags ( iter, 1 );

		/* handle tags toggled on */
		if ( onList != NULL )
		{
			/* get a reverse list */
			for ( q = onList ; q != NULL; q = q->next )
			{
				tagName = ( GTK_TEXT_TAG ( q->data )->name );
				Tcl_AppendStringsToObj ( res, tagName, ( char * ) NULL );
			}
		}

		/* handle tags toggled off */
		if ( offList != NULL )
		{
			/* temporarily turn-off all active tags */
			onList = gtk_text_iter_get_tags ( iter );

			for ( q = onList ; q != NULL; q = q->next )
			{
				tagName = ( GTK_TEXT_TAG ( q->data )->name );
				//revList = g_slist_prepend ( revList, tagName );
				revList = g_slist_append ( revList, tagName );
			}

			for ( q = revList ; q != NULL; q = q->next )
			{
				tagName = q->data ;

				if ( strncmp ( tagName, "<span", 5 ) == 0 )
				{
					Tcl_AppendStringsToObj ( res, "</span>" , ( char * ) NULL );
				}

				else
				{

					Tcl_AppendStringsToObj ( res, str_replace ( tagName, "<", "</" ), ( char * ) NULL );
				}
			}

			/* terminate turned-off tags */
			for ( q = offList ; q != NULL; q = q->next )
			{
				tagName = ( GTK_TEXT_TAG ( q->data )->name );

				if ( strncmp ( tagName, "<span", 5 ) == 0 )
				{
					Tcl_AppendStringsToObj ( res, "</span>" , ( char * ) NULL );
				}

				else
				{

					Tcl_AppendStringsToObj ( res, str_replace ( tagName, "<", "</" ), ( char * ) NULL );
				}
			}

			/* re-activate tags turned on */
			for ( q = onList ; q != NULL; q = q->next )
			{
				tagName = ( GTK_TEXT_TAG ( q->data )->name );
				Tcl_AppendStringsToObj ( res, tagName, ( char * ) NULL );
			}

			/* free up memory */
			g_slist_free ( onList );  onList = NULL;
			g_slist_free ( revList ); revList = NULL;
			g_slist_free ( offList ); offList = NULL;

		}

		/* append chracter at position */
		ch = gtk_text_iter_get_char ( iter );
		Tcl_AppendStringsToObj ( res, &ch, ( char * ) NULL );

		/* move onto next position in text */
		gtk_text_iter_forward_cursor_position ( iter ); //OK
	}

	/* terminate any active tags at end of line */
	if ( gtk_text_iter_backward_to_tag_toggle ( iter, NULL ) )
	{
		onList = gtk_text_iter_get_tags ( iter );

		for ( q = onList ; q != NULL; q = q->next )
		{
			tagName = ( GTK_TEXT_TAG ( q->data )->name );
			revList = g_slist_prepend ( revList, tagName );
		}


		for ( q = revList ; q != NULL; q = q->next )
		{
			tagName = q->data ;

			if ( strncmp ( tagName, "<span", 5 ) == 0 )
			{
				Tcl_AppendStringsToObj ( res, "</span>" , ( char * ) NULL );
			}

			else
			{

				Tcl_AppendStringsToObj ( res, str_replace ( tagName, "<", "</" ), ( char * ) NULL );
			}
		}
	}

	gtk_text_iter_free ( iter );

	g_slist_free ( onList );  onList = NULL;
	g_slist_free ( revList ); revList = NULL;
	g_slist_free ( offList ); offList = NULL;

#ifdef DEBUG_TEXT
	g_print ( "done!\n" );
#endif

	gchar *text = NULL;
	GError *err = NULL;

	if (  pango_parse_markup ( Tcl_GetStringFromObj ( res, NULL ), -1 , NULL, NULL, NULL, NULL, &err ) == 0 )
	{
		g_print ( "=====================================================!\n" );
		g_print ( "WARNING! Malformed Pango Strings:\n\n%s\n\n%s\n", Tcl_GetStringFromObj ( res, NULL ), err->message  );
		g_print ( "=====================================================!\n" );
		Tcl_SetStringObj ( res, "", 0 );
		Tcl_AppendStringsToObj ( res, gtk_text_buffer_get_text ( buffer, start, end, 0 ), ( char * ) NULL );

	}

	return res;
}

