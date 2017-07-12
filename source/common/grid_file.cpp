#ifdef _M_IX86
#include <windows.h>
#include "values.h"
#else
#include <stream.h>
#include <values.h>
#endif

#include "attr_map.h"
#include "bin_file.h"
#include "grid_file.h"
#include "global.h"

/*--------------------------------------------------------------------------*/
/*  GRID_FILE.CPP							    */
/*    This is a subclass of BIN_FILE (binary file); it is specifically	    */
/*    designed to handle data arrayed on an underlying regular grid. For    */
/*    each frame, the last two elements of the frame encode the number of   */
/*    rows and columns that make up the grid for that frame.		    */
/*									    */
/*- Modification History: --------------------------------------------------*/
/*  When:	Who:			Comments:			    */
/*									    */
/*  27-Mar-02	Christopher G. Healey	Initial implementation		    */
/*  06-Apr-03	Christopher G. Healey	Added cols(), rows() methods to get */
/*					all column/row sizes at once	    */
/*  28-Jun-03	Christopher G. Healey	Added min_max()			    */
/*  10-Jul-03	Christopher G. Healey	Added grid_2_bin(), is_gridfile(),  */
/*					type()				    */
/*  12-Jul-03	Christopher G. Healey	Removed extract() and min_max(),    */
/*					made minor mods in bin_file.cpp so  */
/*					inherited method works here	    */
/*--------------------------------------------------------------------------*/


int grid_file::cols( int fr)

  //  This method returns the number of columns for the given frame,
  //  or -1 if no such frame exists
  //
  //  fr:  Frame to query
{
  int    col;				// Column count
  int    i;				// Loop counter
  long   pos = 0L;			// Seek position into file
  float *val;				// Array of column values


  if (fr< 0 ||fr>= t ) {
    return -1;
  }

  if ( ( val = new float[ n ] ) == NULL ) {
    cerr << "grid_file::cols(), out of memory\n";
    return -1;
  }

  for( i = 0; i <= fr; i++ ) {		// Compute end-of-frame position
    pos += e_n[ i ];
  }

  v.get( pos - 1L, (char *) val );	// Grab column count
  col = int( val[ 0 ] );

  delete [] val;
  return col;
}					// End cols


void grid_file::cols( int col[] )

  //  This method returns the number of columns in the data portion for
  //  all the frames
  //
  //  col:  Array to hold column sizes
{
  int  i;				// Loop counter


  for( i = 0; i < t; i++ ) {
		col[ i ] = cols( i );
  }
}					// End method cols


int grid_file::create( string fname, int num, int fr, string nm[],
  attr_map m[], float l[], float h[], int fr_n[], int row[], int col[] )

  //  This method creates a new, empty (all attribute values defaulted to
  //  zero) bin file with the appropriate footer
  //
  //  fname:  Bin file name
  //  num:    Number of attributes per element
  //  fr:     Number of frames
  //  nm:     Attribute names
  //  m:      Attribute map values
  //  l, h:   Low and high allowable range endpoints
  //  fr_n:   Frame sizes
  //  row:    Number of rows for each frame
  //  col:    Number of columns for each frame
{
  float *col_elem = NULL;		// Array of column counts
  int   *fr_count = NULL;		// Frame size PLUS row,col counts
  int    i, j;				// Loop counters
  long   pos = 0L;			// End-of-frame position
  float *row_elem = NULL;		// Array of row counts


  if ( ( col_elem = new float[ num ] ) == NULL ||
       ( row_elem = new float[ num ] ) == NULL ||
       ( fr_count = new int[fr] ) == NULL ) {
    cerr << "grid_file::create(), out of memory\n";

    delete [] col_elem;
    delete [] row_elem;
    delete [] fr_count;
    return 0;
  }

  for( i = 0; i < fr; i++ ) {		// Make space in each frame for..
    fr_count[ i ] = fr_n[ i ] + 2;	// ..row and column count
  }

  if ( !bin_file::create( fname, num, fr, nm, m, l, h, fr_count ) ) {
    delete [] col_elem;
    delete [] row_elem;
    delete [] fr_count;
    return 0;
  }

  //  For each frame, write the proper row/column information to the
  //  end of the frame

  for( i = 0; i < fr; i++ ) {		// For all frames in grid file
    for( j = 0; j < num; j++ ) {	// Set row/column counts
      col_elem[ j ] = col[ i ];
      row_elem[ j ] = row[ i ];
    }

    pos += e_n[ i ];			// Compute end-of-frame position
    v.put( pos - 2L, (char *) row_elem );
    v.put( pos - 1L, (char *) col_elem );
  }					// End for all frames

  delete [] col_elem;
  delete [] row_elem;
  delete [] fr_count;
  return 1;
}					// End method create


