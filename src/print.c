/**
\brief
\author     William J Giddings
\date       22/05/09
\notes      Only a prototype implementation -simple print command

How does it work:
    print text buffer to object...

format:
    gnocl::print    widget ｜ file | preview | text

also:
    add print support to gnocl::text widget
**/

/* print keys names
#define GTK_PRINT_SETTINGS_PRINTER          "printer"
#define GTK_PRINT_SETTINGS_ORIENTATION      "orientation"
#define GTK_PRINT_SETTINGS_PAPER_FORMAT     "paper-format"
#define GTK_PRINT_SETTINGS_PAPER_WIDTH      "paper-width"
#define GTK_PRINT_SETTINGS_PAPER_HEIGHT     "paper-height"
#define GTK_PRINT_SETTINGS_N_COPIES         "n-copies"
#define GTK_PRINT_SETTINGS_DEFAULT_SOURCE   "default-source"
#define GTK_PRINT_SETTINGS_QUALITY          "quality"
#define GTK_PRINT_SETTINGS_RESOLUTION       "resolution"
#define GTK_PRINT_SETTINGS_USE_COLOR        "use-color"
#define GTK_PRINT_SETTINGS_DUPLEX           "duplex"
#define GTK_PRINT_SETTINGS_COLLATE          "collate"
#define GTK_PRINT_SETTINGS_REVERSE          "reverse"
#define GTK_PRINT_SETTINGS_MEDIA_TYPE       "media-type"
#define GTK_PRINT_SETTINGS_DITHER           "dither"
#define GTK_PRINT_SETTINGS_SCALE            "scale"
#define GTK_PRINT_SETTINGS_PRINT_PAGES      "print-pages"
#define GTK_PRINT_SETTINGS_PAGE_RANGES      "page-ranges"
#define GTK_PRINT_SETTINGS_PAGE_SET         "page-set"
#define GTK_PRINT_SETTINGS_FINISHINGS       "finishings"
#define GTK_PRINT_SETTINGS_NUMBER_UP        "number-up"
#define GTK_PRINT_SETTINGS_NUMBER_UP_LAYOUT "number-up-layout"
#define GTK_PRINT_SETTINGS_OUTPUT_BIN       "output-bin"
#define GTK_PRINT_SETTINGS_RESOLUTION_X     "resolution-x"
#define GTK_PRINT_SETTINGS_RESOLUTION_Y     "resolution-y"
#define GTK_PRINT_SETTINGS_PRINTER_LPI      "printer-lpi"

#define GTK_PRINT_SETTINGS_OUTPUT_FILE_FORMAT  "output-file-format"
#define GTK_PRINT_SETTINGS_OUTPUT_URI          "output-uri"

#define GTK_PRINT_SETTINGS_WIN32_DRIVER_VERSION "win32-driver-version"
#define GTK_PRINT_SETTINGS_WIN32_DRIVER_EXTRA   "win32-driver-extra"

*/

#include "gnocl.h"




/* some default values */
#define HEADER_HEIGHT 20.0
#define HEADER_GAP 8.5
#define FOOTER_HEIGHT 20.0
#define FOOTER_GAP 8.5

GtkPrintSettings *settings = NULL;

/* details about the print job */

typedef struct
{
	gchar *filename;
	gchar *exportFilename;
	gdouble font_size;
	gint lines_per_page;
	gchar **lines;
	gint total_lines;
	gint total_pages;
	gchar *font;
	gint line_padding;
	gchar *header;
	gchar *footer;
	gint showProgress;
	/* wjg added */
	gint show_page_num;
	gint show_header;
	gint show_footer;
	gint page_start_num;
	gdouble margin_top;
	gdouble margin_bottom;
	gdouble margin_left;
	gdouble margin_right;
	gint print_bannner;
	gdouble page_width;
	gdouble page_height;
	gchar units;
} PrintData;

/* user data */

typedef struct
{
	GtkWidget  *window;
	GtkWidget  *chooser;
	PrintData  *data;
} Widgets;

//GtkPrintSettings *settings;


static void print_file ( gchar * filename, Widgets * w, gint showDialog );
static void begin_print ( GtkPrintOperation*, GtkPrintContext*, Widgets* );
static void draw_page ( GtkPrintOperation*, GtkPrintContext*, gint, Widgets* );
static void end_print ( GtkPrintOperation*, GtkPrintContext*, Widgets* );
static int print_settings ( GtkPrintSettings *settings, Tcl_Interp *interp, int i, int objc, Tcl_Obj * const objv[] );

static int gnoclGetLength ( gchar *str, float *val, gint *unit );

/*
	These are to be parsed at the begining of the command call

	Create globally available structure to hold parameters
*/

static void doSettingsList ( const gchar *key, const gchar *value, gpointer user_data )
{
	g_print ( "%s %s\n", key, value );
}

static GnoclOption options[] =
{
	{ NULL, GNOCL_STRING, NULL },
	{ NULL },
};

static const char *cmds[] =
{
	"widget", "file",
	"preview", "configure",
	"get", "cget",
	NULL
};

