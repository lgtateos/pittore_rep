#if !defined(SGI) && !defined(SOLARIS) && !defined(LINUX)
#include <windows.h>
#endif

#include <GL/gl.h>
#include <GL/glu.h>
#include <stdio.h>
#include "filereader.h"
#include "global.h"
#include "str.h"
#include "random.h"
#include "stroke.h"
#include "point.h"


/*--------------------------------------------------------------------------*/
/*  STROKE.CPP								    */
/*    Routines to initialize and draw brush strokes as modulating texture   */
/*    maps								    */
/*									    */
/*- Modification History: --------------------------------------------------*/
/*  When:	Who:			Comments:			    */
/*									    */
/*  19-Feb-00	Christopher G. Healey	Initial implementation		    */
/*  09-Apr-02	Christopher G. Healey	Added ability to specify texture    */
/*					to use in draw_stroke() method	    */
/*  05-May-03   Laura G. Tateosian      Added Bezier surface stroke         */
/*                                      method draw_bezier()                */
/*  10-June-03  Laura G. Tateosian      Added NURBS surface stroke method   */
/*                                      draw_nurbs()                        */
/*--------------------------------------------------------------------------*/

//  Module global prototypes
extern int set_orient( float rot, Point pt, float& orientation );
static int  read_tex_map( string& );
static int  check_size( int, int );
static void check_points( int, int );
bool bla = true;
//  Module global variables

#define  MAX_TEX  16			// Maximum number of texture
int i, j; 				// Counters
static int prev_ID = -1;
int      tex_h[ MAX_TEX ];		// Texture height (multiple of 2)
GLuint   tex_ID[ MAX_TEX ];		// Texture map ID
GLubyte *tex_map = NULL;		// Texture map data (as 4-byte RGBA)
int      tex_n = 0;			// Number of textures
int      tex_w[ MAX_TEX ];		// Texture width (multiple of 2)

// Nurbs variables
GLUnurbsObj *theNurb;
	GLUquadricObj* theobj; // ***testing
float* u_knots;
float* v_knots;
int num_u_knots;
int num_v_knots;
float* texture_knots;
float* control_points;
float* vert_sep;		        // Vertical separation btwn control pts
float sum_hor_sep;
float sum_vert_sep;
float* hor_sep;				// Horizontal separation btwn control pts 
float* offset;				// Horizontal offset of leftmost ctlpt zero

/*
// Hard-coded NURBS surface control points for testing
float  ctlpoints[ 7 ][ 4 ][ 3 ] = { 
    {{2,0,0},  {4,0,0},   {6,0,0},  {8,0,0}},
    {{ 7,-4,0},  {9,-4,0},  {11,-4,0},  {13,-4,0}},
    {{-3,-8,0},  {-1,-8,0},   {1,-8,0}, { 3,-8,0}},
    {{7,-12,0}, {9,-12,0},  {11,-12,0},  {13,-12,0}},
    {{-3,-16,0},  {-1,-16,0},  {1,-16,0}, {3,-16,0}},
    {{7,-20,0},  {9,-20,0},  {11,-20,0},  {13,-20,0}},
    {{-3,-24,0}, {-1,-24,0}, {1,-24,0},   {3,-24,0}}
  };

   float texpts[2][2][3] = {
                            {{0,0,0}, {0,1,0}},
			    {{1,0,0}, {1,1,0}}
			  };
	*/		  

void caller( void(*h)(int num), int big ){
  h(big);
}
void check_points( int num_up, int num_vp )

  //Display nurbs control point values and/or draw control points
{
    glPointSize(5);
    glColor3f(1,1,0);
    glBegin(GL_POINTS);
    for (int i = 0; i < num_up*num_vp*3; i++ )
{

     if ( i%3 == 0 ){
      cout<<"pts1["<< i <<"] = "<< control_points[ i ]<<",";
     }else if(i%3 == 1)
     {
      cout<<control_points[ i ]<<",";
       }else
       {
         cout<<control_points[ i ]<<endl;
       glVertex3f(control_points[ i - 2 ], control_points[ i - 1 ], control_points[ i ]);
       }      
    } 
 /*   for (int i =0; i < num_up; i++ ) //7 for ctlpoints
    {
       for (int j = 0; j < num_vp; j++ )
       {
	{ 
	// glVertex3f(ctlpoints[i][j][0], ctlpoints[i][j][1], ctlpoints[i][j][2] );
	 }
	 //cout<<"pt2["<<i<<"]["<<j<<"] = "<<ctlpoints[i][j][0]<<",";
	 //cout<<ctlpoints[i][j][1]<<",";
	 //cout<<ctlpoints[i][j][2]<<endl;
       }
    }*/
    glEnd();
} //end routine check_points

void draw_bezier( stroke_s stroke, int ID )
  // Draw a Bezier surface brush stroke 
{ 
  //static int  prev_ID = -1;		// Previous bound texture ID
  
  //Scale roughly to size of strokes 
  //glScalef( .167, .125, 1);
  
  if ( ID < 0 || ID >= tex_n ) {	// Choose random ID if given ID bad
    ID = int( rand_num() * tex_n );
    ID = ( ID >= tex_n ) ? tex_n - 1 : ID;
  }
  //activate the texture
  if ( ID != prev_ID ) {
    glBindTexture( GL_TEXTURE_2D, tex_ID[ ID ] );
    prev_ID = ID;
  }

  /*
  //Rotating the second set of 4 control points
  // Rotation by hand
   float angle = 45;
   Point midpt;
   angle = angle*3.1415927/180;
   float cosangle = cos(angle);
   float sinangle = sin(angle);
   float x_orig;
   for ( i  = 1; i < 2; i++ ){
     midpt.x = (controlpts[i][1][0]+controlpts[i][2][0])*0.5;
     midpt.y = (controlpts[i][1][1]+controlpts[i][2][1])*0.5;
     for ( j  = 0; j < 4; j++ ){   
       x_orig = controlpts[i][j][0];
       controlpts[i][j][0] = midpt.x + (controlpts[i][j][0]-midpt.x)*cosangle - (controlpts[i][j][1]-midpt.y)*sinangle;
       controlpts[i][j][1] = midpt.y + (x_orig-midpt.x)*sinangle + (controlpts[i][j][1]-midpt.y)*cosangle;
     }
   }
  */
   
  //Initialize an evaluator using glMap
  glMap2f(GL_MAP2_VERTEX_3, //target: 1-d curve, 3 coord. per pt
  0, 1,			    //start and end parameter (t) values
  3, 			    //"stride": pts stored 3 GLfloats apart
  4, 			    //# of control pts
  0, 1,			    //start and end parameter (t) values
  12, 			    //"stride": pts stored 12 GLfloats apart
  4, 			    //# of control pts
  &stroke.controlpts[0][0][0]);	    //control pt data
  
  float texpts[2][2][2] = {  //  Our .map texture format differs from the usual texture format, so
                             //  these texture pts differ from examples in the redbook.       
    {{0,1},{1,1}},    
    {{0,0},{1,0}}
  };
  
  glMap2f(GL_MAP2_TEXTURE_COORD_2, 0, 1, 2, 2, 0, 1, 4, 2, &texpts[0][0][0]);
  
  //Enable an evaluator and the texture using glEnable
  glEnable(GL_MAP2_VERTEX_3);
  glEnable(GL_MAP2_TEXTURE_COORD_2);
  
  //Draw the curve
  //start_stroke();
  

  glEnable(GL_TEXTURE_2D);



  glColor3f(stroke.r, stroke.g, stroke.b);
  glMapGrid2f(20, 0, 1, 20, 0, 1);
  glEvalMesh2( GL_FILL, 0, 20, 0 ,20);  

//  stop_stroke();
   /*
  // for debugging
  glColor3f(.5,0,0);

  glPointSize(3);
  glBegin( GL_POINTS );
    for ( i = 0; i < 4; i++ ){
      if ( i == 1){
        glColor3f(1,0,0);
      }else if (i == 2 ){
        glColor3f(0,.5,0);
      }else if (i == 3){
        glColor3f(0,1,0);
      }
      for( j = 0; j < 4; j++ ){
        glVertex3fv(stroke.controlpts[i][j]);
      }
    }
  glEnd();
  */
} 			//end routine draw_bezier


