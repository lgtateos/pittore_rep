#ifndef GL_DEBUG_CPP
#define GL_DEBUG_CPP

#include "gl_debug.h"

void report_opengl_stacks()
{
    GLint depth;

    glGetIntegerv(GL_PROJECTION_STACK_DEPTH, &depth);
    cerr << "   Projection stack depth = " << depth;
    glGetIntegerv(GL_MAX_PROJECTION_STACK_DEPTH, &depth);
    cerr << " (" << depth << " max)" << endl;

    glGetIntegerv(GL_MODELVIEW_STACK_DEPTH, &depth);
    cerr << "   ModelView stack depth = " << depth;
    glGetIntegerv(GL_MAX_MODELVIEW_STACK_DEPTH, &depth);
    cerr << " (" << depth << " max)" << endl;

    glGetIntegerv(GL_TEXTURE_STACK_DEPTH, &depth);
    cerr << "   Texture stack depth = " << depth;
    glGetIntegerv(GL_MAX_TEXTURE_STACK_DEPTH, &depth);
    cerr << " (" << depth << " max)" << endl;
}

void check_opengl_errors(const char *msg)
{
    bool stack_error = false;

    for(GLenum err=glGetError(); err!=GL_NO_ERROR; err=glGetError())
    {
        cerr << "GL ERROR ";
        if( msg ) cerr << msg;
        cerr << ": " << (const char *)gluErrorString(err) << endl;

        if( err==GL_STACK_OVERFLOW || err==GL_STACK_UNDERFLOW )
            stack_error = true;
    }

    if( stack_error )  report_opengl_stacks();
}

void GetMatrices( const char *msg )
{
  GLfloat mv[ 16 ];
  GLint viewport[ 4 ];
  glGetIntegerv( GL_VIEWPORT, viewport );
  printf("%s\n", msg );
  printf("Viewport Matrix:\n");
  printf("[%d, %d, %d, %d]\n\n", viewport[0], viewport[1], viewport[2], 
	 viewport[3]);

  glGetFloatv(GL_PROJECTION_MATRIX, mv);
  printf("Projection Matrix:\n");
  printf("[ %3.5f %3.5f %3.5f %3.5f ]\n", mv[0], mv[4], mv[8], mv[12]);
  printf("[ %3.5f %3.5f %3.5f %3.5f ]\n", mv[1], mv[5], mv[9], mv[13]);
  printf("[ %3.5f %3.5f %3.5f %3.5f ]\n", mv[2], mv[6], mv[10], mv[14]);
  printf("[ %3.5f %3.5f %3.5f %3.5f ]\n", mv[3], mv[7], mv[11], mv[15]);
  printf("\n");
    
  glGetFloatv(GL_MODELVIEW_MATRIX, mv);
  printf("Modelview Matrix:\n");
  printf("[ %3.5f %3.5f %3.5f %3.5f ]\n", mv[0], mv[4], mv[8], mv[12]);
  printf("[ %3.5f %3.5f %3.5f %3.5f ]\n", mv[1], mv[5], mv[9], mv[13]);
  printf("[ %3.5f %3.5f %3.5f %3.5f ]\n", mv[2], mv[6], mv[10], mv[14]);
  printf("[ %3.5f %3.5f %3.5f %3.5f ]\n", mv[3], mv[7], mv[11], mv[15]);
  printf("\n");
  printf("\n");
}

void report_tranformations( float tr[], float sc[], float rot[] ){
	printf("Transform x y z: %f %f %f\n", tr[0], tr[1], tr[2]);
    printf("Scale x y z: %f %f %f\n", sc[0], sc[1], sc[2]);
	printf("Rotate x y z: %f %f %f\n", rot[0], rot[1], rot[2]);
}


#endif
