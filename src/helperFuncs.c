/*
 * $Id: helperFuncs.c,v 1.11 2005/01/01 15:27:54 baum Exp $
 *
 * This file implements some helper functions
 *
 * Copyright (c) 2001 - 2005 Peter G. Baum  http://www.dr-baum.net
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

/*
   History:
   2014-01: added gnoclUpdate, internal library call to complete drawing tasks
   2013-07: removed gnoclGetWidgetOptions func, moved to parseOptions.c
   2013-06: added str_replace
   2012-03: added trim
   2010-10: added stringtype
   2010-07: added cmds2list
   2009-01: added getIdx
   2009-01: added gnoclGetWigdetGeometry
        05: added gnoclPosOffset from text.c
   2003-04: added gnoclPixbufFromObj
   2002-12: removed gnoclEventToString
            joined with convert.c
        09: gnoclFindLabel, gnoclLabelParseULine
   2001-07: Begin of developement
 */

#include "gnocl.h"
#include <gdk-pixbuf/gdk-pixbuf.h>

typedef struct
{
	GtkWidget *widget;
	GtkType   type;
} FindWidgetStruct;

/**
 Remove white spaces on both sides of a string.
**/

char *trim ( char *s )
{
	char *ptr;

	if ( !s )
	{
		return NULL;   // handle NULL string
	}

	if ( !*s )
	{
		return s;      // handle empty string
	}

	for ( ptr = s + strlen ( s ) - 1; ( ptr >= s ) && isspace ( *ptr ); --ptr );

	ptr[1] = '\0';
	return s;
}


/**
\brief	Write fucntion and step to stdout.
**/
void debugStep ( gchar *str, gdouble i )
{
	g_print ( "%s %2.01f\n", str, i );
}

/**
\brief	Obtain the height and width of a pango string in pixels.
**/
void getTextWidth ( GtkWidget *widget, char * family , int ptsize , int weight , int normalstyle ,
					char * stringtomeasure ,
					int * wdret , int * htret )
{
	PangoFontDescription * fd = pango_font_description_new ( );

	pango_font_description_set_family ( fd, family );
	pango_font_description_set_style ( fd, normalstyle ? PANGO_STYLE_NORMAL : PANGO_STYLE_ITALIC );
	pango_font_description_set_variant ( fd, PANGO_VARIANT_NORMAL );
	pango_font_description_set_weight ( fd, ( PangoWeight ) weight );
	pango_font_description_set_stretch ( fd, PANGO_STRETCH_NORMAL );
	pango_font_description_set_size ( fd, ptsize * PANGO_SCALE );

	PangoContext * context = gtk_widget_get_pango_context ( widget ) ;

	PangoLayout * layout = pango_layout_new ( context );
	pango_layout_set_text ( layout, stringtomeasure, -1 );
	pango_layout_set_font_description ( layout, fd );
	pango_layout_get_pixel_size ( layout, wdret , htret );
	g_object_unref ( layout );
}


/**
\brief Return Units
**/

gchar * getUnits ( gchar *str )
{

	gint ret = 0;

	static const char *units[] =
	{
		"pixel", "points", "inch", "mm",
		NULL
	};

	enum unitIdx
	{
		PixelIdx, PointsIdx, InchIdx, MmIdx
	};

	getIdx ( units, str, &ret );

	if ( ret )
	{
		return str;
	}

	else
	{
		return NULL;
	}

}

/**
\brief	Return the number of items in a space separated list.
**/
int listLength ( char *str )
{

	int len;

	char * pch;
	len = 0;

	pch = strtok ( str, " " );

	while ( pch != NULL )
	{
		pch = strtok ( NULL, " " );
		len++;
	}

	return len;
}

/**
\brief	Return substring of range idx to len from string str.
**/
char *substring ( char *str, int idx, int len )
{

	int i;
	char *to;

	i =  strlen ( str ) ;
	to = ( char* ) malloc ( i );

	strncpy ( to, str + idx, len );
	return to;

}

/**
\brief Programming utility function that lists parameters.
**/
void listParameters ( int objc, Tcl_Obj * const objv[], char *str )
{
	gint i = 0;

	g_print ( "---------------\n" );
	printf ( "%s\n", str );

	while ( i < objc )
	{
		g_print ( "  %d = %s\n", i, Tcl_GetString ( objv[i] ) );
		i++;
	}

}

/**
\brief	Return the number of items in a CSV data string.
**/
int getSize ( char *str1, char *sep )
{
	char *pch;
	int  i;
	char str2[256];

	/* get a copy of the data string */
	sprintf ( str2, "%s", str1 );

	i = 0;
	pch = strtok ( str2, sep );

	while ( pch != NULL )
	{
		pch = strtok ( NULL, sep );
		i++;
	}

	return i;
}

/**
/brief 	Convert a string of CSV decimal numbers into an array of doubles.
**/
int getdoubles ( char *str1, gdouble myFloats[] )
{
	char * pch;
	int  i;
	char str2[strlen ( str1 ) ];

	/* get a copy of the data string */
	sprintf ( str2, "%s", str1 );

	i = 0;
	pch = strtok ( str2, "," );

	while ( pch != NULL )
	{
		myFloats[i] = atof ( pch );
		pch = strtok ( NULL, "," );
		i++;
	}

	return i;

}

