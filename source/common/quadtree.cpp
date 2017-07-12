#ifdef _M_IX86
#include <windows.h>
#endif
#include "global.h"
#ifdef NAMESPACE
  #include <fstream>
  using namespace std;
#else
  #include <fstream.h>
#endif
#include <stdlib.h>
#include <GL/gl.h>
#include "quadtree.h"
#include "global.h"

/*--------------------------------------------------------------------------*/
/*  QUADTREE.CPP							    */
/*    This module implements a 2D quadtree that can subdivide generic data  */
/*    into a hierarchy of quads based on spatial density		    */
/*									    */
/*- Modification History: --------------------------------------------------*/
/*  When:	Who:			Comments:			    */
/*									    */
/*  17-May-01	Christopher G. Healey	Initial implementation		    */
/*  30-Mar-02	Christopher G. Healey	Fixed potential bug in display();   */
/*					we now initialize n=0 to ensure a   */
/*					proper result when no part of the   */
/*					quadtree is in view		    */
/*  30-May-02	Christopher G. Healey	Fixed display_node() to work more   */
/*					efficiently by correctly checking   */
/*					if a node is in view, THEN doing    */
/*					recursive processing of children    */
/*  29-Jun-03	Christopher G. Healey	Fixed catastrophic error, in	    */
/*					display_node(), quadtree node was   */
/*					assumed to be rectangular on-screen */
/*					but this is true only for ortho-    */
/*					graphic (i.e., look-down) project;  */
/*					now properly compute postiion of    */
/*					all 4 node corners before clipping  */
/*--------------------------------------------------------------------------*/


//  Note:  All "inline" methods must be declared before they are first
//         called (required by IRIX compiler)

inline int quadtree::clip_code(
  float x_min, float y_min, float x_max, float y_max, float x, float y )

  //  This routine returns a Cohen-Sutherland bit-code for a given
  //  (element) position and bounding region, recall:
  //
  //  - bit 4: 1 if point above viewport, 0 otherwise
  //  - bit 3: 1 if point below viewport, 0 otherwise
  //  - bit 2: 1 if point to right of viewport, 0 otherwise
  //  - bit 1: 1 if point to left of viewport, 0 otherwise
  //
  //  x_min, y_min:  Lower-left corner of node
  //  x_max, y_max:  Upper-right corner of node
  //  x, y:          Position to test
{
  int  code = 0;			// Bit-code for endpoint position


  if ( y > y_max )			// Check y-extent
    code |= 8;
  else if ( y < y_min )
    code |= 4;

  if ( x > x_max )			// Check x-extent
    code |= 2;
  else if ( x < x_min )
    code |= 1;

  return code;
}					// End method clip_code


inline void quadtree::comp_midpoint( float min[], float max[], float mid[] )

  //  This private routine computes the midpoint of a 2D node
  //
  //  min:  Minimum x, y values
  //  max:  Maximum x, y values
  //  mid:  Storage for midpoint position
{
    mid[ 0 ] = min[ 0 ] + ( max[ 0 ] - min[ 0 ] ) / 2.0;
    mid[ 1 ] = min[ 1 ] + ( max[ 1 ] - min[ 1 ] ) / 2.0;
}					// End method comp_midpoint


inline void quadtree::comp_viewport(
  float m[], float x, float y, float& x_v, float& y_v )

  //  This private routine converts a world x,y pair into a
  //  view-coordinate x,y pair
  //
  //  m:         (PROJECTION * MODELVIEW) matrix
  //  x, y:      World x,y
  //  x_v, y_v:  View x,y
{
  float  w;				// Homogeneous w value


  w = ( m[ 3 ] * x ) + ( m[ 7 ] * y ) + m[ 15 ];
  x_v = ( ( m[ 0 ] * x ) + ( m[ 4 ] * y ) + m[ 12 ] ) / w;
  y_v = ( ( m[ 1 ] * x ) + ( m[ 5 ] * y ) + m[ 13 ] ) / w;
}					// End routine comp_viewport


int quadtree::active()

  //  This method returns TRUE if the quadtree has data active within
  //  it, FALSE otherwise
{
  return ( root != NULL );
}					// End method active


