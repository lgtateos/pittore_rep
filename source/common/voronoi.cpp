//#include <GL/glut.h>
#include "voronoi.h"  
#include "math.h"
#include "global.h"
#include "colour.h"

voronoi::voronoi(){
  n_display_fr = 0;  //  Number of frames to display (for debugging purposes can be less than num_frames).
  num_frames = 0;
  num_vv = 0;
}  // end constructor


voronoi::voronoi(char* bin_fname, int n_faces, int n_display_frames, bool FULL_SET ){
  init_voronoi( bin_fname, n_faces, n_display_frames, FULL_SET );
}  // end constructor


voronoi::~voronoi(){
  free_voronoi();
}  // end destructor

int voronoi::add_to_ptarray(  Point pt, int cur_frame, int& ptarray_max ){//, Point* &ptarray ){
  //Checks if pt exists already; if not adds point to ptarray.
  //Returns pt's index into ptarray.
  int j,k;
  bool found;
  int index;

    // Search the pt array to check for duplication.
    found = 0;
    for ( k = 0; k < num_vv && !found; k++ ){
      if ( pts[cur_frame][ k ] == pt ){
        found = 1;
        index = k;
      }
    }
    if ( found ){  // It's already in the point list.  Enter that point's index.
      return index;  // j+1, since the first entry holds the count
    }else {
    
      // Add new entry (pt) to the point list. 
      if ( num_vv == ptarray_max ){  // Check the size and expand, if necessary.
        expand_ptset( pts[cur_frame], ptarray_max ); 
      }

      pts[cur_frame][ num_vv ] =  pt;
      //  cout<<"here i = "<<i<<" j = "<<j<<" pt_count = "<<pt_count<<endl;
      num_vv++; //up the point count
      return num_vv - 1;  // return the index
    }
}

void voronoi::comp_line( Point s, Point t, float& m_inv, float& b )

  //  This private routine computes the inverse slope and intercept of
  //  the line formed by the given points
  //
  //  s, t:   Points on line
  //  m_inv:  Inverse of slope
  //  b:      Intercept
{
  if ( s.y == t.y ) {
    m_inv = 0;
  } else {
    m_inv = ( t.x - s.x ) / ( t.y - s.y );
  }

  if ( m_inv == 0 ) {
    b = s.y;
  } else {
    b = s.y - ( s.x / m_inv );
  }
}					// End routine comp_line

inline Point voronoi::comp_midpt( Point s, Point t ){
  Point midpt;
  midpt.x = (s.x + t.x)*0.5;
  midpt.y = (s.y + t.y)*0.5;
  return midpt;
}


void voronoi::min_max_radius( float& min, float& max, int cur_frame){
  // cur_frame: current frame
  //
  // This routine finds the largest distance from a voronoi centroid to on of its regions polygonal vertices.
  // During the routine, find_areas, the index of the largest region was identified.  So it just remains to 
  // calculate the distance from center to farthest point.
  
  Point center; 
  float dist = 0;
  int i;  // counter
  int indexL = max_area_index[cur_frame];  // Index of the largest voronoi region
  int indexS = min_area_index[cur_frame];  // Index of the smallest voronoi region
  float tmp;
  Point vert;
  
  //Find the largest radius of the largest region, indexL
  center = data_pts[cur_frame][indexL];

	for( i = 0; i < regions[cur_frame][ indexL ].card ; i++ ){
	vert =  pts[cur_frame][ regions[cur_frame][indexL].ppts[i] ]; // ppts is a list of vertex numbers.  'pts' holds polygon vertices.
		tmp = ( center.x - vert.x )*(center.x - vert.x) + ( center.y - vert.y )*( center.y - vert.y );
		if ( tmp > dist ){
			dist = tmp;
		}
	}
 
  max = sqrt(dist);

  //Find the smallest radius of the smallest region, indexS
  center = data_pts[cur_frame][indexS];

  for( i = 0; i < regions[cur_frame][ indexS ].card ; i++ ){
	vert =  pts[cur_frame][ regions[cur_frame][indexS].ppts[i] ]; // ppts is a list of vertex numbers.  'pts' holds polygon vertices.
		tmp = ( center.x - vert.x )*(center.x - vert.x) + ( center.y - vert.y )*( center.y - vert.y );
		if ( tmp < dist ){
			dist = tmp;
		}
	}

  min = sqrt(dist);
}  // end method min_max_radius


void voronoi::draw_center( int pt_n, int pixel_w, int pixel_h, int cur_frame, float sphere_radius ){

  //Draw voronoi point
 glColor3f(0,0,0);
  // glColor3f(0.7,0.58,0.53);
	if ( pt_n  > (n_valid_pts[ cur_frame ] - 1) ){
	  glColor3f(0,0,1);
	}
  /*glTranslatef(data_pts[ cur_frame ][pt_n].x*pixel_w,  data_pts[ cur_frame ][ pt_n ].y*pixel_w, 0);
  glutSolidSphere(sphere_radius, 100,100);
  glTranslatef(-data_pts[ cur_frame ][pt_n].x*pixel_w,  -data_pts[ cur_frame ][ pt_n ].y*pixel_w, 0);
	*/glPointSize( 3);
	glBegin(GL_POINTS);
	  glVertex3f(data_pts[ cur_frame ][pt_n].x*pixel_w,  data_pts[ cur_frame ][ pt_n ].y*pixel_w, 0);
	glEnd();
  
  
} // end routine draw_center