/**
\brief    Compare string with list of acceptable choices. Return index of any match.
\note     For use in conjuction with switch to determine action based upon one of a range of choices given choices.
\see      getIDx
**/
int compare ( char *str, char *choices[] )
{

	int i;

	for ( i = 0; choices[i] != NULL ; i++ )
	{
		if ( strcmp ( str, choices[i] ) == 0 )
		{
			return i;
		}
	}
}

/**
\brief Return a NULL terminated string array as a list.
**/
gchar * cmds2list ( gchar *arr[] )
{

	gchar buffer[512];
	char *ptr;
	gint len = 0;

	sprintf ( buffer, "%s", "" );

	while ( ptr != NULL )
	{
		sprintf ( buffer, "%s %s", buffer, arr[len] );
		ptr = arr[++len];
	}

	return buffer;
}

/**
\brief	A programming utility function, it returns the type of option string received.
**/
gchar *stringtype ( int type )
{
	switch ( type )
	{
		case GNOCL_STR_EMPTY:
			{
				return "empty";
			}
		case GNOCL_STR_STR:
			{
				return "string";
			}
		case GNOCL_STR_STOCK:
			{
				return "stockItem";
			}
		case GNOCL_STR_FILE:
			{
				return "file";
			}
		case GNOCL_STR_TRANSLATE:
			{
				return "translate";
			}
		case GNOCL_STR_UNDERLINE:
			{
				return "underline";
			}
		case GNOCL_STR_MARKUP:
			{
				return "markup";
			}
		case GNOCL_STR_BUFFER:
			{
				return "pixBuf";
			}
	}

	return "unknown";

}

/**
\brief append to string
**/
char *str_append ( const char *s1, const char *s2 )
{
	char *s0 = malloc ( strlen ( s1 ) + strlen ( s2 ) + 1 );
	strcpy ( s0, s1 );
	strcat ( s0, s2 );
	return s0;
}

/**
\brief	prepend to string
**/
char *str_prepend ( const char *s1, const char *s2 )
{
	char *s0 = malloc ( strlen ( s1 ) + strlen ( s2 ) + 1 );
	strcpy ( s0, s2 );
	strcat ( s0, s1 );
	return s0;
}


/**
\brief	replace all occurances of substring.
**/
char *str_replace ( const char *string, const char *substr, const char *replacement )
{
	char *tok = NULL;
	char *newstr = NULL;
	char *oldstr = NULL;
	char *head = NULL;

	/* if either substr or replacement is NULL, duplicate string a let caller handle it */
	if ( substr == NULL || replacement == NULL ) return strdup ( string );

	newstr = strdup ( string );
	head = newstr;

	while ( ( tok = strstr ( head, substr ) ) )
	{
		oldstr = newstr;
		newstr = malloc ( strlen ( oldstr ) - strlen ( substr ) + strlen ( replacement ) + 1 );

		/*failed to alloc mem, free old string and return NULL */
		if ( newstr == NULL )
		{
			free ( oldstr );
			return NULL;
		}

		memcpy ( newstr, oldstr, tok - oldstr );
		memcpy ( newstr + ( tok - oldstr ), replacement, strlen ( replacement ) );
		memcpy ( newstr + ( tok - oldstr ) + strlen ( replacement ), tok + strlen ( substr ), strlen ( oldstr ) - strlen ( substr ) - ( tok - oldstr ) );
		memset ( newstr + strlen ( oldstr ) - strlen ( substr ) + strlen ( replacement ) , 0, 1 );
		/* move back head right after the last replacement */
		head = newstr + ( tok - oldstr ) + strlen ( replacement );
		free ( oldstr );
	}

	return newstr;
}

/**
\brief	Replace string substrings (vix. Tcl "string map" command).
\args	str	the string to process
		map null terminated array containing target substring and their replacements

		Example:

		gchar *map[] = {
			"<", "&lt;",
			">", "&gt;",
			NULL
			};

		g_print ("%s -> %s\n",tmp, string_map ("<sub>", map));
**/
char *string_map ( char *str, char *map[] )
{

	gint i = 0;
	gchar *newstr;

	newstr = str;

	while ( map[i] != NULL )
	{
		newstr = str_replace ( newstr, map[i], map[i+1] );

		if ( newstr == NULL )
		{
			return str;
		}

		i += 2;
	}

	return newstr;
}

/**
\brief      Get index of matched string in an NULL terminated array of strings.
\author     William J Giddings
\date       05/05/09
\param      gchar *opts[]   Pointer the array containing keywords
\param      gchar *str      Pointer to the string to match
\param      gint *idx       Pointer to int which contains the matched index
\return     boolean, returns 0 is match found else -1
\note       Useful for making choices from a list and processing with switch.
**/
int getIdx ( gchar *opts[], gchar *str, gint *idx )
{
#if 0
	g_print ( "$s\n", __FUNCTION__ );
	g_printf ( "\topts = %s\n", opts );
	g_printf ( "\tstr = %s\n", str );
	g_printf ( "\tidx = %s\n", *idx );
#endif

	int i = 0;

	while ( opts[i] != NULL ) /* go through array until NULL encountered */
	{

		if ( strcmp ( str , opts[i] ) == 0 )
		{
			*idx = i;
			return 0;
		}

		i++;
	}

	/* must be an error here! */
	return -1;
}


