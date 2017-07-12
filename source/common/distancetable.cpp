#ifndef DISTANCETABLE_CPP
#define DISTANCETABLE_CPP

#include "distancetable.h"

/*--------------------------------------------------------------------------*/
/*  DISTANCETABLE.CPP                                                       */ 
/*    This file contains the implementation of a hash table used to store   */
/*    2D points.  The table is constructed to reduce the number of points   */
/*    for distance calculations.                                            */
/*                                                                          */
/*- Modification History ---------------------------------------------------*/
/*  When:       Who:	                Comments:                           */
/*  18-Aug-2005	Brent M. Dennis         Initial implementation              */
/*  13-Apr-2006	Brent M. Dennis         Addressed logical issue about       */
/*                                      whether or not to include points    */
/*                                      that have distances = 0.0 from the  */
/*                                      point whose neighbors or nearest    */ 
/*                                      neighbor is being sought.  Thus, the*/
/*                                      internal search routines now have a */
/*                                      new argument that changes the       */
/*                                      distance comparison function from   */
/*                                      == to >=.                           */
/*--------------------------------------------------------------------------*/


DistanceTable::DistanceTable( )
// Default constructor
{
  x_lo = x_hi = y_lo = y_hi = 0.0;
  table_dim       = 0;
  table           = NULL;
  points          = NULL;
  pts_n = pts_max = 0;
  avail_list.next = 0;

  DEBUG           = 0;
  REPORT_ERROR    = 1;
}                                    // end default constructor

DistanceTable::DistanceTable( int d, int n, Point * pts )
  //   Distance Table Constructor
  // 
  // 
  //   d   :      number of rows for the table
  //   n   :      number of points to add to table
  //   pts :      buffer holding points to add to table. This buffer is
  //              not copied and must not be de-allocated outside of the
  //              DistanceTable class
{
  int i;

  table_dim       = d;               // set class var
  pts_n           = n;               // set class var
  pts_max         = CEIL( 2*pts_n );        // set max pts to 2*pts_n
  points          = new Point[ pts_max ];   // allocate Point buffer
  avail_list.next = 0;               // no old IDs are available at startup
  REPORT_ERROR    = 1;               // default value for REPORT_ERROR is on
  DEBUG           = 0;               // default value for DEBUG is off
  
  x_hi = y_hi = -10000.0; // set initial bounds
  x_lo = y_lo = 10000.0;  // set initial bounds

  // scan the point array to determine the range of coordinate values
  for( i = 0; i < pts_n; i++ )
    {
      points[ i ] = pts[ i ];
      if( pts[ i ].x < x_lo )
	x_lo = pts[ i ].x;
      if( pts[ i ].x > x_hi )
	x_hi = pts[ i ].x;
      
      if( pts[ i ].y < y_lo )
	y_lo = pts[ i ].y;
      if( pts[ i ].y > y_hi )
	y_hi = pts[ i ].y;
    }

  // rng is max - min
  x_rng = x_hi - x_lo;  
  y_rng = y_hi - y_lo;

  // expand table to allow for points to be shifted around.
  // this requires numerous updates
  x_lo = x_lo - x_rng / table_dim;
  x_hi = x_hi + x_rng / table_dim;
  
  y_lo = y_lo - y_rng / table_dim;
  y_hi = y_hi + y_rng / table_dim;
  
  x_rng = x_hi - x_lo;
  y_rng = y_hi - y_lo;

  table_dim = table_dim + 2;

  x_segment = x_rng / ( table_dim - 1 );
  y_segment = y_rng / ( table_dim - 1 );

  BuildTable( );         // construct the table
}                                         //end constructor

DistanceTable::DistanceTable( int d, int n, float * pts )
  //   Distance Table Constructor
  // 
  // 
  //   d   :      number of rows for the table
  //   n   :      number of points to add to table
  //   pts :      buffer holding float representation of points to add to 
  //              table. This buffer is copied into a Point buffer 
  //              representation.  This buffer is for 2D points.
{
  int i;
  
  table_dim       = d;                    // update class var
  pts_n           = n;                    // update class var
  pts_max         = pts_n;                // update class var
  points          = new Point[ pts_max ]; // allocate Point buffer
  avail_list.next = 0;                    // no IDs are available at startup
  DEBUG           = 0;                    // default value for DEBUG is off
  REPORT_ERROR    = 1;                    // default value for REPORT_ERROR 
                                          // is on
  
  x_hi = y_hi = -10000.0;   // set initial bounds
  x_lo = y_lo =  10000.0;   // set initial bounds

  // scan the point array to determine the range of coordinate values
  for( i = 0; i < pts_n; i++ )
    {
      if( pts[ 3*i ] < x_lo )
	x_lo = pts[ 3*i ];
      if( pts[ 3*i ] > x_hi )
	x_hi = pts[ 3*i ];
      
      if( pts[ 3*i + 1 ] < y_lo )
	y_lo = pts[ 3*i + 1 ];
      if( pts[ 3*i + 1 ] > y_hi )
	y_hi = pts[ 3*i + 1 ];
      // add values to Point buffer representation
      points[ i ].x  = pts[ 2*i     ];
      points[ i ].y  = pts[ 2*i + 1 ];
      points[ i ].z  = 0.0;
      points[ i ].ID = i;
      points[ i ].mark_flag = 0;
    }
  
  x_rng = x_hi - x_lo;
  y_rng = y_hi - y_lo;
  
  // expand table to allow for points to be shifted around near
  // neighborhoods.  This involves updating some values.
  x_lo = x_lo - x_rng / table_dim;
  x_hi = x_hi + x_rng / table_dim;
  
  y_lo = y_lo - y_rng / table_dim;
  y_hi = y_hi + y_rng / table_dim;
  
  x_rng = x_hi - x_lo;
  y_rng = y_hi - y_lo;

  table_dim = table_dim + 2;

  x_segment = x_rng / ( table_dim - 1 );
  y_segment = y_rng / ( table_dim - 1 );

  // Build the table
  BuildTable( );  
}                                        // end constructor

DistanceTable::DistanceTable( float grid_size, int n, Point * pts )
  //   Distance Table Constructor
  // 
  // 
  //   grid_size :  specifies the resolution (number of buckets) in
  //                the distance table.  This ensures that any two points
  //                that are at most grid_size units of distance away from
  //                one another either lay in the same bucket or an adjacent
  //                bucket
  //   n         :  number of points to add to table
  //   pts       :  buffer holding points to add to table. This buffer is 
  //                copied into an internal buffer.
{
  int i;
 
  pts_n           = n;                      // update class var
  pts_max         = CEIL( 2*pts_n );        // set max pts to 2*pts_n
  points          = new Point[ pts_max ];   // allocate Point buffer
  avail_list.next = 0;                      // no IDs are available at startup
  DEBUG           = 0;                      // default value for DEBUG is off
  REPORT_ERROR    = 1;                      // default value for REPORT_ERROR 
  
  x_hi = y_hi = -10000.0;            // set initial boundaries
  x_lo = y_lo =  10000.0;            // set initial boundaries

  // scan the point array to determine the range of coordinate values
  for(i = 0; i < pts_n; i++)
    {
      if( pts[ i ].x < x_lo )
	x_lo = pts[ i ].x;
      if( pts[ i ].x > x_hi )
	x_hi = pts[ i ].x;
      
      if( pts[ i ].y < y_lo )
	y_lo = pts[ i ].y;
      if( pts[ i ].y > y_hi )
	y_hi = pts[ i ].y;
      // record points into the internal class Point buffer
      points[ i ] = pts[ i ];
    }
  
  x_rng = x_hi - x_lo;
  y_rng = y_hi - y_lo;

  // expand table to allow for points to be shifted around near
  // neighborhoods, specifically a new perimeter of grids.  This
  // involves updating some values
  x_lo = x_lo - grid_size;
  x_hi = x_hi + grid_size;
  
  y_lo = y_lo - grid_size;
  y_hi = y_hi + grid_size;
  
  x_rng = x_hi - x_lo;
  y_rng = y_hi - y_lo;

  if( x_rng < y_rng )
    table_dim = CEIL( x_rng / grid_size );
  else
    table_dim = CEIL( y_rng / grid_size );
  
  x_segment = x_rng / ( table_dim - 1 );
  y_segment = y_rng / ( table_dim - 1 );

  // build the table
  BuildTable( );
}                                       // end constructor

