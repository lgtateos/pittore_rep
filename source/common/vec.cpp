//#include <iostream.h>
#include <string.h>
#include "global.h"
#include "vec.h"

/*--------------------------------------------------------------------------*/
/*  VEC.CPP								    */
/*    Class to implement a vector, supports disk paging to allow vectors    */
/*    larger than main memory						    */
/*									    */
/*- Modification History: --------------------------------------------------*/
/*  When:	Who:			Comments:			    */
/*									    */
/*  07-Oct-97	Christopher G. Healey	Initial implementation		    */
/*  10-Nov-98	Christopher G. Healey	Added assignment operator	    */
/*  30-Dec-99	Christopher G. Healey	Fixed problem with fstream good(),  */
/*					returns FALSE if stream is already  */
/*					attached during new open() attempt  */
/*  05-Feb-01	Christopher G. Healey	Switched from bit array to track    */
/*					"in-cache" elements to hash table   */
/*  27-Jun-01	Christopher G. Healey	Switch fp to fstream to allow read  */
/*					AND write; udpated all open() to    */
/*					include ios::out		    */
/*  8-Jul-07    Laura G. Tateosian modified vec::prep because Vis Studio.NET 2.0 */
/*                   throws error for subtracting constant from tellg() call  */
/*--------------------------------------------------------------------------*/


void vec::close()

  //  This method closes the input stream and resets the vec to empty
  //
{
  fp.close();				// Close active input stream

  free_stack();				// Free existing stack, hash table
  free_hash_table();

  n = 0;				// Initialize instance variables
  elem_s = 0;
  stack_cur = 0;
  stack_max = 0;
}					// End method close


int vec::copy( const vec& v )

     //  This private method copies v's stack and corresponding hash table,
     //  note WE ASSUME THE STACK AND HASH TABLE ARE EMPTY (caller can use
     //  use free_stack() and free_hash_table() to guarantee this)
     //
     //  v:  Vector object to copy from
{
  s_elem *cur;				// Current stack element
  int     i;				// Loop counter


  fp.close();				// Close previous stream state
  fp.clear();

#ifndef _M_IX86
  fp.open( v.f_name, ios::in | ios::out );
#else
  fp.open( v.f_name, ios::in | ios::out | ios::binary );
#endif

  if ( !fp.good() ) {
    cerr << "vec::copy(), unable to open file \"" << v.f_name << "\"\n";
    return 0;
  }

  elem_s = v.elem_s;			// Copy instance variables
  strcpy( f_name, v.f_name );
  n = v.n;
  stack_max = v.stack_max;

  stack_cur = 0;			// Reset current stack size

  hash = new h_elem *[ H_SIZE ];	// Create hash table
  if ( !hash ) {
    cerr << "vec::copy(), out of memory allocating hash table\n";
    return 0;
  }

  for( i = 0; i < H_SIZE; i++ ) {	// Initialize hash table
    hash[ i ] = NULL;
  }

  head = NULL;
  tail = NULL;

  cur = v.tail;				// Push values from back to front
  while( cur != NULL ) {
    push( cur->val, cur->ID );		// Save node, add addr to hash table
    hash_set( cur->ID, head );

    cur = cur->prev;
  }

  return 1;
}					// End method copy


int vec::elem_size()

     //  Return the size of an individual element to the caller
{
  return elem_s;
}					// End method elem_size


s_elem *vec::find_node( int ID )

     //  This private method returns a pointer to the list node
     //  with the given ID
     //
     //  ID:  ID to search for
{
  s_elem *cur = head;			// Current list node


  while( cur != NULL ) {		// For all nodes in list
    if ( cur->ID == ID ) {
      return cur;
    }

    cur = cur->next;
  }

  return NULL;				// Return node not found
}					// End method find_node


void vec::free_hash_table()

  //  This private method walks the hash table, freeing each bucket
{
  h_elem *cur;				// Current node in bucket
  h_elem *fre;				// Node to free
  int     i;				// Current bucket


  if ( hash == NULL ) {			// Return if no table to free
    return;
  }

  for( i = 0; i < H_SIZE; i++ ) {	// For all buckets
    cur = hash[ i ];

    while( cur != NULL ) {		// While nodes exist
      fre = cur;
      cur = cur->next;

      delete fre;
    }
  }

  delete [] hash;			// Free hash table itself
  hash = NULL;
}					// End method free_hash_table


void vec::free_stack()

     //  This private method walks the stack, freeing each node
{
  s_elem *cur = head;			// Current node on stack
  s_elem *fre;				// Node to free


  while( cur != NULL ) {		// While nodes exist
    fre = cur;
    cur = cur->next;

    delete [] fre->val;
    delete fre;
  }

  head = NULL;				// Reset stack to empty
  tail = NULL;
  stack_cur = 0;
}					// End method free_stack


