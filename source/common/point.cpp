#ifndef POINT_CPP
#define POINT_CPP

#include "point.h"
#include "global.h"
#include "iostream"//#include "iostream.h"
/*--------------------------------------------------------------------------*/
/*  POINT.CPP                                                               */ 
/*	This file contains the implementation of a point class that can be  */
/*      used to store either points or vectors.  This class provides        */
/*      operations for vectors (addition, subtraction, cross product, dot   */
/*      product) and comparison operations for points ( <, <=, ==, >, >== ) */
/*      where x is more important than y  which is more important than z.   */
/*									    */
/*- Modification History ---------------------------------------------------*/
/*  When:	Who:			Comments:			    */
/*									    */
/*  18-Aug-2005	Brent M. Dennis 	Initial implementation		    */
/*--------------------------------------------------------------------------*/

Point::Point( )
{
  x = y = z = 0;

  ID        = -1;
  mark_flag = 0x00;
}

Point::Point( float a, float b )
  // Constructor for Point class
  //
  //
  //  a :  x-value
  //  b :  y-value
{
  x         = a;
  y         = b;
  z         = 0.0;
  ID        = -1;
  mark_flag = 0x00;
}

Point::Point( float a, float b, float c )
  // Constructor for Point class
  //
  //
  //  a :  x-value
  //  b :  y-value
  //  c :  z-value
{
  x         = a;
  y         = b;
  z         = c;
  ID        = -1;
  mark_flag = 0x00;
}                                  // end Point constructor

Point::Point( float a, float b, int id )
  // Constructor for Point class
  // 
  // 
  //   a :  x-value
  //   b :  y-value
  //  id :  point's ID
{
  x         = a;
  y         = b;
  z         = 0.0;
  ID        = id;
  mark_flag = 0x00;
}                                  // end Point constructor

Point::Point( float a, float b, float c, int id )
  // Constructor for Point class
  // 
  // 
  //   a :  x-value
  //   b :  y-value
  //   c :  z-value
  //  id :  point's ID
{
  x         = a;
  y         = b;
  z         = c;
  ID        = id;
  mark_flag = 0x00;
}                                  // end Point constructor

void Point::operator =( const Point & p )
  // Assignment operator for Point class
{
  x  = p.x;
  y  = p.y;
  z  = p.z;
  ID = p.ID;

  mark_flag = p.mark_flag;
}                                         // end operator =

int Point::operator ==( const Point & p )
  // Equality operator for Point class
{
  return ((x == p.x) && (y == p.y) && (z == p.z)); 
}                                          // end operator ==

int Point::operator <=( const Point & p )
  // This method provides a comparative operator.  It ranks first
  // on the x-value, then y, then z
{
  return ((*this == p) || (*this < p)); 
}                                     // end operator <=

int Point::operator <( const Point & p )
  // This method provides a comparative operator.  It ranks first
  // on the x-value, then y, then z
{

  if( x < p.x )
    return 1;
  
  if( ( x == p.x ) && ( y < p.y ) )
    return 1;
  if( ( x == p.x ) && ( y == p.y ) && ( z < p.z ) )
    return 1;
  else
    return 0;
  
}                                      // end operator <
 
int Point::operator >( const Point & p )
  // This method provides a comparative operator.  It ranks first
  // on the x-value, then y, then z
{
	
  if( x > p.x )
    return 1;
	
  if( ( x == p.x ) && ( y > p.y ) )
    return 1;
	
  if( ( x == p.x ) && ( y == p.y ) && ( z > p.z ) )
    return 1;
  else
    return 0;
  
}                                      // end operator >

int Point::operator >=( const Point & p )
  // This method provides a comparative operator.  It ranks first
  // on the x-value, then y, then z
{
  return ( ( *this == p ) || ( *this > p ) ); 
}                                     // end operator >=

void Point::set( float a, float b )
  // This method sets the values of a point
{
  x = a;
  y = b;
}                              // end method set( float, float )

void Point::set( float a, float b, int id )
  // This method sets the values of a point
{
  x  = a;
  y  = b;
  ID = id;
}                              // end method set( float, float, int )

void Point::set( float a, float b, float c )
  // This method sets the values of a point
{
  x = a;
  y = b;	
  z = c;
}                              // end method set( float, float, float )

void Point::set( float a, float b, float c, int id )
  // This method sets the values of a point
{
  x  = a;
  y  = b;	
  z  = c;
  ID = id;
}                              // end method set( float, float, float, int )

void Point::set_mark( int flag )
  // This method sets the mark_flag to a user specified on or off
{
  if( flag )
    mark_flag = 0x01;
  else
    mark_flag = 0x00;
}                              // end method set_mark( )

int Point::markq( )
  // This method returns the marked flag 
{
  return int( mark_flag );
}                              // end method markq( )

void Point::mark( )
  // This method sets the mark flag to 1
{
  mark_flag = 0x01;
}                               // end method mark( )

void Point::unmark( )
  // This method sets the mark flag to 0
{
  mark_flag = 0x00;
}                                // end method unmark( )

Point Point::cross_prod( Point& p )

  //  This method returns the crossproduct of the two vectors
  //
  //  p:  Vector to cross against (assume common tail at origin)
{
  Point  prod;			// Cross-product result


  prod.x = y * p.z - z * p.y;
  prod.y = z * p.x - x * p.z;
  prod.z = x * p.y - y * p.x;

  return prod;
}					// End method cross_prod

float Point::dot_prod( Point& p )

  //  This method returns the dotproduct of the two vectors
  //
  //  p:  Vector to dot against (assume common tail at origin)
{
  float  prod;			// dot-product result

  prod = x*p.x + y*p.y + z*p.z;

  return prod;
}					// End method cross_prod


Point Point::operator+( const Point& p )

  // This method adds two point objects
  //
  // p:  Point to add with
{
  Point  sum;				// Sum of addition


  sum.x = x + p.x;
  sum.y = y + p.y;
  sum.z = z + p.z;

  return sum;
}					// End method operator+


Point Point::operator-( const Point& p )

  // This method subtracts two point objects
  //
  // p:  Point to subtract
{
  Point  sum;				// Sum of subtraction


  sum.x = x - p.x;
  sum.y = y - p.y;
  sum.z = z - p.z;

  return sum;
}					// End method operator-

#endif
