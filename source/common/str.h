//  STRING.H
//    Class definition for STRING.C

#ifndef _string_h
#define _string_h
#include "global.h"
#ifdef NAMESPACE
  #include <fstream>
  using namespace std;
#else
  #include <fstream.h>
#endif
//namespace nstr{
class string {
   private:
      char  BOUNDS_ERR;			// Used for out of bounds error
      int   m_size;			// Maximum length of string
      int   size;			// Length of string
      char *str;			// Character array for string

   public:
      string();				// Constructor
      string( const string& );		// Constructor (copy constructor)
      string( const char * );		// Constructor (copy constructor)
      string( const char );		// Constructor (copy constructor)
     ~string();				// Destructor

      int      is_int();
      int      is_float();
      int      len() const;
      string   substr( int, int );
      int      token( string *, int, char *sep = " " );

      int      operator==( const string& );
      int      operator!=( const string& );
      int      operator<=( const string& );
      int      operator>=( const string& );
      int      operator<( const string& );
      int      operator>( const string& );
      char&    operator[]( int );
      string&  operator=( const string& );
      string   operator+( const string& );
      string   operator+( const char& c );

      int      operator==( char *s ) { return operator==((string) s ); }
      int      operator!=( char *s ) { return operator!=((string) s ); }
      int      operator<=( char *s ) { return operator<=((string) s ); }
      int      operator>=( char *s ) { return operator>=((string) s ); }
      int      operator>( char *s ) { return operator>((string) s ); }
      int      operator<( char *s ) { return operator<((string) s ); }
      string&  operator=( char *s ) { return operator=((string) s ); }
      string   operator+( char *s ) { return operator+((string) s ); }

      operator char*();
      operator unsigned char*();
      operator float();
      operator int();


      friend ostream&  operator<<( ostream&, const string& );
      friend istream&  operator>>( istream&, string& );

};
//}// end namespace nstr
#endif
