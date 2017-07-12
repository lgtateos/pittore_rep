//#include "iostream"//#include <iostream.h>
//#include <fstream>//#include <fstream.h>
//#include <string.h>
#include <stdio.h>
#include "str.h"
#include "filereader.h"
#include "global.h"

/*--------------------------------------------------------------------------*/
/*  FILEREADER.C							    */
/*    This class allows the user to read and write to an external file	    */
/*									    */
/*- Modfication History ----------------------------------------------------*/
/*  When:	Who:			Comments:			    */
/*									    */
/*  21-Sep-93	Christopher G. Healey	Initial implementation		    */
/*  01-Aug-94	Christopher G. Healey	Added "append" option		    */
/*  06-Dec-95	Christopher G. Healey	Added binary read/write and seek    */
/*  31-Jan-03	Christopher G. Healey	Modified getline() to check for an  */
/*					ios::failbit when line longer than  */
/*					array size			    */
/*  31-Jan-03	Christopher G. Healey	Added clear() method to reset the   */
/*					stream's state back to good	    */
/*  04-Mar-03	Christopher G. Healey	Added check for extra CR (0x0d) on  */
/*					DOS-based line, strip if found	    */
/*--------------------------------------------------------------------------*/


void filereader::clear()

   //  This method clears any error bits set on the current input stream
{
   switch( mode ) {
   case READ:
   case READ_WRITE:
      inp_fp->clear();
      break;

   case APPEND:
   case WRITE:
      out_fp->clear();
      break;
   }
}					// End method clear


void filereader::close()

   //  This method closes either the input or output stream
{
   switch( mode ) {			// Select file stream to close
   case READ:				// Close input stream
   case READ_WRITE:
      if ( inp_fp != &cin ) {		// Ensure we don't try to close cin
         ((ifstream *) inp_fp)->close();
         delete inp_fp;
      }

      mode = UNDEF;			// Reset mode to undefined
      break;

   case APPEND:				// Close output stream
   case WRITE:
      if ( out_fp != &cout ) {		// Ensure we don't try to close cout
         ((ofstream *) out_fp)->flush();
         ((ofstream *) out_fp)->close();
         delete out_fp;
      }

      mode = UNDEF;			// Reset mode to undefined
      break;

   default:
      break;
   }
}					// End method close


int filereader::eof()

   //  This method reports true if EOF detected, FALSE otherwise
{
   switch( mode ) {			// Select file stream to query
   case READ:				// Query input stream
   case READ_WRITE:
      return inp_fp->eof();

   case APPEND:				// Query output stream
   case WRITE:
      return out_fp->eof();

   default:
      return 0;
   }
}					// End eof method


int filereader::getline( string& buf )

   //  This method reads the next line from the input stream, and returns
   //  it to the user as a string
   //
   //  line:  Storage for current line
{
   int   done;				// Done reading line flag
   int   len;				// Line length
   char  line[ 4096 ];			// Buffer to hold next line


   if ( mode != READ && mode != READ_WRITE ) {
      return 0;
   }

   buf = "";				// Clear string

   do {					// Read until newline or EOF
      inp_fp->getline( line, 4096 );

   //  If the line's length exceeds the array's length, the array is
   //  NULL terminated, and a failbit is set

      if ( inp_fp->fail() && !inp_fp->eof() ) {
        done = 0;
        inp_fp->clear();

      } else {
        done = 1;

  //  Strip extra CR on the end of DOS-based lines

        if ( line[ strlen( line ) - 1 ] == 0x0d ) {
          line[ strlen( line ) - 1 ] = '\0';
        }
      }

      buf = buf + line;
   }
   while( !done );

   //  If no line present and EOF reached, return FALSE to caller

   len = buf.len();
   if ( len == 0 && inp_fp->eof() ) {
      return 0;
   }

   return 1;
}					// End method getline


int filereader::next( string *buf, int size, char *sep )

   //  This method reads a single line, parses it into individual words,
   //  and stores those words in the given array
   //
   //  buf:   Buffer to use to store words
   //  size:  Maximum size of buffer
   //  sep:   Separator character to use
{
   char  def_sep[] = " \t";		// Default separator (space, tab)
   int   done;				// Done processing line flag
   int   elem = 0;			// Current buffer element
   char  line[ 4096 ];			// Buffer to hold input line


   if ( mode != READ && mode != READ_WRITE ) {
      return 0;
   }

   do {					// Continue until tokens or EOF
      elem = 0;

      do {				// Read until newline or EOF
         inp_fp->getline( line, 4096 );

   //  If the line's length exceeds the array's length, the array is
   //  NULL terminated, and a failbit is set

        if ( inp_fp->fail() && !inp_fp->eof() ) {
          done = 0;
          inp_fp->clear();

        } else {
          done = 1;

  //  Strip extra CR on the end of DOS-based lines

          if ( line[ strlen( line ) - 1 ] == 0x0d ) {
            line[ strlen( line ) - 1 ] = '\0';
          }
        }

        if ( sep == NULL ) {
			elem += string( line ).token( &buf[ elem ], size - elem, def_sep );
        } else {
			elem += string( line ).token( &buf[ elem ], size - elem, sep );
        }
      }
      while( !done );
   }
   while( elem == 0 && !inp_fp->eof() );

   return elem;
}					// End method next


long filereader::offset()

   //  This method returns the current pointer position within the file
{
   if ( mode != READ && mode != READ_WRITE ) {
      return 0L;
   }

   return inp_fp->tellg();
}					// End method offset