void voronoi::draw_delauney(  int cur_frame, int pixel_w, int pixel_h ){
  //Draw delauney triangulation

  int num_neigh = 0;
  
  glPointSize( 3);

    glColor3f(0,.5,1);
      glBegin( GL_POINTS );
      glVertex3f( data_pts[cur_frame ][205].x*pixel_w, data_pts[ cur_frame ][205].y*pixel_h, 0 );
      glVertex3f( data_pts[cur_frame ][6].x*pixel_w, data_pts[ cur_frame ][6].y*pixel_h, 0 );
      glVertex3f( data_pts[cur_frame ][202].x*pixel_w, data_pts[ cur_frame ][202].y*pixel_h, 0 );
      glVertex3f( data_pts[cur_frame ][301].x*pixel_w, data_pts[ cur_frame ][301].y*pixel_h, 0 );
      glVertex3f( data_pts[cur_frame ][0].x*pixel_w, data_pts[ cur_frame ][0].y*pixel_h, 0 );
      glVertex3f( data_pts[cur_frame ][1].x*pixel_w, data_pts[ cur_frame ][1].y*pixel_h, 0 );
      glVertex3f( data_pts[cur_frame ][191].x*pixel_w, data_pts[ cur_frame ][191].y*pixel_h, 0 );
      glVertex3f( data_pts[cur_frame ][296].x*pixel_w, data_pts[ cur_frame ][296].y*pixel_h, 0 );
      glVertex3f( data_pts[cur_frame ][278].x*pixel_w, data_pts[ cur_frame ][278].y*pixel_h, 0 );
      glVertex3f( data_pts[cur_frame ][2].x*pixel_w, data_pts[ cur_frame ][2].y*pixel_h, 0 );
      glVertex3f( data_pts[cur_frame ][299].x*pixel_w, data_pts[ cur_frame ][299].y*pixel_h, 0 );
    glEnd();

/*
 // for( i = 0; i < num_delauney_reg ;  i++ ){
 // for( i = 3750; i < 3755 ;  i++ ){
   for( i = 3750; i < 3760 ;  i++ ){
    glColor3f(0,.5,1);
    glBegin( GL_POINTS );
      glVertex3f( data_pts[cur_frame ][i].x*pixel_w, data_pts[ cur_frame ][i].y*pixel_h, 0 );
    glEnd();

    num_neigh = delauney_ind[ cur_frame ][ i ][ 0 ];
    for ( j = 1; j <= num_neigh ; j++){
      glColor3f(1,.5,0);
      index = delauney_ind[ cur_frame ][ i ][ j ];
      if ( index > 0 ){
	      glBegin(GL_LINES);
           glVertex3f( data_pts[cur_frame ][i].x*pixel_w, data_pts[ cur_frame ][i].y*pixel_h, 0 );
	         glVertex3f(data_pts[ cur_frame ][index].x*pixel_w,  data_pts[ cur_frame ][ index ].y*pixel_h, 0);
	      glEnd();
        glBegin( GL_POINTS );
           glVertex3f(data_pts[ cur_frame ][index].x*pixel_w,  data_pts[ cur_frame ][ index ].y*pixel_h, 0);
        glEnd();
      }
    }
  }  */
}// end routine draw_delauney

void voronoi::draw_regs( int reg_n, int pixel_w, int pixel_h, int cur_frame, float r, float g, float b, GLenum mode ){
  //Draw outlines of the voronoi regions.  Draws regions around invalid points in blue.
  
  int cardinality = 0;
  int poly_pt;
  //Draw voronoi region filled in

  if ( mode == GL_SELECT ){
    glLoadName( reg_n );
  }
  
  glColor3f(r, g, b);
	if ( reg_n < n_valid_pts[ cur_frame ]){
    glBegin(GL_POLYGON);
      cardinality = regions[cur_frame][reg_n].card;
      for (int j = 0; j < cardinality; j++ ){ 
        poly_pt = regions[cur_frame][reg_n].ppts[j];  // The index number of the jth point on the reg_n th polygon
        glVertex3f( pts[cur_frame][ poly_pt ].x*pixel_w, pts[cur_frame][ poly_pt ].y*pixel_h, 0 );
		  }
    glEnd();
  }else { //if ( reg_n  > (n_valid_pts[ cur_frame ] - 1) ){
	  glColor3f(0,0,1);
       glBegin(GL_POLYGON);
      cardinality = regions[cur_frame][reg_n].card;
      for (int j = 0; j < cardinality; j++ ){  
        poly_pt = regions[cur_frame][reg_n].ppts[j];  // The index number of the jth point on the reg_n th polygon
        glVertex3f( pts[cur_frame][ poly_pt ].x*pixel_w, pts[cur_frame][ poly_pt ].y*pixel_h, 0 );
      }
    glEnd();
  }

} // end of draw_regs

void voronoi::draw_stars( int pixel_w, int pixel_h, int cur_frame, int max_x, int max_y ){
  int j, k;
  int position;
  float r, g, b; 
  float temp;
  Point tpt;

  // Draw stars, a line from each valid data point to the nearest centroid.
  int cent;
  glPointSize( 2 );
  position = 0;

  for ( j = 0; j < max_y; j++ ){
	  for ( k = 0; k < max_x; k++){
		  cent = centroid[ cur_frame ][ position ];
		  position++;
		  if ( cent > -1 && cent < n_valid_pts[ cur_frame ] ){
			  temp = ( areas[cur_frame][cent]*0.5 ) + 0.5; //mapping color to higher values; easier to see
			  get_colour( temp, r, g, b ); 
			  glColor3f(r, g, b);
        glBegin(GL_LINES);
	        glVertex2f(k*pixel_w,j*pixel_h);  //Draw line from tip
				  tpt = data_pts[cur_frame][cent];
	        glVertex2f( tpt.x*pixel_w, tpt.y*pixel_h );  // to center of voronoi region
        glEnd();
        glBegin(GL_POINTS);
          glVertex2f(k*pixel_w,j*pixel_h);  //Draw points at tips of stars
		    glEnd();
		  }
	  }
  }

} // end draw_stars


void voronoi::draw_web( int reg_n, int pixel_w, int pixel_h, int cur_frame, float r, float g, float b ){
  //Draw outlines of the voronoi regions.  Draws regions around invalid points in blue.
  
  int cardinality = 0;
  int poly_pt;
  //Draw voronoi region outline
  glColor3f(r, g, b);
	if ( reg_n < n_valid_pts[ cur_frame ]){
    glBegin(GL_LINE_LOOP);
      cardinality = regions[cur_frame][reg_n].card;
      for (int j = 0; j < cardinality; j++ ){ 
        poly_pt = regions[cur_frame][reg_n].ppts[j];  // The index number of the jth point on the reg_n th polygon
        glVertex3f( pts[cur_frame][ poly_pt ].x*pixel_w, pts[cur_frame][ poly_pt ].y*pixel_h, 0 );
		  }
    glEnd();
  }
/*
  glColor3f(1,0,0);
  if ( reg_n  > (n_valid_pts[ cur_frame ] - 1) ){
	  glColor3f(0,0,1);
       glBegin(GL_LINE_LOOP);
      cardinality = regions[cur_frame][reg_n].card;
      for (int j = 0; j < cardinality; j++ ){  
        
        poly_pt = regions[cur_frame][reg_n].ppts[j];  // The index number of the jth point on the reg_n th polygon
        if ( pts[cur_frame][ poly_pt ].x > -5 ){
          glVertex3f( pts[cur_frame][ poly_pt ].x*pixel_w, pts[cur_frame][ poly_pt ].y*pixel_h, 0 );
        }
      }
    glEnd();
  }
*/
} // end of draw_web


