#ifdef _M_IX86
#include <windows.h>
#endif
#include "global.h"

#include <stdio.h>
#include <jpeglib.h>
#include "global.h"

#include <GL/gl.h>
#include "image.h"
#include "jpeg.h"
#include "tiffio.h"

/*--------------------------------------------------------------------------*/
/*  JPEG.CPP								    */
/*    This class allows the user to read and manipulate JPEG (and SGI RGB)  */
/*    image files; data is stored in a linear array in RGBA format (1 byte  */
/*    per component, or 4 bytes per pixel)				    */
/*									    */
/*- Modification History ---------------------------------------------------*/
/*  When:	Who:			Comments:			    */
/*									    */
/*  24-Jun-93	Christopher G. Healey	Implemented from SGI readimg.c	    */
/*  11-May-94	Christopher G. Healey	Converted for RGB2PS program	    */
/*  17-Dec-99   Christopher G. Healey	Converted to make JPEG the standard */
/*                                      image format, with RGB as another   */
/*                                      option				    */
/*  19-Dec-99   Christopher G. Healey   Planned out capture, resize	    */
/*  20-Dec-99	Christopher G. Healey	Converted from ABGR packed order to */
/*					RGBA to support direct framebuffer  */
/*					captures			    */
/*  01-July-03	Christopher G. Healey	Added support for TIFF images	    */
/*--------------------------------------------------------------------------*/

#ifdef _M_IX86
#pragma warning( disable: 4244 )
#endif


short jpeg::a( int x, int y )

  //  This method returns the alpha component of pixel @ (x,y) in image
  //
  //  x,y:  X, Y-coordinate of pixel to query
{
  short  pix_val;			// Pixel's blue component


  pix_val = short( pixel( x, y ) & 0x000000ff );
  return pix_val;
}					// End method a


void jpeg::a( int x, int y, short val )

  //  This method sets the alpha component of pixel @ (x,y) in image
  //
  //  x,y:  X, Y-coordinate of pixel to set
  //  val:  New alpha value to set
{
  long  c;				// New pixel colour


  c = ( pixel( x, y ) & 0xffffff00 ) | long( val );
  pixel( x, y, c );
}					// End method a


short jpeg::b( int x, int y )

  //  This method returns the blue component of pixel @ (x,y) in image
  //
  //  x,y:  X, Y-coordinate of pixel to query
{
  short  pix_val;			// Pixel's blue component


  pix_val = short( ( pixel( x, y ) & 0x0000ff00 ) >> 8 );
  return pix_val;
}					// End method b


void jpeg::b( int x, int y, short val )

  //  This method sets the blue component of pixel @ (x,y) in image
  //
  //  x,y:  X, Y-coordinate of pixel to set
  //  val:  New blue value to set
{
  long  c;				// New pixel colour


  c = ( pixel( x, y ) & 0xffff00ff ) | ( long( val ) << 8 );
  pixel( x, y, c );
}					// End method b


int jpeg::capture( int x, int y, int w, int h )

  //  This method captures a rectangular region of the foreground
  //  framebuffer from lower-left (x,y) to upper-right (x+w,y+h)
  //
  //  x,y:  Lower-left corner of capture region
  //  w,h:  Width, height of capture region
{
  int    i, j;				// Loop counters
  long   r, g, b, a;			// R, G, B, A values
  GLint  r_buf;				// Current GL_READ_BUFFER


  if ( !create( w, h ) ) {
	  cerr << "jpeg::capture(), out of memory capturing framebuffer\n";
    return 0;
  }

  glPixelStorei( GL_PACK_ALIGNMENT, 1 );

  glGetIntegerv( GL_READ_BUFFER, &r_buf );
  glReadBuffer( GL_FRONT );

  for( i = y; i < y + h; i++ ) {	// Read scanline by scanline
    glReadPixels( x, i, w, 1, GL_RGBA, GL_UNSIGNED_BYTE,
                  (GLvoid *) &(pix[ ( i - y ) * xsize ]) );

  //  On PCs data bytes are reversed ABGR, so must swap them back to
  //  get expected ordering RGBA

#ifdef _M_IX86
    for( j = ( i - y ) * xsize; j < ( i - y + 1 ) * xsize; j++ ) {
      a = ( pix[ j ] & 0xff000000 ) >> 24;
      b = ( pix[ j ] & 0xff0000 ) >> 16;
      g = ( pix[ j ] & 0xff00 ) >> 8;
      r = ( pix[ j ] & 0xff );
      pix[ j ] = ( r << 24 ) | ( g << 16 ) | ( b << 8 ) | a;
    }
#endif
  }

  glReadBuffer( r_buf );
  return 1;
}					// End method capture


