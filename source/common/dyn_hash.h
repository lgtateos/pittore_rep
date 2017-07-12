//  DYN_HASH.H
//    Class definition for dynamic hashing

#ifndef _dyn_hash_h
#define _dyn_hash_h

#define rot(x,k) (((x)<<(k)) ^ ((x)>>(32-(k))))

#define mix(a,b,c) \
{ \
  a -= c;  a ^= rot(c, 4);  c += b; \
  b -= a;  b ^= rot(a, 6);  a += c; \
  c -= b;  c ^= rot(b, 8);  b += a; \
  a -= c;  a ^= rot(c,16);  c += b; \
  b -= a;  b ^= rot(a,19);  a += c; \
  c -= b;  c ^= rot(b, 4);  b += a; \
}

#define final(a,b,c) \
{ \
  c ^= b; c -= rot(b,14); \
  a ^= c; a -= rot(c,11); \
  b ^= a; b -= rot(a,25); \
  c ^= b; c -= rot(b,16); \
  a ^= c; a -= rot(c,4);  \
  b ^= a; b -= rot(a,14); \
  c ^= b; c -= rot(b,24); \
}

typedef struct {			// Key/value pair
  unsigned int * k;			// Key
  void *v;				// Pointer to generic value
} kv_pair;

typedef struct {			// Bucket of objects
  int    bit_n;				// Bits used for bucket's entries
  int    n;				// Number of key/val pairs
  kv_pair *val;				// Key/value pairs
} bucket;


class dyn_hash {
private:
  int      bit_n;			// Bits used for hash table
  int      bit_max;			// Maximum hash table size
  int      bucket_n;			// Bucket size
  bucket **tbl;				// Hash table
  int      tbl_n;			// Hash table size
  int      k_n;                         // size of key buffer

  inline int  find_entry(		// Find entry containing key
                unsigned int * k,	// Key to find
                int& i,			// Bucket containing key
                int& j			// Index containing key
              );
  inline unsigned int  hash(		// Hash key
		   unsigned int * k,	// Key to hash
		   int l
		   );
  inline int  pow_2_mask(		// Build power-of-two mask
                int p			// Width of mask
              );

  int   combine_bucket(			// Combine two buckets
          int b0,			// Index of first bucket
          int b1			// Index of second bucket
        );
  int   compact_tbl();			// Reduce hash table by half
  void  copy(				// Copy from exising hash object
          const dyn_hash& h		// Object to copy from
        );
  int   expand_tbl();			// Expand hash table by 2x
  void  free();				// Free all memory in hash table
  int   split_bucket(			// Split bucket into two buckets
          int b				// Bucket to split
        );


public:
  dyn_hash(				// Default constructor
    int n = 16,				// Default bucket size
    int l = 1
  );
  dyn_hash(				// Copy constructor
    const dyn_hash& h			// Object to copy from
  );
 ~dyn_hash();				// Destructor

  int   add(				// Add key/value pair
          unsigned int * k,		// Key
          void *v			// Value pointer
        );
  int   add(				// Add key/value pair
          int k,			// Key
          void *v			// Value pointer
        );
  void *find(				// Find value pointer for key
          unsigned int *k		// Key to find
        );
  void *find(				// Find value pointer for key
          int k				// Key to find
        );
  void  flush();			// Flush hash table to empty
  void *get_entry(                      // Find key/value pair at a table index
		  int idx,              // Index to access
		  unsigned int * k      // Key at index
		  );
  void *get_entry(                      // Find key/value pair at a table index
		  int idx,              // Index to access
		  int  & k              // Key at index
		  );
  int   integrity();			// Check integrity of hash table
  void *remove(				// Remove key (return it's value ptr)
          unsigned int * k		// Key to remove
        );
  void *remove(				// Remove key (return it's value ptr)
          int k				// Key to remove
        );
  int   size();                         // Get number of entries in table
  int   test(				// Test class
          int msg = 1			// Print message flag (default=1)
        );
  int   update(				// Update value pointer for key
          unsigned int * k,		// Key to update
          void *v			// New value pointer
        );
  int   update(				// Update value pointer for key
          int  k,			// Key to update
          void *v			// New value pointer
        );

  dyn_hash&  operator=(			// Assignment operator
               const dyn_hash& h	// Object to assign from
             );
};					// End dyn_hash class

#endif
