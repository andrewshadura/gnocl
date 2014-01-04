/*
 * inheritedOptions.c
 *
 */

/*
   History:
   2013-11: added to core code
*/

#include "gnocl.h"
//#include "gnoclparams.h"

/**
\brief  GtkWidget Inherited Signals
**/
static GnoclOption gtkWidgetOptions[] =
{
	{ "-onStateChange", GNOCL_OBJ, "state-change", gnoclOptOnStateChange},
	{ "-onShowHelp", GNOCL_OBJ, "", gnoclOptOnShowHelp },
	{ "-onRealize", GNOCL_OBJ, "realize", gnoclOptCommand},
	{ "-onUnrealize", GNOCL_OBJ, "unrealize", gnoclOptCommand},
	{ "-onMap", GNOCL_OBJ, "map", gnoclOptCommand},
	{ "-onHide", GNOCL_OBJ, "hide", gnoclOptCommand},
	{ "-onGrabFocus", GNOCL_OBJ, "grab-focus", gnoclOptCommand},
	{ "-onPopupMenu", GNOCL_OBJ, "popup-menu", gnoclOptCommand},
	{ "-onShow", GNOCL_OBJ, "show", gnoclOptCommand},
	{ "-onUnmap", GNOCL_OBJ, "unmap", gnoclOptCommand},
	{NULL}
};

/**
\brief  GtkWidget Inherited Signals
**/
static int appendOptions ( GnoclOption * dest, GnoclOption * src )
{

#if 1
	g_print ( "%s\n", __FUNCTION__ );
#endif

	GnoclOption *dp = dest;
	GnoclOption *sp = src;

	while ( dp->optName )
	{
		++dp;
	}

	while ( sp->optName )
	{
		++sp;
	}

	/* copy also the final NULL */
	memcpy ( dp, src, ( sp - src + 1 ) * sizeof ( *src ) );
	return dp - dest + sp - src;
}

/**
\brief	append clusters of inherited options to custom widget sets
\notes 	first found in treeList.c
**/
void gnoclAppendOptions ( GnoclOption * dest )
{

	appendOptions ( dest, gtkWidgetOptions );
}
