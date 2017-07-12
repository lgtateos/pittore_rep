#include "iostream"//#include <iostream.h>
#include <stdlib.h>
#include <math.h>
#include "colour.h"
#include "global.h"
#include "random.h"
#include "segment.h"
#include "paint.h"
#ifdef NAMESPACE
  #include <fstream>
#else 
  #include "fstream.h"
#endif
/*--------------------------------------------------------------------------*/
/*  PAINT.CPP								    */
/*    Routines to use strokes to paint segments in an underlying dataset    */
/*					   				    */
/*- Modification History: --------------------------------------------------*/
/*  When:	Who:			Comments:			    */
/*								  	    */
/*  18-Jun-01	Christopher G. Healey	Initial implementation		    */
/*  13-Jan-02   Laura G. Tateosian	Relocate stroke shuffle to init_avail*/
/*                                  providing speed-up		            */  
/*--------------------------------------------------------------------------*/

//  Module global prototypes

extern int   get_current_frame();
extern int   get_current_position( int, int );
extern int   is_valid( int, int );
extern int   map_mat( int, int , stroke_s& );  //function in main.cpp
extern int   map_stroke( float, float , stroke_s& );  //function in main.cpp

static int   add_stroke( int *, stroke_s& );
static int   bbox( int **, int, int, int, int *, int * );
static int   cmp_2D_vert( const void *, const void * );
static void  comp_line( point2D, point2D, float&, float& );
static int   expand_pt( point2D *&, int& );
//static int   expand_st( stroke_s *&, int& );
static int   find_pos( int *, stroke_s&, int& );
static int   init_avail();
static int   init_tile( int **, int, int *, int * );
static void   print_tile();
static int   scan_cvt( point2D *, point2D *&, int, int&, int, int );
void         set_array(int tex, Point array[4]);
static void  transform( float, float, float, float *s, point2D *v );
static void  transform_curves( int, float, float, float, float *s, point2D v[] );
void shuffle();

//  Module global variables

point2D  *Avail;			// Available pixels (used in find_pos)
bool bezier_on = false;
int       cur_st_n;			// Number of strokes in this segment
int       num_avail;			// Size of Avail array
int       stroke_type = 2;
float     total_rgb[3];			// Sum of the rgb values this seg
float 	  total_length;			// Sum of stroke lengths for this seg
float 	  total_width;			// Sum of stroke widths for this seg
int     **Tile;				// Pixel status tile
int      Tile_h, Tile_w;		// Height and width of tile array


