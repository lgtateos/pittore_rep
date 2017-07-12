#include "segment.h"
#include "global.h"


/*--------------------------------------------------------------------------*/
/*  SEGMENT.CPP								    */
/*    Routines to segment an RGB image into connected regions, where all    */
/*    pixels in a region are within a user-chosen colour distance from      */
/*    one another							    */
/*									    */
/*- Modification History: --------------------------------------------------*/
/*  When:	Who:			Comments:			    */
/*									    */
/*  28-Nov-99	Christopher G. Healey	Initial implementation		    */
/*  14-Jun-01	Christopher G. Healey	Extended to support arbitrary data  */
/*					with n individual layers	                                      */
/*  10-Mar-06 Laura G. Tateosian Added flushing routine to reuse a segment for a new frame  */
/*--------------------------------------------------------------------------*/

//  Module global variables

struct p_pos {				// Pixel position structu
  int  x, y;
};

float med_area;        // Temporary median area value.
val_s  cur_c;				// Temporary median attr val struct
int frame_n;				// Frame number
//grid_file grid_f;		        // Grid_file allows access to grid arranged date
int seg_type;				// Allows a choice of data attribute to be filtered

//  Module global prototypes

static int    add_segment( int, int, int, val_s&, seg_s&, p_pos *, int, grid_file&);
static void   attr_update( val_s&, float&, float&, p_pos, seg_s&, float&, grid_file& );
static void   grow_segment( seg_s&, int&, int );
static int    neighbour( int, int, p_pos *, val_s, seg_s&, grid_file& );


static int add_segment(
  int x, int y, int n, val_s& c, seg_s& s, p_pos h[], int h_max, grid_file& grid_f )

  //  This private routine begins a new segment at the given pixel
  //  position
  //
  //  x, y:   Column, row of pixel that starts new segment
  //  n:      Segment index assigned to new segment
  //  c:      Segment median attribute values
  //  s:      Structure to hold segments and control information
  //  h:      History array
  //  h_max:  Maximum size of history array
{
	int    centroid;
  int    count = 0;			// Elements included in this segment
	float  cur_area = 0;
  float  cur_den = 1.0;			// Current attr median denominator
  float  cur_num = 1.0;			// Current attr median numerator
  int    h_pos = 0;			// Index position in history array
  int    head = 0;			// Head of queue
  int    i;				// Loop counter
  p_pos  nb[ 8 ];			// Neighbour positions
  int    num;				// Number of neighbours to add
  p_pos  q[ 2048 ];			// Queue of elements to process
  int    tail = 0;			// Tail of queue

  //cout<<"1add segment : x, y = "<<x<<" , "<<y<<endl;
  q[ tail ].x = x;			// Initialize queue
  q[ tail ].y = y;
  tail = ( tail + 1 ) % 2048;

  h[ h_pos ].x = x;			// Initialize history array
  h[ h_pos ].y = y;
  h_pos = ( h_pos + 1 ) % h_max;

  //  Grab attribute values at starting element position

  grid_f.get( ( y * s.w ) + x, frame_n, c.v );

  s.seg[ y ][ x ] = n;
  count++; 

  while( head != tail ) {		// Continue until queue exhausted
    x = q[ head ].x;
    y = q[ head ].y;

    num = neighbour( x, y, nb, c, s, grid_f );	// Query valid neighbour elements
		
    for( i = 0; i < num; i++ ) {	// Add neighbours, update median values
      s.seg[ nb[ i ].y ][ nb[ i ].x ] = n;
      attr_update( c, cur_num, cur_den, nb[ i ], s, cur_area, grid_f);
			
			count++;

      h[ h_pos ].x = nb[ i ].x;		// Record additions in history list
      h[ h_pos ].y = nb[ i ].y;
      h_pos = ( h_pos + 1 ) % h_max;

      q[ tail ].x = nb[ i ].x;		// Append neighbours to queue
      q[ tail ].y = nb[ i ].y;
      tail = ( tail + 1 ) % 2048;
      assert( head != tail );
    }

    head = ( head + 1 ) % 2048;
	}
  
  return count;
}					// End routine add_segment