void draw_bezier( stroke_s stroke, float a, int ID )
  // Draw a Bezier surface brush stroke 
{

  //double t;
  //double y;
  
  //static int  prev_ID = -1;		// Previous bound texture ID
  
  //Scale roughly to size of strokes 
  //glScalef( .167, .125, 1);
  
  if ( ID < 0 || ID >= tex_n ) {	// Choose random ID if given ID bad
    ID = int( rand_num() * tex_n );
    ID = ( ID >= tex_n ) ? tex_n - 1 : ID;
  }
  //activate the texture
  if ( ID != prev_ID ) {
    glBindTexture( GL_TEXTURE_2D, tex_ID[ ID ] );
    prev_ID = ID;
  }
    
  /*
  //Rotating the second set of 4 control points
  // Rotation by hand
   float angle = 45;
   Point midpt;
   angle = angle*3.1415927/180;
   float cosangle = cos(angle);
   float sinangle = sin(angle);
   float x_orig;
   for ( i  = 1; i < 2; i++ ){
     midpt.x = (controlpts[i][1][0]+controlpts[i][2][0])*0.5;
     midpt.y = (controlpts[i][1][1]+controlpts[i][2][1])*0.5;
     for ( j  = 0; j < 4; j++ ){   
       x_orig = controlpts[i][j][0];
       controlpts[i][j][0] = midpt.x + (controlpts[i][j][0]-midpt.x)*cosangle - (controlpts[i][j][1]-midpt.y)*sinangle;
       controlpts[i][j][1] = midpt.y + (x_orig-midpt.x)*sinangle + (controlpts[i][j][1]-midpt.y)*cosangle;
     }
   }
  */
   
  //Initialize an evaluator using glMap
  glMap2f(GL_MAP2_VERTEX_3, //target: 1-d curve, 3 coord. per pt
  0, 1,			    //start and end parameter (t) values
  3, 			    //"stride": pts stored 3 GLfloats apart
  4, 			    //# of control pts
  0, 1,			    //start and end parameter (t) values
  12, 			    //"stride": pts stored 12 GLfloats apart
  4, 			    //# of control pts
  &stroke.controlpts[0][0][0]);	    //control pt data
  
  float texpts[2][2][2] = {  //  Our .map texture format differs from the usual texture format, so
                             //  these texture pts differ from examples in the redbook.       
    {{0,1},{1,1}},    
    {{0,0},{1,0}}
  };
  
  glMap2f(GL_MAP2_TEXTURE_COORD_2, 0, 1, 2, 2, 0, 1, 4, 2, &texpts[0][0][0]);
  
  //Enable an evaluator and the texture using glEnable
  glEnable(GL_MAP2_VERTEX_3);
  glEnable(GL_MAP2_TEXTURE_COORD_2);
  
  //Draw the curve
  start_stroke();
  
  glColor4f(stroke.r, stroke.g, stroke.b, a);
  glMapGrid2f(20, 0, 1, 20, 0, 1);
  glEvalMesh2( GL_FILL, 0, 20, 0 ,20);  

  stop_stroke();

} 			//end routine draw_bezier


void draw_bezier( stroke_s stroke, float texpts[2][2][2], int ID )

  // Draw a Bezier surface brush stroke 
{
  //double t;
  //double y;
  
  //static int  prev_ID = -1;		// Previous bound texture ID
  
  //Scale roughly to size of strokes 
  //glScalef( .167, .125, 1);
  
  if ( ID < 0 || ID >= tex_n ) {	// Choose random ID if given ID bad
    ID = int( rand_num() * tex_n );
    ID = ( ID >= tex_n ) ? tex_n - 1 : ID;
  }
  //activate the texture
  if ( ID != prev_ID ) {
    glBindTexture( GL_TEXTURE_2D, tex_ID[ ID ] );
    prev_ID = ID;
  }
   
  //Initialize an evaluator using glMap
  glMap2f(GL_MAP2_VERTEX_3, //target: 1-d curve, 3 coord. per pt
  0, 1,			    //start and end parameter (t) values
  3, 			    //"stride": pts stored 3 GLfloats apart
  4, 			    //# of control pts
  0, 1,			    //start and end parameter (t) values
  12, 			    //"stride": pts stored 12 GLfloats apart
  4, 			    //# of control pts
  &stroke.controlpts[0][0][0]);	    //control pt data

  
  glMap2f(GL_MAP2_TEXTURE_COORD_2, 0, 1, 2, 2, 0, 1, 4, 2, &texpts[0][0][0]);
  
  //Enable an evaluator and the texture using glEnable
  glEnable(GL_MAP2_VERTEX_3);
  glEnable(GL_MAP2_TEXTURE_COORD_2);
  
  //Draw the curve
  start_stroke();
  
  glColor3f(stroke.r, stroke.g, stroke.b);
  glMapGrid2f(20, 0, 1, 20, 0, 1);
  glEvalMesh2( GL_FILL, 0, 20, 0 ,20);  

  stop_stroke();
 
  /*
  // for debugging
  glColor3f(.5,0,0);
  if (bla)
    glColor3f(1,1,1);
  glPointSize(3);
  glBegin( GL_POINTS );
    for ( i = 0; i < 4; i++ ){
      if ( i == 1 && !bla){
        glColor3f(1,0,0);
      }else if (i == 2 && !bla){
        glColor3f(0,.5,0);
      }else if (i == 3 && !bla){
        glColor3f(0,1,0);
      }
      for( j = 0; j < 4; j++ ){
        glVertex3fv(stroke.controlpts[i][j]);
      }
    }
  glEnd();
  */
} 			//end routine draw_bezier (with variable texture pts)