static int add_stroke( int ll[], stroke_s& st )

  //  This private routine randomly places a single stroke within the
  //  tile, computing the resulting overlap with other strokes, and
  //  the number of stroke pixels that fall outside the segment; the
  //  routine returns the total pixels in the (scan converted) stroke
  //
  //  ll:  Lower-left corner of tile within entire id array
  //  st:  Stroke information structure
{
	int      avail_top_index; // Number of available positions that have not been tried yet
  int      cur_pos;
	int      clip = 0;			// Number of clipped pixels
  int      clip_cut;			// Clip cutoff, in pixels
  int      fail = 0;			// Failure to place stroke flag
  int      i, j;					// Loop counter
  int      num_fail = 0;		// Fail to place stroke counter
  int      overlap = 0;			// Number of overlapping pixels
  point2D *pt;					// Pixels in stroke
  int      pt_max = 64;			// Maximum size of pixel array
  int      pt_n;				// Num pixels in array
  int      success = 0;			// Success to place stroke flag
	point2D  tmp_pt;
  point2D  v[ 4 ];				// Vertices of current stroke
  int      x, y;				// Center of current stroke
  float smaLL = 0;//2*.75 + 1; 
  stroke_s tmp_st;

  avail_top_index = num_avail - 1;
  
  if ( !find_pos( ll, st, cur_pos ) ) {  
    return 0;  // checked all available positions and none found
  }

  pt = new point2D[ pt_max ];
  clip_cut = int( st.s[ 0 ] * st.s[ 1 ] * 0.2 );  // 20% of the area

  do {
    clip = 0;				// Compute clip, overlap counts
    overlap = 0;  
    if ( bezier_on  ){
       transform_curves( st.tex, st.p.x, st.p.y, st.rot, st.s, v);
    }else{  
       transform( st.p.x, st.p.y, st.rot, st.s, v );
    }
    
    pt_n = scan_cvt( v, pt, 0, pt_max, Tile_w, Tile_h );

    for( i = 0; i < pt_n; i++ ) {
      x = int( pt[ i ].x );
      y = int( pt[ i ].y );

      //  If pixel outside tile, or in tile but not in segment, clip;
      //  if pixel is already covered by a stroke, overlap

      if ( x < 0 || x >= Tile_w ||
           y < 0 || y >= Tile_h || Tile[ y ][ x ] == 0 ) {
        clip++;  // beetle
      } else if ( Tile[ y ][ x ] == 2  ) {
        overlap++;
      }
    }

  //  If clipped portion of stroke is too large, try making it smaller;
  //  if that doesn't work, try repositioning the stroke; if that
  //  doesn't work, report failure to place a stroke to the caller

    //if ( clip > clip_cut || overlap > (st.s[ 0 ] * st.s[ 1 ] * 0.8) ) {
    if ( clip > clip_cut){   
     if ( st.s[ 1 ] > st.s[ 0 ] && st.s[1] - 1 > 0) {	// Room to try shrinking stroke?
          st.s[ 1 ]--;
      }else if ( st.s[ 1 ] < st.s[ 0 ] && st.s[0] - 1 > 0) {  //beetle
          st.s[ 0 ]--;
      }else if ( st.s[0] - 1 > 0 && st.s[1] - 1 > 0 ){
          st.s[0] --;
          st.s[1]--;
      }else{				// Cannot shrink stroke
        if ( ++num_fail < 100 ) {	// Try a new stroke position?
          st.s[ 1 ] = st.s[ 0 ] * 4;
					// To avoid retrying the same available position, swap it with the end of the array and then reduce
					// the index of the last position 
					for ( j = 0; j < cur_pos + 1 && j < avail_top_index; j++ ){
					   tmp_pt = Avail[j];
						 Avail[j] = Avail[avail_top_index];
						 Avail[avail_top_index] =  Avail[j];
						 avail_top_index--;
					}
          if ( !find_pos( ll, st, cur_pos ) ) {
            delete [] pt;
            return 0;
          }
				} else {			// Complete failure to place a stroke
          fail = 1;
				}
			}  // if position found
    } else if ( (pt_n - clip - overlap) <= 0 ){  // clip amount allowable, but remaining stroke only overlapping existing one
			 if ( ++num_fail < 100 ) {	// Try a new stroke position?
			   st.s[ 1 ] = st.s[ 0 ] * 4;
					// To avoid retrying the same available position, swap it with the end of the array and then reduce
					// the index of the last position 
					for ( j = 0; j < cur_pos + 1 && j < avail_top_index; j++ ){
					   tmp_pt = Avail[j];
						 Avail[j] = Avail[avail_top_index];
						 Avail[avail_top_index] =  Avail[j];
						 avail_top_index--;
					}
          if ( !find_pos( ll, st, cur_pos ) ) {
            delete [] pt;
            return 0;
          }
			 }else {
				 fail = 1;
			 }
		}else { // Clip amount is allowable
			success = 1;			// Signal successful stroke placement
    }
	}
  while( !success && !fail );

  if ( fail ) {				// Failure to place any stroke
    delete [] pt;
    return 0;
  }

  for( i = 0; i < pt_n; i++ ) {		// Commit stroke
    x = int( pt[ i ].x );
    y = int( pt[ i ].y );
     
    if ( x >=0 && x < Tile_w && y >= 0 && y < Tile_h && Tile[ y ][ x ] == 1 ) {
      Tile[ y ][ x ] = 2;
    }
  }

  tmp_pt = Avail[cur_pos];
	Avail[ cur_pos ] = Avail[ num_avail - 1];
	Avail[ num_avail - 1 ] =  Avail[ cur_pos ];
	num_avail--;

  delete [] pt;
  return ( pt_n - clip - overlap );
}					// End routine add_stroke


