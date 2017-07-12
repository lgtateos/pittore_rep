#include "tile.h"
// TILE.CPP   Laura Tateosian 7/12/06

// Contains methods for creating a tiled style underpainting of a data field

extern int map_color( float x, float y, float color [ 3 ] );//function in main.cpp
static int color_corners(Point corner[], float red[], float green[], float blue[]);
static int color_canvas( Point corner[], float red[], float green[], float blue[], voronoi* voron);
int data_w; int data_h;
int pixel_w; int pixel_h;
float canv_dim;

static int color_canvas( Point corner[], float red[], float green[], float blue[], voronoi* voron, int numvalidpts, int fra_n){
  int count;
  float color [ 3 ];
  int k, m;
  int rc;
  float floorx, floory;
  int reg;
  stroke_s stroke;
  Point pt, tmp;

  rc = color_corners( corner, red, green, blue ); // check if all corners valid
  if (rc == 1){ // if so we're done
    return 1;
  }else if ( rc == 0 ){
    return 0;
  }
  
  // else, try other methods for assigning color
  for( k = 0; k < 4; k++ ){
    if( red[k] == -1 ){ // if corner is invalid
      // give it centroid's color
      floorx = floor((float)corner[k].x/(float)pixel_w);
      floory = floor((float)corner[k].y/(float)pixel_h);
      reg = voron->find_centroid( fra_n, floorx, floory );
      if ( reg >= 0 && reg < numvalidpts ){
        pt = voron->get_centroid_coor( fra_n, reg );
        if (map_color( pt.x*pixel_w,  pt.y*pixel_h, color)){
            red[ k ] = color[ 0 ]; green[ k ] = color[ 1 ]; blue[ k ] = color[ 2 ];
        }
      }else if ( red[k]==-1 ){// if reg was invalid OR map_color didn't work
        // Try using value at center of canvas square
        tmp.x = corner[0].x  + (corner[1].x - corner[0].x)*0.5;
        tmp.y = corner[0].y  + (corner[3].y - corner[0].y)*0.5;
        if (map_color( tmp.x,  tmp.y, color)){
          red[ k ] = color[ 0 ]; green[ k ] = color[ 1 ]; blue[ k ] = color[ 2 ];    
        }else if ( rc = 2 ){  // else use the value of one of the other valid corners
            count = 0;
            for( m = 0; m < 4; m++ ){
              if ( red[ m ] != -1 ){
                red[ k ] = red [ m ]; green[ k ] = green[ m ];  blue[ k ] = blue[ m ]; 
                count++;
              }
            }  
              if ( count < 2 ){
    return 0;
  }
        }
      }
    }
  }
  

   for( k = 0; k < 4; k++ ){
     if ( red[ k ] == -1 ){
       return 0;
     }
   }
   return 1; 
}
static int color_corners(Point corner[], float red[], float green[], float blue[]){
  // Colors each valid corner.  Reports level of success by
  // returning 0 if all corners are invalid, 1 if all corners
  // are valid, and 2 if it's mixed
  
  int has_valid = 0;
  int has_invalid = 0;
  float color[ 3 ];
  for ( int k = 0; k < 4; k++ ){
    if ( corner[k].x >= 0 && corner[k].x/pixel_w < data_w && corner[k].y >=0 && corner[k].y/pixel_h < data_h ){
      if (map_color( corner[k].x,  corner[k].y, color)){
        red[ k ] = color[ 0 ]; green[ k ] = color[ 1 ]; blue[ k ] = color[ 2 ];    
        has_valid = 1;
      }else{
        red[ k ] = -1;
        has_invalid = 1;
      }
    }
  }
  if ( !has_valid ){  // all corners are invalid
    return 0;
  } if( !has_invalid ){  //all corners are valid
     return 1;
  } else{
    return 2; // mixed bag
  }
}

float set_canvas_dim( voronoi* voron, int fra_n  ){
   //voron->max_radius[fra_n]*.6;//voron->max_radius[fra_n]*2;//((pixel_w*2)+4)*4;//8 //2 //4//voron->max_radius[fra_n]*2;
    //canv_dim = 100; //for mucha and IC (USA)
	canv_dim = 30; //for complex (USA)
	return canv_dim;
}


