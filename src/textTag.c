/*
 *  $Id: text.c,v 1.7 2005-01-01 15:27:54 baum Exp $
 *
 *  This file implements the text widget
 *
 *  Copyright (c) 2001 - 2005 Peter G. Baum  http://www.dr-baum.net
 *
 *  See the file "license.terms" for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

/**
\page page_text gnocl::text
\htmlinclude text.html
**/

#include "gnocl.h"

gint usemarkup = 0;


/**
\brief	Return text width Pango markup
**/
Tcl_Obj *gnoclGetToggledTags ( Tcl_Interp *interp, GtkTextBuffer *buffer, GtkTextIter *start, GtkTextIter *end )
{
#ifdef DEBUG_TEXT
	g_print ( "%s\n", __FUNCTION__ );
#endif

	Tcl_Obj *res;
	gchar *txt = NULL;

	GtkTextIter *iter;
	gunichar ch;
	GSList *p, *onList, *offList;

	gchar *tagName;

	//txt = gtk_text_buffer_get_text ( buffer, start, end, 1 );

	//res = Tcl_NewStringObj ( txt, -1 );
	//g_free ( txt );

	res = Tcl_NewStringObj ( "", 0 );

	iter = gtk_text_iter_copy ( start );

	while ( gtk_text_iter_equal ( iter, end ) != 1 )
	{
		ch = gtk_text_iter_get_char ( iter );

		onList = gtk_text_iter_get_toggled_tags ( iter, 1 );
		offList = gtk_text_iter_get_toggled_tags ( iter, 0 );

		for ( p = offList; p != NULL; p = p->next )
		{
			tagName = ( GTK_TEXT_TAG ( p->data )->name );
			//g_print ( "off = %s\n", tagName );
			//Tcl_AppendStringsToObj ( res, "</", tagName, ">", ( char * ) NULL );
		}


		for ( p = onList; p != NULL; p = p->next )
		{
			tagName = ( GTK_TEXT_TAG ( p->data )->name );
			//g_print ( "on = %s\n", tagName );
			//Tcl_AppendStringsToObj ( res, "<", tagName, ">", ( char * ) NULL );
		}

		//g_print ( "%c\n", ch );
		//Tcl_AppendStringsToObj ( res, &ch, ( char * ) NULL );

		gtk_text_iter_forward_char ( iter );
	}

	gtk_text_iter_free ( iter );

	return res;
}

/**
\brief	Set text tag attribute.
**/
int gnoclOptTagBackgroundStipple ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	GtkWidget *container;
	container =  gtk_widget_get_parent ( obj );

#ifdef DEBUG_TEXT
	debugStep ( __FUNCTION__, 1.0 );
	g_print ( "Feature not yet implemented\n" );
#endif

	return TCL_OK;
}

/**
\brief	Set text tag attribute.
**/
int gnoclOptTagTextDirection ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{

#ifdef DEBUG_TEXT
	debugStep ( __FUNCTION__, 1.0 );
#endif

	/* options: leftRight | rightLeft | none */

	/*
	typedef enum
	{
	  GTK_TEXT_DIR_NONE,
	  GTK_TEXT_DIR_LTR,
	  GTK_TEXT_DIR_RTL,
	} GtkTextDirection;
	*/

	int idx;

	const char *txt[] = { "none", "leftRight", "rightLeft", NULL };

	if ( Tcl_GetIndexFromObj ( NULL, opt->val.obj, txt, NULL, TCL_EXACT, &idx ) != TCL_OK )
	{
		Tcl_AppendResult ( interp, "Unknown direction \"", Tcl_GetString ( opt->val.obj ), "\". Must be one of none, leftRight or rightLeft.", NULL );
		return TCL_ERROR;
	}

	g_object_set ( obj, opt->propName, idx, NULL );

	return TCL_OK;
}

/**
\brief	Set text tag attribute.
**/
int gnoclOptTextTagForegroundStipple ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	GtkWidget *container;
	container =  gtk_widget_get_parent ( obj );