int jpeg::copy( const jpeg& img )

  //  This private method copies state from an existing JPEG object
  //
  //  img:  Image to copy from
{
  int  i;				// Loop counter


  //  Attempt to allocate memory for pixel map (also sets xsize and
  //  ysize)

  if ( !create( img.xsize, img.ysize ) ) {
	cerr << "jpeg::copy(), out of memory copying JPEG object\n";
    return 0;
  }

  depth = img.depth;			// Copy name, image depth, quality
  strcpy( name, img.name );
  qual = img.qual;

  for( i = 0; i < xsize * ysize; i++ ) {
    pix[ i ] = img.pix[ i ];
  }

  return 1;
}					// End method copy


int jpeg::create( int w, int h )

  //  This method creates a new RGB image, which is initialized to all
  //  black
  //
  //  w:  Width of new pixmap
  //  h:  Height of new pixmap
{
  xsize = w;
  ysize = h;

  if ( pix != NULL ) {			// Free old pixel array
    delete [] pix;
  }

  pix = new unsigned long[ xsize * ysize ];

  if ( !pix ) {				// Reset on out of memory
	cerr << "jpeg::create(), out of memory creating JPEG object\n";

    xsize = 0;
    ysize = 0;
    return 0;
  }

  return 1;
}					// End method create


void jpeg::draw()

  //  This routine draws the image into the current framebuffer
{
  glRasterPos2i( 0, 0 );
  glDrawPixels( xsize, ysize, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid *) pix );
}					// End method draw


short jpeg::g( int x, int y )

  //  This method returns the green component of pixel @ (x,y) in image
  //
  //  x,y:  X, Y-coordinate of pixel to query
{
  short  pix_val;			// Pixel's green component


  pix_val = short( ( pixel( x, y ) & 0x00ff0000 ) >> 16 );
  return pix_val;
}					// End method g


void jpeg::g( int x, int y, short val )

  //  This method sets the green component of pixel @ (x,y) in image
  //
  //  x,y:  X, Y-coordinate of pixel to set
  //  val:  New green value to set
{
  long  c;				// New pixel colour


  c = ( pixel( x, y ) & 0xff00ffff ) | ( long( val ) << 16 );
  pixel( x, y, c );
}					// End method g


int jpeg::height()

  //  This method returns the height of the image
{
   return ysize;
}					// End method height


long jpeg::pixel( int x, int y )

  //  This method returns the pixel @ (x,y) in image
  //
  //  x,y:  X, Y-coordinate of pixel to query
{
  int  index;				// Index into pixel map


  if ( x < 0 || x >= xsize || y < 0 || y >= ysize ) {
    return 0;
  }

  index = ( y * xsize ) + x;
  return pix[ index ];			// Return selected pixel
}					// End method pixel


void jpeg::pixel( int x, int y, long val )

  //  This method stores the pixel @ (x,y) in image
  //
  //  x,y:  X, Y-coordinate of pixel to query
  //  val:  Value to store
{
  int  index;				// Index into pixel map


  if ( x < 0 || x >= xsize || y < 0 || y >= ysize ) {
    return;
  }

  index = ( y * xsize ) + x;
  pix[ index ] = val;			// Assign to selected pixel
}					// End method pixel


int jpeg::quality()

  //  This method returns the current quality level
{
  return qual;
}					// End method quality


void jpeg::quality( int q )

  //  This method sets the image quality level
  //
  //  q:  Quality (0-100%)
{
  if ( q < 0 || q > 100 ) {
	cerr << "jpeg::quality(), invalid quality " << q << "\n";
    return;
  }

  qual = q;
}					// End method quality


short jpeg::r( int x, int y )

  //  This method returns the red component of pixel @ (x,y) in image
  //
  //  x,y:  X, Y-coordinate of pixel to query
{
  short  pix_val;			// Pixel's red component


  pix_val = short( ( pixel( x, y ) & 0xff000000 ) >> 24 );
  return pix_val;
}					// End method r


void jpeg::r( int x, int y, short val )

  //  This method sets the red component of pixel @ (x,y) in image
  //
  //  x,y:  X, Y-coordinate of pixel to set
  //  val:  New red value to set
{
  long  c;				// New pixel colour


  c = ( pixel( x, y ) & 0x00ffffff ) | ( long( val ) << 24 );
  pixel( x, y, c );
}					// End method r


