//  PAINT.H
//    Prototypes for PAINT.CPP

#ifndef _paint_h
#define _paint_h

#include "segment.h"


struct point2D {
  float  x, y;				// Position in 2D space
};

struct stroke_s {
  //int cp[ 2 ]; 				// number of control points
  float controlpts[4][4][3];  //control points for Bezier strokes
  point2D  p;				  // Center of stroke
  float    r, g, b;		// RGB colour of stroke
  float    rr[2], gg[2], bb[2]; //RGB colours for variegated strokes.

  float    rot;				// Counterclockwise rotation
  float    s[ 2 ];		// X, Y scale factors
  int seg_index;      // 
  int tex;
  char type;
  int shape;
};

void find_avg_size( float*);
int  expand_st( stroke_s *&, int& );
void set_bezier( bool bo );
int  stroke( int **, int, int, int, stroke_s *&, int, int&, float, int );

#endif
