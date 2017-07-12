#include "global.h"
#if defined(NAMESPACE)
#include <iostream>
#include <fstream>

//using cerr;			// std namespace use defn's
#else

  #include <iostream.h>
  #include <fstream.h>
#endif
#include <assert.h>
#include "dyn_hash.h"

/*--------------------------------------------------------------------------*/
/*  DYN_HASH.CPP                                                            */
/*    This class implements a dynamic hash table                            */
/*                                                                          */
/*- Modification History: --------------------------------------------------*/
/*  When:       Who:                    Comments:			    */
/*                                                                          */
/*  27-Apr-04	Christopher G. Healey	Initial implementation	            */
/*  28-Mar-06	Brent M. Dennis	        Added size() and get_entry()        */
/*                                      functions                           */
/*  19-May-06	Brent M. Dennis	        Updated class to accept buffers of  */
/*                                      unsigned ints for keys, effectively */
/*                                      moving from a 32-bit key to variable*/
/*                                      key size.  Also, preserved the      */
/*                                      original 32-bit API so existing code*/
/*                                      does not have to change.            */
/*--------------------------------------------------------------------------*/


inline int dyn_hash::pow_2_mask( int p )

  //  This method computes a power-of-2 bitmask
  //
  //  p:  Compute bitmask with p 1's (i.e., 2^p - 1)
{
  if ( p > 0 )
    return 0xffffffff >> ( 32 - p );
  else
    return 0;
}                                                      // End method pow_2_mask

int dyn_hash::add( int k, void *v )
  
  // This method adds the given entry with the given key value
  // 
  //  k:  Key value
  //  v:  Pointer to value to add
{
  if( k_n != 1 )
    return 0;
  else
    return add( (unsigned int *) &k, v );
}                                                      // End method add

int dyn_hash::add( unsigned int * k, void *v )

  // This method adds the given entry with the given key value
  //
  //  k:  Key value
  //  v:  Pointer to value to add
{
  int  i;
  unsigned int  h;				// Hash value
  int  h_bit;				// Relevant bits from hash value


  if ( find( k ) != NULL ) {
	cerr<< "dyn_hash::add(), duplicate key " << k << "\n";
    return 0;
  }

  h = hash( k, k_n );			// Hash key
  h_bit = h >> ( bit_max - bit_n );	// Isolate relevant bits

  //  Split full bucket until space exists to hold new record

  while ( tbl[ h_bit ]->n == bucket_n ) {
    if ( !split_bucket( h_bit ) ) {
      return 0;
    }
    h_bit = h >> ( bit_max - bit_n );	// Recompute target bucket index
  }
  for( i = 0; i < k_n; i++ )
    tbl[ h_bit ]->val[ tbl[ h_bit ]->n ].k[ i ] = k[ i ];
  tbl[ h_bit ]->val[ tbl[ h_bit ]->n ].v = v;
  tbl[ h_bit ]->n++;

  return 1;
}                                                     // End method add


int dyn_hash::combine_bucket( int b0, int b1 )

  //  This method combines buckets i and j into a single bucket, then
  //  commonly references that new bucket
  //
  //  b0,b1:  Indices of buckets to combine
{
  int  i, j;				// Loop counter


  for( i = 0; i < tbl[ b1 ]->n; i++ ) {
    for( j = 0; j < k_n; j++ )
      tbl[ b0 ]->val[ tbl[ b0 ]->n ].k[ j ] = tbl[ b1 ]->val[ i ].k[ j ];
    tbl[ b0 ]->val[ tbl[ b0 ]->n ].v = tbl[ b1 ]->val[ i ].v;

    tbl[ b0 ]->n++;
    if ( tbl[ b0 ]->n >= bucket_n ) {
      cerr << "dyn_hash::combine_bucket(), bucket " << b0 << " overflow\n";
      return 0;
    }
  }

  for( i = 0; i < bucket_n; i++ )
    delete [] tbl[ b1 ]->val[ i ].k;

  delete [] tbl[ b1 ]->val;		// Free second bucket
  delete [] tbl[ b1 ];

  tbl[ b1 ] = tbl[ b0 ];		// Reference common bucket
  return 1;
}					// End method combine_bucket