int filereader::open( char m )

   //  This method opens either cin or cout for direct stream access
   //
   //  m:  Mode to open (r=read from cin, w=write to cout)
{
   int  rc = 1;				// Return code


   switch( m ) {
      case 'r':
      case 'R':
         inp_fp = &cin;
         mode = READ;
         break;

      case 'w':
      case 'W':
         out_fp = &cout;
         mode = WRITE;
         break;

      default:
         cout << "Error: Unknown mode " << mode << " specified\n";
         rc = 0;
         mode = UNDEF;
         break;
   }					// End switch on mode

   return rc;
}					// End method open


int filereader::open( string fname, char m )

   //  This method opens the given file in the given mode (r or w)
   //
   //  fname:  Name of file to open
   //  m:      Mode to open (r=read, w=write)
{
   int  rc = 1;				// Return code


   switch( m ) {
      case 'r':				// Open for read..
      case 'R':
      case 'x':				// ..or for read-write
      case 'X':
         close();			// Close any currently open stream
         
         if ( m == 'r' || m == 'R' ) {
          // char buf[ 1024 ];
            inp_fp = new ifstream;
            //cout<<"My output Name:    "<<fname;
            //sprintf( buf, "ADDRESS 0x%x",inp_fp );
            //printf("%s\n", buf );
#ifndef _M_IX86
            ((ifstream *) inp_fp)->open( fname, ios::in );
#else
            ((ifstream *) inp_fp)->open( fname, ios::in | ios::binary );
#endif
            mode = READ;

         } else {
            inp_fp = new fstream;
#ifndef _M_IX86
            ((fstream *) inp_fp)->open( fname, ios::in | ios::out );
#else
            ((fstream *) inp_fp)->open( fname, ios::in|ios::out|ios::binary );
#endif
            mode = READ_WRITE;
         }

         if ( !inp_fp->good() ) {
            //cout << "Error: File " << fname << " would not open\n";
           cout<<"Error: File "<<fname<<" would not open\n";

            rc = 0;
            delete inp_fp;
            mode = UNDEF;
         }

         break;

      case 'a':				// Open for append..
      case 'A':
      case 'w':				// ..or for write
      case 'W':
         close();			// Close any currently open stream
         out_fp = new ofstream;		// Create new output file stream

         if ( m == 'a' || m == 'A' ) {
#ifndef _M_IX86
            ((ofstream *) out_fp)->open( fname, ios::app );
#else
            ((ofstream *) out_fp)->open( fname, ios::app | ios::binary );
#endif
            mode = APPEND;

         } else {
#ifndef _M_IX86
            ((ofstream *) out_fp)->open( fname, ios::out );
#else
            ((ofstream *) out_fp)->open( fname, ios::out | ios::binary );
#endif
            mode = WRITE;
         }

         if ( !out_fp->good() ) {
            printf( "Error: File %s would not open\n", fname);

            rc = 0;
            delete out_fp;
            mode = UNDEF;
         }

         break;

      default:
         cout << "Error: Unknown mode " << mode << " specified\n";

         rc = 0;
         mode = UNDEF;
         break;
   }					// End switch on mode

   return rc;
}					// End method open


int filereader::read_raw( char *buf, int len )

   //  This method read a block of bytes from the input stream
   //
   //  buf:  Pointer to input buffer
   //  len:  Length of buffer
{
   if ( mode != READ && mode != READ_WRITE ) {
      return 0;
   }

   inp_fp->read( buf, len );
   return inp_fp->gcount();
}					// End method read_raw


void filereader::seek( long off, seek_mode pos )

   //  This method seeks forwards or backwards in the file from the given
   //  position (BEGIN, CUR, or END)
   //
   //  off:  Offset to seek by
   //  pos:  Position to seek from
{
   if ( mode != READ && mode != READ_WRITE ) {
      return;
   }

   switch( pos ) {			// Seek from user-chosen position
   case BEGIN:
      inp_fp->seekg( off, ios::beg );
      break;

   case END:
      inp_fp->seekg( off, ios::end );
      break;

   case CUR:
      inp_fp->seekg( off, ios::cur );
      break;
   };
}					// End method seek


void filereader::write( string line )

   //  This method writes a single line to the output stream
   //
   //  line:  Line to send to output stream
{
   if ( mode != APPEND && mode != WRITE && mode != READ_WRITE ) {
      return;
   }

   if ( mode == READ_WRITE ) {
	   *((fstream *) inp_fp) << line;
      ((fstream *) inp_fp)->flush();
   } else {
      *out_fp << line;
      ((ofstream *) out_fp)->flush();
   }
}					// End method write


void filereader::write_raw( char *buf, int len )

   //  This method writes a block of bytes to the output stream
   //
   //  buf:  Pointer to buffer to write
   //  len:  Length of output buffer
{
   if ( mode != APPEND && mode != WRITE && mode != READ_WRITE ) {
      return;
   }

   if ( mode == READ_WRITE ) {
      ((fstream *) inp_fp)->write( buf, len );
   } else {
      out_fp->write( buf, len );
   }
}					// End method write_raw


filereader::filereader()

   //  Constructor for filereader objects, set mode to undefined
{
   mode = UNDEF;
}					// End constructor method


filereader::~filereader()

   //  Destructor for filereader objects, ensure file is closed properly
{
   close();
}					// End destructor method