Point voronoi::find_com( int cur_frame, int reg_n ){

  int i;
  float A;  //area
  int cardinality;
  float xsum = 0;
  float ysum = 0;
  Point* pt_arr;
  A = get_area( cur_frame, reg_n);
  cardinality = get_cardinality( cur_frame, reg_n);
  pt_arr = new Point [ cardinality + 1];
  
  for ( i = 0; i < cardinality; i++){
    pt_arr[i] = get_poly_pt( cur_frame, reg_n, i);
  }

  pt_arr[cardinality] = pt_arr[0];  //set P_n = P_0

  for ( i = 0; i < cardinality; i++){
    //xsum+= (pt_arr[i].x + pt_arr[i+1].x)*( (pt_arr[i].x*pt_arr[i+1].y) - (pt_arr[i+1].x*pt_arr[i].y) );
    //ysum+=  (pt_arr[i].y + pt_arr[i+1].y)*( (pt_arr[i].x*pt_arr[i+1].y) - (pt_arr[i+1].x*pt_arr[i].y) );
    xsum+= pt_arr[i].x;
    ysum+= pt_arr[i].y;
  }
  
  delete [] pt_arr;
  
 // xsum = 1/(6*A)*xsum;
 // ysum = 1/(6*A)*ysum;
  xsum = xsum/(float)cardinality;
  ysum = ysum/(float)cardinality;
  
  Point tmp_pt;
  tmp_pt.x = xsum;
  tmp_pt.y = ysum;

  
  return tmp_pt;
}  // end routine find_com


inline double voronoi::find_dist_sq ( Point tmp1, Point tmp2 ){
  return (tmp1.x - tmp2.x)*(tmp1.x - tmp2.x) + (tmp1.y - tmp2.y)*(tmp1.y - tmp2.y);
}


double voronoi::find_datapt_dist_sq ( int cur_frame, Point pt1, int pt_n2 ){
  
  Point tmp2;
  
  tmp2.x = data_pts[cur_frame][pt_n2].x;
	tmp2.y = data_pts[cur_frame][pt_n2].y;

  return find_dist_sq(pt1, tmp2);
} // end routine find_dist_sq


double voronoi::find_polypt_dist_sq ( int cur_frame, int reg_n, int card_n1, int card_n2 ){
  Point tmp1;
  Point tmp2;
  
  tmp1.x = pts[cur_frame][ regions[cur_frame][reg_n].ppts[card_n1] ].x;
  tmp1.y = pts[cur_frame][ regions[cur_frame][reg_n].ppts[card_n1] ].y;
    
  tmp2.x = pts[cur_frame][ regions[cur_frame][reg_n].ppts[card_n2] ].x;
  tmp2.y = pts[cur_frame][ regions[cur_frame][reg_n].ppts[card_n2] ].y;

  return find_dist_sq(tmp1, tmp2);
} // end routine find_polypt_dist_sq

int voronoi::find_mixed_reg_index( int cur_frame, int x, int y, int region_id, float* valuey, int** reg_a){
  // This routine identifies boundary regions ( regions bordering regions in a different size category)
  //
  // Returns 0 if the current region is NOT abutting a region in a different size category.
  // Returns 1 if the current region is abutting a region in a different size category.
  //
  // This routine should only be called if segmentation_type == -1  

  int i, j;                      // counter
  int cardinality;
  int cur_x; int cur_y;       // current point ( for looping through neighbors )
  float corner_x; float corner_y;
  int nb_reg_id;
  float nb_reg_area;          // neighbor's region's area
  int rc;

  int    off[ 8 ][ 2 ] = {		// Offset of each neighbour
    { -1,  1 }, {  0,  1 }, {  1,  1 },
    { -1,  0 },             {  1,  0 },
    { -1, -1 }, {  0, -1 }, {  1, -1 }
  };

  rc = 0;  // default ID as small region with no large region neighbors

  cardinality = get_cardinality(cur_frame,region_id);
  for (j = 0; j < cardinality; j++ ){  
    corner_x = get_poly_pt(cur_frame, region_id, j ).x;
    corner_y = get_poly_pt(cur_frame, region_id, j ).y;
    for( i = 0; i < 8; i++ ) {		// For all neighbours
	    cur_x = corner_x - off[ i ][ 0 ]; // set cur_x to next neighbor's x position 
      cur_y = corner_y - off[ i ][ 1 ];
      
	    // if outside of dataset boundaries
      if ( cur_x < 0 || cur_x >= col[cur_frame]  || cur_y < 0 || cur_y >= row[cur_frame] ){
        continue;
      }
    
      nb_reg_id = find_centroid(cur_frame, cur_x, cur_y );
      if ( nb_reg_id == -1 ){  // no neighbor found ...why?
         continue;
      }
      if ( nb_reg_id == region_id ){  // lives in my region. I'm not interested.
        continue;
      }
      if ( nb_reg_id >= n_valid_pts[cur_frame] && reg_a[cur_frame][nb_reg_id] == -1 ){  // region has invalid center and has not been categorized yet
        continue;
      }
      
	    switch ( reg_a[cur_frame][nb_reg_id] ){  
        case 0:  // borders a small nonboundary region
          return 0;
        case 1:
          if ( rc != 3 ){
            rc = 1;
          }
          break;
        case 2:
          if ( rc != 3 ){
            rc = 2;
          }
          break;
        case 3:
          rc = 3;
          break;
        case 6:
          // do nothing
          break;
        default:
          cerr<<"In main.cpp: Region ID should be 0,1,2, 3, or 6. Region id = "<<reg_a[cur_frame][nb_reg_id]<<endl;
      } //end switch
      
    }// end i loop 
  } // end j loop

  return rc;
}   // end routine get_invalid_reg_id

