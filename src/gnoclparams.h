
/*
gnoclparams.h
*/

typedef enum GnoclStringType_
{
	GNOCL_STR_EMPTY      = 0,       // empty string
	GNOCL_STR_STR        = 1 << 0,  // normal string
	GNOCL_STR_STOCK      = 1 << 1,  // (potentially) the name of a stock item
	GNOCL_STR_FILE       = 1 << 2,  // (potentially) the name of a file
	GNOCL_STR_TRANSLATE  = 1 << 3,  // to be translated via gettext
	GNOCL_STR_UNDERLINE  = 1 << 4,  // '_' marks underline and accelerator
	GNOCL_STR_MARKUP     = 1 << 5,  // markup for label
	GNOCL_STR_BUFFER	 = 1 << 6,   // (potentially) the name of a pixbuf
	GNOCL_STR_ICON_THEME = 1 << 7  // (potentially) the name of a icon theme object
} GnoclStringType;


/* in radioButton.c  for menuRadioItem */

typedef struct
{
	Tcl_Interp *interp;
	GArray     *widgets;
	int        inSetVar;
	char       *variable;
	//GSList     *list;
	/* grouping for toolbar item */
} GnoclRadioGroup;

/*
 * radioButton declarations
 */
typedef struct
{
	char            *name;
	GnoclRadioGroup *group;
	GtkWidget       *widget;
	char            *onToggled;
	Tcl_Obj         *onValue;
} GnoclRadioParams;


/**
\brief    Function associated with the widget.
\note     These are specific to the gnocl package alone, perhaps these should be put into specific library?
*/

typedef struct
{
	GtkWidget	*aspectFrame;
	Tcl_Interp  *interp;
	gchar       *name;
	gchar 		*label;
	gfloat 		xalign;
	gfloat		yalign;
	gfloat 		ratio;
	gboolean 	obey_child;

} AspectFrameParams;

typedef struct
{
	GtkWidget	*iconView;
	GtkWidget   *scrolledWindow;
	Tcl_Interp  *interp;
	gchar       *name;
	GtkListStore *list_store;
	gint		icon_width;
} IconViewParams;

typedef struct
{
	GtkPrintOperation *operation;
	GtkPrintSettings  *settings;
	Tcl_Interp  *interp;
	char        *name;
	gchar 		*filename;
	gdouble 	font_size;
	gint 		lines_per_page;
	gchar 		**lines;
	gint 		total_lines;
	gint 		total_pages;
} PrintOperationsParams;


typedef struct
{
	GtkEntry    *entry;
	GtkEntryCompletion *completion;
	Tcl_Interp  *interp;
	char        *name;
	char        *variable;
	char        *focus;
	char        *onChanged;
	int         inSetVar;
	gboolean	check;
} EntryParams;


typedef struct
{
	GtkButton   *button;
	Tcl_Interp  *interp;
	char        *name;
	char        *iconName;
	char		*data; /* no longer used */
	char		*align;
	GtkAlignment *alignment;
	char		*baseFont;
} ButtonParams;


typedef struct
{
	GtkTextView       *textView;
	GtkScrolledWindow *scrolled;
	Tcl_Interp        *interp;
	char              *name;
	char              *textVariable;
	char              *onChanged;
	int               inSetVar;
	/* check usage of these in the module */
	gboolean          useMarkup;
} TextParams;

typedef struct
{
	GtkLabel    *label;
	Tcl_Interp  *interp;
	char        *name;
	char        *textVariable;
	char        *onChanged;
	int         inSetVar;
} LabelParams;

typedef struct
{
	char       *name;
	Tcl_Interp *interp;
	GtkScale   *scale;
	char       *onValueChanged;
	char       *variable;
	int        inSetVar;
} ScaleParams;

typedef struct
{
	char       *name;
	Tcl_Interp *interp;
	GtkProgressBar *pbar;
	char       *onFractionChanged;
	char       *onTextChanged;
	char       *variable;
	int        inSetVar;
	char       *textVariable;
	int        inSetText;
} ProgressBarParams;

typedef struct
{
	GtkLabel    *label;
	Tcl_Interp  *interp;
	char        *name;
	char        *textVariable;
	char        *onChanged;
	int         inSetVar;
} AccelaratorParams;

typedef struct
{
	char         *name;
	Tcl_Interp   *interp;
	GtkScrollbar *scroll;
	char         *onValueChanged;
	char         *variable;
	int          inSetVar;
} ScrollParams;


typedef struct
{
	char        *name;
	Tcl_Interp  *interp;
	GtkComboBox *comboBox;
	char        *onChanged;
	char        *variable;
	int         inSetVar;
} ComboBoxParams;

typedef struct
{
	Tcl_Interp        *interp;
	char              *name;
	GtkTreeView       *view;
	GtkScrolledWindow *scrollWin;
	int               noColumns;
	int               isTree;
	GtkEntry 		  *searchEntry;
	GHashTable        *idToIter;
	char			  *data;
} TreeListParams;

