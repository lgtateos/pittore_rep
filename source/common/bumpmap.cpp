#ifdef WIN32
#include<glut.h>
#endif

#ifdef SOLARIS
#include <GL/glut.h>
#endif

#include <GL/glu.h>
#include <GL/gl.h>

#include "bumpmap.h"

// BUMPMAP.CPP   Laura Tateosian 7/07/06
// 
// Contains methods for adding computing lighting with the 
// phong model, given a color and normal, on a per pixel bases


bumpmap::bumpmap(){
}  // end constructor

bumpmap::~bumpmap(){
  dump_buff();
}

void bumpmap::dump_buff(){
  delete [] cf;
  delete [] hf;
  delete [] tf;
  delete [] bf;
}

void bumpmap::alloc_buffers( int fwi, int fhe){
  // fwi:       Width of the framebuffer
  // fhe:       Height of the framebuffer
  tuple_sz1 = 3;
  tuple_sz2 = 1;
  fw = fwi;
  fh = fhe;
  cf = new float [ fw*fh*tuple_sz1 ];
  hf = new float [ fw*fh*tuple_sz2 ];
  tf = new float [ fw*fh*tuple_sz2 ];
  bf = new float [ fw*fh*tuple_sz2 ];
}


void  bumpmap::check_buffer( char type, int i, int j ){
  // buff of size 3*fw*fh
  // i: x coord
  // j: y coord
  
  int k;
  int position;
  int tuple_sz;
  if( tuple_sz1 > tuple_sz2){
    tuple_sz = tuple_sz1;
  }else{
    tuple_sz = tuple_sz2;
  }
  float* value = new float [tuple_sz];

  if ( i >= fw || j >= fh ){
    printf( "In light.cpp check_buffer: Invalid buffer position (%d,%d)\n", i, j );
    printf( "x coord must be in [0%d) and y coord must be in [0,%d)\n", fw, fh);
  }
  
  if ( type == 'c' ){
  for (k = 0; k < tuple_sz1; k++){
    position = tuple_sz1*(i + (j*fw )) + k;
      value[k] = cf[position];
    }
  }else{
    for (k = 0; k < tuple_sz2; k++){
      position = tuple_sz2*(i + (j*fw )) + k;
      if( type == 'h' ){
        value[k] = hf[position];
      }else if(type == 't'){
        value[k] = tf[position];
      }else {
        value[k] = bf[position];
      }
    }
  }// end for k
  if( type == 'c'){
    printf("Buff %c at (%d,%d): (%f, %f, %f)\n", type, i, j, value[0], value[1], value[2] );
  }else{
    //printf("Buff %c at (%d,%d): (%f, %f, %f, %f)\n", type, i, j, value[0], value[1], value[2], value[3] );
    printf("Buff %c at (%d,%d): (%f)\n", type, i, j, value[0] );
  }

  delete [] value;
}


void bumpmap::compute_color( float color[ 3 ], float pos [], float N[ 3 ] ){
  
  // color[3]: Pure unlit color requested by caller at that pixel position.  
  // pos:      Current position being colored
  // N[3]:     Normal vector at that pixel position.

  
  // This method computes the lit color at the given pixel point ( return these values in the array color[3] ).
  // The lit color is determined with the Phong shading model described here:
  // Phong shading model p730 FVD, equation 16.14.  lamda is the component (R, G, or B)
  // I_lamda = ambient term + diffuse and specular term
       // ambient term = I_(a lamda)*k_a*O_(d lamda) 
          //I_(a lamda) = intensity of the ambient light
          //k_a = ambient-reflection coefficient
          //O_(d lamda) = an object's diffuse red, blue, or green components in the RGB color system 
       // diffuse and specular term = f_att*I_(p lamda)[ k_d*O_(d lamda)*(N.L) + k_s*(R.V)^n] 
          //f_att = light source attenuation  //letting this be 1, so light sources are at infinity
          //I_(p lamda) = the point light source's intensity
          //N normal vector
          //L direction to the light source
          //R direction light is reflected
          //V direction to the viewpoint
          //n material's specular-reflection exponent
  
  float ambient; float diffuse; float specular;
  float I_a = 1.2;//.2;
  float I_p = 1;//.7;
  float O_d[3]= {color[0],color[1],color[2]}; // = color;


  float k_a[3]; //.1;
  float k_d[3]; //.8;
  float k_s[3]; //.1;

  set_k_values( k_a, k_d, k_s, color );

  float L_pos[3] = {280, 280, 300};  // position the light

  //float L[3]= { L_pos[0] - pos[0] ,L_pos[1] - pos[1], L_pos[2] - 0 }; // seems to work too  //vector L is components of the light's position - the components of the position currently being colored
  float L[3]= { .5774 , .5774, .5774};  //1/sqrt(3)
/*  if ( N[0] == 0 ){
    N[0] = .1;
    L[0] = .1;
    L[1] = 1;
    L[2] = 1;
  }else if (N[1] == 0 ){
    N[1] = .1;
    L[0] = 1;
    L[1] = .1;
    L[2] = 1;
  }*/
  normalize( L );
  normalize( N );
  
  float N_dot_L; 
  N_dot_L  = N[0]*L[0] + N[1]*L[1] + N[2]*L[2];
  if ( N_dot_L < 0 ){
    N_dot_L = 0;
  // N_dot_L  = -1*N[0]*L[0] + -1*N[1]*L[1] + -1*N[2]*L[2];
  }
  // compute specular component variables
  float V[3] = { fw*.5, fh*.5, 10}; 
  normalize( V );
  float R[3] = { 2*N_dot_L*N[0] - L[0],2*N_dot_L*N[1] - L[1],2*N_dot_L*N[2] - L[2]}; //R = 2(N•L)N - L
  normalize( R );
  float R_dot_V = R[0]*V[0] + R[1]*V[1] + R[2]*V[2];
  if ( R_dot_V < 0 ){
    R_dot_V = 0;
  }

  //float k_s = 0; removing specularity from the equation
  //int n = 2; controls size of specular reflections
  
  // compute ambient component
  for ( int i = 0; i < 3; i++ ){
    ambient = I_a * k_a[i] * O_d[i];
    diffuse = I_p * k_d[i] * O_d[i] * N_dot_L;
    specular = 1*(I_p * k_s[i] * R_dot_V * R_dot_V);
    color[i] = ambient + diffuse + specular;
  }
}  // end compute_color