void voronoi::find_areas( int cur_frame ){
  // Find the area of each region in the voronoi diagram that corresponds to a valid data entry.  
  // This method for finding the area of convex polygons finds the determinant of a 2D matrix and multiplies
	// it by 1/2.
  //
	// For coordinates listed counterclockwise (x_1, y_1),(x_2,y_2),....,(x_n,y_n),
	//
	//           | x_1  y_1  |
	//           | x_2  y_2  |
	//           | x_3  y_3  |
	// Area = 1/2|  .    .   | = 1/2[(x_1*y_2 + x_2*y_3 + x_3*y_4 +...+x_n*y_1) - (y_1*x_2 + y_2*x_3 + y_3*x_4 +...+y_n*x_1)]
  //           |  .    .   |        |-----------------sum1-----------------|     |-----------------sum2-----------------|
  //           |  .    .   |
	//           | x_n  y_n  |
	//           | x_1  y_1  |
	//
  // Note that if the coordinates are given clockwise, the result is -1*Area.
	
	//float x, y, u, v;
 
	int i, j, n; //counter
  float sum1, sum2;
  areas[ cur_frame ] = new float [ n_valid_pts[cur_frame] ]; 
  for( n = 0; n < n_valid_pts[cur_frame]; n++ ){
		 sum1 = 0;
		 sum2 = 0;
		 
		 // 
		 for( i = 0 ; i < regions[cur_frame][n].card;  i++){  // for cardinality of current polygon
		
			 if (i+1 < regions[cur_frame][ n ].card ){
			   j = i+1;
			 }else{
			   j = 0;
			 }
		 
       sum1 += pts[ cur_frame ][ regions[cur_frame][n].ppts[i] ].x * pts[ cur_frame ][ regions[cur_frame][n].ppts[j] ].y; 
			 sum2 += pts[ cur_frame ][ regions[cur_frame][n].ppts[i] ].y * pts[ cur_frame ][ regions[cur_frame][n].ppts[j] ].x;

		 } // end for cardinality of current polygon   
		 
		 // complete the calculation
		 areas[cur_frame][n] = 0.5*(sum1 - sum2);
		 // Some polygon vertices are listed in clockwise order (yielding -1*Area).  Correcting these here.
     if ( areas[cur_frame][n] < 0 ){
		   areas[cur_frame][n] = -1*areas[cur_frame][n];
		 }
		 
		 if( !n){  // if n==0, (i.e. beginning of new frame) initialize min and max
		   min_area[cur_frame] = areas[cur_frame][n];
			 max_area[cur_frame] = areas[cur_frame][n];
			 max_area_index[cur_frame] = 0;
       min_area_index[cur_frame] = 0;
		 } 
		 
		 if ( areas[cur_frame][n] < min_area[cur_frame] ){
       min_area_index[cur_frame] = n;
		   min_area[cur_frame] = areas[cur_frame][n];
		 }else if ( areas[cur_frame][n] > max_area[cur_frame] ){
			 max_area_index[cur_frame] = n;
		   max_area[cur_frame]= areas[cur_frame][n];
		 }
	} // end current polygon
	
  // Normalize ranges to use values (0-1) as color t-values
	float range = max_area[cur_frame] - min_area[cur_frame];
	for( n = 0; n < n_valid_pts[cur_frame]; n++){
		areas[cur_frame][n] = ( areas[cur_frame][n] - min_area[cur_frame] )/range;
	}
}  // end method find_areas


int voronoi::find_centroid( int cur_frame, float x, float y ){
	// This method uses a previously created distance table to
	// find the nearest neighbor of the given point.  Returns an index into the list of data_pts

	int ID;


	Point my_pnt;   
	
	my_pnt.x = x;
	my_pnt.y = y;

 dt[cur_frame]->GetNearestNeighbor_NonMember( my_pnt , ID );
 //dt[cur_frame]->GetNearestNeighbor( my_pnt , ID );
	return ID;
}  // end method find_centroid

int voronoi::find_centroid( int cur_frame, Point my_pnt ){
	// This method does not use a distance table
	// find the nearest neighbor of the given point.  Returns an index into the list of data_pts

	int ID = 0;
  
  double mindist = find_datapt_dist_sq( cur_frame, my_pnt, 0 ); // initialize dist
  double dist = mindist;

  for ( int i = 1; i < num_regions[ cur_frame ]; i++ ){
    dist = find_datapt_dist_sq( cur_frame, my_pnt, i );
    if ( dist < mindist ){
      mindist = dist;
      ID = i;
    }
  }

  return ID;
}  // end method find_centroid


float voronoi::find_near_reg( int frame, Point pt, int& reg){
  // pt: The point whose neighbors we're seeking.
  // reg:  The index of the nearest voronoi region with a valid center.
  
  // Returns the SQUARE distance to the center of the nearest valid region
  int i; //counter
  float dist;
  float tmp; // temporarily hold the distance

  // initialize vars
  reg = -1; 
  // Initialize distance var.
  dist = find_datapt_dist_sq ( frame, pt, 0 );

  for ( i = 0; i < n_valid_pts[frame]; i++ ){
    tmp = find_datapt_dist_sq ( frame, pt, i );
    if ( tmp < dist ){
      dist = tmp;
      reg = i;
    }
  }
  return dist;
}

void voronoi::find_near_regs( int frame, Point pt, int& vreg, float& distv, int& invreg, float& distinv){
  // pt: The point whose neighbors we're seeking.
  // vreg:  The index of the nearest voronoi region with a valid center.
  // invpt: The index of the nearest voronoi region with an invalid center.
  
  // distv:   hold the shortest distance to the center of a valid region
  // distinv: hold the shortest distance to the center of an invalid region
  int i; //counter
  float tmp; // temporarily hold the distance

  // initialize vars
  vreg = -1; invreg = -1;
 
  
  for ( i = 0; i < n_valid_pts[frame]; i++ ){
    tmp = find_datapt_dist_sq ( frame, pt, i );
    if ( tmp < distv ){
      distv = tmp;
      vreg = i;
    }
  }
  for( i = n_valid_pts[frame]; i < num_regions[frame]; i++ ){
     tmp = find_datapt_dist_sq ( frame, pt, i );
    if ( tmp < distinv ){
      distinv = tmp;
      invreg = i;
    }
  }

}