/**
\brief	Implement the the gnocl::print command
**/
int gnoclPrintCmd ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{

	if ( gnoclGetCmdsAndOpts ( interp, cmds, options, objv, objc ) == TCL_OK )
	{
		return TCL_OK;
	}


#ifdef DEBUG_PRINT
	debugStep ( __FUNCTION__, 1 );
#endif




	enum optIdx
	{
		WidgetIdx, FileIdx,
		PreviewIdx, ConfigureIdx,
		GetIdx, CgetIdx

	};


	/* initialize some values */
	Widgets *w;
	/* allocate some memory to hold pointers */
	w = g_slice_new ( Widgets );
	w->data = g_slice_new ( PrintData );

	gchar *fileName = NULL;
	gchar font[32];
	float font_size;
	gint line_padding;

	int i;
	int idx;

	gchar *str1;
	gchar *str2;

	/* set some defaults */
	w->data->font = "Serif";
	w->data->font_size = 10.0;
	w->data->line_padding = 1;
	w->data->header = "Gnocl";
	w->data->exportFilename = "outfile.pdf";
	w->data->showProgress = 0;


	debugStep ( "Settings=NULL", 1 );

	if ( settings == NULL )
	{
		settings = gtk_print_settings_new();
	}

	else
	{
		g_print ( "Settings Exist\n" );
	}

	debugStep ( __FUNCTION__, 2 );

	debugStep ( __FUNCTION__, 3 );

	if ( Tcl_GetIndexFromObj ( interp, objv[1], cmds, "command", TCL_EXACT, &idx ) != TCL_OK )
	{
		Tcl_WrongNumArgs ( interp, 1, objv, "[widget | file | preview | settings | configure | get | cget]\n" );
		return TCL_ERROR;
	}

	debugStep ( __FUNCTION__, 4 );

	switch ( idx )
	{
		case GetIdx:
			{
				g_print ( "get\n" );

				//void (*GtkPrintSettingsFunc) (const gchar *key, const gchar *value, gpointer user_data);
				//gtk_print_settings_foreach (GtkPrintSettings *settings, GtkPrintSettingsFunc func, gpointer user_data);
				gtk_print_settings_foreach ( settings, doSettingsList, NULL );
				return TCL_OK;
			}
			break;
		case ConfigureIdx:
			{
#ifdef DEBUG_PRINT
				g_print ( "ConfigureIdx %s\n", Tcl_GetString ( objv[1] ) );
#endif

				print_settings ( settings, interp, 1, objc, objv );
			}
			break;
		case WidgetIdx:
			{

				g_print ( "%s 1 %s\n", __FUNCTION__, Tcl_GetString ( objv[2] ) );


				if ( objc < 3 )
				{
					Tcl_WrongNumArgs ( interp, 1, objv, "option widgetid " );
					return TCL_ERROR;
				}


				g_print ( "print widget 2\n" );


				GtkScrolledWindow   *scrolled = gnoclGetWidgetFromName ( Tcl_GetString ( objv[2] ), interp );
				GtkTextView     *text = GTK_TEXT_VIEW ( gtk_bin_get_child ( GTK_BIN ( scrolled ) ) );
				GtkTextBuffer  *buffer = gtk_text_view_get_buffer ( text );

				char *txt;

				g_print ( "print widget 3\n" );

				GtkTextIter startIter, endIter;

				g_print ( "print widget 4\n" );


				if ( posToIter ( interp, objv[3], buffer, &startIter ) != TCL_OK )
				{
					return TCL_ERROR;
				}


				g_print ( "print widget 5\n" );

				if ( posToIter ( interp, objv[4], buffer, &endIter ) != TCL_OK )
				{
					return TCL_ERROR;
				}


				g_print ( "print widget 6\n" );

				txt = gtk_text_buffer_get_text ( buffer, &startIter, &endIter, 1 );

				g_print ( "print widget 7\n%s", txt );

				FILE *fp;
				fp = fopen ( "~prntmp~", "w" );
				fprintf ( fp, "%s", txt );
				fclose ( fp );

				print_file ( "~prntmp~", w, 1 );

				remove ( "~prntmp~" );

			}

			break;

			/* print text file from disk */
		case FileIdx:
			{
				/* code from Krause */
				//GtkWidget *hbox, *print;

				listParameters ( objc, objv, "FileIdx" );

				fileName = Tcl_GetString ( objv[2] );

				gint action;
				action = GTK_PRINT_OPERATION_ACTION_EXPORT;

				/* parse and set parameters */

				static const char *printOptions[] =
				{
					"-header", "-footer",
					"-linePadding",
					"-baseFont",
					"-showPageNum", "-showHeader", "-showFooter",
					"-pageStartNum",
					"-marginTop", "-marginBottom", "-marginLeft", "-marginRight",
					"-banner", "-printBanner",
					"-pageWidth", "-pageHeight",
					"-units", "-action", "-exportFilename",
					"-showProgress",
					NULL
				};

				enum optsIdx
				{
					HeaderIdx, FooterIdx,
					LinePaddingIdx,
					BaseFontIdx,
					ShowPageNumIdx, ShowHeaderIdx, ShowFooterIdx,
					PageStartNumIdx,
					MarginTopIdx, MarginBottomIdx, MarginLeftIdx, MarginRightIdx,
					BannerIdx, PrintBannerIdx,
					PageWidthIdx, PageHeightIdx,
					UnitsIdx, ActionIdx, ExportFileNameIdx,
					ShowProgressIdx
				};

				int idx2;

				/* parse all the options */
				for ( i = 3 ; i < objc ; i += 2 )
				{
					getIdx ( printOptions, Tcl_GetString ( objv[i] ), &idx2 );

					switch ( idx2 )
					{
						case ShowProgressIdx:
							{
								gint show;
								Tcl_GetIntFromObj ( NULL, objv[i+1], &show );
								w->data->showProgress = show;
							}
							break;
						case ExportFileNameIdx:
							{
								debugStep ( "ExportFileNameIdx", 1 );
								w->data->exportFilename = Tcl_GetString ( objv[i+1] );
							}
							break;


							/*
													case PageSizeIdx:
														{

															g_print ( "PageSizeIdx\n" );
															static const char *sizes[] =
															{
																"A3", "A4", "A5",
																"B5",
																"letter", "executive", "legal",
																NULL
															};


							g_print("paper size = %s\n",  gtk_print_settings_get_paper_size (settings) );


															enum SizesIdx
															{
																A3Idx, A4Idx, A5Idx,
																B5Idx,
																LetterIdx, ExecutiveIdx, LegalIdx
															};

															gint psIdx;

															getIdx ( sizes, Tcl_GetString ( objv[i+1] ), &psIdx );

															switch ( psIdx )
															{
																case A3Idx:
																	{
																		g_print ( "set to A5\n" );
																		gtk_print_settings_set_paper_size (settings, GTK_PAPER_NAME_A3 );
																	} break;
																case A4Idx:
																	{
																		g_print ( "set to A4\n" );
																		gtk_print_settings_set_paper_size (settings, "iso_a4");
																		g_print ( "set to A4\n" );
																	} break;
																case A5Idx:
																	{
																		gtk_print_settings_set_paper_size (settings, GTK_PAPER_NAME_A5 );
																	} break;
																case B5Idx:
																	{
																		gtk_print_settings_set_paper_size (settings, GTK_PAPER_NAME_B5 );
																	} break;
																case LetterIdx:
																	{
																		gtk_print_settings_set_paper_size (settings, GTK_PAPER_NAME_LETTER );
																	} break;
																case ExecutiveIdx:
																	{
																		gtk_print_settings_set_paper_size (settings, GTK_PAPER_NAME_EXECUTIVE );
																	} break;
																case LegalIdx:
																	{
																		gtk_print_settings_set_paper_size (settings, GTK_PAPER_NAME_LEGAL );
																	} break;
																default: {}
															}
														}
														break;
							*/
						case ActionIdx:
							{

								debugStep ( "ActionIdx", 1 );
								debugStep ( Tcl_GetString ( objv[i+1] ), 1 );

								static const char *actions[] =
								{
									"dialog", "print", "preview", "export",
									NULL
								};

								enum SizesIdx
								{
									DialogIdx, PrintIdx, PreviewIdx, ExportIdx,
								};

								gint aIdx;

								getIdx ( actions, Tcl_GetString ( objv[i+1] ), &aIdx );

								switch ( aIdx )
								{
									case DialogIdx:
										{
											action = GTK_PRINT_OPERATION_ACTION_PRINT_DIALOG;
										} break;
									case PrintIdx:
										{
											action = GTK_PRINT_OPERATION_ACTION_PRINT;
										} break;
									case PreviewIdx:
										{
											action = GTK_PRINT_OPERATION_ACTION_PREVIEW;
										} break;
									case ExportIdx:
										{
											action = GTK_PRINT_OPERATION_ACTION_EXPORT;
										} break;
									default:
										{
											action = GTK_PRINT_OPERATION_ACTION_PRINT_DIALOG;
										}
								}

							}
							break;
						case HeaderIdx:
							{
								w->data->header = Tcl_GetString ( objv[i+1] );
							}
							break;
						case FooterIdx:
							{
								w->data->footer = Tcl_GetString ( objv[i+1] );
							}
							break;
						case LinePaddingIdx:
							{
								gint padding;
								Tcl_GetIntFromObj ( NULL, objv[i+1], &padding );
								w->data->line_padding = padding;
							}
							break;
						case BaseFontIdx:
							{
								/* obtain values from a list */
								Tcl_Obj *pobj;
								gdouble size;
								/* font */
								Tcl_ListObjIndex ( interp, objv[i+1], 0, &pobj );
								w->data->font = Tcl_GetString ( pobj );
								/* size */
								Tcl_ListObjIndex ( interp, objv[i+1], 1, &pobj ) ;
								Tcl_GetDoubleFromObj ( interp, pobj, &size );
								w->data->font_size = size;
							}
							break;
						case ShowPageNumIdx:
							{
								gint showNum;
								Tcl_GetIntFromObj ( NULL, objv[i+1], &showNum );
								w->data->show_page_num = showNum;
							} break;
						case ShowHeaderIdx:
							{
								gint showHdr;
								Tcl_GetIntFromObj ( NULL, objv[i+1], &showHdr );
								w->data->show_header = showHdr;
							} break;
						case ShowFooterIdx:
							{
								gint showFtr;
								Tcl_GetIntFromObj ( NULL, objv[i+1], &showFtr );
								w->data->show_footer = showFtr;
							} break;
						case PageStartNumIdx:
							{
								gint startNum;
								Tcl_GetIntFromObj ( NULL, objv[i+1], &startNum );
								w->data->page_start_num = startNum;
							} break;
						case MarginTopIdx:
							{
								double margin;

								Tcl_GetDoubleFromObj (  NULL, objv[i+1], &margin );

								w->data->margin_top = margin;
							} break;
						case MarginBottomIdx:
							{
								double margin;

								Tcl_GetDoubleFromObj (  NULL, objv[i+1], &margin );

								w->data->margin_bottom = margin;
							} break;
						case MarginLeftIdx:
							{
								double margin;

								Tcl_GetDoubleFromObj (  NULL, objv[i+1], &margin );

								w->data->margin_left = margin;
							} break;
						case MarginRightIdx:
							{
								double margin;

								Tcl_GetDoubleFromObj (  NULL, objv[i+1], &margin );

								w->data->margin_right = margin;
							} break;
						case BannerIdx: {} break;
						case PrintBannerIdx:
							{
								gint banner;
								Tcl_GetIntFromObj ( NULL, objv[i+1], &banner );
								w->data->print_bannner = banner;
							} break;
						case PageWidthIdx:
							{
								double width;

								Tcl_GetDoubleFromObj (  NULL, objv[i+1], &width );

								w->data->page_width = width;
							} break;
						case PageHeightIdx:
							{
								double height;

								Tcl_GetDoubleFromObj (  NULL, objv[i+1], &height );

								w->data->page_height = height;

							} break;
						case UnitsIdx:
							{
								g_print ( "UnitsIdx = %s\n", Tcl_GetString ( objv[i+1] ) );

								if ( getUnits ( Tcl_GetString ( objv[i+1] ) ) )
								{
									w->data->units = Tcl_GetString ( objv[i+1] );
								}

								else
								{
									Tcl_SetResult ( interp, "Invalid unit. Must be one of pixel, points, inch or mm.", TCL_STATIC );
									return TCL_ERROR;
								}


							} break;
						default: {}
					}
				} /* end of for-loop */


				if ( exists ( fileName ) )
				{
					print_file ( fileName, w , action );
				}

				else
				{
					return TCL_ERROR;
				}
			}

			return TCL_OK;
			//break;
		case PreviewIdx:
			{
#ifdef DEBUG_PRINT
				g_print ( "Preview\n" );
#endif

				if ( objc <= 3 )
				{
					Tcl_WrongNumArgs ( interp, 1, objv, "option widgetid " );
					return TCL_ERROR;
				}

			}
	}

	debugStep ( __FUNCTION__, 4 );

	return TCL_OK;

}


