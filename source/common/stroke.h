//  STROKE.H
//    Prototypes for STROKE.CPP

#ifndef _stroke_h
#define _stroke_h

#include "str.h"
#include "paint.h"

float get_controlpts( stroke_s& stroke );
float find_shift( float& shift1x, float& shift1y, float& shift2x, float& shift2y, stroke_s stroke );
void draw_bezier( stroke_s stroke, int ID = -1 );
void draw_bezier( stroke_s stroke, float a, int ID = -1 );
void draw_bezier( stroke_s stroke, float texpts[2][2][2], int ID = -1 );
void draw_nurbs( float, float, float, int ID = -1 );
void draw_stroke_no_tex( float r, float g, float b );
void draw_stroke( float, float, float, int ID = -1 );
void draw_stroke( float, float, float, float, int ID = -1 );
void draw_stroke_choptex( float r, float g, float b, float size, int ID );
void draw_stroke_no_tex( float r[], float g[], float b[]);
void draw_stroke_smooth_shading( float r[], float g[], float b[], int ID, float size = 1 );
void draw_3tone_bezier( stroke_s stroke, int ID = -1 );
void draw_3tone_stroke( stroke_s stroke, int ID = -1 );
void draw_trapezoid_stroke( float r, float g, float b, int ID );
void draw_variegated_stroke( stroke_s stroke, int ID );
void draw_polygon( float, float, float, int ID = -1 );
int  get_id(int);
void init_nurbs();
void init_stroke( string );
void setup_nurbs(  int, int, float );
void start_stroke();
void stop_stroke();
void caller( void(*h)(int num), int big );

#endif
