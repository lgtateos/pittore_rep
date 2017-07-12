#include "iostream"//#include <iostream.h>
#include "grid_file.h"
#include "highlight.h"
#include "global.h"
#include "random.h"
#include "paint.h"
#include "vec.h"


/*--------------------------------------------------------------------------*/
/*  HIGHLIGHT.CPP						            */
/*    Routines use the Floyd/Steinberg Error Diffusion technique to create  */
/*    a stokeset that will be drawn in the foreground to highlight a        */
/*    particular high spacial frequency property.			    */
/*					   				    */
/*- Modification History: --------------------------------------------------*/
/*  When:	Who:			Comments:			    */
/*								  	    */
/*  17-Jan-02	Laura G. Tateosian	Initial implementation		    */  
/*--------------------------------------------------------------------------*/

//  Module global prototypes

extern void  comp_highlight_size( stroke_s&);
extern float get_value( int, float [] );  //function in main.cpp
extern int   map_stroke( float, float , stroke_s& );  //function in main.cpp 

static void  delete_array( int h );
inline void  diffuse_error( float**&, int, int, float, int w );
static int   expand_st( stroke_s* &st, int& st_max, int size );
static void  jiggle( float&, float& );
static void  map_highlight_strokes( stroke_s* &, int w, int h);
static int   scan_data( int** &status_array, float** &, int w, int h);

//  Module global variables
float   attr_max;		// Max value of the highlighted attr
float   attr_min;		// Min value of the highlighted attr
float   flag = -1000;			// Indicates invalid data value
//int     **status_array;			// status_array: 1 means stroke; 0 means no stroke

//  Function definitions

static void delete_array( int h){
  //  This routine frees memory that after the array is no longer needed
  
  int i;  	// Loop counter
  for ( i = 0; i < h; i++ ){
//    delete [] status_array[ i ];
  }
  
//  delete [] status_array;
}			// End routine delete_arrays

inline void diffuse_error( float** &accum, int y, int x, float error, int w) {

  //  Distribute difference between rounded value and actual value in accum across 
  //  adjacent array members.  Snow plowing from left to right, top to bottom

  float value1, value2, value3;   //  Dummies for out-of-array edges 
  value1 = value2 = value3 = 0;
  
  if ( x+1 < w - 1 ) {
    if(accum[y][x+1] > flag){
      //grid square to the right gets 7/16 of the error
      accum[ y ][ x+1 ] = 7*error/16 + accum[ y ][ x+1 ];
      value1 = 7*error/16;
    }
  }
  if ( y - 1 >=  0 && x - 1 >= 0 ){
    if(accum[ y-1 ][x-1 ] > flag ){
      //grid square below and to the left gets 3/16 of the error
      accum[ y-1 ][ x-1 ] = 3*error/16 + accum[ y-1 ][ x-1 ];
      value2 = 3*error/16;
    }
  }
  if ( y - 1 >=  0 ){
    if (accum[ y-1][ x ] > flag){
      //grid square below gets 5/16 of the error
      accum[ y-1 ][ x ] = 5*error/16 + accum[ y-1 ][ x ];
      value3 = 5*error/16;
    }
}
  if ( y-1 > 0 && x+1 < w - 1 ){
    if ( accum[ y-1 ][ x+1 ] > flag ){
      //grid square below and to the right gets the rest of the error 
      accum[ y-1 ][ x+1 ] = error - ( value1 + value2 + value3 ) + accum[ y-1 ][ x+1 ];
    }
}
 // file<< " accum["<<y<<"]["<<x<<"] = "<<accum[y][x]<<" error = "<<error<<endl;
 
}			// End routine diffuse_error

static int expand_st( stroke_s* &st, int& st_max, int size )

  //  This routine doubles the size of the given stroke list
  //
  //  st:      Stroke list to double
  //  st_max:  New maximum list size
{
  int       i;				// Loop counter
  stroke_s *tmp_st;			// Temporary stroke list


  tmp_st = new stroke_s[ size ];
  if ( !tmp_st ) {
    cerr << "expand_st(), out of memory creating new stroke list\n";
    return 0;
  }

  for( i = 0; i < st_max; i++ ) {	// Copy list
    tmp_st[ i ] = st[ i ];
  }

  delete [] st;				// Reference new list
  st = tmp_st;

  st_max = size;				// Double list size
  return 1;
}					// End routine expand_st

