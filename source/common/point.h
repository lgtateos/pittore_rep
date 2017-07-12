#ifndef POINT_H
#define POINT_H

#include <stdio.h>

#ifndef MAXFLOAT
#define MAXFLOAT ((float) 3.40282346638528860e+38) 
#endif

class Point
{
public:
  Point( );
  Point( float, float );
  Point( float, float, float );
  Point( float, float, int );
  Point( float, float, float, int );
  
  void set( float, float );
  void set( float, float, int );
  void set( float, float, float );
  void set( float, float, float, int );
 
  int  markq( );
  void set_mark( int );
  void mark( );
  void unmark( );

  Point  cross_prod( Point& );
  float  dot_prod( Point& );
  Point  operator+( const Point& );
  Point  operator-( const Point& );
  
  void operator = ( const Point & );
  int  operator ==( const Point & );
  int  operator <=( const Point & );
  int  operator < ( const Point & );
  int  operator > ( const Point & );
  int  operator >=( const Point & );

  float x, y, z;            // x, y, and z vals of the point
  int   ID;                 // ID value of point
  char  mark_flag;          // flag indicating a "mark"
};

#endif