#ifdef DEBUG_TEXT
	debugStep ( __FUNCTION__, 1.0 );
	g_print ( "Feature not yet implemented\n" );
#endif

	return TCL_OK;
}

/**
\brief	Set text tag attribute.
**/
int gnoclOptTextTagPriority ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{

#ifdef DEBUG_TEXT
	debugStep ( __FUNCTION__, 1.0 );
#endif

	gint priority;

	Tcl_GetIntFromObj ( interp, opt->val.obj, &priority );


	gtk_text_tag_set_priority ( GTK_TEXT_TAG ( obj ), priority );


	return TCL_OK;
}

/**
\brief	Add default set of tag with pango compliant tagnames.
**/
int gnoclOptMarkupTags ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
#ifdef DEBUG_TEXT
	g_print ( "%s %d\n", __FUNCTION__, Tcl_GetString ( opt->val.obj ) );
#endif

	extern gint usemarkup;

	assert ( strcmp ( opt->optName, "-markupTags" ) == 0 );

	/* modify this to destroy tags */
	if ( strcmp ( Tcl_GetString ( opt->val.obj ), "1" ) == 0 )
	{
		usemarkup = 1;
		/* create default markup tag set */
	}

	else
	{
		usemarkup = 1;
		/* delete markup tags */
	}

#ifdef DEBUG_TEXT
	g_print ( "usemarkup = %d\n", usemarkup );
#endif

	GtkTextBuffer *buffer = gtk_text_view_get_buffer ( GTK_TEXT_VIEW ( obj ) );

	gtk_text_buffer_create_tag ( buffer, "b", "weight", PANGO_WEIGHT_BOLD, NULL ); //bold

	//GtkTextTag *tag gtk_text_tag_new (b);

	//g_object_set ( tag, "weight", PANGO_WEIGHT_BOLD, NULL );

	gtk_text_buffer_create_tag ( buffer, "i", "style", PANGO_STYLE_ITALIC, NULL ); //italic
	gtk_text_buffer_create_tag ( buffer, "s", "strikethrough", 1, NULL ); //strikethrough
	gtk_text_buffer_create_tag ( buffer, "u", "underline", PANGO_UNDERLINE_SINGLE, NULL ); //underline

	/*
	gtk_text_buffer_create_tag (buffer, "sub","font", font, NULL);  //bold
	gtk_text_buffer_create_tag (buffer, "sup","font", font, NULL);  //bold
	gtk_text_buffer_create_tag (buffer, "small","font", font, NULL);  //bold
	*/
	gtk_text_buffer_create_tag ( buffer, "tt", "font", "Monospace", NULL ); //tt

	/*
		gtk_text_buffer_create_tag ( buffer, "fg=red", "underline", PANGO_UNDERLINE_SINGLE, NULL ); //underline
		gtk_text_buffer_create_tag ( buffer, "fg=green", "underline", PANGO_UNDERLINE_SINGLE, NULL ); //underline
		gtk_text_buffer_create_tag ( buffer, "fg=blue", "underline", PANGO_UNDERLINE_SINGLE, NULL ); //underline
		gtk_text_buffer_create_tag ( buffer, "fg=cyan", "underline", PANGO_UNDERLINE_SINGLE, NULL ); //underline
		gtk_text_buffer_create_tag ( buffer, "fg=magenta", "underline", PANGO_UNDERLINE_SINGLE, NULL ); //underline
		gtk_text_buffer_create_tag ( buffer, "fg=yellow", "underline", PANGO_UNDERLINE_SINGLE, NULL ); //underline
		gtk_text_buffer_create_tag ( buffer, "fg=gray", "underline", PANGO_UNDERLINE_SINGLE, NULL ); //underline
		gtk_text_buffer_create_tag ( buffer, "fg=black", "underline", PANGO_UNDERLINE_SINGLE, NULL ); //underline
		gtk_text_buffer_create_tag ( buffer, "fg=white", "underline", PANGO_UNDERLINE_SINGLE, NULL ); //underline

		gtk_text_buffer_create_tag ( buffer, "bg=red", "underline", PANGO_UNDERLINE_SINGLE, NULL ); //underline
		gtk_text_buffer_create_tag ( buffer, "bg=green", "underline", PANGO_UNDERLINE_SINGLE, NULL ); //underline
		gtk_text_buffer_create_tag ( buffer, "bg=blue", "underline", PANGO_UNDERLINE_SINGLE, NULL ); //underline
		gtk_text_buffer_create_tag ( buffer, "bg=cyan", "underline", PANGO_UNDERLINE_SINGLE, NULL ); //underline
		gtk_text_buffer_create_tag ( buffer, "bg=magenta", "underline", PANGO_UNDERLINE_SINGLE, NULL ); //underline
		gtk_text_buffer_create_tag ( buffer, "bg=yellow", "underline", PANGO_UNDERLINE_SINGLE, NULL ); //underline
		gtk_text_buffer_create_tag ( buffer, "bg=gray", "underline", PANGO_UNDERLINE_SINGLE, NULL ); //underline
		gtk_text_buffer_create_tag ( buffer, "bg=black", "underline", PANGO_UNDERLINE_SINGLE, NULL ); //underline
		gtk_text_buffer_create_tag ( buffer, "bg=white", "underline", PANGO_UNDERLINE_SINGLE, NULL ); //underline
	*/

	return TCL_OK;

}


