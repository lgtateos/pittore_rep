#ifdef _M_IX86
//#include "values.h"
#include <time.h>
#else
#include <unistd.h>
#endif

#include "timing.h"

/*--------------------------------------------------------------------------*/
/*  TIMING.CPP								    */
/*    Routines to return usecond timing on PC or Solaris hardware	    */
/*									    */
/*- Modification History: --------------------------------------------------*/
/*  When:	Who:			Comments:			    */
/*									    */
/*  01-Feb-03	Christopher G. Healey	Initial implementation		    */
/*  08-Feb-03	Christopher G. Healey	Added usleep() for Windows	    */
/*--------------------------------------------------------------------------*/


long usec_time()

  //  Return current time in usecs (1000000 usecs in one second)
{
#ifdef _M_IX86
  clock_t          c_tv;		// Windows struct for per-frame timing

#else
  struct timeval   tv;			// UNIX structs for per-frame timing
  struct timezone  tz;
#endif


#ifdef _M_IX86
  c_tv = clock();
  return long( ( float( c_tv ) / CLOCKS_PER_SEC ) * 1000000.0 );

#else
  gettimeofday( &tv, &tz );
  return long( tv.tv_sec * 1000000 ) + long( tv.tv_usec );
#endif
}					// End routine usec_time


#ifdef _M_IX86
void usleep( int wait )

  //  This routine implements sleep on Windows
  //
  //  wait:  Time to wait (in useconds, 1000000 usec = 1 sec)
{
  clock_t  cur;				// Current time
  clock_t  target;			// Target time to release sleep


  target = clock_t( float( wait ) / 1000000.0 * CLOCKS_PER_SEC );
  target = target + clock();
  cur = clock();
  while( target > cur )
    cur = clock();
}					// End Windows routine usleep
#endif