int quadtree::build( qtree_elem *e, int n, int e_max )

  //  This routine beings the recursive building of the quadtree
  //
  //  e:      Elements to store in quadtree (x, y, ID)
  //  n:      Size of element array
  //  e_max:  Maximum elements allowed in each node (default=16)
{
  int    i;				// Loop counter
  float  mid[ 2 ];			// Midpoint x, y position


  if ( ( root = new qtree_node ) == NULL ) {
    printf( "quadtree::build(), out of memory creating root node\n");
    return 0;
  }

  root->n = n;				// Save elements in quadtree
  find_min_max( e, n, root->min, root->max );

  if ( n <= e_max ) {			// Everything fits in root?
    for( i = 0; i < 4; i++ ) {		// Initialize child nodes
      root->child[ i ] = NULL;
    }

    if ( ( root->e = new qtree_elem[ n ] ) == NULL ) {
      printf( "quadtree::build(), out of memory creating element list\n");

      root->n = 0;
      root->e = NULL;
      return 0;
    }

    for( i = 0; i < n; i++ ) {		// Save elements in node
      root->e[ i ] = e[ i ];
    }

  } else {				// Subdivision required
    root->e = NULL;
    comp_midpoint( root->min, root->max, mid );

    root->child[ 0 ] =
      build_node( e, n, e_max, root->min[0], root->min[1], mid[0], mid[1] );
    root->child[ 1 ] =
      build_node( e, n, e_max, mid[0], root->min[1], root->max[0], mid[1] );
    root->child[ 2 ] =
      build_node( e, n, e_max, mid[0], mid[1], root->max[0], root->max[1] );
    root->child[ 3 ] =
      build_node( e, n, e_max, root->min[0], mid[1], mid[0], root->max[1] );
  }

  return 1;
}					// End method build


qtree_node *quadtree::build_node( qtree_elem *e, int& n, int e_max,
  float x_min, float y_min, float x_max, float y_max )

  //  This private method recursively constructs interior quadtree nodes
  //
  //  e:             Elements (possibly) within node to construct
  //  n:             Element count
  //  e_max:         Maximum elements allowed in each node
  //  x_min, y_min:  Minimum x, y extents for current node
  //  x_max, y_max:  Maximum x, y extents for current node
{
  int         c;			// Count of elements within node
  int         i;			// Loop counter
  float       mid[ 2 ];			// Midpoint of node
  qtree_node *node;			// Quadtree node
  int         pos;			// Position within full element list
  qtree_elem  swap_e;			// Temporary swap element


  //  Split the element list into elements that belong in another node
  //  (at the front) and elements that belong in this node (at the
  //  end); process elements for this node, recursively if necessary

  for( i = 0, pos = 0; i < n; i++ ) {
    if ( clip_code( x_min, y_min, x_max, y_max, e[ i ].x, e[ i ].y ) != 0 ) {
      swap_e = e[ i ];
      e[ i ] = e[ pos ];
      e[ pos++ ] = swap_e;
    }
  }
  c = n - pos;				// Count of elements within node

  if ( c == 0 ) {			// No elements within node?
    return NULL;
  } else {
    n -= c;				// Update list size to reflect..
  }					// ..the elements we removed

  if ( ( node = new qtree_node ) == NULL ) {
    printf( "quadtree::build_node(), out of memory creating node\n");
    return NULL;
  }

  node->min[ 0 ] = x_min;		// Initialize spatial extent
  node->min[ 1 ] = y_min;
  node->max[ 0 ] = x_max;
  node->max[ 1 ] = y_max;

  node->n = c;				// Save elements in subtree

  if ( c <= e_max ) {			// Everything fits in node?
    if ( ( node->e = new qtree_elem[ c ] ) == NULL ) {
      printf( "quadtree::build_node(), out of memory creating element list\n");

      delete node;
      return NULL;
    }

    for( i = 0; i < 4; i++ ) {		// Initialize child nodes
      node->child[ i ] = NULL;
    }
    for( i = 0; i < c; i++ ) {		// Save elements in node
      node->e[ i ] = e[ i + pos ];
    }

  } else {				// Subdivision required
    node->e = NULL;
    comp_midpoint( node->min, node->max, mid );

    node->child[ 0 ] =
      build_node( &e[ pos ], c, e_max, x_min, y_min, mid[ 0 ], mid[ 1 ] );
    node->child[ 1 ] =
      build_node( &e[ pos ], c, e_max, mid[ 0 ], y_min, x_max, mid[ 1 ] );
    node->child[ 2 ] =
      build_node( &e[ pos ], c, e_max, mid[ 0 ], mid[ 1 ], x_max, y_max );
    node->child[ 3 ] =
      build_node( &e[ pos ], c, e_max, x_min, mid[ 1 ], mid[ 0 ], y_max );
  }

  return node;
}					// End method build_node