int dyn_hash::compact_tbl()

  //  This method halves the size of the hash table
{
  int      i;				// Loop counter
  bucket **new_tbl;			// New hash table
  int      new_tbl_n;			// New hash table size


  new_tbl_n = tbl_n / 2;
  if ( ( new_tbl = new bucket *[ new_tbl_n ] ) == NULL ) {
    return 0;
  }

  //  In new hash table, pairs of entries pointing to a common bucket
  //  are compressed into a single hash table entry

  for( i = 0; i < new_tbl_n; i++ ) {
    new_tbl[ i ] = tbl[ i * 2 ];
  }

  delete [] tbl;			// Free old hash table
  tbl = new_tbl;			// Point to new hash table

  bit_n--;				// Update bit count and mask
  tbl_n = new_tbl_n;

  return 1;
}					// End method compact_tbl


void dyn_hash::copy( const dyn_hash& dh )

  //  This method copies the state of an already-exisiting dynamic
  //  hash object
  //
  //  dh:  Dynamic hash object to copy from
{
  int  i, j, k;				// Loop counters


  free();				// Free existing memory

  bit_n = dh.bit_n;			// Copy instance variables
  bit_max = dh.bit_max;
  bucket_n = dh.bucket_n;
  tbl_n = dh.tbl_n;

  tbl = new bucket *[ tbl_n ];		// Allocate hash table
  assert( tbl != NULL );

  for( i = 0; i < tbl_n; i++ ) {	// Copy buckets
    tbl[ i ] = new bucket;
    tbl[ i ]->val = new kv_pair[ bucket_n ];
    for( j = 0; j < bucket_n; j++ )
      tbl[ i ]->val[ i ].k = new unsigned int[ k_n ];

    assert( tbl[ i ] != NULL && tbl[ i ]->val != NULL );

    tbl[ i ]->n = dh.tbl[ i ]->n;
    tbl[ i ]->bit_n = dh.tbl[ i ]->bit_n;

    for( j = 0; j < tbl[ i ]->n; j++ ) {
      for( k = 0; k < k_n; k++ )
	tbl[ i ]->val[ j ].k[ k ] = dh.tbl[ i ]->val[ j ].k[ k ];
    }					// End for all key/value pairs
  }					// End for all buckets
}					// End method copy


int dyn_hash::expand_tbl()

  //  This method doubles the size of the hash table
{
  int      i;				// Loop counter
  bucket **new_tbl;			// New hash table
  int      new_tbl_n;			// New hash table size


  if ( bit_n == bit_max ) {		// All bits in use in hash table?
    return 0;
  }

  new_tbl_n = tbl_n * 2;
  if ( ( new_tbl = new bucket *[ new_tbl_n ] ) == NULL ) {
    return 0;
  }

  //  In new hash table, each pair of entries points to a common
  //  bucket (pointers taken from the current hash table)

  for( i = 0; i < new_tbl_n; i += 2 ) {
    new_tbl[ i ] = tbl[ int( i / 2 ) ];
    new_tbl[ i + 1 ] = new_tbl[ i ];
  }

  delete [] tbl;			// Free old hash table
  tbl = new_tbl;			// Point to new hash table

  bit_n++;				// Update bit count and mask
  tbl_n = new_tbl_n;

  return 1;
}                                                     // End method expand_tbl

void *dyn_hash::find( int k )
  //  This method returns a pointer to the value attached to the given
  //  key, or NULL if no such key exists
  //
  //  k:  Key of node to find
{
  if( k_n != 1 )
    return NULL;
  else
    return find( (unsigned int *) &k );
}                                                    // End method find  

