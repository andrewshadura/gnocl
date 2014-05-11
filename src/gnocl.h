#ifndef GNOCL_H_INCLUDED
#define GNOCL_H_INCLUDED

/*
 * $Id: gnocl.h,v 1.36 2005/08/16 20:57:45 baum Exp $
 *
 * This file implements a Tcl interface to gnome and GTK+
 *
 * Copyright (c) 2001 - 2005 Peter G. Baum  http://www.dr-baum.net
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

/**
\brief  This file implements a Tcl interface to gnome and GTK+
**/

#include "tcl.h"
#include <glib.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <glib/gprintf.h>
#include <glade/glade.h>


#include <gtk-unix-print-2.0/gtk/gtkprinter.h>

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <math.h>

#include "gnoclparams.h"

/* set some padding values */
#define GNOCL_PAD_TINY  2
#define GNOCL_PAD_SMALL 4
#define GNOCL_PAD       8
#define GNOCL_PAD_BIG  12

/* set some values */
enum rolloverModes { GNOCL_ROLLOVER_NONE, GNOCL_ROLLOVER_FG, GNOCL_ROLLOVER_BG, GNOCL_ROLLOVER_BOTH };

/* string prefix to mark stock items */
#define GNOCL_STOCK_PREFIX "%#"
/* transfer string ownership */
#define GNOCL_MOVE_STRING(src,dest) \
      do{ g_free(dest); dest = src; src = NULL; } while( 0 )
/* transfer obj ownership */
#define GNOCL_MOVE_OBJ(src,dest) \
      do{ if(dest) Tcl_DecrRefCount( dest ); \
      dest = src; Tcl_IncrRefCount( dest ); } while( 0 )

/*
typedef enum GnoclStringType_
{
	GNOCL_STR_EMPTY     = 0,       // empty string
	GNOCL_STR_STR       = 1 << 0,  // normal string
	GNOCL_STR_STOCK     = 1 << 1,  // (potentially) the name of a stock item
	GNOCL_STR_FILE      = 1 << 2,  // (potentially) the name of a file
	GNOCL_STR_TRANSLATE = 1 << 3,  // to be translated via gettext
	GNOCL_STR_UNDERLINE = 1 << 4,  // '_' marks underline and accelerator
	GNOCL_STR_MARKUP    = 1 << 5,  // markup for label
	GNOCL_STR_BUFFER    = 1 << 6   // (potentially) the name of a pixbuf
} GnoclStringType;
*/

GnoclStringType gnoclGetStringType ( Tcl_Obj *obj );
char *gnoclGetString ( Tcl_Obj *op );
char *gnoclGetStringFromObj ( Tcl_Obj *op, int *len );
char *gnoclGetStringUline ( Tcl_Obj *op, char **pattern );
char *gnoclStringDup ( Tcl_Obj *op );
char *gnoclGetStockLabel ( Tcl_Obj *obj, Tcl_Interp *interp );
int gnoclGetStockItem ( Tcl_Obj *obj, Tcl_Interp *interp, GtkStockItem *sp );

Tcl_Obj *gnoclGtkToStockName ( const char *gtk );
const char *gnoclGetAppName ( Tcl_Interp *interp );
const char *gnoclGetAppVersion ( Tcl_Interp *interp );
char **gnoclGetArgv ( Tcl_Interp *interp, int *argc );

int gnoclRegisterWidget ( Tcl_Interp *interp, GtkWidget *widget, Tcl_ObjCmdProc *proc );

/* WJG Added 05-03-09 */
int gnoclRegisterPixbuf ( Tcl_Interp *interp, GdkPixbuf *pixbuf, Tcl_ObjCmdProc *proc );


int gnoclMemNameAndWidget ( const char *name, GtkWidget *widget );
int gnoclForgetWidgetFromName ( const char *name );
char *gnoclGetAutoWidgetId ( void );
const char *gnoclGetNameFromWidget ( GtkWidget *widget );
GtkWidget *gnoclGetWidgetFromName ( const char *name, Tcl_Interp *interp );
GtkWidget *gnoclChildNotPacked ( const char *name, Tcl_Interp *interp );
int gnoclAssertNotPacked ( GtkWidget *child, Tcl_Interp *interp,  const char *name );

int gnoclGetBothAlign ( Tcl_Interp *interp, Tcl_Obj *obj, gfloat *xAlign,
						gfloat *yAlign );
int gnoclGetPadding ( Tcl_Interp *interp, Tcl_Obj *obj, int *pad );

enum GnoclOptionType
{
	GNOCL_STRING,
	GNOCL_DOUBLE,
	GNOCL_INT,
	GNOCL_BOOL,
	GNOCL_OBJ,
	GNOCL_LIST
};

typedef struct
{
	char *command;
	Tcl_Interp *interp;
	void       *data;
} GnoclCommandData;

typedef struct
{
	char c;
	enum GnoclOptionType type;
	union
	{
		double     d;
		int        i;
		int        b;
		const char *str;
		Tcl_Obj    *obj;
	}    val;
} GnoclPercSubst;

const char *gnoclPercentSubstitution ( GnoclPercSubst *p, int no, const char *str );