DistanceTable::DistanceTable( float grid_size, int n, float * pts )
  //   Distance Table Constructor
  // 
  // 
  //   grid_size :  specifies the resolution (number of buckets) in
  //                the distance table.  This ensures that any two points
  //                that are at most grid_size units of distance away from
  //                one another either lay in the same bucket or an adjacent
  //                bucket
  //   n         :  number of points to add to table
  //   pts       :  buffer holding float representation of points to add 
  //                to table. Points are 2D. This buffer is  copied into an 
  //                internal buffer.
{
  int i;
  
  pts_n           = n;                      // update class var
  pts_max         = CEIL( 2*pts_n );        // set max pts to 2*pts_n
  points          = new Point[ pts_max ];   // allocate Point buffer
  avail_list.next = 0;                      // no IDs are available at startup
  DEBUG           = 0;                      // default value for DEBUG is off
  REPORT_ERROR    = 1;                      // default value for REPORT_ERROR 
  
  x_hi = y_hi = -10000.0;            // set initial boundaries
  x_lo = y_lo =  10000.0;            // set initial boundaries
  
  // scan the point array to determine the range of coordinate values
  for(i = 0; i < pts_n; i++)
    {
      if( pts[ 2*i ] < x_lo )
	x_lo = pts[ 2*i ];
      if( pts[ 2*i ] > x_hi )
	x_hi = pts[ 2*i ];
      
      if( pts[ 2*i + 1 ] < y_lo )
	y_lo = pts[ 2*i + 1 ];
      if( pts[ 2*i + 1 ] > y_hi )
	y_hi = pts[ 2*i + 1 ];
      
      // record points into the internal class Point buffer
      points[ i ].x  = pts[ 2*i     ];
      points[ i ].y  = pts[ 2*i + 1 ];
      points[ i ].z  = 0.0;
      points[ i ].ID = i;
      points[ i ].mark_flag = 0;
    }
  
  x_rng = x_hi - x_lo;
  y_rng = y_hi - y_lo;
  
  // expand table to allow for points to be shifted around near
  // neighborhoods. This involves updating some values
  x_lo = x_lo - grid_size;
  x_hi = x_hi + grid_size;
  
  y_lo = y_lo - grid_size;
  y_hi = y_hi + grid_size;
  
  x_rng = x_hi - x_lo;
  y_rng = y_hi - y_lo;

  if( x_rng < y_rng )
    table_dim = CEIL( x_rng / grid_size );
  else
    table_dim = CEIL( y_rng / grid_size );
  
  x_segment = x_rng / ( table_dim - 1 );
  y_segment = y_rng / ( table_dim - 1 );

  // build the table
  BuildTable( );
}                                    // end constructor

DistanceTable::~DistanceTable( )
  //
  //   Destructor function
{
  EmptyTable();

  delete [] points;
  
  DTAvailNode *ptr1, *ptr2;

  // delete avail list
  if( avail_list.next )
    {
      ptr1 = avail_list.next;
      while( ptr1 )
	{
	  ptr2 = ptr1;
	  delete ptr2;
	  ptr1 = ptr1->next;
	}
    }
}                                  // end destructor

int DistanceTable::AddPoint( Point & p )
  //  Inserts a point to the distance table;  returns the ID of the
  //  point in the table if the insertion is successful
  //
  //
  //  p   : A reference to a Point instance
{
  // verify point lies in the appropiate bounding box
  // represented by this table if not reject point
  if( p.x < x_lo || p.y < y_lo ||
      p.x > x_hi || p.y > y_hi )
    {
      if( REPORT_ERROR )
	{
	  printf("ERROR: Attempted to add a point outside of the table's");
	  printf(" allowed ranges: [%f, %f]", p.x, p.y);
	  printf(" into [%f, %f]x[%f, %f]\n", x_lo, y_lo, x_hi, y_hi);
	}
      return 0;
    }

  int j;
  // if the insertion of this point exceeds the maximum number of allowed
  // points then we need to extend the available storage space
  if( pts_n + 1 == pts_max )
    {
      ExtendPointArray();
    }

  // get new point ID
  int new_pt_ID; 

  // check avail list first for any available old point IDs
  if( avail_list.next )
    {
      // get first ID on avail_list
      new_pt_ID = avail_list.next->avail_ID;

      DTAvailNode * curr = avail_list.next;
      avail_list.next = curr->next;
      // deallocate node
      delete curr;
    }
  else
    {
      // next available ID = pts_n (in zero based indices)
      new_pt_ID = pts_n; 
    }
  
  // record new point in point array
  points[ new_pt_ID ]    = p;
  points[ new_pt_ID ].ID = new_pt_ID;
  pts_n                  = pts_n + 1;  // incrememnt the number of points
  
  // Add point to the hash table
  int hash_x, hash_y;
  // compute the appropriate hash values
  if( points[ new_pt_ID ].x == x_hi )
    hash_x = table_dim - 1;
  else
    hash_x = FLOOR( (points[ new_pt_ID ].x - x_lo) / x_segment );
  
  if( points[new_pt_ID].y == y_hi )
    hash_y = table_dim - 1;
  else
    hash_y = FLOOR( (points[ new_pt_ID ].y - y_lo) / y_segment );

  if( points[new_pt_ID].x == x_hi )
    hash_x = table_dim - 1;
  else
    hash_x = FLOOR( (points[ new_pt_ID ].x - x_lo) / x_segment );
  
  if( points[new_pt_ID].y == y_hi )
   hash_y = table_dim - 1;
  else
    hash_y = FLOOR( (points[ new_pt_ID ].y - y_lo) / y_segment );
      
  // double the size of a bucket if necessary
  if( counts[ hash_x ][ hash_y ] == max_cnts[ hash_x ][ hash_y ] )
    {
      // allocate a new bucket
      int * temp   = new int[ 2*max_cnts[ hash_x ][ hash_y ] ];
      int max_size = max_cnts[ hash_x ][ hash_y ];
      // re-write the old array to the new one
      for( j = 0; j < max_size; j++ )
        temp[ j ] = table[ hash_x ][ hash_y ][ j ];
      // de-allocate memory
      delete [] table[ hash_x ][ hash_y ];
      // update pointer in table to point to new bucket
      table[ hash_x ][ hash_y ] = temp;
      // update the maximum size of this bucket
      max_cnts[ hash_x ][ hash_y ] = 2*max_cnts[ hash_x ][ hash_y ];
      temp = NULL;
    }
  
  // record the point in a bucket
  table[ hash_x ][ hash_y ][ counts[ hash_x ][ hash_y ] ] = new_pt_ID;
  // update the current size of the bucket
  counts[ hash_x ][ hash_y ] = counts[ hash_x ][ hash_y ] + 1;
  
  // return the point's ID 
  return new_pt_ID;
}                                          // end AddPoint( )