void *dyn_hash::find( unsigned int * k )

  //  This method returns a pointer to the value attached to the given
  //  key, or NULL if no such key exists
  //
  //  k:  Key of node to find
{
  int     i;				// Index of hash table entry w/key
  int     j;				// Index of bucket entry w/key


  if ( !find_entry( k, i, j ) ) {	// Search for node in hash table
    return NULL;
  } else {
    return tbl[ i ]->val[ j ].v;
  }
}					// End method find

inline int dyn_hash::find_entry( unsigned int * k, int& b, int& idx )

  //  This method finds the bucket entry containing the given key, if
  //  it exists; sets b to hash table index, idx to bucket index of
  //  key; return 1 on success, 0 otherwise
  //
  //  k:    Key of node to find
  //  b:    Bucket containing key
  //  idx:  Bucket entry containing key
{
  unsigned int  h;				// Hash value
  int  i, j;				// Loop counter
  int  flag;

  h = hash( k, k_n );
  b = h >> ( bit_max - bit_n );		// Isolate relevant bits

  for( i = 0; i < tbl[ b ]->n; i++ ) {	// For all keys in potential bucket
    flag = 1;
    for( j = 0; j < k_n; j++ )
      flag &= tbl[ b ]->val[ i ].k[ j ] == k[ j ];
    if ( flag ) {	// Key found?
      idx = i;
      return 1;
    }
  }

  return 0;
}					// End method find_entry


void dyn_hash::flush()

  //  This method frees all entries in the dynamic hash table, then
  //  resets the table to a valid starting point
{
  int  i, j;				// Loop counter


  free();

  bit_n = 1;				// Start with 2-bucket table
  tbl_n = 2;

  tbl = new bucket *[ tbl_n ];
  assert( tbl != NULL );

  for( i = 0; i < tbl_n; i++ ) {	// Create buckets' arrays
    tbl[ i ] = new bucket;
    assert( tbl[ i ] != NULL );

    tbl[ i ]->val = new kv_pair[ bucket_n ];

    for( j = 0; j < bucket_n; j++ )
      tbl[ i ]->val[ j ].k = new unsigned int[ k_n ];

    assert( tbl[ i ]->val != NULL );

    tbl[ i ]->n = 0;
    tbl[ i ]->bit_n = 1;
  }
}					// End method flush


void dyn_hash::free()

  //  This method frees all entries in the dynamic hash table
{
  int  cur_bit_n;			// Common bit count for buckets
  int  i, j;				// Loop countertabl


  i = 0;
  while ( i < tbl_n ) {			// For all buckets in table
    cur_bit_n = tbl[ i ]->bit_n;
    for( j = 0; j < bucket_n; j++ )
      delete [] tbl[ i ]->val[ j ].k;
    delete [] tbl[ i ]->val;
    delete tbl[ i ];

    //  Must walk to next real bucket; the distance to walk depends on
    //  bits for current bucket versus bits for table as a whole:
    //
    //  - diff=0, 1 ref to bucket, step 1 forward
    //  - diff=1, 2 ref to bucket, step 2 forward
    //    ...
    //  - diff=n, 2^n ref to bucket, step forward 2^n
    
    i += pow_2_mask( bit_n - cur_bit_n ) + 1;
  }

  delete [] tbl;			// Free table

  bit_n = 0;				// Reset table parameters
  tbl_n = 0;
  tbl = NULL;
}					// End method free

void * dyn_hash::get_entry( int index, int & k )
{
  unsigned int K;
  void * v;
  if( k_n != 1 )
    return NULL;
  else
    {
      v = get_entry( index, &K );
      k = K;
      return v;
    }
}