int quadtree::CS_clip( float x0, float y0, float x1, float y1 )

  //  This private routine uses Cohen-Sutherland to clip a node edge
  //  to the standard view frustum (-1,-1 to 1,1)
  //
  //  x0, y0:  First endpoint
  //  x1, y1:  Second endpoint
{
  int    accept;			// Accept edge within viewport flag
  int    c0, c1;			// Clip codes for endpoints
  int    c_out;				// Clip code for "outside" endpoint
  int    done = 0;			// Done processing flag
  float  x;				// X-intersection point
  float  x_max = 1, x_min = -1;		// Viewport x-boundaries
  float  y;				// Y-intersection point
  float  y_max = 1, y_min = -1;		// Viewport y-boundaries


  c0 = clip_code( x_min, y_min, x_max, y_max, x0, y0 );
  c1 = clip_code( x_min, y_min, x_max, y_max, x1, y1 );

  do {					// Continue until intersect identified
    if ( c0 == 0 && c1 == 0 ) {		// Trivial accept
      accept = 1;
      done = 1;

    } else if ( c0 & c1 ) {		// Trivial reject
      accept = 0;
      done = 1;

    } else {				// Potential intersection
      if ( c0 != 0 ) {			// Choose endpoint outside viewport
        c_out = c0;
      } else {
        c_out = c1;
      }

      if ( c_out & 8 ) {		// Out endpoint above viewport?
        x = x0 + ( x1 - x0 ) * ( y_max - y0 ) / ( y1 - y0 );
        y = y_max;

      } else if ( c_out & 4 ) {		// Out endpoint below viewport?
        x = x0 + ( x1 - x0 ) * ( y_min - y0 ) / ( y1 - y0 );
        y = y_min;

      } else if ( c_out & 2 ) {		// Out endpoint to right of viewport?
        y = y0 + ( y1 - y0 ) * ( x_max - x0 ) / ( x1 - x0 );
        x = x_max;

      } else {				// Out endpoint to left of viewport
        y = y0 + ( y1 - y0 ) * ( x_min - x0 ) / ( x1 - x0 );
        x = x_min;
      }

      if ( c0 == c_out ) {		// Move endpoint to intersect point
        x0 = x;
        y0 = y;
        c0 = clip_code( x_min, y_min, x_max, y_max, x0, y0 );
      } else {
        x1 = x;
        y1 = y;
        c1 = clip_code( x_min, y_min, x_max, y_max, x1, y1 );
      }
    }
  } while( !done );

  return accept;
}					// End method CS_clip


int quadtree::display( qtree_elem e[], int& n, int max )

  //  This method identifies quadtree nodes that are within the
  //  current viewport; it stores the elements within those nodes
  //  (plus a count) so the calling program can display them; the
  //  method returns TRUE if the element array overflows, FALSE
  //  otherwise
  //
  //  e:    Element array
  //  n:    Number of elements stored in array
  //  max:  Size of array

{
  float  m[ 16 ];			// 4x4 modelview+projection matrix
  int    mode;				// Current matrix mode


  n = 0;				// Start at front of array

  //  Compute PROJECTION_MATRIX * MODELVIEW_MATRIX to transform into
  //  view coordinate space

  glGetIntegerv( GL_MATRIX_MODE, &mode );

  glMatrixMode( GL_PROJECTION );
  glPushMatrix();
  glGetFloatv( GL_MODELVIEW_MATRIX, m );
  glMultMatrixf( m );
  glGetFloatv( GL_PROJECTION_MATRIX, m );
  glPopMatrix();

  glMatrixMode( mode );

  return display_node( root, m, e, n, max );
}					// End method display


