/*
  Copyright (C) 2002 Giulio Lunati

  This is free software; you can redistribute it and/or modify
  it under the terms of the version 2 of the GNU General Public
  License as published by the Free Software Foundation.

  This software is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this software; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,
  USA.

http://src.gnu-darwin.org/ports/graphics/claraocr/work/clara-0.9.9/

*/

/*

deskew.c: Deskewing routines.

*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
/* double roundf(double x); */
//#include "common.h"

//#define MSG(args...) fprintf(stderr, ## args)

#define IMP(im,x,y,z) (gl_pixmap[(x)+(y)*GL_XRES])
#define IMV(im,x,y,z) (im)->p[z][TRIM((y),0,(im->r)-1)][TRIM((x),0,(im->c)-1)]

#define DIV(a,b) (((a)+abs(b)/((a)>0?2:-2))/(b))
#define MIN(A,B) ((A)<(B)?(A):(B))
#define MAX(A,B) ((A)>(B)?(A):(B))
#define TRIM(X,A,B) MIN(MAX((X),(A)),(B))

typedef unsigned char cgray;

typedef struct
{
	int r, c, d;
	cgray **p[3];
} image;

int maxval;

/*

histogram-based global threshold.

*/
#define HIST 256
long int hist[HIST];
double hist0, hist1, hist2;

/*

For compatability

*/
int GL_XRES, GL_YRES;
unsigned char *gl_pixmap = NULL;

/**
 **/
double im_horiz_score ( image *im, int s, int cols /* num of cols */ )
{
	double f, sx, sy, qy;
	int C, c, x, dx, y, z, beg, end;

	if ( im->d != 1 )
	{
		g_print ( "im_horiz_score: supported only pgm" );
		exit ( 1 );
	}

	C = im->c / cols; /* col width */

	dx = C / 40 + 1; /* 40 test-point per row */

	f = ( double ) s / C;

	if ( s >= 0 )
	{
		beg = 0;
		end = im->r - s;
	}

	else
	{
		beg = -s;
		end = im->r;
	}

	z = 0;

	for ( y = beg, sy = 0, qy = 0; y < end - 1; y++ )
	{
		for ( c = 0; c < im->c; c += C )
			for ( x = 0, sx = 0; x < C; x += dx )
				sx += IMP ( im, x + c , y + ( int ) rintf ( f * x ) , z );

		sy += sx;
		qy += sx * sx;
	}

	sy /= ( im->r - abs ( s ) );
	qy /= ( im->r - abs ( s ) );
	return qy - sy * sy;
}

#define MAX_SKEW 0.2
/**
 **/
double im_horiz_calc ( image *im, int cols )
{
	int s, step, sm;
	double fm, f, c;

	for ( step = ( im->c / cols ) * MAX_SKEW / 2, sm = 0, fm = im_horiz_score ( im, sm, cols ); step; step *= c )
	{
		s = sm;
		c = 0.7;
		s += step;
		f = im_horiz_score ( im, s, cols );

		if ( f > fm )
		{
			fm = f;
			sm = s;
			c = 0.7;
		}

		s -= step * 2;
		f = im_horiz_score ( im, s, cols );

		if ( f > fm )
		{
			fm = f;
			sm = s;
			c = 0.7;
		}
	}

	f = ( double ) ( sm ) / ( im->c / cols );
	return f;
}

/* ******* TRANSFORM ******* */
/**
 **/
void im_shear_y ( image *im, double f )
{
	int D, d, x, y, z, s;
	double m1, m0;
	cgray *p;

	if ( im->d != 1 )
	{
		g_print ( "im_shear_y: supported only pgm" );
		exit ( 1 );
	}

	s = ( int ) ( f * ( im->c - 1 ) );
	/* p= pgm_allocrow(im->r); */
	p = alloca ( im->r );
	D = -s / 2;

	for ( z = 0; z < im->d; z++ )
		for ( x = 0; x < im->c; x++ )
		{
			if ( ( m1 = modff ( f * x + D, &m0 ) ) < 0 ) m0--, m1++;

			d = m0;
			m0 = 1 - m1;

			if ( d < 0 )
			{
				for ( y = 0; y + d < 0 ; y++ ) p[y] = IMP ( im, x, 0, z );

				for ( ; y < im->r; y++ ) p[y] = IMP ( im, x, y + d, z );
			}

			else
			{
				for ( y = 0; y + d < im->r; y++ ) p[y] = IMP ( im, x, y + d, z );

				for ( ; y < im->r; y++ ) p[y] = IMP ( im, x, im->r - 1, z );
			}

			for ( y = 0; y < im->r - 1; y++ )
				IMP ( im, x, y, z ) = TRIM ( m0 * p[y] + m1 * p[y+1] , 0 , maxval );

			IMP ( im, x, y, z ) = p[y];
		}

	/* pgm_freerow(p); */
}

/**
 **/