void * dyn_hash::get_entry( int index, unsigned int * k )

  // this routine accesses the entry at the specified index of the
  // hash tabl
  // 
  // index    index into the hash table
  // k        holder for key located at the specified index of hash table
{
  int  cur_bit_n;			// Common bit count for buckets
  int  i; int j; int itr;                             // Loop countertable
  int  cnt; int done;                       // count var and done flag
  void * v = NULL;                      // holder for return value

  i    = 0;
  cnt  = 0;
  done = 0;
  while ( i < tbl_n && !done ) {		// For all buckets in table
    cur_bit_n = tbl[ i ]->bit_n;

    if( cnt + tbl[ i ]->n > index )
      {
	j = index - cnt;
	for( itr = 0; itr < k_n; itr++ )
	  k[ itr ] = tbl[ i ]->val[ j ].k[ itr ];
	v = tbl[ i ]->val[ j ].v;
	done = 1;
      }
    cnt += tbl[ i ]->n;
    //  Must walk to next real bucket; the distance to walk depends on
    //  bits for current bucket versus bits for table as a whole:
    //
    //  - diff=0, 1 ref to bucket, step 1 forward
    //  - diff=1, 2 ref to bucket, step 2 forward
    //    ...
    //  - diff=n, 2^n ref to bucket, step forward 2^n
    
    i += pow_2_mask( bit_n - cur_bit_n ) + 1;
  }

  return v;
}                                       // End method get_entry

/*
inline int dyn_hash::hash( int k )

  //  This method returns a hash value for the given key; use Robert
  //  Jenkin's 32-bit mix function (2^31 possible values)
  //
  //  k:  Key to hash
{

  k += (k << 12);
  k ^= (k >> 22);
  k += (k << 4);
  k ^= (k >> 9);
  k += (k << 10);
  k ^= (k >> 2);
  k += (k << 7);
  k ^= (k >> 12);

  return k;
}					// End method hash
*/
inline unsigned int dyn_hash::hash( unsigned int * k, int l )
{
  unsigned int a,b,c,len;

  /* Set up the internal state */
  len = (unsigned int) l;
  a = b = c = 0xdeadbeef + (len<<2) + 13;

  /*------------------------------------------------- handle most of the key */
  while (len > 3)
  {
    a += *k++;
    b += *k++;
    c += *k++;
    mix(a,b,c);
    len -= 3;
  }

  /*--------------------------------------------- handle the last 3 uint32's */
  switch(len)                        /* all the case statements fall through */
  { 
  case 3 : c+=k[2];
  case 2 : b+=k[1];
  case 1 : a+=k[0];
    final(a,b,c);
  case 0:     /* case 0: nothing left to add */
    break;
  }
  /*------------------------------------------------------ report the result */
  return c;
}

int dyn_hash::integrity()

  //  This method verifies the integrity of data stored in the dynamic
  //  hash table; it returns 1 if integrity is verified, 0 otherwise
{
  int  blk_n;				// Num table entry w/ref to cur bucket
  unsigned int  h;				// Hash value
  int  h_bit;				// Relevant bits from hash value
  int  i, j;				// Loop counters


  i = 0;				// Check hash value/bucket matches
  while( i < tbl_n ) {
    for( j = 0; j < tbl[ i ]->n; j++ ) {
      h = hash( tbl[ i ]->val[ j ].k, k_n );
      h_bit = h >> ( bit_max - bit_n );	// Isolate relevant bits

      //  If the pointer to the bucket that should contain the key
      //  isn't the same as the pointer to the bucket that actually
      //  DOES contain the key, signal error

      if ( tbl[ h_bit ] != tbl[ i ] ) {
		cerr << "dyn_hash::integrity(), key/bucket mismatch, ";
        cerr << "key= " << tbl[ i ]->val[ j ].k << ", ";
        cerr << "hash=" << h << ", ";
        cerr << "hash_bit=" << h_bit << ", ";
        cerr << "bucket=" << i << ", ";
        cerr << "&tbl[i]=" << tbl[ i ] << ", ";
        cerr << "&tbl[hash_bit]=" << tbl[ h_bit ] << "\n";
        return 0;
      }
    }

    //  Check that all common bucket entries in hash table point to
    //  the same bucket

    blk_n = pow_2_mask( bit_n - tbl[ i ]->bit_n ) + 1;
    for( j = i + 1; j < i + blk_n; j++ ) {
      if ( tbl[ i ] != tbl[ j ] ) {
        cerr << "dyn_hash::integrity(), common bucket entry mismatch, ";
        cerr << "b0=" << i << ", ";
        cerr << "b1=" << j << ", ";
        cerr << "&b0=" << tbl[ i ] << ", ";
        cerr << "&b1=" << tbl[ j ] << ", ";
        cerr << "table bit_n=" << bit_n, ", ";
        cerr << "bucket bit_n=" << tbl[ i ]->bit_n, "\n";
        return 0;
      }					// End if common bucket ptr mismatch
    }					// End for all common buckets

    i += blk_n;
  }					// End for all hash table entries

  return 1;
}					// End method integrity

