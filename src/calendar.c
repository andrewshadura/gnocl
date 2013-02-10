/*
calendar.c
author   William J Giddings
date    20/05/09
*/

/* user documentation */

/**
\page page_calendar gnocl::calendar
\section cal1 DESCRIPTION
    Implementation of gnocl::calendar

\subsection cal2 OPTIONS
\b -day \e integer
\n Description

\b -month \e integer
\n Description

\b -year \e integer
\b Description

\b -rowHeight \e integer
\n Description

\b -colWidth \e integer
\n Description

\b -monthChange \e boolean (default: 1)
\n Description

\b -dayNames \e boolean (default: 1)
\n Description

\b -details \e boolean (default: 1)
\n Description

\b -heading \e boolean (default: 1)
\n Description

\b -weekNumbers \e boolean (default: 1)
\n Description

\b -onDaySelected \e string (default: "")
\n Tcl command which is executed in the global scope when the date highlighted in the widget is changed. Before evaluation the following percent strings are substituted:
\n
\n     \%w widget name
\n     \%g glade name
\n     \%d day number (range 1-31)
\n     \%m month number  (range 1-12)
\n     \%y year

\b -onDoubleDaySelected \e string (default: "")
\n Tcl command which is executed in the global scope when the highlighted date is double clicked. Before evaluation the following percent strings are substituted:
\n
\n     \%w widget name
\n     \%g glade name
\n     \%d day number (range 1-31)
\n     \%m month number  (range 1-12)
\n     \%y year

\b -onMonthChanged \e string (default: "")
\n Tcl command which is executed in the global scope if the month displayed in the widget is changed. Before evaluation the following percent strings are substituted:
\n
\n     \%w widget name
\n     \%g glade name
\n     \%d day number (range 1-31)
\n     \%m month number  (range 1-12)
\n     \%y year

\b -onNextMonth \e string (default: "")
\n Tcl command which is executed in the global scope if a mouse button is clicked over the month label 'next' glyph (>). Before evaluation the following percent strings are substituted:
\n
\n     \%w widget name
\n     \%g glade name
\n     \%d day number (range 1-31)
\n     \%m month number  (range 1-12)
\n     \%y year

\b -onNextYear \e string (default: "")
\n Tcl command which is executed in the global scope if a mouse button is clicked over the year label 'next'  glyph (>). Before evaluation the following percent strings are substituted:
\n
\n     \%w widget name
\n     \%g glade name
\n     \%d day number (range 1-31)
\n     \%m month number  (range 1-12)
\n     \%y year

\b -onPrevMonth \e string (default: "")
\n Tcl command which is executed in the global scope if a mouse button is clicked over the year label 'previous'  glyph (<). Before evaluation the following percent strings are substituted:
\n
\n     \%w widget name
\n     \%g glade name
\n     \%d day number (range 1-31)
\n     \%m month number  (range 1-12)
\n     \%y year

\b -onPrevYear \e string (default: "")
\n Tcl command which is executed in the global scope if a mouse button is clicked over the year label 'previous' glyph (<). Before evaluation the following percent strings are substituted:
\n
\n     \%w widget name
\n     \%g glade name
\n     \%d day number (range 1-31)
\n     \%m month number  (range 1-12)
\n     \%y year

\subsection cal3 COMMANDS
\n \b cget

\n \b class
\n Returns the class type for this widget, i.e. calendar.

\n \b configure  [\e -option  \e value...]
\n Configures the widget. Option may have any of the values accepted on creation of the widget.

\n \b detail
\n \b SubCommands
\n \b add [ \e -option \e value... ]
\n \b Options
\n \b -date
\n \b -text
\n \b remove [ \e -option \e value... ]
\n \b Options
\n \b -date

\n \b delete
\n Deletes the widget and the associated Tcl command.

\n \b get

\n \b mark

\n \b set
**/

/**
 \par Modification History
 \verbatim

 \endverbatim

 \todo
 button-BLAH, BLAH
**/

#include "gnocl.h"
#include "gnoclparams.h"

static void calendar_date_to_string ( CalendarParams *para, char *buffer, gint buff_len )
{
	GDate *date;
	guint year, month, day;

	gtk_calendar_get_date ( GTK_CALENDAR ( para->calendar ), &year, &month, &day );
	date = g_date_new_dmy ( day, month + 1, year );
	g_date_strftime ( buffer, buff_len - 1, "%x", date );

	g_date_free ( date );
}

/**
\brief	Add detail entry to the widget list,
**/
static void calendar_set_detail ( CalendarParams *para, guint year, guint month, guint day, gchar *detail )
{
	gchar *key = g_strdup_printf ( "%04d-%02d-%02d", year, month + 1, day );
	g_hash_table_replace ( para->details, key, detail );
}