void DistanceTable::BuildTable( )
  // Internal routine that adds all points in the internal point buffer
  // to their appropriate buckets in the hash table. 
{
  int i, j;
  // Create the table as a 2D array of int pointers 
  table = new int **[ table_dim ];
  for( i = 0; i < table_dim; i++)
    table[ i ] = new int *[ table_dim ];

  // Create count arrays
  counts   = new int *[ table_dim ];  // current bucket size
  max_cnts = new int *[ table_dim ];  // current maximum bucket size
  
  for( i = 0; i < table_dim; i++)
    {
      counts[ i ]   = new int[ table_dim ];
      max_cnts[ i ] = new int[ table_dim ];
    }

  // enter the initial values for the tables
  for( i = 0; i < table_dim; i++)
    for( j = 0; j < table_dim; j++)
      {
        table[ i ][ j ]      = new int[ DT_BUCKET_MAX_SIZE ];
	counts[ i ][ j ]     = 0;
        max_cnts[ i ][ j ]   = DT_BUCKET_MAX_SIZE;
      }
  
  int hash_x, hash_y;  // hash variables used to find appropriate bucket

  // add each point to the hash table and increment 
  // the bucket counts accordingly
  for( i = 0; i < pts_n; i++ )
    {
      // compute the hash values for the current point
      if( points[ i ].x == x_hi )
	hash_x = table_dim - 1;
      else
	hash_x = FLOOR( (points[ i ].x - x_lo) / x_segment );
      
      if( points[ i ].y == y_hi )
	hash_y = table_dim - 1;
      else
	hash_y = FLOOR( (points[ i ].y - y_lo) / y_segment );
      // double the size of a bucket if necessary
      if( counts[ hash_x ][ hash_y ] == max_cnts[ hash_x ][ hash_y ] )
        {
	  // create a new bucket twice as large as the previous one
          int * temp   = new int[ 2*max_cnts[ hash_x ][ hash_y ] ];
          int max_size = max_cnts[ hash_x ][ hash_y ];
	  // record the contents of the old bucket
          for( j = 0; j < max_size; j++ )
            temp[ j ] = table[ hash_x ][ hash_y ][ j ];
	  // de-allocate the old bucket
          delete [] table[ hash_x ][ hash_y ];
	  // add the new bucket to the hash table
          table[ hash_x ][ hash_y ] = temp;
	  // update the maximum bucket size
          max_cnts[ hash_x ][ hash_y ] = 2*max_cnts[ hash_x ][ hash_y ];
          temp = NULL;
        }
      
      // record the point in a bucket
      table[ hash_x ][ hash_y ][ counts[ hash_x ][ hash_y ] ] = i;
      // update the current size of the bucket
      counts[ hash_x ][ hash_y ] = counts[ hash_x ][ hash_y ] + 1;
    }
}                                            // end BuildPoint( )

void DistanceTable::EmptyTable( )
  //  Internal routine used to de-allocate the hash table of points
  //  and the tables used to store the maximum and current bucket sizes
{
  int i, j;

  DTAvailNode *ptr1, *ptr2;

  // de-allocate the tables

  for( i = 0; i < table_dim; i++)
    for( j = 0; j < table_dim; j++)
      delete [] table[ i ][ j ];

  for( i = 0; i < table_dim; i++)
    {
      delete [] max_cnts[ i ];
      delete [] table[ i ];
      delete [] counts[ i ];
    }

  delete [] table;
  delete [] max_cnts;
  delete [] counts;

  // de-allocate the availability list of available point IDs
  ptr1 = avail_list.next;
  while( ptr1 )
    {
      ptr2 = ptr1;
      ptr1 = ptr1->next;
      delete ptr2;
    }
}                                                // end EmptyTable( )

void DistanceTable::ExtendPointArray( )
  //  Internal routine used to increase the size of the internal Point
  //  buffer.  The default increase in size is 50% of the old array
{
  int i;

  int new_max       = CEIL( 1.5*pts_max );         // compute new max
  Point * new_array = new Point[ new_max ];        // allocate new Point buffer

  // record old point array in new larger point array
  for( i = 0; i < pts_max; i++)
    new_array[ i ] = points[ i ];
  // record new max value
  pts_max = new_max;
  // deallocate old array
  delete [] points;
  // update points array pointer
  points = new_array;
}                                               // end ExtendPointArray( )

inline float DistanceTable::FindNearestNeighbor( float * pnt_buff, 
						 int & nearest_neigh,
						 int EXISTS )
  // Internal routine that finds the closest neighbor in the distance table
  // to a given point.  Returns the distance from the target point to
  // its nearest neighbor. 
  //
  //
  // pnt_buff      : 2D float buffer used to represent a point
  // nearest_neigh : write the ID of the nearest neighbor to here
{
  int   i, z;               // itr vars
  int   count = 0;          // var used to record number of possible nearest
                            // neighbors
  float n_radius = FLT_MAX; // distance from target point to nearest neighbor

  int   hash_x, hash_y;  // hash vars

  float P_X = pnt_buff[ 0 ];  // record the point's X value
  float P_Y = pnt_buff[ 1 ];  // record the point's Y value
  // find the keys or hash value for this point
  if( P_X >= x_hi )
    hash_x = table_dim - 1;
  else if( P_X <= x_lo )
    hash_x = 0;
  else
    hash_x = FLOOR( (P_X - x_lo) / x_segment );
  
  if( P_Y >= y_hi )
    hash_y = table_dim - 1;
  else if( P_Y <= y_lo )
    hash_y = 0;
  else
    hash_y = FLOOR( (P_Y - y_lo) / y_segment );

  // the list of buckets to search for the nearest neighbor in
  int zones[ 9 ][ 2 ] = { { hash_x - 1, hash_y + 1 }, 
			  { hash_x    , hash_y + 1 },       
			  { hash_x + 1, hash_y + 1 }, 
			  { hash_x - 1, hash_y     },       
			  { hash_x    , hash_y     }, 
			  { hash_x + 1, hash_y     },       
			  { hash_x - 1, hash_y - 1 }, 
			  { hash_x    , hash_y - 1 },       
			  { hash_x + 1, hash_y - 1 } };
  
  int curr = 0;   // var used to store the current nearest neighbor
  int bucket_cnt; // var used to hold number of points in current bucket
  float d_x, d_y, temp; // distance vars
  
  count = 0;  // init count to zero

  int R, C;   // vars used to hold the row and col indices of current bucket

  // for each zone (bucket) search for the nearest neighbor
  for( z = 0; z < 9; z++ ){
      // ensure valid zone to search
      if( zones[ z ][ 0 ] > -1 && zones[ z ][ 0 ] < table_dim &&
	      zones[ z ][ 1 ] > -1 && zones[ z ][ 1 ] < table_dim ){
          if( DEBUG ){
	          printf("Accessing zone [%d][%d]\n", zones[z][0], zones[z][1]);
	        }
          R = zones[ z ][ 0 ];  // get this zone's row index
          C = zones[ z ][ 1 ];  // get this zone's col index
          bucket_cnt = counts[ R ][ C ]; // get this zone's bucket size
	  
          // for each point in the bucket see if it is the nearest neighbor
          for( i = 0; i < bucket_cnt; i++ ){
            curr = table[ R ][ C ][ i ]; // get the ID of this point
            if( DEBUG ){
		          printf("  Checking point %4d [%f, %f]  ", curr,
			        points[ curr ].x, points[ curr ].y);
		        }
	  
	          d_x = points[ curr ].x - P_X;  // compute d_x
	          d_y = points[ curr ].y - P_Y;  // compute d_y
    	      
	          temp = d_x*d_x + d_y*d_y;  // compute the distance as a squared
	                                    // sum
	          if(DEBUG)
		          printf("d = %f\n", temp);

	          // if this point's distance to the target is less than
	          // the current minimum, store it as the new nearest neighbor
            if( EXISTS ){ // check to see if we want to include an exact match
              if( temp > 0.0 && temp < n_radius ){
		            // valid neighbor within range
		            n_radius             = temp;
		            sort_array[ 0 ].ID   = curr;  // only use the first entry
		            sort_array[ 0 ].dist = temp;  // in the sort_array
		            count++;  // increment number of points processed
		          }
            }else if( temp >= 0.0 && temp < n_radius ){
		          // valid neighbor within range
		          n_radius             = temp;
		          sort_array[ 0 ].ID   = curr;  // only use the first entry
		          sort_array[ 0 ].dist = temp;  // in the sort_array
		          count++;  // increment number of points processed
		        }
          } // end for bucket_cnt
        }
  }// end for each zone

  // if count != 0 then a neighbor was found
  if( count )
    {
      // write the return values and return
      nearest_neigh = sort_array[ 0 ].ID;
      return sort_array[ 0 ].dist;
    }
  else  // no neighbor was found
    {
      // write the failed return values and return
      nearest_neigh = -1;
      return 0.0;
    }
}                                             // end FindNearestNeighbor( )