void im_shear_x ( image *im, double f )
{
	int D, d, x, y, z, s;
	double m0, m1;
	cgray *p;

	if ( im->d != 1 )
	{
		g_print ( "im_shear_x: supported only pgm" );
		exit ( 1 );
	}

	s = ( int ) ( f * ( im->r - 1 ) );
	/*pgm_allocrow(im->c);*/
	p = alloca ( im->c );
	D = -s / 2;

	for ( z = 0; z < im->d; z++ )
		for ( y = 0; y < im->r; y++ )
		{
			if ( ( m1 = modff ( f * y + D, &m0 ) ) < 0 ) m0--, m1++;

			d = m0;
			m0 = 1 - m1;

			if ( d < 0 )
			{
				for ( x = 0; x + d < 0 ; x++ ) p[x] = IMP ( im, 0, y, z );

				for ( ; x < im->c; x++ ) p[x] = IMP ( im, x + d, y, z );
			}

			else
			{
				for ( x = 0; x + d < im->c; x++ ) p[x] = IMP ( im, x + d, y, z );

				for ( ; x < im->c; x++ ) p[x] = IMP ( im, im->c - 1, y, z );
			}

			for ( x = 0; x < im->c - 1; x++ )
				IMP ( im, x, y, z ) = TRIM ( m0 * p[x] + m1 * p[x+1], 0, maxval );

			IMP ( im, x, y, z ) = p[x];
		}

	/* pgm_freerow(p); */
}

/**
 * guchar * gdk_pixbuf_get_pixels (const GdkPixbuf *pixbuf);
 **/
int deskew ( int reset, int XRES, int YRES, unsigned char *pixmap, int sk )
{
	static image img;
	static double f, a, b;
	static int st;
	int r;

	/* default to incomplete */
	r = 1;

	if ( reset )
	{
		st = 1;
	}

	else if ( st == 1 )
	{
		img.r = YRES;
		img.c = XRES;
		img.d = 1;
		img.p[0] = img.p[1] = img.p[2] = NULL;
		gl_pixmap = pixmap;
		GL_XRES = XRES;
		GL_YRES = YRES;
		maxval = 255;
		//show_hint(2,"DESKEW: detecting...");
		g_print ( "DESKEW:: detecting...\n" );
		st = 2;
	}

	else if ( st == 2 )
	{
		f = im_horiz_calc ( &img, sk );
		//show_hint(2,"DESKEW: found <%f>, now rotating",f);
		g_print ( "DESKEW: found <%f>, now rotating\n", f );
		st = 3;
	}

	else if ( st == 3 )
	{
		a = ( hypot ( 1, f ) - 1 ) / f;
		b = -2 * a / ( a * a + 1 );
		im_shear_x ( &img, -a );
		im_shear_y ( &img, -b );
		im_shear_x ( &img, -a );
		//show_hint(2,"DESKEW: finished");
		g_print ( "DESKEW: finished\n" );
		st = 0;
		r = 0;
	}

	return ( r );
}

/**
 **/
void im_stat ( image *im )
{
	int x, y, z;

	if ( im->d != 1 )
	{
		g_print ( "im_stat: supported only pgm" );
		exit ( 1 );
	}

	for ( x = 0; x < HIST; x++ ) hist[x] = 0;

	z = 0;

	for ( y = 0; y < im->r; y++ )
		for ( x = 0; x < im->c; x++ )
			hist[HIST*IMP ( im, x, y, z ) /maxval]++;
}

/**
 **/
double hist_mean ( double A, double B )
{
	int a, b, x;
	double f;
	a = HIST * A;
	b = HIST * B;
	hist0 = hist1 = hist2 = 0.0;

	for ( x = a; x < b; x++ )
	{
		hist0 += hist[x];
		hist1 += ( f = x * hist[x] );
		hist2 += f * x;
	}

	return hist1 / hist0 / HIST;
}

/**
 **/
double hist_weight ( double A, double B, int entropy )
{
	double f;
	/* DBG("<%f,%f:",A,B); */
	f = hist_mean ( A, B );
	f = ( 1 + hist2 / hist0 ) / HIST / HIST - f * f;

	if ( entropy ) f = log ( f / hist0 / hist0 );

	f *= hist0;
	/* DBG("%f>",f); */
	return f;
}

#define SPLIT_SCORE(a,b,i,m)\
 hist_weight((double)(a)/HIST,(double)(i)/HIST,m) + hist_weight((double)(i+1)/HIST,(double)(b)/HIST,m)

/**
 **/
double hist_split ( double A, double B, int m )
{
	int a, b, s, step, sm;
	double fm, f, f0;

	a = HIST * A;
	b = HIST * B;

	for ( step = ( b - a ) / 4, s = sm = ( a + b ) / 2, fm = SPLIT_SCORE ( a, b, sm, m ); step; step /= -2 )
	{
		s = sm;
		/* DBG("\n\n%d  %f",s,fm); */
		s += step;
		f = SPLIT_SCORE ( a, b, s, m );

		if ( f < fm )
		{
			fm = f;
			sm = s;
		}

		/* DBG("\n%d  %f",s,f); */
		s -= step * 2;
		f = SPLIT_SCORE ( a, b, s, m );

		if ( f < fm )
		{
			fm = f;
			sm = s;
		}

		/* DBG("\n%d  %f",s,f); */
	}

	if ( m )
	{
		f0 = hist_weight ( 0, 1, m );
		g_print ( "\nautothreshold score = %4f", ( f0 - fm ) / hist0 );

		if ( f0 <= fm ) MSG ( " !!! WARNING !!! AUTOTHRESHOLD FAILED !!!" );
	}

	return ( double ) sm / HIST;
}

/**
 **/
double hist_thresh ( int XRES, int YRES, unsigned char *pixmap )
{
	double thr = 0;
	static image img;
	static int st;

	{
		img.r = YRES;
		img.c = XRES;
		img.d = 1;
		img.p[0] = img.p[1] = img.p[2] = NULL;
		//gl_pixmap = pixmap;
		GL_XRES = XRES;
		GL_YRES = YRES;
		maxval = 255;
		show_hint ( 2, "computing threshold..." );
		st = 2;
		im_stat ( &img );
		thr = hist_split ( 0, 1, thr );
		return ( thr );
	}

}