static void attr_update( val_s& c, float& num, float& den, p_pos p, seg_s& s, float& cur_area, grid_file& grid_f )

  //  This private routine updates the median attr value by adding a
  //  scaled version of the given elements's attrs to the total; the
  //  scaling factor is user-chosen, r_num / r_den, which means that
  //  each pixel has an influence of (r_num / r_den) of the previous
  //  element, that is:
  //
  //  - elem 1:  c_med = 1 * c_1
  //  - elem 2:  c_med = ( 1 - (r_num / r_den) ) c_med + (r_num / r_den) c_2
  //  - elem 3:  c_med = ( 1 - (r_num / r_den) ) c_med + (r_num / r_den)^2 c_3
  //  - elem 4:  c_med = ( 1 - (r_num / r_den) ) c_med + (r_num / r_den)^2 c_4
  //
  //  and so on (above example is for any of the attr values). Special
  //  math is performed to ensure a normalized c_med, that is, 0 <=
  //  c_med <= 1
  //
  //  This version uses floating-point numbers rather than exact
  //  numerators and denominators to track the scaling factor (hence,
  //  the ..._f in the routine's name). Although theoretically less
  //  precise, this will avoid floating-point overflow, since the
  //  numerator-denominator requires a pow( r_num, i - 1 ) at each
  //  step i.
  //
  //  c:     Current median values (num attributes)
  //  num:   Current numerator for updating median
  //  den:   Current denominator for updating median
  //  p:     Element position whose attrs we want to add
  //  s:     Structure to hold segments and control information
{
  int    i;				// Loop counter
  float  r;				// Current elements's scale factor

  num = num * ( s.num / s.den );	// Compute scaling factor
  den = den + num;
  r = num / den;

  //  Note:  Uses global cur_c, allocated (and deallocated) in segment();
  //         this avoids repeated new/delete, which is costly

    grid_f.get( ( p.y * s.w ) + p.x, frame_n, cur_c.v );

    for( i = 0; i < s.attr_n; i++ ) {
		  c.v[ i ] = ( ( 1.0 - r ) * c.v[ i ] ) + ( r * cur_c.v[ i ] );
	  }

}					// End routine attr_update


static void grow_segment( seg_s& s, int& seg_c_max, int factor )

  //  This routine grows a full segment array by a given factor
  //
  //  s:          Structure to hold segments and control information
  //  seg_c_max:  Current maximum array size
  //  factor:     Factor to grow
{
  int    i;				// Loop counter
  val_s *new_seg_c;			// New segment array
  float *new_seg_n;			// New segment size array

  new_seg_c = new val_s[ seg_c_max * factor ];
  for( i = 0; i < seg_c_max; i++ ) {
    new_seg_c[ i ].v = s.seg_c[ i ].v;
  }

  new_seg_n = new float[ seg_c_max * factor ];
  for( i = 0; i < seg_c_max; i++ ) {
    new_seg_n[ i ] = s.seg_n[ i ];
  }

  delete [] s.seg_c;
  delete [] s.seg_n;

  s.seg_c = new_seg_c;
  s.seg_n = new_seg_n;
  seg_c_max *= factor;
}					// End routine grow_segment