void *dyn_hash::remove( int k )
  //  This method removes the entry with the given key from the hash
  //  table, returning its value pointer (or NULL if no such key
  //  exists)
  //
  //  k:  Key of entry to remove
{
  if( k_n != 1 )
    return NULL;
  else
    return remove( (unsigned int *) &k );
}                                                     // End method remove  

void *dyn_hash::remove( unsigned int * k )

  //  This method removes the entry with the given key from the hash
  //  table, returning its value pointer (or NULL if no such key
  //  exists)
  //
  //  k:  Key of entry to remove
{
  int   cur_bit_n;			// Common bit count for buckets
  int   done;				// Done processing flag
  int   i;				// Index of hash table entry w/key
  int   j;				// Index of bucket entry w/key
  int   itr;
  int   n;				// Number of entries in bucket
  void *v;				// Value pointer of entry to remove


  if ( !find_entry( k, i, j ) ) {	// Search for node in hash table
    return NULL;
  }

  v = tbl[ i ]->val[ j ].v;		// Grab value pointer

  //  If last entry is being removed, no need to plug the hole

  n = tbl[ i ]->n - 1;
  if ( j != n ) {			// Removing interior entry?
    for( itr = 0; itr < k_n; itr++ )
      tbl[ i ]->val[ j ].k[ itr ] = tbl[ i ]->val[ n ].k[ itr ];
    tbl[ i ]->val[ j ].v = tbl[ i ]->val[ n ].v;
  }
  tbl[ i ]->n--;			// Reduce entry count for bucket

  //  Combine buckets if:
  //  - not already at lowest bit count for hash table
  //  - bucket is on frontier of trie
  //  - bucket plus buddy bucket are below 80% of a full bucket

  if ( bit_n > 1 && tbl[ i ]->bit_n == bit_n ) {
    j = ( i % 2 == 0 ) ? i + 1 : i - 1;	// Locate buddy bucket

    if ( tbl[ i ]->n + tbl[ j ]->n < bucket_n * 0.8 ) {
      if ( !combine_bucket( i, j ) ) {
        return NULL;
      }
      tbl[ i ]->bit_n--;

      //  If all buckets have a common bit count (which must now be at
      //  least one less than hash table's bit count), collapse the
      //  hash table

      cur_bit_n = tbl[ i ]->bit_n;

      i = 0;
      done = 0;
      while( !done && i < tbl_n ) {	// Search for mismatched bit counts
        if ( tbl[ i ]->bit_n != cur_bit_n ) {
          done = 1;
        } else {
          i += pow_2_mask( bit_n - tbl[ i ]->bit_n ) + 1;
        }
      }

      if ( !done ) {			// If all bucket bit counts match..
        compact_tbl();			// ..halve the size of the hash table
      }
    }
  }

  return v;
}					// End method remove