enum GnoclOptionStatus
{
	/* the order is important for gnoclClearOptions */
	GNOCL_STATUS_CLEAR,
	GNOCL_STATUS_CHANGED_ERROR,
	GNOCL_STATUS_CHANGED,
	GNOCL_STATUS_SET_ERROR,
	GNOCL_STATUS_SET
};

enum GnoclCgetReturn
{
	GNOCL_CGET_ERROR,
	GNOCL_CGET_HANDLED,
	GNOCL_CGET_NOTHANDLED
};

struct GnoclOption_;
typedef int ( gnoclOptFunc ) ( Tcl_Interp *, struct GnoclOption_ *, GObject *, Tcl_Obj **ret );

typedef struct GnoclOption_
{
	const char           *optName;
	enum GnoclOptionType type;
	const char           *propName;        /* NULL for no automatic setting */
	gnoclOptFunc         *func;
	enum GnoclOptionStatus status;
	union
	{
		gboolean b;
		gint     i;
		gdouble  d;
		gchar    *str;
		Tcl_Obj  *obj;
	}          val;
} GnoclOption;


/* WJG STUFF */

int gnoclOptWindowCenter ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret );
int gnoclOptGdkBaseFont ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret );

/* moved into text.c, perhaps rename to gnoclOptOnTagEvent */
// gnoclOptFunc gnoclOptOnEvent;
gnoclOptFunc gnoclOptOnScroll;

/* WJG STUFF ENDS */


int gnoclOptGeneric ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, const char *optName, const char *txt[], const int types[], Tcl_Obj **ret );

gnoclOptFunc gnoclOptGdkColorEvenRow; /* still a stub, widget style settings */

