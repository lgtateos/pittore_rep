#include "reporter.h"


void report( int numb, int t, grid_file gfile){
  
  // After gridfile has been opened, this can be called to report the footer.
  //
  // numb: the number of attributes for each element
  // t		 Number of time steps/frames set to one to process frames separately 
  // gfile: grid_file object for handling details of file access
  // 
  //////////////////////////////////////////////////////////////
  int mm;
  string*  attribute_names;  // Attribute name
  int* col;			   // Number of columns of data per frame
  float data;		   // Temporary data value variable

  ifstream in;		   // Input file stream
  float* hi;		   // Highest allowable endpoint variable
  float* lo;		   // Lowest allowable endpoint variable			
  attr_map* map;       // Feature map ID
  float* min;          // Minimum/maximum data attribute value
  float* max;
  int n;			   // Number of attributes
  ofstream out;		   // Output file stream
  int* row;			   // Number of rows of data per frame
   
  int true_t;          // Number of time steps/frames
  int* frame_size;     // Number of elements in the first frame
  attribute_names = new string [ numb ];
  map = new attr_map [ numb];
  lo = new float[ numb];
  hi = new float[ numb];
  frame_size = new int[ t ];
  row = new int[ t ];
  col = new int[ t ];
  min = new float[ numb ];
  max = new float[ numb ];
  mm = gfile.min_max( min, max );
  cout<<"Result of min_max = "<<mm<<endl;
  string tmp;
  gfile.describe( tmp, n, t, attribute_names, map, lo, hi, frame_size, row, col );


  int i; //counter


  ////////////////////////////////////////////////////////

  printf("n = %d\n", n);
  printf("t = %d\n", t);
  for( i = 0; i < t; i++ ) {
    printf("frame_size[ %d ] = %d\n", i, frame_size[ i ]);
  }
  for( i = 0; i < numb; i++ ) {		
  printf("attr_name[ %d ] = %s\n", i, (char*) attribute_names[i]);
  } 
 
  for( i = 0; i < numb; i++ ) {		// Print visual feature map IDs
    printf("map [ %d ] = %d\n", i, map[i].vf_get());
  } 

  for( i = 0; i < numb; i++ ) {		// Print lo and hi ALLOWABLE value range boundaries
    printf("lo [ %d ] = %f    hi [ %d ] = %f\n" ,i , lo[i], i, hi[i]);
  }
  
  for ( i = 0; i < numb; i++){   // Print min and max valid data values
    printf("min [ %d ] = %f    max [%d ] = %f\n", i, min[i], i,max[i]);
  }

  // Clean memory
  delete [] attribute_names;
  delete [] map;
  delete [] lo;
  delete [] hi;
  delete [] frame_size;
  delete [] row;
  delete [] col;
  delete [] min;
  delete [] max;

} //end of report
