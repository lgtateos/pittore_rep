//  QUADTREE.H
//    Structs and class definition for QUADTREE.CPP

#ifndef _quadtree_h
#define _quadtree_h

#include "togl.h"

struct qtree_elem {			// Generic quadtree element
  float  x, y;				// Position in 2D space
  int    ID;				// Element ID (defined by caller)
};

struct qtree_node {			// Node within quadtree
  qtree_node *child[ 4 ];		// Child nodes (NULL if no child)
  qtree_elem *e;			// Elem in node (only used in child)
  float       max[ 2 ];			// Maximum x, y values
  float       min[ 2 ];			// Minimum x, y values
  int         n;			// Number of elem in node
};


class quadtree {
private:
  qtree_node *root;			// Root node of quadtree


  int         clip_code( float, float, float, float, float, float );
  void        comp_midpoint( float *, float *, float * );
  void        comp_viewport( float *, float, float, float&, float& );
  int         CS_clip( float, float, float, float );
  int         display_node( qtree_node *, float *, qtree_elem *, int&, int );
  void        find_min_max( qtree_elem *, int, float *, float * );
  qtree_node *build_node(
                qtree_elem *, int&, int, float, float, float, float );
  void        free_node( qtree_node * );

public:
  quadtree();
 ~quadtree();

  int         active();
  int         build( qtree_elem *, int, int max_e = 16 );
  int         display( qtree_elem *, int&, int );
  void        flush();
};

#endif