typedef struct
{
	char       *onResponse;
	char       *name;
	GtkWidget  *dialog;
	Tcl_Interp *interp;
	GPtrArray  *butRes;
	int        isModal;
	int        ret;
} DialogParams;

typedef struct
{
	char       *onResponse;
	char       *name;
	GtkDialog  *dialog;
	Tcl_Interp *interp;
	GPtrArray  *butRes;
	int        isModal;
	int        ret;
} InputDialogParams;


typedef struct
{
	char             *name;
	Tcl_Interp       *interp;
	char             *onClicked;
	GtkColorSelectionDialog *colorSel;
} ColorSelDialogParams;

typedef struct
{
	Tcl_Interp           *interp;
	char                 *name;
	int                  getURIs;
	GtkFileChooserDialog *fileDialog;
} FileSelDialogParams;

typedef struct
{
	char             *name;
	Tcl_Interp       *interp;
	char             *onClicked;
	GtkFontSelectionDialog *fontSel;
} FontSelDialogParams;

typedef struct
{
	GtkToolItem  *item;
	char 		*name;
	char 		*onClicked;
	char 		*data;	// redundant
	Tcl_Interp 	*interp;
} ToolButtonParams;

typedef struct
{
	GtkWidget	*item;
	GtkWidget	*menu;
	char		*name;
	char		*onClicked;
	Tcl_Interp	*interp;
} ToolButtonMenuParams;

typedef struct
{
	GtkPageSetup *setup;
	GtkPaperSize *size;
	int unit;
	char *name;
	Tcl_Interp *interp;
} PaperSetupParams;


typedef struct _CalendarData
{
	GtkWidget	*calendar_widget;
	GtkWidget	*flag_checkboxes[6];
	gboolean	settings[6];
	GtkWidget	*font_dialog;
	GtkWidget	*window;
	GtkWidget	*prev2_sig;
	GtkWidget	*prev_sig;
	GtkWidget	*last_sig;
	GtkWidget	*month;
	GHashTable	*details_table;
	GtkTextBuffer	*details_buffer;
	guint		details_changed;
} CalendarData;

typedef struct
{
	char			*name;
	Tcl_Interp		*interp;
	GtkWidget 		*calendar;
	GHashTable		*details;
	GtkTextBuffer	*buffer;
	guint			changed;
} CalendarParams;


typedef struct
{
	char *text;
	char *font;
	char *clr;
	char *pos;
} splashText;

typedef struct
{
	int  progBar;

	float progress;
	float range;

	int width;
	int height;

	char appName[32];
	char appFont[32];
	char appWeight[32];
	char appClr[32];
	char appPos[32];

	char caption[32];
	char capFont[32];
	char capWeight[32];
	char capClr[32];
	char capPos[32];

	char version[32];
	char verFont[32];
	char verWeight[32];
	char verClr[32];
	char verPos[32];

	char byLine[32];
	char byFont[32];
	char byWeight[32];
	char byClr[32];
	char byPos[32];

	char copyright[48];
	char crFont[32];
	char crWeight[32];
	char crClr[32];
	char crPos[32];

	char *bgImage;
	char *maskImage;
	char *bgclr;

	char *exitCommand;

	GtkImage *image;
	char message[32];
	char *name;

	Tcl_Interp *interp;
	GtkWidget *window;
	GdkPixbuf *pixbuf;
	GdkPixbuf *pixbuf2;
} SplashScreenParams;

typedef struct
{
	GdkPixbuf *pixbuf;
	cairo_t *cr;
	Tcl_Interp *interp;
	char *fname;
} PixbufParams;


typedef struct
{
	GtkAccelGroup *group;
	GtkWindow *window;
	Tcl_Interp *interp;
} AccGrpParams;

typedef struct
{
	GtkToolbar *toolBar;
	GtkTextView *textView;
	GtkScrolledWindow *source;
	char *SVBaseFont;
	char *SVBaseClr;
	//GtkTextView *sourceView;
	Tcl_Interp *interp;
	char *name;
	char *fgClr;
	char *bgClr;
	GtkWidget *fgImg;
	GtkWidget *bgImg;
	GtkWidget *bg;
	GtkWidget *fg;
} RichTextToolbarParams;

typedef struct
{
	char       *name;
	Tcl_Interp *interp;
	GtkWidget  *widget;
	char       *onToggled;
	char       *variable;
	Tcl_Obj    *onValue;
	Tcl_Obj    *offValue;
	int        inSetVar;
} GnoclCheckParams;

typedef struct
{
	char       *name;
	Tcl_Interp *interp;
	GtkToolItem  *item;
	char       *onToggled;
	char       *variable;
	Tcl_Obj    *onValue;
	Tcl_Obj    *offValue;
	int        inSetVar;
	char       *data;
} GnoclToolBarCheckParams;