inline float DistanceTable::FindNeighbors( float * pnt_buff, int max_count, 
					   float threshold, int & n_neigh, 
					   int * array, int BUFF_SIZE, 
					   int EXISTS )

  //  Internal routine used to determine the points that are within a 
  //  user-defined distance of a specified point.  If threshold is larger 
  //  than the size of space represented by each bucket, then this routine 
  //  can not guarantee to find all points that are within the distance 
  //  threshold from the specified point.  Returns the distance from the 
  //  furtherest neighbor.
  //
  //
  //  pnt_buff   :  specifies the point as a 2D float buffer
  //                for whom to find the nearest neighbors of 
  //  max_count  :  places an upper bound on the number of neighbors to find.
  //                if not 0, then it finds the k-nearest neighbors with
  //                k = max_count
  //  threshold  :  points must be closer than this distance to the target 
  //                point to be considered a neighbor
  //  n_neigh    :  write the number of neighbors found here
  //  array      :  buffer used to write the IDs of the found neighbors
  //  BUFF_SIZE  :  maximum size of the buffer array
{
  int   i, j, z;      // itr vars
  int   count = 0;    // neighbor count var
  float n_radius;     // variable of the distance between a neighbor and the
                      // target point 

  // distance computations are done without a sqrt()
  float max_dist = threshold*threshold;  // maximum distance allowed
  
  int   hash_x, hash_y;   // hash vars

  float P_X = pnt_buff[ 0 ];
  float P_Y = pnt_buff[ 1 ];
  // compute the hash values for this point
  if( P_X >= x_hi )
    hash_x = table_dim - 1;
  else if( P_X <= x_lo )
    hash_x = 0;
  else
    hash_x = FLOOR( (P_X - x_lo) / x_segment );
  
  if( P_Y >= y_hi )
    hash_y = table_dim - 1;
  else if( P_Y <= y_lo )
    hash_y = 0;
  else
    hash_y = FLOOR( (P_Y - y_lo) / y_segment );

  // the list of buckets to search in for possible neighbors
  int zones[ 9 ][ 2 ] = { { hash_x - 1, hash_y + 1 }, 
			  { hash_x    , hash_y + 1 },       
			  { hash_x + 1, hash_y + 1 }, 
			  { hash_x - 1, hash_y     },       
			  { hash_x    , hash_y     }, 
			  { hash_x + 1, hash_y     },       
			  { hash_x - 1, hash_y - 1 }, 
			  { hash_x    , hash_y - 1 },       
			  { hash_x + 1, hash_y - 1 } };

  int curr = 0;         // ID of the current point
  int bucket_cnt;       // var used to store bucket counts
  float d_x, d_y, temp; // distance vars

  count = 0;    // initialize found neighbors count to 0
  int R, C;     // vars used to store current row and col indices

  // search each zone(bucket) for neighbors
  for( z = 0; z < 9; z++)
    {
      // ensure valid zone to search
      if( zones[ z ][ 0 ] > -1 && zones[ z ][ 0 ] < table_dim &&
	  zones[ z ][ 1 ] > -1 && zones[ z ][ 1 ] < table_dim )
	{
	  if( DEBUG )
	    {
	      printf("Accessing zone [%d][%d]\n", zones[z][0], zones[z][1]);
	    }

          R = zones[ z ][ 0 ];   // get this zone's row
          C = zones[ z ][ 1 ];   // get this zone's col

          bucket_cnt = counts[ R ][ C ];  // get this zone's bucket count

	  // search this bucket for possible neighbors
	  for( i = 0; i < bucket_cnt; i++ )
	    {
              curr = table[ R ][ C ][ i ];  // get a point
	      if( DEBUG )
		{
		  printf("  Checking point %4d [%f, %f]  ", curr,
			 points[ curr ].x, points[ curr ].y);
		}
	  
	      d_x = points[ curr ].x - P_X; // compute d_x
	      d_y = points[ curr ].y - P_Y; // compute d_y
	      
	      temp = d_x*d_x + d_y*d_y;  // distance from target to this
	                                 // point as a squared sum
	      if( DEBUG) 
		printf("d = %f\n", temp);

	      // if temp is below the distance threshold record it
	      if( EXISTS )
		{
		  if( temp > 0.0 && temp < max_dist )
		    {
		      // val
		      sort_array[ count ].ID   = curr;
		      sort_array[ count ].dist = temp;
		      count++; // increase number of found neigbors
		      if( count > DT_BUFFER_N ) // check for buffer overflow
			{
			  printf("ERROR: Internal buffer overflow in ");
			  printf("DistanceTable:FindNeighbors()\n");
			}
		    }
		}
	      else
		{
		  if( temp >= 0.0 && temp < max_dist )
		    {
		      // valid neighbor within range so add it to buffer
		      sort_array[ count ].ID   = curr;
		      sort_array[ count ].dist = temp;
		      count++; // increase number of found neigbors
		      if( count > DT_BUFFER_N ) // check for buffer overflow
			{
			  printf("ERROR: Internal buffer overflow in ");
			  printf("DistanceTable:FindNeighbors()\n");
			}
		    }
		}
	    }
	}
    }

  // the buffer sort_array now holds all neighbors within threshold
  // Also, remember zones may have contained the point whose neighbors 
  // we are searching for, therefore remove him from the candidates

  // if max_count is not zero, then we only want to take the k-closest
  // neighbors with k = max_count
  if( count > max_count && max_count != 0)
    count = max_count;
  
  DTDNode curr_node;
  
  int largepos;
  
  if( count > 0 )
    {
      // if we want to sort the neighbors from closest to furtherest
      // then do it here
      if( SORT_NEIGHBORS ) 
        {
          // use a simple selection sort to sort the neighbors
          // from closest to furtherest
          for( i = count - 1; i > 0; i-- )
            {
              curr_node = sort_array[ i ];
              largepos = 0;
              for( j = 1; j <= i; j++ )
                {
                  if( sort_array[ largepos ].dist < sort_array[ j ].dist )
                    largepos = j;
                }
              if( largepos != i )
                {
                  curr_node              = sort_array[ i ];
                  sort_array[ i ]        = sort_array[ largepos ];
                  sort_array[ largepos ] = curr_node;
                }
            } 
          // returning the maximum distance from the target to one of its
	  // neighbors
          n_radius = sort_array[ count - 1 ].dist;
        }
      else
        {
	  // do not sort the neighbors, just find the furtherest
	  // distance from the target point to one of its neighbors
          n_radius = sort_array[ 0 ].dist;
          for( i = 1; i < count; i++ )
            {
              if( n_radius < sort_array[ i ].dist )
                n_radius = sort_array[ i ].dist;
            }
        }

      // check for buffer overflow from internal sort_array to user
      // supplied buffer to write the results to
      if( count > BUFF_SIZE )
	printf("ERROR: Buffer overflow: DistanceTable::FindNeighbors()!\n");
      // put the results in the user supplied buffer
      for( i = 0; i < count; i++ )
        array[ i ] = sort_array[ i ].ID;
    }
  else
    {
      // failed return values
      count    = 0;
      n_radius = 0.0;
    }
  // set the return values and return 
  n_neigh = count;  // write the number of neighbors found
  return n_radius;
}                                             // end FindNeighbors( )

float DistanceTable::GetNearestNeighbor( float * pnt_buff, 
					 int & nearest_neigh )
  // Internal routine that finds the closest neighbor in the distance table
  // to a given point omitting distances == 0.0.  Returns the distance from 
  // the target point to its nearest neighbor. 
  //
  //
  // pnt_buff      : 2D float buffer used to represent a point
  // nearest_neigh : write the ID of the nearest neighbor to here
{
  // call internal routine for finding the nearest neighbor
  return FindNearestNeighbor( pnt_buff, nearest_neigh, 1 );
}                                           // end GetNearestNeighbor( )

float DistanceTable::GetNearestNeighbor( Point p, int & nearest_neigh )
  // Internal routine that finds the closest neighbor in the distance table
  // to a given point omitting distances == 0.0.   Returns the distance 
  // from the target point to its nearest neighbor. 
  //
  //
  // p             : a point instance for whom to find its nearest neighbor
  // nearest_neigh : write the ID of the nearest neighbor to here
{
  float pnt_buff[ 3 ];  // make a buffer format for internal routine
  pnt_buff[ 0 ] = p.x;
  pnt_buff[ 1 ] = p.y;
  pnt_buff[ 2 ] = p.z;
  // call internal routine for finding the nearest neighbor
  return FindNearestNeighbor( pnt_buff, nearest_neigh, 1 );
}                                           // end GetNearestNeighbor( )