int vec::get( int i, char v[] )

     //  This method gets the given value, fetching it from the disk if
     //  necessary
     //
     //  i:  Index of element to get
     //  v:  Storage for element
{
  s_elem *node;				// Pointer to node to free

  if ( i < 0 || i >= n ) {		// Index out of bounds?
    cerr << "vec::get(), element out of bounds\n";
    return 0;
  }

  if ( ( node = hash_get( i ) ) != NULL ) {
	  promote( node );

  } else {				// Element stored on disk
    if ( stack_cur >= stack_max ) {	// Stack full, re-use tail node
      node = tail;
      hash_remove( node->ID );
      tail = tail->prev;		// Update tail node
      if ( tail == NULL ) {		// Could be NULL if 1-element stack
	head = NULL;
      } else {
	tail->next = NULL;
      }
      stack_cur--;

    } else {				// Space on stack, make new node
      node = new s_elem;		// Allocate node
      if ( node )
	node->val = new char[ elem_s ];

      if ( !node || !node->val ) {
        cerr << "vec::get(), out of memory allocating new stack node\n";
	return 0;
      }
    }
    node->ID = i;
    fp.seekg( i * elem_s );
    fp.read( node->val, elem_s );
    push_node( node );			// Push element onto top of stack
    hash_set( i, head );
  }
  memmove( v, head->val, elem_s );	// Return data to caller
  return 1;
}					// End method get


inline s_elem *vec::hash_get( int ID )

  //  This method locates the entry with the given ID in the hash
  //  table, returning its location in the cache or NULL if no such
  //  cache entry exists
  //
  //  ID:  ID of entry to find
{
  int      b = ID % H_SIZE;		// Bucket to probe
  h_elem  *node;			// Pointer to candidate bucket


  node = hash[ b ];			// Point to start of bucket

  while( node != NULL ) {		// For all entries in bucket
    if ( node->elem->ID == ID ) {	// Search for target entry
      return node->elem;
    }
    node = node->next;
  }

  return NULL;				// Return no such entry in cache
}					// End method hash_get


inline void vec::hash_remove( int ID )

  //  This method removes the entry with the given ID from the hash
  //  table
  //
  //  ID:  ID of entry to remove
{
  int      b = ID % H_SIZE;		// Bucket to probe
  h_elem  *node;			// Pointer to candidate bucket


  node = hash[ b ];			// Point to start of bucket

  while( node != NULL ) {		// For all entries in bucket
    if ( node->elem->ID == ID ) {	// Search for target entry
      break;
    }
    node = node->next;
  }

  if ( node == NULL ) {			// If no such node, remove fails
    return;
  }

  if ( hash[ b ] == node ) {		// Removing front of list?
    hash[ b ] = node->next;
  } else {				// Removing internal to list
    node->prev->next = node->next;
  }

  if ( node->next ) {			// Not at end of list?
    node->next->prev = node->prev;
  }

  delete node;				// Free node to remove
}					// End method hash_remove


inline void vec::hash_set( int ID, s_elem *elem )

  //  This method adds the given entry to the appropriate position in
  //  the hash table
  //
  //  ID:    ID of entry to remove
  //  elem:  Pointer to element in cache
{
  int     b = ID % H_SIZE;		// Bucket to hold entry
  h_elem *node;				// Node to add to bucket


  node = new h_elem;
  if ( !node ) {
    cerr << "vec::hash_set(), out of memory allocating bucket entry\n";
    return;
  }

  node->elem = elem;			// Save pointer to cache entry
  node->prev = NULL;			// Setup node for front of bucket
  node->next = hash[ b ];

  if ( hash[ b ] ) {			// Update previous front of bucket
    hash[ b ]->prev = node;
  }

  hash[ b ] = node;			// Point to new front-of-bucket
}					// End method hash_set


void vec::prep( int footer )

  //  This private method prepares a binary input file by creating the
  //  hash table, and reading up to stack_max elements onto the stack
  //
  //  footer:  Size of any binary "tail" of control information that
  //           may be attached to the end of the file (defaults to 0)
{
  char *cur;				// Current element value
  int   i;				// Loop counter
  int   num;				// Number of elements onto stack
  int   cpos;

  fp.seekg( 0, ios::end );		// Get datafile size, in elements
  cpos = fp.tellg();
  n = (cpos - footer)/elem_s;  //n = ( fp.tellg() - footer ) / elem_s;  Vis Studio.NET 2.0 throws error for subtracting constant from tellg() call
  

  //  Compute size of data portion of file, and ensure it is evenly
  //  divisible by element size
  if ( ( cpos - footer ) % elem_s != 0 ) {  
    cerr << "Warning:  In vec::prep()\n";
    cerr << "Uneven element size (" << elem_s << ")";
    cerr << "for total file size (" << fp.tellg() << ")\n";
  }

  free_hash_table();			// Create new "In cache" hash table
  cur = new char[ elem_s ];
  hash = new h_elem *[ H_SIZE ];	// Create hash table

  if ( !hash || !cur ) {
    cerr << "vec::copy(), out of memory allocating hash table\n";
    return;
  }

  for( i = 0; i < H_SIZE; i++ ) {	// Initialize hash table
    hash[ i ] = NULL;
  }

  //  Read up to stack_max elements onto stack, update hash table to
  //  point to proper stack entries (or NULL if no stack entry)

  free_stack();				// Free old stack

  fp.seekg( 0, ios::beg );
  num = ( n < stack_max ) ? n : stack_max;

  for( i = 0; i < num; i++ ) {		// For all possible elements
    fp.read( cur, elem_s );

    push( cur, i );
    hash_set( i, head );
  }

  delete [] cur;			// Free temporary stack element
}					// End method prep