int jpeg::read( char file[] )

  //  This method opens a JPEG file, reads the information and stores
  //  it in the image instance variable
  //
  //  file:  Name of JPEG file
{
  FILE *in;				// Binary input file handle
  int   rc;				// Return code

  jpeg_decompress_struct c_info;	// JPEG configuration info struct
  jpeg_error_mgr         jerr;		// JPEG error handler


  c_info.err = jpeg_std_error( &jerr );	// Standard JPEG error handler
  jpeg_create_decompress( &c_info );

  if ( ( in = fopen( file, "rb" ) ) == NULL ) {
    cerr << "jpeg::read(), cannot open input JPEG file " << file << "\n";
    return 0;
  }
  jpeg_stdio_src( &c_info, in );

  jpeg_read_header( &c_info, TRUE );	// Read header info
  jpeg_start_decompress( &c_info );	// Start JPEG decompression

  switch( c_info.output_components ) {	// Switch on image depth
  case 1:
    rc = read_jpeg_grey( c_info );
    break;

  case 3:
    rc = read_jpeg_rgb( c_info );
    break;

  default:
    cerr << "jpeg::read(), file " << file << " has an unknown depth\n";
    rc = 0;
  }

  jpeg_finish_decompress( &c_info );	// Decompression completed
  jpeg_destroy_decompress( &c_info );

  return rc;
}					// End routine read


int jpeg::read_jpeg_grey( jpeg_decompress_struct& c_info )

  //  This method opens an greyscale JPEG file, reads the information
  //  and stores it in the image instance variable
  //
  //  c_info:  JPEG configuration info struct
{
  int      c;				// Column index in current JPEG row
  int      i, j;			// Loop counters
  int      p;				// Pixel index in packed image data
  JSAMPLE *row;				// Single row of R,G,B values


  row = new JSAMPLE[ c_info.output_width ];

  if ( !row || !create( c_info.output_width, c_info.output_height ) ) {
    cerr << "jpeg::read_jpeg_grey(), out of memory reading JPEG file\n";
    return 0;
  }

  //  Read all scanlines, since JPEG has first scanline at upper-left
  //  and RGB has first scanline at lower-left, must reverse the order
  //  of the scanlines

  for( i = ysize - 1; i >= 0; i-- ) {
    p = i * xsize;			// Starting pixel for current scanline
    jpeg_read_scanlines( &c_info, &row, 1 );

    //  For all pixels in a scanline, pack them into the pix[] array,
    //  placing them in B,G,R order

    c = 0;
    for( j = 0; j < xsize; j++ ) {
      pix[ p ] = 0xff;
      pix[ p ] |= long( row[ c ] ) << 24;
      pix[ p ] |= long( row[ c ] ) << 16;
      pix[ p++ ] |= long( row[ c++ ] ) << 8;
    }
  }

  delete [] row;
  return 1;
}					// End routine read_jpeg_grey


int jpeg::read_jpeg_rgb( jpeg_decompress_struct& c_info )

  //  This method opens an rgb JPEG file, reads the information and
  //  stores it in the image instance variable
  //
  //  c_info:  JPEG configuration info struct
{
  int      c;				// Column index in current JPEG row
  int      i, j;			// Loop counters
  int      p;				// Pixel index in packed image data
  JSAMPLE *row;				// Single row of R,G,B values


  row = new JSAMPLE[ c_info.output_width * 3 ];

  if ( !row || !create( c_info.output_width, c_info.output_height ) ) {
    cerr << "jpeg::read_jpeg_rgb(), out of memory reading JPEG file\n";
    return 0;
  }

  //  Read all scanlines, since TIFF has first scanline at upper-left
  //  and RGB has first scanline at lower-left, must reverse the order
  //  of the scanlines

  for( i = ysize - 1; i >= 0; i-- ) {
    p = i * xsize;			// Starting pixel for current scanline
    jpeg_read_scanlines( &c_info, &row, 1 );

    //  For all pixels in a scanline, pack them into the pix[] array,
    //  placing them in B,G,R order

    c = 0;
    for( j = 0; j < xsize; j++ ) {
      pix[ p ] = 0xff;
      pix[ p ] |= long( row[ c++ ] ) << 24;
      pix[ p ] |= long( row[ c++ ] ) << 16;
      pix[ p++ ] |= long( row[ c++ ] ) << 8;
    }
  }

  delete [] row;
  return 1;
}					// End routine read_jpeg_rgb


