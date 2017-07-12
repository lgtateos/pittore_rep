// LIGHT.H
//    Prototypes for LIGHT.C
#ifndef _bumpmap_h
#define _bumpmap_h

#include <math.h>
#include <GL/gl.h>
#include <stdio.h>
// light.h
class bumpmap {
private:
  int fw; int fh;  // framebuffer dimensions
  int tuple_sz1;
  int tuple_sz2;
  float* cf;  // color buffer
  float* hf;  // height field buffer
  float* tf;  // textured height field buffer
  float* bf;

  void compute_color( float color[ 3 ], float pos [], float N[ 3 ] );
  void find_gradient( int x, int y, int position, float normal[ 3 ]); 
  void normalize( float buf[]);
  void order_color_components( float color[ 3 ], int& largest, int& other1, int& other2 );
  void set_k_values( float k_a[ 3 ], float k_d[ 3 ], float k_s[ 3 ], float color[ 3 ] );
public:
  bumpmap();
  ~bumpmap();
  void dump_buff();
  void alloc_buffers( int fwi, int fhe);  // must be called first
  void check_buffer( char type, int i, int j );       // debugging code
  void display_bumped1( void (*d)(char), int w, int h, float clearcolor[ 3 ] );
  void display_bumped( void (*d1)(char),  void (*d2)( bool, int, int, int, int, float[ 3 ]) , int w, int h, int pixel_w, int pixel_h, float clearcolor[ 3 ] );
  void init_buffer( char the_char );      // call three times from draw routine
  void light_buffer();                    // call when all three buffers (cf, hf, tf ) are initialized
};
#endif _bumpmap_h