void voronoi::free_voronoi(){
  int i, j; //counter
  for( j = 0; j < n_display_fr; j++ ){
    for ( i = 0; i < num_regions[j]; i++ ){   
      delete [] regions[j][i].ppts;      
    }
    delete [] regions[j];
    delete [] areas[j];
    delete [] centroid[j];
    delete [] data_pts[j];
    delete dt[j];
    delete [] pts[j];
  }
  delete [] areas;
  delete [] regions;
  delete [] centroid;
  delete [] col;
  delete [] data_pts;
  delete [] dt;
  delete [] frame_size;
  delete [] max_area;
  delete [] min_area;
  delete [] max_area_index;
  delete [] min_area_index;
  delete [] max_radius;
  delete [] min_radius;
  delete [] n_valid_pts;
  delete [] num_regions;
  delete [] row;
  delete [] pts; 
}

float voronoi::get_area( int cur_frame, int reg_n ){

	if( reg_n == -1 || reg_n > n_valid_pts[ cur_frame ]){  // not a valid reg_n because not a valid data point
	  return -1;
	}

	return areas[ cur_frame ][ reg_n ];
}  // end method get_area

int voronoi::expand_int_array( int* &array, int desired_size, int& array_max )

  //  This routine doubles the size of the given 2D point list
  //
  //  pt:      Point list to double
  //  pt_max:  New maximum list size
{
  int      i;				// Loop counter
  int *tmp_array;			// Temporary point list


  tmp_array = new int[ desired_size ];
  if ( !tmp_array ) {
    cerr << "voronoi::expand_int_array(), out of memory creating new point list\n";
    return 0;
  }

  for( i = 0; i < array_max; i++ ) {	// Copy list
    tmp_array[ i ] = array[ i ];
  }

  delete [] array;				// Reference new list
  array = tmp_array;

  array_max = desired_size;				// Double list size
  return 1;
}					// End routine expand_int_array

int voronoi::expand_ptset( Point* &pts, int& pt_max )

  //  This routine doubles the size of the given 2D point list
  //
  //  pt:      Point list to double
  //  pt_max:  New maximum list size
{
  int      i;				// Loop counter
  Point *tmp_pt;			// Temporary point list


  tmp_pt = new Point[ pt_max * 2 ];
  if ( !tmp_pt ) {
    cerr << "expand_pt(), out of memory creating new point list\n";
    return 0;
  }

  for( i = 0; i < pt_max; i++ ) {	// Copy list
    tmp_pt[ i ] = pts[ i ];
  }

  delete [] pts;				// Reference new list
  pts = tmp_pt;

  pt_max *= 2;				// Double list size
  return 1;
}					// End routine expand_pt

void voronoi::find_local_area( int cur_frame, int reg_n, float& avg_area, float& max_area ){
  // This routine finds the average area of the current region and its neighbors.
  // It also finds the max area of all of these.
  float count = 1;
  float cur_area;
  
  int card = get_cardinality( cur_frame, reg_n );
  int* neigh = new int [ card ];  
  
  find_neighbor_regions( cur_frame, reg_n, card, neigh );
  
  avg_area = get_area(cur_frame, reg_n);
  max_area = avg_area;
  
  for( int i = 0; i < card; i ++ ){
    if( neigh[i] > - 1 ){
      cur_area = get_area( cur_frame, neigh[i] );
      if ( cur_area > 0 ){   // Don't include invalid regions. Invalid regions have "area" of -1.
        avg_area = avg_area + cur_area;
        if ( cur_area > max_area ){
          max_area = cur_area;
        }
        count++;
      }
    }
  }

  avg_area = avg_area/count;
}
void voronoi::find_neighbor_regions( int cur_frame, int reg_n, int card, int* neigh_regs  ){
  // Finds a point in each region neighboring reg_n and uses this to determine
  // gather a list of neighboring region numbers, placed in neigh_regs

  // neigh_regs:  Array of size card, allocated by caller
  
  int i;
  Point center;
  float dx, dy;
  float m_inv, b;
  float m_inv_abs;
  Point pt1, pt2, midpt;
 
  
  Point* neigh = new Point [ card ];

  for ( i = 0; i < card; i++ ){
    pt1 = get_poly_pt( cur_frame, reg_n, i );  // Get the cth polygon point.
    if ( i + 1 < card ){
      pt2 = get_poly_pt( cur_frame, reg_n, i+1 );  // Get the cth polygon point.
    }else{
      pt2 = get_poly_pt( cur_frame, reg_n, 0 );  // Get the cth polygon point.
    }
    
    midpt = comp_midpt( pt1, pt2 );
    center = get_centroid_coor( cur_frame, reg_n );
    comp_line( center, midpt, m_inv, b );
    if ( m_inv == 0 ) {// handle vertical and horizontal lines

      if (center.x == midpt.x){ //vertical line
        neigh[i].x = center.x;
        neigh[i].y = ( center.y < midpt.y ) ? midpt.y + min_radius[cur_frame] : midpt.y - min_radius[cur_frame];
      
      }else{         // horizontal line
        neigh[i].y = center.y;
        neigh[i].x = ( center.x < midpt.x ) ? midpt.x + min_radius[cur_frame] : midpt.x - min_radius[cur_frame];
      }
    
    }else{ // slanted line
      m_inv_abs = fabs(m_inv);
      if( m_inv_abs > fabs(1/m_inv) ){
        dx = 0.5*min_radius[cur_frame];
        dy = dx/m_inv_abs;
      }else{
        dy = 0.5*min_radius[cur_frame];
        dx = dy*m_inv_abs;
      }
      
      if ( m_inv > 0 ){ 
        if ( center.y < midpt.y ){
          neigh[i].x = midpt.x +dx;
          neigh[i].y = midpt.y +dy;
        }else{
          neigh[i].x = midpt.x -dx;
          neigh[i].y = midpt.y -dy;
        }
      }else{
        if ( center.y < midpt.y ){
          neigh[i].x = midpt.x -dx;
          neigh[i].y = midpt.y +dy;
        }else{
          neigh[i].x = midpt.x +dx;
          neigh[i].y = midpt.y -dy;
        }
      }
    }
  }// end cardinality loop
  
  for ( i = 0; i < card; i++ ){
    neigh_regs[ i ] = find_centroid( cur_frame, neigh[i].x, neigh[i].y );  
  } 

 
  delete [] neigh;
}