float DistanceTable::GetNearestNeighbor_NonMember( float * pnt_buff, 
						   int & nearest_neigh )
  // Internal routine that finds the closest neighbor in the distance table
  // to the given point.  Returns the distance from the target point to
  // its nearest neighbor.  This function includes distances == 0.0
  //
  //
  // pnt_buff      : 2D float buffer used to represent a point
  // nearest_neigh : write the ID of the nearest neighbor to here
{
  // call internal routine for finding the nearest neighbor
  return FindNearestNeighbor( pnt_buff, nearest_neigh, 0 );
}                                        // end GetNearestNeighbor_NonMember( )

float DistanceTable::GetNearestNeighbor_NonMember( Point p, 
						   int & nearest_neigh )
  // Internal routine that finds the closest neighbor in the distance table
  // to the given point.  Returns the distance from the target point to
  // its nearest neighbor.  This function includes distances == 0.0
  //
  //
  // p             : a point instance for whom to find its nearest neighbor
  // nearest_neigh : write the ID of the nearest neighbor to here
{
  float pnt_buff[ 3 ];  // make a buffer format for internal routine
  pnt_buff[ 0 ] = p.x;
  pnt_buff[ 1 ] = p.y;
  pnt_buff[ 2 ] = p.z;
  // call internal routine for finding the nearest neighbor
  return FindNearestNeighbor( pnt_buff, nearest_neigh, 0 );
}                                        // end GetNearestNeighbor_NonMember( )

float DistanceTable::GetNeighbors( int WHICH, int max_count, float threshold, 
				   int & n_neigh, int * array, int BUFF_SIZE )
  //  Determine the points that are within a user-defined distance of
  //  a specified point.  If threshold is larger than the size of space
  //  represented by each bucket, then this routine can not guarantee to find
  //  all points that are within the distance threshold from the specified 
  //  point.  Returns the distance from the furtherest neighbor.  This 
  //  function omits distances == 0.0
  //
  //
  //  WHICH      :  specifies the point ID of the point in the distance table
  //                for whom to find the nearest neighbors of 
  //  max_count  :  places an upper bound on the number of neighbors to find.
  //                if not 0, then it finds the k-nearest neighbors with
  //                k = max_count
  //  threshold  :  points must be closer than this distance to the target 
  //                point to be considered a neighbor
  //  n_neigh    :  write the number of neighbors found here
  //  array      :  buffer used to write the IDs of the found neighbors
  //  BUFF_SIZE  :  maximum size of the buffer array
{

  // ensure that the target point is a valid index value
  if( WHICH < 0 || WHICH > pts_max )
    {
      if( REPORT_ERROR )
	{
	  printf("ERROR: Illegal Index in DistanceTable::GetNeighbors() ");
	  printf(": %d\n", WHICH);
	}
      n_neigh = 0;
      return 0.0;
    }
  // record the Point value as a float buffer for internal routine
  float pnt_buff[ 3 ];
  pnt_buff[ 0 ] = points[ WHICH ].x;
  pnt_buff[ 1 ] = points[ WHICH ].y;
  pnt_buff[ 2 ] = 0.0;
  

  // call the internal routine for finding the neighbors
  return FindNeighbors( pnt_buff, max_count, threshold, n_neigh, array, 
			BUFF_SIZE, 1 );

}                                                     // end GetNeigbors( )

float DistanceTable::GetNeighbors( float * pnt_buff, int max_count, 
				   float threshold, int & n_neigh, int * array,
				   int BUFF_SIZE )
  //  Determine the points that are within a user-defined distance of
  //  a specified point.  If threshold is larger than the size of space
  //  represented by each bucket, then this routine can not guarantee to find
  //  all points that are within the distance threshold from the specified 
  //  point.  Returns the distance from the furtherest neighbor. This
  //  function omits distances == 0.0
  //
  //  pnt_buff   :  specifies the point as a 2D float buffer
  //                for whom to find the nearest neighbors of 
  //  max_count  :  places an upper bound on the number of neighbors to find.
  //                if not 0, then it finds the k-nearest neighbors with
  //                k = max_count
  //  threshold  :  points must be closer than this distance to the target 
  //                point to be considered a neighbor
  //  n_neigh    :  write the number of neighbors found here
  //  array      :  buffer used to write the IDs of the found neighbors
  //  BUFF_SIZE  :  maximum size of the buffer array
{
  // call the internal routine for finding the neighbors

  // call the internal routine for finding the neighbors
  return FindNeighbors( pnt_buff, max_count, threshold, n_neigh, array,
			BUFF_SIZE, 1 );
}                                                     // end GetNeigbors( )

float DistanceTable::GetNeighbors( Point p, int max_count, 
				   float threshold, int & n_neigh, int * array,
				   int BUFF_SIZE )
  //  Determine the points that are within a user-defined distance of
  //  a specified point.  If threshold is larger than the size of space
  //  represented by each bucket, then this routine can not guarantee to find
  //  all points that are within the distance threshold from the specified 
  //  point.  Returns the distance from the furtherest neighbor. This
  //  function omits distances == 0.0
  //
  //  p          :  specifies the point as a Point object
  //                for whom to find the nearest neighbors of 
  //  max_count  :  places an upper bound on the number of neighbors to find.
  //                if not 0, then it finds the k-nearest neighbors with
  //                k = max_count
  //  threshold  :  points must be closer than this distance to the target 
  //                point to be considered a neighbor
  //  n_neigh    :  write the number of neighbors found here
  //  array      :  buffer used to write the IDs of the found neighbors
  //  BUFF_SIZE  :  maximum size of the buffer array
{
  float pnt_buff[ 3 ];
  pnt_buff[ 0 ] = p.x;
  pnt_buff[ 1 ] = p.y;
  pnt_buff[ 2 ] = p.z;

  // call the internal routine for finding the neighbors
  return FindNeighbors( pnt_buff, max_count, threshold, n_neigh, array,
			BUFF_SIZE, 1 );
}                                                     // end GetNeigbors( )

float DistanceTable::GetNeighbors_NonMember( float * pnt_buff, int max_count, 
					     float threshold, int & n_neigh, 
					     int * array, int BUFF_SIZE )
  //  Determine the points that are within a user-defined distance of
  //  a specified point.  If threshold is larger than the size of space
  //  represented by each bucket, then this routine can not guarantee to find
  //  all points that are within the distance threshold from the specified 
  //  point.  Returns the distance from the furtherest neighbor. This
  //  function includes distances == 0.0
  //
  //  pnt_buff   :  specifies the point as a 2D float buffer
  //                for whom to find the nearest neighbors of 
  //  max_count  :  places an upper bound on the number of neighbors to find.
  //                if not 0, then it finds the k-nearest neighbors with
  //                k = max_count
  //  threshold  :  points must be closer than this distance to the target 
  //                point to be considered a neighbor
  //  n_neigh    :  write the number of neighbors found here
  //  array      :  buffer used to write the IDs of the found neighbors
  //  BUFF_SIZE  :  maximum size of the buffer array
{
  // call the internal routine for finding the neighbors

  // call the internal routine for finding the neighbors
  return FindNeighbors( pnt_buff, max_count, threshold, n_neigh, array,
			BUFF_SIZE, 1 );
}                                               // end GetNeigbors_NonMember( )

float DistanceTable::GetNeighbors_NonMember( Point p, int max_count, 
					     float threshold, int & n_neigh, 
					     int * array, int BUFF_SIZE )
  //  Determine the points that are within a user-defined distance of
  //  a specified point.  If threshold is larger than the size of space
  //  represented by each bucket, then this routine can not guarantee to find
  //  all points that are within the distance threshold from the specified 
  //  point.  Returns the distance from the furtherest neighbor. This
  //  function includes distances == 0.0
  //
  //  p          :  specifies the point as a Point object
  //                for whom to find the nearest neighbors of 
  //  max_count  :  places an upper bound on the number of neighbors to find.
  //                if not 0, then it finds the k-nearest neighbors with
  //                k = max_count
  //  threshold  :  points must be closer than this distance to the target 
  //                point to be considered a neighbor
  //  n_neigh    :  write the number of neighbors found here
  //  array      :  buffer used to write the IDs of the found neighbors
  //  BUFF_SIZE  :  maximum size of the buffer array
{
  float pnt_buff[ 3 ];
  pnt_buff[ 0 ] = p.x;
  pnt_buff[ 1 ] = p.y;
  pnt_buff[ 2 ] = p.z;

  // call the internal routine for finding the neighbors
  return FindNeighbors( pnt_buff, max_count, threshold, n_neigh, array,
			BUFF_SIZE, 1 );
}                                               // end GetNeigbors_NonMember( )



