//    Prototypes for HIGHLIGHT.H

#ifndef _highlight_h
#define _highlight_h

#include "paint.h"
#include "grid_file.h"

int  highlight(int** &status_array, int frame, float hfactor, int num_hig, int num_attr, int w, int h, float min, float max, grid_file* gf);

#endif
