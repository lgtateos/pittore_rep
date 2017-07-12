#include <fstream>//#include <fstream.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "global.h"
#include "str.h"

/*--------------------------------------------------------------------------*/
/*  STRING.C								    */
/*    This class implements strings in C++, which can be created, assigned  */
/*    compared, and manipulated in an intuitive manner			    */
/*									    */
/*- Modification History ---------------------------------------------------*/
/*  When:	Who:			Comments:			    */
/*									    */
/*  05-Aug-93	Christopher G. Healey	Initial implementation		    */
/*              George Tsiknis						    */
/*--------------------------------------------------------------------------*/
//namespace nstr{

int string::is_int()

   //  This method returns TRUE if the string represents a valid integer,
   //  FALSE otherwise
{
   //  Ensure the characters in the string are only digits

   if ( size != strspn( str, "0123456789+-" ) ) {
      return 0;

   } else {
      return 1;
   }
}					// End method is_int


int string::is_float()

   //  This method returns TRUE if the string represents a valid float,
   //  FALSE otherwise
{
   //  Ensure the characters in the string are only digits or decimal place

   if ( size != strspn( str, "0123456789.+-" ) ) {
      return 0;

   } else {
      return 1;
   }
}					// End method is_float


int string::len() const

   //  This method returns the length of the given string
{
   return size;
}					// End method len


string string::substr( int a, int b )

   //  This method returns the substring in the range str[ a..b ]
   //
   //  a:  Start index of substring
   //  b:  End index of substring
{
   string  ret_str;			// String object to return


   if ( a < 0 || a >= size || b < 0 || b >= size || a > b )
      return ret_str;

   //  Ensure ret_str.str large enough to hold ( b - a + 1 ) chars plus
   //  one char for end-of-string marker

   if ( ( b - a + 2 ) > ret_str.m_size ) {
      do {
         ret_str.m_size *= 2;
      }
      while( ( b - a + 2 ) > ret_str.m_size );

      delete ret_str.str;
      ret_str.str = new char[ ret_str.m_size + 1 ];
      assert( ret_str.str != NULL );
   }

   ret_str.size = b - a + 1;
   strncpy( ret_str.str, &str[ a ], b - a + 1 );
   ret_str.str[ ret_str.size ] = '\0';

   return ret_str;
}					// End method substr


int string::token( string buf[], int n, char *sep )

   //  This method parses the string into individual words, and stores
   //  those words in the given array
   //
   //  buf:  Buffer to use to store words
   //  n:    Maximum size of buffer
   //  sep:  Separator character(s) to use
{
   int  elem = 0;			// Current buffer element
   int  i = 0;				// Current line element
   int  s;				// Start of substring position


   while( i != size && elem < n ) {

   //  First, set current token string to be empty, then strip any white
   //  space which may be sitting in front of the next token

      for( ; i != size && strchr( sep, str[ i ] ) != NULL; i++ );

      if ( i == size )			// Line ended in sep chars?
         return elem;

      if ( str[ i ] == '\"' ) {		// Process quoted text?
         i++;				// Ignore quote character
         s = i;				// Store start substring position

         while( i != size && str[ i ] != '\"' )
            i++;

         buf[ elem ] = substr( s, i - 1 );

         if ( i == size ) {
            printf( "Warning: Mismatched quotes\n");
            return elem;
         }

         i++;				// Ignore closing quote
         elem++;			// Increment token count

   //  Otherwise, if not end of line, process normal token

      } else {
         s = i;				// Store start substring position

         while( i != size && strchr( sep, str[ i ] ) == NULL )
            i++;

         buf[ elem ] = substr( s, i - 1 );
         elem++;
      }
   }

   return elem;				// Return token count
}					// End method token


int string::operator==( const string& s )

   //  Compare operator for strings, returns TRUE if strings are equal,
   //  FALSE otherwise
   //
   //  s:  String to compare against
{
   return !strcmp( str, s.str );
}					// End operator ==


int string::operator!=( const string& s )

   //  Compare operator for strings, return TRUE if strings not equal,
   //  FALSE otherwise
   //
   //  s:  String to compare against
{
   return strcmp( str, s.str ) != 0;
}					// End operator !=


int string::operator<=( const string& s )

   //  Compare operator for strings, returns TRUE if string <= s,
   //  FALSE otherwise
   //
   //  s:  String to compare against
{
   return strcmp( str, s.str ) <= 0;
}					// End operator <=


int string::operator>=( const string& s )

   //  Compare operator for strings, returns TRUE if string >= s,
   //  FALSE otherwise
   //
   //  s:  String to compare against
{
   return strcmp( str, s.str ) >= 0;
}					// End operator >=


int string::operator<( const string& s )

   //  Compare operator for strings, returns TRUE if string < s,
   //  FALSE otherwise
   //
   //  s:  String to compare against
{
   return strcmp( str, s.str ) < 0;
}					// End operator <


int string::operator>( const string& s )

   //  Compare operator for strings, returns TRUE if string > s,
   //  FALSE otherwise
   //
   //  s:  String to compare against
{
   return strcmp( str, s.str ) > 0;
}					// End operator >