void draw_low_detail2( bool textured, bool rectangular, int d_w, int d_h, int pix_w, int pix_h, float lowleft[ 2 ], float upright[ 2 ], voronoi* voron, int numvalidpts, int fra_n  ){
  data_w = d_w; data_h = d_h;
  pixel_w = pix_w; pixel_h = pix_h;

  float dx = upright[0] - lowleft[0];
  float dy = upright[1] - lowleft[1];
  if ( rectangular ){
    dx = data_w*pixel_w;
    dy = data_h*pixel_h;
    lowleft[0] = 0;    lowleft[1] = 0;
    upright[0] = data_w;    upright[1] = data_h;
  }

  float red[ 5 ];  float green[ 5 ];   float blue[ 5 ];
  int i; int j; int k; int m;
  int rc;
  int flag; int  found;
  int floorx; int floory;
  int reg;
  stroke_s stroke;
  
  float fcd = 0.25*canv_dim;
  Point corner [4];
  Point mini[4];
  Point box[ 4 ];
  Point base[ 4 ];
  Point pt;
  Point tmp;

  int canvas_cols = ceil( dx/canv_dim );
  int canvas_rows = ceil( dy/canv_dim );


  //cout<<" ur  = "<<upright[0]<<" "<<upright[1]<<endl;
  //cout<<" ll  = "<<lowleft[0]<<" "<<lowleft[1]<<endl;
  box[ 0 ].x = 0; box[ 0 ].y = 0;
  box[ 1 ].x = 0.5*canv_dim; box[ 1 ].y = 0;
  box[ 2 ].x = 0.5*canv_dim; box[ 2 ].y = 0.5*canv_dim;
  box[ 3 ].x = 0; box[ 3 ].y = 0.5*canv_dim;
 /* //for debugging
  glPushMatrix();
  glTranslatef( lowleft[fra_n][0] + dx*.5, lowleft[fra_n][1] + dy*.5, 0);
  glScalef( dx, dy, 1);
  draw_stroke(0,1, 0);
  glPopMatrix();
*///end for debugging
  

  for (  i = 0; i < canvas_rows; i++ ){
    for ( j = 0 ;  j < canvas_cols; j++ ){
      if (textured){
        start_stroke();
      }  
      glPushMatrix();
      
      stroke.p.x = lowleft[0] + j*canv_dim + 0.5*canv_dim;
      stroke.p.y = lowleft[1] + i*canv_dim + 0.5*canv_dim;
      glTranslatef( stroke.p.x, stroke.p.y, 0);
     // glScalef( canv_dim, canv_dim, 1);
      //cout<<"width = "<<width<<" height = "<<height<<endl;
      
      // 3    2
      //  x  x
      //  x  x
      // 0    1 

      // Determine corner vertex colors
      corner[0].x = stroke.p.x - 0.5*canv_dim;
      corner[0].y = stroke.p.y - 0.5*canv_dim;
      corner[1].x = stroke.p.x + 0.5*canv_dim;
      corner[1].y = stroke.p.y - 0.5*canv_dim;
      corner[2].x = stroke.p.x + 0.5*canv_dim;
      corner[2].y = stroke.p.y + 0.5*canv_dim;
      corner[3].x = stroke.p.x - 0.5*canv_dim;
      corner[3].y = stroke.p.y + 0.5*canv_dim;
    
      rc = color_corners( corner, red, green, blue );
      if ( rc == 1 ){  // all corners valid.  draw canvas square
        glScalef( canv_dim, canv_dim, 1);
        if (!textured){
          draw_stroke_no_tex(red,green,blue);
        }else{
        draw_stroke_smooth_shading( red, green, blue, 0, 0.5 );
        }
      } else if ( rc == 2 || rc == 0){  // mixed bag.  try splitting into 4 squares
        // get corners of bottom left square to use as anchors for each quarter
        base[ 0 ] = corner[ 0 ];
        base[ 1 ].x = corner[ 0 ].x + (corner[ 1 ].x - corner[ 0 ].x)*0.5;
        base[ 1 ].y = corner[ 0 ].y;
        base[ 2 ].x = corner[ 0 ].x + (corner[ 1 ].x - corner[ 0 ].x)*0.5;
        base[ 2 ].y = corner[ 0 ].y + (corner[ 3 ].y - corner[ 0 ].y)*0.5;
        base[ 3 ].x = corner[ 0 ].x;
        base[ 3 ].y = corner[ 0 ].y + (corner[ 3 ].y - corner[ 0 ].y)*0.5; 
        
        for ( k = 0; k < 4; k++ ){
          // shift box to position of canvas
          mini[ 0 ] = base[ k ] + box[ 0 ];
          mini[ 1 ] = base[ k ] + box[ 1 ];
          mini[ 2 ] = base[ k ] + box[ 2 ];
          mini[ 3 ] = base[ k ] + box[ 3 ];
        
        // draw or reject bottom left square
          rc = color_canvas( mini, red, green, blue, voron, numvalidpts, fra_n );
          if ( rc == 1 ){
            glPushMatrix();
            if ( k == 0 ){
         
             glTranslatef( -fcd, -fcd, 0 );
            //  glTranslatef(  -0.25*canv_dim , -0.25*canv_dim, 0);
             }else if ( k == 1 ){
             glTranslatef( fcd , -fcd, 0);
             //  glTranslatef( 0.25*canv_dim , -0.25*canv_dim, 0);
           }else if ( k == 2 ){
              glTranslatef( fcd , fcd, 0);
              //glTranslatef( 0.25*canv_dim , 0.25*canv_dim , 0);
           } else if ( k == 3 ){
            // glTranslatef( -20 , 20 , 0); 
             glTranslatef( -fcd , fcd , 0);
            }
           
           /*
        if ( textured ){
          red[0] = 1; green[0] = 1;  blue[0] = 1;
          red[1] = 1; green[1] = 1;  blue[1] = 1;
          red[2] = 1; green[2] = 1;  blue[2] = 1;
          red[3] = 1; green[3] = 1;  blue[3] = 1;

        }else {
          red[0] = 0; green[0] = 0;  blue[0] = 1;
          red[1] = 0; green[1] = 0;  blue[1] = 1;
          red[2] = 0; green[2] = 0;  blue[2] = 1;
          red[3] = 0; green[3] = 0;  blue[3] = 1;
        }
           */
           glScalef( canv_dim*.5, canv_dim*.5, 1);
           if (!textured){ 
             draw_stroke_no_tex(red, green, blue);}
           else{
             draw_stroke_smooth_shading( red, green, blue, 0, .25 );
           }
            glPopMatrix();
          }
        }
      }
       //  draw_stroke(red[0],green[0],blue[0], 0);
      glPopMatrix();
      if(textured){
        stop_stroke();
      }
    }
  }
} // end routine draw_low_detail2