static int neighbour( int x, int y, p_pos nb[], val_s c, seg_s& s, grid_file& grid_f )

  //  This private routine finds all neighbours that are:
  //
  //  - not part of some other segment
  //  - within the dataset boundaries
  //  - within the attribute error cutoff
  //
  //  Number of neighbours found returned to caller
  //
  //  x, y:  Column, row of pixel that defines neighbourhood to check
  //  nb:    Position of each neighbour found
  //  c:     Current median value (for all n attributes)
  //  s:     Structure to hold segments and control information
{
	float cur_area;
  int    cur_x, cur_y;			// Current neighbour to check
  int    i, j;				// Loop counters
  int    num = 0;			// Number of neighbours
  int    off[ 8 ][ 2 ] = {		// Offset of each neighbour
    { -1,  1 }, {  0,  1 }, {  1,  1 },
    { -1,  0 }, {  1,  0 },
    { -1, -1 }, {  0, -1 }, {  1, -1 }
  };
  int     rc;				// Attributes OK flag
	int     rc1 = 0;  // Flag to indicate if median data has invalid entry (<-990)
	int     rc2 = 0;  // Flag to indicate if current data element has invalid entry (<-990)


	for( i = 0; i < 8; i++ ) {		// For all neighbours
    
		cur_x = x - off[ i ][ 0 ]; // set cur_x to next neighbor's x position 
    cur_y = y - off[ i ][ 1 ];

		// if outside of dataset boundaries
    if ( cur_x < 0 || cur_x >= s.w || cur_y < 0 || cur_y >= s.h ) {
      continue;
    }
    
		// if s.seg[ cur_y ][ cur_x ] != -1, it has already gotten a segment membership  
    if ( s.seg[ cur_y ][ cur_x ] != -1 ) {
      continue;
    }

    rc = 1;  // one neighbor added as default
		
    //  Note:  Uses global cur_c, allocated (and deallocated) in segment();
    //         this avoids repeated new/delete, which is costly

      grid_f.get( ( cur_y * s.w ) + cur_x, frame_n, cur_c.v );
		 
   //  Ensure each attribute difference is within the error bounds to
         //  allow the neighbour
         //  - HACK - HACK - HACK - HACK -
         //  Only test selected attributes
			if( seg_type == 0 ){     // test first four attributes
			  for( j = 0; j < 4; j++ ) {
          if ( fabs( c.v[ j ] - cur_c.v[ j ] ) > s.err[ j ] ) {
            rc = 0;
            break;
				  }
			  }
			}else if( seg_type > 0 ){ // only test selected attribute
         if(fabs( c.v[ seg_type ] - cur_c.v[ seg_type ] ) > s.err[ 1 ] ) {
           rc = 0;
         }       
      } 
	
    if ( rc ) {				// Add if all attrs within error limit
      nb[ num ].x = cur_x;
      nb[ num ].y = cur_y;
      num++;
    }
  }  // end of for all neighbors
  return num;
}					// End routine neighbour

void dealloc_seg_arr( seg_s& s ){
  if ( s.seg != NULL ){ 
    for( int i = 0; i < s.h; i++){
      delete [] s.seg[i];
    }
    delete [] s.seg;
  }
}

void flush_seg_full( seg_s& s){
  if ( s.err != NULL ){
    delete [] s.err;
  }
  flush_seg( s, -1, -1 );
}

void flush_seg( seg_s& s, int h, int w){
  //This routine should be called if seg_s s has been used and the caller wants to reuse it.  It deallocates the dynamic variable in 
  // seg_s struct, with the exception of s.err.
  // s.err is calculated in relation to the mins and maxs over all the frames, so remains constant across frames.
  // s.seg is also deallocated if the dimensions of the new frame are different from h and w.
  // 
  // s previously used seg_s struct
  // h and w previous values of s.h and s.w respectively
  // segmentation_type

  if( s.seg_c!= NULL){
    delete [] s.seg_c;
  }
  if( s.seg_n!= NULL){
    delete [] s.seg_n;
  }

  if ( s.h != h || s.w != w){ // if either dimensions are different in the new frame deallocate old segment array and allocate new one
    dealloc_seg_arr( s );
  }
}