int jpeg::read_tiff( char file[] )

  //  This method opens a TIFF file, reads the information and stores
  //  it in the image instance variable
  //
  //  file:  Name of TIFF file
{
  uint32 *buf;				// Image buffer
  uint32  h;				// Height of image
  TIFF   *tif;				// Pointer to TIFF data stream
  uint32  w;				// Width of image


  if ( ( tif = TIFFOpen( file, "r" ) ) == NULL ) {
    return 0;
  }

  //  Read width, height of image, allocate a buffer to hold image's
  //  RGBA data

  TIFFGetField( tif, TIFFTAG_IMAGEWIDTH, &w );
  TIFFGetField( tif, TIFFTAG_IMAGELENGTH, &h );

  buf = (uint32 *) _TIFFmalloc( w * h * sizeof( uint32 ) );
  if ( buf == NULL ) {
    TIFFClose( tif );
    return 0;
  }

  //  Read the image data and process into the appropriate instance
  //  variables

  if ( TIFFReadRGBAImage( tif, w, h, buf, 0 ) ) {
  }

  TIFFClose( tif );			// Close TIFF file
  _TIFFfree( buf );			// Free image buffer

  return 1;
}					// End routine read_tiff


//  Modules associated with SGI RGB files are only compiled on the
//  SGI/IRIX platform (a CC directive is included in this platform's
//  makefile to define "SGI" to TRUE)

#ifdef SGI

int jpeg::read_rgb( char file[] )

  //  This method opens an RGB file, reads the information and stores
  //  it in the image instance variables
  //
  //  file:  Name of RGB file
{
  IMAGE *img;				// Pointer to RGB header
  int    tmp = 0;			// Temporary integer value


  //  Open file, ensure it exists, ensure it is an RGB file

  img = iopen( file, "r", tmp, tmp, tmp, tmp, tmp );
  if ( img == NULL ) {
     cerr << "jpeg::read_rgb(), RGB file " << file << " does not exist\n";
     return 0;
  }

  strcpy( name, file );

  switch( img->zsize ) {		// Switch on image depth
  case 1:
    return read_rgb_grey( img );

  case 3:
    return read_rgb_rgb( img );

  default:
    cerr << "jpeg::read_rgb(), file " << file << " has an unknown depth\n";
    return 0;
  }					// End switch on image depth
}					// End method read


int jpeg::read_rgb_grey( IMAGE *img )

  //  This private method reads a greyscale RGB image
  //
  //  img:  Pointer to RGB image file
{
  short *g;				// Intensity component array
  int    i, j;				// Loop counters
  int    index = 0;			// Index into pixel map


  depth = img->zsize;			// Set image depth, width, height
  xsize = img->xsize;
  ysize = img->ysize;

  g = new short[ xsize ];

  if ( !g || !create( xsize, ysize ) ) {
    cerr << "jpeg::read_rgb_grey(), out of memory reading RGB file\n";

    xsize = 0;				// Reset width, height of image
    ysize = 0;
    return 0;
  }

  for( i = 0; i < ysize; i++ ) {	// For every row..
    getrow( img, g, i, 0 );

    for( j = 0; j < xsize; j++ ) {	// Copy row data into pixel map
      pix[ index ] = 0xff;
      pix[ index ] |= long( g[ j ] ) << 24;
      pix[ index ] |= long( g[ j ] ) << 16;
      pix[ index++ ] |= long( g[ j ] ) << 8;
    }					// End for all columns in row
  }					// End for all rows in image

  delete [] g;
  return 1;
}					// End method read_rgb_grey


int jpeg::read_rgb_rgb( IMAGE *img )

  //  This private method reads an rgb RGB image
  //
  //  img:  Pointer to RGB image file
{
  int    i, j;				// Loop counters
  int    index = 0;			// Index into pixel map

  short *b;				// Blue component array
  short *g;				// Green component array
  short *r;				// Red component array


  depth = img->zsize;			// Set image depth, width, height
  xsize = img->xsize;
  ysize = img->ysize;

  r = new short[ xsize ];		// Allocate components, pixel map
  g = new short[ xsize ];
  b = new short[ xsize ];

  if ( !r || !g || !b || !create( xsize, ysize ) ) {
    cerr << "jpeg::read_rgb_rgb(), out of memory reading RGB file\n";

    xsize = 0;				// Reset width, height of image
    ysize = 0;
    return 0;
  }

  for( i = 0; i < ysize; i++ ) {	// For every row..
    getrow( img, r, i, 0 );		// ..read image data
    getrow( img, g, i, 1 );
    getrow( img, b, i, 2 );

    for( j = 0; j < xsize; j++ ) {	// Copy row data into pixel map
      pix[ index ] = 0xff;
      pix[ index ] |= long( r[ j ] ) << 24;
      pix[ index ] |= long( g[ j ] ) << 16;
      pix[ index++ ] |= long( b[ j ] ) << 8;
    }					// End for all columns in row
  }					// End for all rows in image

  delete [] r;				// Free pixel row storage
  delete [] g;
  delete [] b;

  return 1;
}					// End method read_rgb_rgb