gnoclOptFunc gnoclOptOrientation;
gnoclOptFunc gnoclOptNotify;
gnoclOptFunc gnoclOptAnchor;
gnoclOptFunc gnoclOptBothAlign;
gnoclOptFunc gnoclOptChild;
gnoclOptFunc gnoclOptCommand;
gnoclOptFunc gnoclOptData;
gnoclOptFunc gnoclOptDnDTargets;
gnoclOptFunc gnoclOptGdkColor;
gnoclOptFunc gnoclOptGdkBaseFont;
gnoclOptFunc gnoclOptGdkColorBase;
gnoclOptFunc gnoclOptGdkColorBg;
gnoclOptFunc gnoclOptGdkColorFg;
gnoclOptFunc gnoclOptFrameLabelWidget;
gnoclOptFunc gnoclOptToolButtonLabelWidget;
gnoclOptFunc gnoclOptToolButtonIconWidget;
gnoclOptFunc gnoclOptGdkColorText;
gnoclOptFunc gnoclOptGdkColorRollOver;
gnoclOptFunc gnoclOptHalign;
gnoclOptFunc gnoclOptIcon;
gnoclOptFunc gnoclOptJustification;
gnoclOptFunc gnoclOptEllipsize;
gnoclOptFunc gnoclOptLabelFull;
gnoclOptFunc gnoclOptOnButton;
gnoclOptFunc gnoclOptOnButton2; //<--------------------------------------
//gnoclOptFunc gnoclOptOnSelectionChanged;
gnoclOptFunc gnoclOptOnFontSet;
gnoclOptFunc gnoclOptOnFileSet;
gnoclOptFunc gnoclOptOnFolderSet;
gnoclOptFunc gnoclOptOnConfigure;
gnoclOptFunc gnoclOptOnDelete;
gnoclOptFunc gnoclOptOnDragData;
gnoclOptFunc gnoclOptOnDropData;
gnoclOptFunc gnoclOptOnDragEnd;
gnoclOptFunc gnoclOptOnEnterLeave;
gnoclOptFunc gnoclOptOnKeyPress;
gnoclOptFunc gnoclOptOnKeyRelease;
gnoclOptFunc gnoclOptOnMotion;
void doOnMotion ( GtkWidget *widget, GdkEventMotion *event, gpointer data );
gnoclOptFunc gnoclOptOnButtonMotion;
gnoclOptFunc gnoclOptCharWidth;
gnoclOptFunc gnoclOptOnBackSpace;
gnoclOptFunc gnoclOptOnClipboard;
gnoclOptFunc gnoclOptOnUndoRedo;
gnoclOptFunc gnoclOptOnTextInsert;
gnoclOptFunc gnoclOptOnChanged;
gnoclOptFunc gnoclOptOnModified;
gnoclOptFunc gnoclOptOnApplyTag;
gnoclOptFunc gnoclOptOnRemoveTag;
gnoclOptFunc gnoclOptOnFocus;
gnoclOptFunc gnoclOptOnEnterLeave;
gnoclOptFunc gnoclOptOnBeginUserAction;
gnoclOptFunc gnoclOptOnEndUserAction;
gnoclOptFunc gnoclOptOnInsertChildAnchor;
gnoclOptFunc gnoclOptOnLInkButton;
gnoclOptFunc gnoclOptOnDeletFromCursor;
gnoclOptFunc gnoclOptOnInsertAtCursor;
gnoclOptFunc gnoclOptOnMoveCursor;
gnoclOptFunc gnoclOptOnMoveFocus;
gnoclOptFunc gnoclOptOnMoveViewport;
gnoclOptFunc gnoclOptOnPageHorizontally ;
gnoclOptFunc gnoclOptOnPasteClipboard ;
gnoclOptFunc gnoclOptOnPopulatePopup;
gnoclOptFunc gnoclOptOnSelectAll;
gnoclOptFunc gnoclOptIconTooltip;
gnoclOptFunc gnoclOptOnPreEditChanged;
gnoclOptFunc gnoclOptOnIconPress;
gnoclOptFunc gnoclOptOnSetAnchor;
gnoclOptFunc gnoclOptOnScrollAdjustments;
gnoclOptFunc gnoclOptOnToggleOverwrite;
gnoclOptFunc gnoclOptIcons;
gnoclOptFunc gnoclOptOnPasteDone;
gnoclOptFunc gnoclOptOnDelete;
gnoclOptFunc gnoclOptOnMarkSet;
gnoclOptFunc gnoclOptOnMarkDelete;
gnoclOptFunc gnoclOptOnExpose;
gnoclOptFunc gnoclOptOnClicked;
gnoclOptFunc gnoclOptOnColorSet;
gnoclOptFunc gnoclOptOnStateChange;
gnoclOptFunc gnoclOptOnButtonClicked;
gnoclOptFunc gnoclOptOnColumnClicked;
gnoclOptFunc gnoclOptOnInteractiveSearch;
gnoclOptFunc gnoclOptArrowTooltip;
gnoclOptFunc gnoclOptOnShowHelp;
gnoclOptFunc gnoclOptPadding;
gnoclOptFunc gnoclOptPangoScaledInt;
gnoclOptFunc gnoclOptPangoStretch;
gnoclOptFunc gnoclOptPangoStyle;
gnoclOptFunc gnoclOptPangoVariant;
gnoclOptFunc gnoclOptPangoWeight;
gnoclOptFunc gnoclOptPangoWrapMode;
gnoclOptFunc gnoclOptPosition;
gnoclOptFunc gnoclOptRelief;
gnoclOptFunc gnoclOptButtonRelief;
gnoclOptFunc gnoclOptRGBAColor;
gnoclOptFunc gnoclOptScale;
gnoclOptFunc gnoclOptShadow;
gnoclOptFunc gnoclOptSizeGroup;
gnoclOptFunc gnoclOptHWGroup;
gnoclOptFunc gnoclOptTooltip;
gnoclOptFunc gnoclOptUnderline;
gnoclOptFunc gnoclOptWidget;
gnoclOptFunc gnoclOptWindowTypeHint;
gnoclOptFunc gnoclOptWrapmode;
gnoclOptFunc gnoclOptTransientWindow;
gnoclOptFunc gnoclOptPopupMenu;
gnoclOptFunc gnoclOptMask;
gnoclOptFunc gnoclOptMask2;
gnoclOptFunc gnoclOptBackgroundImage;
gnoclOptFunc gnoclOptBackgroundImage2;
gnoclOptFunc gnoclOptHasFrame;
gnoclOptFunc gnoclOptShadow;
gnoclOptFunc gnoclOptCurrentFolder;
gnoclOptFunc gnoclOptArrow;
gnoclOptFunc gnoclOptDefaultWidget;
gnoclOptFunc gnoclOptCursor;
gnoclOptFunc gnoclOptKeepAbove;
gnoclOptFunc gnoclOptKeepBelow;
gnoclOptFunc gnoclOptOpacity;
gnoclOptFunc gnoclOptStick;
gnoclOptFunc gnoclOptFullScreen;
gnoclOptFunc gnoclGetParent;

gnoclOptFunc gnoclOptOnBackspace;
gnoclOptFunc gnoclOptOnDeleteFromCursor;
gnoclOptFunc gnoclOptOnEntryInsert;
gnoclOptFunc gnoclOptOnPreEditChanged;
gnoclOptFunc gnoclOptOnQueryToolTip;
gnoclOptFunc gnoclOptMoveHandle;
gnoclOptFunc gnoclOptHeightRequest;
gnoclOptFunc gnoclOptOnActivateCurrentLink;
gnoclOptFunc gnoclOptOnActivateLink;

int gnoclClearOptions ( GnoclOption *opts );
int gnoclGetOptions ( Tcl_Interp *interp, GnoclOption *options );
int gnoclGetCommands ( Tcl_Interp *interp, char *cmds[] );
int gnoclResetSetOptions ( GnoclOption *opts );
int gnoclSetOptions ( Tcl_Interp *interp, GnoclOption *opts, GObject *object, int no );
int gnoclGetIndexFromObjStruct ( Tcl_Interp *interp, Tcl_Obj *objPtr, char **tablePtr, int offset, char *msg, int flags, int *indexPtr );
int gnoclParseOptions ( Tcl_Interp *interp, int objc, Tcl_Obj * const objv[], GnoclOption *opts );
int gnoclParseAndSetOptions ( Tcl_Interp *interp, int objc, Tcl_Obj * const objv[], GnoclOption *opts, GObject *object );
int gnoclCget ( Tcl_Interp *interp, int objc, Tcl_Obj * const objv[], GObject *gObj, GnoclOption *opts, int *idx );
int gnoclCgetOne ( Tcl_Interp *interp, Tcl_Obj *obj, GObject *gObj, GnoclOption *opts, int *idx );
int gnoclCgetNotImplemented ( Tcl_Interp *interp, GnoclOption *opt );
Tcl_Obj *gnoclCgetButtonText ( Tcl_Interp *interp, GtkButton *button );
int gnoclConfigButtonText ( Tcl_Interp *interp, GtkButton *button, Tcl_Obj *txtObj );