/**
/brief  Print Button callback function
    GtkButton   *button     Pointer to the widget that called this function.
    Widgets     *w          User data.
**/
/* Print the selected file with a font of "Monospace 10". */
static void print_file ( gchar * filename, Widgets * w, gint action )
{

	debugStep ( __FUNCTION__, 1 );

	GtkPrintOperation *operation;
	GtkWidget *dialog;
	GError *error = NULL;

	gint res;

	if ( filename == NULL )
	{
		return;
	}

	/* Create a new print operation, applying saved print settings if they exist. */
	operation = gtk_print_operation_new ();
	debugStep ( __FUNCTION__, 2 );

	/* how to get values from settings? */
	if ( settings != NULL )
	{
		gtk_print_operation_set_print_settings ( operation, settings );

		g_print ( "ORIENTATION = %d\n", gtk_print_settings_get_orientation ( settings ) );
		g_print ( "SIZE = %d\n", gtk_print_settings_get_paper_size ( settings ) );
		g_print ( "SIZE = %s\n", gtk_print_settings_get_paper_size ( settings ) );

	}

	else
	{
		g_print ( "settings no use!\n" );
	}

	//w->data = g_slice_new ( PrintData );

	w->data->filename = g_strdup ( filename );

	debugStep ( __FUNCTION__, 3 );

	g_signal_connect ( G_OBJECT ( operation ), "begin_print", G_CALLBACK ( begin_print ), ( gpointer ) w );

	debugStep ( "begin_print", 4 );

	g_signal_connect ( G_OBJECT ( operation ), "draw_page", G_CALLBACK ( draw_page ), ( gpointer ) w );

	g_signal_connect ( G_OBJECT ( operation ), "end_print", G_CALLBACK ( end_print ), ( gpointer ) w );

	/* Run the default print operation that will print the selected file. */
	//res = gtk_print_operation_run ( operation, GTK_PRINT_OPERATION_ACTION_PRINT_DIALOG, GTK_WINDOW ( w->window ), &error );

	gtk_print_operation_set_show_progress ( operation, w->data->showProgress );

	if ( action == GTK_PRINT_OPERATION_ACTION_EXPORT )
	{
		gtk_print_operation_set_export_filename ( operation, w->data->exportFilename );
	}

	res = gtk_print_operation_run ( operation, action, NULL, &error );

	debugStep ( __FUNCTION__, 7 );

	debugStep ( __FUNCTION__, 10 );

	g_object_unref ( operation );

}