/**
\brief	Retrieve detail entry from the widget list,
**/
static gchar* calendar_get_detail ( CalendarParams *para, guint year, guint month, guint day )
{
#ifdef DEBUG_CALENDAR
	g_print ( "%s-1\n", __FUNCTION__ );
#endif

	const gchar *detail;
	gchar *key;

	key = g_strdup_printf ( "%04d-%02d-%02d", year, month + 1, day );

#ifdef DEBUG_CALENDAR
	g_print ( "%s-2\n", __FUNCTION__ );
#endif

	detail = g_hash_table_lookup ( para->details, key );

#ifdef DEBUG_CALENDAR
	g_print ( "%s-3\n", __FUNCTION__ );
#endif

	g_free ( key );

	return ( detail ? g_strdup ( detail ) : NULL );
}

/**
\brief	Add update calendar details,
**/
static void calendar_update_details ( CalendarParams *para )
{

#ifdef DEBUG_CALENDAR
	g_print ( "%s-1\n", __FUNCTION__ );
#endif

	guint year, month, day;
	gchar *detail;

#ifdef DEBUG_CALENDAR
	g_print ( "%s-2\n", __FUNCTION__ );
#endif
	gtk_calendar_get_date ( GTK_CALENDAR ( para->calendar ), &year, &month, &day );

#ifdef DEBUG_CALENDAR
	g_print ( "%s-3\n", __FUNCTION__ );
#endif
	detail = calendar_get_detail ( para, year, month, day );

#ifdef DEBUG_CALENDAR
	g_print ( "%s-4\n", __FUNCTION__ );
#endif
	g_signal_handler_block ( para->buffer, para->changed );

#ifdef DEBUG_CALENDAR
	g_print ( "%s-5\n", __FUNCTION__ );
#endif
	gtk_text_buffer_set_text ( para->buffer, detail ? detail : "", -1 );

#ifdef DEBUG_CALENDAR
	g_print ( "%s-6\n", __FUNCTION__ );
#endif
	g_signal_handler_unblock ( para->buffer, para->changed );

#ifdef DEBUG_CALENDAR
	g_print ( "%s-7\n", __FUNCTION__ );
#endif

	g_free ( detail );
}

/**
\brief	Callback function to handle click on calendar,
**/
static gchar* calendar_detail_cb ( GtkCalendar *calendar, guint year, guint month, guint day, gpointer data )
{
	return calendar_get_detail ( data, year, month, day );
}

/**
\brief	Add demonstration date to the basic widget,
**/
static void demonstrate_details ( CalendarParams *para )
{

#ifdef DEBUG_CALENDAR
	g_print ( "%s-1\n", __FUNCTION__ );
#endif

	static char *rainbow[] = { "#900", "#980", "#390", "#095", "#059", "#309", "#908" };
	GtkCalendar *calendar = GTK_CALENDAR ( para->calendar );
	gint row, col;

	for ( row = 0; row < 6; ++row )
		for ( col = 0; col < 7; ++col )
		{
			gint year, month, day;
			gchar *detail;

			year = calendar->year;
			month = calendar->month;
			month += calendar->day_month[row][col];
			day = calendar->day[row][col];

			if ( month < 1 )
			{
				month += 12;
				year -= 1;
			}

			else if ( month > 12 )
			{
				month -= 12;
				year += 1;
			}

			detail = g_strdup_printf ( "<span color='%s'>yadda\n"
									   "(%04d-%02d-%02d)</span>",
									   rainbow[ ( day-1 ) % 7],
									   year, month, day );

			calendar_set_detail ( para, year, month - 1, day, detail );
		}

	gtk_widget_queue_resize ( para->calendar );
	calendar_update_details ( para );
}


/**
\brief	Function associated with the widget.
**/
static void destroyFunc (
	GtkWidget *widget,
	gpointer data )
{
#ifdef DEBUG_ENTRY
	g_print ( "entry/staticFuncs/destroyFunc\n" );
#endif


	CalendarParams *para = ( CalendarParams * ) data;

	gnoclForgetWidgetFromName ( para->name );
	Tcl_DeleteCommand ( para->interp, para->name );

	/* include some code to free-up the hash table and textbuffer */
}


/**
\brief      Callback function to focus change event.
**/
/*
author     William J Giddings
date      10/05/2010
*/
static gboolean doOnDoubleDaySelect ( GtkCalendar *calendar, gpointer data )
{
#ifdef DEBUG_CALENDAR
	g_print ( "%s-1\n", __FUNCTION__ );
#endif

	GnoclCommandData *cs = ( GnoclCommandData * ) data;

	GnoclPercSubst ps[] =
	{

		{ 'd', GNOCL_INT },		/* day */
		{ 'm', GNOCL_INT },  	/* month */
		{ 'Y', GNOCL_INT },  	/* day */
		{ 'w', GNOCL_STRING },  /* widget */
		{ 'g', GNOCL_STRING },  /* glade name */
		{ 'x', GNOCL_INT },  	/* pointer x-position in window */
		{ 'y', GNOCL_INT },  	/* pointer y-position in window */
		{ 'X', GNOCL_INT },  	/* pointer x-position on screen */
		{ 'Y', GNOCL_INT },  	/* pointer y-position on screen */
		{ 's', GNOCL_STRING },  /* signal */
		{ 0 }
	};

	gint year, month, day;
	gchar str[20];
	gint x, y;
	gint root_x;
	gint root_y;

	gtk_calendar_get_date ( calendar , &year, &month, &day );
	gtk_widget_get_pointer  ( calendar , &x, &y );

	/******************/
	gdk_window_get_origin ( calendar->widget.window, &root_x, &root_y );

	//root_x += calendar->widget.allocation.x;
	//root_x -= calendar->widget.style->xthickness;

	//root_y += calendar->widget.allocation.y;
	//root_y -= calendar->widget.style->ythickness;
	/******************/

	ps[0].val.i = day;
	ps[1].val.i = month + 1;
	ps[2].val.i = year;
	ps[3].val.str = gnoclGetNameFromWidget ( calendar );
	ps[4].val.str = gtk_widget_get_name ( GTK_WIDGET ( calendar ) );
	ps[5].val.i = x;
	ps[6].val.i = y;
	ps[7].val.i = root_x;
	ps[8].val.i = root_y;
	ps[9].val.str = "day-selected-double-click";

	gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 );

	return 0;
}