/**
\author     William J Giddings
\date       15/01/09
**/
char *gnoclGetWidgetGeometry ( GtkWidget *widget )
{
	gint x, y;
	gint w, h;
	char txt[500];

	x = widget->allocation.x;
	y = widget->allocation.y;
	w = widget->allocation.width;
	h = widget->allocation.height;

	sprintf ( txt, "%d %d %d %d", x, y, w, h );

	return txt;
}


/**
\brief      Find the "internal children" of a container widget.
**/
static void findChildIntern ( GtkWidget *widget, gpointer data )
{
	FindWidgetStruct *fw = ( FindWidgetStruct * ) data;

	if ( fw->widget == NULL )
	{
		if ( GTK_CHECK_TYPE ( widget, fw->type ) )
		{
			fw->widget = widget;
		}

		else if ( GTK_IS_CONTAINER ( widget ) )
		{
			gtk_container_foreach ( GTK_CONTAINER ( widget ), findChildIntern, data );
		}
	}
}

/**
\brief
**/
GtkWidget *gnoclFindChild ( GtkWidget *widget, GtkType type )
{
	FindWidgetStruct fw;
	fw.widget = NULL;
	fw.type = type;
	findChildIntern ( widget, &fw );
	return fw.widget;
}


/**
**/
int gnoclPosOffset ( Tcl_Interp *interp, const char *txt, int *offset )
{
	*offset = 0;
	/* pos[+-]offset */

	if ( *txt == '+' || *txt == '-' )
	{
		if ( sscanf ( txt + 1, "%d", offset ) != 1 )
		{
			Tcl_AppendResult ( interp, "invalid offset \"", txt + 1, "\"", NULL );
			return TCL_ERROR;
		}

		if ( *txt == '-' )
			*offset *= -1;

		for ( ++txt; isdigit ( *txt ); ++txt )
			;
	}

	if ( *txt )
	{
		Tcl_AppendResult ( interp, "invalid appendix \"", txt, "\"", NULL );
		return TCL_ERROR;
	}

	return TCL_OK;
}

/**
\brief Substitute percentage marker with values and then execute script.

**/
int gnoclPercentSubstAndEval ( Tcl_Interp *interp, GnoclPercSubst *ps, const char *orig_script, int background )
{

#ifdef DEBUG_HELPERFUNCS
	g_print ( "gnoclPercentSubstAndEval\n" );
#endif

	int        len = strlen ( orig_script );
	const char *old_perc = orig_script;
	const char *perc;
	GString    *script = g_string_sized_new ( len + 20 );
	int        ret;

	for ( ; ( perc = strchr ( old_perc, '%' ) ) != NULL; old_perc = perc + 2 )
	{
		g_string_sprintfa ( script, "%.*s", perc - old_perc, old_perc );

#ifdef DEBUG_HELPERFUNCS
		printf ( "\t1) script: \"%s\"\n", script->str );
#endif

		if ( perc[1] == '%' )
			g_string_append_c ( script, '%' );
		else
		{
			int k = 0;

			while ( ps[k].c && ps[k].c != perc[1] )
				++k;

			if ( ps[k].c == 0 )
			{
#ifdef DEBUG_HELPERFUNCS
				printf ( "\t2) DEBUG: unknown percent substitution %c\n", perc[1] );
#endif
				/*
				Tcl_AppendResult( interp, "unknown percent substitution" ,
				      (char *)NULL );
				g_string_free( script, 1 );
				return TCL_ERROR;
				*/
				g_string_append_c ( script, '%' );
				g_string_append_c ( script, perc[1] );
			}

			else
			{
				switch ( ps[k].type )
				{
					case GNOCL_STRING:
						/* FIXME: escape special characters: ' ', '\n', '\\' ...
						   or use Tcl_EvalObj? */

						if ( ps[k].val.str != NULL )
						{
							/* handle special characters correctly
							   TODO: would it be better to escape
							         special characters?
							*/
							char *txt = Tcl_Merge ( 1, &ps[k].val.str );
							g_string_append ( script, txt );
#ifdef DEBUG_HELPERFUNCS
							printf ( "\t3) percent string: \"%s\" -> \"%s\"\n", ps[k].val.str, txt );
#endif
							Tcl_Free ( txt );
							//g_string_free (txt,1);
						}

						else
							g_string_append ( script, "{}" );

						break;

					case GNOCL_OBJ:

						if ( ps[k].val.obj != NULL )
						{
							/* embedden 0s should be UTF encoded. Should
							   therefor also work. */
							/*
							g_string_sprintfa( script, "%s",
							      Tcl_GetString( ps[k].val.obj ) );
							*/
							const char *argv[2] = { NULL, NULL };
							char *txt;
							argv[0] = Tcl_GetString ( ps[k].val.obj );
							txt = Tcl_Merge ( 1, argv );
							g_string_append ( script, txt );
							Tcl_Free ( txt );

						}

						else
							g_string_append ( script, "{}" );

						break;

					case GNOCL_INT:
						g_string_sprintfa ( script, "%d", ps[k].val.i );

						break;

					case GNOCL_BOOL:
						g_string_sprintfa ( script, "%d", ps[k].val.b != 0 );

						break;

					case GNOCL_DOUBLE:
						g_string_sprintfa ( script, "%f", ps[k].val.d );

						break;

					default:
						assert ( 0 );

						break;
				}
			}
		}
	}

	g_string_append ( script, old_perc );

	/* Tcl_EvalObj would be faster and more elegant, but incompatible: eg. two consecutive percent substitutions without space */
	ret = Tcl_EvalEx ( interp, script->str, -1, TCL_EVAL_GLOBAL | TCL_EVAL_DIRECT );

#ifdef DEBUG_HELPERFUNCS
	printf ( "\t4) DEBUG: script in percEval: %s -> %d %s\n", script->str, ret, Tcl_GetString ( Tcl_GetObjResult ( interp ) ) );
#endif

	g_string_free ( script, 1 );

	if ( background && ret != TCL_OK )
	{
		Tcl_BackgroundError ( interp );
	}

	return ret;
}