int gnoclButtonSetSize ( Tcl_Interp  *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret );

int gnoclKeyFileCmd ( ClientData data, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[] );

const char *gnoclGetOptCmd ( GObject *obj, const char *signal );
int gnoclDisconnect ( GObject *obj, const char *signal, GCallback handler );
int gnoclConnectOptCmd ( Tcl_Interp *interp, GObject *object, const char *signal, GCallback handler, GnoclOption *opt, void *data, Tcl_Obj **ret );


int gnoclOptOnInsertText ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret );
int gnoclConnectSignalCmd ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret );
int gnoclOptOnDeleteRange ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret );

/* now handled within each specific module as signal handlers can vary */
//int gnoclOptOnChanged ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret );

int gnoclOptOnModified ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret );
int gnoclOptOnApplyTag ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret );
int gnoclOptOnBeginUserAction  ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret );
int gnoclOptOnEndUserAction  ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret );
int gnoclOptOnInsertChildAnchor  ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret );
int gnoclOptOnRemoveTag ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret );
int gnoclOptOnMarkSet ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret );
int gnoclOptOnMarkDelete ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret );
int gnoclOptOnChildAttached ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret );
int gnoclOptOnChildDetached ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret );
int gnoclOptOnLinkButton ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret );
int gnoclOptOnValueChanged ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret );


int gnoclOptOnColorWheelChanged ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret );
int gnoclOptOnColorWheelMove ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret );

int gnoclOptOnACtivate ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret );

int gnoclDelete ( Tcl_Interp *interp, GtkWidget *widget, int objc, Tcl_Obj * const objv[] );
int gnoclAttachVariable ( GnoclOption *newVar, char **oldVar, const char *signal, GObject *obj, GCallback gtkFunc, Tcl_Interp *interp, Tcl_VarTraceProc tclFunc, gpointer data );
int gnoclAttachOptCmdAndVar ( GnoclOption *newCmd, char **oldCmd, GnoclOption *newVar, char **oldVar, const char *signal, GObject *obj, GCallback gtkFunc, Tcl_Interp *interp, Tcl_VarTraceProc tclFunc, gpointer data );
int gnoclGetScrollbarPolicy ( Tcl_Interp *interp, Tcl_Obj *obj, GtkPolicyType *hor, GtkPolicyType *vert );
int gnoclGetSelectionMode ( Tcl_Interp *interp, Tcl_Obj *obj, GtkSelectionMode *selection );
int gnoclGetFontTxt ( Tcl_Interp *interp, Tcl_Obj *obj, const char **font );
int gnoclGetGdkFont ( Tcl_Interp *interp, Tcl_Obj *obj, GdkFont **font );
int gnoclGetGdkColorAlloc ( Tcl_Interp *interp, Tcl_Obj *obj, GtkWidget *widget, GdkColor *color, int *a );
int gnoclGetAnchorStyle ( Tcl_Interp *interp, Tcl_Obj *obj, GtkAnchorType *style );
int gnoclGetJustification ( Tcl_Interp *interp, Tcl_Obj *obj, GtkJustification *type );
int gnoclGetOrientationType ( Tcl_Interp *interp, Tcl_Obj *obj, GtkOrientation *orient );
int gnoclGetImage ( Tcl_Interp *interp, Tcl_Obj *obj, GtkIconSize size, GtkWidget **widget );
GtkTooltips *gnoclGetTooltips( );
GtkAccelGroup *gnoclGetAccelGroup( );

int gnoclEditablePosToIndex ( Tcl_Interp *interp, Tcl_Obj *obj, GtkEditable *editable, int *pidx );
int gnoclHandleEditableCmds ( int idx, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[], GtkEditable *editable );

#ifdef GNOCL_USE_GNOME
int gnoclRegisterHintAppBar ( GtkWidget *widget, GnomeAppBar *appBar );
GnomeAppBar *gnoclGetHintAppBar ( GtkWidget *widget );
#endif

