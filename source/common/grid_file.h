//  GRID_FILE.H
//    Class definition for GRID_FILE.CPP

#ifndef _grid_file_h
#define _grid_file_h
#include "global.h"

#ifdef NAMESPACE
  #include <string>
#else 
  #include <string.h>
#endif

#include "attr_map.h"
#include "bin_file.h"


class grid_file : public bin_file {
public:
  string  type() { return "GRID"; }

  int     create( string, int, int,
	               string *, attr_map *, float *, float *, int *, int *, int * );
  int     describe( string&, int&, int&, string* &, attr_map* &,
            float* &, float* &, int* &, int* &, int* & );
  int     cols( int );
  void    cols( int * );
  int     elem_n( int );
  void    elem_n( int * );
  int     equivalent( bin_file *, int& );
  int     get( int x, int fr, float val[] );
  int     grid_2_bin( string );
  int     is_gridfile();
  int     put( int, int, float * );
  int     rows( int );
  void    rows( int * );
};

#endif