#endif					// For "#ifdef SGI"; IRIX platform


int jpeg::resize( int x, int y, long fill )

  //  This method resizes the image's width and height
  //
  //  x,y:   New image width, height
  //  fill:  Colour to use to fill any new pixels
{
  if ( !resize_x( x, fill ) ) {
    return 0;
  }

  if ( !resize_y( y, fill ) ) {
    return 0;
  }

  return 1;
}					// End method resize


int jpeg::resize_x( int x, long fill )

  //  This method changes the width of the image
  //
  //  x:     New width
  //  fill:  Colour to use to fill new pixels
{
  int            i, j;			// Loop counters
  unsigned long *new_pix;		// New pixel array


  if ( x == xsize ) {			// No change req'd
    return 0;
  }

  new_pix = new unsigned long[ x * ysize ];
  if ( !new_pix ) {
    cerr << "jpeg::resize_x(), out of memory resizing image width\n";
    return 0;
  }

  for( i = 0; i < ysize; i++ ) {	// For all rows
    if ( x < xsize ) {			// Make row smaller
      for( j = 0; j < x; j++ ) {
        new_pix[ ( i * x ) + j ] = pix[ ( i * xsize ) + j ];
      }

    } else {				// Make row bigger
      for( j = 0; j < xsize; j++ ) {
        new_pix[ ( i * x ) + j ] = pix[ ( i * xsize ) + j ];
      }

      for( j = xsize; j < x; j++ ) {	// Fill new pixels
        new_pix[ ( i * x ) + j ] = fill;
      }
    }
  }

  delete [] pix;			// Free old image array
  pix = new_pix;			// Reference new image array

  xsize = x;				// Save new image width
  return 1;
}					// End method resize_x


int jpeg::resize_y( int y, long fill )

  //  This method changes the height of the image
  //
  //  y:     New height
  //  fill:  Colour to use to fill new pixels
{
  int            i, j;			// Loop counters
  unsigned long *new_pix;		// New pixel array


  if ( y == ysize ) {			// No change req'd
    return 0;
  }

  new_pix = new unsigned long[ xsize * y ];
  if ( !new_pix ) {
    cerr << "jpeg::resize_y(), out of memory resizing image height\n";
    return 0;
  }

  if ( y < ysize ) {			// Make image shorter
    for( i = 0; i < y; i++ ) {
      for( j = 0; j < xsize; j++ ) {
        new_pix[ ( i * xsize ) + j ] = pix[ ( i * xsize ) + j ];
      }
    }

  } else {				// Make image taller
    for( i = 0; i < ysize; i++ ) {
      for( j = 0; j < xsize; j++ ) {
        new_pix[ ( i * xsize ) + j ] = pix[ ( i * xsize ) + j ];
      }
    }

    for( i = ysize; i < y; i++ ) {	// Fill new pixels
      for( j = 0; j < xsize; j++ ) {
        new_pix[ ( i * xsize ) + j ] = fill;
      }
    }
  }

  delete [] pix;			// Free old image array
  pix = new_pix;			// Reference new image array

  ysize = y;				// Save new image height
  return 1;
}					// End method resize_x


int jpeg::width()

  //  This method returns the width of the image
{
   return xsize;
}					// End method width


int jpeg::write( char file[], int comp )

  //  This method writes either a greyscale (comp=1) or rgb (comp=3)
  //  JPEG image file
  //
  //  file:  Name of file to write
  //  comp:  Number of colour components (1=greyscale, 3=rgb)
{
  switch( comp ) {
  case 1:
    return write_jpeg_grey( file );

  case 3:
    return write_jpeg_rgb( file );

  default:
    cerr << "jpeg::write(), ";
    cerr << "unknown components " << comp << " writing JPEG image\n";
    return 0;
  }
}					// End routine write