void bumpmap::init_buffer( char type ){
  // type: 'c', 'h', or 't' for cf, hf, or tf
  // cf: color field
  // hf: height field
  // tf: texture height field
  // Buffers, cf, hf, and tf must be allocated prior to calling init_buffer
  // They must be of size 3*fw*fh 
 
  if ( type == 'c' ){
    glReadPixels( 0, 0, fw, fh, GL_RGB, GL_FLOAT, (GLvoid*) cf );
  }else if ( type == 'h' ){
    glReadPixels( 0, 0, fw, fh, GL_RED, GL_FLOAT, (GLvoid*) hf );
  }else if ( type == 't' ) {
    glReadPixels( 0, 0, fw, fh, GL_RED, GL_FLOAT, (GLvoid*) tf );
  }else {
    glReadPixels( 0, 0, fw, fh, GL_RED, GL_FLOAT, (GLvoid*) bf );
  }
}

void bumpmap::display_bumped1( void (*d)(char), int w, int h, float clearcolor[ 3 ] ){
  float white[ 3 ] = { 1, 1, 1 };
  d('c');
  init_buffer( 'c' );       
  //bm.check_buffer('c', 500, 3);
  
  glClearColor(0,0,0,1);
  glClear( GL_COLOR_BUFFER_BIT );

  d('h');
  init_buffer( 'h' );

  glClear( GL_COLOR_BUFFER_BIT );
  
  d('t');
  init_buffer( 't');
  
  glClearColor( clearcolor[ 0 ], clearcolor[ 1 ], clearcolor[ 2 ], 1 );
  glClear( GL_COLOR_BUFFER_BIT );
  glFlush();

  glPopMatrix();
  light_buffer();
}

void bumpmap::display_bumped( void (*d1)(char), void (*d2)( bool, int, int, int, int, float[ 3 ]), int w, int h, int pixel_w, int pixel_h, float clearcolor[ 3 ] ){
  float white[ 3 ] = { 1, 1, 1 };

  d2( 0, w, h, pixel_w, pixel_h, clearcolor );
  d1('c');
  init_buffer( 'c' );       
  //bm.check_buffer('c', 500, 3);
  
  glClearColor(0,0,0,1);
  glClear( GL_COLOR_BUFFER_BIT );

  d1('h');
  init_buffer( 'h' );

  glClear( GL_COLOR_BUFFER_BIT );
  
  d2( 1, w, h, pixel_w, pixel_h, white  );
  d1('t');
  init_buffer( 't');
  
  glClearColor( clearcolor[ 0 ], clearcolor[ 1 ], clearcolor[ 2 ], 1 );
  glClear( GL_COLOR_BUFFER_BIT );
  glFlush();

  glPopMatrix();
  light_buffer();
}

void bumpmap::find_gradient( int x, int y, int position, float normal[ 3 ]){
  int i; int cur_x; int cur_y;
  float h_neigh;
  int cur_position;
  float h;
  
  int    off[ 8 ][ 2 ] = {		// Offset of each neighbour
    { -1,  1 }, {  0,  1 }, {  1,  1 },
    { -1,  0 },             {  1,  0 },
    { -1, -1 }, {  0, -1 }, {  1, -1 }
  }; 

  normal[2] = .001;
  h = hf[position] + tf[position]; 


  for( i = 0; i < 8; i++ ) {		// For all neighbours   
    if ( off[ i ][ 0 ] != -1 && off[ i ][ 1 ] != -1 ){
    cur_x = x + off[ i ][ 0 ]; // set cur_x to next neighbor's x position 
    cur_y = y + off[ i ][ 1 ];

		// if outside of dataset boundaries
    if ( cur_x < 0 || cur_x >= fw || cur_y < 0 || cur_y >= fh ) {
      continue;
    }
   
  // cur_x = x+1; cur_y = y +1;
   cur_position = cur_x + (cur_y)*fw;//3*(i+1 + ((j+1)*fw)) + 2;
   // normal[2] = hf[position] + tf[position] - ( hf[up_right] + tf[up_right] );
   h_neigh = hf[cur_position] + tf[cur_position];
   
   if (fabs(h_neigh - h) > fabs(normal[2]) ){
     normal[0] = off[i][0];  
     normal[1] = off[i][1];  
     normal[2] = h- h_neigh;
    }
  }

  }
}