/**
\brief
**/
int gnoclAttachVariable ( GnoclOption *newVar, char **oldVar, const char *signal, GObject *obj,
						  GCallback gtkFunc, Tcl_Interp *interp, Tcl_VarTraceProc tclFunc, gpointer data )
{
	if ( *oldVar && ( newVar == NULL || newVar->status == GNOCL_STATUS_CHANGED ) )
		Tcl_UntraceVar ( interp, *oldVar, TCL_TRACE_WRITES | TCL_GLOBAL_ONLY,
						 tclFunc, data );

	if ( newVar == NULL || newVar->status != GNOCL_STATUS_CHANGED
			|| newVar->val.str[0] == 0 )
	{
		/* no new variable -> delete all */
		if ( *oldVar )
		{
			g_signal_handlers_disconnect_matched ( obj,
												   G_SIGNAL_MATCH_FUNC, 0, 0, NULL, ( gpointer * ) gtkFunc, NULL );
			g_free ( *oldVar );
			*oldVar = NULL;
		}
	}

	else
	{
		if ( *oldVar == NULL )    /* new variable but old didn't exist */
			g_signal_connect ( obj, signal, gtkFunc, data );
		else                      /* new variable and old did exist */
			g_free ( *oldVar );

		*oldVar = newVar->val.str;    /* transfer ownership */

		newVar->val.str = NULL;

		Tcl_TraceVar ( interp, *oldVar, TCL_TRACE_WRITES | TCL_GLOBAL_ONLY,
					   tclFunc, data );
	}

	return TCL_OK;
}

/**
\brief  Provides tracer function on variables
 */
int gnoclAttachOptCmdAndVar ( GnoclOption *newCmd, char **oldCmd, GnoclOption *newVar, char **oldVar,
							  const char *signal, GObject *obj, GCallback gtkFunc, Tcl_Interp *interp,
							  Tcl_VarTraceProc tclFunc, gpointer data )
{
	const int wasConnected = *oldVar != NULL || *oldCmd != NULL;

	/* handle variable */

	if ( newVar == NULL || newVar->status == GNOCL_STATUS_CHANGED )
	{
		if ( *oldVar )
		{
			Tcl_UntraceVar ( interp, *oldVar, TCL_TRACE_WRITES | TCL_GLOBAL_ONLY, tclFunc, data );
			g_free ( *oldVar );
			*oldVar = NULL;
		}
	}

	if ( newVar && newVar->status == GNOCL_STATUS_CHANGED && *newVar->val.str != '\0' )
	{
		*oldVar = g_strdup ( newVar->val.str );
		Tcl_TraceVar ( interp, *oldVar, TCL_TRACE_WRITES | TCL_GLOBAL_ONLY, tclFunc, data );
	}

	/* handle command */

	if ( newCmd == NULL || newCmd->status == GNOCL_STATUS_CHANGED )
	{
		if ( *oldCmd )
		{
			g_free ( *oldCmd );
			*oldCmd = NULL;
		}
	}

	if ( newCmd && newCmd->status == GNOCL_STATUS_CHANGED && *newCmd->val.str != '\0' )
	{
		*oldCmd = g_strdup ( newCmd->val.str );
	}

	/* if cmd or var is set, we need the gtkFunc */

	if ( *oldVar || *oldCmd )
	{
		if ( wasConnected == 0 )
		{
			g_signal_connect ( G_OBJECT ( obj ), signal, gtkFunc, data );
		}
	}

	else if ( wasConnected )
	{
		g_signal_handlers_disconnect_matched ( G_OBJECT ( obj ), G_SIGNAL_MATCH_FUNC, 0, 0, NULL, ( gpointer * ) gtkFunc, NULL );
	}

	return TCL_OK;
}