int dyn_hash::size( )

  // this method returns the number of entries in the table
{
  int  cur_bit_n;			// Common bit count for buckets
  int  i;				// Loop countertable
  int  ret_val = 0;

  i = 0;

  while ( i < tbl_n ) {			// For all buckets in table
    cur_bit_n = tbl[ i ]->bit_n;

    ret_val += tbl[ i ]->n;
    //  Must walk to next real bucket; the distance to walk depends on
    //  bits for current bucket versus bits for table as a whole:
    //
    //  - diff=0, 1 ref to bucket, step 1 forward
    //  - diff=1, 2 ref to bucket, step 2 forward
    //    ...
    //  - diff=n, 2^n ref to bucket, step forward 2^n
    
    i += pow_2_mask( bit_n - cur_bit_n ) + 1;
  }

  return ret_val;
}                                       // End method size

int dyn_hash::split_bucket( int b )

  //  This method splits the given bucket, expanding the hash table if
  //  necessary, and re-inserting the original contents into the
  //  appropriate buckets
  //
  //  b:  Index of bucket to split
{
  bucket *b1;				// New bucket 1
  int     b1_n = 0;			// Entries in of new bucket 1
  int     blk_n;			// Num table entry w/ref to cur bucket
  bucket *cur_b;			// Current bucket
  int     cur_bit_n;			// Current bucket's bitcount
  int     cur_n = 0;			// Entries left in current bucket 0
  unsigned int     h;				// Hash value
  int     h_bit;			// Relevant bits from hash value
  int     i, j;				// Loop counters

  b1 = new bucket;			// Create new bucket
  if ( b1 == NULL ) {
    return 0;
  }

  b1->val = new kv_pair[ bucket_n ];	// Create key/value array

  if ( b1->val == NULL ) {
    delete b1;
    return 0;
  }
  
  for( j = 0; j < bucket_n; j++ )
    b1->val[ j ].k = new unsigned int[ k_n ];

  cur_b = tbl[ b ];			// Reference current bucket
  cur_bit_n = tbl[ b ]->bit_n;

  //  If all available bits in hash table are being used by bucket to
  //  split, must expand the hash table's size

  if ( cur_b->bit_n == bit_n ) {
    if ( !expand_tbl() ) {
      for( j = 0; j < bucket_n; j++ )
	delete [] b1->val[ j ].k;
      delete [] b1->val;
      delete b1;
      return 0;
    }
    b *= 2;				// New location of bucket to split
  }

  //  Re-insert values into new buckets, based on the "new" bit (if
  //  bit=0, into bucket 0, if bit=1, into bucket 1)

  for( i = 0; i < cur_b->n; i++ ) {
    h = hash( cur_b->val[ i ].k, k_n );
    h_bit = h >> ( bit_max - ( cur_bit_n + 1 ) );

    if ( ( h_bit & 1 ) == 0 ) {
      for( j = 0; j < k_n; j++ )
	cur_b->val[ cur_n ].k[ j ] = cur_b->val[ i ].k[ j ];
      cur_b->val[ cur_n ].v = cur_b->val[ i ].v;
      cur_n++;
    } else {
      for( j = 0; j < k_n; j++ )
	b1->val[ b1_n ].k[ j ] = cur_b->val[ i ].k[ j ];
      b1->val[ b1_n ].v = cur_b->val[ i ].v;
      b1_n++;
    }
  }

  cur_b->n = cur_n;			// Update bucket counts
  b1->n = b1_n;

  cur_b->bit_n = cur_bit_n + 1;		// Update bit counts
  b1->bit_n = cur_bit_n + 1;

  //  Insert new buckets into appropriate slots in hash table; blk_n
  //  is number of entries in hash table that pointed to bucket to
  //  split, first half point to b0, second half to b1; b is set to
  //  beginning of common block

  blk_n = pow_2_mask( bit_n - cur_bit_n ) + 1;
  b = int( b / blk_n ) * blk_n;

  for( i = 0; i < blk_n / 2; i++ ) {
    tbl[ b + i ] = cur_b;
  }
  for( i = blk_n / 2; i < blk_n; i++ ) {
    tbl[ b + i ] = b1;
  }
  return 1;
}					// End method split_bucket