/**
\brief	Make month changer a positive and not negative choice.
**/
int gnoclOptShowMonthChange ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
#ifdef DEBUG_CALENDAR
	g_print ( "%s-1\n", __FUNCTION__ );
#endif" );
	guint show;


	assert ( strcmp ( opt->optName, " - monthChange" ) == 0 );

	show = atoi ( Tcl_GetStringFromObj ( opt->val.obj, NULL ) );

	if ( show == 1 )
	{
		show = 0;
	}

	else
	{
		show = 1;
	}

	g_object_set ( obj, "no - month - change", show, NULL );

	return TCL_OK;

}

/**
\brief	Allow real month numbers to be used; i.e. 1-12 and not 0-11.
**/
int gnoclOptMonth ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
#ifdef DEBUG_CALENDAR
	g_print ( " % s - 1\n",__FUNCTION__ );
#endif
	guint month;

	assert ( strcmp ( opt->optName, " - month" ) == 0 );

	month = atoi ( Tcl_GetStringFromObj ( opt->val.obj, NULL ) );

	g_object_set ( obj, "month", --month, NULL );

	return TCL_OK;

}


/**
\brief      Callback function to focus change event.
**/
static gboolean doOnDaySelected ( GtkCalendar *calendar, gpointer data )
{
#ifdef DEBUG_CALENDAR
	g_print ( " % s - 1\n",__FUNCTION__ );
#endif);

	GnoclCommandData *cs = ( GnoclCommandData * ) data;

	GnoclPercSubst ps[] =
		{

			{ 'd', GNOCL_INT },    /* day */
			{ 'm', GNOCL_INT },    /* month */
			{ 'y', GNOCL_INT },    /* year */
			{ 'w', GNOCL_STRING }, /* widget */
			{ 'g', GNOCL_STRING }, /* glade name */
			{ 'x', GNOCL_INT },    /* pointer x-position */
			{ 'y', GNOCL_INT },    /* pointer y-position */
			{ 's', GNOCL_STRING }, /* signal-type */
			{ 0 }
		};

	gint year, month, day;
	gchar str[20];
	gint x, y;

	gtk_calendar_get_date ( calendar , &year, &month, &day );

	gtk_widget_get_pointer  ( calendar , &x, &y );

	ps[0].val.i = day;
	ps[1].val.i = month;
	ps[2].val.i = year;
	ps[3].val.str = gnoclGetNameFromWidget ( calendar );
	ps[4].val.str = gtk_widget_get_name ( GTK_WIDGET (calendar) );
	ps[5].val.i = x;
	ps[6].val.i = y;
	ps[7].val.str = "day - selected";

	gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 );

	return 0;
}

/**
\brief
**/
int gnoclOptOnDaySelected ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
#ifdef DEBUG_CALENDAR
	g_print ( " % s - 1\n",__FUNCTION__ );
#endif

	assert ( strcmp ( opt->optName, " - onDaySelected" ) == 0 );
	return gnoclConnectOptCmd ( interp, obj, "day - selected", G_CALLBACK ( doOnDaySelected ), opt, obj, ret );
}


/**
\brief
\note
**/
/*
author     William J Giddings
date      10/05/2010
*/
int gnoclOptOnDoubleDaySelected ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	assert ( strcmp ( opt->optName, " - onDoubleDaySelected" ) == 0 );
	return gnoclConnectOptCmd ( interp, obj, "day - selected - double - click", G_CALLBACK ( doOnDoubleDaySelect ), opt, NULL, ret );
}

/**
\brief
\note
**/