static int bbox( int **id, int w, int h, int seg_id, int ll[], int ur[] )
  //  This routine computes the bounding box of the given segment
  //
  //  id:      Expanded segment ID array
  //  w, h:    Width, height of ID array
  //  seg_id:  ID of segment to stroke
  //  ll:      Lower-left corner of bounding box
  //  ur:      Upper-right corner of bounding box
{
  int  i, j;				// Loop counters


  ll[ 0 ] = w + 1;			// Initialize corners
  ll[ 1 ] = h + 1;
  ur[ 0 ] = -1;
  ur[ 1 ] = -1;

  for( i = 0; i < h; i++ ) {		// For all entries in ID array
    for( j = 0; j < w; j++ ) {
      if ( id[ i ][ j ] == seg_id ) {
        if ( ll[ 0 ] > j )
          ll[ 0 ] = j;
        if ( ll[ 1 ] > i )
          ll[ 1 ] = i;

        if ( ur[ 0 ] < j )
          ur[ 0 ] = j;
        if ( ur[ 1 ] < i )
          ur[ 1 ] = i;
      }
    }
  }

  return( ur[ 0 ] != -1 );		// Return corners found flag
}					// End routine bbox


static int cmp_2D_vert( const void *a, const void *b )

  //  Quicksort compare routine for 2D vertices
  //
  //  a:  Pointer to first 2D vertex float array
  //  b:  Pointer to second 2D vertex float array
{
  float *q, *r;				// Two arrays to compare


  q = (float *) a;
  r = (float *) b;

  if ( q[ 1 ] < r[ 1 ] )		// Check Y-coords first
    return -1;
  else if ( q[ 1 ] > r[ 1 ] )
    return 1;
  else {				// If Y-coords equal, check X-coords
    if ( q[ 0 ] < r[ 0 ] )
      return -1;
    else if ( q[ 0 ] > r[ 0 ] )
      return 1;
    else
      return 0;
  }
}					// End routine cmp_2D_vert


static void comp_line( point2D s, point2D t, float& m_inv, float& b )

  //  This private routine computes the inverse slope and intercept of
  //  the line formed by the given points
  //
  //  s, t:   Points on line
  //  m_inv:  Inverse of slope
  //  b:      Intercept
{
  if ( s.y == t.y ) {
    m_inv = 0;
  } else {
    m_inv = ( t.x - s.x ) / ( t.y - s.y );
  }

  if ( m_inv == 0 ) {
    b = s.y;
  } else {
    b = s.y - ( s.x / m_inv );
  }
}					// End routine comp_line


static int expand_pt( point2D* &pt, int& pt_max )

  //  This routine doubles the size of the given 2D point list
  //
  //  pt:      Point list to double
  //  pt_max:  New maximum list size
{
  int      i;				// Loop counter
  point2D *tmp_pt;			// Temporary point list


  tmp_pt = new point2D[ pt_max * 2 ];
  if ( !tmp_pt ) {
    cerr << "expand_pt(), out of memory creating new point list\n";
    return 0;
  }

  for( i = 0; i < pt_max; i++ ) {	// Copy list
    tmp_pt[ i ] = pt[ i ];
  }

  delete [] pt;				// Reference new list
  pt = tmp_pt;

  pt_max *= 2;				// Double list size
  return 1;
}					// End routine expand_pt


