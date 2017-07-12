#if !defined(SGI) && !defined(SOLARIS) && !defined(LINUX)
#include <windows.h>
#endif

#include <GL/gl.h>
#include <GL/glu.h>
#include "lighting.h"

/*--------------------------------------------------------------------------*/
/*  LIGHTING.CPP								                                            */
/*    Routines to enable and initialize lighting                            */
/*    								                                                      */
/*		                                                    							    */
/*- Modification History: --------------------------------------------------*/
/*  When:	Who:			Comments:			                                          */ 
/*									                                                        */
/*  9-Sep-06		Laura G. Tateosian   Initial implementation		              */
/*--------------------------------------------------------------------------*/

static void enable_lighting( void );

void enable_lighting(){
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glEnable( GL_COLOR_MATERIAL );
  glEnable(GL_DEPTH_TEST);
}

void init_lighting()
{
 enable_lighting();

 GLfloat lmodel_ambient[] = {.1, .1, .1, 1.0};

 GLfloat light_position0[] = {  100.0, 5.0,  10.0, 1.0 };
 //GLfloat light_position1[] = {  4, 4,  1, 0.0 };
 //GLfloat light_position2[] = {  0.0,     0.7071,  0.7071, 0.0 };
 //GLfloat light_position3[] = {  0.0,     0.0,     1.0000, 0.0 };

 GLfloat light_ambient[]   = { 0.2, 0.2, 0.2, 1.0 };
 GLfloat zeros[]           = { 0.0, 0.0, 0.0, 0.0 };
 GLfloat light_diffuse[]   = { 0.7, 0.7, 0.7, 1.0 };
 GLfloat light_specular[]  = { 0.1, 0.1, 0.1, 1.0 };

 GLfloat material_amb[]    = { 0.2, 0.2, 0.2, 1.0 };
 GLfloat material_dif[]    = { 1.0, 0.0, 0.0, 1.0 };

 glLightModelfv( GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);
 glMaterialfv( GL_FRONT, GL_AMBIENT, material_amb );
 glMaterialfv( GL_FRONT, GL_DIFFUSE, material_dif );

 glShadeModel( GL_SMOOTH );

 glLightfv( GL_LIGHT0, GL_POSITION, light_position0 );
 glLightfv( GL_LIGHT0, GL_AMBIENT,  light_ambient );
 glLightfv( GL_LIGHT0, GL_DIFFUSE,  light_diffuse );
 glLightfv( GL_LIGHT0, GL_SPECULAR, light_specular );

/* glLightfv( GL_LIGHT1, GL_POSITION, light_position1 );
 glLightfv( GL_LIGHT1, GL_AMBIENT,  light_ambient );
 glLightfv( GL_LIGHT1, GL_DIFFUSE,  light_diffuse );
 glLightfv( GL_LIGHT1, GL_SPECULAR, light_specular);
*/

 glEnable( GL_LIGHT0 );
 //glEnable( GL_LIGHT1 );
                  
 glColorMaterial( GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE );// specifies the faces to be updated, which material properies  are updated
                                                               // a change to the current color with glColor updates the specified material properties
}