int highlight( int** &status_array, int frame, float hfactor, int num_hig, int num_attr, int w, int h, float min, float max, grid_file* gf ){
  //  This routine begins the highlight stoke list creation process. 
  //  
  //  status_array: status_array: 1 means stroke; 0 means no stroke
  //  num_hig:  Index of the highlight feature in list in attr_map.cpp::name(int)
  //  accum:    Array of data values from highlighted attribute
  //  attr:		The attributes being highlighted 
  //  w:		The width of the input data 
  //  h:		The height of the input data
  //  min:		The minimum value of the highlighted attribute
  //  max:		The maximum value of the highlighted attribute
  //  high_st:	The stoke list for the highlights
  int     i; int j;  // Loop counters
  float** accum; // Data values of highlighted attribute
  attr_min = min;
  attr_max = max;
  int count = 0;
  bool status_array_exists = true;

//  cout<<"In highlight.cpp, highlight, the min = "<<min<<" and the max = "<<max<<endl;
  
  if (num_hig == -1){
  	cerr<<"In highlight.cpp: highlight has not been mapped to any data attribute"<<endl;
	return 0;
  }
  
  accum = new float *[ h ];      // Create highlight value array	
  if( status_array == NULL ){
    status_array = new int* [h];
    status_array_exists = false;
  }
  for ( i = 0; i < h; i++ ){
    accum[ i ] = new float[ w ];
    if( !status_array_exists ){
      status_array[i] = new int [w];
    }
  }
  //  init_array( w, h);
  float* value;     //  Array to hold all of the attributes' values
  value = new float[ num_attr ];
  
  // set all valid values to the range of [0,hfactor]
  // set all invalid values to -1
  for ( i = 0; i < h; i++) {
    for ( j = 0; j < w; j++ ) {
      status_array[i][j] = 0;
      gf->get( ( i*w ) + j, frame, value );
		  if ( gf->allowable(value) ){
        accum[i][j] = get_value(num_hig, value)* hfactor; //multiply normalized value by hfactor
		  }else{
			  accum[i][j] = -1000;
		  }
		  // cout<<"the value at x = "<<j<<" , y = "<<i<<" = "<<hlight_val[i][j]<<endl;
    }
  }
  
  count = scan_data(status_array, accum, w, h);
/*
  if ( count > array_size ){
    expand_st( st, max _count, count);
  }
*/
  //map_highlight_strokes(high_st, w, h);

  //delete_array( h );
  for ( i = 0; i < h; i++ ){
     delete [] accum [ i ];
  }
  delete value;
  		  
  delete [] accum;

  return count;
}			// End routine highlight

static void jiggle( float& x, float& y ){
  
  //  This routine randomly offsets the x, y coordinates of the highlight strokes  
  //
  //  x, y the coordinate values to be jiggled

   float rand1 = rand_num();
   float rand2 = rand_num();
   if (rand1 < .25){
     x += rand_num();
     y += rand_num();
   }else if (rand1 < .5){
     x -= rand_num();
     y -= rand_num();
   }else if (rand1 <.75){
     x -= rand_num();
     y += rand_num();
   }else{
     x += rand_num();
     y -= rand_num();
   }

   if (rand2 < .25){
     x += rand_num();
     y += rand_num();
   }else if (rand2 < .5){
     x -= rand_num();
     y -= rand_num();
   }else if (rand2 <.75){
     x -= rand_num();
     y += rand_num();
   }else{
     x += rand_num();
     y -= rand_num();
   }
} 			// End routine jiggle

/*
static void map_highlight_strokes(stroke_s* &high_st, int w, int h ){
  int i; int j; int fit1; int fit2;		// Loop counter
  int index;				// high_st array index
  float x_coord; float y_coord;   // coordinates to lay stroke

  for (i = 0; i < h; i++){
    for (j = 0; j < w; j++){
      fit1 = 0;
      fit2 = 0;
      index = i*w + j;
      
      if(status_array[i][j] == 1 ){
		    x_coord = j*4;        //HACK !!!! HACK !!!! HACK !!!!!  assumed value 4 for PIX_W
        y_coord = i*4;
        jiggle( x_coord, y_coord );
        fit1 = map_stroke( x_coord, y_coord, high_st[ index ]); //map_stroke in main.cpp
  	
        if ( fit1 ){
			    high_st[ index ].p.x = x_coord;
			    high_st[ index ].p.y = y_coord;
			    comp_highlight_size( high_st[ index ] ); //comp_highlight_size in main.cpp
			    high_st[ index ].r = .5843; 
			    high_st[ index ].g = 1;
			    high_st[ index ].b = .1686;
			    high_st[index].rot += 90; 
		    }else { 
			    fit2 = map_stroke( j, i, high_st[ index ]); //map_stroke in main.cpp

			    if ( fit2 ){
				    high_st[ index ].p.x = j;
				    high_st[ index ].p.y = i;
				    comp_highlight_size( high_st[ index ] ); //comp_highlight_size in main.cpp
				    high_st[ index ].r = .7; 
				    high_st[ index ].g = 1;
				    high_st[ index ].b = 0;
				    high_st[index].rot += 90; 
			    }
        }
      }
  	
      if ( fit1 == 0 && fit2 == 0 ){	//if status_array[i][j] == 0 or both fit attempts unsuccessful
        high_st[index ].r = -1;  //flag stroke not to be drawn
      }
    }
  }
}			// End routine map_highlight_strokes
*/
static int scan_data(int** &status_array, float** &accum, int w, int h)

  //  This routine scans through the data from the top left corner of the data
  //  as it appears on the screen, updating accum with error diffusion and 
  //  status_array when the associated accum value becomes >=.5
  //  Returns the count of ones in status_array (the number of highlight strokes)
  //
  //  accum:    Array of data values from highlighted attribute
{
  int i; int j;	 //Loop counter
  float error; //Difference between actual and rounded value
  int rounded_value;
  int count = 0;

  for( i = h - 1; i > -1; i-- )
  {
    for ( j = 0; j < w;  j++ ) 
    { 
      if (accum[i][j] > flag){
        if ( accum[i][j] >= 0 ){
          rounded_value = (accum[i][j] >= 0.5) ? 1 : 0;
          status_array[i][j] = rounded_value;
          count += rounded_value;  // adds one if rounded up.(zero otherwise)
        }else{
          rounded_value = 0;
          status_array[i][j] = 0;
        }
	      //	cerr<<"status_array[i][j]= "<< status_array[i][j]<<endl;     
        error = accum[i][j] - rounded_value;
	      diffuse_error(accum, i, j, error, w);
      }
    }
  }
  return count;
}			// End routine scan_data
