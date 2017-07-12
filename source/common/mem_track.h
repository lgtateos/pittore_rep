#ifndef MEM_TRACK_H
#define MEM_TRACK_H

#ifdef _MEM_DEBUG

#include <exception>
#include <new>
#include <cstdlib>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "dyn_hash.h"

// the record struct in the memory tracking data structure
struct ALLOC_INFO
{
  int address;          // address of the start of memory block being recorded
  int size;             // the size of the memory block 
  char  file[ 128 ];    // absolute file name where memory operation occurred
  int line;             // line number where memory operation occurred
};

// the data structure holding allocation entries, defined in the 
// global namespace somewhere else
extern dyn_hash mem_hash_table;

// routines for managing and reporting the allocation table
void AddTrack( int, int, const char *, int );
void DumpUnfreed( );
void _Output_String( char * );
void RemoveTrack( int );

// the overloading of the global new(), delete(), new[](), and delete[]()
// operators.  Normally, these operators only take one argument, the
// size of the memory block to be allocated, and then the "new" operators 
// would return the address of that memory block.  The overloaded
// versions also take in the necessary information for tracking
// memory leaks, the name of the file and the line number where an allocation
// or de-allocation occurred.  

#ifdef _MEM_DEBUG  // use a preprocessor definition to turn this feature on
inline void * operator new( unsigned int size, const char *file, int line )
{
  void * ptr = (void *)malloc( size );
  AddTrack( (int) ptr, size, file, line );
  return (ptr);
};

inline void * operator new[]( unsigned int size, const char *file, int line )
{
  void * ptr = (void *)malloc( size );
  AddTrack( (int) ptr, size, file, line );
  return (ptr );
};


inline void operator delete( void *p )
{
  RemoveTrack( (int) p );
  free( p );
};

inline void operator delete[]( void *p )
{
  RemoveTrack( (int) p );
  free( p );
};

inline void operator delete( void *p, const char &file, int line )
{
  RemoveTrack( (int) p );
  free( p );
};

inline void operator delete[]( void *p, const char *file, int line )
{
  RemoveTrack( (int) p );
  free( p );
};
#endif

// in order to change from a one argument new() operation to a 3 argument
// new() opeartion we use MACROS that will insert the predefined MACROS
// __FILE__ and __LINE__ as arguments into the new() operator
#ifdef _MEM_DEBUG
#define DEBUG_NEW new( __FILE__, __LINE__ )
#else
#define DEBUG_NEW new
#endif
#define new DEBUG_NEW

#endif 
#endif
