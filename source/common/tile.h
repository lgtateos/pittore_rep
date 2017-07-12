#ifndef _tile_h
#define _tile_h

#ifdef WIN32
#include <windows.h>
#include<glut.h>
#endif

#ifdef SOLARIS
#include <GL/glut.h>
#endif

#include <GL/glu.h>
#include <GL/gl.h>

#include "global.h"
#include "stroke.h"
#include "point.h"
#include "voronoi.h"

void draw_low_detail( bool textured, bool rectangular, int d_w, int d_h, int pix_w, int pix_h, float lowleft[ 2 ], float upright[ 2 ], voronoi* voron, int numvalidpts, int fra_n  );
void draw_low_detail2( bool textured, bool rectangular, int d_w, int d_h, int pix_w, int pix_h, float lowleft[ 2 ], float upright[ 2 ], voronoi* voron, int numvalidpts, int fra_n  );
void draw_canvas( bool textured, int d_w, int d_h, int pix_w, int pix_h, float color[ 3 ] );
float set_canvas_dim( voronoi*, int fra_n );

#endif _tile_h