float find_shift( float& shift1x, float& shift1y, float& shift2x, float& shift2y, stroke_s stroke ){
  // Given a  rotated stroke, this routine samples orientation values one quarter of the way 
  // from both ends of the stroke.  These values are used to determine how to shift the 
  // end controlpts to reflect these orientations.  The shift values are used in the calling
  // routine to create a curved stroke.

  // shift1x:  horizontal shift for top end controlpts
  // shift1y:  vertical shift for top end controlpts
  // shift2x:  horizontal shift for bottom end controlpts
  // shift2y:  vertical shift for bottom end controlpts
  // stroke:   the stroke with scaling and rotation information set  

  float theta, theta_m1, theta_m2, theta_diff, diff;
  float costheta;
  float r;
  float dx, dy;
  Point m1, m2;
  
  // Get orientation at center, noting the following:  
  // In our scheme, strokes are positioned upright unless a rotation is applied, so
  // stroke.rot stores the amount a stroke should be rotated from vertical to achieve
  // the desired rotation.  By adding 90 degrees to stroke.rot, we get the angle of orientation
  // from horizontal, which is what we need for these calculations.
  theta = stroke.rot + 90;  
  

  // Find m1's and m2's positions (1/4 of the way from the ends of the rotated stroke)
  r = stroke.s[1]*0.25;  //one fourth of the stroke length
  costheta = cos(theta*0.0174533);  // 0.01745 = pi/180--conversion to radians.
  dx = r*costheta;
  dy = sqrt( ( r*r )-(dx*dx) );
  m1.x = stroke.p.x + dx;      //stroke's center x + dx 
  m1.y = stroke.p.y + dy;      //stroke's center y + dy
  m2.x = stroke.p.x - dx;
  m2.y = stroke.p.y - dy;

  // To find shift1, get orientation at m1
  if( !set_orient( stroke.rot, m1, theta_m1) ){
    shift1x = 0;
    shift1y = 0.5;
    diff = 0;
  }else {
    // Find theta_m1 minus theta.
    theta_m1 = theta_m1 + 90;
    theta_diff = theta_m1 - theta;
    //If abs(theta_diff) > 90, you're taking the long way, since direction is not considered. 
    if ( theta_diff > 90 ){ // if it's in quadrant II, find the opposite of the supplement, in quadrant IV        
        theta_diff = theta_diff - 180;
    }else if( theta_diff < -90 ){ // if it's in quadrant III, find the opposite of the supplement, in quadrant I 
        theta_diff = theta_diff + 180;
    }
    // theta_diff is in [-90,90].  We're interested in the normalized abs value
    if ( theta_diff < 0 ){
      diff = -1*theta_diff*0.0111111; // 0.0111111 = 1/90
    } else {
      diff = theta_diff*0.0111111;
    }
    
    theta_diff = theta_diff*0.0174533;// 0.01745 = pi/180;
    
    // Use 2D rotation matrix to find the new position for p1
    // Use angle formula for q = theta_new, x = 0, y = 1/3 * stroke.s[1]/stroke.s[0], 
    // since basic stroke is 1 unit centered at 0,0 and
    // stroke.s[1]/stroke.s[0] is the ratio of height to width 
    // Later the shiftx will be multiplied by stroke.s[0] to scale it.
    // x' = xcosq - ysinq
    // y' = xsinq + ycosq
    shift1x = -.33333333*stroke.s[1]*sin(theta_diff)/stroke.s[0]; 
    shift1y = .33333333*cos(theta_diff) + 0.166666667; //add 1/6 to since y has real position = 1/3 + 1/6
  }

  //To find shift2 get orientation at m2
  if ( ! set_orient( stroke.rot, m2, theta_m2) ){
    shift2x = 0;
    shift2y = -0.5;
  }else{
    // Find appropriate shift2 values
    theta_m2 = theta_m2 + 90;
    theta_diff = theta_m2 - theta;

    if ( theta_diff > 90 ){
        theta_diff = theta_diff - 180;
    }else if( theta_diff < -90 ){
        theta_diff = theta_diff + 180;
    }
    
    // theta_diff is in [-90,90].  We want the normalized absolute value.
    if( theta_diff  <  0 ){
      diff = max( diff,  -1*theta_diff*0.0111111);  // .00555556 = 1/90
    }else{
      diff = max( diff,  theta_diff*0.0111111 );
    }

    theta_diff = theta_diff*0.0174533;// 0.01745 = pi/180;
    
    // Use 2D rotation matrix to find the new location for p2
    // Use angle formula for q = theta_new, x = 0, y = 1/3* stroke.s[1]/stroke.s[0], 
    // since basic stroke is 1 unit centered at 0,0.
    // x' = xcosq - ysinq
    // y' = xsinq + ycosq
    shift2x = .33333333*stroke.s[1]*sin(theta_diff)/stroke.s[0];
    shift2y = -.33333333*cos(theta_diff) - .166666667; //add 1/6 to since y has real position = 1/3 + 1/6
  }
  return diff;
}


float get_controlpts( stroke_s& stroke ){
  int i, j;
  float diff;
  float shift1x, shift1y, shift2x, shift2y;

  //Specify an 1 by 1 patch of 16 control points centered about (0,0)
  for ( i = 0; i < 4; i++ ){
    for( j = 0; j < 4; j++ ){
      stroke.controlpts[i][j][0] = -0.5 + j*.3333333;  //x
      stroke.controlpts[i][j][1] = 0.5 -  i*.3333333;  //y
      stroke.controlpts[i][j][2] = 0;                  //z 
    }
  }

  diff = find_shift( shift1x, shift1y, shift2x, shift2y, stroke );

  // Shift x and y values of end set of controlpts based on angles near ends 
  for( i = 0; i < 4; i++ ){ 
      stroke.controlpts[0][i][0] += shift1x;
      stroke.controlpts[0][i][1] = shift1y;
      stroke.controlpts[3][i][0] += shift2x;
      stroke.controlpts[3][i][1] = shift2y;
  }

  return diff;
} // end routine get_controlpts



static int check_size( int w, int h )

  //  This routine ensures both the width and height of the texture
  //  map are powers of 2
  //
  //  w, h:  Width, height to check
{
  int  h_OK = 0;			// Height OK flag
  int  pow = 1;				// Current power of 2
  int  w_OK = 0;			// Width OK flag


  while( w >= pow || h >= pow ) {	// Continue until power passes w, h
    h_OK |= ( h == pow );		// Check for power of 2 match
    w_OK |= ( w == pow );

    pow *= 2;
  }

  return ( h_OK && w_OK );		// Return TRUE if w, h both matched
}					// End routine check_size


void make_knots( float*& knots, int array_size, int clamp_size )

  // Create knot array with values monotonically increasing from 0 to 1. 
  // 0 and 1 are repeated clamp_size times on each end to clamp the ends
  // of the curve to the first and last control points
{  
   int mid_size = array_size - 2* clamp_size;
   knots = new float[array_size];
   
   // Let the first clampsize knots be 0 and the last
   // clamp_size knots be 1.
   for (i = 0; i < clamp_size ; i++ )
   {
     knots[i] = 0.0;
     knots[ i + array_size - clamp_size ] = 1.0;
   }
   
   // Knots between the clamping values are evenly distributed btwn 0 & 1.
   for (i = 0; i < mid_size ; i++ ) 
   {
     knots[ i + clamp_size ] =  float( i + 1 )/float( mid_size + 1 );
   }
 /*   
 //For testing knot array values:
   for (i = 0; i < array_size ; i++ )
   {
     cout<< "knots["<<i<<"] ="<<knots[i]<<endl;
   }*/
}			//End routine make_knots