int jpeg::write_jpeg_grey( char file[] )

  //  This private method writes an greyscale JPEG file
  //
  //  file:  Name of output JPEG file
{
  int      c;				// Column index in current JPEG row
  int      i, j;			// Loop counters
  int      mis_x, mis_y;		// First occurance of mismatched pixel
  int      mismatch = 0;		// Nongrey pixels where R != G != B
  FILE    *out;				// Binary output file handle
  int      p;				// Pixel index in packed image data
  short    r, b;			// R, B of current pixel
  JSAMPLE *row;				// Single row of R,G,B values

  jpeg_compress_struct  c_info;		// JPEG configuration info struct
  jpeg_error_mgr        jerr;		// JPEG error handler


  c_info.err = jpeg_std_error( &jerr );	// Standard JPEG error handler
  jpeg_create_compress( &c_info );

  c_info.image_width = xsize;		// Set defaults
  c_info.image_height = ysize;
  c_info.input_components = 1;
  c_info.in_color_space = JCS_GRAYSCALE;
  jpeg_set_defaults( &c_info );

  jpeg_set_quality( &c_info, qual, 1 );	// Set quality level

  if ( ( out = fopen( file, "wb" ) ) == NULL ) {
    cerr << "jpeg::write_jpeg_grey(), ";
    cerr << "cannot open output JPEG file " << file << "\n";
    return 0;
  }
  jpeg_stdio_dest( &c_info, out );

  row = new JSAMPLE[ xsize * 3 ];	// Create scanline (R,G,B)

  jpeg_start_compress( &c_info, TRUE );	// Start JPEG compression

  //  Write all scanlines, since JPEG has first scanline at
  //  upper-right and RGB has first scanline at lower-right, must
  //  reverse the order of the scanlines

  for( i = ysize - 1; i >= 0; i-- ) {
    p = i * xsize;			// Starting pixel for current scanline

   //  For all pixels in a scanline, unpack them from the pix[]
   //  array, place them in R,G,B order into the scanline, then write
   //  the scanline to the JPEG file

    c = 0;
    for( j = 0; j < xsize; j++ ) {
      row[ c++ ] = ( pix[ p ] >> 16 ) & 0xff;
      r = ( pix[ p ] >> 24 ) & 0xff;
      b = ( pix[ p++ ] >> 8 ) & 0xff;

      //  Count mismatched pixels where R != G or B != G, report as a
      // warning at the end of writing

      if ( r != row[ c - 1 ] || b != row[ c - 1 ] ) {
        if ( mismatch++ == 0 ) {
           mis_x = j;
           mis_y = i;
        }
      }

    }
    jpeg_write_scanlines( &c_info, &row, 1 );
  }

  jpeg_finish_compress( &c_info );	// Compression completed
  jpeg_destroy_compress( &c_info );

  if ( mismatch != 0 ) {		// Report mismatched pixels
    cerr << "jpeg::write_jpeg_grey(), " << mismatch << " non-grey pixels, ";
    cerr << "first occurrance at (" << mis_x << "," << mis_y << ")\n";
  }

  fclose( out );
  delete [] row;

  return 1;
}					// End routine jpeg_write_grey


int jpeg::write_jpeg_rgb( char file[] )

  //  This private method writes an RGB JPEG file
  //
  //  file:  Name of output JPEG file
{
  int      c;				// Column index in current JPEG row
  int      i, j;			// Loop counters
  FILE    *out;				// Binary output file handle
  int      p;				// Pixel index in packed image data
  JSAMPLE *row;				// Single row of R,G,B values

  jpeg_error_mgr        jerr;		// JPEG error handler
  jpeg_compress_struct  c_info;		// JPEG configuration info struct


  c_info.err = jpeg_std_error( &jerr );	// Standard JPEG error handler
  jpeg_create_compress( &c_info );

  c_info.image_width = xsize;		// Set defaults
  c_info.image_height = ysize;
  c_info.input_components = 3;
  c_info.in_color_space = JCS_RGB;
  jpeg_set_defaults( &c_info );

  jpeg_set_quality( &c_info, qual, 1 );	// Set quality level

  if ( ( out = fopen( file, "wb" ) ) == NULL ) {
    cerr << "jpeg::write_jpeg_rgb(), ";
    cerr << "cannot open output JPEG file " << file << "\n";
    return 0;
  }
  jpeg_stdio_dest( &c_info, out );

  row = new JSAMPLE[ xsize * 3 ];	// Create scanline (R,G,B)

  jpeg_start_compress( &c_info, TRUE );	// Start JPEG compression

  //  Write all scanlines, since JPEG has first scanline at
  //  upper-right and RGB has first scanline at lower-right, must
  //  reverse the order of the scanlines

  for( i = ysize - 1; i >= 0; i-- ) {
    p = i * xsize;			// Starting pixel for current scanline

   //  For all pixels in a scanline, unpack them from the pix[]
   //  array, place them in R,G,B order into the scanline, then write
   //  the scanline to the JPEG file

    c = 0;
    for( j = 0; j < xsize; j++ ) {
      row[ c++ ] = ( pix[ p ] >> 24 ) & 0xff;
      row[ c++ ] = ( pix[ p ] >> 16 ) & 0xff;
      row[ c++ ] = ( pix[ p++ ] >> 8 ) & 0xff;
    }
    jpeg_write_scanlines( &c_info, &row, 1 );
  }

  jpeg_finish_compress( &c_info );	// Compression completed
  jpeg_destroy_compress( &c_info );

  fclose( out );
  delete [] row;

  return 1;
}					// End routine write_jpeg_grey