static gboolean doOnMonthChanged ( GtkCalendar *calendar, gpointer data )
{
#ifdef DEBUG_CALENDAR
	g_print ( " % s - 1\n",__FUNCTION__ );
#endif

	GnoclCommandData *cs = ( GnoclCommandData * ) data;

	GnoclPercSubst ps[] =
		{

			{ 'd', GNOCL_INT },    /* day */
			{ 'm', GNOCL_INT },    /* month */
			{ 'y', GNOCL_INT },    /* year */
			{ 'w', GNOCL_STRING }, /* widget */
			{ 'g', GNOCL_STRING }, /* glade name */
			{ 'x', GNOCL_INT },    /* pointer x-position */
			{ 'y', GNOCL_INT },    /* pointer y-position */
			{ 's', GNOCL_STRING }, /* signal-type */
			{ 0 }
		};

	gint year, month, day;
	gchar str[20];
	gint x, y;

	gtk_calendar_get_date ( calendar , &year, &month, &day );

	gtk_widget_get_pointer  ( calendar , &x, &y );

	ps[0].val.i = day;
	ps[1].val.i = month;
	ps[2].val.i = year;
	ps[3].val.str = gnoclGetNameFromWidget ( calendar );
	ps[4].val.str = gtk_widget_get_name ( GTK_WIDGET (calendar) );
	ps[5].val.i = x;
	ps[6].val.i = y;
	ps[7].val.str = "month - changed";

	gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 );

	return 0;
}

/**
\brief
**/
int gnoclOptOnMonthChanged ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	assert ( strcmp ( opt->optName, " - onMonthChanged" ) == 0 );
	return gnoclConnectOptCmd ( interp, obj, "month - changed", G_CALLBACK ( doOnMonthChanged ), opt, NULL, ret );
}



/**
\brief      Callback function to focus change event.
**/
static gboolean doOnNextMonth ( GtkCalendar *calendar, gpointer data )
{
#ifdef DEBUG_CALENDAR
	g_print ( " % s - 1\n",__FUNCTION__ );
#endif
	GnoclCommandData *cs = ( GnoclCommandData * ) data;

	GnoclPercSubst ps[] =
		{

			{ 'd', GNOCL_INT },    /* day */
			{ 'm', GNOCL_INT },    /* month */
			{ 'y', GNOCL_INT },    /* year */
			{ 'w', GNOCL_STRING }, /* widget */
			{ 'g', GNOCL_STRING }, /* glade name */
			{ 'x', GNOCL_INT },    /* pointer x-position */
			{ 'y', GNOCL_INT },    /* pointer y-position */
			{ 's', GNOCL_STRING }, /* signal-type */
			{ 0 }
		};

	gint year, month, day;
	gchar str[20];
	gint x, y;

	gtk_calendar_get_date ( calendar , &year, &month, &day );

	gtk_widget_get_pointer  ( calendar , &x, &y );

	ps[0].val.i = day;
	ps[1].val.i = month;
	ps[2].val.i = year;
	ps[3].val.str = gnoclGetNameFromWidget ( calendar );
	ps[4].val.str = gtk_widget_get_name ( GTK_WIDGET (calendar) );
	ps[5].val.i = x;
	ps[6].val.i = y;
	ps[7].val.str = "next - month";

	gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 );

	return 0;
}
/**
\brief
**/
int gnoclOptonNextMonth ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	assert ( strcmp ( opt->optName, " - onNextMonth" ) == 0 );
	return gnoclConnectOptCmd ( interp, obj, "next - month", G_CALLBACK ( doOnNextMonth ), opt, NULL, ret );
}




/**
\brief      Callback function to focus change event.
**/
static gboolean doOnPrevMonth ( GtkCalendar *calendar, gpointer data )
{
#ifdef DEBUG_CALENDAR
	g_print ( " % s - 1\n",__FUNCTION__ );
#endif

	GnoclCommandData *cs = ( GnoclCommandData * ) data;

	GnoclPercSubst ps[] =
		{

			{ 'd', GNOCL_INT },    /* day */
			{ 'm', GNOCL_INT },    /* month */
			{ 'y', GNOCL_INT },    /* year */
			{ 'w', GNOCL_STRING }, /* widget */
			{ 'g', GNOCL_STRING }, /* glade name */
			{ 'x', GNOCL_INT },    /* pointer x-position */
			{ 'y', GNOCL_INT },    /* pointer y-position */
			{ 's', GNOCL_STRING }, /* signal-type */
			{ 0 }
		};

	gint year, month, day;
	gchar str[20];
	gint x, y;

	gtk_calendar_get_date ( calendar , &year, &month, &day );

	gtk_widget_get_pointer  ( calendar , &x, &y );

	ps[0].val.i = day;
	ps[1].val.i = month;
	ps[2].val.i = year;
	ps[3].val.str = gnoclGetNameFromWidget ( calendar );
	ps[4].val.str = gtk_widget_get_name ( GTK_WIDGET (calendar) );
	ps[5].val.i = x;
	ps[6].val.i = y;
	ps[7].val.str = "prev - month";

	gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 );

	return 0;
}
/**
\brief
**/
int gnoclOpOnPrevMonth ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	assert ( strcmp ( opt->optName, " - onPrevMonth" ) == 0 );
	return gnoclConnectOptCmd ( interp, obj, "prev - month", G_CALLBACK ( doOnPrevMonth ), opt, NULL, ret );
}