void bumpmap::light_buffer(){
  int i; int j; int k;
  float color [3];
  float * normal = new float [3];
  int position;
  int temp_pos; int temp_pos2;
  float pos [2];
  int up_right;
  float cmax = 1;

  glBegin( GL_POINTS );
  for( j = 0; j < fh; j++ ){
    for( i = 0; i < fw; i ++ ){

      for (k = 0; k < 3; k++){
        position = 3*(i + (j*fw )) + k;
        color[k] = cf[position]; 
      }
  
      if ( i+1 >= fw || j+1 >=fh){
        normal[0] = 0;
        normal[1] = 0;
        normal[2] = 1;
      }else{
        normal[0] = .707;
        normal[1] = .707;
        position = i + (j*fw);
   //     find_gradient( i, j, position, normal);
        up_right = i+1 + (j+1)*fw;//3*(i+1 + ((j+1)*fw)) + 2;
        normal[2] = hf[position] + tf[position] - ( hf[up_right] + tf[up_right] );
        normal[2] = -1*normal[2];
		//normal[2] = hf[position]+ tf[position]; //for creating greyscale heightfield image for ppt

        // bumpmap from FVD p 744
        temp_pos = (i + 1) + (j*fw); 
    //    normal[0] = ( hf[position] + tf[position] ) - (hf[temp_pos] + tf[temp_pos]) ;
        temp_pos = i  + ((j+1)*fw);
    //    normal[1] = hf[position] + tf[position]  - ( hf[temp_pos] + tf[temp_pos] );
    //    normal[2] = 1;
        
      }
      pos[0] = i; pos[1] = j;
      compute_color( color, pos, normal );


     
	  glColor3f( color[0], color[1], color[2] );
	 // glColor3f( normal[2], normal[2], normal[2] );// for creating greyscale heightfield images
      glVertex2f(i,j);
    }  
  }
  glEnd();   // end of drawpoints

  delete [] normal;
}  // end routine light_buffer


inline void bumpmap::normalize( float buf[]){
  double radicand = buf[0]*buf[0] + buf[1]*buf[1] + buf[2]*buf[2];
  double length_inv;
  if ( radicand > 0 ){
   length_inv = 1/sqrt(radicand);
  }else{
    length_inv = 0;
  }
  buf[0] = buf[0]*length_inv;
  buf[1] = buf[1]*length_inv;
  buf[2] = buf[2]*length_inv;
}

void bumpmap::order_color_components( float color[ 3 ], int& largest, int& other1, int& other2 ){
  int i;

  // Find index of largest color  
  float  max_color = 0;
  largest = 2;//0;
  for( i = 2; i >= 0; i-- ){
    if ( color[i] > max_color ){
      largest = i;
      max_color = color[i];
    }
  }
  // Assign other 2 indices to variables
  other1 = -1;
  
  for( i = 2; i >= 0; i--){
    if ( i != largest && other1 == -1){
      other1 = i;
    }
    if ( i != other1 && i != largest ){
      other2 = i;
    }
  }
}

void  bumpmap::set_k_values( float k_a[ 3 ], float k_d[ 3 ], float k_s[ 3 ], float color[ 3 ] ){
  int i;
  float ratio_inv;
  
  int largest; int other1; int other2;
  order_color_components( color, largest, other1, other2 );
   
  k_a[ largest ] = .1;
  k_d[ largest ] = .9;
  if( color[ largest ] == 0 ){
    ratio_inv = 1;
  }else{
    ratio_inv = color[ other1 ]/color[ largest ];
  }
  k_a[ other1 ] = k_a[ largest ]*ratio_inv;
  k_d[ other1 ] = k_d[ largest ]*ratio_inv;
  
  if( color[ largest ] != 0 ){  // else just keep ratio_inv = 1 as set above.
    ratio_inv = color[ other2 ]/color[ largest ];
  }
  
  k_a[ other2 ] = k_a[ largest ]*ratio_inv;
  k_d[ other2 ] = k_d[ largest ]*ratio_inv;
   
  for( i = 0; i <  3; i++ ){
    k_s[ i ]= 1 - k_a[ i ] - k_d[ i ]; 
    if ( k_s[ i ] < 0 ){
      k_s[ i ] = 0;
    } 
    if ( k_s[ i ] > .15 ){
      k_s[ i ] = .15;
    }
  }

}