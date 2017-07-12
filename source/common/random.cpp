#include <stdlib.h>
#include "random.h"

#ifdef SGI
#include <unistd.h>
#else
#include <time.h>
#endif

/*--------------------------------------------------------------------------*/
/*  RANDOM.CPP								    */
/*    Seed/generate random numbers under IRIX or Windows NT		    */
/*									    */
/*- Modification History: --------------------------------------------------*/
/*  When:	Who:			Comments:			    */
/*									    */
/*  15-Jun-99	Christopher G. Healey	Initial implementation		    */
/*  29-Dec-99	Christopher G. Healey	Added #define macros to separate    */
/*					SGI and NT routines (allows all	    */
/*					source to live in a single file)    */
/*--------------------------------------------------------------------------*/

float rand_num()

  //  This routine returns a random number between 0 and 1
{
#ifdef SGI
  return float( drand48() );
#else
  return float( rand() ) / float( RAND_MAX );
#endif
}


void seed()
{
#ifdef SGI
  srand48( getpid() );
#else
  srand( (unsigned) time( NULL ) );
#endif
}


void seed( int s )
{
#ifdef SGI
  srand48( s );
#else
  srand( s );
#endif
}