/**
\brief      Callback function to focus change event.
**/
static gboolean doOnNextYear ( GtkCalendar *calendar, gpointer data )
{
#ifdef DEBUG_CALENDAR
	g_print ( " % s - 1\n",__FUNCTION__ );
#endif

	GnoclCommandData *cs = ( GnoclCommandData * ) data;

	GnoclPercSubst ps[] =
		{

			{ 'd', GNOCL_INT },    /* day */
			{ 'm', GNOCL_INT },    /* month */
			{ 'y', GNOCL_INT },    /* year */
			{ 'w', GNOCL_STRING }, /* widget */
			{ 'g', GNOCL_STRING }, /* glade name */
			{ 'x', GNOCL_INT },    /* pointer x-position */
			{ 'y', GNOCL_INT },    /* pointer y-position */
			{ 's', GNOCL_STRING }, /* signal-type */
			{ 0 }
		};

	gint year, month, day;
	gchar str[20];
	gint x, y;

	gtk_calendar_get_date ( calendar , &year, &month, &day );

	gtk_widget_get_pointer  ( calendar , &x, &y );

	ps[0].val.i = day;
	ps[1].val.i = month;
	ps[2].val.i = year;
	ps[3].val.str = gnoclGetNameFromWidget ( calendar );
	ps[4].val.str = gtk_widget_get_name ( GTK_WIDGET (calendar) );
	ps[5].val.i = x;
	ps[6].val.i = y;
	ps[7].val.str = "next - year";

	gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 );

	return 0;
}

/**
\brief
**/
int gnoclOptOnNextYear ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	assert ( strcmp ( opt->optName, " - onNextYear" ) == 0 );
	return gnoclConnectOptCmd ( interp, obj, "next - year", G_CALLBACK ( doOnNextYear ), opt, NULL, ret );
}

/**
\brief      Callback function to focus change event.
**/
static gboolean doOnPrevYear ( GtkCalendar *calendar, gpointer data )
{
#ifdef DEBUG_CALENDAR
	g_print ( " % s - 1\n",__FUNCTION__ );
#endif

	GnoclCommandData *cs = ( GnoclCommandData * ) data;

	GnoclPercSubst ps[] =
		{

			{ 'd', GNOCL_INT },    /* day */
			{ 'm', GNOCL_INT },    /* month */
			{ 'y', GNOCL_INT },    /* year */
			{ 'w', GNOCL_STRING }, /* widget */
			{ 'g', GNOCL_STRING }, /* glade name */
			{ 'x', GNOCL_INT },    /* pointer x-position */
			{ 'y', GNOCL_INT },    /* pointer y-position */
			{ 's', GNOCL_STRING }, /* signal-type */
			{ 0 }
		};

	gint year, month, day;
	gchar str[20];
	gint x, y;

	gtk_calendar_get_date ( calendar , &year, &month, &day );

	gtk_widget_get_pointer  ( calendar , &x, &y );

	ps[0].val.i = day;
	ps[1].val.i = month;
	ps[2].val.i = year;
	ps[3].val.str = gnoclGetNameFromWidget ( calendar );
	ps[4].val.str = gtk_widget_get_name ( GTK_WIDGET (calendar) );
	ps[5].val.i = x;
	ps[6].val.i = y;
	ps[7].val.str = "prev - year";

	gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 );

	return 0;
}

/**
\brief
**/
int gnoclOptOnPrevYear ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	assert ( strcmp ( opt->optName, " - onPrevYear" ) == 0 );
	return gnoclConnectOptCmd ( interp, obj, "prev - year", G_CALLBACK ( doOnPrevYear ), opt, NULL, ret );
}


/**
\brief
**/
void gnoclOptFont  ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{

	const char *font = NULL;
	GtkRcStyle *style;


	//	font = Tcl_GetStringFromObj ( opt->val.obj, NULL ) ;

	//	style = gtk_rc_style_new ();
	//	pango_font_description_free ( style->font_desc );
	//	style->font_desc = pango_font_description_from_string ( font );
	//	gtk_widget_modify_style ( obj->window, style );
}


/*
"day"                      gint                  : Read / Write
"detail - height - rows"       gint                  : Read / Write
"detail - width - chars"       gint                  : Read / Write
"month"                    gint                  : Read / Write
"no - month - change"          gboolean              : Read / Write
"show - day - names"           gboolean              : Read / Write
"show - details"             gboolean              : Read / Write
"show - heading"             gboolean              : Read / Write
"show - week - numbers"        gboolean              : Read / Write
"year"                     gint                  : Read / Write

*/

/*
"day - selected"                                   : Run First
"day - selected - double - click"                      : Run First
"month - changed"                                  : Run First
"next - month"                                     : Run First
"next - year"                                      : Run First
"prev - month"                                     : Run First
"prev - year"
*/

