
#include "gnocl.h"

/**
\brief
**/
static void  doOnCurveChanged ( GtkIconView *iconview, GtkTreePath *path, gpointer user_data )
{
	GnoclCommandData *cs = ( GnoclCommandData * ) user_data;

	//if ( *cs->interp->result == '\0' )
	//{
	GnoclPercSubst ps[] =
	{
		{ 'w', GNOCL_STRING },  /* printiconview */
		{ 0 }
	};

	ps[0].val.str = gnoclGetNameFromWidget ( iconview );
	gnoclPercentSubstAndEval ( cs->interp, ps, cs->command, 1 );
	//}
}

/**
\brief      This is for all callbacks which have only %w as substitution
**/
static int gnoclOptOnCurveChanged ( Tcl_Interp *interp, GnoclOption *opt, GObject *obj, Tcl_Obj **ret )
{
	assert ( opt->type == GNOCL_OBJ );

	return gnoclConnectOptCmd ( interp, obj, opt->propName, G_CALLBACK ( doOnCurveChanged ), opt, NULL, ret );
}


static int RangeIdx = 0;
static int CurveTypeIdx = 1;

/*
  "curve-type"               GtkCurveType          : Read / Write
  "max-x"                    gfloat                : Read / Write
  "max-y"                    gfloat                : Read / Write
  "min-x"                    gfloat                : Read / Write
  "min-y"                    gfloat                : Read / Write
*/
static GnoclOption curveOptions[] =
{
	/* custom commands */
	{ "-range", GNOCL_STRING, NULL },    /* 0 */
	{ "-curvetype", GNOCL_STRING, NULL }, /* 1 */

	/* handle events */
	{ "-onCuvedChanged", GNOCL_OBJ, "", gnoclOptOnCurveChanged},
	{ NULL },
};

/**
\brief
\author     William J Giddings
\date       12-Feb-09
\since      0.9.94
**/
static int configure ( Tcl_Interp *interp, GtkCurve *widget, GnoclOption options[] )
{
#ifdef DEBUG_CURVE
	g_print ( "CONFIGURE\n" );
#endif

	if ( options[RangeIdx].status == GNOCL_STATUS_CHANGED )
	{

		g_print ( "----->%s\n", options[RangeIdx].val.str );

		//char *str = Tcl_GetString ( options[RangeIdx].val.obj  );

		float min_x;
		float max_x;
		float min_y;
		float max_y;

		gint ret = 0;

		ret = sscanf ( options[RangeIdx].val.str , "%f %f %f %f", &min_x, &max_x, &min_y, &max_y );

		if ( ret == 4 )
		{
			gtk_curve_set_range ( widget, min_x, max_x, min_y, max_y );
		}

		else
		{
			Tcl_SetResult ( interp, "GNOCL ERROR: Wrong number of values passed to option -range.\n", TCL_STATIC );
			return TCL_ERROR;
		}
	}


	if ( options[CurveTypeIdx].status == GNOCL_STATUS_CHANGED )
	{

		char *str = Tcl_GetString ( options[CurveTypeIdx].val.obj  );

		/* linear interpolation */
		if ( !strcmp ( str, "linear" ) )
		{
			printf ( "1 gnoclOptCurvetype = %s\n", str );
			gtk_curve_set_curve_type ( GTK_CURVE ( widget ), GTK_CURVE_TYPE_LINEAR );

		}

		/* spline interpolation */
		if ( !strcmp ( str, "spline" ) )
		{

			printf ( "2 gnoclOptCurvetype = %s\n", str );
			gtk_curve_set_curve_type ( GTK_CURVE ( widget ), GTK_CURVE_TYPE_SPLINE );

		}

		/* free form curve */
		if ( !strcmp ( str, "free" ) )
		{
			printf ( "3 gnoclOptCurvetype = %s\n", str );
			gtk_curve_set_curve_type ( GTK_CURVE ( widget ) , GTK_CURVE_TYPE_FREE );

		}

	}

	return TCL_OK;
}


/**
\brief
\author     William J Giddings
\date       12-Feb-09
\since      0.9.94
**/
static int cget ( Tcl_Interp *interp, GtkWidget *widget, GnoclOption options[], int idx )
{
	printf ( "cget\n" );

	return gnoclCgetNotImplemented ( interp, options + idx );
}

