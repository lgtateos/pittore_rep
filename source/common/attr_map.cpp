#include "str.h"
#include "attr_map.h"
#include "global.h"

/*--------------------------------------------------------------------------*/
/*  ATTR_MAP.CPP							    */
/*    This class maintains a visual mapping history for a single attribute  */
/*									    */
/*- Modification History: --------------------------------------------------*/
/*  When:	Who:			Comments:			    */
/*									    */
/*  05-Jun-00	Christopher G. Healey	Initial implementation		    */
/*--------------------------------------------------------------------------*/

  // Required definition to allow class to see the static const member
  // variable properly

#ifdef _M_IX86
static const int     vf_num;
#else
const int     attr_map::vf_num;
#endif


string attr_map::name()

  //  This routine returns the name for the given object's current
  //  attribute assignment
{
  return name( cur_vf );
}					// End method name


string attr_map::name( int i )

  //  This routine converts an attribute index into a descriptive name
  //
  //  i:  Index to convert
{
	const string  name[ NONE + 1 ] = {
    "Colour", "Size", "Density", "Regularity", "Orientation", "Greyscale",
      "Flicker", "Direction", "Velocity", "Contrast", "Proportion", "X", "Y", "Z", "None"
  };


  if ( i >= 0 && i <= NONE ) {
    return name[ i ];
  } else {
    return "Undefined";
  }
}					// End method name


int attr_map::num_feature()

  //  This routine returns the number of visual features current defined
{
  return vf_num;
}					// End method num_feature


int attr_map::vf_get()

  //  This method returns the index of the currently assigned visual
  //  feature
{
  return cur_vf;
}					// End method vf_get


void attr_map::vf_set( int feature )

  //  This method sets the currently assigned visual feature
  //
  //  feature:  New feature ID
{
  if ( feature >= 0 && feature < vf_num ) {
    cur_vf = feature;
  }
}					// End method vf_set


attr_map& attr_map::operator=( int feature )

  //  Assignment operator
  //
  //  feature:  New feature ID
{
  if ( feature >= 0 && feature < vf_num ) {
    cur_vf = feature;
  }

  return *this;
}					// End assignment operator


int attr_map::operator==( const attr_map& a ) const

  //  Equality operator
  //
  //  a:  attr_map object to compare against
{
  return ( cur_vf == a.cur_vf );
}					// End equality operator


int attr_map::operator==( const vf_type& vf ) const

  //  Equality operator
  //
  //  vf:  vf_type ID to compare against
{
  return ( cur_vf == vf );
}					// End equality operator


int attr_map::operator!=( const attr_map& a ) const

  //  Inequality operator
  //
  //  a:  attr_map object to compare against
{
  return ( cur_vf != a.cur_vf );
}					// End inequality operator


int attr_map::operator!=( const vf_type& vf ) const

  //  Inequality operator
  //
  //  vf:  vf_type ID to compare against
{
  return ( cur_vf != vf );
}					// End inequality operator


attr_map::operator int()

  //  Cast to int (used to assign attr_map object to int variable)
{
  return int( cur_vf );
}					// End cast to integer


attr_map::attr_map()

  //  Default constructor
{
  cur_vf = NONE;
}					// End default constructor


attr_map::attr_map( vf_type vf )

  //  Default constructor
{
  cur_vf = vf;
}
