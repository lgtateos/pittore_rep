#ifdef _M_IX86
#include <windows.h>
#else
#include <stream.h>
#endif

#include <string.h>
#include "attr_map.h"
#include "filereader.h"
#include "str.h"
#include "bin_file.h"
#include "global.h"

/*--------------------------------------------------------------------------*/
/*  BIN_FILE.CPP							    */
/*    This class manages a binary input file. This file is made up of "t"   */
/*    frames of binary data, plus a footer that describes the file. See	    */
/*    the routine "read_footer()" for a description of the footer's format  */
/*									    */
/*- Modification History: --------------------------------------------------*/
/*  When:	Who:			Comments:			    */
/*									    */
/*  25-Jun-01	Christopher G. Healey	Initial implementation		    */
/*  29-Sep-01	Christopher G. Healey	Added describe(), extract()	    */
/*  28-Jun-03	Christopher G. Healey	Added allowable(), min_max()	    */
/*  28-Jun-03	Christopher G. Healey	Added close() to open() method, to  */
/*					ensure any existing file is closed  */
/*  11-Jul-03	Christopher G. Healey	Added type()			    */
/*--------------------------------------------------------------------------*/


int bin_file::allowable( float v[] )

  //  This method returns TRUE if all the given attribute values are
  //  allowable, FALSE otherwise
{
  int  i;				// Loop counter


  for( i = 0; i < n; i++ ) {
/*    if( v[ i ] > 4000 && i == 3){
     // cout<< "Value is > 1000:  v["<<i<<"] = "<<v[i]<<endl;
      count_500++;
    }
    if( v[ i ] < 400 && i == 4){
      tount_500++;
    }

    if( v[ i ] > 5000 && i == 3){
      
      count_10++;
    }
    if( v[ i ] < 500 && i == 4){
      tount_10++;
    }
*/
    if ( v[ i ] < lo[ i ] || v[ i ] > hi[ i ] ) {
      return 0;
    }
  }

  return 1;
}					// End method allowable


attr_map bin_file::attr_map_val( int i )

  //  This method returns the map for the given attribute, or NONE on
  //  error
  //
  //  i:  Attribute ID
{
  attr_map  m;				// Temporary map to return on error


  if ( i < 0 || i >= n ) {		// Ensure attribute exists
    return m;				// Return NONE (default attr_map val)
  }

  return map[ i ];
}					// End method attr_map_val


void bin_file::attr_map_val( attr_map m[] )

  //  This method returns map values for all the attributes
  //
  //  m:  Array to hold map values
{
  int  i;				// Loop counter


  for( i = 0; i < n; i++ ) {
    m[ i ] = map[ i ];
  }
}					// End method attr_map_val


int bin_file::attr_map_val( int i, attr_map m )

  //  This method sets the map for the given attribute
  //
  //  i:  Attribute ID
  //  m:  Map value to set
{
  if ( i < 0 || i >= n ) {		// Ensure attribute exists
    return 0;
  }

  map[ i ] = m;
  return 1;
}					// End method attr_map_val


string bin_file::attr_name( int i )

  //  This method returns the name for the given attribute, or an empty
  //  string on error
  //
  //  i:  Attribute ID
{
  if ( i < 0 || i >= n ) {		// Ensure attribute exists
    return "";
  }

  return name[ i ];
}					// End method attr_name


void bin_file::attr_name( string nm[] )

  //  This method returns names for all the attributes
  //
  //  nm:  Array to hold names
{
  int  i;				// Loop counter


  for( i = 0; i < n; i++ ) {
    nm[ i ] = name[ i ];
  }
}					// End method attr_name


int bin_file::attr_name( int i, string nm )

  //  This method sets the name for the given attribute
  //
  //  i:   Attribute ID
  //  nm:  Name to set
{
  if ( i < 0 || i >= n ) {		// Ensure attribute exists
    return 0;
  }

  name[ i ] = nm;
  return 1;
}					// End method attr_name_val


void bin_file::close()

  //  This method closes the input stream and resets the bin file to
  //  empty
{
  v.close();				// Close input vector, flush state
  flush();
}					// End method close