/* in helperFuncs */
GtkWidget *gnoclFindChild ( GtkWidget *widget, GtkType type );
int gnoclPosOffset ( Tcl_Interp *interp, const char *txt, int *offset );
int gnoclPercentSubstAndEval ( Tcl_Interp *interp, GnoclPercSubst *ps, const char *orig_script, int background );
int gnoclGet2Boolean ( Tcl_Interp *interp, Tcl_Obj *obj, int *b1, int *b2 );
int gnoclGet2Int ( Tcl_Interp *interp, Tcl_Obj *obj, int *b1, int *b2 );
int gnoclGet2Double ( Tcl_Interp *interp, Tcl_Obj *obj, double *b1, double *b2 );
GdkPixbuf *gnoclPixbufFromObj ( Tcl_Interp *interp, GnoclOption *opt );


GdkPixbuf *gnoclBlendPixbufFromObj ( Tcl_Interp *interp, GnoclOption *opt );


char *gnoclGetWidgetGeometry ( GtkWidget *widget );

/* in menu */
int gnoclMenuShellAddChildren ( Tcl_Interp *interp, GtkMenuShell *shell, Tcl_Obj *children, int atEnd );

/* in menuItem */
Tcl_Obj *gnoclCgetMenuItemAccel ( Tcl_Interp *interp, GtkMenuItem *item );
int gnoclMenuItemHandleAccel ( Tcl_Interp *interp, GtkMenuItem *item, Tcl_Obj *accelObj );
Tcl_Obj *gnoclCgetMenuItemText ( Tcl_Interp *interp, GtkMenuItem *item );
int gnoclMenuItemHandleText ( Tcl_Interp *interp, GtkMenuItem *item, Tcl_Obj *textObj );

/* in text.c */
int gnoclTextCommand ( GtkTextView *textView, Tcl_Interp * interp, int objc, Tcl_Obj *  const objv[], int cmdNo, int isTextWidget );

/* in sourceVSiew.c */

/*
 * recentChooserDialog declarations
 */
int gnoclRecentChooserDialogCmd ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] );
int gnoclRecentManagerCmd ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] );
int gnoclPageSetupDialogCmd ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] );
int gnoclPrintDialogCmd ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] );

int gnoclPrinterDialogCmd ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] );
int gnoclPageSetupCmd ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] );

int gnoclMenuRecentChooserCmd ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] );

/*
 * volumeButton declarations
 */
Tcl_Obj *gnoclVolumeButtonText ( Tcl_Interp *interp, GtkWidget *widget );
int gnoclConfigVolumeButtonText ( Tcl_Interp *interp, GtkWidget *widget, Tcl_Obj *txtObj );
int gnoclVolumeButtonCmd ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] );



/*
 * scaleButton declarations
 */
Tcl_Obj *gnoclCgetScaleButtonText ( Tcl_Interp *interp, GtkWidget *widget );
int gnoclConfigScaleButtonText ( Tcl_Interp *interp, GtkWidget *widget, Tcl_Obj *txtObj );
int gnoclScaleButtonCmd ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] );

/*
 * linkButton declarations
 */
Tcl_Obj *gnoclCgetLinkButtonText ( Tcl_Interp *interp, GtkWidget *widget );
int gnoclConfigLinkButtonText ( Tcl_Interp *interp, GtkWidget *widget, Tcl_Obj *txtObj );
int gnoclLinkButtonCmd ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] );
int gnoclArrowButtonCmd ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] );
//int gnoclAcceleratorCmd ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] );

int gnoclDialCmd ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] );
int gnoclSpinnerCmd ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] );
int gnoclLevelCmd ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] );

int gnoclRichTextToolBarCmd ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] );



//int gnoclSpinnerCmd ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] );

/*
 */
int gnoclRecentChooserCmd ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] );

/*
 * toggleButton declarations
 */

/* in toggleButton.c  for menuCheckItem and toolBar checkItem */

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
	char	   *data;
} GnoclToggleParams;


/**
 * AUTHOR
 *  PGB
 * SOURCE
 */

typedef struct
{
	char          *name;
	char          *variable;
	char          *onValueChanged;
	GtkSpinButton *spinButton;
	Tcl_Interp    *interp;
	int           inSetVar;
} SpinButtonParams;


typedef struct
{
	GKeyFile 	*keyFile;
	char        *name;
	char		*fname;
	Tcl_Interp    *interp;
} KeyFileParams;



/*
 * file & folderChooseButton declarations
 */
int gnoclFileChooserButtonCmd ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] );
int gnoclFolderChooserButtonCmd ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] );


/*
 *
 */
int gnoclDrawingAreaCmd ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] );

void gnoclToggleDestroyFunc ( GtkWidget *widget, gpointer data );
int gnoclToggleToggle ( Tcl_Interp *interp, int objc, Tcl_Obj * const objv[], GnoclToggleParams *para );
int gnoclToggleSetActive ( GnoclToggleParams *para, GnoclOption *active );
int gnoclToggleSetValue ( GnoclToggleParams *para, Tcl_Obj *obj );
void gnoclToggleToggledFunc ( GtkWidget *widget, gpointer data );
char *gnoclToggleTraceFunc ( ClientData data, Tcl_Interp *interp, const char *name1, const char *name2, int flags );
int gnoclToggleVariableValueChanged ( GnoclToggleParams *para );
int gnoclToggleButtonCmd ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] );
int gnoclHandleBoxCmd ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] );
int gnoclAssistantCmd ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] );