/**
\brief
**/
int gnoclGet2Boolean ( Tcl_Interp *interp, Tcl_Obj *obj, int *b1, int *b2 )
{
	int no;

	if ( Tcl_ListObjLength ( interp, obj, &no ) == TCL_OK
			&& ( no == 2 || no == 1 ) )
	{
		if ( no == 1 )
		{
			if ( Tcl_GetBooleanFromObj ( interp, obj, b1 ) != TCL_OK )
				return TCL_ERROR;

			*b2 = *b1;

			return TCL_OK;
		}

		else
		{
			Tcl_Obj *tp;

			if ( Tcl_ListObjIndex ( interp, obj, 0, &tp ) == TCL_OK )
			{
				if ( Tcl_GetBooleanFromObj ( interp, tp, b1 ) != TCL_OK )
					return TCL_ERROR;

				if ( Tcl_ListObjIndex ( interp, obj, 1, &tp ) == TCL_OK )
				{
					if ( Tcl_GetBooleanFromObj ( interp, tp, b2 ) != TCL_OK )
						return TCL_ERROR;
				}

				return TCL_OK;
			}
		}
	}

	Tcl_AppendResult ( interp, "Expected boolean value or list of "

					   "two boolean values but got \"", Tcl_GetString ( obj ), "\"", NULL );

	return TCL_ERROR;
}

/**
**/
int gnoclGet2Int ( Tcl_Interp *interp, Tcl_Obj *obj, int *b1, int *b2 )
{
	int no;

	if ( Tcl_ListObjLength ( interp, obj, &no ) == TCL_OK
			&& ( no == 2 || no == 1 ) )
	{
		if ( no == 1 )
		{
			if ( Tcl_GetIntFromObj ( interp, obj, b1 ) != TCL_OK )
				return TCL_ERROR;

			*b2 = *b1;

			return TCL_OK;
		}

		else
		{
			Tcl_Obj *tp;

			if ( Tcl_ListObjIndex ( interp, obj, 0, &tp ) == TCL_OK )
			{
				if ( Tcl_GetIntFromObj ( interp, tp, b1 ) != TCL_OK )
					return TCL_ERROR;

				if ( Tcl_ListObjIndex ( interp, obj, 1, &tp ) == TCL_OK )
				{
					if ( Tcl_GetIntFromObj ( interp, tp, b2 ) != TCL_OK )
						return TCL_ERROR;
				}

				return TCL_OK;
			}
		}
	}

	Tcl_AppendResult ( interp, "Expected integer value or list of "
					   "two integer values but got \"", Tcl_GetString ( obj ), "\"", NULL );

	return TCL_ERROR;
}

/**
**/
int gnoclGet2Double ( Tcl_Interp *interp, Tcl_Obj *obj, double *b1, double *b2 )
{
	int no;

	if ( Tcl_ListObjLength ( interp, obj, &no ) == TCL_OK
			&& ( no == 2 || no == 1 ) )
	{
		if ( no == 1 )
		{
			if ( Tcl_GetDoubleFromObj ( interp, obj, b1 ) != TCL_OK )
			{
				return TCL_ERROR;
			}

			*b2 = *b1;

			return TCL_OK;
		}

		else
		{
			Tcl_Obj *tp;

			if ( Tcl_ListObjIndex ( interp, obj, 0, &tp ) == TCL_OK )
			{
				if ( Tcl_GetDoubleFromObj ( interp, tp, b1 ) != TCL_OK )
				{
					return TCL_ERROR;
				}

				if ( Tcl_ListObjIndex ( interp, obj, 1, &tp ) == TCL_OK )
				{
					if ( Tcl_GetDoubleFromObj ( interp, tp, b2 ) != TCL_OK )
					{
						return TCL_ERROR;
					}
				}

				return TCL_OK;
			}
		}
	}

	Tcl_AppendResult ( interp, "Expected float value or list of "

					   "two float values but got \"", Tcl_GetString ( obj ), "\"", NULL );

	return TCL_ERROR;
}

/**
\brief	Complete internal drawing tasks
**/
void gnoclUpdate ()
{
	int nMax = 500;
	int n;

	for ( n = 0; n < nMax && gtk_events_pending(); ++n )
	{
		gtk_main_iteration_do ( 0 );
	}
}

/**
\brief
**/
static int getScrollbarPolicy ( Tcl_Interp *interp, Tcl_Obj *obj, GtkPolicyType *pol )
{
	const char *txt[] = { "always", "never", "automatic", NULL };
	GtkPolicyType policies[] =
	{
		GTK_POLICY_ALWAYS, GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC
	};
	int on;

	if ( Tcl_GetBooleanFromObj ( NULL, obj, &on ) == TCL_OK )
	{
		*pol = on ? GTK_POLICY_ALWAYS : GTK_POLICY_NEVER;
	}

	else
	{
		int idx;

		if ( Tcl_GetIndexFromObj ( interp, obj, txt, "scrollbar policy", TCL_EXACT, &idx ) != TCL_OK )
		{
			return TCL_ERROR;
		}

		*pol = policies[idx];
	}

	return TCL_OK;
}



/**
 */
int gnoclGetScrollbarPolicy ( Tcl_Interp *interp, Tcl_Obj *obj, GtkPolicyType *hor, GtkPolicyType *vert )
{
	int no;

	if ( Tcl_ListObjLength ( interp, obj, &no ) != TCL_OK  || no > 2 )
	{
		Tcl_SetResult ( interp, "policy must be either a single value "
						"or a list with two elements.", TCL_STATIC );
		return TCL_ERROR;
	}

	if ( no == 1 )
	{
		if ( getScrollbarPolicy ( interp, obj, hor ) != TCL_OK )
			return TCL_ERROR;

		*vert = *hor;
	}

	else
	{
		Tcl_Obj *tp;

		if ( Tcl_ListObjIndex ( interp, obj, 0, &tp ) != TCL_OK )
			return TCL_ERROR;

		if ( getScrollbarPolicy ( interp, tp, hor ) != TCL_OK )
			return TCL_ERROR;

		if ( Tcl_ListObjIndex ( interp, obj, 1, &tp ) != TCL_OK )
			return TCL_ERROR;

		if ( getScrollbarPolicy ( interp, tp, vert ) != TCL_OK )
			return TCL_ERROR;
	}

	return TCL_OK;
}