int grid_file::describe( string& fname, int& num, int& fr, string*& nm,
  attr_map*& m, float*& l, float*& h, int*& fr_n, int*& row, int*& col )

  //  This method describes various aspects of the current grid file
  //
  //  fname:  Bin file name
  //  num:    Number of attributes per element
  //  fr:     Number of frames
  //  nm:     Attribute names
  //  m:      Attribute map values
  //  l, h:   Low and high allowable range endpoints
  //  fr_n:   Frame sizes
  //  row:    Number of rows for each frame
  //  col:    Number of columns for each frame
{
  int  i;				// Loop counter


  if ( !bin_file::describe( fname, num, fr, nm, m, l, h, fr_n ) ) {
    return 0;
  }

  row = new int[ t ];
  col = new int[ t ];

  if ( !row || !col ) {
    cerr << "grid_file::describe(), out of memory creating arrays\n";
    return 0;
  }

  for( i = 0; i < t; i++ ) {
    row[ i ] = rows( i );		// Grab row, column counts
    col[ i ] = cols( i );

    fr_n[ i ] -= 2;			// Remove row, column from frame size
  }

  return 1;
}					// End method describe


int grid_file::elem_n( int i )

  //  This method returns the size of the data portion of the given
  //  frame, or -1 on error
  //
  //  i:  Frame ID
{
  int  size;				// Size of frame


  if ( ( size = bin_file::elem_n( i ) ) == -1 ) {
    return -1;
  } else {
    return size - 2;
  }
}					// End method elem_n


void grid_file::elem_n( int fr_n[] )

  //  This method returns the size of the data portion for all the
  //  frames
  //
  //  fr_n:  Array to hold frame sizes
{
  int  i;				// Loop counter


  bin_file::elem_n( fr_n );

  for( i = 0; i < t; i++ ) {		// Remove row/column count
    if ( fr_n[ i ] != -1 ) {
      fr_n[ i ] -= 2;
    }
  }
}					// End method elem_n


int grid_file::equivalent( bin_file *b, int& err )

  //  This method returns TRUE if the two grid files appear
  //  equivalent, otherwise return FALSE and error codes (in addition
  //  to codes set in bin_file):
  //
  //   1000:  Grid/bin file mismatch
  //   1001:  Row/column count mismatch
  //
  //  g:    Pointer to grid file to compare against
  //  err:  Error code on error
{
  int  i;				// Loop counter


  if ( b->type() != "GRID" ) {		// Not a grid file?
    err = 1000;
    return 0;
  }

  if ( !bin_file::equivalent( b, err ) ) {
    return 0;
  }

  for( i = 0; i < t; i++ ) {		// Check row/column counts
    if ( rows( i ) != ((grid_file *) b)->rows( i ) ||
         cols( i ) != ((grid_file *) b)->cols( i ) ) {
      cerr << "grid_file::equivalent(), ";
      cerr << "frame " << i << " row/column mismatch\n";
      err = 1001;
      return 0;
    }
  }

  return 1;
}					// End method equivalent


int grid_file::get( int x, int fr, float val[] )

  //  This method reads the attribute values at the given position in
  //  the given frame; do not allow row or column to be accessed
  //
  //  x:    Position (within frame) of element to read
  //  fr:   Frame of element to read
  //  val:  Storage for attribute values
{
  if (fr< 0 ||fr>= t || x < 0 || x >= e_n[fr] - 2 ) {
    cerr << "grid_file::get(), element out of bounds\n";
    return 0;
  }

  return bin_file::get( x, fr, val );
}					// End method get