/*
 * VteTerminal widget stuff - this belongs to the VTE package, not gnocl!
 *
 */
int gnoclVteTerminalCmd ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] );


/*
 * checkButton declarations
 */
/* in checkButton.c  for menuCheckItem and toolBar checkItem */


/*
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
	GtkWidget  *widget;
	GtkToggleAction  *item;
	char       *onToggled;
	char       *variable;
	Tcl_Obj    *onValue;
	Tcl_Obj    *offValue;
	int        inSetVar;
} GnoclToolBarCheckParams;

*/
int gnoclCheckIsOn ( Tcl_Interp *interp, Tcl_Obj *onValue, Tcl_Obj *offValue, Tcl_Obj *val );
void gnoclCheckDestroyFunc ( GtkWidget *widget, gpointer data );
void gnoclCheckToggledFunc ( GtkWidget *widget, gpointer data );
char *gnoclCheckTraceFunc ( ClientData data, Tcl_Interp *interp, const char *name1, const char *name2, int flags );
int gnoclCheckSetValue ( GnoclCheckParams *para, Tcl_Obj *obj );
int gnoclCheckOnToggled ( Tcl_Interp *interp, int objc, Tcl_Obj * const objv[], GnoclCheckParams *para );
int gnoclCheckSetActive ( GnoclCheckParams *para, GnoclOption *opt );
int gnoclCheckVariableValueChanged ( GnoclCheckParams *para );

/* in radioButton.c  for menuRadioItem */
/*
typedef struct
{
	Tcl_Interp *interp;
	GArray     *widgets;
	int        inSetVar;
	char       *variable;
	//GSList     *list;
} GnoclRadioGroup;
*/

/*
 * radioButton declarations
 */

/*
typedef struct
{
	char            *name;
	GnoclRadioGroup *group;
	GtkWidget       *widget;
	char            *onToggled;
	Tcl_Obj         *onValue;
} GnoclRadioParams;
*/

/*
 * radioButton declarations
 */

GnoclRadioGroup *gnoclRadioGroupNewGroup ( const char *var, Tcl_Interp *interp );
int gnoclRadioGroupAddWidgetToGroup ( GnoclRadioGroup *group, GnoclRadioParams *para );
int gnoclRadioRemoveWidgetFromGroup ( GnoclRadioGroup *group, GnoclRadioParams *para );
GnoclRadioParams *gnoclRadioGetActivePara ( GnoclRadioGroup *group );
GnoclRadioGroup *gnoclRadioGetGroupFromVariable ( const char *var );
GnoclRadioParams *gnoclRadioGetParam ( GnoclRadioGroup *group, int n );
int gnoclRadioSetValueActive ( GnoclRadioParams *para, GnoclOption *value, GnoclOption *active );
void gnoclRadioDestroyFunc ( GtkWidget *widget, gpointer data );
void gnoclRadioToggledFunc ( GtkWidget *widget, gpointer data );
char *gnoclRadioTraceFunc ( ClientData data, Tcl_Interp *interp, const char *name1, const char *name2, int flags );
Tcl_Obj *gnoclRadioGetValue ( GnoclRadioParams *para );
int gnoclRadioSetValue ( GnoclRadioParams *para, Tcl_Obj *val );
int gnoclRadioOnToggled ( Tcl_Interp *interp, int objc, Tcl_Obj * const objv[], GnoclRadioParams *para );

Tcl_ObjCmdProc gnoclDebugCmd;
Tcl_ObjCmdProc gnoclCallbackCmd;
Tcl_ObjCmdProc gnoclClipboardCmd;
Tcl_ObjCmdProc gnoclConfigureCmd;
Tcl_ObjCmdProc gnoclInfoCmd;
Tcl_ObjCmdProc gnoclMainLoop;
Tcl_ObjCmdProc gnoclUpdateCmd;
Tcl_ObjCmdProc gnoclResourceFileCmd;
Tcl_ObjCmdProc gnoclGetStyleCmd;
Tcl_ObjCmdProc gnoclSetStyleCmd;
Tcl_ObjCmdProc gnoclWinfoCmd;
Tcl_ObjCmdProc gnoclScreenCmd;
Tcl_ObjCmdProc gnoclPrintCmd;
Tcl_ObjCmdProc gnoclPixBufCmd;
Tcl_ObjCmdProc gnoclPixMapCmd;
Tcl_ObjCmdProc gnoclClr2RGBCmd;
Tcl_ObjCmdProc gnoclRGB2HexCmd;
Tcl_ObjCmdProc gnoclParseColorCmd;
Tcl_ObjCmdProc gnoclStatusIconCmd;
Tcl_ObjCmdProc gnoclSplashScreenCmd;
Tcl_ObjCmdProc gnoclAboutDialogCmd;
Tcl_ObjCmdProc gnoclActionCmd;
Tcl_ObjCmdProc gnoclButtonCmd;
Tcl_ObjCmdProc gnoclBoxCmd;

Tcl_ObjCmdProc gnoclHBoxCmd;
Tcl_ObjCmdProc gnoclVBoxCmd;

