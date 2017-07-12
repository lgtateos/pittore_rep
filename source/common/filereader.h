//  FILEREADER.H
//    Class definition for FILEREADER.C

#ifndef _filereader_h
#define _filereader_h

#include "global.h"
#ifdef NAMESPACE
  #include <iostream>
  #include <fstream>
 // #include <string>
  using namespace std;
#else
  #include <fstream.h>
  #include <iostream.h>
  #include <string.h>
#endif
#include "str.h"

typedef enum {				// Enumerate valid seek modes
   BEGIN, CUR, END
} seek_mode;


class filereader {
   private:
      typedef enum {			// Enumerate valid filereader modes
         APPEND, READ, WRITE, READ_WRITE, UNDEF
      } fmode;

      istream *inp_fp;			// Input file stream
      ostream *out_fp;			// Output file stream
      fmode    mode;			// Current filereader mode

   public:
      filereader();			// Constructor
     ~filereader();			// Destructor

      void  clear();
      void  close();
	  int   getline( string& );
      int   eof();
      long  offset();
      int   open( char );
	  int   open( string, char );
	  int   next( string *, int, char *sep = NULL );
      int   read_raw( char *, int );
	  int   parse( string *, int, string, char * );
      void  seek( long, seek_mode );
	  void  write( string );
      void  write_raw( char *, int );
};

#endif