/**
\brief
\author     William J Giddings
\date       12-Feb-09
\since      0.9.94
**/
static int curveFunc ( ClientData data, Tcl_Interp *interp, int objc, Tcl_Obj * const objv[] )
{
#ifdef DEBUG_CURVE
	listParameters ( objc, objv, "curveFunc" );
#endif

	static const char *cmds[] =
	{
		"set", "reset", "get",
		"delete", "configure",
		"cget", "class",
		NULL
	};
	enum cmdIdx
	{
		SetIdx, ResetIdx, GetIdx,
		DeleteIdx, ConfigureIdx,
		CgetIdx, ClassIdx
	};

	GtkCurve *widget = GTK_WIDGET ( data );

	int idx;

	if ( objc < 2 )
	{
		Tcl_WrongNumArgs ( interp, 1, objv, "command" );
		return TCL_ERROR;
	}

	if ( Tcl_GetIndexFromObj ( interp, objv[1], cmds, "command", TCL_EXACT, &idx ) != TCL_OK )
	{
		return TCL_ERROR;
	}

	switch ( idx )
	{
		case SetIdx:
			{


				if ( strcmp (  Tcl_GetString ( objv[2] ), "gamma" ) == 0 )
				{
					float gamma;

					sscanf ( Tcl_GetString ( objv[3] ), "%f", &gamma );

					gtk_curve_set_gamma ( widget, gamma );
				}

				if ( strcmp (  Tcl_GetString ( objv[2] ), "vector" ) == 0 )
				{

					gint n;
					gint veclen;
					Tcl_Obj *tp;
					float value;
					const char  *str;

					Tcl_ListObjLength ( interp,  objv[3] , &veclen );

					float vector[veclen];

					for ( n = 0; n < veclen; ++n )
					{
						/* obtain a pointer to the specified item in the list */
						Tcl_ListObjIndex ( interp, objv[3], n, &tp );
						sscanf ( Tcl_GetString ( tp ), "%f", &value );
						vector[n] = value;
					}

					gtk_curve_set_vector ( widget, veclen, vector );
				}
			}
			break;
		case ResetIdx:
			{
				printf ( "reset\n" );
				gtk_curve_reset ( widget );
			}
			break;
		case GetIdx:
			{
				Tcl_Obj *list;
				int veclen = 15;
				int k;
				gfloat vector[veclen];
				gtk_curve_get_vector ( widget, veclen, vector );
				gchar str[24];

				list = Tcl_NewListObj ( objc, objv );

				for ( k = 0; k < veclen; ++k )
				{
					g_print ( "vectors = %d %f\n", k, vector[k] );

					sprintf ( str, "%f", vector[k] );

					Tcl_ListObjAppendElement ( NULL, list, Tcl_NewStringObj ( str, -1 ) );
				}

				Tcl_SetObjResult ( interp, list );
			}
			break;
		case ClassIdx:
			{
				printf ( "Class\n" );
				Tcl_SetObjResult ( interp, Tcl_NewStringObj ( "curve", -1 ) );
			}
			break;
		case DeleteIdx:
			{
				return gnoclDelete ( interp, GTK_WIDGET ( widget ), objc, objv );
			}
		case ConfigureIdx:
			{
				int ret = TCL_ERROR;
				printf ( "Configure\n" );

				if ( gnoclParseAndSetOptions ( interp, objc - 1, objv + 1, curveOptions, G_OBJECT ( widget ) ) == TCL_OK )
				{

					printf ( "Configure\n" );
					ret = configure ( interp, widget, curveOptions );
				}

				gnoclClearOptions ( curveOptions );

				return ret;
			}

			break;


		case CgetIdx:
			{
				int     idx;

				switch ( gnoclCget ( interp, objc, objv, G_OBJECT ( widget ), curveOptions, &idx ) )
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
							return cget ( interp, widget, curveOptions, idx );
						}
				}
			}
	}

	return TCL_OK;
}

/**
\brief
\author     William J Giddings
\date       12-Feb-09
\since      0.9.94
**/
int gnoclGammaCurveCmd ( ClientData data, Tcl_Interp * interp, int objc, Tcl_Obj * const objv[] )
{

	int            ret = TCL_OK;
	GtkWidget      *curve;

	if ( 1 )
	{
		g_print ( "1\n" );

		if ( gnoclParseOptions ( interp, objc, objv, curveOptions ) != TCL_OK )
		{
			gnoclClearOptions ( curveOptions );
			return TCL_ERROR;
		}
	}

	curve = gtk_gamma_curve_new() ;
	gtk_curve_reset ( curve );

	gtk_widget_show ( GTK_WIDGET ( curve ) );


	if ( 1 )
	{
		g_print ( "2\n" );
		ret = gnoclSetOptions ( interp, curveOptions, G_OBJECT ( curve ), -1 );
		g_print ( "\t2a\n" );

		if ( ret == TCL_OK )
		{
			ret = configure ( interp, G_OBJECT ( curve ), curveOptions );
		}

		g_print ( "\t2b\n" );
	}

	g_print ( "3\n" );
	gnoclClearOptions ( curveOptions );

	/* STEP 3)  -show the widget */

	if ( ret != TCL_OK )
	{
		gtk_widget_destroy ( GTK_WIDGET ( curve ) );
		return TCL_ERROR;
	}


	g_print ( "4\n" );
	/* STEP 4)  -everything has worked, register the widget with the Tcl interpretor. */
	return gnoclRegisterWidget ( interp, GTK_WIDGET ( curve ), curveFunc );
}

