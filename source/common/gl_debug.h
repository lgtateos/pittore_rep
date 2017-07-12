#ifndef GL_DEBUG_H
#define GL_DEBUG_H

#ifdef WIN32
#include <windows.h>
#endif
#include "global.h"
#include <GL/glu.h>
#include <GL/gl.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef NAMESPACE
  #include <iostream>
  using namespace std;
//using std::istream;
//using std::ostream;

#else
#if defined(MAC) || defined(MACOSX)
#include <iostream.h>
#else
#include <iostream.h>
#include <fstream.h>
#endif
#endif

void   check_opengl_errors(const char *msg);
void   GetMatrices(const char *msg );
void   report_opengl_stacks( );
void   report_tranformations( float tr[], float sc [], float rot [] );




#endif