void draw_nurbs( float r, float g, float b, int ID )  
  //  Draw a NURBS curve brush stroke centered about (0,0) with width 1, height
  //  determined by the number of control points;
  //  user can modify position with translate, size with scale
  //
  //  r, g, b:  Underlying polygon color to modulate
  //  ID:       ID of texture to layer on stroke (defaults to -1)
{					  
   // static int  prev_ID = -1;		// Previous bound texture ID


    if ( ID < 0 || ID >= tex_n ) {	// Choose random ID if given ID bad
      ID = int( rand_num() * tex_n );
      ID = ( ID >= tex_n ) ? tex_n - 1 : ID;
    }

    if ( ID != prev_ID ) {
      glBindTexture( GL_TEXTURE_2D, tex_ID[ ID ] );
      prev_ID = ID;
    }
            


    glColor3f(r, g, b);
 //   glPushMatrix();
 //   glRotatef(330.0, 1.,0.,0.);
  //  glScalef (.6, .6, 1);
    
    //start_stroke();  //Bind texture in preperation for rendering

     float texpts[2][2][2] = {
    {{0,0},{0,1}},
    {{1,0},{1,1}}
  };

    gluBeginSurface(theNurb);
/*
	//Apply texture
  float tmp = texpts[0][0][0];

	gluNurbsSurface(theNurb, 	     
	    4, texture_knots,
	    4, texture_knots,
	    2 * 3,
	    3,
	    &texpts[0][0][0], 
	    2, 2,
  
	    GL_MAP2_TEXTURE_COORD_2); 
 
        //Draw curved surface
	gluNurbsSurface(theNurb,        	     
            num_u_knots, u_knots,
            num_v_knots, v_knots,
	    4* 3,
	    3,
	    &ctlpoints[0][0][0], 
	    //&control_points[0],
	    4, 4,
	    GL_MAP2_VERTEX_3);
   gluEndSurface(theNurb);    

     */

 //gluSphere(theobj, 5, 100, 100);

  //stop_stroke();  //Disable texture mapping 
    //draw_points(7,4);
   // check_points( 4, 4 );
    delete [] u_knots;
    delete [] v_knots;
    delete [] texture_knots; 
 //   delete [] texpts;
    delete [] control_points;
  //  glPopMatrix();
  //  glFlush();

}  // end routine draw_nurbs

void draw_polygon( float r, float g, float b, int ID )

  //  Draw a brush stroke centered about (0,0) with width 1, height 4;
  //  user can modify position with translate, size with scale
  //
  //  r, g, b:  Underlying polygon color to modulate
  //  ID:       ID of texture to layer on stroke (defaults to -1)
{
 // static int  prev_ID = -1;		// Previous bound texture ID

  int temp_id = ID;

  if ( ID < 0 || ID >= tex_n ) {	// Choose random ID if given ID bad
    ID = int( rand_num() * tex_n );
    ID = ( ID >= tex_n ) ? tex_n - 1 : ID;
  }
  
  printf("In draw_polygon, the attempted id, temp_id = %d and the chosen ID = %d\n", temp_id, ID);
  
  if ( ID != prev_ID ) {
    glBindTexture( GL_TEXTURE_2D, tex_ID[ ID ] );
    prev_ID = ID;
  }

  glColor3f( r, g, b );
  glBegin( GL_POLYGON );			// Draw textured polygon
/*  glTexCoord2f( 0, 0 );  //triangle
  glVertex2f( -0.5,-0.5 );
  glTexCoord2f( 1, 0 );
  glVertex2f( .5,-0.5 );
  glTexCoord2f( 0.5, 1  );
  glVertex2f( 0,0.5 );
 */
	glTexCoord2f( 0, 0 );
 // glVertex2f( 72, 304 );
  glVertex2f( 350, 650 );

// glTexCoord2f( 0, 655/1024 );
//	glTexCoord2f( 0, .639 );
 glTexCoord2f( 0, 1 );
 //glVertex2f( 72, 959 );
  glVertex2f( 350, 750 );

 // glTexCoord2f( 679/1024, 655/1024 );
 //glTexCoord2f( .663, .639 );
glTexCoord2f( 1, 1 ); 
//glVertex2f( 751, 959 );
glVertex2f( 450, 750 );

 // glTexCoord2f( 679/1024, 0 );
  //glTexCoord2f( .663, 0 );
  glTexCoord2f( 1, 0 );
//	glVertex2f( 751, 304 );
	glVertex2f( 450, 650  );
	glEnd();
}					// End routine draw_polygon

void draw_stroke_no_tex( float r, float g, float b )

  //  Draw a brush stroke centered about (0,0) with width 1, height 4;
  //  user can modify position with translate, size with scale
  //
  //  r, g, b:  Underlying polygon color to modulate
  //  ID:       ID of texture to layer on stroke (defaults to -1)
{
  float  v[ 4 ][ 2 ] = {		// Stroke vertices
    { -0.5, -0.5 },   //  3      2
    {  0.5, -0.5 },   //   x    x
    {  0.5,  0.5 },   //   x    x
    { -0.5,  0.5 }    //  0      1
  };


  glColor3f( r, g, b );

  glBegin( GL_POLYGON );			// Draw textured polygon

  glVertex2fv( v[ 0 ] );

  glVertex2fv( v[ 3 ] );

  glVertex2fv( v[ 2 ] );

  glVertex2fv( v[ 1 ] );
	glEnd();
}					// End routine draw_stroke_no_tex


void draw_stroke( float r, float g, float b, int ID )

  //  Draw a brush stroke centered about (0,0) with width 1, height 4;
  //  user can modify position with translate, size with scale
  //
  //  r, g, b:  Underlying polygon color to modulate
  //  ID:       ID of texture to layer on stroke (defaults to -1)
{
    
  float  v[ 4 ][ 2 ] = {		// Stroke vertices
    { -0.5, -0.5 },   //  3      2
    {  0.5, -0.5 },   //   x    x
    {  0.5,  0.5 },   //   x    x
    { -0.5,  0.5 }    //  0      1
  };

 // static int  prev_ID = -1;		// Previous bound texture ID

  if ( ID < 0 || ID >= tex_n ) {	// Choose random ID if given ID bad
    ID = int( rand_num() * tex_n );
    ID = ( ID >= tex_n ) ? tex_n - 1 : ID;
  }

  if ( ID != prev_ID ) {
    glBindTexture( GL_TEXTURE_2D, tex_ID[ ID ] );
    prev_ID = ID;
  }

  glColor3f( r, g, b );

  glBegin( GL_QUADS );			// Draw textured polygon
  glTexCoord2f( 0, 0 );
  glVertex2fv( v[ 0 ] );
  glTexCoord2f( 0, 1 );
  glVertex2fv( v[ 3 ] );
  glTexCoord2f( 1, 1 );
  glVertex2fv( v[ 2 ] );
  glTexCoord2f( 1, 0 );
  glVertex2fv( v[ 1 ] );
	glEnd();
}					// End routine draw_stroke