Point DistanceTable::GetPoint( int WHICH )
  //  Routine used to get a copy of the point specified by WHICH
  //
  //
  //  WHICH : ID of the target point
{
  // ensure that the index provided is valid
  if( WHICH < 0 || WHICH >= pts_max )
    {
      if( REPORT_ERROR )
	{
	  printf("ERROR: Illegal index in DistanceTable::GetPoint(): %d\n", 
		 WHICH );
	}
      // if this index value is invalid, return nothing
      Point dead_pt;
      return dead_pt;
    }
  else if( points[ WHICH ].ID == -1 ) 
    {
      // this point is no longer stored in the internal point buffer
      if( REPORT_ERROR )
	{
	  printf("ERROR: Illegal index in DistanceTable::GetPoint(): ");
	  printf("%d\t Invalid point\n", WHICH);
	}
      // return a dead point
      Point dead_pt;
      return dead_pt;
    }
  else // return the point located at the index WHICH 
    return points[ WHICH ];
}                                                 // end GetPoint( )

Point * DistanceTable::GetPointBuffer()
  //  Routine that provides access to the internal point buffer
{
  return points;
}                                                 // end GetPointBuffer( )

void DistanceTable::Hash( int WHICH, int & hash_x, int & hash_y )
  //  Internal routine used to compute the hash value for a given point
  //
  //
  // WHICH   :  Get the hash values for the point specified by this value
  // hash_x  :  Write the col hash for the specified point here
  // hash_y  :  Write the row hash for the specified point here
{
  if( points[ WHICH ].x == x_hi )
    hash_x = table_dim - 1;
  else
    hash_x = FLOOR( (points[ WHICH ].x - x_lo) / x_segment );
  
  if( points[ WHICH ].y == y_hi )
    hash_y = table_dim - 1;
  else
    hash_y = FLOOR( (points[ WHICH ].y - y_lo) / y_segment );
}                                                // end Hash( )

void DistanceTable::PrintTable( )
  //  Routine used to print information about the hash_table
{
  int i, j, k;
  printf("Table Range: [ %f, %f ]x[ %f, %f ]\n", x_lo, y_lo, x_hi, y_hi);
  printf("\ttable_dim = %d\n", table_dim );
  printf("\t[ x_segment, y_segment ] = [ %f, %f ]\n", x_segment, y_segment );
  printf("\tTOTAL POINTS = %d\n", pts_n );
  printf("\n");
  int curr;
  for( i = 0; i < table_dim; i++ )
    for( j = 0; j < table_dim; j++ )
      {
        printf("[%3d][%3d] = (%2d) ", i, j, counts[ i ][ j ] );
        int max_size = counts[ i ][ j ];
        for( k = 0; k < max_size; k++ )
          {
            curr = table[ i ][ j ][ k ];
            printf(" %d", curr);
          }
        printf("\n");
      }
  printf("\n");
}                                                // end PrintTable( )

float DistanceTable::RebuildTable( int new_dim )
  //  Routine called to rebuild the table with a specified number of 
  //  rows and cols
  //
  // 
  //  new_dim : number of new rows and cols to build the table with
{
  // empty the existing table
  EmptyTable();

  float new_resolution;   // var used to store the new table's resolution

  table_dim = new_dim;    // update class var

  x_segment = x_rng / ( table_dim - 1 );  // compute new x_segment
  y_segment = y_rng / ( table_dim - 1 );  // compute new y_segment

  // the new resolution of the table is the maximum of the x,y segments
  if( x_segment > y_segment )
    new_resolution = x_segment; 
  else
    new_resolution = y_segment;

  int i, j; // itr vars

  // Create the new table
  table = new int **[ table_dim ];
  for( i = 0; i < table_dim; i++)
    table[ i ] = new int *[ table_dim ];

  // Create new count arrays
  counts   = new int *[ table_dim ];
  max_cnts = new int *[ table_dim ];
  for( i = 0; i < table_dim; i++)
    {
      counts[ i ]   = new int[ table_dim ];
      max_cnts[ i ] = new int[ table_dim ];
    }

  // add empty buckets to the hash table and initialize the count tables
  for( i = 0; i < table_dim; i++)
    for( j = 0; j < table_dim; j++)
      {
        table[ i ][ j ]      = new int[ DT_BUCKET_MAX_SIZE ];
	counts[ i ][ j ]     = 0;
	max_cnts[ i ][ j ]   = DT_BUCKET_MAX_SIZE;
      }
  
  int hash_x, hash_y;   // hash vars

  // for each point in the internal buffer, check to see if the point
  // at each location in this buffer is valid and add it to the new table
  for( i = 0; i < pts_max; i++ )
    {
      if( points[ i ].ID != -1 ) // check point's validity
	{
	  // compute the new hash values for this point
          if( points[ i ].x == x_hi )
            hash_x = table_dim - 1;
          else
            hash_x = FLOOR( (points[ i ].x - x_lo) / x_segment );
      
          if( points[ i ].y == y_hi )
            hash_y = table_dim - 1;
          else
            hash_y = FLOOR( (points[ i ].y - y_lo) / y_segment );
          
          // double the size of a bucket if necessary
          if( counts[ hash_x ][ hash_y ] == max_cnts[ hash_x ][ hash_y ] )
            {
	      // create a new bucket
              int * temp   = new int[ 2*max_cnts[ hash_x ][ hash_y ] ];
              int max_size = max_cnts[ hash_x ][ hash_y ];
	      // write the old bucket's contents to the new bucket
              for( j = 0; j < max_size; j++ )
                temp[ j ] = table[ hash_x ][ hash_y ][ j ];
	      // de-allocate old bucket
              delete [] table[ hash_x ][ hash_y ];
	      // add new bucke to the hash_table
              table[ hash_x ][ hash_y ] = temp;
	      // update the maximum bucket size of this bucket
              max_cnts[ hash_x ][ hash_y ] = 2*max_cnts[ hash_x ][ hash_y ];
              temp = NULL;
            }
          
          // record the point in a bucket
          table[ hash_x ][ hash_y ][ counts[ hash_x ][ hash_y ] ] = i;
          // update the current size of the bucket
          counts[ hash_x ][ hash_y ] = counts[ hash_x ][ hash_y ] + 1;
	}
    }
  // return the spatial resolution of the new table
  return new_resolution;
}                                                 // end RebuildTable( int )