/**
\brief	Set text tag attribute.
**/
int gnocOptTextTagLanguage ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{

	gchar *lang;
	gint i;

	lang = Tcl_GetStringFromObj ( opt->val.obj, NULL );

#ifdef DEBUG_TEXT
	debugStep ( __FUNCTION__, 1.0 );
	g_print ( "Language = %s\n", lang );
#endif

	i = getLanguage ( lang );

	g_print ( "idx = %d\n", i );


	return TCL_OK;
}

/**
\brief	Set text tag attribute.
**/
int gnoclOptTextTagTabs ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
#ifdef DEBUG_TEXT
	debugStep ( __FUNCTION__, 1.0 );
	g_print ( "Feature not yet implemented\n" );
#endif

	return TCL_OK;
}


/**
\brief	Return text width Pango markup
**/
Tcl_Obj *gnoclGetMarkUpString ( Tcl_Interp *interp, GtkTextBuffer *buffer, GtkTextIter *start, GtkTextIter *end )
{
#ifdef DEBUG_TEXT
	g_print ( "%s usemarkup = %d\n", __FUNCTION__, usemarkup );
#endif

	Tcl_Obj *res;
	gchar *txt = NULL;

	GtkTextIter *iter;
	gunichar ch;
	GSList *p, *onList, *offList;

	gchar *tagName;

	txt = gtk_text_buffer_get_text ( buffer, start, end, 1 );

	//res = Tcl_NewStringObj ( txt, -1 );
	//g_free ( txt );

	res = Tcl_NewStringObj ( "", 0 );

	iter = gtk_text_iter_copy ( start );

	while ( gtk_text_iter_equal ( iter, end ) != 1 )
	{
		ch = gtk_text_iter_get_char ( iter );

		onList = gtk_text_iter_get_toggled_tags ( iter, 1 );
		offList = gtk_text_iter_get_toggled_tags ( iter, 0 );

		for ( p = offList; p != NULL; p = p->next )
		{
			tagName = ( GTK_TEXT_TAG ( p->data )->name );
			g_print ( "off = %s\n", tagName );
			Tcl_AppendStringsToObj ( res, "</", tagName, ">", ( char * ) NULL );
		}


		for ( p = onList; p != NULL; p = p->next )
		{
			tagName = ( GTK_TEXT_TAG ( p->data )->name );
			g_print ( "on = %s\n", tagName );
			Tcl_AppendStringsToObj ( res, "<", tagName, ">", ( char * ) NULL );
		}

		g_print ( "%c\n", ch );
		Tcl_AppendStringsToObj ( res, &ch, ( char * ) NULL );

		gtk_text_iter_forward_char ( iter );
	}

	gtk_text_iter_free ( iter );


#ifdef DEBUG_TEXT
	g_print ( "done!\n" );
#endif

	return res;
}