static GnoclOption calendarOptions[] =
	{
		/* custom propeties */
		{ " - font", GNOCL_OBJ, "", gnoclOptGdkBaseFont}, //gnoclOptSetFont}
		{ " - baseColor", GNOCL_OBJ, "normal", gnoclOptGdkColorBase },


		/* widget specific properties */
		{ " - day", GNOCL_INT, "day" },

		//{ " - month", GNOCL_INT, "month" },
		{ " - month", GNOCL_OBJ, "", gnoclOptMonth },
		{ " - year", GNOCL_INT, "year" },
		{ " - rowHeight", GNOCL_INT, "detail - height - rows"  },
		{ " - colWidth", GNOCL_INT, "detail - width - chars" },

		//{ " - noMonthChange", GNOCL_BOOL, "no - month - change" },
		{ " - monthChange", GNOCL_OBJ, "", gnoclOptShowMonthChange },
		{ " - dayNames", GNOCL_BOOL, "show - day - names" },
		{ " - details", GNOCL_BOOL, "show - details" },
		{ " - heading", GNOCL_BOOL, "show - heading" },
		{ " - weekNumbers", GNOCL_BOOL, "show - week - numbers" },

		/* widget specific signals */
		{ " - onDaySelected", GNOCL_OBJ, "", gnoclOptOnDaySelected },
		{ " - onDoubleDaySelected", GNOCL_OBJ, "", gnoclOptOnDoubleDaySelected },
		{ " - onMonthChanged", GNOCL_OBJ, "", gnoclOptOnMonthChanged },
		{ " - onNextMonth", GNOCL_OBJ, "", gnoclOptonNextMonth },
		{ " - onNextYear", GNOCL_OBJ, "", gnoclOptOnNextYear },
		{ " - onPrevMonth", GNOCL_OBJ, "", gnoclOpOnPrevMonth },
		{ " - onPrevYear", GNOCL_OBJ, "", gnoclOptOnPrevYear },

		/* Inherited properties */
		{ " - name", GNOCL_STRING, "name" },
		{ " - data", GNOCL_OBJ, "", gnoclOptData },
		{ " - name", GNOCL_STRING, "name" },
		{ " - visible", GNOCL_BOOL, "visible" },
		{ " - sensitive", GNOCL_BOOL, "sensitive" },
		{ " - tooltip", GNOCL_OBJ, "", gnoclOptTooltip },
		{ " - onShowHelp", GNOCL_OBJ, "", gnoclOptOnShowHelp },


		{ NULL },
	};

/**
\brief
\date
**/

static int configure ( Tcl_Interp *interp, CalendarParams *para, GnoclOption options[] )
{
#ifdef DEBUG_CALENDAR
	g_print ( " % s - 1\n",__FUNCTION__ );
#endif

	return TCL_OK;
}

/**
\brief
\date
**/
int gnoclConfigcalendar ( Tcl_Interp *interp, GtkWidget *widget, Tcl_Obj *txtObj )
{
#ifdef DEBUG_CALENDAR
	g_print ( " % s - 1\n",__FUNCTION__ );
#endif

	GtkArrow *arrow;

	/* the arrow is the child of the widget */

	arrow = gnoclFindChild ( GTK_WIDGET ( widget ), GTK_TYPE_ARROW ) ;
	gtk_arrow_set ( arrow, GTK_ARROW_DOWN, GTK_SHADOW_OUT );

	return TCL_OK;
}

/**
\brief
\date
**/
static int cget ( Tcl_Interp *interp, GtkWidget *widget, GnoclOption options[], int idx )
{
#ifdef DEBUG_CALENDAR
	g_print ( " % s - 1\n",__FUNCTION__ );
#endif

	const char *dataIDa = "gnocl::data1";
	const char *dataIDb = "gnocl::data2";

	GtkWidget *arrow = gnoclFindChild ( GTK_WIDGET ( widget ), GTK_TYPE_BUTTON );
	char *str;

	/* this will enable us to obtain widget data for the arrow object */
	char *result = g_object_get_data ( G_OBJECT ( arrow ), dataIDa );


#ifdef DEBUG_CALENDAR
	g_print ( "cget result = % s\n", result );
#endif

	str = gnoclGetNameFromWidget ( arrow );


#ifdef DEBUG_CALENDAR
	g_print ( "configure % s\n", str );
#endif

	/*-----*/
	/*
	Tcl_Obj *obj = NULL;

	if ( idx == textIdx )
	{
	obj = gnoclCgetButtonText ( interp, widget );
	}

	else if ( idx == iconsIdx )
	{
	GtkWidget *image = gnoclFindChild ( GTK_WIDGET ( widget ), GTK_TYPE_IMAGE );

	if ( image == NULL )
	obj = Tcl_NewStringObj ( "", 0 );
	else
	{
	gchar   *st;
	g_object_get ( G_OBJECT ( image ), "stock", &st, NULL );

	if ( st )
	{
	obj = Tcl_NewStringObj ( " % #", 2 );
	Tcl_AppendObjToObj ( obj, gnoclGtkToStockName ( st ) );
	g_free ( st );
	}

	else
	{
	Tcl_SetResult ( interp, "Could not determine icon type.",
	TCL_STATIC );
	return TCL_ERROR;
	}
	}
	}

	if ( obj != NULL )
	{
	Tcl_SetObjResult ( interp, obj );
	return TCL_OK;
	}
	*/


	return gnoclCgetNotImplemented ( interp, options + idx );
}

