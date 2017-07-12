#ifndef DISTANCETABLE_H
#define DISTANCETABLE_H

#ifdef _MEM_DEBUG
#include "mem_track.h"
#endif

#include "point.h"
#include <float.h>
#include <stdio.h>

#define SORT_NEIGHBORS 1

#ifndef FLOOR
#define FLOOR( my_val ) ( int( my_val ) )
#endif

#ifndef CEIL
#define CEIL( my_val ) (my_val-int(my_val)>0.0) ? int(my_val+1.0) : int(my_val)
#endif

// structure used to sort points based on a value
struct DTDNode{
  int   ID;
  float dist;
};

// structure used to store any point IDs that were deleted from
// the table.  This way, if a new point is added, it can assume a
// previously used and available ID
struct DTAvailNode{
  int           avail_ID;
  DTAvailNode * next;
};

#define DT_BUCKET_MAX_SIZE 16    // initial bucket size
#define DT_BUFFER_N        1024  // size of internal static buffer

class DistanceTable
{
public:
  DistanceTable( );
  DistanceTable( int, int, Point * );
  DistanceTable( int, int, float * );
  DistanceTable( float, int, Point * );
  DistanceTable( float, int, float * );
 
 
  ~DistanceTable();
  
  int     AddPoint( Point & );
  
  float   GetNeighbors( int, int, float, int &, int *, int );
  float   GetNeighbors( float *, int, float, int &, int *, int );
  float   GetNeighbors( Point, int, float, int &, int *, int );
  float   GetNearestNeighbor( float *, int & );
  float   GetNearestNeighbor( Point, int & );

  float   GetNearestNeighbor_NonMember( float *, int & );
  float   GetNearestNeighbor_NonMember( Point, int & );
  float   GetNeighbors_NonMember( float *, int, float, int &, int *, int );
  float   GetNeighbors_NonMember( Point, int, float, int &, int *, int );

  Point   GetPoint( int );
  Point * GetPointBuffer( );
  void    Hash( int, int &, int & );
  void    PrintTable( );
  float   RebuildTable( int );
  int     RebuildTable( float );
  int     RemovePoint( int );
  int     UpdatePoint( int, Point & );
  int     UpdatePoint( int, float * );

private:
  
  void  BuildTable( );
  void  EmptyTable( );
  void  ExtendPointArray( );
  float FindNeighbors( float *, int, float, int &, int *, int, int );
  float FindNearestNeighbor( float *, int &, int );

  int   table_dim;               // number of rows and cols in table
  float x_hi, x_lo, y_hi, y_lo;  // the bounding box for valid points in table
  float x_rng, y_rng;            // the dimensions of the bounding box
  float x_segment, y_segment;    // resolutions of a bucket
  int   pts_n;                   // number of points in internal point buffer
  int   pts_max;                 // maximum number of points that can be
                                 // stored in the internal point buffer
  
  Point * points;        // internal buffer used to store the points
  int *** table;         // hash table of points
  int **  counts;        // table of number of points in each bucket
  int **  max_cnts;      // table of maximum number of points currently 
                         // allowed in each bucket

  DTAvailNode avail_list; // data structure to keep track of available point
                          // IDs.  If a point is removed, its ID is added here

  DTDNode sort_array[ DT_BUFFER_N ]; // static buffer for reporting results

  int DEBUG;        // DEBUG flag used to report various issues to user
  int REPORT_ERROR; // DEBUG flag used to report various issues to user
};

#endif