int expand_st( stroke_s* &st, int& st_max )

  //  This routine doubles the size of the given stroke list
  //
  //  st:      Stroke list to double
  //  st_max:  New maximum list size
{
  int       i;				// Loop counter
  stroke_s *tmp_st;			// Temporary stroke list


  tmp_st = new stroke_s[ st_max * 2 ];
  if ( !tmp_st ) {
    cerr << "expand_st(), out of memory creating new stroke list\n";
    return 0;
  }

  for( i = 0; i < st_max; i++ ) {	// Copy list
    tmp_st[ i ] = st[ i ];
  }

  delete [] st;				// Reference new list
  st = tmp_st;

  st_max *= 2;				// Double list size
  return 1;
}					// End routine expand_st

void find_avg_size( float* avg)
// This routine returns the average size of a stroke in the segment just stroked 
{
  //  avg[0]:  average width
  //  avg[1];  average length
  if (cur_st_n == 0)
  {
    cur_st_n = 1;
  }
  avg[ 0 ] = total_width/cur_st_n;
  avg[ 1 ] = total_length/cur_st_n;

}
static int find_pos( int ll[], stroke_s &st, int& chosen_one )

  //  This routine randomly chooses one of the remaining available
  //  segment pixels
  //
  //  ll:  Lower-left corner of tile within entire id array
  //  st:  Stroke to position
{
  int      i;				// Loop counter
  int      x, y;			// Temporary avail list indices
 
	if ( stroke_type == 0 ){
    for( i = 0; i < num_avail; i++ ) {	// Search for a valid pixel
      x = int( Avail[ i ].x ) + ll[ 0 ];
      y = int( Avail[ i ].y ) + ll[ 1 ];
      if ( Tile[ y -ll[1] ][ x -ll[0]] == 1){
		if( map_mat( x, y, st )){ 		//set color, rotation of stroke; fails if
         //	cout<<";";                                           						//a corner in surrounding square < -990
          st.p = Avail[ i ];   			//set position of stroke
				//	cout<<"st.p = "<<st.p.x<<" "<<st.p.y<<endl;
				//	cout<<"1111111 st.p = "<<x<<" "<<y<<"  " <<st.rot<<endl; 
					chosen_one = i;
          return 1;
        }
      }
	  }
	}else{ 
    for( i = 0; i < num_avail; i++ ) {	// Search for a valid pixel
      x = int( Avail[ i ].x ) + ll[ 0 ];
      y = int( Avail[ i ].y ) + ll[ 1 ];
      if ( Tile[ y -ll[1] ][ x -ll[0]] == 1){
				if( map_stroke( x, y, st )){ 		//set color, rotation of stroke; fails if
                                        //a corner in surrounding square < -990
          st.p = Avail[ i ];   			    //set position of stroke
					
          chosen_one = i;
          return 1;
        }
      }
	  }
	}
  return 0;
}					// End routine find_pos

static void print_tile(){
	int i; int j;					//Loop counters
  int two_count = 0; int one_count = 0;

  for( i = 0; i < Tile_h; i++ ) {	// Scan tile for available pixels
    for( j = 0; j < Tile_w; j++ ) {
      if ( Tile[ i ][ j ] == 2 ) {
        two_count++;
			}else if ( Tile[ i ][ j ] == 1 ){
			  one_count++;
			}
    }
	}
	cout<<two_count<<" positions covered.   "<< one_count <<" available."<<endl;
}


static int init_avail() 

  //  This routine initializes an array of available pixels, and shuffles
  //  them into a random order
{ 
  int i; int j;					//Loop counters
     
  num_avail = 0;

  for( i = 0; i < Tile_h; i++ ) {	// Scan tile for available pixels
    for( j = 0; j < Tile_w; j++ ) {
      if ( Tile[ i ][ j ] == 1 ) {
        Avail[ num_avail ].x = j;
        Avail[ num_avail ].y = i; 
	      num_avail++;
      }
    }
  }
  if ( num_avail == 0 ) {			// No pixels available?
    return 0;
  }

  shuffle();
  shuffle();
  shuffle();

  return 1;
}					//End routine init_avail