/**
\brief	Add tagname to a Tcl list.
**/
void gnoclGetTagName ( GtkTextTag *tag, gpointer data )
{
#ifdef DEBUG_TEXT
	g_print ( "%s %s\n", __FUNCTION__, tag->name );
#endif

	Tcl_Obj **resList = data;

	Tcl_ListObjAppendElement ( NULL, *resList, Tcl_NewStringObj ( tag->name, -1 ) );

}


/**
\brief	Return a list of all occuranances of a named tag within a buffer
**/
void gnoclGetTagRanges ( GtkTextBuffer *buffer, gchar *tagName )
{
#ifdef DEBUG_TEXT
	g_print ( "%s %s\n", __FUNCTION__, tagName );
#endif


	GtkTextIter iter;
	GtkTextTag *tag;
	GtkTextTagTable *table;
	gint cc;

	gtk_text_buffer_get_start_iter ( buffer, &iter );
	table = gtk_text_buffer_get_tag_table ( buffer );
	tag = gtk_text_tag_table_lookup ( table, tagName );


	while ( ( gtk_text_iter_forward_to_tag_toggle ( &iter, tag ) ) == TRUE )
	{
		cc = gtk_text_iter_get_offset ( &iter );
		printf ( "cc: %d\n", cc );
	}
}

/**
\brief      Return list of tag settings and their values in the format
            { tagName1 {attribute-list} tagName2 {attribute-list}..  }
\note       1) Only returns those attribute which have been set away from their default values.
            Getting a handle on the various tag parameters has been a process of trial and error.
            An alternative way of getting the colour values but this time in hex is:
            GdkColor *bg_color2  = &tag_appearance->bg_color;
            char *clr = gdk_color_to_string ( bg_color2 );
            sprintf ( tmp2, "clr = %s ",clr);


  guint GSEAL (bg_color_set) : 1;
  guint GSEAL (bg_stipple_set) : 1;
  guint GSEAL (fg_color_set) : 1;
  guint GSEAL (scale_set) : 1;
  guint GSEAL (fg_stipple_set) : 1;
  guint GSEAL (justification_set) : 1;
  guint GSEAL (left_margin_set) : 1;
  guint GSEAL (indent_set) : 1;
  guint GSEAL (rise_set) : 1;
  guint GSEAL (strikethrough_set) : 1;
  guint GSEAL (right_margin_set) : 1;
  guint GSEAL (pixels_above_lines_set) : 1;
  guint GSEAL (pixels_below_lines_set) : 1;
  guint GSEAL (pixels_inside_wrap_set) : 1;
  guint GSEAL (tabs_set) : 1;
  guint GSEAL (underline_set) : 1;
  guint GSEAL (wrap_mode_set) : 1;
  guint GSEAL (bg_full_height_set) : 1;
  guint GSEAL (invisible_set) : 1;
  guint GSEAL (editable_set) : 1;
  guint GSEAL (language_set) : 1;
  guint GSEAL (pg_bg_color_set) : 1;


**/