void vec::promote( s_elem *node )

     //  This private method promotes the given stack entry to the top of
     //  the stack
     //
     //  node:  Pointer to node to promote
{

  if ( node == NULL ) {		// Ensure node exists
    cerr << "vec::promote(), invalid stack element\n";
    return;
  }

  if ( node == head ) {			// Node already top of stack, bail
    return;

  } else if ( node == tail ) {		// Node at end of stack
    node->prev->next = NULL;
    tail = node->prev;

  } else {				// Node interior to stack
    node->prev->next = node->next;
    node->next->prev = node->prev;
  }
  stack_cur--;

  push_node( node );			// Push node to top of stack
}					// End method promote


void vec::push( char v[], int id )

     //  This private method pushes the given values into a node placed
     //  on the top of the stack
     //
     //  v:   Pointer to array of bytes holding value
     //  id:  ID of element
{
  s_elem *node;				// Node to add to top of stack


  node = new s_elem;			// Allocate node
  if ( node )
    node->val = new char[ elem_s ];

  if ( !node || !node->val ) {
    cerr << "vec::push(), out of memory allocating new stack node\n";
    return;
  }

  node->ID = id;
  memmove( node->val, v, elem_s );

  push_node( node );			// Push node onto top of stack
}					// End method push


void vec::push_node( s_elem *node )

     //  This private method pushes the given node onto the top of the
     //  stack (included in addition to pushing key,val pair, since if
     //  you have the node you shouldn't alloc or dealloc memory, slow)
     //
     //  node:  Pointer to node to push
{
  if ( head == NULL ) {			// First node in list?
    node->prev = NULL;
    node->next = NULL;
    head = node;
    tail = node;

  } else {				// Push node onto top of list
    node->next = head;
    node->prev = NULL;
    head->prev = node;
    head = node;
  }

  stack_cur++;
}					// End method push_node


int vec::put( int i, char v[] )

  //  This method puts the given value, writing it from the disk and
  //  updating the in-memory cache, if necessary necessary
  //
  //  i:  Index of element to put
  //  v:  Pointer to element data
{
  s_elem *node;				// Pointer to node to free


  if ( i < 0 || i >= n ) {		// Index out of bounds?
    cerr << "vec::put(), element out of bounds\n";
    return 0;
  }

  fp.seekg( i * elem_s );		// Save value on disk
  fp.write( v, elem_s );

  //  If element in cache, promote it and update its value field

  if ( ( node = hash_get( i ) ) != NULL ) {
    promote( node );
    memmove( node->val, v, elem_s );
  }

  return 1;
}					// End method put


int vec::read( char *name, int e_size, int s_size, int footer )

  //  This method opens a binary file and prepares it for use
  //
  //  name:    Binary filename
  //  e_size:  Element size (in bytes)
  //  s_size:  Stack size (in elements)
  //  footer:  Size of any binary "tail" of control information that
  //           may be attached to the end of the file (defaults to 0)
{
  if ( e_size <= 0 ) {
    cerr << "vec::read(), element size must be greater than 0\n";
    return 0;
  } else if ( s_size <= 0 ) {
    cerr << "vec::read(), maximum stack size must be greater than 0\n";
    return 0;
  }

  elem_s = e_size;			// (Re)set element, stack sizes
  stack_max = s_size;

  fp.close();				// Close previous stream state
  fp.clear();

#ifndef _M_IX86
  fp.open( name, ios::in | ios::out );
#else
  fp.open( name, ios::in | ios::out | ios::binary );
#endif

  if ( !fp.good() ) {			// Ensure open request succeeded
    cerr << "vec::read(), unable to open file \"" << name << "\"\n";
    return 0;
  }

  prep( footer );			// Build stack, hash table
  strcpy( f_name, name );

  return 1;
}					// End method read


int vec::size()

     //  This method returns the size of the vector, in elements
{
  return n;
}					// End method size


vec& vec::operator=( const vec& v )

     //  Assignment operator
     //
     //  v:  Vector to assign from
{
  fp.close();				// Close active input stream

  free_stack();				// Free existing stack, hash table
  free_hash_table();

  copy( v );
  return *this;
}					// End copy constructor


vec::vec()

     //  Default constructor
{
  n = 0;				// Initialize instance variables
  elem_s = 0;
  stack_cur = 0;
  stack_max = 0;

  head = NULL;
  tail = NULL;
  hash = NULL;
}					// End default constructor


vec::vec( const vec& v )

     //  Copy constructor
     //
     //  v:  Vec object to copy from
{
  n = 0;				// Initialize instance variables
  elem_s = 0;
  stack_cur = 0;
  stack_max = 0;

  head = NULL;
  tail = NULL;
  hash = NULL;

  copy( v );
}					// End copy constructor


vec::~vec()

     //  Destructor
{
  free_stack();				// Free allocated memory, close file
  free_hash_table();
  fp.close();
}					// End destructor
