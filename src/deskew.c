/*
This example finds a line in an image area of interest
then rotates the entire image to make the line perfectly horizontal or vertical.

http://www.catenary.com/howto/deskew.html#C_annotate

*/


// ........... Helper function prototypes ...........
static void calcMinRotImageArea ( double angDeg, imgdes far *image, int far *dcols, int far *drows );
int deskewwholepage ( imgdes *srcimg, POINT pt1, POINT pt2 );
int find_a_line ( imgdes *srcimg, POINT * leftmost,  POINT * rightmost );

#define CALC_WIDTH(image)  ((image)->endx - (image)->stx + 1) // Width in pixels, defined in vicdefs.h
#define CALC_HEIGHT(image) ((image)->endy - (image)->sty + 1) // Height in pixels, defined in vicdefs.h

// Deskew command selected, operates on global image defined elsewhere in the program
void DoDeskew ( HWND hWnd )
{
	int rcode, rc;
	POINT pt1, pt2;

	Image.stx = 266;
	Image.sty = 34;
	Image.endx = 402;
	Image.endy = 64;

// Find a line in the image area, pt1 and pt2 will receive the coordinates of the endpoints

	rcode = find_a_line ( &Image, &pt1, &pt2 );

	if ( rcode == NO_ERROR )
	{
		rc = MessageBox ( hWnd, "Entire image will be rotated based on black line in image area",
						  "Deskew the page", MB_OKCANCEL );

		if ( rc == IDOK )
			// Deskew whole page based on line defined by pt1 and pt2
			rcode = deskewwholepage ( &Image, pt1, pt2 );

	}

	if ( rcode != NO_ERROR )
		error_handler ( hWnd, rcode ); // Handle any errors
}



// Rotates entire image to align make a line defined by pt1 - pt2 normal
int deskewwholepage ( imgdes *srcimg, POINT pt1, POINT pt2 )
{
	int rcode;
	double deltay, deltax, angle;
	int dx, dy;
	imgdes timage;

	// Determine slope of the line and then the angle
	deltay = ( double ) ( pt2.y - pt1.y );
	deltax = ( double ) ( pt2.x - pt1.x );

	angle = atan2 ( deltay, deltax );
	angle = RADTODEG ( angle );

	if ( angle >= 45.0 )
		angle -= 90.0;
	else if ( angle <= -45.0 )
		angle += 90.0;

	// Set to rotate entire image
	srcimg->stx = 0;
	srcimg->sty = 0,
			srcimg->endx = ( UINT ) srcimg->bmh->biWidth - 1;
	srcimg->endy = ( UINT ) srcimg->bmh->biHeight - 1;
	calcMinRotImageArea ( angle, srcimg, &dx, &dy );  // see below
	// Rotate the entire image
	// Allocate space for the new image
	if ( ( rcode = allocimage ( &timage, dx, dy, srcimg->bmh->biBitCount ) ) == NO_ERROR )
	{
		// Set background to white
		zeroimage ( 255, &timage );

		/* Rotate image into timage */
		if ( ( rcode = rotate ( angle, srcimg, &timage ) ) == NO_ERROR )
		{
			/* Success, copy palette into timage */
			copyimagepalette ( srcimg, &timage );
			// Success, free source image
			freeimage ( srcimg );
			// Assign timage to the source image
			copyimgdes ( &timage, srcimg );
		}

		else // Error in rotating image, release timage memory
			freeimage ( &timage );
	}

	return ( rcode );
}

// Find black line on a white background
int find_a_line ( imgdes *srcimg, POINT * leftmost,  POINT * rightmost )
{
	int rcode = NO_ERROR;
	int x, y;
	int src_pixelval;
	int limit;

	// This function is for 1-bit and 8-bit grayscale only
	if ( srcimg->bmh->biBitCount == 1 )
		limit = 1;
	else if ( srcimg->bmh->biBitCount == 8 )
		limit = 128;
	else
		return ( BAD_BPP );

	// Assume diagonal line
	leftmost->x = srcimg->endx;
	leftmost->y = srcimg->endy;
	rightmost->x = srcimg->stx;
	rightmost->y = srcimg->sty;

	for ( y = ( int ) srcimg->sty; y <= ( int ) srcimg->endy; y++ )
	{
		for ( x = ( int ) srcimg->stx; x <= ( int ) srcimg->endx; x++ )
		{
			src_pixelval = getpixelcolor ( srcimg, x, y );

			if ( src_pixelval < limit )
			{
				if ( x < leftmost->x )
				{
					leftmost->x = x;
					leftmost->y = y;
				}

				if ( x > rightmost->x )
				{
					rightmost->x = x;
					rightmost->y = y;
				}
			}
		}
	}

	return ( rcode );
}

// Calculate the cols and rows (in pixels) that will contain the rotated image area.
static void calcMinRotImageArea ( double angDeg, imgdes far *image,
								  int far *dcols, // Dimensions of image area that can hold the
								  int far *drows ) //  rotated source image
{
#define L_MULTIP 15 // Power of 2 to scale doubles to
#define MRD_FACTOR (double)(1L << L_MULTIP)
#define MRL_FACTOR (1L << L_MULTIP)

#define MRSCALE_UPD(dval) ((long)((dval) * MRD_FACTOR))
#define MRSCALE_DNL(lval) ((int)((lval + (MRL_FACTOR - 1)) / MRL_FACTOR))
#define PI 3.141592654
#define DEGTORAD(ang) ((ang) * PI / 180.0)
	unsigned cols, rows;
	long sintheta, costheta;
	double angRad;

	// Make sure angle lies between -360 and +360
	while ( angDeg > 360.0 )
		angDeg -= 360.0;

	while ( angDeg < -360.0 )
		angDeg += 360.0;

	// Convert angle to radians
	angRad = DEGTORAD ( angDeg );
	// Calc a long int version of our sine and cosine
	sintheta = MRSCALE_UPD ( sin ( angRad ) );
	costheta = MRSCALE_UPD ( cos ( angRad ) );

	// Calc absolute values of sine and cosine
	if ( sintheta < 0L )
		sintheta = -sintheta;

	if ( costheta < 0L )
		costheta = -costheta;

	cols = CALC_WIDTH ( image );
	rows = CALC_HEIGHT ( image );
	*dcols = MRSCALE_DNL ( costheta * cols + sintheta * rows );
	*drows = MRSCALE_DNL ( sintheta * cols + costheta * rows );
}