/**
 */
int gnoclGetSelectionMode ( Tcl_Interp *interp, Tcl_Obj *obj, GtkSelectionMode *selection )
{
	const char *txt[] = { "single", "browse", "multiple", "extended", NULL };
	GtkSelectionMode modes[] = { GTK_SELECTION_SINGLE, GTK_SELECTION_BROWSE, GTK_SELECTION_MULTIPLE, GTK_SELECTION_EXTENDED };

	int idx;

	if ( Tcl_GetIndexFromObj ( interp, obj, txt, "selection modes",
							   TCL_EXACT, &idx ) != TCL_OK )
		return TCL_ERROR;

	*selection = modes[idx];

	return TCL_OK;
}



/**
 */
int gnoclGetOrientationType ( Tcl_Interp *interp, Tcl_Obj *obj, GtkOrientation *orient )
{
	const char *txt[] = { "horizontal", "vertical", NULL };
	GtkOrientation types[] = { GTK_ORIENTATION_HORIZONTAL, GTK_ORIENTATION_VERTICAL };

	int idx;

	if ( Tcl_GetIndexFromObj ( interp, obj, txt, "orientation", TCL_EXACT, &idx ) != TCL_OK )
	{
		return TCL_ERROR;
	}

	*orient = types[idx];

	return TCL_OK;
}


/**
 */
int gnoclGetWindowType ( Tcl_Interp *interp, Tcl_Obj *obj, GtkWindowType *type )
{
	const char *txt[] = { "popup", "toplevel", NULL };

	GtkWindowType types[] = { GTK_WINDOW_POPUP, GTK_WINDOW_TOPLEVEL };

	int idx;

	if ( Tcl_GetIndexFromObj ( interp, obj, txt, "types", TCL_EXACT, &idx ) != TCL_OK )
	{
		return TCL_ERROR;
	}

	*type = types[idx];

	return TCL_OK;
}


/**
\brief	This function simply loads a single file from disk into memory.
**/
GdkPixbuf *gnoclPixbufFromObj ( Tcl_Interp *interp, GnoclOption *opt )
{
	char *txt = gnoclGetString ( opt->val.obj );
	GError *error = NULL;
	GdkPixbuf *pix = gdk_pixbuf_new_from_file ( txt, &error );

	assert ( gnoclGetStringType ( opt->val.obj ) == GNOCL_STR_FILE );

	if ( pix == NULL )
	{
		Tcl_SetResult ( interp, error->message, TCL_VOLATILE );
		g_error_free ( error );
		return NULL;
	}

	return pix;
}

/**
\brief      Read in a list of images and bitmasks, composite them to a single
            pixbuf and then return a pointer to the pixbuf to the calling function
\note      This assumes that the images to be composite and the masks are ALL of
            matching sizes. There is no attempt to reposition or resize graphics.
            Key Library Functions
            GtkWidget  *gtk_image_new_from_file (const gchar *filename);
            GtkWidget  *gtk_image_new_from_pixbuf (GdkPixbuf *pixbuf);
            void gtk_image_get_image (GtkImage *image, GdkImage **gdk_image, GdkBitmap **mask);
            GtkWidget  *gtk_image_new_from_image (GdkImage *image, GdkBitmap *mask);
            GdkPixbuf  *gtk_image_get_pixbuf (GtkImage *image);
            void gdk_pixbuf_render_threshold_alpha (GdkPixbuf *pixbuf,GdkBitmap *bitmap, int src_x,
                int src_y, int dest_x, int dest_y, int width, int height, int alpha_threshold);
            ie. gdk_pixbuf_render_threshold_alpha (pb,mp,0,0,0,0,-1,-1,128);
 */