int bin_file::find_map( attr_map::vf_type feat )

  //  This method searchers for an attribute using the given map value
  //
  //  feat:  Visual feature to search for
{
  int  i;				// Loop counter


  for( i = 0; i < n; i++ ) {
    if ( map[ i ].vf_get() == feat ) {
      return i;
    }
  }

  return -1;
}					// End method find_map


long bin_file::comp_footer_size()

  //  This private routine returns the footer size (128 bytes per attr
  //  name + 1 int per attr map + 2 floats per attr range endpoints +
  //  1 int per frame size + 2 ints for n, t
{
  return ( n * 128 ) + ( n * sizeof( int ) ) + ( 2 * n *sizeof( float ) ) +
         ( t * sizeof( int ) ) + ( 2 * sizeof( int ) );
}					// End method comp_footer_size


void bin_file::copy( const bin_file& b )

  //  This private method copies instance variables from an already
  //  existing bin_file object
  //
  //  b:  Object to copy from
{
  int  i;				// Loop counter
  int  max = -1;			// Maximum frame size


  e_n = new int[ b.t ];			// Attempt to allocate arrays
  lo = new float[ b.n ];	
  hi = new float[ b.n ];
  name = new string[ b.n ];
  map = new attr_map[ b.n ];

  if ( !e_n || !lo || !hi || !name || !map ) {
    cerr << "bin_file::copy(), out of memory\n";

    flush();
    return;
  }

  n = b.n;				// Copy n, t
  t = b.t;

  for( i = 0; i < t; i++ ) {		// Copy frame size counts
    e_n[ i ] = b.e_n[ i ];
    max = ( e_n[ i ] > max ) ? e_n[ i ] : max;
  }

  for( i = 0; i < n; i++ ) {		// Copy ranges, attr names and maps
    lo[ i ] = b.lo[ i ];
    hi[ i ] = b.hi[ i ];
    name[ i ] = b.name[ i ];
    map[ i ] = b.map[ i ];
  }

  v_name = b.v_name;			// Attach to input stream

  if ( max < VEC_BUF_MAX ) {
    v.read( v_name, n * sizeof( float ), max, comp_footer_size() );
  } else {
    v.read( v_name, n * sizeof( float ), VEC_BUF_MAX, comp_footer_size() );
  }
}					// End method copy


int bin_file::create( string fname, int num, int fr,
					 string nm[], attr_map m[], float l[], float h[], int fr_n[] )

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
{
  float   *buf;				// Temporary buffer of floats
  float    f;				// Temporary float value
  fstream  fp;				// Output file stream
  int      i, j;			// Loop counters
  int      max = -1;			// Maximum frame size
  char     str[ 128 ];			// Temporary C++ string
  int      vf;				// Current visual feature ID


#ifdef SGI
  fp.open( (char *) fname, ios::out );
#else
  fp.open( (char *) fname, ios::out | ios::binary );
#endif

  if ( !fp.good() ) {
    printf("bin_file::create(), could not open %s for output\n", fname);
    return 0;
  }

  flush();				// Clear any existing data

  e_n = new int[ fr ];			// Initialize instance variables
  lo = new float[ num ];
  hi = new float[ num ];
  map = new attr_map[ num ];
  name = new string[ num ];

  if ( !e_n || !lo || !hi || !map || !name ) {
    cerr << "bin_file::create(), out of memory\n";

    flush();
    return 0;
  }

  n = num;				// Set n, t
  t = fr;

  for( i = 0; i < t; i++ ) {		// Copy frame size counts
    e_n[ i ] = fr_n[ i ];
    max = ( e_n[ i ] > max ) ? e_n[ i ] : max;
  }

  for( i = 0; i < n; i++ ) {		// Copy range, attr names and maps
    lo[ i ] = l[ i ];
    hi[ i ] = h[ i ];
    map[ i ] = m[ i ];
    name[ i ] = nm[ i ];
  }

  //  Attempt to allocate a buffer big enough to hold the largest
  //  frame's floats; if possible, initialize buffer to zero and write
  //  the appropriate number of properly sized frames; if memory is
  //  not available, default to write floats one-by-one

 if ( ( buf = new float[ n * max ] ) == NULL ) {
    f = 0;
    for( i = 0; i < t; i++ ) {
      for( j = 0; j < n * e_n[ i ]; j++ ) {
        fp.write( (char *) &f, sizeof( float ) );
      }
    }

  } else {
    for( i = 0; i < n * max; i++ ) {
      buf[ i ] = 0;
    }

    for( i = 0; i < t; i++ ) {
      fp.write( (char *) buf, n * e_n[ i ] * sizeof( float ) );
    }

    delete [] buf;
  } 

  //  Append appropriate footer to file

  for( i = 0; i < n; i++ ) {		// Write attribute names
    strcpy( str, (char *) nm[ i ] );
    fp.write( str, 128 );
  }

  for( i = 0; i < n; i++ ) {		// Write visual feature map IDs
    vf = m[ i ].vf_get();
    fp.write( (char *) &vf, sizeof( int ) );
  }

  for( i = 0; i < n; i++ ) {		// Write hi range boundary
    fp.write( (char *) &h[ i ], sizeof( float ) );
  }

  for( i = 0; i < n; i++ ) {		// Write lo range boundary
    fp.write( (char *) &l[ i ], sizeof( float ) );
  }

  for( i = 0; i < t; i++ ) {		// Write frame counts
    fp.write( (char *) &e_n[ i ], sizeof( int ) );
  }

  fp.write( (char *) &n, sizeof( int ) );
  fp.write( (char *) &t, sizeof( int ) );

  fp.close();				// Close output file stream
  open( fname );			// Re-attach to newly created file

  return 1;
}					// End method create


