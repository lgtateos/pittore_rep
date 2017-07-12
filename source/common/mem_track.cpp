#ifndef MEM_TRACK_CPP
#define MEM_TRACK_CPP

#include "mem_track.h"

#ifdef _MEM_DEBUG

/*--------------------------------------------------------------------------*/
/*  MEM_TRACK.CPP                                                           */
/*    These routines overload the global memory operators new(), new[](),   */
/*    delete(), and delete[]() in order to keep track of memory being       */
/*    allocated and de-allocated in a project.  Memory tracking is done     */
/*    with a hash table that keeps track of the size and address            */
/*    of each memory block allocated or de-allocated as well as the file    */
/*    and line number where these operations took place.                    */
/*    Installation into code base requires these steps:                     */
/*      1)  Add _MEM_DEBUG to list of preprocessor defintions               */
/*      2)  Add mem_track.h and mem_track.cpp to project and include        */
/*          mem_track.h in every source file where you want memory tracking */
/*      3)  Add dyn_hash mem_hash_table; to global namespace                */
/*      4)  Call DumpUnfreed( ) to get a report of current un-freed memory  */
/*      *)  Must not #include "mem_track.h" before a #include <stdlib.h>    */
/*									    */
/*- Modification History ---------------------------------------------------*/
/*  When:       Who:                    Comments:                           */
/*									    */
/*   7-Mar-06   Brent M. Dennis         Initial implementation              */
/*  28-Mar-06   Brent M. Dennis         Replaced linked list with hash table*/ 
/*--------------------------------------------------------------------------*/

void AddTrack( int addr, int asize, const char *fname, int lnum )
  // routine that adds a record to the allocation list
  //
  // addr      address of start of memory address being allocated
  // asize     size of memory block being allocated
  // fname     name of source file where new() or new[]() was called 
  // lnum      line number of source file where new() or new[]() was called 
{
  ALLOC_INFO * info;
  // allocate new record
  info = (ALLOC_INFO *)malloc( sizeof(ALLOC_INFO) );   
  // record information for this memory allocation 
  info->address = addr;
  strncpy( info->file, fname, 127 );
  info->line    = lnum;
  info->size    = asize;
  mem_hash_table.add( addr, (void *) info );
}

void RemoveTrack( int addr )
  // routine to remove a record from the allocation list
  //
  // addr    address of memory block that needs to be removed from allocation
  //         list
{
  ALLOC_INFO *ptr, *last;
  mem_hash_table.remove( addr );
}

void DumpUnfreed( )
  // routine that reports the current state of the allocation table
{
  int totalSize = 0;
  char buf[ 1024 ];
 
  int size = mem_hash_table.size( );
  int i;
  int key;
  void *   v_ptr;
  ALLOC_INFO * ptr;
  for( i = 0; i < size; i++ )
    {
      v_ptr = mem_hash_table.get_entry( i, key );
      ptr = (ALLOC_INFO *) v_ptr;
      sprintf( buf, "%-50s:\t\tLINE %d, \t\tADDRESS 0x%x\t%d unfreed\n",
	       ptr->file, ptr->line, ptr->address, ptr->size );
      _Output_String( buf );
      totalSize += ptr->size;
    }
  sprintf( buf, "------------------------------------------------------\n" );
  _Output_String( buf );
  sprintf( buf, "Total Unfreed: %d bytes\n", totalSize );
  _Output_String( buf );
}

void _Output_String( char * buf )
  // routine to control how records are reported
  // for now, just using stdout, but this could be change to write to
  // a file or some other output stream
{
  printf("%s\n", buf );
}


#endif 
#endif