char& string::operator[]( int i )

   //  Indexing operator for strings, return reference to given element
   //
   //  i:  Index into string
{
   if ( i < 0 || i >= size ) {
      printf("Warning: String out of bounds\n");

      BOUNDS_ERR = 0;			// Reference to error character
      return BOUNDS_ERR;
   }

   return str[ i ];
}					// End operator []


string& string::operator=( const string& s )

   //  Assignment operator for strings, copy from already existing object
   //
   //  s:  String object to copy from
{
   if ( s.size > m_size ) {		// Not enough space for old string?
      do {
         m_size *= 2;
      }
      while( s.size > m_size );

      delete str;
      str = new char[ m_size + 1 ];
      assert( str != NULL );
   }

   size = s.size;			// Copy instance variable values
   strcpy( str, s.str );

   return *this;
}					// End operator =


string string::operator+( const string& s )

   //  This method concatenates the given string onto our string
   //
   //  s:  String to concatenate
{
   string  ret_str;			// String object to return


   if ( size + s.size > ret_str.m_size ) {
      do {
         ret_str.m_size *= 2;
      }
      while( size + s.size > ret_str.m_size );

      delete ret_str.str;
      ret_str.str = new char[ ret_str.m_size + 1 ];
      assert( ret_str.str != NULL );
   }

   ret_str.str[ 0 ] = '\0';

   if ( size > 0 ) {
      strcpy( ret_str.str, str );	// Copy into string object
   }

   strcat( ret_str.str, s.str );
   ret_str.size = size + s.size;	// Set new string's length

   return ret_str;			// Return string object
}					// End operator +


string string::operator+( const char& c )

   //  This method concatenates the given character onto our string
   //
   //  c:  Character to concatenate
{
   char    arr[ 2 ];			// Array to hold char to concatenate
   string  ret_str;			// String object to return


   arr[ 0 ] = c;			// Create array with 1 char in it
   arr[ 1 ] = '\0';

   if ( size + 1 > ret_str.m_size ) {
      do {
         ret_str.m_size *= 2;
      }
      while( size + 1 > ret_str.m_size );

      delete ret_str.str;
      ret_str.str = new char[ ret_str.m_size + 1 ];
      assert( ret_str.str != NULL );
   }

   ret_str.str[ 0 ] = '\0';

   if ( size > 0 ) {
      strcpy( ret_str.str, str );	// Copy into string object
   }

   strcat( ret_str.str, arr );

   ret_str.size = size + 1;		// Set new string's length
   return ret_str;			// Return string object
}					// End operator +


string::operator char*()

   //  This method provides automatic conversion from string to char *
{
   return  str;
}					// End string to char * conversion


string::operator unsigned char*()

   //  This method provides automatic conversion from string to
   //  unsigned char *
{
  return (unsigned char *) str;
}					// End string to unsigned char *


string::operator float()

   //  This method provides automatic conversion from string to float
{
   return atof( str );
}					// End string to float conversion


string::operator int()

   //  This method provides automatic conversion from string to integer
{
   return atoi( str );
}					// End string to int conversion


string::string()

   //  Constructor for strings, initialize instance variables to given size
{
   m_size = 32;

   str = new char[ m_size + 1 ];
   assert( str != NULL );

   str[ 0 ] = '\0';
   size = 0;
}					// End constructor method


string::string( const char *s )

   //  Constructor for strings, cast from a character array to a string
   //
   //  s:  Character array to cast from
{
   m_size = 32;

   if ( !s ) {				// Assigning from NULL pointer
      str = new char[ m_size + 1 ];
      assert( str != NULL );

      str[ 0 ] = '\0';
      size = 0;

   } else {
      while( strlen( s ) > m_size ) {
         m_size *= 2;
      }

      str = new char[ m_size + 1 ];
      assert( str != NULL );

      size = strlen( s );		// Copy instance variable values
      strcpy( str, s );
   }
}					// End constructor method


string::string( const char c )

   //  Constructor for strings, cast from a character to a string
   //
   //  c:  Character to cast from
{
   m_size = 32;

   str = new char[ m_size + 1 ];	// Allocate memory for the string
   assert( str != NULL );

   str[ 0 ] = c;
   str[ 1 ] = '\0';

   size = 1;
}					// End constructor method


string::string( const string& s )

   //  Constructor for strings, assign from already existing object
   //
   //  s:  String object to copy from
{
   m_size = s.m_size;

   str = new char[ m_size + 1 ];	// Allocate memory for the string
   assert( str != NULL );

   size = s.size;			// Copy instance variable values
   strcpy( str, s.str );
}					// End copy constructor


string::~string()

   //  Destructor for strings, free any memory which may be allocated
{
   if ( str ) {				// Free memory, if it exists
      delete str;
   }
}					// End destructor method


ostream& operator<<( ostream& stream, const string& s )

   //  This function allows strings to be used as part of an output stream
   //  redirection chain
   //
   //  stream:  Output stream to direct string
   //  s:       String to output
{
   return stream << s.str;
}					// End operator <<


istream& operator>>( istream& stream, string& s )

   //  This function allows strings to be used as part of an input stream
   //  redirection chain
   //
   //  stream:  Input stream to direct into string
   //  s:       String to hold input
{
   char  tmp_str[ 1024 ];		// Temporary character array


   stream.getline( tmp_str, 1024 );	// Read one line of input

   s = tmp_str;
   return stream;
}			// End operator >>


//}//end namespace nstr