int segment( int offset, seg_s& s, int type, grid_file& g_file, int frame_num )

  //  This routine begins the segmentation process. We reserve segment
  //  ID 0 for any segments that produce only a small number of pixels
  //  (usually < 1% of the total pixels in the image)
  //
  //  The following fields in s must be initialized by caller:
  //  - f, w, h, attr_n, seg (allocated 2D integer array [w][h]), min,
  //    err ([attr_n] for every attr), num, den
  //
  //  The following fields in s are created or seg during segmentation:
  //  - seg_c, seg_n, n
  //
  //  The number of segments created is also returned to the caller
  //  
  //  offset:  	Offset into the data file, to adjust for the choice of frame
  //  s:  	Structure to hold segments and control information
	//  type: Data property to segment on.  0 uses the first 4.  Positive ints give the attribute number.
	//        -1 indications segmentation on voronoi region size.
  //  g_file:   Grid_file to access data
  //
{
  val_s  c_med;				// Current segment's median attribute values
  p_pos *hist;				// Pixel history array
  int    hist_n;			// Pixel history size
  int    i, j, k;			// Column, row loop counters
  int    n = 1;				// Total number of segments
  int    p_count;			// Unsegmented pixel count
  int    seg_c_max;			// Max size of colour median list
  int    seg_num;			// Number of pixels in cur segment
  int    seg_min;			// Minimum segment size in pixels
  
  c_med.v = NULL;
  frame_n = frame_num;
  //grid_f = g_file;
  seg_type = type;

  seg_min = int( float( s.w * s.h ) * s.min );
  p_count = s.w * s.h;

  seg_c_max = 2;			// Create median attribute value list
  s.seg_c = new val_s[ seg_c_max ];
  s.seg_n = new float[ seg_c_max ];
 
	//  Create a global median attribute value structure, used by helper
  //  routines to temporarily read data, allows us to avoid numerous
  //  expensive new/delete requests

  cur_c.v = new float[ s.attr_n ];

  //  We maintain a history of the last seg_min pixels added to the
  //  current segment. If the total number of pixels in the segment
  //  is less than seg_min, we ignore the segment, and the history gives
  //  us an efficient way of "resetting" the pixels to special segment
  //  index 0

  if ( seg_min == 0 ) {			// Need at least 1 value in segment
    hist_n = 1;
  } else {
    hist_n = seg_min;
  }
  hist = new p_pos[ hist_n ];

  for( i = 0; i < s.h; i++ ) {		// Initialize segment indices..
    for( j = 0; j < s.w; j++ ) {	// ..to "undefined"
      s.seg[ i ][ j ] = -1;
    }
  }

  //  Walk through image, and for any unindexed pixel, begin a new
  //  segment at that pixel position
int count = 0;
  for( i = 0; i < s.h; i++ ) {
    for( j = 0; j < s.w; j++ ) {
			count++;
      if ( s.seg[ i ][ j ] == -1 ) {  
        c_med.v = new float[ s.attr_n ];
				assert( c_med.v != NULL );      // if c_med.v == NULL quit program            

        seg_num = add_segment( j, i, n, c_med, s, hist, seg_min, g_file );
				  
      //  Tag any segment that contains less than seg_min pixels as
      //  special segment 0, too small to be considered
        if ( seg_num < seg_min ) {
          for( k = 0; k < seg_num; k++ ) {
            s.seg[ hist[ k ].y ][ hist[ k ].x ] = 0;
	        }
          delete [] c_med.v;
          c_med.v = NULL;
				} else {
         // cerr << "Segment " << n << ", " << seg_num << " pixels ";
         // cerr << form( "(%4.1f%%)\n",
         //               float( seg_num ) / float( s.w * s.h ) * 100.0 );
         p_count -= seg_num;

         if ( n == seg_c_max ) {	// Median colour list full?
           grow_segment( s, seg_c_max, 2 );
         }
         s.seg_c[ n ] = c_med;
         s.seg_n[ n ] = seg_num;
         n++;
        }				// End else add segment to segment list
      }					// End if pixel is unsegmented   
		}					// End for all columns
  }					// End for all rows

  s.n = n;
	

//  cerr << "\n" << p_count << " unsegmented pixels ";
//  cerr << form( "(%4.1f%%)\n", float( p_count ) / float( s.w * s.h ) * 100.0 );

  delete [] hist;
  if (  c_med.v != NULL ){
    delete [] c_med.v;
  }
  delete [] cur_c.v;
  //grid_f.close();
  //g_file.close();
  return n;
}					// End routine segment