void draw_stroke( float r, float g, float b, float a, int ID )

  //  Draw a brush stroke centered about (0,0) with width 1, height 4;
  //  user can modify position with translate, size with scale
  //
  //  r, g, b:  Underlying polygon color to modulate
  //  ID:       ID of texture to layer on stroke (defaults to -1)
{

 float  v[ 4 ][ 2 ] = {		// Stroke vertices
    { -0.5, -0.5 },   //  3      2
    {  0.5, -0.5 },   //   x    x
    {  0.5,  0.5 },   //   x    x
    { -0.5,  0.5 }    //  0      1
  };
 // static int  prev_ID = -1;		// Previous bound texture ID

  if ( ID < 0 || ID >= tex_n ) {	// Choose random ID if given ID bad
    ID = int( rand_num() * tex_n );
    ID = ( ID >= tex_n ) ? tex_n - 1 : ID;
  }

  if ( ID != prev_ID ) {
    glBindTexture( GL_TEXTURE_2D, tex_ID[ ID ] );
    prev_ID = ID;
  }

  glColor4f( r, g, b, a );

  glBegin( GL_QUADS );			// Draw textured polygon
  glTexCoord2f( 0, 0 );
  glVertex2fv( v[ 0 ] );
  glTexCoord2f( 0, 1 );
  glVertex2fv( v[ 3 ] );
  glTexCoord2f( 1, 1 );
  glVertex2fv( v[ 2 ] );
  glTexCoord2f( 1, 0 );
  glVertex2fv( v[ 1 ] );
	glEnd();
}					// End routine draw_stroke


void draw_stroke_choptex( float r, float g, float b, float size, int ID )

  //  Draw a brush stroke centered about (0,0) with width 1, height 4;
  //  user can modify position with translate, size with scale
  //
  //  r, g, b:  Underlying polygon color to modulate
  //  ID:       ID of texture to layer on stroke (defaults to -1)
{

 float  v[ 4 ][ 2 ] = {		// Stroke vertices
    { -0.5, -0.5 },   //  3      2
    {  0.5, -0.5 },   //   x    x
    {  0.5,  0.5 },   //   x    x
    { -0.5,  0.5 }    //  0      1
  };
 // static int  prev_ID = -1;		// Previous bound texture ID

  if ( ID < 0 || ID >= tex_n ) {	// Choose random ID if given ID bad
    ID = int( rand_num() * tex_n );
    ID = ( ID >= tex_n ) ? tex_n - 1 : ID;
  }




  if ( ID != prev_ID ) {
    glBindTexture( GL_TEXTURE_2D, tex_ID[ ID ] );
    prev_ID = ID;
  }

  float t[ 4 ][ 2 ] = {    // Texture vertices
    { 0, 0 },
    { 0, 1 },
    { 1, 1 },
    { 1, 0 }
  };

  for ( int i = 0; i < 4 ; i++ ){
    for ( int j = 0; j < 2 ; j++ ){
      t[ i ][ j ] = t[ i ][ j ]*size;
    }
  }


  glColor3f( r, g, b );

  glBegin( GL_QUADS );			// Draw textured polygon
  glTexCoord2fv( t[ 0 ] );
  glVertex2fv( v[ 0 ] );

  glTexCoord2fv( t[ 1 ] );
  glVertex2fv( v[ 3 ] );
  
  glTexCoord2fv( t[ 2 ] );
  glVertex2fv( v[ 2 ] );
  
  glTexCoord2fv( t[ 3 ] );
  glVertex2fv( v[ 1 ] );
	
	glEnd();
}					// End routine draw_stroke_choptex


void draw_trapezoid_stroke( float r, float g, float b, int ID )

  //  Draw a brush stroke centered about (0,0) with width 1, height 4;
  //  user can modify position with translate, size with scale
  //
  //  r, g, b:  Underlying polygon color to modulate
  //  ID:       ID of texture to layer on stroke (defaults to -1)
{
  float  v[ 4 ][ 2 ] = {		// Stroke vertices
    { -0.5, -0.5 },   //  3      2
    {  0.5, -0.5 },   //   x    x
    {  0.25,  0.5 },   //   x    x
    { -0.25,  0.5 }    //  0      1
  };

 // static int  prev_ID = -1;		// Previous bound texture ID

  if ( ID < 0 || ID >= tex_n ) {	// Choose random ID if given ID bad
    ID = int( rand_num() * tex_n );
    ID = ( ID >= tex_n ) ? tex_n - 1 : ID;
  }

  if ( ID != prev_ID ) {
    glBindTexture( GL_TEXTURE_2D, tex_ID[ ID ] );
    prev_ID = ID;
  }

  glColor3f( r, g, b );

  glBegin( GL_QUADS );			// Draw textured polygon
  glTexCoord2f( 0, 0 );
  glVertex2fv( v[ 0 ] );
  glTexCoord2f( 0, 1 );
  glVertex2fv( v[ 3 ] );
  glTexCoord2f( 1, 1 );
  glVertex2fv( v[ 2 ] );
  glTexCoord2f( 1, 0 );
  glVertex2fv( v[ 1 ] );
	glEnd();
}					// End routine draw_trapezoid_stroke


void draw_stroke_no_tex( float r[], float g[], float b[])

  //  Draw a brush stroke centered about (0,0) with width 1, height 4;
  //  user can modify position with translate, size with scale
  //
  //  r, g, b:  Underlying polygon color to modulate
  //  ID:       ID of texture to layer on stroke
  //  size:     Fraction of the texture to be used 
{
  float  v[ 4 ][ 2 ] = {		// Stroke vertices
    { -0.5, -0.5 },   //  3      2
    {  0.5, -0.5 },   //   x    x
    {  0.5,  0.5 },   //   x    x
    { -0.5,  0.5 }    //  0      1
  };
  
  glBegin( GL_POLYGON );			// Draw textured polygon
    //glTexCoord2f( 0, 0 );

    glColor3f( r[0], g[0], b[0] );
    glVertex2fv( v[ 0 ] );
    

    glColor3f( r[3], g[3], b[3] );
    glVertex2fv( v[ 3 ] );
    

    glColor3f( r[2], g[2], b[2] );
    glVertex2fv( v[ 2 ] );
    

    glColor3f( r[1], g[1], b[1] );
    glVertex2fv( v[ 1 ] );
	glEnd();
}					// End routine draw_stroke_no_tex