void voronoi::get_area_bounds( float* &min_a, float* &max_a ){
  min_a = min_area;
  max_a = max_area;
	return;
}  // end method get_area_bounds


int voronoi::get_cardinality( int cur_frame, int reg_n ){
	if( reg_n == -1 ){  // not a valid reg_n because not a valid data point
    cout<<"In voronoi::get_cardinality(int cur_frame, int reg_n ), reg_n = -1"<<endl;
		return -1;
	} 
	return regions[cur_frame][reg_n].card;
}  // end method get_cardinality

Point voronoi::get_centroid_coor( int cur_frame, int position ){
  	Point center;
    center.x = -1;  
    center.y = -1;
    if ( position >= frame_size[ cur_frame ] || position < 0 ){
		cerr<<"In get_centroid_coor, voronoi.cpp, invalid position requested: "<< position<<endl;
		return center;  // flag caller that points are invalid
	}
  center = data_pts[cur_frame][position];
  return center;
}    // end method get_centroid_coor

int voronoi::get_centroid_ind( int cur_frame, int position ){
	if ( position >= frame_size[ cur_frame ] || position < 0 ){
		cerr<<"In get_centroid_ind, voronoi.cpp, invalid position requested: "<< position<<endl;
		return -1;
	}
	return centroid[ cur_frame ][ position ];
}   // end method get_centroid_ind


Point voronoi::get_data_pt( int cur_frame, int pt_n ){
   Point pt;
   pt.x = data_pts[cur_frame][pt_n].x;
   pt.y = data_pts[cur_frame][pt_n].y;
   pt.z = 0;
	 return pt;
}  // end method get_data_pts


int* voronoi::get_frame_size(){
	return frame_size;
}  // end method get_frame_size


int* voronoi::get_num_cols( ){
	if (!col){
		cerr<<"Error in voronoi.cpp: init_voronoi must be called before get_num_cols"<<endl;
		cerr<<"col is null"<<endl;
	}
	return col;
}  // end method get_num_cols

int voronoi::get_num_regions( int cur_frame ){
	if (!num_regions){
		cerr<<"Error in voronoi.cpp: init_voronoi must be called before get_num_regions(int cur_frame)"<<endl;
		cerr<<"Num_regions is null"<<endl;
    return 0;
	}
  if ( cur_frame < 0 || cur_frame >= n_display_fr ){
    cerr<<"Error in voronoi.cpp: get_num_regions called with invalid frame number: "<<cur_frame<<endl;
    return 0;
  }
  return num_regions[ cur_frame ];
}

/*int* voronoi::get_num_regions(){
	if (!num_regions){
		cerr<<"Error in voronoi.cpp: init_voronoi must be called before get_num_regions"<<endl;
		cerr<<"Num_regions is null"<<endl;
    return num_regions;
	}
  return num_regions;
}  // end method get_num_regions
*/
void voronoi::get_num_regions( int* num_rgs, int size ){
	if (!num_regions){
		cerr<<"Error in voronoi.cpp: init_voronoi must be called before get_num_regions"<<endl;
		cerr<<"num_regions is null"<<endl;
	}

  for( int i = 0; i < size; i++ ){
    num_rgs[ i ] = num_regions[ i ];
  }
}  // end method get_num_regions

int* voronoi::get_num_rows(){
	if (!row){
		cerr<<"Error in voronoi.cpp: init_voronoi must be called before get_num_rows"<<endl;
		cerr<<"row is null"<<endl;
	}
	return row;
}  // end method get_num_rows


void voronoi::get_num_valid_pts( int* num_v_pts, int size ){
	if (!n_valid_pts){
		cerr<<"Error in voronoi.cpp: init_voronoi must be called before get_num_valid_pts"<<endl;
		cerr<<"n_valid_pts is null"<<endl;
	}

  for( int i = 0; i < size; i++ ){
    num_v_pts[ i ] = n_valid_pts[ i ];
  }
}  // end method get_num_valid_pts

int voronoi::get_num_vv(int cur_frame){
  return num_vv;
}

//float* get_poly_pt( int cur_frame, int reg_n, int card_n){
//	float* tmp = new float [3];
//	tmp[0] = pts[cur_frame][ regions[cur_frame][reg_n].ppts[card_n] ].x;
//	tmp[1] = pts[cur_frame][ regions[cur_frame][reg_n].ppts[card_n] ].y;
//	tmp[2] = 0;
//	return tmp;
//}  // end method get_poly_pt

Point voronoi::get_poly_pt( int cur_frame, int reg_n, int card_n){
	Point tmp;
  tmp.x = pts[cur_frame][ regions[cur_frame][reg_n].ppts[card_n] ].x;
  tmp.y = pts[cur_frame][ regions[cur_frame][reg_n].ppts[card_n] ].y;
  tmp.z = 0;
  tmp.mark_flag = pts[cur_frame][ regions[cur_frame][reg_n].ppts[card_n] ].mark_flag;
  tmp.ID = pts[cur_frame][ regions[cur_frame][reg_n].ppts[card_n] ].ID;

	return tmp;
}  // end method get_poly_pt

Point voronoi::get_vv( int cur_frame, int index ){
  Point dummy;
  if( index >= num_vv || index < 0){
    dummy.x = -1;
    dummy.y = -1;
    cerr<<"In voronoi::get_vv. Index out of bounds."<<endl;
    return dummy;
  }
  return pts[cur_frame][ index ];
}

int voronoi::get_vv_index( int cur_frame, int reg_n, int card_n ){
  return regions[cur_frame][reg_n].ppts[card_n];
}