int bin_file::describe( string& fname, int& num, int& fr,
					   string*& nm, attr_map*& m, float*& l, float*& h, int*& fr_n )

  //  This method describes various aspects of the current bin file
  //
  //  fname:  Bin file name
  //  num:    Number of attributes per element
  //  fr:     Number of frames
  //  nm:     Attribute names
  //  m:      Attribute map values
  //  l, h:   Low and high allowable range endpoints
  //  fr_n:   Frame sizes
{
  int  i;				// Loop counter


  fname = v_name;			// Copy filename, attrs, frames
  num = n;
  fr = t;

  nm = new string[ n ];			// Allocate arrays
  m = new attr_map[ n ];
  l = new float[ n ];
  h = new float[ n ];
  fr_n = new int[ t ];

  if ( !nm || !m || !l || !h || !fr_n ) {
    cerr << "bin_file::describe(), out of memory creating arrays\n";
    return 0;
  }

  for( i = 0; i < n; i++ ) {		// Copy name, map, lo, hi
    nm[ i ] = name[ i ];
    m[ i ] = map[ i ];
    l[ i ] = lo[ i ];
    h[ i ] = hi[ i ];
  }

  for( i = 0; i < t; i++ ) {		// Copy frame counts
    fr_n[ i ] = e_n[ i ];
  }

  return 1;
}					// End method describe


int bin_file::elem_n( int i )

  //  This method returns the frame size for the given frame, or -1 on
  //  error
  //
  //  i:   Frame ID
{
  if ( i < 0 || i >= t )		// Ensure frame exists
    return -1;

  return e_n[ i ];
}					// End method elem_n


void bin_file::elem_n( int fr_n[] )

  //  This method returns frame sizes for all the frames
  //
  //  fr_n:  Array to hold frame sizes
{
  int  i;				// Loop counter


  for( i = 0; i < t; i++ ) {
    fr_n[ i ] = e_n[ i ];
  }
}					// End method elem_n