void gnoclGetTagSettings ( GtkTextTag *tag, gpointer data )
{
#ifdef DEBUG_TEXT
	//g_print ( "%s 1 \n", __FUNCTION__ );
#endif

	if ( tag == NULL )
	{
		return;
	}

	char **str = data;

	static char tmp[300];
	static char tmp2[50];

	GtkTextAppearance *tag_appearance;
	tag_appearance = tag->values;

	PangoFontDescription *font;
	GtkTextDirection direction;
	guint realized;

	gchar **fontName = NULL;
	gchar **fontFamily = NULL;
	gchar **fontDesc = NULL;

	/*
	 * Basic font settings will always be set for each tag.
	 */
	g_object_get ( G_OBJECT ( tag ), "font", &fontName, "family", &fontFamily, "font-desc", fontDesc,  NULL );

	/* name */
	strcat ( tmp, "" );
	sprintf ( tmp2, "%s {", tag->name );
	strcat ( tmp, tmp2 );

	/* priority */
	strcat ( tmp, " -priority" );
	sprintf ( tmp2, " %d ", tag->priority );
	strcat ( tmp, tmp2 );

	/* font */
	strcat ( tmp, " -font" );
	sprintf ( tmp2, " { %s }", fontName  );
	strcat ( tmp, tmp2 );

	/* font family */
	if  ( fontFamily != NULL )
	{
		strcat ( tmp, " -fontFamily" );
		sprintf ( tmp2, " { %s }", fontFamily );
		strcat ( tmp, tmp2 );
	}

	/*  background colour */
	if ( tag->bg_color_set )
	{
		GdkColor bg_color  = tag_appearance->bg_color;
		strcat ( tmp, " -background" );
		sprintf ( tmp2, " { %d %d %d }", bg_color.red, bg_color.green, bg_color.blue );
		strcat ( tmp, tmp2 );
	}

	/*  background stipple */
	if ( tag->bg_stipple_set )
	{
		GdkBitmap *bg_stipple  = tag_appearance->bg_stipple;

		strcat ( tmp, " -bgStipple" );
		sprintf ( tmp2, " { NOT YET IMPLEMENTED }" );
		strcat ( tmp, tmp2 );

	}

	/*  foreground colour */
	if ( tag->fg_color_set )
	{
		GdkColor fg_color  = tag_appearance->fg_color;
		/*  build up the output list */
		strcat ( tmp, " -foreground" );
		sprintf ( tmp2, " { %d %d %d }", fg_color.red, fg_color.green, fg_color.blue );
		strcat ( tmp, tmp2 );
	}


	/*  paragraph background colour */
	if ( tag->pg_bg_color_set )
	{
		GdkColor *pg_bg_color;

		//GdkColor pg_bg_color  = tag->values->pg_bg_color;

		g_object_get ( tag, "paragraph-background-gdk", pg_bg_color );

		/*  build up the output list */
		strcat ( tmp, " -paragraph" );
		//sprintf ( tmp2, " { %d %d %d}", pg_bg_color.red, pg_bg_color.green, pg_bg_color.blue );
		sprintf ( tmp2, " { NOT YET IMPLEMENTED%d %d %d %d }" );
		strcat ( tmp, tmp2 );
	}

	/*  font scaling */
	if ( tag->scale_set )
	{
		strcat ( tmp, " -scale" );
		sprintf ( tmp2, " %f", tag->values->font_scale );
		strcat ( tmp, tmp2 );
	}

	/*  foreground stipple */
	if ( tag->fg_stipple_set )
	{
		GdkBitmap *fg_stipple = tag_appearance->fg_stipple;

		strcat ( tmp, " -fgStipple" );
		sprintf ( tmp2, " { NOT YET IMPLEMENTED }" );
		strcat ( tmp, tmp2 );

	}

	/*  text justfification */
	if ( tag->justification_set )
	{
		strcat ( tmp, " -justification" );

		switch ( tag->values->justification )
		{
			case GTK_JUSTIFY_LEFT:
				{
					sprintf ( tmp2, " left" );
				} break;
			case GTK_JUSTIFY_RIGHT:
				{
					sprintf ( tmp2, " right" );
				} break;
			case GTK_JUSTIFY_CENTER:
				{
					sprintf ( tmp2, " center" );
				} break;
			case GTK_JUSTIFY_FILL:
				{
					sprintf ( tmp2, " fill" );
				} break;
			default:
				{
					return TCL_ERROR;
				}
		}

		strcat ( tmp, tmp2 );
	}

	/*  default indent */
	if ( tag->indent_set )
	{
		strcat ( tmp, " -indent" );
		sprintf ( tmp2, " %d", tag->values->indent );
		strcat ( tmp, tmp2 );
	}

	/*  text rise, i.e. for superscript */
	if ( tag->rise_set )
	{
		gint rise = tag_appearance->rise;
	}

	/*  strikethrough or, overstrike */
	if ( tag->strikethrough_set )
	{
		guint strikethrough;

		strcat ( tmp, " -strikethrough" );

		g_object_get ( tag, "strikethrough", &strikethrough, NULL );

		sprintf ( tmp2, " %d", strikethrough );
		strcat ( tmp, tmp2 );
	}

	/* right margin */
	if ( tag->right_margin_set )
	{
		strcat ( tmp, " -rightMargin" );
		sprintf ( tmp2, " %d", tag->values->right_margin );
		strcat ( tmp, tmp2 );
	}

	/* */
	if ( tag->pixels_above_lines_set )
	{
		strcat ( tmp, " -pixelsAboveLines" );
		sprintf ( tmp2, " %d", tag->values->pixels_above_lines );
		strcat ( tmp, tmp2 );
	}

	/* */
	if ( tag->pixels_below_lines_set )
	{
		strcat ( tmp, " -pixelsBelowLines" );
		sprintf ( tmp2, " %d", tag->values->pixels_below_lines );
		strcat ( tmp, tmp2 );
	}

	/* */
	if ( tag->pixels_inside_wrap_set )
	{
		//gint pixels_inside_wrap = tag_attributes->pixels_inside_wrap;
		//printf ( "pixels_inside_wrap\t%d\n", tag->values->pixels_inside_wrap );
	}

	/* */
	if ( tag->tabs_set )
	{
		PangoTabArray *tabs = tag->values->tabs;
	}

	/* underline */
	if ( tag->underline_set != 0 )
	{
		guint underline;

		g_object_get ( tag, "underline", &underline, NULL );
		strcat ( tmp, " -underline" );

		switch ( underline )
		{
			case PANGO_UNDERLINE_NONE:
				{
					sprintf ( tmp2, " %s", "none" );
				}
				break;
			case PANGO_UNDERLINE_SINGLE:
				{
					sprintf ( tmp2, " %s", "single" );
				}
				break;
			case PANGO_UNDERLINE_DOUBLE :
				{
					sprintf ( tmp2, " %s", "double" );
				}
				break;
			case PANGO_UNDERLINE_LOW:
				{
					sprintf ( tmp2, " %s", "low" );
				}
				break;
			case PANGO_UNDERLINE_ERROR:
				{
					sprintf ( tmp2, " %s", "error" );
				}
				break;
			default:
				{
					//Tcl_SetObjResult ( interp, Tcl_NewStringObj ( "WARNING: Unable to obtain tag underline setting.", -1 ) );
					return TCL_ERROR;
				}
		}

		strcat ( tmp, tmp2 );
	}

	/* wrap mode */
	if ( tag->wrap_mode_set )
	{
		GtkWrapMode wrap_mode;
		gint wrapMode;

		g_object_get ( tag, "wrap-mode", &wrapMode, NULL );


		strcat ( tmp, " -wrapMode" );

		switch ( wrapMode )
		{
			case GTK_WRAP_CHAR:
				{
					sprintf ( tmp2, " %s", "char" );
				} break;
			case GTK_WRAP_NONE:
				{
					sprintf ( tmp2, " %s", "none" );
				} break;
			case GTK_WRAP_WORD:
				{
					sprintf ( tmp2, " %s", "word" );
				}
				break;
			case GTK_WRAP_WORD_CHAR:
				{
					sprintf ( tmp2, " %s", "wordChar" );
				}
				break;
			default:
				{
					sprintf ( tmp2, " %s", "none" );
				}
		}

		strcat ( tmp, tmp2 );
	}

	/* height */
	if ( tag->bg_full_height_set )
	{
		/*  Background is fit to full line height rather than
		 *  baseline +/- ascent/descent (font height)
		*/
		guint bg_full_height;
	}

	/* visible */
	if ( tag->invisible_set )
	{
		guint invisible = tag->values->invisible;
	}

	/* editable */
	if ( tag->editable_set )
	{
		/*  can edit this text */
		guint editable = tag->values->editable;
	}

	/* language */
	if ( tag->language_set )
	{
		PangoLanguage *language;
	}


	/*  terminate attribute list */
	strcat ( tmp, " } " );

	/* pass string to calling function */
	*str = tmp;
}
