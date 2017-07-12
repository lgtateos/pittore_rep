//  VEC.H
//    Class definition for VEC.CPP

#ifndef _vec_h
#define _vec_h

#include "global.h"
#ifdef NAMESPACE
  #include <fstream>
  #include <iostream>
  using namespace std;
#else 
  #include <fstream.h>
  #include <iostream.h>
#endif

#include <stdio.h>

#define H_SIZE  101			// Size of "In cache" hash table

struct s_elem {				// In-memory element
  int     ID;				// Element ID
  char   *val;				// Element value
  s_elem *prev;				// Previous stack node
  s_elem *next;				// Next stack node
};

struct h_elem {				// "In cache" hash element
  s_elem *elem;				// Element referenced by hash entry
  h_elem *prev;				// Previous hash entry
  h_elem *next;				// Next hash entry
};

class vec {
  private:
    int       elem_s;			// Element size (in bytes)
    char      f_name[ 512 ];		// Input filename
    fstream   fp;			// Pointer into input stream
    h_elem  **hash;			// "In cache" hash table
    s_elem   *head;			// Pointer to head of stack
    int       n;			// Size of input file (in elements)
    int       stack_cur;		// Current stack size
    int       stack_max;		// Maximum allowable stack size
    s_elem   *tail;			// Pointer to tail of stack

    int     copy( const vec& );
    s_elem *find_node( int );
    void    free_hash_table();
    void    free_stack();
    void    pop();
    void    prep( int );
    void    promote( s_elem * );
    void    push( char *, int );
    void    push_node( s_elem * );

    inline s_elem *hash_get( int );
    inline void    hash_remove( int );
    inline void    hash_set( int, s_elem * );

  public:
    vec();
    vec( const vec& );
   ~vec();

    void  close();
    int   elem_size();
    int   get( int, char * );
    int   put( int, char * );
    int   read( char *, int, int, int footer = 0 );
    int   size();

    vec&  operator=( const vec& );
};

#endif
