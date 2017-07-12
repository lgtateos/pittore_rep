//  ATTR_MAP.H
//    Class definition for ATTR_MAP.CPP

#ifndef _attr_map_h
#define _attr_map_h

#include "str.h"


class attr_map {
private:
  int  cur_vf;				// Currently assigned visual feature

public:
  typedef enum {			// Enumerate visual feature types
    COLOUR,
    SIZE,
    DENSITY,
    REGULARITY,
    ORIENTATION,
    GREYSCALE,
    FLICKER,
    DIR,
    VEL,
    CONTRAST,
    PROPORTION,
    X_POS,
    Y_POS,
    Z_POS,
    NONE				// Keep NONE as the last entry
  } vf_type;

  //  NOTE: Special case in C++ allows static const INTEGRAL data
  //        member to be initialized in .h file; we still need (and have)
  //        a corresponding definition of the member variable at the top
  //        of the .cpp file, but without the initialization
  //
  //  NOTE: vf_num is public so it can be used directly (e.g., as the
  //        size for an array definition) in the .cpp file
  //
  //  NOTE: vf_name[] entries are at the top of attr_map.cpp

  const static int  vf_num = NONE + 1;	// Number of visual features (shared)


  attr_map();
  attr_map( vf_type );

  string    name();
  string    name( int );
  int       num_feature();
  int       vf_get();
  void      vf_set( int );

  attr_map& operator=( int );
  int       operator==( const attr_map & ) const;
  int       operator==( const vf_type & ) const;
  int       operator!=( const attr_map & ) const;
  int       operator!=( const vf_type & ) const;

  operator int();			// Cast (assign) to integer
};

#endif