int grid_file::grid_2_bin( string fname )

  //  This method converts a grid file to a bin file; the implied
  //  spatial layout of elements in the grid file is explicitly added
  //  to the elements in the bin file
{
  bin_file  b;				// New bin file object
  int      *col = NULL;			// Column size array
  int      *fr_n = NULL;		// Number of elements per frame
  float    *h = NULL;			// Attribute high range
  int       i, j, k;			// Loop counters
  float    *l = NULL;			// Attribute low range
  attr_map *m = NULL;			// Attribute visual features
  string   *nm = NULL;			// Attribute names
  int      *row = NULL;			// Row size array
  float    *val = NULL;			// Array for one element's attr vals


  l = new float[ n + 2 ];		// Create working arrays
  h = new float[ n + 2 ];
  m = new attr_map[ n + 2 ];
  nm = new string[ n + 2 ];
  fr_n = new int[ n + 2 ];
  val = new float[ n + 2 ];
  col = new int[ t ];
  row = new int[ t ];

  if ( !nm || !l || !h || !m || !fr_n || !val || !col || !row ) {
    cerr << "grid_file::grid_2_bin(), out of memory allocating arrays\n";

    delete [] l;
    delete [] h;
    delete [] m;
    delete [] nm;
    delete [] fr_n;
    delete [] val;
    delete [] col;
    delete [] row;
    return 0;
  }

  cols( col );				// Get row, column counts
  rows( row );

  nm[ 0 ] = "X";			// Initialize spatial attributes
  nm[ 1 ] = "Y";
  l[ 0 ] = -MAXINT;
  l[ 1 ] = -MAXINT;
  h[ 0 ] = MAXINT;
  h[ 1 ] = MAXINT;
  m[ 0 ].vf_set( attr_map::NONE );
  m[ 1 ].vf_set( attr_map::NONE );

  for( i = 0; i < n; i++ ) {		// Copy existing attribute descriptions
    l[ i + 2 ] = lo[ i ];
    h[ i + 2 ] = hi[ i ];
    m[ i + 2 ] = map[ i ];
    nm[ i + 2 ] = name[ i ];
  }

  for( i = 0; i < t; i++ ) {		// Initialize frame sizes
    fr_n[ i ] = e_n[ i ] - 2;		// Remove 2 entries for row/column
  }

  if ( !b.create( fname, n + 2, t, nm, m, l, h, fr_n ) ) {
    cerr << "grid_file::grid_2_bin(), ";
    printf("could not create bin file %s\n", fname);
    return 0;
  }

  for( i = 0; i < t; i++ ) {		// For all frames
    for( j = 0; j < row[ i ]; j++ ) {	// For all rows and cols in frame
      for( k = 0; k < col[ i ]; k++ ) {
        get( ( j * col[ i ] ) + k, i, &val[ 2 ] );

        val[ 0 ] = k;			// Save spatial position of element
        val[ 1 ] = j;

        b.put( ( j * col[ i ] ) + k, i, val );
      }
    }
  }

  delete [] l;				// Free working arrays
  delete [] h;
  delete [] m;
  delete [] nm;
  delete [] val;
  delete [] col;
  delete [] row;

  b.close();
  return 1;
}					// End method grid_2_bin


int grid_file::is_gridfile()

  //  This method returns TRUE if the given file satisfies the grid
  //  file requirements (i.e., each frame has a row/col at the end,
  //  and the number of elements in each frame matches row * col),
  //  FALSE otherwise
{
  int    c;				// Number of columns in current frame
  int    i, j;				// Loop counters
  int    pos;				// Position in file
  int    r;				// Number of rows in current frame
  float *val;				// Current attribute values


  if ( ( val = new float[ n ] ) == NULL ) {
    cerr << "grid_file::is_gridfile(), out of memory\n";
    return 0;
  }

  pos = 0;
  for( i = 0; i < t; i++ ) {		// For all frames
    pos += e_n[ i ];			// Update end-of-frame position

    v.get( pos - 2L, (char *) val );	// Grab row count
    r = int( val[ 0 ] );

    for( j = 1; j < n; j++ ) {		// Check entire vector has row count
      if ( r != int( val[ j ] ) ) {
        return 0;
      }
    }

    v.get( pos - 1L, (char *) val );	// Grab column count
    c = int( val[ 0 ] );

    for( j = 1; j < n; j++ ) {		// Check entire vector has col count
      if ( c != int( val[ j ] ) ) {
        return 0;
      }
    }

    if ( r * c != elem_n( i ) ) {	// Ensure frame size is correct
      return 0;
    }
  }					// End for all frames

  delete [] val;
  return 1;
}					// End method is_gridfile


int grid_file::put( int x, int fr, float val[] )

  //  This method writes the attribute values at the given position in
  //  the given frame; do not allow row or column to be accessed
  //
  //  x:    Position (within frame) of element to read
  //  fr:   Frame of element to read
  //  val:  Storage for attribute values
{
  if (fr < 0 ||fr >= t || x < 0 || x >= e_n[fr ] - 2 ) {
    cerr << "grid_file::put(), element out of bounds\n";
    return 0;
  }

  return bin_file::put( x, fr, val );
}					// End method put


int grid_file::rows( int fr)

  //  This method returns the number of rows for the given frame, or
  //  -1 if no such frame exists
  //
  //  fr:  Frame to query
{
  int    i;				// Loop counter
  long   pos = 0L;			// Seek position into file
  int    row;				// Row count
  float *val;				// Array of column values


  if ( fr < 0 || fr >= t ) {
    return -1;
  }

  if ( ( val = new float[ n ] ) == NULL ) {
    cerr << "grid_file::rows(), out of memory\n";
    return -1;
  }

  for( i = 0; i <= fr; i++ ) {		// Compute end-of-frame position
    pos += e_n[ i ];
  }

  v.get( pos - 2L, (char *) val );	// Grab row count
  row = int( val[ 0 ] );

  delete [] val;
  return row;
}					// End rows


void grid_file::rows( int row[] )

  //  This method returns the number of rows in the data portion for
  //  all the frames
  //
  //  row:  Array to hold row sizes
{
  int  i;				// Loop counter


  for( i = 0; i < t; i++ ) {
    row[ i ] = rows( i );
  }
}					// End method rows