int quadtree::display_node(
  qtree_node *node, float m[], qtree_elem e[], int& n, int max )

  //  This private method identifies nodes that are within the current
  //  viewport, saving the elements within those nodes for later
  //  display; returns TRUE if element array overflows
  //
  //  node:  Root of tree to display
  //  m:     Modelview+Projection matrix
  //  e:     Element array
  //  n:     Number of elements stored in array
  //  max:   Size of array
{
  int    i;				// Loop counter
  int    overflow = 0;			// Array overflow flag
  int    rc;				// Return code for edge clips
  float  x[ 4 ], y[ 4 ];		// View-normalized x,y of node corners


  if ( node == NULL ) {			// Quit on empty tree
    return 0;
  }

  comp_viewport( m, node->min[ 0 ], node->min[ 1 ], x[ 0 ], y[ 0 ] );
  comp_viewport( m, node->max[ 0 ], node->min[ 1 ], x[ 1 ], y[ 1 ] );
  comp_viewport( m, node->max[ 0 ], node->max[ 1 ], x[ 2 ], y[ 2 ] );
  comp_viewport( m, node->min[ 0 ], node->max[ 1 ], x[ 3 ], y[ 3 ] );

  //  Clip node edges to viewport (stop as soon as one intersects
  //  the viewport)

  rc  = CS_clip( x[ 0 ], y[ 0 ], x[ 1 ], y[ 1 ] );
  if ( !rc )
    rc |= CS_clip( x[ 1 ], y[ 1 ], x[ 2 ], y[ 2 ] );
  if ( !rc )
    rc |= CS_clip( x[ 2 ], y[ 2 ], x[ 3 ], y[ 3 ] );
  if ( !rc )
    rc |= CS_clip( x[ 3 ], y[ 3 ], x[ 0 ], y[ 0 ] );

  //  Check for: (1) some node edge intersects viewport, or (2) node
  //  surrounds viewport (so some part of node within viewport)

  if ( rc ||
       ( x[ 0 ] <= -1 && x[ 1 ] >= 1 && x[ 2 ] >= 1 && x[ 3 ] <= -1 &&
         y[ 0 ] <= -1 && y[ 1 ] <= -1 && y[ 2 ] >= 1 && y[ 3 ] >= 1 ) ) {
    if ( node->e != NULL ) {		// Leaf node with data
      if ( node->n > max - n ) {	// Will this node overflow array?
        overflow = 1;
        for( i = 0; n < max; i++ ) {
          e[ n++ ] = node->e[ i ];
        }

      } else {				// No array overflow
        for( i = 0; i < node->n; i++ ) {
          e[ n++ ] = node->e[ i ];
        }
      }

    } else {				// Interior node with childern
      for( i = 0; i < 4; i++ ) {	// Test child nodes
        overflow |= display_node( node->child[ i ], m, e, n, max );
      }
    }
  }

  return overflow;
}					// End method display_node


void quadtree::find_min_max( qtree_elem *e, int n, float min[], float max[] )

  //  This private method computes the minimum and maximum x, y
  //
  //  e:    Elements to store in quadtree (x, y, ID)
  //  n:    Size of element array
  //  min:  Minimum values
  //  max:  Maximum values
{
  int  i;				// Loop counter


  if ( n == 0 ) {			// No data to process?
    return;
  }

  min[ 0 ] = e[ 0 ].x;			// Initialize minimum, maximum
  max[ 0 ] = e[ 0 ].x;
  min[ 1 ] = e[ 0 ].y;
  max[ 1 ] = e[ 0 ].y;

  for( i = 1; i < n; i++ ) {		// Test all remaining elements
    if ( min[ 0 ] > e[ i ].x )		// Test x
      min[ 0 ] = e[ i ].x;
    else if ( max[ 0 ] < e[ i ].x )
      max[ 0 ] = e[ i ].x;

    if ( min[ 1 ] > e[ i ].y )		// Test y
      min[ 1 ] = e[ i ].y;
    else if ( max[ 1 ] < e[ i ].y )
      max[ 1 ] = e[ i ].y;
  }
}					// End method find_min_max


void quadtree::flush()

  //  This method clears the tree of all nodes
{
  free_node( root );
  root = NULL;
}


void quadtree::free_node( qtree_node *node )

  //  This private routine recursively frees nodes in a quadtree
  //
  //  node:  Root of tree to free
{
  int  i;				// Loop counter


  if ( node == NULL ) {			// Quit on empty tree
    return;
  }

  if ( node->e != NULL ) {		// Free elem array if it exists
    delete [] node->e;
  }

  for( i = 0; i < 4; i++ ) {		// Free child nodes
    free_node( node->child[ i ] );
  }

  delete node;
}					// End method free_node


quadtree::quadtree()

  //  Default constructor
{
  root = NULL;
}					// End default constructor


quadtree::~quadtree()

  //  Destructor
{
  free_node( root );
}					// End destructor