/**
\brief	Begin the printing by retrieving the contents of the selected files and spliting it into single lines of text.
**/
static void begin_print ( GtkPrintOperation * operation, GtkPrintContext * context, Widgets * w )
{
#ifdef DEBUG_PRINT
	g_print ( "begin_print\n" );
#endif
	gchar *contents;
	gdouble height;
	gsize length;

	/* Retrieve the file contents and split it into lines of text. */
	g_file_get_contents ( w->data->filename, &contents, &length, NULL );
	w->data->lines = g_strsplit ( contents, "\n", 0 );

	/* Count the total number of lines in the file. */
	w->data->total_lines = 0;

	while ( w->data->lines[w->data->total_lines] != NULL )
	{
		w->data->total_lines++;
	}

	/* Based on the height of the page and font size, calculate how many lines can be
	 * rendered on a single page. A padding of 3 is placed between lines as well. */
	height = gtk_print_context_get_height ( context ) - HEADER_HEIGHT - HEADER_GAP;

	w->data->lines_per_page = floor ( height / ( w->data->font_size + w->data->line_padding ) );

	w->data->total_pages = ( w->data->total_lines - 1 ) / w->data->lines_per_page + 1;

	gtk_print_operation_set_n_pages ( operation, w->data->total_pages );

	g_free ( contents );


	/* send info to the interpretor */
	/*
		GnoclCommandData *cs = ( GnoclCommandData * ) user_data;

		GnoclPercSubst ps[] =
		{
			{ 'c', GNOCL_INT },
			{ 0 }
		};

		ps[0].val.i = 1;

		gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 );
	*/
}

/**
\brief 	Draw the page, which includes a header with the file name and page number along
		with one page of text with a font of "Monospace 10"
**/
static void draw_page ( GtkPrintOperation * operation, GtkPrintContext * context, gint page_nr, Widgets * w )
{
#ifdef DEBUG_PRINT
	g_print ( "draw_page\n" );
#endif

	cairo_t *cr;
	PangoLayout *layout;
	gdouble width, text_height;
	gint line, i, text_width, layout_height;
	PangoFontDescription *desc;
	gchar *page_str;

#ifdef DEBUG_PRINT
	g_print ( "draw_page -1 %s %d\n", w->data->font, w->data->font_size );
#endif
	cr = gtk_print_context_get_cairo_context ( context );
	width = gtk_print_context_get_width ( context );
	layout = gtk_print_context_create_pango_layout ( context );

	desc = pango_font_description_from_string ( w->data->font );
	pango_font_description_set_size ( desc, w->data->font_size * PANGO_SCALE );

#ifdef DEBUG_PRINT
	g_print ( "draw_page -2  Render the page header with the filename and page number.\n" );
#endif
	pango_layout_set_font_description ( layout, desc );
	pango_layout_set_text ( layout, w->data->header, -1 );
	pango_layout_set_width ( layout, -1 );
	pango_layout_set_alignment ( layout, PANGO_ALIGN_LEFT );
	pango_layout_get_size ( layout, NULL, &layout_height );
	text_height = ( gdouble ) layout_height / PANGO_SCALE;

	/* draw the header */
	cairo_move_to ( cr, 0, ( HEADER_HEIGHT - text_height ) / 2 );
	pango_cairo_show_layout ( cr, layout );

	/* draw the page number */
	page_str = g_strdup_printf ( "%d of %d", page_nr + 1, w->data->total_pages );
	pango_layout_set_text ( layout, page_str, -1 );
	pango_layout_get_size ( layout, &text_width, NULL );
	pango_layout_set_alignment ( layout, PANGO_ALIGN_RIGHT );

	cairo_move_to ( cr, width - ( text_width / PANGO_SCALE ), ( HEADER_HEIGHT - text_height ) / 2 );
	pango_cairo_show_layout ( cr, layout );

	/* draw the footer */
	//height = gtk_print_context_get_height ( context ) - HEADER_HEIGHT - HEADER_GAP;
	//cairo_move_to ( cr, width - ( text_width / PANGO_SCALE ), ( FOOTER_HEIGHT - text_height ) / 2 );

	g_print ( "page height = %f px\n", gtk_print_context_get_height ( context ) );

	cairo_move_to ( cr, 0, gtk_print_context_get_height ( context ) );
	pango_layout_set_text ( layout, w->data->footer, -1 );
	pango_cairo_show_layout ( cr, layout );

#ifdef DEBUG_PRINT
	g_print ( "draw_page -4 Render the page text with the specified font and size. \n" );
#endif

	/* Render the page text with the specified font and size. */
	cairo_move_to ( cr, 0, HEADER_HEIGHT + HEADER_GAP );
	line = page_nr * w->data->lines_per_page;
	pango_cairo_show_layout ( cr, layout );


	for ( i = 0; i < w->data->lines_per_page && line < w->data->total_lines; i++ )
	{
		pango_layout_set_text ( layout, w->data->lines[line], -1 );
		pango_cairo_show_layout ( cr, layout );
		cairo_rel_move_to ( cr, 0, w->data->font_size +  w->data->line_padding );
		line++;
	}


#ifdef DEBUG_PRINT
	g_print ( "draw_page -5\n" );
#endif

	g_free ( page_str );

	g_object_unref ( layout );
	pango_font_description_free ( desc );
#ifdef DEBUG_PRINT
	g_print ( "draw_page -6\n" );
#endif
}