int bin_file::equivalent( bin_file *b, int& err )

  //  This method returns TRUE if the two bin files appear equivalent,
  //  otherwise return FALSE and error codes:
  //
  //   0:  External binary file would not open
  //   1:  File size mismatch
  //   2:  Attribute count mismatch
  //   3:  Frame count mismatch
  //   4:  Frame size mismatch
  //   5:  Hi/lo range mismatch
  //   6:  Feature type mismatch
  //   7:  Attribute name mismatch
  //
  //  b:    Pointer to binary file to compare against
  //  err:  Error code on error
{
  filereader  fp[ 2 ];			// Input file streams
  int         i;			// Loop counter


  if ( !fp[ 0 ].open( v_name, 'r' ) || !fp[ 1 ].open( b->v_name, 'r' ) ) {
    cerr << "bin_file::equivalent(), cannot open raw files\n";
    err = 0;
    return 0;
  }

  fp[ 0 ].seek( 0, END );
  fp[ 1 ].seek( 0, END );
  if ( fp[ 0 ].offset() != fp[ 1 ].offset() ) {
    cerr << "bin_file::equivalent(), file size mismatch\n";
    err = 1;
    return 0;
  }
  fp[ 0 ].close();
  fp[ 1 ].close();

  if ( n != b->n ) {
    cerr << "bin_file::equivalent(), attribute count mismatch\n";
    err = 2;
    return 0;
  }

  if ( t != b->t ) {
    cerr << "bin_file::equivalent(), frame count mismatch\n";
    err = 3;
    return 0;
  }

  for( i = 0; i < t; i++ ) {
    if ( e_n[ i ] != b->e_n[ i ] ) {
      cerr << "bin_file::equivalent(), frame " << i << " size mismatch\n";
      err = 4;
      return 0;
    }
  }

  for( i = 0; i < n; i++ ) {
    if ( hi[ i ] != b->hi[ i ] || lo[ i ] != b->lo[ i ] ) {
      cerr << "bin_file::equivalent(), frame " << i << " range mismatch\n";
      err = 5;
      return 0;
    }

    if ( map[ i ] != b->map[ i ] ) {
      cerr << "bin_file::equivalent(), frame " << i << " feature mismatch\n";
      err = 6;
      return 0;
    }

    if ( name[ i ] != b->name[ i ] ) {
      cerr << "bin_file::equivalent(), frame " << i << " name mismatch\n";
      err = 7;
      return 0;
    }
  }

  return 1;
}					// End method equivalent


int bin_file::extract( string fname, int x, int y )

  //  This method extracts frames x-y inclusive to a new bin file
  //
  //  fname:  Name of new bin file
  //  x, y:   Start and end frames to extract
{
  bin_file  bf;				// New binary file to create
  int       i, j;			// Loop counters
  float    *val;			// Array for one element's attr vals


  if ( x > y ) {			// Ensure x <= y
    i = x;
    x = y;
    y = i;
  }

  if ( x < 0 || y >= t ) {
    cerr << "bin_file::extract(), frame endpoints out-of-bounds\n";
    return 0;
  }

  if ( ( val = new float[ n ] ) == NULL ) {
    cerr << "bin_file::extract(), out of memory\n";

    delete [] val;
    return 0;
  }

  if ( !bf.create( fname, n, y - x + 1, name, map, lo, hi, &e_n[ x ] ) ) {
    delete [] val;
    return 0;
  }

  //  We extract using e_n[] rather than elem_n() because we want ALL
  //  DATA AND CONTROL VALUES in each frame (e.g., including row/col
  //  for grid file)

  for( i = x; i <= y; i++ ) {		// For all frames to extract
    for( j = 0; j < e_n[ i ]; j++ ) {

  //  Must force use of bin_file's get(), since other get() methods
  //  will try to protect their control data

      bin_file::get( j, i, val );
      bf.put( j, i - x, val );
    }
  }

  bf.close();

  delete [] val;
  return 1;
}					// End method extract


void bin_file::flush()

  //  This private method frees and re-initializes all instance
  //  variables
{
  delete [] e_n;			// Free allocated memory
  delete [] lo;
  delete [] hi;
  delete [] map;
  delete [] name;

  n = 0;				// Re-initialize instance variables
  t = 0;

  e_n = NULL;
  lo = NULL;
  hi = NULL;
  map = NULL;
  name = NULL;
}					// End method flush