void init_seg(seg_s& s, int w, int h, int num_attr, float min, float num, float den){
  s.w = w;			// Width of data field
  s.h = h;			// Height of data field
  s.attr_n = num_attr;		// Number of attrs in each element
 // s.seg = NULL;			// 2D array of segment IDs (one per element)
  s.seg_c = NULL;		// Array of median attr vals for each segment
  s.seg_n = NULL;		// Size of segment (as fraction of data size)

  s.n = 0;			// Number of segments
  s.min = min;			// Minimum allowable segment size
  s.err = NULL;			// Error cutoff for each attr
  s.num = num;			// Numerator of new attr weighting factor
  s.den = den;			// Denominator of new attr weighting factor

  for( int i = 0; i < s.h; i++ ) {		// Initialize segment indices..
    for( int j = 0; j < s.w; j++ ) {	// ..to "undefined"
      s.seg[ i ][ j ] = -1;
    }
  }
}

void alloc_seg_array( seg_s& s, int w, int h ){
  int i;
  s.seg = new int *[ h ];         // And Create segment ID array
	for ( i = 0; i < h; i++ ) {
	  s.seg[ i ] = new int[ w ];
	}
}

int read_segment( seg_s& the_seg, int* regarr, int numreg, float ll[], float ur[], string seg_filename ){
 int i, j;
 long pos;
// ifstream fin;
// fin.open( (char*) seg_filename );
 //if ( !fin.good() ){
 filereader fp;

 fp.open( seg_filename, 'r' ) ;
 fp.seek(0, END);
 if( !fp.offset()){
   fp.close();
   return 0;
 }else {
   fp.seek(0, BEGIN);
   for( i = 0; i < numreg; i++ ){
     fp.read_raw( (char *) &regarr[i], sizeof( int ) ); 
   }
  
   fp.read_raw( (char*) &ll[0], sizeof(float) );
   fp.read_raw( (char*) &ll[1], sizeof(float) );
   fp.read_raw( (char*) &ur[0], sizeof(float) );
   fp.read_raw( (char*) &ur[1], sizeof(float) );
  
   for( i = 0; i < the_seg.h; i++ ){
    for( j = 0; j < the_seg.w; j++ ){
      
      pos = fp.offset();
      fp.read_raw( (char *) &the_seg.seg[i][j], sizeof( int ) );
      //printf("the_seg.seg[%d][%d] =%d\n",i, j, the_seg.seg[i][j]);
    }
   }
   return 1;
 }
}

void write_segment(seg_s the_seg, int* regarr, int numreg, float ll[], float ur[], int cur_frame, string seg_filename ){
  int i, j;
  filereader fp;
  long pos;
  fp.open( seg_filename, 'x' );
  pos = fp.offset();
  cerr<< "in write_segment with num_reg = "<<numreg<<endl;
  for( i = 0; i < numreg; i++ ){
    fp.write_raw( (char *) &regarr[i], sizeof( int ) ); 
  }
  pos = fp.offset();
  fp.write_raw( (char*) &ll[0], sizeof(float) );
  fp.write_raw( (char*) &ll[1], sizeof(float) );
  fp.write_raw( (char*) &ur[0], sizeof(float) );
  fp.write_raw( (char*) &ur[1], sizeof(float) );
  pos = fp.offset();
  for( i = 0; i < the_seg.h; i++ ){
    for( j = 0; j < the_seg.w; j++ ){
      //printf("the_seg.seg[%d][%d] =%d\n",i, j, the_seg.seg[i][j]);
      fp.write_raw( (char *) &the_seg.seg[i][j], sizeof( int ) );
    }
  }
  pos = fp.offset();
  fp.close();
}