Tcl_ObjCmdProc gnoclFixedCmd;
Tcl_ObjCmdProc gnoclCheckButtonCmd;
Tcl_ObjCmdProc gnoclColorButtonCmd;
Tcl_ObjCmdProc gnoclColorSelectionCmd;
Tcl_ObjCmdProc gnoclColorWheelCmd;
Tcl_ObjCmdProc gnoclComboBoxCmd;
Tcl_ObjCmdProc gnoclComboEntryCmd;
Tcl_ObjCmdProc gnoclComboCmd;
Tcl_ObjCmdProc gnoclDialogCmd;
Tcl_ObjCmdProc gnoclEntryCmd;
Tcl_ObjCmdProc gnoclEventBoxCmd;
Tcl_ObjCmdProc gnoclExpanderCmd;
Tcl_ObjCmdProc gnoclFileSelectionCmd;
Tcl_ObjCmdProc gnoclFileChooserCmd;

Tcl_ObjCmdProc gnoclAcceleratorCmd;


/* move megawidgets into a separate package? */
Tcl_ObjCmdProc gnoclLabelEntryCmd;


/* anticipate some problems with this clashing old codes */
Tcl_ObjCmdProc gnoclFileChooserDialogCmd;
Tcl_ObjCmdProc gnoclFontSelectionDialogCmd;
Tcl_ObjCmdProc gnoclColorSelectionDialogCmd;
Tcl_ObjCmdProc gnoclFontButtonCmd;
Tcl_ObjCmdProc gnoclFontSelectionCmd;
Tcl_ObjCmdProc gnoclImageCmd;
Tcl_ObjCmdProc gnoclLabelCmd;
Tcl_ObjCmdProc gnoclListCmd;
Tcl_ObjCmdProc gnoclMenuCmd;
Tcl_ObjCmdProc gnoclMenuBarCmd;
Tcl_ObjCmdProc gnoclMenuItemCmd;
Tcl_ObjCmdProc gnoclMenuCheckItemCmd;
Tcl_ObjCmdProc gnoclMenuRadioItemCmd;
Tcl_ObjCmdProc gnoclMenuSeparatorCmd;
Tcl_ObjCmdProc gnoclNotebookCmd;
Tcl_ObjCmdProc gnoclOptionMenuCmd;
Tcl_ObjCmdProc gnoclPanedCmd;
Tcl_ObjCmdProc gnoclToolPaletteCmd;
Tcl_ObjCmdProc gnoclToolItemGroupCmd;
Tcl_ObjCmdProc gnoclPlugCmd;
Tcl_ObjCmdProc gnoclProgressBarCmd;

Tcl_ObjCmdProc gnoclPBarCmd;

Tcl_ObjCmdProc gnoclRadioButtonCmd;
Tcl_ObjCmdProc gnoclScaleCmd;
Tcl_ObjCmdProc gnoclScrolledWindowCmd;
Tcl_ObjCmdProc gnoclSeparatorCmd;
Tcl_ObjCmdProc gnoclSocketCmd;
Tcl_ObjCmdProc gnoclSpinButtonCmd;
Tcl_ObjCmdProc gnoclStatusBarCmd;
Tcl_ObjCmdProc gnoclTableCmd;
Tcl_ObjCmdProc gnoclTextCmd;
Tcl_ObjCmdProc gnoclTextViewCmd;
//Tcl_ObjCmdProc gnoclClockCmd;
Tcl_ObjCmdProc gnoclTickerTapeCmd;
Tcl_ObjCmdProc gnoclToolBarCmd;
Tcl_ObjCmdProc gnoclTreeCmd;
Tcl_ObjCmdProc gnoclWindowCmd;
Tcl_ObjCmdProc gnoclCalendarCmd;
Tcl_ObjCmdProc gnoclCurveCmd;
Tcl_ObjCmdProc gnoclGammaCurveCmd;
Tcl_ObjCmdProc gnoclRulerCmd;
Tcl_ObjCmdProc gnoclInfoBarCmd;
Tcl_ObjCmdProc gnoclLayoutCmd;
Tcl_ObjCmdProc gnoclAspectFrameCmd;
Tcl_ObjCmdProc gnoclIconViewCmd;
Tcl_ObjCmdProc gnoclAssistantCmd;

/* some simple megawidgets */
Tcl_ObjCmdProc gnoclLabelEntryCmd;

/* extra debugging func */
Tcl_ObjCmdProc gnoclPutsOb;

/* some miscellaneous functions */
Tcl_ObjCmdProc gnoclSignalStopCmd;
Tcl_ObjCmdProc gnoclSignalEmitCmd;
Tcl_ObjCmdProc gnoclBeepCmd;
Tcl_ObjCmdProc gnoclPangoCmd;
Tcl_ObjCmdProc gnoclCommandsCmd;
Tcl_ObjCmdProc gnoclHsv2RgbCmd;
Tcl_ObjCmdProc gnoclStringCmd;
Tcl_ObjCmdProc gnoclBindCmd;
Tcl_ObjCmdProc gnoclStockItemCmd;
Tcl_ObjCmdProc gnoclIconThemeCmd;
Tcl_ObjCmdProc gnoclSoundCmd;
Tcl_ObjCmdProc gnoclPointerCmd;
Tcl_ObjCmdProc gnoclExecCmd;
Tcl_ObjCmdProc gnoclToggleCmd;
Tcl_ObjCmdProc gnoclSetPropertyCmd;
Tcl_ObjCmdProc gnoclSetOpts;
Tcl_ObjCmdProc gnoclShowUriCmd;
//Tcl_ObjCmdProc gnoclCairoCmd;
Tcl_ObjCmdProc gnoclToolTip;