void shuffle(){
  int  a, b;				// Temporarily Avail list indices
  point2D  tmp_pt;			// Temporary 2D point structure

  for( int i = 0; i < num_avail; i++ ) {		// Shuffle the available pixels
    if( rand_num() <= 0.5){
      a = int( rand_num()*0.5 * (num_avail - 1) );
      b = int( (rand_num() *0.5* (num_avail - 1)) + 0.5 );
    }else{
      a = int( (rand_num()*0.5*(num_avail - 1)) + .25 );
      if( rand_num() < .5 ){
        b = int( rand_num() *0.25*(num_avail - 1) );
      }else{
        b = int( (rand_num()*0.25*(num_avail - 1)) + .75 );
      }
    }

    tmp_pt = Avail[ a ];
    Avail[ a ] = Avail[ b ];
    Avail[ b ] = tmp_pt;
  }
}

static int init_tile( int **id, int seg_id, int ll[], int ur[] )

  //  This routine initializes the tile's pixels to 1 (within segment
  //  but not covered by a stroke) or 0 (outside segment); during
  //  painting the pixels will be switched to 2 (covered by a stroke);
  //  the routine returns the number of pixels within the segment
  //
  //  id:      Expanded segment ID array
  //  seg_id:  ID of segment to stroke
  //  ll:      Lower-left corner of segment in ID array
  //  ur:      Upper-right corner of segment in ID array
{
  int  count = 0;			// Pixel count
  int  i, j;				// Loop counters


  for( i = 0; i < Tile_h; i++ ) {	// For all pixels in bounding box
    for( j = 0; j < Tile_w; j++ ) {
      if ( id[ i + ll[ 1 ] ][ j + ll[ 0 ] ] == seg_id ) {
        Tile[ i ][ j ] = 1;   
        count++;

      } else {
        Tile[ i ][ j ] = 0;
      }
    }
  }

  return count;
}					// End routine init_tile


static int scan_cvt(
  point2D vt[], point2D* &pt, int pt_n, int& pt_max, int w, int h )

  //  This routine scan converts the (possibly transformed) rectangle
  //  with the given vertex coordinates, returning the pixel count and
  //  filling the given array with pixel coordinates
  //
  //  vt:      Rectangle vertices
  //  pt:      Point list
  //  pt_n:    Start storing points here
  //  pt_max:  Maximum size of point list
  //  w, h:    Width, height of image canvas
{
  int      cur_y;			// Current Y-value
  int      i;				// Loop counter
  point2D  l[ 2 ];			// Left segment
  float    l_b;				// Left Y-intercept
  float    l_m_inv;			// Left slope reciprical
  float    l_x;				// Current left segment X-value
  point2D  r[ 2 ];			// Right segment
  float    r_b;				// Right Y-intercept
  float    r_m_inv;			// Right slope reciprical
  float    r_x;				// Current right segment X-value
  point2D  v[ 4 ];			// Sorted copy of rectange vertices


  v[ 0 ] = vt[ 0 ];
  v[ 1 ] = vt[ 1 ];
  v[ 2 ] = vt[ 2 ];
  v[ 3 ] = vt[ 3 ];
  qsort( (void *) v, 4, sizeof( float ) * 2, cmp_2D_vert );

  if ( pt_max == 0 ) {			// Create pixel list, if req'd
    pt_max = 64;
    pt = new point2D[ pt_max ];
  }

  l[ 0 ] = v[ 0 ];			// Build first left, right segments
  r[ 0 ] = v[ 0 ];
  if ( v[ 1 ].x < v[ 2 ].x ) {
    l[ 1 ] = v[ 1 ];
    r[ 1 ] = v[ 2 ];
  } else {
    r[ 1 ] = v[ 1 ];
    l[ 1 ] = v[ 2 ];
  }

  comp_line( l[ 0 ], l[ 1 ], l_m_inv, l_b );
  comp_line( r[ 0 ], r[ 1 ], r_m_inv, r_b );

  //  Compute starting Y-value; since canvas runs from (0,0) to (w,h),
  //  we would normally clip Y to w and X to h, but we assume caller
  //  checks to ensure scan converted pixels are within its canvas

  cur_y = int( ceil( v[ 0 ].y ) );

  //  Use formula y = ( 1 / m_inv ) * x + b to compute starting X-values

  l_x = ( l_m_inv == 0 ) ? l[ 0 ].x : ( cur_y - l_b ) * l_m_inv;
  r_x = ( r_m_inv == 0 ) ? r[ 0 ].x : ( cur_y - r_b ) * r_m_inv;

  //  Again, we would normallly check for cur_y < h, but caller deals
  //  with pixels outside canvas

  while( cur_y <= v[ 3 ].y ) {
    if ( cur_y >= l[ 1 ].y ) {		// If past top of left line, update
      l[ 0 ] = l[ 1 ];
      l[ 1 ] = v[ 3 ];

      comp_line( l[ 0 ], l[ 1 ], l_m_inv, l_b );
      l_x = ( l_m_inv == 0 ) ? l[ 0 ].x : ( cur_y - l_b ) * l_m_inv;
    }

    if ( cur_y >= r[ 1 ].y ) {		// If past top of right line, update
      r[ 0 ] = r[ 1 ];
      r[ 1 ] = v[ 3 ];

      comp_line( r[ 0 ], r[ 1 ], r_m_inv, r_b );
      r_x = ( r_m_inv == 0 ) ? r[ 0 ].x : ( cur_y - r_b ) * r_m_inv;
    }

    //  Stroke current scanline

    for( i = int( ceil( l_x ) ); i <= floor( r_x ); i++ ) {
      if ( i >= 0 && i < w ) {		// Ensure pixel in canvas
        if ( pt_n == pt_max ) {		// Expand position array, if req'd
          expand_pt( pt, pt_max );
        }
  
        pt[ pt_n ].x = i;		// Store current pixel position
        pt[ pt_n++ ].y = cur_y;
      }					// End if pixel in canvas
    }					// End for all pixels on scanline

    l_x += l_m_inv;			// Move to next scanline
    r_x += r_m_inv;
    cur_y++;
  }

  return pt_n;				// Return pixel count
}					// End routine scan_cvt