int bin_file::get( int x, int fr, float val[] )

  //  This method reads the attribute values at the given position in
  //  the given frame
  //
  //  x:    Position (within frame) of element to read
  //  fr:   Frame of element to read
  //  val:  Storage for attribute values
{
  int  i;				// Loop counter
  int  pos = 0;				// Raw position of element in file


  if ( fr < 0 || fr >= t || x < 0 || x >= e_n[ fr ] ) {
    cerr << "bin_file::get(), element out of bounds\n";
    return 0;
  }

  //  Compute offset into file, attempt to read requested attribute
  //  values

  for( i = 0; i < fr; i++ ) {
    pos += e_n[ i ];
  }
  pos += x;

  return v.get( pos, (char *) val );
}					// End method get


int bin_file::get( int i, float val[] )

  //  This method reads the attribute values at the given raw position
  //  in the data file
  //
  //  i:    Raw position to read
  //  val:  Storage for attribute values
{
  return v.get( i, (char *) val );
}					// End method get


int bin_file::min_max( float min[], float max[] )

  //  This method computes the minimum and maximum (allowable) value
  //  for each attribute; returns TRUE if at least one min/max found,
  //  FALSE otherwise; do not process row or column counts
  //
  //  min:  Minimum attribute values
  //  max:  Maximum attribute values
{
  int    done;				// Done processing flag
  int    i, j, k;			// Loop counters
  int    pos;				// Position in file
  float *val;				// Current attribute values
  count_500 = 0;
  count_10 = 0;
  tount_500 = 0;
  tount_10 = 0;

  if ( ( val = new float[ n ] ) == NULL ) {
    return 0;
  }

  //  Scan for first allowable entry, use it to initialize min/max

  done = 0;
  for( i = 0; !done && i < t; i++ ) {
    for( j = 0; !done && j < elem_n( i ); j++ ) {
      get( j, i, val );

      if ( allowable( val ) ) {		// Initialize min/max
        done = 1;
        get( j, i, min );
        get( j, i, max );
      }
    }					// End for all elements in frame
  }					// End for all frames

  if ( !done ) {			// No allowable value found?
    delete [] val;
    return 0;
  }

  //  We scan using elem_n() rather than e_n[] because we want DATA VALUES
  //  only in each frame (e.g., we DO NOT want row/col for grid file);
  //  elem_n() for a given object automatically excludes control data

  for( i = 0; i < t; i++ ) {		// Scan entire file for min/max values
    for( j = 0; j < elem_n( i ); j++ ) {
      get( j, i, val );

      if ( allowable( val ) ) {
        for( k = 0; k < n; k++ ) {
          if ( min[ k ] > val[ k ] ) {
            min[ k ] = val[ k ];
          } else if ( max[ k ] < val[ k ] ) {
            max[ k ] = val[ k ];
          }
        }				// End for all attribute values
      }					// End if element is allowable
    }					// End for all elements in frame
  }					// End for all frames

//  cout<<"count_500 = "<<count_500<<" tount_500 ="<< tount_500<<" count_10 = "<< count_10<<" tount_10 = "<< tount_10<<endl;

  delete [] val;
  return 1;
}					// End method min_max


int bin_file::open( string f_name )

  //  This method opens a binary data file for input
  //
  //  f_name:  Name of input stream
{
  int  i;				// Loop counter
  int  max = -1;			// Maximum frame size


  close();				// Close any existing file

  v_name = f_name;			// Copy name of input stream

  if ( !read_footer() ) {		// Attempt to read data footer
    return 0;
  }

  for( i = 0; i < t; i++ ) {		// Compute maximum frame size
    max = ( e_n[ i ] > max ) ? e_n[ i ] : max;
  }

  if ( max < VEC_BUF_MAX ) {
    v.read( v_name, n * sizeof( float ), max, comp_footer_size() );
  } else {
    v.read( v_name, n * sizeof( float ), VEC_BUF_MAX, comp_footer_size() );
  }

  return 1;
}					// End method open


int bin_file::put( int x, int f, float val[] )

  //  This method write the attribute values at the given position in
  //  the given frame
  //
  //  x:    Position (column and row) of element to write
  //  f:    Frame of element to write
  //  val:  Attribute values to write
{
  int  i;				// Loop counter
  int  pos = 0;				// Raw position of element in file


  if ( f < 0 || f >= t || x < 0 || x >= e_n[ f ] ) {
    cerr << "bin_file::put(), element out of bounds\n";
    return 0;
  }

  //  Compute offset into file, attempt to write requested attribute
  //  values

  for( i = 0; i < f; i++ ) {
    pos += e_n[ i ];
  }
  pos += x;

  return v.put( pos, (char *) val );
}					// End method put