/**
\brief
\date
**/
int calendarFunc ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{

#ifdef DEBUG_CALENDAR
	g_print(" % s - 1\n",__FUNCTION__);
#endif



	static const char *cmds[] =
		{
			"detail", "delete", "configure", "cget",
			"get", "set", "class", "mark",
			NULL
		};

	enum cmdIdx
	{
		DetailIdx, DeleteIdx, ConfigureIdx, CgetIdx,
		GetIdx, SetIdx,	ClassIdx, MarkIdx
	};

	//GtkWidget *widget = GTK_WIDGET ( data );

	CalendarParams *para = ( CalendarParams * ) data;

	int idx;

	/*
	if ( objc < 2 )
	{
	Tcl_WrongNumArgs ( interp, 1, objv, "command" );
	return TCL_ERROR;
	}
	*/
	if ( Tcl_GetIndexFromObj ( interp, objv[1], cmds, "command", TCL_EXACT, &idx ) != TCL_OK )
	{
		return TCL_ERROR;
	}

	switch ( idx )
	{
		case DetailIdx:
			{

				if ( !strcmp ( Tcl_GetString ( objv[2] ), "add" ) )
				{
#ifdef DEBUG_CALENDAR
					g_print(" % s - DetailIdx - 1\n",__FUNCTION__);
#endif
					gint ret;
					gchar *date = NULL;
					gchar *text = NULL;

					gint day, month, year;

					if ( !strcmp ( Tcl_GetString ( objv[3] ), " - date" ) )
					{
						date = Tcl_GetString ( objv[4] );

						if ( !strcmp ( Tcl_GetString ( objv[5] ), " - text" ) )
						{
							text = Tcl_GetString ( objv[6] );
						}

					}


					if ( !strcmp ( Tcl_GetString ( objv[3] ), " - detail" ) )
					{
#ifdef DEBUG_CALENDAR
						g_print(" % s - DetailIdx - 2\n",__FUNCTION__);
#endif
						text = Tcl_GetString ( objv[4] );

						if ( !strcmp ( Tcl_GetString ( objv[5] ), " - text" ) )
						{
							date = Tcl_GetString ( objv[6] );
						}


					}

#ifdef DEBUG_CALENDAR
					g_print(" % s - DetailIdx - 3\n",__FUNCTION__);
					g_print ( "date add % s\n", date );
					g_print ( "text add % s\n", text );
#endif

					/* Places a visual marker on a particular day. */
					gtk_calendar_set_detail_func ( para->calendar, calendar_detail_cb, data, NULL );

					sscanf ( date, " % d % d % d", &day, &month, &year );

					calendar_set_detail ( para, year, month, day, text );


				}

				if ( !strcmp ( Tcl_GetString ( objv[2] ), "remove" ) )
				{

#ifdef DEBUG_CALENDAR
					g_print(" % s - DetailIdx - 4\n",__FUNCTION__);
#endif
					gint ret;
					gint year, month, day;
					sscanf ( Tcl_GetString ( objv[3] ), " % d % d % d", &day, &month, &year );


#ifdef DEBUG_CALENDAR
					g_print(" % s - DetailIdx - 4\n",__FUNCTION__);
					g_print ( "detail remove % d\n", day );
#endif
					/* Places a visual marker on a particular day. */
					ret = gtk_calendar_mark_day ( para->calendar, day );
				}

#ifdef DEBUG_CALENDAR
				g_print(" % s - DetailIdx - 5\n",__FUNCTION__);
#endif
			}
			break;
		case ClassIdx:
			{
				Tcl_SetObjResult ( interp, Tcl_NewStringObj ( "calendar", -1 ) );
			}
			break;
		case DeleteIdx:
			{
				return gnoclDelete ( interp, GTK_WIDGET ( para->calendar ), objc, objv );
			}
			break;
		case ConfigureIdx:
			{

#ifdef DEBUG_CALENDAR
				g_print ( " % s - ConfigureIdx\n",__FUNCTION__ );
#endif
				int ret = TCL_ERROR;

				if ( gnoclParseOptions ( interp, objc-1, objv + 1, calendarOptions ) == TCL_OK )
				{
					ret = gnoclSetOptions ( interp, calendarOptions, G_OBJECT ( para->calendar ), -1 );
					ret = configure ( interp, para, calendarOptions );
				}

				gnoclClearOptions ( calendarOptions );

				return ret;
			}

			break;
		case GetIdx:
			{
				if ( objc != 2 )
				{
					Tcl_WrongNumArgs ( interp, 2, objv, NULL );
					return TCL_ERROR;
				}


				gchar str[12];

				gint year, month, day;

				gtk_calendar_get_date ( para->calendar, &year, &month, &day );

				/* correct month, jan = 0, dec = 11 */
				sprintf ( str, " % d % d % d", day, month + 1, year );

				Tcl_SetObjResult ( interp, Tcl_NewStringObj ( str, -1 ) );
			}
			break;

		case SetIdx:
			{
				if ( objc != 3 )
				{
					Tcl_WrongNumArgs ( interp, 3, objv, NULL );
					return TCL_ERROR;
				}

				gint year, month, day;
				sscanf ( Tcl_GetString ( objv[2] ), " % d % d % d", &day, &month, &year );
				/*
				gboolean gtk_calendar_select_month (GtkCalendar *calendar, guint month, guint year);
				void gtk_calendar_select_day (GtkCalendar *calendar,guint day);
				gboolean gtk_calendar_mark_day (GtkCalendar *calendar, guint day);
				gboolean gtk_calendar_unmark_day (GtkCalendar *calendar, guint day);
				void gtk_calendar_clear_marks (GtkCalendar *calendar);
				*/

				/* correct month, jan = 0, dec = 11 */
				gtk_calendar_select_month ( para->calendar, month-1, year );
				gtk_calendar_select_day ( para->calendar, day );

			}
			break;

		case MarkIdx:
			{
				if ( objc <= 2  )
				{
					Tcl_WrongNumArgs ( interp, 4, objv, NULL );
					return TCL_ERROR;
				}

				/* usage <widget-id> mark set|unset [day] */

#ifdef DEBUG_CALENDAR
				g_print(" % s - MarkIdx - 1\n",__FUNCTION__);

				g_print ( "subcommand = % s\n", Tcl_GetString ( objv[2] ) );
				g_print ( "day = % d\n", Tcl_GetString ( objv[3] ) );
#endif
				gint ret;
				gint day;

				Tcl_GetIntFromObj ( NULL, objv[3], &day );

				if ( !strcmp ( Tcl_GetString ( objv[2] ), "set" ) )
				{

#ifdef DEBUG_CALENDAR
					g_print(" % s - MarkIdx - 2\n",__FUNCTION__);
					g_print ( "set $d\n", day );
#endif


					/* Places a visual marker on a particular day. */
					ret = gtk_calendar_mark_day ( para->calendar, day );

				}

				if ( !strcmp ( Tcl_GetString ( objv[2] ), "unset" ) )
				{
#ifdef DEBUG_CALENDAR
					g_print(" % s - MarkIdx - 3\n",__FUNCTION__);
					g_print ( "unset\n" );
#endif				

					/* Removes the visual marker from a particular day. */
					ret = gtk_calendar_unmark_day ( para->calendar, day );

				}


				if ( !strcmp ( Tcl_GetString ( objv[2] ), "clear" ) )
				{
#ifdef DEBUG_CALENDAR
					g_print(" % s - MarkIdx - 4\n",__FUNCTION__);
					g_print ( "clear\n" );
#endif

					/* Remove all visual markers. */
					gtk_calendar_clear_marks ( para->calendar );

				}

				/*
				gboolean gtk_calendar_mark_day (GtkCalendar *calendar, guint day);
				gboolean gtk_calendar_unmark_day (GtkCalendar *calendar, guint day);
				*/
			}
			break;

		case CgetIdx:
			{
				int     idx;

				switch ( gnoclCget ( interp, objc, objv, G_OBJECT ( para->calendar ), calendarOptions, &idx ) )
				{
					case GNOCL_CGET_ERROR:
						{
							return TCL_ERROR;
						}

					case GNOCL_CGET_HANDLED:
						{
							return TCL_OK;
						}

					case GNOCL_CGET_NOTHANDLED:
						{
							return cget ( interp, para->calendar, calendarOptions, idx );
						}
				}
			}
	}

	return TCL_OK;
}