int jpeg::write_tiff( char file[], int comp )

  //  This method writes either a greyscale (comp=1) or rgb (comp=3)
  //  TIFF image file
  //
  //  file:  Name of file to write
  //  comp:  Number of colour components (1=greyscale, 3=rgb)
{
  switch( comp ) {
  case 1:
    return 0;

  case 3:
    return write_tiff_rgb( file );

  default:
    cerr << "jpeg::write_tiff(), ";
    cerr << "unknown components " << comp << " writing TIFF image\n";
    return 0;
  }
}					// End routine write_tiff


int jpeg::write_tiff_rgb( char file[] )

  //  This private method writes an RGB TIFF file
  //
  //  file:  Name of output TIFF file
{
  int     c;				// Current entry in row buffer
  int     i, j;				// Loop counters
  TIFF   *out;				// TIFF output file stream
  int     p;				// Current pixel
  uint32  row;				// Current row
  int     row_len;			// Byte length of row of pixels
  int     smp_p_pix = 3;		// Samples per pixel (RGB)

  unsigned char *buf;			// Buffered row of pixels


  if ( ( out = TIFFOpen( file, "w" ) ) == NULL ) {
    return 0;
  }

  //  Initalize image file directory entries

  TIFFSetField( out, TIFFTAG_IMAGEWIDTH, xsize );
  TIFFSetField( out, TIFFTAG_IMAGELENGTH, ysize );
  TIFFSetField( out, TIFFTAG_SAMPLESPERPIXEL, smp_p_pix );
  TIFFSetField( out, TIFFTAG_BITSPERSAMPLE, 8 );
  TIFFSetField( out, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT );
  TIFFSetField( out, TIFFTAG_COMPRESSION, COMPRESSION_LZW );
  TIFFSetField( out, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG );
  TIFFSetField( out, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB );

  row_len = smp_p_pix * xsize;

  if ( TIFFScanlineSize( out ) <= row_len ) {
    buf = (unsigned char *) _TIFFmalloc( row_len );
  } else {
    buf = (unsigned char *) _TIFFmalloc( TIFFScanlineSize( out ) );
  }

  //  Request the desired default strip size to be the byte length of
  //  one row of pixels, take the actual value set by the TIFF library
  //  and update the ROWSPERSTRIP entry in the image file directory

  TIFFSetField(
    out, TIFFTAG_ROWSPERSTRIP, TIFFDefaultStripSize( out, row_len ) );

  //  Write all scanlines, since TIFF has first scanline at
  //  upper-right and RGB has first scanline at lower-right, must
  //  reverse the order of the scanlines

  row = 0;
  for( i = ysize - 1; i >= 0; i-- ) {
    p = i * xsize;			// Starting pixel for current scanline

    //  For all pixels in a scanline, pack them into the buffer,
    //  placing them in B,G,R order

    c = 0;
    for( j = 0; j < xsize; j++ ) {
      buf[ c++ ] = long( pix[ p ] & 0xff000000 ) >> 24;
      buf[ c++ ] = long( pix[ p ] & 0xff0000 ) >> 16;
      buf[ c++ ] = long( pix[ p++ ] & 0xff00 ) >> 8;
      //buf[ c++ ] = 0xff;
    }

    if ( TIFFWriteScanline( out, buf, row++, 0 ) < 0 ) {
      return 0;
    }
  }

  TIFFClose( out );
  _TIFFfree( buf );

  return 1;
}					// End method write_tiff_rgb


//  Modules associated with SGI RGB files are only compiled on the
//  SGI/IRIX platform (a CC directive is included in this platform's
//  makefile to define "SGI" to TRUE)

#ifdef SGI

int jpeg::write_rgb( char file[], int comp )

  //  This method writes either a greyscale (comp=1) or rgb (comp=3)
  //  SGI RGB image file
  //
  //  file:  Name of file to write
  //  comp:  Number of colour components (1=greyscale, 3=rgb)
{
  switch( comp ) {
  case 1:
    return write_rgb_grey( file );

  case 3:
    return write_rgb_rgb( file );

  default:
    cerr << "jpeg::write_rgb(), ";
    cerr << "unknown components " << comp << " writing RGB image\n";
    return 0;
  }
}					// End routine write_rgb