int DistanceTable::RebuildTable( float new_grid_size )
  //  Routine that rebuilds the distance table such that if any two 
  // points are within new_grid_size units of distance, then they lie 
  // in adjacent table entries.  Return the new table dimension (number of
  // rows and columns )
  //
  //
  // new_grid_size : the size of each bucket with respect to the units of
  //                 distance each bucket represents
{
  // de-allocate any existing table
  EmptyTable();
   
  // compute the new table dimension
  if( x_rng < y_rng )
    table_dim = CEIL( x_rng / new_grid_size );
  else
    table_dim = CEIL( y_rng / new_grid_size );

  x_segment = x_rng / (table_dim - 1);  // compute the new x_segmnet
  y_segment = y_rng / (table_dim - 1);  // compute the new y_segment
  
  int i, j;  // itr vars

  // Create the table
  table = new int **[ table_dim ];
  for( i = 0; i < table_dim; i++)
    table[ i ] = new int *[ table_dim ];

  // Create new count arrays
  counts   = new int *[ table_dim ];
  max_cnts = new int *[ table_dim ];
  for( i = 0; i < table_dim; i++)
    {
      counts[ i ]   = new int[ table_dim ];
      max_cnts[ i ] = new int[ table_dim ];
    }
  // add buckets to the new table and initialize the count tables
  for( i = 0; i < table_dim; i++)
    for( j = 0; j < table_dim; j++)
      {
        table[ i ][ j ]      = new int[ DT_BUCKET_MAX_SIZE ];
	counts[ i ][ j ]     = 0;
	max_cnts[ i ][ j ]   = DT_BUCKET_MAX_SIZE;
      }
  
  int hash_x, hash_y;    // hash vars

  // for each point in the internal buffer, check to see if the point
  // at each location in this buffer is valid and add it to the new table
  for( i = 0; i < pts_max; i++ )
    {
      if( points[ i ].ID != -1 )  // check point's validity
	{
	  // compute this point's hash values
          if( points[ i ].x == x_hi )
            hash_x = table_dim - 1;
          else
            hash_x = FLOOR( (points[ i ].x - x_lo) / x_segment );
      
          if( points[ i ].y == y_hi )
            hash_y = table_dim - 1;
          else
            hash_y = FLOOR( (points[ i ].y - y_lo) / y_segment );
          
          // double the size of a bucket if necessary
          if( counts[ hash_x ][ hash_y ] == max_cnts[ hash_x ][ hash_y ] )
            {
	      // allocate a new bucket
              int * temp   = new int[ 2*max_cnts[ hash_x ][ hash_y ] ];
              int max_size = max_cnts[ hash_x ][ hash_y ];
	      // write the old bucket's contents to the new bucket
              for( j = 0; j < max_size; j++ )
                temp[ j ] = table[ hash_x ][ hash_y ][ j ];
	      // de-allocate the old bucket
              delete [] table[ hash_x ][ hash_y ];
	      // add the new bucket to the table
              table[ hash_x ][ hash_y ] = temp;
	      // update the maximum bucket size for this bucket
              max_cnts[ hash_x ][ hash_y ] = 2*max_cnts[ hash_x ][ hash_y ];
              temp = NULL;
            }
          
          // record the point in a bucket
          table[ hash_x ][ hash_y ][ counts[ hash_x ][ hash_y ] ] = i;
          // update the current size of the bucket
          counts[ hash_x ][ hash_y ] = counts[ hash_x ][ hash_y ] + 1;
	}
    }
  // return the new table dimension
  return table_dim;
}                                                 // end RebuildTable( float )

int DistanceTable::RemovePoint( int WHICH )
  //  Routine used to remove a point specified by WHICH.  
  //
  //
  //  WHICH : the ID of the point to remove

{
  // ensure that WHICH is a valid index
  if( WHICH < 0 || WHICH >= pts_max)
    {
      if( REPORT_ERROR )
	{
	  printf("ERROR Illegal index for DistanceTable::RemovePoint() : ");
	  printf("%d\n", WHICH);
	}
      return 1;
    }
  if( points[ WHICH ].ID == -1 )
    {
      // ensure that WHICH actually holds a valid point
      if( REPORT_ERROR )
	{
	  printf("ERROR Illegal index for DistanceTable::RemovePoint() : ");
	  printf("%d Invalid point \n", WHICH);
	}
      return 1;
    }

  int   hash_x, hash_y;  // hash vars

  // represents an empty/dead point
  points[ WHICH ].ID = -1;

  // hash the existing point
  if( points[ WHICH ].x == x_hi )
    hash_x = table_dim - 1;
  else
    hash_x = FLOOR( (points[ WHICH ].x - x_lo) / x_segment );
  
  if( points[ WHICH ].y == y_hi )
    hash_y = table_dim - 1;
  else
    hash_y = FLOOR( (points[ WHICH ].y - y_lo) / y_segment );
  
  int i, curr;
  int found = 0;  // found flag
  int bucket_cnt = counts[ hash_x ][ hash_y ];  // number of points in bucket
  // list of points in zone must be searched, so iterate through bucket
  for( i = 0; i < bucket_cnt && !found; i++ )
    {
      curr = table[ hash_x ][ hash_y ][ i ];
      if( curr == WHICH ) // located node to be removed
	  found = 1;
    }

  if( !found ) // unable to find point, error message
    {
      if( REPORT_ERROR )
	{
	  printf("ERROR in RemovePoint():  Unable to locate point %d ", WHICH);
	  printf("in zone [%d, %d]\n", hash_x, hash_y );
	}
      return 0;
    }
  else
    {
      // remove point from bucket
      for( ; i < bucket_cnt; i++ )
        {
          table[ hash_x ][ hash_y ][ i - 1 ] = table[ hash_x ][ hash_y ][ i ];
        }
      // decrease bucket count
      counts[ hash_x ][ hash_y ] = counts[ hash_x ][ hash_y] - 1;
      
      // add ID to the avail_list
      DTAvailNode * new_node = new DTAvailNode;
      new_node->avail_ID     = WHICH;
      DTAvailNode * curr     = &avail_list;
      int stop               = 0;                // stop flag
      // find proper place in avail_list to add new ID
      while( curr->next && !stop )
	{
	  if( curr->next->avail_ID > WHICH )
	    stop = 1;
	  else
	    curr = curr->next;
	}
      // curr points to the node before the proper location of the new node
      // so insert new_node after curr
      new_node->next = curr->next;
      curr->next     = new_node;
      // decrement point count
      pts_n          = pts_n - 1;
      // return successful delete
      return 1;
    }
}                                                // end RemovePoint( )


int DistanceTable::UpdatePoint( int WHICH, Point & p )
  // Routine that changes the point at index WHICH to a user specified
  // point value.  Returns 1 if successful, else 0.
  //
  //
  // WHICH :  index of the point to change
  // p     :  new point instance to update old point with
{
  // ensure that WHICH is a valid index
  if( WHICH < 0 || WHICH >= pts_max)
    {
      if( REPORT_ERROR )
	{
	  printf("ERROR Illegal index for DistanceTable::UpdatePoint() : ");
	  printf("%d\n", WHICH);
	}
      return 0;
    }
  // ensure the point at index WHICH is a valid point
  if( points[ WHICH ].ID == -1 )
    {
      if( REPORT_ERROR )
	{
	  printf("ERROR Illegal index for DistanceTable::UpdatePoint() : ");
	  printf("%d Invalid Point\n", WHICH );
	}
      return 0;
    }
  
  // verify point lies in the appropiate bounding box
  // represented by this table, if not reject point
  if( p.x < x_lo || p.y < y_lo ||
      p.x > x_hi || p.y > y_hi )
    {
      if( REPORT_ERROR )
	{
	  printf("ERROR: Attempted to add a point outside of the table's");
	  printf(" allowed ranges: [%f, %f]", p.x, p.y);
	  printf(" into [%f, %f]x[%f, %f]\n", x_lo, y_lo, x_hi, y_hi);
	}
      return 0;
    }

  if( p.ID != WHICH ) // error message, a point at index WHICH should have 
                      // an ID == WHICH
    {
      if( REPORT_ERROR )
	{
	  printf("POTENTIAL ERROR in UpdatePoint: updating a point with ");
	  printf("mismatched IDs -- %d != %d\n", WHICH, p.ID);
	}
      return 0;
    }

  int   hash_x, hash_y;           // hash vars
  int   new_hash_x, new_hash_y;   // new hash vars

  // hash the existing point
  if( points[ WHICH ].x == x_hi )
    hash_x = table_dim - 1;
  else
    hash_x = FLOOR( (points[ WHICH ].x - x_lo) / x_segment );
  
  if( points[ WHICH ].y == y_hi )
    hash_y = table_dim - 1;
  else
    hash_y = FLOOR( (points[ WHICH ].y - y_lo) / y_segment );

  // hash the new point
  if( p.x == x_hi )
    new_hash_x = table_dim - 1;
  else
    new_hash_x = FLOOR( (p.x - x_lo) / x_segment );
  
  if( p.y == y_hi )
    hash_y = table_dim - 1;
  else
    new_hash_y = FLOOR( (p.y - y_lo) / y_segment );

  int i, j;  // itr vars
  
  // check to see if new point changed zones
  if( new_hash_x == hash_x && new_hash_y == hash_y )
    {
      points[ WHICH ] = p;
    }
  else 
    // point moved to a new zone so delete it from current zone and add
    // it to new zone
    {
      int curr       = 0;
      int found      = 0;
      int bucket_cnt = counts[ hash_x ][ hash_y ];
      // search list of points in bucket for the point with ID == WHICH
      for( i = 0; i < bucket_cnt && !found; i++ )
	{
          curr = table[ hash_x ][ hash_y ][ i ];
	  if( curr == WHICH ) // located point to be removed
	    {
	      found = 1;
	    }
	}

      if( !found) // error message, unable to find the point in its bucket
	{
	  if( REPORT_ERROR )
	    {
	      printf("ERROR in UpdatePoint():  Unable to locate point ");
	      printf("%d ", WHICH);
	      printf("in zone [%d, %d]\n", hash_x, hash_y);
	    }
	  return 0;
	}
      else
        {
          // remove point from its bucket  
          for( ; i < bucket_cnt; i++ )
            {
              table[ hash_x ][ hash_y ][ i - 1 ] = table[ hash_x ][ hash_y ][ i ];
            }
          // decrease this bucket's count
          counts[ hash_x ][ hash_y ] = counts[ hash_x ][ hash_y ] - 1;
        }

      // double the size of the bucket receiving the updated point if necessary
      if( counts[ new_hash_x ][ new_hash_y ] == 
          max_cnts[ new_hash_x ][ new_hash_y ] )
        {
	  // allocate the new bucket
          int * temp   = new int[ 2*max_cnts[ new_hash_x ][ new_hash_y ] ];
          int max_size = max_cnts[ new_hash_x ][ new_hash_y ];
	  // write the old bucket's contents to the new bucket
          for( j = 0; j < max_size; j++ )
            temp[ j ] = table[ new_hash_x ][ new_hash_y ][ j ];
	  // de-allocate the old bucket
          delete [] table[ new_hash_x ][ new_hash_y ];
	  // add the new bucket to the table
          table[ new_hash_x ][ new_hash_y ] = temp;
	  // update this bucket maximum bucket coun
          max_cnts[ new_hash_x ][ new_hash_y ] = 
            2*max_cnts[ new_hash_x ][ new_hash_y ];
          temp = NULL;
        }

      // record the point in a bucket
      table[ new_hash_x ][ new_hash_y ][ counts[ new_hash_x ][ new_hash_y ] ] 
        = WHICH;
      // update the current size of the bucket
      counts[ new_hash_x ][ new_hash_y ] = counts[ new_hash_x ][ new_hash_y ] 
        + 1;
      points[ WHICH ] = p;
    }
  // return successful update
  return 1;
}                                            // end UpdatePoint( int, Point )