Tcl_ObjCmdProc gnoclGrabCmd;

#ifdef GNOCL_USE_GNOME
Tcl_ObjCmdProc gnoclAboutCmd;
Tcl_ObjCmdProc gnoclAppCmd;
Tcl_ObjCmdProc gnoclAppBarCmd;
#endif

#endif

/* the following constructors now need to be made public in order to run glade */

int setGtkWidgetFunc ( Tcl_Interp *interp, GtkWidget *widget, int idx, Tcl_Obj *resList , gchar *gladeName, char *name );

typedef int ( gnoclWidgetFunc ) ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] );

gnoclWidgetFunc windowFunc;
gnoclWidgetFunc textViewFunc;
gnoclWidgetFunc labelFunc;
gnoclWidgetFunc toolButtonFunc;
gnoclWidgetFunc statusBarFuc;
gnoclWidgetFunc toolBarFunc;
gnoclWidgetFunc buttonFunc;
gnoclWidgetFunc entryFunc;
gnoclWidgetFunc toggleButtonFunc;
gnoclWidgetFunc boxFunc;
gnoclWidgetFunc menuBarFunc;
gnoclWidgetFunc menuFunc;
gnoclWidgetFunc menuItemFunc;
gnoclWidgetFunc checkButtonFunc;
gnoclWidgetFunc spinButtonFunc;
gnoclWidgetFunc radioButtonFunc;
gnoclWidgetFunc fileChooserButtonFunc;
gnoclWidgetFunc clrButtonFunc;
gnoclWidgetFunc fontButtonFunc;
gnoclWidgetFunc linkButtonFunc;
gnoclWidgetFunc scaleButtonFunc;
gnoclWidgetFunc volumeButtonFunc;
gnoclWidgetFunc imageFunc;
gnoclWidgetFunc scaleFunc;
gnoclWidgetFunc scrollFunc;
gnoclWidgetFunc comboBoxFunc;
gnoclWidgetFunc progressBarFunc;
gnoclWidgetFunc treeListFunc;
gnoclWidgetFunc iconViewFunc;
gnoclWidgetFunc handleBoxFunc;
gnoclWidgetFunc calendarFunc;
gnoclWidgetFunc separatorFunc;
gnoclWidgetFunc arrowButtonFunc;
gnoclWidgetFunc drawingAreaFunc;
gnoclWidgetFunc recentChooserFunc;
gnoclWidgetFunc aboutDialogFunc;
gnoclWidgetFunc dialogFunc;
gnoclWidgetFunc colorSelDialogFunc;
gnoclWidgetFunc colorSelectionFunc;
gnoclWidgetFunc fileDialogFunc;
gnoclWidgetFunc expanderFunc;
gnoclWidgetFunc scrlWindowFunc;
gnoclWidgetFunc fontSelDialogFunc;
gnoclWidgetFunc inputDialogFunc;
gnoclWidgetFunc recentChooserDialogFunc;
gnoclWidgetFunc assistantFunc;
gnoclWidgetFunc accelaratorFunc;
gnoclWidgetFunc panedFunc;
gnoclWidgetFunc notebookFunc;
gnoclWidgetFunc eventBoxFunc;
gnoclWidgetFunc fileChooserFunc;

/* whole block removed into gnoclparams.h */


/* appended by module-assistant-pkg */
Tcl_ObjCmdProc gnoclTextBufferCmd;

/* used in creating lists from widget and pixbuf hashtables */
void hash_to_list ( gpointer key, gpointer value, gpointer user_data );
gint sorter ( gconstpointer a, gconstpointer b );

int pixBufFunc ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] );

void gtk_text_buffer_insert_markup ( GtkTextBuffer *buffer, GtkTextIter *iter, const gchar *markup );

/* found in toolBar.c */
int getTextAndIcon ( Tcl_Interp *interp, GtkToolbar *toolbar, GnoclOption *txtOpt, GnoclOption *iconOpt, char **txt, GtkWidget **item, int *isUnderline );

/* used by toolbar, moved to helperfuncs */
void setUnderline ( GtkWidget *item );
int getTextAndIcon ( Tcl_Interp *interp, GtkToolbar *toolbar, GnoclOption *txtOpt, GnoclOption *iconOpt, char **txt, GtkWidget **item, int *isUnderline );
Tcl_Obj *cgetText ( GtkWidget *item );

void gnoclUpdate ();

/* pixbuf */
int gnoclCreatePixbufFromIcon ( Tcl_Interp * interp, GtkIconTheme * icon_theme, const gchar * icon_name, GtkIconSize size );
