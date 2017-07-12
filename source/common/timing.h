//  TIMING.H
//    Prototypes for TIMING.CPP

#ifndef _timing_h
#define _timing_h

long  usec_time();

#ifdef _M_IX86
void  usleep( int wait );
#endif

#endif