void voronoi::get_poly_pt_neighs( int cur_frame, int reg_n, int pt_index, Point vv_neigh [ 2 ] ){
  // Returns the Voronoi vertex adjacent to the Voronoi vertex at pt_index on region reg_n
  // vv_neigh:  the neighbor points to be found
  int card  = get_cardinality( cur_frame, reg_n);
  int index1; int index2;
  index1 = (pt_index + 1)%card;
  if ( pt_index > 0){
    index2 = pt_index - 1;
  }else{
    index2 = card - 1;
  }

  vv_neigh[0] = get_poly_pt( cur_frame, reg_n, index1 );
  vv_neigh[1] = get_poly_pt( cur_frame, reg_n, index2 );
}

int voronoi::get_poly_pt_index( int cur_frame, int reg_n, Point the_pt ){
  // Returns the Voronoi vertex polygon index of the requested point; if not found, returns -1
  int card = get_cardinality( cur_frame, reg_n );
  Point cur_pt;
  for ( int i = 0; i < card; i++ ){
    cur_pt = get_poly_pt( cur_frame, reg_n, i );
    if( the_pt == cur_pt){
      return i;
    }
  }
  return -1;  // if not found return -1
}

void voronoi::get_basename(string start_name, string* base_name){
  // This routine finds the root of a file name (anything that comes before the specification of pc or unix).
  // 
  // string * base_name is the address of the string we want to change
  int i;
  string array [ 10 ];
  start_name.token( array, 10, ".");
  string stbuff;
  start_name = array[0] + string(".");
  i = 0;
  while( stbuff != "pc." && stbuff!= "unix." ){
    if ( i != 0 ) {
      start_name = start_name + stbuff;
    }
    i++;
	stbuff = array[i]+string(".");
  }
  *base_name = start_name; // * dereferences the pointer so we can use the string assigment operation
                           // In other words, it allows you to assign start_name to the string located at the address given by base_name
}

void voronoi::init_voronoi( char* bin_fname, int n_faces, int n_display_frames, bool FULL_SET ){
  int DEBUG = 0;
  int i, j; //counters
  int temp;

  string base_name;
  get_basename((string)bin_fname, &base_name );

  int num_attr;

  float* buff;
		
  char reg_fname [256];
  char vert_fname [256];
	
  n_display_fr = n_display_frames;
  
  dt = new DistanceTable* [ n_display_fr ];
  num_regions = new int [ n_display_fr ];
  n_valid_pts = new int [ n_display_fr ];
  regions = new poly* [ n_display_fr ];
  pts = new Point* [ n_display_fr ];
  data_pts = new Point* [ n_display_fr ];
  //delauney_ind = new int** [ n_display_fr ];
	
  areas = new float* [ n_display_fr ];
  min_area = new float [ n_display_fr ];
  max_area = new float [ n_display_fr ];
  max_area_index = new int [ n_display_fr ];
  min_area_index = new int [ n_display_fr ];
  max_radius = new float [ n_display_fr ];
  min_radius = new float [ n_display_fr ];

  // base_name = bin_fname + "bin";
  gridf.open( bin_fname );
  num_frames = gridf.frame_n();
  col = new int [ num_frames ];
  row = new int [ num_frames ];
  frame_size = new int [ num_frames ];
  gridf.rows( row );
  gridf.cols( col );
  centroid = new int* [ n_display_fr ];
  num_attr = gridf.attr_n();
  gridf.elem_n( frame_size );
  buff = new float [ num_attr ];
  int x = 0;
  int y = 0;

  for ( i = 0; i <  n_display_fr; i++ ){
    sprintf( reg_fname, "%sf%d.%d.vorout", (char*)base_name, i, n_faces );
    sprintf( vert_fname, "%sf%d.%d.vorin", (char*)base_name, i, n_faces );
    if (DEBUG = 1){
       cout<<"reg_fname: "<<reg_fname<<endl;
       cout<<"vert_fname: "<<vert_fname<<endl;
    }
    read_points(vert_fname, i, FULL_SET);
    read_regions(reg_fname, i);
    
    // read_delauney( "test.txt", i );
    find_areas(i);
		
    if (!FULL_SET){
      min_max_radius(min_radius[i], max_radius[i],i);
      cout<<"max_radius of "<<i<<" = "<<max_radius[i]<<endl;
      dt[i] = new DistanceTable(max_radius[ i ], num_regions[i], data_pts[ i ]);
      cout<<"distance table ready"<<endl;
    
      centroid[i] = new int [ frame_size[i]];
    
      for( j = 0;  j < frame_size[i]; j++ ){ 
        if (j > 0){
          temp = j%col[i];
          if ( temp == 0 ) {  //new row
            y++;
            x = 0;
		  }else {
             x++;
		  }
		}
        gridf.get( j, i, buff );
        if( gridf.allowable( buff ) ){
          centroid[ i ][ j ] = find_centroid( i, x, y );
        }else{
           centroid[ i ][ j ] = -1;
		}
	  }
      
      // The last 2 'elements' in each frame of a gridfile hold the col and row information.
	  if ( gridf.is_gridfile() ){
	    centroid[i][ frame_size[i] - 2 ] = -1;
	    centroid[i][ frame_size[i] - 1 ] = -1;
	  }
	}
  } // end for each frame
  //outf.close();
  cout<<"end init_voronoi"<<endl;
  delete [] buff;
  gridf.close();
} // end method init_voronoi