void set_bezier( bool bo ){
  bezier_on = bo;
}

int stroke( int **id, int w, int h,
  int seg_id, stroke_s * &st, int st_n, int& st_max, float cov, int seg_type )

  //  This routine builds a list of strokes to cover a user-chosen segment
  //  from the dataset; returns the number of strokes in the stroke list
  //
  //  id:      Expanded segment ID array
  //  w, h:    Width, height of ID array
  //  seg_id:  ID of segment to stroke
  //  st:      Array of strokes for this segment
  //  st_n:    Starting position for new strokes in array
  //  st_max:  Current maximum size of stroke array
  //  cov:     Requested coverage for segment
  //  size:    Requested starting size for segment's strokes
{
	int  count;				// Count of pixels in segment
  int  cut;				// Segment size cutoff (in pixels)
  int  i;				// Loop counter
  int  ll[ 2 ];				// Lower-left corner of seg bbox
  int  pix_n;				// Count of pixels in current stroke
  int  start;				// Position of first new segment    
  int  ur[ 2 ];				// Upper-right corner of seg bbox

  cur_st_n = 0;
  total_length = 0;
  total_width = 0;
 
	int flag = 0;
	
	if ( !bbox( id, w, h, seg_id, ll, ur ) ) {  // start finding the bounding box with the first element member (j-1,i-1).
    cerr<<"In paint.cpp, segment "<<seg_id<<" no bounding box created (no element found)."<<endl;
		return st_n;
  }		

  //  Build available pixel list (used in find_pos, but allocated once
  //  here to avoid repeated new/delete) and 2D tile array to hold pixel
  //  status

  Tile_w = ur[ 0 ] - ll[ 0 ] + 1;
  Tile_h = ur[ 1 ] - ll[ 1 ] + 1;

  Avail = new point2D[ Tile_w * Tile_h ];

  Tile = new int *[ Tile_h ];
  for( i = 0; i < Tile_h; i++ ) {
    Tile[ i ] = new int[ Tile_w ];
  }

  count = init_tile( id, seg_id, ll, ur );

  cut = int( count * ( 1.0 - cov ) );
  start = st_n;

  init_avail();
  // "Segment " << seg_id << ": ";
  do {
    if ( st_n == st_max ) {
      expand_st( st, st_max );
    }

   // st[ st_n ].s[ 0 ] = size[ 0 ];
   // st[ st_n ].s[ 1 ] = size[ 1 ];
   
    pix_n = add_stroke( ll, st[ st_n ] );

    if ( pix_n != 0 ) {
     // cerr << "."; period
      total_length += st[ st_n ].s[ 1 ];
      total_width  += st[ st_n ].s[ 0 ];
      cur_st_n++;
      st_n++;
      count -= pix_n;
    }
  }
  while( count > cut && pix_n != 0 );
  if ( count > cut ) {
    cerr << "(bailed @ count of " << count << "; wanted " << cut << ")\n";
    cerr << "Placed "<< st_n<<" strokes."<<endl;
  } else {
    cerr << "(met count @ " << count << ")\n";
  }

  for( i = start; i < st_n; i++ ) {	// Offset new strokes into world coords
    st[ i ].p.x += ll[ 0 ];
    st[ i ].p.y += ll[ 1 ];
  }

  for( i = 0; i < Tile_h; i++ ) {	// Free tile, availability arrays
    delete [] Tile[ i ];
  }
  delete [] Tile;
  delete [] Avail;
  return st_n;
}					// End method stroke