void draw_stroke_smooth_shading( float r[], float g[], float b[], int ID, float size )

  //  Draw a brush stroke centered about (0,0) with width 1, height 4;
  //  user can modify position with translate, size with scale
  //
  //  r, g, b:  Underlying polygon color to modulate
  //  ID:       ID of texture to layer on stroke
  //  size:     Fraction of the texture to be used 
{
  float  v[ 4 ][ 2 ] = {		// Stroke vertices
    { -0.5, -0.5 },   //  3      2
    {  0.5, -0.5 },   //   x    x
    {  0.5,  0.5 },   //   x    x
    { -0.5,  0.5 }    //  0      1
  };

  float t[ 4 ][ 2 ] = {    // Texture vertices
    { 0, 0 },
    { 0, 1 },
    { 1, 1 },
    { 1, 0 }
  };

  for ( int i = 0; i < 4 ; i++ ){
    for ( int j = 0; j < 2 ; j++ ){
      t[ i ][ j ] = t[ i ][ j ]*size;
    }
  }


 // static int  prev_ID = -1;		// Previous bound texture ID

  if ( ID < 0 || ID >= tex_n ) {	// Choose random ID if given ID bad
    ID = int( rand_num() * tex_n );
    ID = ( ID >= tex_n ) ? tex_n - 1 : ID;
  }

  if ( ID != prev_ID ) {
    glBindTexture( GL_TEXTURE_2D, tex_ID[ ID ] );
    prev_ID = ID;
  }
  
  glBegin( GL_QUADS );			// Draw textured polygon
    //glTexCoord2f( 0, 0 );
    glTexCoord2fv( t[ 0 ] );
    glColor3f( r[0], g[0], b[0] );
    glVertex2fv( v[ 0 ] );
    
    glTexCoord2fv( t[ 1 ] );
    glColor3f( r[3], g[3], b[3] );
    glVertex2fv( v[ 3 ] );
    
    glTexCoord2fv( t[ 2 ] );
    glColor3f( r[2], g[2], b[2] );
    glVertex2fv( v[ 2 ] );
    
    glTexCoord2fv( t[ 3 ] );
    glColor3f( r[1], g[1], b[1] );
    glVertex2fv( v[ 1 ] );
	glEnd();
}					// End routine draw_stroke_smooth_shading


void draw_variegated_stroke( stroke_s stroke, int ID )

  //  Draw a brush stroke centered about (0,0) with width 1, height 4;
  //  user can modify position with translate, size with scale
  //
  //  r, g, b:  Underlying polygon color to modulate
  //  ID:       ID of texture to layer on stroke (defaults to -1)
{
  float  v[ 6 ][ 2 ] = {		// Stroke vertices
                               // 1     3     5
    { -0.5, -0.5 },            //  x    x    x 
    { -0.5,  0.5 },            //
    {  0.0, -0.5 },            //           
    {  0.0,  0.5 },            //
    {  0.5, -0.5 },            //  x    x    x
    {  0.5,  0.5 }             // 0     2     4
  };                         
  
  //static int  prev_ID = -1;		// Previous bound texture ID
  

  if ( ID < 0 || ID >= tex_n ) {	// Choose random ID if given ID bad
    ID = int( rand_num() * tex_n );
    ID = ( ID >= tex_n ) ? tex_n - 1 : ID;
  }

  if ( ID != prev_ID ) {
    glBindTexture( GL_TEXTURE_2D, tex_ID[ ID ] );
    prev_ID = ID;
  }

  glColor3f( stroke.rr[0], stroke.gg[0], stroke.bb[0] );
/*
  glBegin( GL_QUADS_STRIPS );			// Draw textured polygon
  glTexCoord2f( 0, 0 );
  glVertex2fv( v[ 0 ] );
  glTexCoord2f( 0, 1 );
  glVertex2fv( v[ 1 ] );
  glVertex2fv( v[ 3 ] );
  glVertex2fv( v[ 2 ] );
  glTexCoord2f( 1, 1 );
  glVertex2fv( v[ 5 ] );
  glTexCoord2f( 1, 0 );
  glVertex2fv( v[ 4 ] );
	glEnd();
*/
  glBegin( GL_QUADS );
  glTexCoord2f( 0, 0 );
  glVertex2fv( v[ 0 ] );
   glTexCoord2f( 0, 1 ) ;
  glVertex2fv( v[ 1 ] );
  glTexCoord2f( .5, 1);
  glVertex2fv( v[ 3 ] );
  glTexCoord2f( .5, 0 );
  glVertex2fv( v[ 2 ] );

  glColor3f( stroke.rr[ 1 ], stroke.gg[ 1 ], stroke.bb[ 1 ] );
  
  glTexCoord2f( 0.5, 0.0 );
  glVertex2fv( v[ 2 ] );
  glTexCoord2f( 0.5, 1 );
  glVertex2fv( v[ 3 ] );
  glTexCoord2f(  1, 1 );
  glVertex2fv( v[ 5 ] );
  glTexCoord2f( 1, 0 );
  glVertex2fv( v[ 4 ] );
  glEnd();
}					// End routine draw_variegated_stroke

void draw_3tone_bezier( stroke_s stroke, int ID )
  //  Draw a bezier stroke with by calling draw_bezier on 3 sets of abutting control points;
  //  Creates same variegation effect as draw_3tone_stroke does for a rectangular stroke.
  //  user can modify position with translate, and rotate the stroke, but the stroke is prescaled.
  //
  //  stroke:   Struct which provides color information for variegation (r, g, b, rr,gg, and bb).
  //  ID:       ID of texture to layer on stroke (defaults to -1)
{
  stroke_s tmp_st;        // stroke to hold controlpt array for drawing each portion of the stroke
  float texpts[2][2][2] = {
                            {{0,0},{0,1}},
                            {{1,0},{1,1}}
                          };// texture points, set to draw_bezier with the correct portion of the texture  
  float bandw; // default variegation width
  float wigglew; // maximum random shift (left or right) from default
  float r1, r2;  //randum numbers for determining amount and direction of wiggle
  float left, right; // boundaries for left and right variegation in terms distance from 0 (keep for texture mapping)
  float leftv, rightv; // boundaries for left and right variegation in terms distance from -5 (since the stroke vertices start with x = -0.5)
  float diff;         // rightv - leftv to find width of middle strip
  bandw = .125;  //.33
  wigglew = bandw*.5;

  
  // Set left variegation boundary
  r1 = rand_num()*wigglew;//.1;          // Randomly choose how much wiggle
  r2 = ( rand_num() < 0.5 ) ? -1 : 1;    // Randomly choose left shift or right shift. (neg or pos)
  left = bandw + (r1*r2);
 
  // Set right variegation boundary
  r1 = rand_num()*wigglew;//.1;          // Randomly choose how much wiggle
  r2 = ( rand_num() < 0.5 ) ? -1 : 1;    // Randomly choose left shift or right shift. (neg or pos)
  right = 1 - bandw + (r1*r2); 

  // Shift left for vertex offset from 0.
  leftv = -0.5 + left; rightv = -0.5 + right;                          
  
  // Set up left strip
  // Set control points for left strip
  for( i = 0; i < 4; i++ ){
    for( j = 0; j < 4; j++ ){
      tmp_st.controlpts[i][j][0] = stroke.controlpts[i][0][0] + j*left*.333333333333;
      tmp_st.controlpts[i][j][1] = stroke.controlpts[i][j][1];
      tmp_st.controlpts[i][j][2] = stroke.controlpts[i][j][2];
    }
  }

  // Adjust texture points for left strip
  //    {{0,0},{0,1}},
  //    {{left,0},{left,1}}
   texpts[1][0][0] = left;   
   texpts[1][1][0] = left;
 
   // Draw left strip with "left" color
   tmp_st.r = stroke.rr[0];
   tmp_st.g = stroke.gg[0];
   tmp_st.b = stroke.bb[0];
   bla = true;
   draw_bezier( tmp_st, texpts, ID );
   bla = false;


  // Set up middle strip
  // Set control points for left strip
  diff = right - left;  //width of middle strip
  for( i = 0; i < 4; i++ ){
    for( j = 0; j < 4; j++ ){
      tmp_st.controlpts[i][j][0] = left + stroke.controlpts[i][0][0] + j*diff*.333333333333;
      tmp_st.controlpts[i][j][1] = stroke.controlpts[i][j][1];
      tmp_st.controlpts[i][j][2] = stroke.controlpts[i][j][2];
    }
  }

  // Adjust texture points for middle strip
  //    {{left,0},{left,1}},
  //    {{right,0},{right,1}}
  texpts[0][0][0] = left;   
  texpts[0][1][0] = left;
  texpts[1][0][0] = right;   
  texpts[1][1][0] = right;

   // Draw middle strip
   tmp_st.r = stroke.r;
   tmp_st.g = stroke.g;
   tmp_st.b = stroke.b;
   draw_bezier( tmp_st, texpts, ID );


  // Set up right strip
  // Set control points for left strip
  for( i = 0; i < 4; i++ ){
    for( j = 0; j < 4; j++ ){
      tmp_st.controlpts[i][j][0] = right + stroke.controlpts[i][0][0] + j*(1-right)*.333333333333;
      tmp_st.controlpts[i][j][1] = stroke.controlpts[i][j][1];
      tmp_st.controlpts[i][j][2] = stroke.controlpts[i][j][2];
    }
  }

  // Adjust texture points for right strip
  //    {{right,0},{right,1}},
  //    {{1,0},{1,1}}
  texpts[0][0][0] = right;   
  texpts[0][1][0] = right;
  texpts[1][0][0] = 1;   
  texpts[1][1][0] = 1;
 
   // Draw right strip
   tmp_st.r = stroke.rr[1];
   tmp_st.g = stroke.gg[1];
   tmp_st.b = stroke.bb[1];
   draw_bezier( tmp_st, texpts, ID );

}  // end routine draw_3tone_bezier

