//  RGB.H
//    Class definition for RGB.CPP

#ifndef _jpeg_h
#define _jpeg_h

#include "global.h"
#ifdef NAMESPACE
  #include <fstream>
  #include <iostream>
  #include <string>
  using namespace std;
#else
  #include <fstream.h>
  #include <iostream.h>
  #include <string.h>
#endif

#include <jpeglib.h>

#ifdef SGI
#include "image.h"
#endif


class jpeg {
  private:
    int   depth;			// Image depth
    char  name[ 256 ];			// Name of input file
    int   qual;				// Quality (0-100) for JPEG image
    int   xsize;			// Width of image
    int   ysize;			// Height of image

    unsigned long *pix;			// Pixel map

    int    copy( const jpeg& );
    long   pixel( int, int );
    void   pixel( int, int, long );
    int    read_jpeg_grey( jpeg_decompress_struct& );
    int    read_jpeg_rgb( jpeg_decompress_struct& );
    int    resize_x( int, long );
    int    resize_y( int, long );
    int    write_jpeg_grey( char * );
    int    write_jpeg_rgb( char * );
    int    write_tiff_rgb( char * );

  //  Routines to handle SGI RGB files only included on SGI/IRIX
  //  platform

    #ifdef SGI

    int    write_rgb_grey( char * );
    int    write_rgb_rgb( char * );
    int    read_rgb_grey( IMAGE * );
    int    read_rgb_rgb( IMAGE * );

    #endif

  public:
    jpeg();				// Constructor
    jpeg( const jpeg& );		// Constructor
   ~jpeg();				// Destructor

    short  a( int, int );
    void   a( int, int, short );
    short  b( int, int );
    void   b( int, int, short );
    int    capture( int, int, int, int );
    int    create( int, int );
    void   draw();
    short  g( int, int );
    void   g( int, int, short );
    int    height();
    int    quality();
    void   quality( int );
    int    read( char * );
    int    read_tiff( char * );
    short  r( int, int );
    void   r( int, int, short );
    int    resize( int, int, long fill = 0L );
    int    width();
    int    write( char *, int comp = 3 );
    int    write_tiff( char *, int comp = 3 );

    jpeg&  operator=( const jpeg& );

  //  Routines to handle SGI RGB files only included on SGI/IRIX
  //  platform

    #ifdef SGI

    int    read_rgb( char * );
    int    write_rgb( char *, int comp = 3 );

    #endif
};

#endif