/**
\brief
\date
**/
int gnoclCalendarCmd ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{

#ifdef DEBUG_CALENDAR
	g_print ( "gnoclCalendarCmd 1\n" );
#endif

	int ret = TCL_OK;

	CalendarParams *para;

	/* STEP 1) test the options for errors */
	if ( gnoclParseOptions ( interp, objc, objv, calendarOptions ) != TCL_OK )
	{
		gnoclClearOptions ( calendarOptions );
		return TCL_ERROR;
	}

	/* STEP 2) build and show the base object */
	para = g_new ( CalendarParams, 1 );
	para->calendar = GTK_CALENDAR ( gtk_calendar_new() );
	para->details = g_hash_table_new_full ( g_str_hash, g_str_equal, g_free, g_free );

#ifdef DEBUG_CALENDAR
	g_print ( "gnoclCalendarCmd 2\n" );
#endif

	gtk_widget_show ( GTK_WIDGET ( para->calendar ) );

	/* STEP 3) apply options */
	ret = gnoclSetOptions ( interp, calendarOptions, G_OBJECT ( para->calendar ), -1 );

	if ( ret == TCL_OK )
	{
		ret = configure ( interp, G_OBJECT ( para->calendar ), calendarOptions );
	}

	gnoclClearOptions ( calendarOptions );

	if ( ret != TCL_OK )
	{
		gtk_widget_destroy ( GTK_WIDGET ( para->calendar ) );
		return TCL_ERROR;
	}

#ifdef DEBUG_CALENDAR
	g_print ( "gnoclCalendarCmd 3\n" );
#endif

	/* STEP 4)  -everything has worked, register the widget with the Tcl interpretor. */

	para->name = gnoclGetAutoWidgetId();
	g_signal_connect ( G_OBJECT ( para->calendar ), "destroy", G_CALLBACK ( destroyFunc ), para );
	gnoclMemNameAndWidget ( para->name, GTK_WIDGET ( para->calendar ) );
	Tcl_CreateObjCommand ( interp, para->name, calendarFunc, para, NULL );
	Tcl_SetObjResult ( interp, Tcl_NewStringObj ( para->name, -1 ) );

	demonstrate_details ( para );

	return TCL_OK;
}

