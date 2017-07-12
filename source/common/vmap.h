//  VMAP.H
//    Prototypes for VMAP.CPP

#ifndef _vmap_h
#define _vmap_h

#include "global.h"
#ifdef NAMESPACE
  #include<iostream> 
  using namespace std;
#else
  #include<iostream.h> 
#endif

#include <stdlib.h>
#include <tcl.h>
#include <tk.h>
#include "attr_map.h"
#include "bin_file.h"
#include "global.h"
#include "str.h"

#include "bin_file.h"
#include "togl.h"

int   vmap_invoke( Togl *, bin_file * );
void  vmap_post_init( Tcl_Interp * );

#endif