GdkPixbuf *gnoclBlendPixbufFromObj ( Tcl_Interp *interp, GnoclOption *opt )
{

	GdkPixbuf *pixbuf = NULL;
	GError *err = NULL;
	GtkWidget *gtk_image = NULL;
	GtkWidget *gtk_image_blend = NULL;
	GdkImage *gdk_image = NULL;
	GdkBitmap *gdk_bitmap = NULL;

	char * pch;

	printf ( "helperFuncs/gnoclBlendPixbufFromObj pixbuf %s\n", Tcl_GetString ( opt->val.obj ) );

	pch = strtok ( Tcl_GetString ( opt->val.obj ), " " );
	int aa = 0;
	int bb = 0;

	while ( pch != NULL )
	{
		switch ( aa )
		{
			case 0:
				{
					g_print ( "pch = %s\n", pch );
					pixbuf = gdk_pixbuf_new_from_file ( pch , &err );

					if ( err != NULL )
					{
						g_warning ( "%s", err->message );
						g_error_free ( err );
						return NULL;
					}

					/* error checking over, create the image */
					gtk_image = gtk_image_new_from_pixbuf ( pixbuf );

					/* create a gdk_image from this, dump any alpha channel */

					if ( gdk_image == NULL )
					{

						/* get the size of the pixbuf */

						g_print ( "make a new gdk_image\n" );
						gdk_image = gdk_image_new (
										GDK_IMAGE_FASTEST,
										gdk_visual_get_system(),
										gdk_pixbuf_get_width ( pixbuf ),
										gdk_pixbuf_get_height ( pixbuf ) );
					}

					g_print ( "AAA\n" );
					gtk_image_get_image ( GTK_IMAGE ( gtk_image ), &gdk_image, NULL );
					g_print ( "BBB\n" );

				}

				break;
			case 1:
				{

					pixbuf = gdk_pixbuf_new_from_file ( pch, &err );

					if ( err != NULL )
					{
						g_warning ( "%s", err->message );
						g_error_free ( err );
						return NULL;
					}

					/* error checking over, create the clipping mask */
					gdk_pixbuf_render_threshold_alpha ( pixbuf, gdk_bitmap, 0, 0, 0, 0, -1, -1, 1 );
				}

				break;
		}

		if ( aa < 3 )
		{
			aa++;
		}

		else
		{
			aa = 0;
		}

		pch = strtok ( NULL, " " );

		/* do the actual compositing onto the buffer background */

		gtk_image_blend = gtk_image_new_from_image ( gdk_image, gdk_bitmap );

		g_print ( "composite images now!\n" );


	}

	if ( 0 )
	{
		return gtk_image_get_pixbuf ( gtk_image_blend );
	}

	else
	{
		return pixbuf;
	}
}

/* reverse:  reverse string s in place */
void reverse ( char s[] )
{
	int i, j;
	char c;

	for ( i = 0, j = strlen ( s ) - 1; i < j; i++, j-- )
	{
		c = s[i];
		s[i] = s[j];
		s[j] = c;
	}
}


/**
\brief	convert n to characters in s
**/
void itoa ( int n, char s[] )
{
	int i, sign;

	if ( ( sign = n ) < 0 ) /* record sign */
		n = -n;          /* make n positive */

	i = 0;

	do  /* generate digits in reverse order */
	{
		s[i++] = n % 10 + '0';   /* get next digit */
	}
	while ( ( n /= 10 ) > 0 );   /* delete it */

	if ( sign < 0 )
		s[i++] = '-';

	s[i] = '\0';
	reverse ( s );
}

/**
\brief	Simple check to see if a file exists.
**/
int exists ( const char *fname )
{
	FILE *file;

	if ( file = fopen ( fname, "r" ) )
	{
		fclose ( file );
		return 1;
	}

	return 0;
}

/**
\brief called from Toolbar libraries
**/


/**
\brief
**/
Tcl_Obj *cgetText ( GtkWidget *item )
{

	/* FIXME: is there really only one label? */
	GtkWidget *label = gnoclFindChild ( item, GTK_TYPE_LABEL );

	if ( label != NULL )
	{
		const char *txt = gtk_label_get_label ( GTK_LABEL ( label ) );

		if ( txt != NULL )
		{
			Tcl_Obj *obj = Tcl_NewStringObj ( txt, -1 );
			/* FIXME: that does not work
			if( gtk_button_get_use_stock( GTK_BUTTON( item ) ) )
			{
			   Tcl_Obj *old = obj;
			   obj = Tcl_NewStringObj( "%#", 2 );
			   Tcl_AppendObjToObj( obj, old );
			}
			else
			*/

			if ( gtk_label_get_use_underline ( GTK_LABEL ( label ) ) )
			{
				Tcl_Obj *old = obj;
				obj = Tcl_NewStringObj ( "%_", 2 );
				Tcl_AppendObjToObj ( obj, old );
			}

			return obj;
		}
	}

	return Tcl_NewStringObj ( "", 0 );;
}


void setUnderline ( GtkWidget *item )
{

	/* FIXME: is there really only one label? */
	GtkWidget *label = gnoclFindChild ( item, GTK_TYPE_LABEL );

	//assert ( label );
	if ( label !=  NULL )
	{
		gtk_label_set_use_underline ( GTK_LABEL ( label ), 1 );
	}
}