void draw_3tone_stroke( stroke_s stroke, int ID )

  //  Draw a brush stroke centered about (0,0) with 3 vertical stripes for variegation;
  //  user can modify position with translate, size with scale
  //
  //  stroke:   Struct which provides color information for variegation (r, g, b, rr,gg, and bb).
  //  ID:       ID of texture to layer on stroke (defaults to -1)
{
  float bandw; // default variegation width
  float wigglew; // maximum random shift (left or right) from default
  float r1, r2;  //randum numbers for determining amount and direction of wiggle
  float left, right; // boundaries for left and right variegation in terms distance from 0 (keep for texture mapping)
  float leftv, rightv; // boundaries for left and right variegation in terms distance from -5 (since the stroke vertices start with x = -0.5)
  bandw = .125;  //.33
  wigglew = bandw*.5;
  
  // Set left variegation boundary
  r1 = rand_num()*wigglew;//.1;          // Randomly choose how much wiggle
  r2 = ( rand_num() < 0.5 ) ? -1 : 1;    // Randomly choose left shift or right shift. (neg or pos)
  left = bandw + (r1*r2);
 
  // Set right variegation boundary
  r1 = rand_num()*wigglew;//.1;          // Randomly choose how much wiggle
  r2 = ( rand_num() < 0.5 ) ? -1 : 1;    // Randomly choose left shift or right shift. (neg or pos)
  right = 1 - bandw + (r1*r2); 

  // Shift left for vertex offset from 0.
  leftv = -0.5 + left; rightv = -0.5 + right;    
  
  float  v[ 8 ][ 2 ] = {		// Stroke vertices
                               // 1     3    5    7
    { -0.5, -0.5 },            //  x    x    x   x
    { -0.5,  0.5 },            //
    {  leftv, -0.5 },          //           
    {  leftv,  0.5 },          //
    {  rightv,  -0.5 },        //
    {  rightv,  0.5 },         //
    {  0.5, -0.5 },            //  x    x    x   x
    {  0.5,  0.5 }             // 0     2    4    6
  };                         
  
  //static int  prev_ID = -1;		// Previous bound texture ID
  
 // int temp_id = ID;

  if ( ID < 0 || ID >= tex_n ) {	// Choose random ID if given ID bad
    ID = int( rand_num() * tex_n );
    ID = ( ID >= tex_n ) ? tex_n - 1 : ID;
  }

 // printf("In draw_stroke, the attempted id, temp_id = %d and the chosen ID = %d\n", temp_id, ID);

  if ( ID != prev_ID ) {
    glBindTexture( GL_TEXTURE_2D, tex_ID[ ID ] );
    prev_ID = ID;
  }

  glColor3f( stroke.rr[0], stroke.gg[0], stroke.bb[0] );

  glBegin( GL_QUADS );
    glTexCoord2f( 0, 0 );
    glVertex2fv( v[ 0 ] );
    glTexCoord2f( 0, 1 ) ;
    glVertex2fv( v[ 1 ] );
    glTexCoord2f( left, 1);
    glVertex2fv( v[ 3 ] );
    glTexCoord2f( left, 0 );
    glVertex2fv( v[ 2 ] );

    glColor3f( stroke.r, stroke.g, stroke.b );
    
    glTexCoord2f( left, 0.0 );
    glVertex2fv( v[ 2 ] );
    glTexCoord2f( left, 1 );
    glVertex2fv( v[ 3 ] );
    glTexCoord2f( right, 1 );
    glVertex2fv( v[ 5 ] );
    glTexCoord2f( right, 0 );
    glVertex2fv( v[ 4 ] );
    
    glColor3f( stroke.rr[ 1 ], stroke.gg[ 1 ], stroke.bb[ 1 ] );
    
    glTexCoord2f( right, 0.0 );
    glVertex2fv( v[ 4 ] );
    glTexCoord2f( right, 1 );
    glVertex2fv( v[ 5 ] );
    glTexCoord2f(  1, 1 );
    glVertex2fv( v[ 7 ] );
    glTexCoord2f( 1, 0 );
    glVertex2fv( v[ 6 ] ); 
  glEnd();
}					// End routine draw_variegated_stroke



int get_id( int number)
{
  return tex_ID[number];
  }

void init_nurbs(void)
{
    theNurb = gluNewNurbsRenderer();
    theobj = gluNewQuadric(); //***testing
    gluNurbsProperty(theNurb, GLU_SAMPLING_TOLERANCE, 25.0);
    gluNurbsProperty(theNurb, GLU_DISPLAY_MODE, GLU_FILL);   
}

void init_stroke( string fname )

  //  This routine initializes the proper GL texture binding, by
  //  reading a stroke texture from the given file
  //
  //  fname:  Name of stroke texture file
{
  char buff[256];
  sprintf(buff, "%s", (char*)fname);
  if ( !read_tex_map( fname ) ) {
	  printf( "read_tex_map failed for %s\n", buff);
    return;
  }

  //  Note:  glBindTexture IS NECESSARY here, because the first call
  //         with a new texture ID creates the texture (as opposed to the
  //         glBindTexture call in draw_stroke, which activates the
  //         texture)

  glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );
  glGenTextures( 1, &tex_ID[ tex_n ] );
  glBindTexture( GL_TEXTURE_2D, tex_ID[ tex_n ] );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );

  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, tex_w[ tex_n ], tex_h[ tex_n ],
                0, GL_RGBA, GL_UNSIGNED_BYTE, tex_map );

  glEnable( GL_BLEND );
  glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
  delete [] tex_map;
  tex_n++;
}					// End routine init_stroke

