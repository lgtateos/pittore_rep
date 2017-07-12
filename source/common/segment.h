//  SEGMENT.H
//    Prototypes for SEGMENT.C

#ifndef _segment_h
#define _segment_h
#include "global.h"
#ifdef NAMESPACE
  #include <iostream>
  using namespace std;
#else
  #include <iostream.h>
#endif
#include <math.h>
#include <assert.h>

#include "grid_file.h"
#include "vec.h"
#include "voronoi.h"


struct val_s {			// Attribute value structure
  float *v;			// Allow dynamic number of layers
};

struct seg_s {
  vec     f;			// Input data stream to segment
  int     w;			// Width of data field
  int     h;			// Height of data field
  int     attr_n;		// Number of attrs in each element
  int   **seg;			// 2D array of segment IDs (one per element)
  val_s  *seg_c;		// Array of median attr vals for each segment
  float  *seg_n;		// Size of segment (as fraction of data size)
  int     n;			// Number of segments
  float   min;			// Minimum allowable segment size
  float  *err;			// Error cutoff for each attr
  float   num;			// Numerator of new attr weighting factor
  float   den;			// Denominator of new attr weighting factor
};

void alloc_seg_array( seg_s&, int, int );
void dealloc_seg_arr( seg_s& );
void flush_seg_full( seg_s& );
void flush_seg( seg_s&, int, int );
void init_seg(seg_s&, int, int, int, float, float, float);

int  segment( int, seg_s&, int, grid_file&, int);
int read_segment( seg_s& the_seg, int* regarr, int numreg, float ll[], float ur[], string seg_filename );
void write_segment(seg_s the_seg, int* regarr, int numreg, float ll[], float ur[], int cur_frame, string seg_filename );


#endif