int jpeg::write_rgb_grey( char file[] )

  //  This method writes an greyscale RGB image file
  //
  //  file:  Name of file to write
{
  short *g;				// Grey component array
  int    i, j;				// Loop counters
  IMAGE *img;				// Pointer to SGI image struct
  int    index = 0;			// Index into pixel map
  int    mis_x, mis_y;			// First occurance of mismatched pixel
  int    mismatch = 0;			// Nongrey pixels where R != G != B
  short  r, b;				// R, B of current pixel


  img = iopen( file, "w", RLE( 1 ), 2, xsize, ysize, 1 );
  if ( img == NULL ) {
    cerr << "jpeg::write_rgb_grey(), ";
    cerr << "RGB file " << file << " could not be created\n";
    return 0;
  }

  g = new short[ xsize ];		// Allocate component array

  if ( !g ) {
    cerr << "jpeg::write_rgb_grey(), ",
    cerr << "out of memory writing greyscale RGB file\n";
    return 0;
  }

  for( i = 0; i < ysize; i++ ) {	// Write pixels, row by row
    for( j = 0; j < xsize; j++ ) {
      r = short( ( pix[ index ] >> 24 ) & 0xff );
      g[ j ] = short( ( pix[ index ] >> 16 ) & 0xff );
      b = short( ( pix[ index++ ] >> 8 ) & 0xff );

      //  Count mismatched pixels where R != G or B != G, report as a
      // warning at the end of writing

      if ( r != g[ j ] || b != g[ j ] ) {
        if ( mismatch++ == 0 ) {
           mis_x = j;
           mis_y = i;
        }
      }
    }					// End for all columns

    putrow( img, g, i, 0 );		// Write current row
  }					// End for all rows

  iclose( img );
  delete [] g;

  if ( mismatch != 0 ) {		// Report mismatched pixels
    cerr << "jpeg::write_rgb_grey(), " << mismatch << " non-grey pixels, ";
    cerr << "first occurrance at (" << mis_x << "," << mis_y << ")\n";
  }

  return 1;
}					// End method write_rgb_grey


int jpeg::write_rgb_rgb( char file[] )

  //  This method writes an RGB image file
  //
  //  file:  Name of file to write
{
  int    i, j;				// Loop counters
  IMAGE *img;				// Pointer to SGI image struct
  int    index = 0;			// Index into pixel map

  short *b;				// Blue component array
  short *g;				// Green component array
  short *r;				// Red component array


  img = iopen( file, "w", RLE( 1 ), 3, xsize, ysize, 3 );
  if ( img == NULL ) {
    cerr << "jpeg::write_rgb_rgb(), ";
    cerr << "RGB file " << file << " could not be created\n";
    return 0;
  }

  r = new short[ xsize ];		// Allocate components
  g = new short[ xsize ];
  b = new short[ xsize ];

  if ( !r || !g || !b ) {
    cerr << "jpeg::write_rgb_rgb(), out of memory writing RGB file\n";
    return 0;
  }

  for( i = 0; i < ysize; i++ ) {	// Write pixels, row by row
    for( j = 0; j < xsize; j++ ) {
      r[ j ] = short( ( pix[ index ] >> 24 ) & 0xff );
      g[ j ] = short( ( pix[ index ] >> 16 ) & 0xff );
      b[ j ] = short( ( pix[ index++ ] >> 8 ) & 0xff );
    }

    putrow( img, r, i, 0 );		// Write current row
    putrow( img, g, i, 1 );
    putrow( img, b, i, 2 );
  }

  iclose( img );

  delete [] r;
  delete [] g;
  delete [] b;

  return 1;
}					// End method write_rgb_rgb

#endif					// For "#ifdef SGI"; IRIX platform


jpeg& jpeg::operator=( const jpeg& img )

  //  Assignment operator for jpeg objects, copy from already existing
  //  object
  //
  //  img:  Reference to jpeg object to copy from
{
  copy( img );
  return *this;
}					// End operator=


jpeg::jpeg()

  //  Constructor for jpeg objects, initialize instance variables
{
  name[ 0 ] = '\0';			// Set name to blank
  xsize = 0;				// Set size to 0,0
  ysize = 0;
  qual = 75;				// Default quality of 75%
  pix = NULL;				// Initialize pixel map
}					// End constructor


jpeg::jpeg( const jpeg& img )

  //  Copy constructor for jpeg objects, copy from already existing object
  //
  //  img:  Reference to jpeg object to copy from
{
  pix = NULL;
  copy( img );
}					// End copy constructor


jpeg::~jpeg()

  //  Destructor for jpeg objects, free memory allocated to pixel
  //  arrays
{
  if ( pix ) {				// Ensure image data exists
    delete [] pix;			// Free pixel map, name
  }
}					// End destructor