int dyn_hash::test( int msg )

  //  This method tests the add, find, update, and remove methods in
  //  the dynamic hash class
  //
  //  msg:  Print message flag (default=1)
{
  /*
  dyn_hash  h(16,3);				// Dynamic hash object to test
  int       i; int j; int itr;				// Loop counter
  unsigned int       k[ 3*25000 ];			// Temporary key array
  int       p0, p1;			// Temporary array indices
  unsigned int       tmp_k[ 3 ];			// Temporary key value
  int vals[ 25000 ];
  void     *v;				// Value pointer
  unsigned int * KEY = new unsigned int[ k_n ];
  unsigned int * KEY2 = new unsigned int[ k_n ];

  if ( msg )
    cerr << "Populating hash table...\n";

  for( i = 1; i <= 50000; i++ ) {
    for( j = 0; j < k_n; j++ )
      KEY[ j ] = i*k_n + j;
    h.add( KEY, (void *) i );
  }

  if ( msg )
    cerr << "Testing integrity of 50000 adds... ";

  if ( !h.integrity() ) {
    if ( msg )
      cerr << "failed\n";
    return 0;
  }

  if ( msg )
    cerr << "passed\nTesting 25000 successful finds... ";

  for( i = 1; i <= 50000; i += 2 ) {
    for( j = 0; j < k_n; j++ )
      KEY[ j ] = i*k_n + j;
    v = h.find( KEY );

    if ( v == NULL || int( v ) != i ) {
      if ( msg )
        cerr << "failed @ " << i << "\n";
      return 0;
    }
  }

  if ( msg )
    cerr << "passed\nTesting 25000 unsuccessful finds... ";

  for( i = 75000; i < 100000; i++ ) {
    for( j = 0; j < k_n; j++ )
      KEY[ j ] = i*k_n + j;
    v = h.find( KEY );

    if ( v != NULL ) {
      if ( msg )
        cerr << "failed @ " << i << "\n";
      return 0;
    }
  }

  if ( msg )
    cerr << "passed\nTesting 25000 updates...";

  for( i = 1; i <= 50000; i += 2 ) {
    for( j = 0; j < k_n; j++ )
      KEY[ j ] = i*k_n + j;
    h.update( KEY, (void *) i );
  }

  if ( !h.integrity() ) {
    if ( msg )
      cerr << "failed\n";
    return 0;
  }

  if ( msg )
    cerr << "passed\nTesting 50000 removes...";

  for( i = 50000; i >= 1; i-- ) {
    for( j = 0; j < k_n; j++ )
      KEY[ j ] = i*k_n + j;
    v = h.remove( KEY );

    if ( int( v ) != i ) {
      if ( msg )
        cerr << "failed @ " << i << "\n";
      return 0;
    }

    if ( i % 1000 == 0 && !h.integrity() ) {
      if ( msg )
        cerr << "integrity failed @ " << i << "\n";
      return 0;
    }
  }

  if ( !h.integrity() ) {
    if ( msg )
      cerr << "failed\n";
    return 0;
  }

  if ( msg )
    cerr << "passed\nRepopulating hash table...\n";

  for( i = 1; i <= 50000; i++ ) {

    for( j = 0; j < k_n; j++ )
      KEY[ j ] = i*k_n + j;
    
    if ( i <= 25000 ) {
      for( j = 0; j < k_n; j++ )
	k[ 3*(i - 1) + j ] = KEY[ j ];
      vals[ i - 1 ] = k_n*i;
    }

    h.add( KEY, (void *) (k_n*i) );
  }

  int tmp;
  for( i = 0; i < 250000; i++ ) {

    for( j = 0; j < k_n; j++ )
      KEY[ j ] = i*k_n + j;
    for( j = 0; j < k_n; j++ )
      KEY2[ j ] = (i+1)*k_n + j;

    p0 = hash( KEY, k_n ) % 25000;
    p1 = hash( KEY2, k_n ) % 25000;

    for( itr = 0; itr < k_n; itr++ )
      tmp_k[ itr ] = k[ k_n*p0 + itr ];
    for( itr = 0; itr < k_n; itr++ )
      k[ k_n*p0 + itr ] = k[ k_n*p1 + itr ];
    for( itr = 0; itr < k_n; itr++ )
      k[ k_n*p1 + itr ] = tmp_k[ itr ];

    tmp        = vals[ p0 ];
    vals[ p0 ] = vals[ p1 ];
    vals[ p1 ] = tmp;
  }

  if ( msg )
    cerr << "Testing 25000 random removes... ";

  for( i = 0; i < 25000; i++ ) {
    v = h.remove( k + 3*i );

    if ( int( v ) != vals[ i ] ) {
      if ( msg )
        cerr << "failed @ " << i << "\n";
      return 0;
    }

    if ( i % 1000 == 0 && !h.integrity() ) {
      if ( msg )
        cerr << "integrity failed @ " << i << "\n";
      return 0;
    }
  }

  if ( !h.integrity() ) {
    if ( msg )
      cerr << "failed\n";
    return 0;
  }

  if ( msg )
    cerr << "passed\n";
  */
  return 1;
}					// End method test