void set_array(int tex, Point array[4]){
  switch (tex){
    case 0:
    case 1:
      array[0].x = -0.31;
      array[0].y = -0.45;
      array[1].x = 0.436;
      array[1].y = -0.28;
      array[2].x = 0.04;
      array[2].y = 0.37;
      array[3].x = -0.375;
      array[3].y = 0.3;
      break;
    case 2:
    case 3:
      array[0].x = -0.34;
      array[0].y = -0.45;
      array[1].x = 0.33;
      array[1].y = -0.305;
      array[2].x = 0.29;
      array[2].y = 0.5;
      array[3].x = -0.31;
      array[3].y = 0.45;
      break;
    case 4:
    case 5:
      array[0].x = -0.33;
      array[0].y = -0.44;
      array[1].x = 0.45;
      array[1].y = -0.29;
      array[2].x = 0.195;
      array[2].y = 0.41;
      array[3].x = -0.34;
      array[3].y = 0.25;
      break;
    case 6:
    case 7:
      array[0].x = -0.45;
      array[0].y = -0.435;
      array[1].x = 0.37;
      array[1].y = -0.435;
      array[2].x = 0.39;
      array[2].y = 0.4;
      array[3].x = -0.435;
      array[3].y = 0.44;
      break;
    case 8:
    case 9:
      array[0].x = -0.33;
      array[0].y = -0.46;
      array[1].x = 0.5;
      array[1].y = -0.38;
      array[2].x = 0.235;
      array[2].y = 0.455;
      array[3].x = -0.5;
      array[3].y = 0.335;
      break;
    case 10:
    case 11:
      array[0].x = -0.29;
      array[0].y = -0.44;
      array[1].x = 0.46;
      array[1].y = -0.35;
      array[2].x = 0.325;
      array[2].y = 0.41;
      array[3].x = -0.425;
      array[3].y = 0.455;
      break;
    case 12:
    case 13:
      array[0].x = -0.435;
      array[0].y = -0.395;
      array[1].x = 0.437;
      array[1].y = -0.44;
      array[2].x = 0.437;
      array[2].y = 0.448;
      array[3].x = -0.435;
      array[3].y = 0.448;
      break; 
    default:
      array[0].x = -0.5;
      array[0].y = -0.5;
      array[1].x = 0.5;
      array[1].y = -0.5;
      array[2].x = 0.5;
      array[2].y = 0.5;
      array[3].x = -0.5;
      array[3].y = 0.5;
      break;
  }
} //set_array


