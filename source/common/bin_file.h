//  BIN_FILE.H
//    Class definition for BIN_FILE.CPP

#ifndef _bin_file_h
#define _bin_file_h

#include "attr_map.h"
#include "str.h"
#include "vec.h"

const static int  VEC_BUF_MAX = 5000;	// Maximum vec buffer size

class bin_file {
protected:
  int      *e_n;			// Number of elements per frame
  float    *hi;				// Upper allowable attr val range
  float    *lo;				// Lower allowable attr val range
  attr_map *map;			// Attribute to visual feature maps
  int       n;				// Number of attributes per element
  string   *name;		// Attribute names
  int       t;				// Number of frames in file
  vec       v;				// Input file stream
  string    v_name;			// Input file name


  virtual long  comp_footer_size();
  virtual void  copy( const bin_file& );
  virtual void  flush();
  virtual int   read_footer();

public:
  bin_file();
  bin_file( const bin_file& );
 ~bin_file();

  //  Basic accessors are coded here for simplicity
  int count_500, tount_500, count_10, tount_10;
  virtual int       attr_n() { return n; }
  virtual int       frame_n() { return t; }
  virtual string    filename() { return v_name; }
  virtual string    type() { return "BIN"; }

  virtual int       allowable( float * );
  virtual attr_map  attr_map_val( int );
  virtual void      attr_map_val( attr_map * );
  virtual int       attr_map_val( int, attr_map );
  virtual string    attr_name( int );
  virtual void      attr_name( string * );
  virtual int       attr_name( int, string );
  virtual void      close();
  virtual int       create( string, int, int,
	                  string *, attr_map *, float *, float *, int * );
  virtual int       describe( string&, int&, int&,
	                  string* &, attr_map* &, float* &, float* &, int* & );
  virtual int       elem_n( int );
  virtual void      elem_n( int * );
  virtual int       equivalent( bin_file *, int& );
  virtual int       extract( string, int, int );
  virtual int       find_map( attr_map::vf_type );
  virtual int       get( int, int, float * );
  virtual int       get( int, float * );
  virtual int       min_max( float *, float * );
  virtual int       open( string );
  virtual int       put( int, int, float * );
  virtual int       put( int, float * );
  virtual int       range( int, float * );
  virtual void      range( float r[][ 2 ] );
  virtual int       range_hi( int, float& );
  virtual void      range_hi( float * );
  virtual int       range_lo( int, float& );
  virtual void      range_lo( float * );
  virtual int       size();
  virtual int       update_footer();

  virtual bin_file&  operator=( const bin_file& );
};

#endif