int DistanceTable::UpdatePoint( int WHICH, float * new_pos )
  // Routine that changes the point at index WHICH to a user specified
  // point value.  Returns 1 if successful, else 0.
  //
  //
  // WHICH   :  index of the point to change
  // new_pos :  3D buffer holding the new values for this point
{
  // ensure that WHICH refers to a valid index
  if( WHICH < 0 || WHICH >= pts_max)
    {
      if( REPORT_ERROR )
	{
	  printf("ERROR Illegal index for DistanceTable::UpdatePoint() : ");
	  printf("%d\n", WHICH );
	}
      return 0;
    }
  // ensure that the point at the index WHICH is a valid point
  if( points[ WHICH ].ID == -1 )
    {
      if( REPORT_ERROR )
	{
	  printf("ERROR Illegal index for DistanceTable::UpdatePoint() : ");
	  printf("%d Invalid Point\n", WHICH );
	}
      return 0;
    }
  
  // verify point lies in the appropiate bounding box
  // represented by this table if not reject point
  if( new_pos[ 0 ] < x_lo || new_pos[ 1 ] < y_lo ||
      new_pos[ 0 ] > x_hi || new_pos[ 1 ] > y_hi )
    {
      if( REPORT_ERROR )
	{
	  printf("ERROR: Attempted to add a point outside of the table's");
	  printf(" allowed ranges: [%f, %f]", new_pos[ 0 ], new_pos[ 1 ] );
	  printf(" into [%f, %f]x[%f, %f]\n", x_lo, y_lo, x_hi, y_hi);
	}
      return 0;
    }

  int   hash_x, hash_y;          // hash vars
  int   new_hash_x, new_hash_y;  // new hash vars

  // hash the existing point
  if( points[ WHICH ].x == x_hi )
    hash_x = table_dim - 1;
  else
    hash_x = FLOOR( (points[ WHICH ].x - x_lo) / x_segment );
  
  if( points[ WHICH ].y == y_hi )
    hash_y = table_dim - 1;
  else
    hash_y = FLOOR( (points[ WHICH ].y - y_lo) / y_segment );

  // hash the new point
  if( new_pos[ 0 ] == x_hi )
    new_hash_x = table_dim - 1;
  else
    new_hash_x = FLOOR( (new_pos[ 0 ] - x_lo) / x_segment );
  
  if( new_pos[ 1 ] == y_hi )
    hash_y = table_dim - 1;
  else
    new_hash_y = FLOOR( (new_pos[ 1 ] - y_lo) / y_segment );

  int i, j;    // itr vars
  
  // check to see if new point changed buckets
  if( new_hash_x == hash_x && new_hash_y == hash_y )
    {
      points[ WHICH ].x = new_pos[ 0 ];
      points[ WHICH ].y = new_pos[ 1 ];
      points[ WHICH ].z = new_pos[ 2 ];
    }
  else 
    // point moved to a new bucket so delete it from current bucket and add
    // it to new bucket
    {
      int curr       = 0;
      int found      = 0;
      int bucket_cnt = counts[ hash_x ][ hash_y ];
      // search list of points in bucket for the point with ID == WHICH
      for( i = 0; i < bucket_cnt && !found; i++ )
	{
          curr= table[ hash_x ][ hash_y ][ i ];
	  if( curr == WHICH ) // located point to be removed
	    {
	      found = 1;
	    }
	}

      if( !found ) // error message, unable to find point in its bucket
	{
	  if( REPORT_ERROR )
	    {
	      printf("ERROR in UpdatePoint():  Unable to locate point ");
	      printf("%d ", WHICH);
	      printf("in zone [%d, %d]\n", hash_x, hash_y);
	    }
	  return 0;
	}
      else
        {
          // remove point from bucket  
          for( ; i < bucket_cnt; i++ )
            {
              table[ hash_x ][ hash_y ][ i - 1 ] = table[ hash_x ][ hash_y ][ i ];
            }
          // decrease bucket count
          counts[ hash_x ][ hash_y ] = counts[ hash_x ][ hash_y] - 1;
          
        }

      // double the size of a bucket if necessary
      if( counts[ new_hash_x ][ new_hash_y ] == 
          max_cnts[ new_hash_x ][ new_hash_y ] )
        {
	  // allocate the new bucket
          int * temp   = new int[ 2*max_cnts[ new_hash_x ][ new_hash_y ] ];
          int max_size = max_cnts[ new_hash_x ][ new_hash_y ];
	  // write the old bucket's contents to the new bucket
          for( j = 0; j < max_size; j++ )
            temp[ j ] = table[ new_hash_x ][ new_hash_y ][ j ];
	  // de-allocate the old bucket
          delete [] table[ new_hash_x ][ new_hash_y ];
	  // add the new bucket to the table
          table[ new_hash_x ][ new_hash_y ] = temp;
	  // update this bucket's maximum bucket size
          max_cnts[ new_hash_x ][ new_hash_y ] = 
            2*max_cnts[ new_hash_x ][ new_hash_y ];
          temp = NULL;
        }

      // record the point in a bucket
      table[ new_hash_x ][ new_hash_y ][ counts[ new_hash_x ][ new_hash_y ] ] 
        = WHICH;
      // update the current size of the bucket
      counts[ new_hash_x ][ new_hash_y ] = counts[ new_hash_x ][ new_hash_y ] 
        + 1;
      // write the point to the internal point array
      points[ WHICH ].x = new_pos[ 0 ];
      points[ WHICH ].y = new_pos[ 1 ];
      points[ WHICH ].z = new_pos[ 2 ];
    }
  // return successful update
  return 1;
}                                           // end UpdatePoint( int, float * )

#endif