void draw_low_detail( bool textured, bool rectangular, int d_w, int d_h, int pix_w, int pix_h, float lowleft[ 2 ], float upright[ 2 ], voronoi* voron, int numvalidpts, int fra_n  ){
  data_w = d_w; data_h = d_h;
  pixel_w = pix_w; pixel_h = pix_h;

  float dx = upright[0] - lowleft[0];
  float dy = upright[1] - lowleft[1];
  if ( rectangular ){
    dx = data_w*pixel_w;
    dy = data_h*pixel_h;
    lowleft[0] = 0;    lowleft[1] = 0;
    upright[0] = data_w;    upright[1] = data_h;
  }

  float color[3]={1,1,1};
  float red[ 5 ];  float green[ 5 ];   float blue[ 5 ];
  int i; int j; int k; int m;
  int rc;
  int flag, found;
  int floorx, floory;
  int reg;
  stroke_s stroke;
  
  float fcd = 0.25*canv_dim;
  Point corner [4];
  Point mini[4];
  Point box[ 4 ];
  Point base[ 4 ];
  Point pt;
  Point tmp;
  int count = 0;;
  for (  i = lowleft[1]; i < upright[1]; i++){//i < upright[1]; i++ ){
    for ( j = lowleft[0] ;  j < upright[0]; j++){//j < upright[0]; j++ ){   
      stroke.p.x = j;
      stroke.p.y = i;
      
      if (map_color(stroke.p.x,  stroke.p.y, color)){
        count++;
        if (textured){
          start_stroke();
        }
        glPushMatrix();
        glTranslatef( stroke.p.x, stroke.p.y, 0);
        stroke.r = color[0]; stroke.g = color[1]; stroke.b = color[2];
        glScalef( pix_w, pix_h, 1);
        if (!textured){
          draw_stroke_no_tex(stroke.r, stroke.g, stroke.b);
        }else{
          draw_stroke_choptex(stroke.r, stroke.g, stroke.b,1, 0);
        }

        glPopMatrix();
       if(textured){
        stop_stroke();
       }
      }

    }
  }
  cout<<"THE count = "<<count<<endl;
  
} // end routine draw_low_detail


void draw_canvas( bool textured, int d_w, int d_h, int pix_w, int pix_h, float color [ 3 ]  ){
  //canv_dim = 64; pulsar
  float dx;
  float dy;
 
  dx = d_w*pix_w;
  dy = d_h*pix_h;


  float red[ 5 ];  float green[ 5 ];   float blue[ 5 ];
  int i; int j; int k; int m;
  int rc;
  int flag; int found;
  int floorx; int  floory;
  int reg;
  stroke_s stroke;
  
  float fcd = 0.25*canv_dim;

  int canvas_cols = ceil( dx/canv_dim );
  int canvas_rows = ceil( dy/canv_dim );

  for (  i = -10; i < canvas_rows + 10; i++ ){
    for ( j = -10 ;  j < canvas_cols + 10; j++ ){
      if (textured){
        start_stroke();
      }

      glPushMatrix();
      
      stroke.p.x =  j*canv_dim + 0.5*canv_dim;
      stroke.p.y =  i*canv_dim + 0.5*canv_dim;
      glTranslatef( stroke.p.x, stroke.p.y, 0);
                 
      if ( textured ){
        red[0] = 1; green[0] = 1;  blue[0] = 1;
        red[1] = 1; green[1] = 1;  blue[1] = 1;
        red[2] = 1; green[2] = 1;  blue[2] = 1;
        red[3] = 1; green[3] = 1;  blue[3] = 1;

      }else {
        red[0] = color[0]; green[0] = color[1];  blue[0] = color[2];
        red[1] = color[0]; green[1] = color[1];  blue[1] = color[2];
        red[2] = color[0]; green[2] = color[1];  blue[2] = color[2];
        red[3] = color[0]; green[3] = color[1];  blue[3] = color[2];
      }

      glScalef( canv_dim, canv_dim, 1);
      if (!textured){ 
        draw_stroke_no_tex(red, green, blue);
      } else{
        draw_stroke_smooth_shading( red, green, blue, 14, 1);
      }
      glPopMatrix();

      if(textured){
        stop_stroke();
      }
    }
  }
} // end routine draw_low_detail