void make_control_points(float*& points, int n, int m, float* vert_sep, float*
hor_sep, float* offset )
// Generates arrays of control points
	// n				 # of control points in u (vert) direction
	// m				 # of control points in v (hor) direction 					 
	// vert_sep		         Vertical separation btwn control pts
	// hor_sep			 Horizontal separation btwn control pts 
	// offset			 Horizontal offset of leftmost ctlpt zero
{
  int size = n*m*3;
  points = new float[ size ];
  
  for( i = 0 ; i < size ; i++ )
  {
    switch ( i%3 ) {
    case 0:
     if ( (i % ( m * 3 )) == 0 )
      {
        points[ i ] = offset[ i ];
      }else
      {
        points[ i ] = points[ i - 3 ] + hor_sep[ i ];
      }
      break;
    
    case 1:
      if (i == 1 )
      { 
        points[ i ] = -(sum_vert_sep/2);
      }else if ( i% ( m* 3) == 1 )
      {
        points[ i ] = points[ i - 3 ] + vert_sep[ i ];
      }else
      {
        points[ i ] = points[ i - 3 ];
      }
      break;   
    case 2:
      points[ i ] = 0; 
      break;
    }
  }
   delete [] hor_sep;
   delete [] vert_sep;  
   delete [] offset;  
 
}  			//end routine make_control_points


static int read_tex_map( string& fname )

  //  This routine reads the texture map from an input file, of the
  //  format:
  //
  //    row col
  //    RGBA_1_1 RGBA_1_2 ... RGBA_0_col
  //    ...
  //    RGBA_row_1 RGBA_row_2 ... RGBA_row_col
{
  int         count = 0;		// Component counter
  filereader  fp;			// Input file stream
  int         i, j;			// Loop counters
  string      tok[ 512 ];		// Tokens on current line
  
//string      tok[ 4096 ];		// Tokens on current line
  
  if ( !fp.open( fname, 'r' ) ) {	// Open input file
    return 0;
  }

  fp.next( tok, 512 );			// Read row, col counts
	//fp.next( tok, 4096 );
  tex_h[ tex_n ] = int( tok[ 0 ] );
  tex_w[ tex_n ] = int( tok[ 1 ] );

  if ( !check_size( tex_w[ tex_n ], tex_h[ tex_n ] ) ) {
    cerr << "Error:  " << tex_w[ tex_n ];
    cerr << " or " << tex_h[ tex_n ] << " not a power of 2\n";
    return 0;
  }
  cerr << "Good:  " << tex_w[ tex_n ];
  cerr << " and " << tex_h[ tex_n ] << " are a power of 2\n";
  tex_map = new GLubyte[ tex_h[ tex_n ] * tex_w[ tex_n ] * 4 ];

  for( i = 0; i < tex_h[ tex_n ]; i++ ) {
    fp.next( tok, 512 );	
    //fp.next( tok, 4096 );
    for( j = 0; j < tex_w[ tex_n ]; j++ ) {
      tex_map[ count++ ] = (GLubyte) int( tok[ ( j * 4 ) ] );
      tex_map[ count++ ] = (GLubyte) int( tok[ ( j * 4 ) + 1 ] );
      tex_map[ count++ ] = (GLubyte) int( tok[ ( j * 4 ) + 2 ] );
      tex_map[ count++ ] = (GLubyte) int( tok[ ( j * 4 ) + 3 ] );
   } 
  }
  fp.close();
  return 1;
}					// End routine read_tex_map

void setup_nurbs( int num_up, int num_vp, float scale ) 
{
   int u_clamp_size;
   int v_clamp_size;
   
   vert_sep = new float[ num_up*num_vp*3 ];
   hor_sep = new float[ num_up*num_vp*3 ];
   offset = new float[ num_up*num_vp*3 ];
   
   
   sum_hor_sep = 0;
   sum_vert_sep = 0;
   for( i = 0 ; i < num_up*num_vp*3; i ++ )
   {
      if ( num_up <= 4 ){
        hor_sep[ i ] = 2/(float)num_vp;      
      }else if ( num_up <= 5 ){
        hor_sep[ i ] = 3/(float)num_vp;
      }else if ( num_up <= 6 ){
        hor_sep[ i ] = 4/(float)num_vp;
      }else if ( num_up <= 7 ){
        hor_sep[ i ] = 5/(float)num_vp;
      }else {
        hor_sep[ i ] = 6/(float)num_vp;
      }
   sum_hor_sep = sum_hor_sep + hor_sep[ i ];	 
   //cerr<<"i = " <<i<<"  hor_sep ="<<hor_sep[i]<<endl;
   //cerr<<"i = " <<i<<"  sum_hor_sep ="<<sum_hor_sep<<endl;
   }
   
   sum_hor_sep = sum_hor_sep/(3.0*(float)num_up);
     //cerr<<endl<<endl;   
   sum_vert_sep = 4 * sum_hor_sep;   
   for( i = 0 ; i < num_up*num_vp*3 ; i ++ )
   {
     vert_sep[ i ] = sum_vert_sep/( (float)num_up - 1.0 );
     
     if (i % (num_vp*3) == 0 && i != 0)
     {
  //     sum_vert_sep = sum_vert_sep + vert_sep[ i ];
       //cerr<<"i = " <<i<<"  sum_vert_sep ="<<sum_vert_sep<<endl;
     }

     //cerr<<"i = " <<i<<"  vert_sep ="<<vert_sep[i]<<endl;
  // cerr<<"i = " <<i<<"  sum_vert_sep ="<<sum_vert_sep<<endl;
   }
   
   // offset[0] = 0;
   for(i = 0 ; i < num_up*num_vp * 3 ; i ++ )
   { 
     if (i% (2*num_vp) == 0 ) 
     { 
       offset[ i ] = -sum_hor_sep;
     }else
     {
       offset[ i ] = 0;
     }
   }
    num_u_knots = num_up + 3 + 1;
    num_v_knots = num_vp + 3 + 1;
    make_control_points( control_points, num_up, num_vp, vert_sep, hor_sep, offset );
    
    if ( num_up < 4 )
    { 
      u_clamp_size = num_up;
    }else
    {
      u_clamp_size = 4;
    }
    
    if ( num_vp < 4 )
    { 
      v_clamp_size = 2;
    }else
    {
      v_clamp_size = 4;
    } 
    make_knots( u_knots, num_u_knots, u_clamp_size);
    make_knots( v_knots, num_v_knots, v_clamp_size );
    make_knots( texture_knots, 4, 2 );
  
}		//end setup_nurbs


void start_stroke()

  //  This routine binds the proper textures in preparation for
  //  rendering strokes
{
  glDepthMask( GL_FALSE );

  glEnable( GL_TEXTURE_2D );
  glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
  //glBindTexture( GL_TEXTURE_2D, tex_ID );
}					// End routine start_stroke()


void stop_stroke()
  //  This routine disables texture mapping after strokes are rendered
{
  glDisable( GL_TEXTURE_2D );
  glDepthMask( GL_TRUE );
}					// End routine stop_stroke()