/**
\brief	Modified version of getTextAndIcon to allow for new Toolbar API
\param
		Tcl_Interp *interp		pointer to Tcl interpreter
		GtkToolbar *toolbar		pointer to toolbar object that will receive new item
		GnoclOption *txtOpt		pointer to item label ??
		GnoclOption *iconOpt	pointer to item icon ??
		char **txt				handle on text pointer
		GtkWidget **icon		handle on image widget pointer
		int *isUnderline		pointer to int

\note	Either -text or -icon must be set.

**/
int getTextAndIcon ( Tcl_Interp *interp, GtkToolbar *toolbar,
					 GnoclOption *txtOpt, GnoclOption *iconOpt,
					 char **txt, GtkWidget **item, int *isUnderline )
{
	*item = NULL;
	*txt = NULL;
	*isUnderline = 0;

	GtkImage *image;
	gchar *name;
	Tcl_Obj *icon;
	GnoclStringType type;

	/* error check, must have an icon defined! */
	if ( iconOpt->status != GNOCL_STATUS_CHANGED && txtOpt->status != GNOCL_STATUS_CHANGED )
	{
		Tcl_SetObjResult ( interp, Tcl_NewStringObj ( "GNOCL ERROR! Either -icon and -text must be set.", -1 ) );
		return TCL_ERROR;
	}


	/* if only text set, must check for percent string */
	if ( txtOpt->status == GNOCL_STATUS_CHANGED )
	{
		/* assume text == icon */
		name = gnoclGetStringFromObj ( txtOpt->val.obj, NULL );
		icon = txtOpt->val.obj;
		type = gnoclGetStringType ( txtOpt->val.obj );

	}

	/* icon only */
	if ( iconOpt->status == GNOCL_STATUS_CHANGED )
	{
		icon = iconOpt->val.obj;

		if ( txtOpt->status != GNOCL_STATUS_CHANGED )
		{
			name = gnoclGetStringFromObj ( iconOpt->val.obj, NULL );
		}

	}

	type = gnoclGetStringType ( icon );

	/* stock item */
	if ( type & GNOCL_STR_STOCK )
	{
#if 0
		g_print ( "STOCK\n" );
#endif
		GtkStockItem stockItem;
		GtkIconSize sz;

		if ( gnoclGetStockItem ( icon, interp, &stockItem ) != TCL_OK )
		{
			return TCL_ERROR;
		}

		sz = gtk_toolbar_get_icon_size ( toolbar );
		image = gtk_image_new_from_stock ( stockItem.stock_id, sz );

	}

	/* file */
	else if ( type & GNOCL_STR_FILE )
	{
#if 0
		g_print ( "FILE\n" );
#endif
		GError *error = NULL;
		GdkPixbuf *pixbuf = NULL;

		pixbuf = gdk_pixbuf_new_from_file ( icon, NULL );
		image = gtk_image_new_from_pixbuf ( pixbuf );

	}

	/* buffer */
	else if ( type & GNOCL_STR_BUFFER )
	{
#if 0
		g_print ( "BUFFER\n" );
#endif
		GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file ( icon, NULL );
		GtkWidget *image = gtk_image_new_from_pixbuf ( pixbuf );

	}

	/* get the text string */
	else
	{
		GtkIconSize sz;

		sz = gtk_toolbar_get_icon_size ( toolbar );
		GtkWidget *image = gtk_image_new_from_stock ( GTK_STOCK_MISSING_IMAGE, sz );

	}

	/*
		uncertain what's happening here
		this isUnderline was
	*/
	if ( type & GNOCL_STR_UNDERLINE )
	{
		if ( txt == NULL )
		{
			*isUnderline = 0;
		}

		else
		{
			*isUnderline = 1;
		}
	}

	gtk_widget_show ( image );

	*item = image;

	*txt = g_strdup ( name );

	*isUnderline = 1;

	return TCL_OK;
}



/**
    search for first occurance of p in s, starting from i
    done
**/
int strnfrst ( char *s, char *p, int i )
{
#if 0
	g_print ( "FUNC: %s\n", __FUNCTION__ );
#endif

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
\brief
**/
int strip_chars ( char * string, char * chars )
{
#if 0
	g_print ( "FUNC: %s\n", __FUNCTION__ );
#endif
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
			//g_print ( "got tag %s\n", chars );
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

	//g_print ( "string = %s : newstr = %s\n", string, newstr );

	strcpy ( string, newstr );

	string[j] = '\0';

	return c;
}


/**
    extract a range of characters from string s starting from position a to position b
    done
**/
char *strrng ( char *dest, const char *src, int a, int b )
{
#if 0
	g_print ( "FUNC: %s\n", __FUNCTION__ );
#endif

	unsigned i, j;

	j = 0;

	for ( i = a; i < b; i++ )
	{
		dest[j++] = src[i];
	}

	dest[j] = '\0';
	return dest;

}



/* insert character in string, return new string */
char * strcinsert ( char *str, char *dest, char *c, int i )
{
#if 0
	g_print ( "FUNC: %s\n", __FUNCTION__ );
#endif

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
#if 0
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
char *sconcat ( const char *s1, const char *s2 )
{
#if 0
	g_print ( "FUNC: %s\n", __FUNCTION__ );
#endif

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
int contains ( char character, char * list )
{
#if 0
	g_print ( "FUNC: %s\n", __FUNCTION__ );
#endif

	while ( *list )
	{
		if ( character == *list )
			return 1;

		++list;
	}

	return 0;
}


/**
\brief
**/
char * removeChar ( char *str, char garbage )
{

#if 0
	g_print ( "FUNC: %s\n", __FUNCTION__ );
#endif

	char *src, *dst;

	for ( src = dst = str; *src != '\0'; src++ )
	{
		*dst = *src;

		if ( *dst != garbage ) dst++;
	}

	*dst = '\0';

	//g_print ( "%s\n", dst );

	return dst;
}


/* remove range of characters from string */
char * strcremove ( char *str, char *dest, int first, int last )
{
#if 1
	g_print ( "%s 1 first = %d ; last = %d\n", __FUNCTION__, first, last );
#endif
	int j = 0;
	char *ptr;
	ptr = str;

	while ( j < strlen ( str ) )
	{
		//g_print ( "j = %d\n", j );

		if ( j >= first && j <= last )
		{
			//g_print ( "remove %c\n", *str + j );
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