int bin_file::put( int i, float val[] )

  //  This method write the attribute values at the given raw position
  //  in the data file
  //
  //  i:    Raw position to write
  //  val:  Attribute values to write
{
  return v.put( i, (char *) val );
}					// End method put


int bin_file::range( int i, float r[] )

  //  This method returns the lo-hi endpoints for the given attribute
  //
  //  i:  Attribute ID
  //  r:  Array to hold endpoints
{
  if ( i < 0 || i >= n )		// Ensure attribute exists
    return 0;

  r[ 0 ] = lo[ i ];
  r[ 1 ] = hi[ i ];
  return 1;
}					// End method range


void bin_file::range( float r[][ 2 ] )

  //  This method returns the lo-hi endpoints for all the attributes
  //
  //  r:  Array to hold endpoints
{
  int  i;				// Loop counter


  for( i = 0; i < n; i++ ) {
    r[ i ][ 0 ] = lo[ i ];
    r[ i ][ 1 ] = hi[ i ];
  }
}					// End method range


int bin_file::range_hi( int i, float& r )

  //  This method returns the hi endpoint for the given attribute
  //
  //  i:  Attribute ID
  //  r:  Array to hold endpoints
{
  if ( i < 0 || i >= n )		// Ensure attribute exists
    return 0;

  r = hi[ i ];
  return 1;
}					// End method range_hi


void bin_file::range_hi( float r[] )

  //  This method returns the hi endpoint for all the attributes
  //
  //  r:  Array to hold endpoints
{
  int  i;				// Loop counter


  for( i = 0; i < n; i++ ) {
    r[ i ] = hi[ i ];
  }
}					// End method range_hi


int bin_file::range_lo( int i, float& r )

  //  This method returns the lo endpoint for the given attribute
  //
  //  i:  Attribute ID
  //  r:  Array to hold endpoints
{
  if ( i < 0 || i >= n )		// Ensure attribute exists
    return 0;

  r = lo[ i ];
  return 1;
}					// End method range_lo


void bin_file::range_lo( float r[] )

  //  This method returns the lo endpoint for all the attributes
  //
  //  r:  Array to hold endpoints
{
  int  i;				// Loop counter


  for( i = 0; i < n; i++ ) {
    r[ i ] = lo[ i ];
  }
}					// End method range_lo