/**
\brief
**/
/* Clean up after the printing operation since it is done. */
static void end_print ( GtkPrintOperation * operation, GtkPrintContext * context, Widgets * w )
{
#ifdef DEBUG_PRINT
	g_print ( "%s\n", __FUNCTION__ );
#endif

	g_strfreev ( w->data->lines );
	g_slice_free1 ( sizeof ( PrintData ), w->data );
	w->data = NULL;
}

/**
\brief	Parse and set printer settings.
**/
static int print_settings ( GtkPrintSettings * settings, Tcl_Interp * interp, int i, int objc, Tcl_Obj * const objv[] )
{
#ifdef DEBUG_PRINT
	g_print ( "print_settings %d %d\n", i, objc );
#endif

	static const char *options[] =
	{
		"-orientation", "-paperSize", "-paperWidth", "-paperHeight",
		"-useColor", "-collate", "-duplex", "-quality",
		"-copies", "-numberUp", "-layout", "-resolution",
		"-xyRes", "-printerLPI", "-scale", "-pageRange",
		"-page", "-defaultSource", "-media", "-dither",
		"-finishing", "-outputBin", "-baseFont",
		NULL
	};
	enum optIdx
	{
		OrientationIdx, PaperSizeIdx, PaperWidthIdx, PaperHeightIdx,
		UseColorIdx, CollateIdx, DuplexIdx, QualityIdx,
		CopiesIdx, NumberUpIdx, LayoutIdx, ResolutionIdx,
		XYResIdx, PrinterLPIIdx, ScaleIdx, PageRangeIdx,
		PageIdx, DefaultSourceIdx, MediaIdx, DitherIdx,
		FinishingIdx, OutputBinIdx, BaseFontIdx
	};

	int idx, idx2;
	gint flag;

	/* parse all the options */
	for ( i = i + 1 ; i < objc ; i += 2 )
	{

#ifdef DEBUG_PRINT
		g_print ( "%d %s %s objc = %d\n", i, Tcl_GetString ( objv[i] ), Tcl_GetString ( objv[i+1] ), objc );
#endif

		getIdx ( options, Tcl_GetString ( objv[i] ), &idx );

		switch ( idx )
		{
			case BaseFontIdx:
				{
					g_print ( "BaseFontIdx %s\n", Tcl_GetString ( objv[i+1] ) );

					/* obtain values from a list */
					Tcl_Obj *pobj;

					/* font */
					Tcl_ListObjIndex ( interp, objv[i+1], 0, &pobj );
					gtk_print_settings_set ( settings, "font", Tcl_GetString ( pobj ) );

					/* size */
					Tcl_ListObjIndex ( interp, objv[i+1], 1, &pobj ) ;
					gtk_print_settings_set ( settings, "font-size", Tcl_GetString ( pobj ) );


				}
				break;
			case OrientationIdx:
				{
#ifdef DEBUG_PRINT
					g_print ( "OrientationIdx %s\n", Tcl_GetString ( objv[i+1] ) );
#endif

					/**
					  GTK_PAGE_ORIENTATION_PORTRAIT,
					  GTK_PAGE_ORIENTATION_LANDSCAPE,
					  GTK_PAGE_ORIENTATION_REVERSE_PORTRAIT,
					  GTK_PAGE_ORIENTATION_REVERSE_LANDSCAPE
					**/

					static const char *orientation[] =
					{
						"portrait", "landscape",
						"reverse-portrait", "reverse-landscape",
						NULL
					};

					enum orientationIdx
					{
						PortraitIdx, LandscapeIdx,
						ReversePortraitIdx, ReverserLandscapeIdx
					};


					if ( Tcl_GetIndexFromObj ( interp, objv[i+1], orientation, "command", TCL_EXACT, &idx2 ) != TCL_OK )
					{
						return TCL_ERROR;
					}

					switch ( idx2 )
					{
						case GTK_PAGE_ORIENTATION_PORTRAIT:
							{
								g_print ( ">>>>%d\n", idx2 );
							} break;
						case GTK_PAGE_ORIENTATION_LANDSCAPE:
							{
								g_print ( "%d\n", idx2 );
							} break;
						case GTK_PAGE_ORIENTATION_REVERSE_PORTRAIT:
							{
								g_print ( "%d\n", idx2 );
							} break;
						case GTK_PAGE_ORIENTATION_REVERSE_LANDSCAPE:
							{
								g_print ( "%d\n", idx2 );
							} break;
					}

					gtk_print_settings_set_orientation ( settings, idx2 );

				} break;
			case PaperSizeIdx:
				{
#ifdef DEBUG_PRINT
					g_print ( "PaperSizeIdx\n" );
#endif

					static const char *psize[] =
					{
						"A3", "A4", "A5",
						"B5", "letter",
						"executive", "legal",
						NULL
					};

					enum psizeIdx
					{
						A3Idx, A4Idx, A5Idx,
						B5Idx, LetterIdx,
						ExecutiveIdx, LegalIdx
					};

					if ( Tcl_GetIndexFromObj ( interp, objv[i+1], psize, "command", TCL_EXACT, &idx2 ) != TCL_OK )
					{
						return TCL_ERROR;
					}

					switch ( idx2 )
					{
						case A3Idx:
							{
								gtk_print_settings_set_paper_size ( settings, gtk_paper_size_new ( GTK_PAPER_NAME_A3 ) );
							} break;
						case A4Idx:
							{
								gtk_print_settings_set_paper_size ( settings, gtk_paper_size_new ( GTK_PAPER_NAME_A4 ) );

							} break;
						case A5Idx:
							{
								gtk_print_settings_set_paper_size ( settings, gtk_paper_size_new ( GTK_PAPER_NAME_A5 ) );
							} break;
						case B5Idx:
							{
								gtk_print_settings_set_paper_size ( settings, gtk_paper_size_new ( GTK_PAPER_NAME_B5 ) );
							} break;
						case LetterIdx:
							{
								gtk_print_settings_set_paper_size ( settings, gtk_paper_size_new ( GTK_PAPER_NAME_LETTER ) );
							} break;
						case ExecutiveIdx:
							{
								gtk_print_settings_set_paper_size ( settings, gtk_paper_size_new ( GTK_PAPER_NAME_EXECUTIVE ) );
							} break;
						case LegalIdx:
							{
								gtk_print_settings_set_paper_size ( settings, gtk_paper_size_new ( GTK_PAPER_NAME_LEGAL ) );
							} break;
					}

				} break;
			case PaperWidthIdx:
				{
					/*
					  GTK_UNIT_PIXEL,
					  GTK_UNIT_POINTS,
					  GTK_UNIT_INCH,
					  GTK_UNIT_MM
					*/

#ifdef DEBUG_PRINT
					g_print ( "PaperWidthIdx\n" );
#endif

					float width;
					gint unit;

					gnoclGetLength ( Tcl_GetString ( objv[i+1] ), &width, &unit );
					gtk_print_settings_set_paper_width  ( settings, width, unit );

				} break;
			case PaperHeightIdx:
				{
#ifdef DEBUG_PRINT
					g_print ( "PaperHeightIdx\n" );
#endif

					float height;
					gint unit;

					gnoclGetLength ( Tcl_GetString ( objv[i+1] ), &height, &unit );
					g_print ( "PaperHeightIdx height = %f unit = %d\n", height, unit );
					gtk_print_settings_set_paper_width  ( settings, height, unit );

				} break;
			case UseColorIdx:
				{
#ifdef DEBUG_PRINT
					g_print ( "UseColorIdx\n" );
#endif
					Tcl_GetIntFromObj ( NULL, Tcl_GetString ( objv[i+1] ), flag );
					gtk_print_settings_set_use_color ( settings, flag );

				} break;
			case CollateIdx:
				{
#ifdef DEBUG_PRINT
					g_print ( "CollateIdx\n" );
#endif
					Tcl_GetIntFromObj ( NULL, Tcl_GetString ( objv[i+1] ), flag );
					gtk_print_settings_set_collate      ( settings, flag );
				} break;
			case DuplexIdx:
				{
#ifdef DEBUG_PRINT
					g_print ( "DuplexIdx\n" );
#endif

					static const char *duplex[] =
					{
						"simple", "horizontal", "vertical",
						NULL
					};

					enum duplexIdx
					{
						SimpleIdx, HorizontalIdx, VerticalIdx
					};
					/*
					  GTK_PRINT_DUPLEX_SIMPLEX,
					  GTK_PRINT_DUPLEX_HORIZONTAL,
					  GTK_PRINT_DUPLEX_VERTICAL
					*/

					gint dpIdx;

					if ( Tcl_GetIndexFromObj ( interp, objv[i+1], duplex, "command", TCL_EXACT, &dpIdx ) != TCL_OK )
					{
						return TCL_ERROR;
					}

					switch ( dpIdx )
					{
						case SimpleIdx:
							{
								gtk_print_settings_set_duplex ( settings, GTK_PRINT_DUPLEX_SIMPLEX );
							} break;
						case HorizontalIdx:
							{
								gtk_print_settings_set_duplex ( settings, GTK_PRINT_DUPLEX_HORIZONTAL );
							} break;
						case VerticalIdx:
							{
								gtk_print_settings_set_duplex ( settings, GTK_PRINT_DUPLEX_VERTICAL );
							}
							break;
						default: {}
					}



				} break;
			case QualityIdx:
				{
#ifdef DEBUG_PRINT
					g_print ( "QualityIdx\n" );
#endif



					static const char *quality[] =
					{
						"low", "normal", "high", "draft",
						NULL
					};

					enum qualityIdx
					{
						LowIdx, NormalIdx, HighIdx, DraftIdx
					};

					/*
					  GTK_PRINT_QUALITY_LOW,
					  GTK_PRINT_QUALITY_NORMAL,
					  GTK_PRINT_QUALITY_HIGH,
					  GTK_PRINT_QUALITY_DRAFT
					*/

					gint qIdx;

					if ( Tcl_GetIndexFromObj ( interp, objv[i+1], quality, "command", TCL_EXACT, &qIdx ) != TCL_OK )
					{
						return TCL_ERROR;
					}

					switch ( qIdx )
					{
						case LowIdx:
							{
								gtk_print_settings_set_quality ( settings, GTK_PRINT_QUALITY_LOW );
							} break;
						case NormalIdx:
							{
								gtk_print_settings_set_quality ( settings, GTK_PRINT_QUALITY_NORMAL );
							} break;
						case HighIdx:
							{
								gtk_print_settings_set_quality ( settings, GTK_PRINT_QUALITY_HIGH );
							}
							break;
						case DraftIdx:
							{
								gtk_print_settings_set_quality ( settings, GTK_PRINT_QUALITY_DRAFT );
							}
							break;
						default: {}
					}



				} break;
			case CopiesIdx:
				{
#ifdef DEBUG_PRINT
					g_print ( "CopiesIdx\n" );
#endif

					Tcl_GetIntFromObj ( NULL, Tcl_GetString ( objv[i+1] ), flag );
					gtk_print_settings_set_n_copies      ( settings, flag );

				} break;
			case NumberUpIdx:
				{
#ifdef DEBUG_PRINT
					g_print ( "NumberUpIdx\n" );
#endif

					static const char *nup[] =
					{
						"lrtb", "lrbt", "rltb", "rlbt",
						"tblr", "tbrl", "btlr", "btrl",
						NULL
					};

					enum nupIdx
					{
						lrtbIdx, lrbtIdx, rltbIdx, rlbtIdx,
						tblrIdx, tbrlIdx, btlrIdx, btrlIDx
					};

					/*
					  GTK_NUMBER_UP_LAYOUT_LEFT_TO_RIGHT_TOP_TO_BOTTOM, < nick=lrtb >
					  GTK_NUMBER_UP_LAYOUT_LEFT_TO_RIGHT_BOTTOM_TO_TOP, < nick=lrbt >
					  GTK_NUMBER_UP_LAYOUT_RIGHT_TO_LEFT_TOP_TO_BOTTOM, < nick=rltb >
					  GTK_NUMBER_UP_LAYOUT_RIGHT_TO_LEFT_BOTTOM_TO_TOP, < nick=rlbt >

					  GTK_NUMBER_UP_LAYOUT_TOP_TO_BOTTOM_LEFT_TO_RIGHT, < nick=tblr >
					  GTK_NUMBER_UP_LAYOUT_TOP_TO_BOTTOM_RIGHT_TO_LEFT, < nick=tbrl >
					  GTK_NUMBER_UP_LAYOUT_BOTTOM_TO_TOP_LEFT_TO_RIGHT, < nick=btlr >
					  GTK_NUMBER_UP_LAYOUT_BOTTOM_TO_TOP_RIGHT_TO_LEFT  < nick=btrl >

					*/

					gint nIdx;

					if ( Tcl_GetIndexFromObj ( interp, objv[i+1], nup, "command", TCL_EXACT, &nIdx ) != TCL_OK )
					{
						return TCL_ERROR;
					}

					switch ( nIdx )
					{
						case lrtbIdx:
							{
								gtk_print_settings_set_number_up ( settings,   GTK_NUMBER_UP_LAYOUT_LEFT_TO_RIGHT_TOP_TO_BOTTOM );
							} break;
						case lrbtIdx:
							{
								gtk_print_settings_set_number_up ( settings,  GTK_NUMBER_UP_LAYOUT_LEFT_TO_RIGHT_BOTTOM_TO_TOP );
							} break;
						case rltbIdx:
							{
								gtk_print_settings_set_number_up ( settings, GTK_NUMBER_UP_LAYOUT_RIGHT_TO_LEFT_TOP_TO_BOTTOM );
							}
							break;
						case rlbtIdx:
							{
								gtk_print_settings_set_number_up ( settings,   GTK_NUMBER_UP_LAYOUT_RIGHT_TO_LEFT_BOTTOM_TO_TOP );
							}
							break;

						case tblrIdx:
							{
								gtk_print_settings_set_number_up ( settings,  GTK_NUMBER_UP_LAYOUT_TOP_TO_BOTTOM_LEFT_TO_RIGHT );
							} break;
						case tbrlIdx:
							{
								gtk_print_settings_set_number_up ( settings, GTK_NUMBER_UP_LAYOUT_TOP_TO_BOTTOM_RIGHT_TO_LEFT );
							} break;
						case btlrIdx:
							{
								gtk_print_settings_set_number_up ( settings, GTK_NUMBER_UP_LAYOUT_BOTTOM_TO_TOP_LEFT_TO_RIGHT );
							}
							break;
						case btrlIDx :
							{
								gtk_print_settings_set_number_up ( settings, GTK_NUMBER_UP_LAYOUT_BOTTOM_TO_TOP_RIGHT_TO_LEFT );
							}
							break;
						default: {}
					}

				} break;
			case LayoutIdx:
				{
#ifdef DEBUG_PRINT
					g_print ( "LayoutIdx\n" );
#endif
				} break;
			case ResolutionIdx:
				{
#ifdef DEBUG_PRINT
					g_print ( "ResolutionIdx\n" );
#endif

					Tcl_GetIntFromObj ( NULL, Tcl_GetString ( objv[i+1] ), flag );
					gtk_print_settings_set_resolution      ( settings, flag );


				} break;
			case XYResIdx:
				{
#ifdef DEBUG_PRINT
					g_print ( "XYResIdx\n" );
#endif

					gint resolution_x, resolution_y;

					sscanf ( Tcl_GetString ( objv[i+1] ), "%d %d", &resolution_x, &resolution_y );

					gtk_print_settings_set_resolution_xy ( settings, resolution_x, resolution_y );

				} break;
			case PrinterLPIIdx:
				{
#ifdef DEBUG_PRINT
					g_print ( "PrinterLPIIdx\n" );
#endif
					gdouble lpi;
					Tcl_GetDoubleFromObj ( interp, objv[i+1], &lpi );
					gtk_print_settings_set_printer_lpi  ( settings, lpi );

				} break;
			case ScaleIdx:
				{
#ifdef DEBUG_PRINT
					g_print ( "ScaleIdx\n" );
#endif
					gdouble scale;
					Tcl_GetDoubleFromObj ( interp, objv[i+1], &scale );
					gtk_print_settings_set_scale ( settings, scale );

				} break;
			case PageRangeIdx:
				{
					/*
					typedef struct {
					  gint start;
					  gint end;
					} GtkPageRange;
					*/

					GtkPageRange page_ranges;

					gint start, end;

					sscanf ( Tcl_GetString ( objv[i+1] ), "%d %d", &start, &end );

					page_ranges.start = start - 1;
					page_ranges.end = end - 1;

#ifdef DEBUG_PRINT
					g_print ( "PageRangeIdx %d - %d\n", page_ranges.start, page_ranges.end  );
#endif

					gtk_print_settings_set_page_ranges ( settings, &page_ranges, 1 );

				} break;
			case PageIdx:
				{
#ifdef DEBUG_PRINT
					g_print ( "PageIdx\n" );
#endif


					static const char *pages[] =
					{
						"all", "current", "ranges", "selection",
						NULL
					};

					enum pagesIdx
					{
						AllIdx, CurrentIdx, RangesIdx, SelectionIdx
					};

					/*
					  GTK_PRINT_PAGES_ALL,
					  GTK_PRINT_PAGES_CURRENT,
					  GTK_PRINT_PAGES_RANGES,
					  GTK_PRINT_PAGES_SELECTION
					*/

					gint pIdx;

					if ( Tcl_GetIndexFromObj ( interp, objv[i+1], pages, "command", TCL_EXACT, &pIdx ) != TCL_OK )
					{
						return TCL_ERROR;
					}

					gtk_print_settings_set_print_pages ( settings, pIdx );
					/*
										switch ( pIdx )
										{
											case AllIdx:
												{
													gtk_print_settings_set_print_pages ( settings, GTK_PRINT_PAGES_ALL );
												} break;
											case CurrentIdx:
												{
													gtk_print_settings_set_print_pages ( settings, GTK_PRINT_PAGES_CURRENT );
												} break;
											case RangesIdx:
												{
													gtk_print_settings_set_print_pages ( settings, GTK_PRINT_PAGES_RANGES );
												}
												break;
											case SelectionIdx:
												{
													gtk_print_settings_set_print_pages ( settings, GTK_PRINT_PAGES_SELECTION );
												}
												break;
											default: {}
										}
					*/

				} break;
			case DefaultSourceIdx:
				{
#ifdef DEBUG_PRINT
					g_print ( "DefaultSourceIdx = %s\n", Tcl_GetString ( objv[i+1] ) );
#endif
					gint srcIdx;

					static const char *sources[] =
					{
						"auto", "cassette", "envelope", "envelope-manual",
						"lower", "manual", "middle", "only-one", "form-source",
						"large-capacity", "large-format", "tractor", "small-format",
						NULL
					};


					if ( Tcl_GetIndexFromObj ( interp, objv[i+1], sources, "command", TCL_EXACT, &srcIdx ) != TCL_OK )
					{
						return TCL_ERROR;
					}

					gtk_print_settings_set_default_source ( settings, Tcl_GetString ( objv[i+1] ) );

				} break;
			case MediaIdx:
				{
#ifdef DEBUG_PRINT
					g_print ( "MediaIdx =%s\n", Tcl_GetString ( objv[i+1] ) );
#endif

					gint mdIdx;

					/* values obtained from:
					 	ftp://ftp.pwg.org/pub/pwg/.../cs-pwgmsn10-20020226-5101.1.pdf
					 	04/04/11
					*/
					static const char *mediaTypes[] =
					{
						"stationery", "stationery-coated", "stationery-inkjet",
						"stationery-preprinted", "stationery-letterhead", "stationery-prepunched",
						"stationery-fine", "stationery-heavyweight", "stationery-lightweight",
						"transparency", "envelope", "envelope-plain",
						"envelope-window", "continuous", "continuous-long",
						"continuous-short", "tab-stock", "pre-cut-tabs",
						"full-cut-tabs", "multi-part-form", "labels",
						"multi-layer", "screen", "screen-paged",
						"photographic", "photographic-glossy", "photographic-high-gloss",
						"photographic-semi-gloss", "photographic-satin", "photographic-matte",
						"photographic-film", "back-print-film", "cardstock", "roll",
						NULL
					};

					if ( Tcl_GetIndexFromObj ( interp, objv[i+1], mediaTypes, "command", TCL_EXACT, &mdIdx ) != TCL_OK )
					{
						return TCL_ERROR;
					}

					gtk_print_settings_set_media_type ( settings, Tcl_GetString ( objv[i+1] ) );

				} break;
			case DitherIdx:
				{
#ifdef DEBUG_PRINT
					g_print ( "DitherIdx = %s\n", Tcl_GetString ( objv[i+1] ) );
#endif
					gint dIdx;;
					static const char *ditherTypes[] =
					{
						"none", "coarse", "fine",
						"lineart", "grayscale", "error-diffusion",
						NULL
					};

					if ( Tcl_GetIndexFromObj ( interp, objv[i+1], ditherTypes, "command", TCL_EXACT, &dIdx ) != TCL_OK )
					{
						return TCL_ERROR;
					}

					gtk_print_settings_set_dither ( settings, Tcl_GetString ( objv[i+1] ) );

				} break;
			case FinishingIdx:
				{
#ifdef DEBUG_PRINT
					g_print ( "FinishingIdx\n" );
#endif

					gchar *finishings;
					finishings = Tcl_GetString ( objv[i+1] );

					gtk_print_settings_set_finishings   ( settings, finishings );

				} break;
			case OutputBinIdx:
				{
#ifdef DEBUG_PRINT
					g_print ( "OutputBinIdx\n" );
#endif
					gchar *output_bin = Tcl_GetString ( objv[i+1] );
					gtk_print_settings_set_output_bin   ( settings, output_bin );


				} break;
			default:
				{
#ifdef DEBUG_PRINT
					g_print ( "default\n" );
#endif
				} break;
		}

	}

	return TCL_OK;

}

/**
\brief	split string containing length into component value and unit
**/
static int gnoclGetLength ( gchar * str, float * val, gint * unit )
{
	gchar *ptr;
	gint pos;
	gchar tmp[8];

	if ( strstr ( str, "pxs" ) != NULL )
	{
		ptr = strstr ( str, "pxs" );
		pos = ( ptr - str );
		*unit = GTK_UNIT_PIXEL;
	}

	else if ( strstr ( str, "pts" ) != NULL )
	{
		ptr = strstr ( str, "pts" );
		pos = ( ptr - str );
		*unit = GTK_UNIT_POINTS;
	}

	else if ( strstr ( str, "ins" ) != NULL )
	{
		ptr = strstr ( str, "ins" );
		pos = ( ptr - str );
		*unit = GTK_UNIT_INCH;
	}

	else if ( strstr ( str, "mm" ) != NULL )
	{
		ptr = strstr ( str, "mm" );
		pos = ( ptr - str );
		*unit = GTK_UNIT_MM;

	}

	else
	{
		return TCL_ERROR;
	}

	/* copy first block of the string, then NULL terminate */
	strncpy ( tmp, str, pos );
	tmp[pos] = '\0';

	sscanf ( str, "%f", val );

	return TCL_OK;
}