void voronoi::mark_pt( int cur_frame, int reg_n, int card_n, char the_mark){
  pts[cur_frame][ regions[cur_frame][reg_n].ppts[card_n] ].mark_flag = the_mark;
}
/*
void voronoi::read_delauney(char* filename, int cur_frame){
  // This method reads the qhull output that countains the delauney triangulation information 
	// and stores it for display.
 	// 
	// qhull delauney output format is a text file with:
	// # of delauney facets (triangles) on the first line
	// Each of the rest of the lines has:
  // A number of neighboring regions followed by the indices of that many neighbors.
  // The indices correspond to position in the list of input points.

  int i, j;
  int num_neigh;
  string buffer;
  string stbuffer[100];
	
	fp_v.open( filename, 'r');
	
	//Read the number of regions
	fp_v.getline( buffer );
	num_delauney_reg = (int)buffer;
	
  // Read the rest of the lines, initializing a pointer based on the first number on each line
	delauney_ind[cur_frame] = new int* [ num_delauney_reg ];
	for (i = 0; i < num_delauney_reg; i++){
    fp_v.getline(buffer);
	  buffer.token( stbuffer, 10, " ");
	  num_neigh = stbuffer[0];
    delauney_ind[ cur_frame ][ i ] = new int [ num_neigh + 1 ];
    
    // Put the number of neighbors in the first position in the array
    delauney_ind[ cur_frame ][ i ][ 0 ] = num_neigh;
    // Add the rest of the neighbors to the array
    for ( j = 1; j <= num_neigh; j++ ){
      delauney_ind[ cur_frame ][ i ][ j ] = stbuffer[ j ];
    }
  }
	fp_v.close();

}// end method read_delauney
*/

void voronoi::read_points(char* filename, int cur_frame, bool FULL_SET){
  // This method reads the qhull input that generated the Voronoi output 
	// read by read_regions and stores it for display.
 	// 
	// qhull input format is a text file with:
	// # of dimensions on the first line
	// # of points on the second line
	// A list of points (one per line). In our case, each is a data point.

  int i;
  int num_points;
  string buffer;
  string stbuffer[100];
	
   fp_v.open( filename, 'r');

   //Read the points from the voronoi input file
   fp_v.getline( buffer );
   buffer.token(stbuffer, 100, " ");
   n_valid_pts[cur_frame] = (int)stbuffer[1];  //stbuffer[0] is a '#' to indicate to qhull that it's a comment.

  num_points = (int)stbuffer[2];  //num points we want to use stored on this line now.
  num_regions[cur_frame] = num_points;
  
  fp_v.getline( buffer ); // eat dimension line
  fp_v.getline( buffer );  //Eat this, since it contains the artificial frame I put around that datasets; whereas

  if ( FULL_SET ){
    num_points = (int)buffer;  //num points we want to use stored on this line now.
    num_regions[cur_frame] = num_points;
  }

  // num_points now as the sum of valid and invalid bounds without this extra square.
  //# of points line  Note: This should be same as number of regions

	//num_points = atoi(buffer);
	data_pts[cur_frame] = new Point [ num_points ];
	for (i = 0; i < num_points; i++){
    fp_v.getline(buffer);
	  buffer.token( stbuffer, 10, " ");
	  data_pts[cur_frame][ i ].x = stbuffer[0];
		data_pts[cur_frame][ i ].y = stbuffer[1];
		data_pts[cur_frame][ i ].ID = i;
	}
	fp_v.close();

}// end method read_points


void voronoi::read_regions(char* filename, int cur_frame){
  // This method reads the qhull Voronoi ouput and stores it for display
 	// 
	// qhull Voronoi output format is a text file with:
	// # of dimensions on the first line
	// # of points; num. of regions; on the second line (separated by space);
	// A list of points (one per line) that are vertices of polygon faces
	// A list of faces (one face per line) as indices into the vertex list
	
	int i; int j;
    string buffer;
    int bufsz = 150;
    string stbuffer[150];
  
  

  fp_v.open( filename, 'r'); 
	
  fp_v.getline( buffer );// eat number of dimensions
  fp_v.getline( buffer ); // this row has # of points, # of regions
  buffer.token(stbuffer,bufsz," ");
  num_vv = (int)stbuffer[0];  // global variable to keep a count of the number of Voronoi vertices

 // num_regions[cur_frame] = (int)stbuffer[1]; 

  //Read voronoi polygonal region vertices from voronoi output
  pts[cur_frame] = new Point [num_vv];
  for ( i = 0; i < num_vv; i++ ){
    fp_v.getline(buffer);
    buffer.token(stbuffer, bufsz, " ");

    pts[cur_frame][i].x = (float)stbuffer[0];
    pts[cur_frame][i].y = (float)stbuffer[1];

  }

  regions[cur_frame] = new poly [ num_regions[cur_frame] ];
  for ( i = 0; i < num_regions[cur_frame]; i++ ){
    fp_v.getline(buffer);
    buffer.token(stbuffer, bufsz, " ");
    regions[cur_frame][i].card = (int)stbuffer[0];
    regions[cur_frame][i].ppts = new int [ regions[cur_frame][i].card ];
    for ( j = 0; j < regions[cur_frame][i].card; j++){
      regions[cur_frame][i].ppts[j] = stbuffer[ j+1 ];
    }
  }
  fp_v.close();
} // end method read_regions

int voronoi::update_polygon( int cur_frame, int reg_n, int new_card, int ppt_indices [ ], int new_pts_index ){
  //This routine updates a Voronoi region after a new point has been added and the convex hull recalculated.  It is 
  // therefore assumed that the new cardinality <= the original cardinality.  Returns 1 if successful; 0 otherwise.
  // cur_frame: current frame
  // reg_n: region number
  // new_card: new cardinality
  // int ppt_indices[]: indices into ppt array of points on convex hull (except one is an additional point
  // int new_pts_index: new point was added to pts (list of poly points) at this position in the list
  
  int i; // counter
  int ppts_index; // index into ppts array
  
  int cur_size = regions[cur_frame][reg_n].card;
  if ( new_card > cur_size ){
    expand_int_array( regions[cur_frame][reg_n].ppts, new_card, cur_size );
    //cerr<<"In voronoi::update_polygon.  Region cardinality increased."<<endl;
  }

  int* temp_array = new int [ new_card ];
  
  for ( i = 0; i < new_card; i++ ){
    ppts_index = ppt_indices[ i ];
    if ( ppts_index == regions[cur_frame][reg_n].card ){
      temp_array[ i ] = new_pts_index;
    }else{
      temp_array[ i ] = regions[cur_frame][reg_n].ppts[ ppts_index ];
    }
  }

  regions[cur_frame][reg_n].card = new_card;
  for ( i = 0; i < new_card; i++ ){
    regions[cur_frame][reg_n].ppts[ i ] = temp_array[ i ];
  }

  delete [ ] temp_array;
  return 1;
}