static void transform_curves( int tex, float x, float y, float rot, float s[], point2D v[] )

  //  This routine transforms a stroke centered about (0,0) of size 1
  //  pixel wide by 1 pixel high with the given translate, rotate, and
  //  scale values
  //
  //  x, y:  Translation in X and Y
  //  rot:   Rotation (counterclockwise) about Z, in degrees
  //  s:     Scale factors in X and Y
  //  v:     Vertices of transformed stroke
{
  float  cur_v[ 2 ];			// Current vertex values
  int    i;				// Loop counter
  Point array[ 4 ];

  set_array( tex , array);

  v[ 0 ].x = array[0].x * s[ 0 ];
  v[ 0 ].y = array[0].y * s[ 1 ];
  v[ 1 ].x = array[1].x * s[ 0 ];
  v[ 1 ].y = array[1].y * s[ 1 ];
  v[ 2 ].x = array[2].x * s[ 0 ];
  v[ 2 ].y = array[2].y * s[ 1 ];
  v[ 3 ].x = array[3].x * s[ 0 ];
  v[ 3 ].y = array[3].y * s[ 1 ];

 /* if( type == 'p' ){
    v[ 0 ].x = st.controlpts[2][0][0];//*s[0];
    v[ 0 ].y = st.controlpts[2][0][1];//*s[1];

    v[ 1 ].x = st.controlpts[2][3][0];//*s[0];
    v[ 1 ].y = st.controlpts[2][3][1];//*s[1];

    v[ 2 ].x = st.controlpts[1][3][0];//*s[0];
    v[ 2 ].y = st.controlpts[1][3][1];//*s[1];
    
    v[ 3 ].x = st.controlpts[1][0][0];//*s[0];
    v[ 3 ].y = st.controlpts[1][0][1];//*s[1];
  }*/

  rot = rot * 3.14159 / 180.0;

  for( i = 0; i < 4; i++ ) {
    cur_v[ 0 ] = ( v[ i ].x * cos( rot )  ) - ( v[ i ].y * sin( rot ) );
    cur_v[ 1 ] = ( v[ i ].x * sin( rot )  ) + ( v[ i ].y * cos( rot ) );

    v[ i ].x = cur_v[ 0 ] + x;
    v[ i ].y = cur_v[ 1 ] + y;
  }

}					// End routine transform_curves


static void transform( float x, float y, float rot, float s[], point2D v[] )

  //  This routine transforms a stroke centered about (0,0) of size 1
  //  pixel wide by 1 pixel high with the given translate, rotate, and
  //  scale values
  //
  //  x, y:  Translation in X and Y
  //  rot:   Rotation (counterclockwise) about Z, in degrees
  //  s:     Scale factors in X and Y
  //  v:     Vertices of transformed stroke
{
  float  cur_v[ 2 ];			// Current vertex values
  int    i;				// Loop counter


  v[ 0 ].x = -0.5 * s[ 0 ];
  v[ 0 ].y = -0.5 * s[ 1 ];
  v[ 1 ].x =  0.5 * s[ 0 ];
  v[ 1 ].y = -0.5 * s[ 1 ];
  v[ 2 ].x =  0.5 * s[ 0 ];
  v[ 2 ].y =  0.5 * s[ 1 ];
  v[ 3 ].x = -0.5 * s[ 0 ];
  v[ 3 ].y =  0.5 * s[ 1 ];

  rot = rot * 3.14159 / 180.0;

  for( i = 0; i < 4; i++ ) {
    cur_v[ 0 ] = ( v[ i ].x * cos( rot )  ) - ( v[ i ].y * sin( rot ) );
    cur_v[ 1 ] = ( v[ i ].x * sin( rot )  ) + ( v[ i ].y * cos( rot ) );

    v[ i ].x = cur_v[ 0 ] + x;
    v[ i ].y = cur_v[ 1 ] + y;
  }
}					// End routine transform