int bin_file::read_footer()

  //  This private method interprets an input file's footer, which is
  //  of the form:
  //
  //    attr_name_1 ... attr_name_n   (each 128-byte char fields)
  //    map_1 ... map_n               (each 4-byte int fields)
  //    hi_1 ... hi_n                 (each 4-byte float fields)
  //    lo_1 ... lo_n                 (each 4-byte float fields)
  //    frame_1 ... frame_t           (each 4-byte int fields)
  //    n                             (one 4-byte int, num attrs)
  //    t                             (one 4-byte int, num time steps)
{
  char      attr_name[ 128 ];		// Attribute name
  ifstream  fp;				// Input file stream
  int       i;				// Loop counter
  int       m;				// Temporary feature map ID
  long      seek_pos;			// Seek position in file
  int       sum = 0;			// Sum of frame size counts
  int       cpos;


#ifdef SGI
  fp.open( (char *) v_name, ios::in );
#else
  fp.open( (char *) v_name, ios::in | ios::binary );
#endif

  if ( !fp.good() ) {
    printf("bin_file::read_footer(), cannot open %s \n", v_name);
    return 0;
  }

  //  Read timesteps and number of attributes

  fp.seekg( ( -2L * (long) sizeof( int ) ) , ios::end );
  fp.read( (char *) &n, sizeof( int ) );
  fp.read( (char *) &t, sizeof( int ) );

  e_n = new int[ t ];			// Allocate e_n, lo, hi, name arrays
  lo = new float[ n ];
  hi = new float[ n ];
  map = new attr_map[ n ];
  name = new string[ n ];

  seek_pos = -comp_footer_size();
  fp.seekg( seek_pos, ios::end );

  for( i = 0; i < n; i++ ) {		// Read attribute names
    fp.read( attr_name, 128 );
    name[ i ] = attr_name;
  }

  for( i = 0; i < n; i++ ) {		// Read visual feature map IDs
    fp.read( (char *) &m, sizeof( int ) );
    map[ i ].vf_set( m );
  }

  for( i = 0; i < n; i++ ) {		// Read hi range boundary
    fp.read( (char *) &hi[ i ], sizeof( float ) );
  }

  for( i = 0; i < n; i++ ) {		// Read lo range boundary
    fp.read( (char *) &lo[ i ], sizeof( float ) );
  }

  for( i = 0; i < t; i++ ) {		// Read frame counts
    fp.read( (char *) &e_n[ i ], sizeof( int ) );
    sum += e_n[ i ];
  }

  //  Ensure data portion of file matches frame counts

  fp.seekg( 0L, ios::end );
  cpos = fp.tellg();
  if ( cpos - comp_footer_size() != sum * n * (long) sizeof( float ) ) {
    cerr << "bin_file::read_footer(), file size mismatch, ";
    cerr << sum << " ELEM x " << n << " ATTR != ";
    cerr << ( cpos - comp_footer_size() ) / sizeof( float ) << "\n";
   
    return 0;
  }

  fp.close();
  return 1;
}					// End method read_footer


int bin_file::size()

  //  This method returns the number of elements contained in the
  //  file
{
  return v.size();
}					// End method size


int bin_file::update_footer()

  //  This method updates the values in the footer of the bin file,
  //  based on the instance variables that hold those values
{
  fstream  fp;				// Input file stream
  int      i;				// Loop counter
  long     seek_pos;			// Seek position in file
  char     str[ 128 ];			// Temporary C++ string
  int      vf;				// Current visual feature ID


#ifdef SGI
  fp.open( (char *) v_name, ios::in | ios::out );
#else
  fp.open( (char *) v_name, ios::in | ios::out | ios::binary );
#endif

  if ( !fp.good() ) {
    printf("bin_file::update_footer(), cannot open %s \n", v_name);
    return 0;
  }

  seek_pos = -comp_footer_size();
  fp.seekg( seek_pos, ios::end );

  for( i = 0; i < n; i++ ) {		// Write attribute names
    strcpy( str, (char *) name[ i ] );
    fp.write( str, 128 );
  }

  for( i = 0; i < n; i++ ) {		// Write visual feature map IDs
    vf = map[ i ].vf_get();
    fp.write( (char *) &vf, sizeof( int ) );
  }

  for( i = 0; i < n; i++ ) {		// Write hi range boundary
    fp.write( (char *) &hi[ i ], sizeof( float ) );
  }

  for( i = 0; i < n; i++ ) {		// Write lo range boundary
    fp.write( (char *) &lo[ i ], sizeof( float ) );
  }

  for( i = 0; i < t; i++ ) {		// Write frame counts
    fp.write( (char *) &e_n[ i ], sizeof( int ) );
  }

  fp.write( (char *) &n, sizeof( int ) );
  fp.write( (char *) &t, sizeof( int ) );

  return 1;
}					// End method update_footer


bin_file& bin_file::operator=( const bin_file& b )

  //  Assignment operator
  //
  //  b:  Object to assign from
{
  flush();
  copy( b );

  return *this;
}					// End assignment operator


bin_file::bin_file()

  //  Default constructor
{
  n = 0;				// Initialize instance variables
  t = 0;

  e_n = NULL;
  hi = NULL;
  lo = NULL;
  map = NULL;
  name = NULL;
}					// End default constructor


bin_file::bin_file( const bin_file& b )

  //  Copy constructor
  //
  //  b:  Object to copy from
{
  copy( b );				// Copy instance variables
}					// End copy constructor


bin_file::~bin_file()

  //  Destructor
{
  flush();
}					// End destructor