int dyn_hash::update( int k, void *v )
  //  This method updates the pointer to the value attached to the
  //  given key; return 1 on successful update, 0 otherwise
  //
  //  k:  Key of node to find
  //  v:  New value pointer
{
  if( k_n != 1 )
    return 0;
  else
    return update( (unsigned int *) &k, v );
}                                                    // End method update  

int dyn_hash::update( unsigned int * k, void *v )

  //  This method updates the pointer to the value attached to the
  //  given key; return 1 on successful update, 0 otherwise
  //
  //  k:  Key of node to find
  //  v:  New value pointer
{
  int     i;				// Index of hash table entry w/key
  int     j;				// Index of bucket entry w/key


  if ( !find_entry( k, i, j ) ) {	// Search for node in hash table
    return 0;
  }

  tbl[ i ]->val[ j ].v = v;
  return 1;
}					// End method update


dyn_hash& dyn_hash::operator=( const dyn_hash& dh )

  //  Assignment operator
  //
  //  dh:  Dynamic hash object to assign from
{
  copy( dh );
  return *this;
}					// End assignment operator


dyn_hash::dyn_hash( int n, int l )

  //  Default constructor
  //
  //  n:  Bucket size (default=16)
{
  int  i, j;				// Loop counter


  bit_max = 32;				// Use 3-bit hash values

  bit_n = 1;				// Start with 2-bucket table
  tbl_n = 2;
  bucket_n = n;
  k_n   = l;
  tbl = new bucket *[ tbl_n ];
  assert( tbl != NULL );

  for( i = 0; i < tbl_n; i++ ) {	// Create buckets' arrays
    tbl[ i ] = new bucket;
    assert( tbl[ i ] != NULL );

    tbl[ i ]->val = new kv_pair[ bucket_n ];
    assert( tbl[ i ]->val != NULL );

    for( j = 0; j < bucket_n; j++ )
      tbl[ i ]->val[ j ].k = new unsigned int[ k_n ];

    tbl[ i ]->n = 0;
    tbl[ i ]->bit_n = 1;
  }
}					// End default constructor


dyn_hash::dyn_hash( const dyn_hash& dh )

  //  Copy constructor
  //
  //  dh:  Dynamic hash object to copy from
{
  tbl_n = 0;				// Ensure free() in copy() works
  tbl = NULL;

  copy( dh );
}					// End copy constructor


dyn_hash::~dyn_hash()

  //  Destructor
{
  free();
}					// End destructor


