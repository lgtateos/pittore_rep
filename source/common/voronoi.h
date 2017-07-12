// VORONOI.H
//    Prototypes for VORONOI.C

#ifndef _voronoi_h
#define _voronoi_h

#ifdef WIN32
#include <windows.h>
#include<glut.h>
#endif

#ifdef SOLARIS
#include <GL/glut.h>
#endif

#include <GL/glu.h>
#include <GL/gl.h>

#include <stdlib.h>
#include <stdio.h>

#include "colour.h"
#include "distancetable.h"
#include "filereader.h"
#include "grid_file.h"
#include "point.h"  // Brent's point class.  Must put it's include directory first, so it doesn't see Chris's point.h
#include "str.h"


struct poly{
  int card;
  int* ppts;
};

class voronoi {
private:
  float** areas;     //  Voronoi region areas.
  int* col; int* row;
  int** centroid;
  Point** data_pts;  //  Mesh simplified data points.
  int*** delauney_ind;
  DistanceTable** dt;
  filereader fp_v;
  int* frame_size;
  grid_file gridf;
  float* min_area; float* max_area;
  int* max_area_index;
  int* min_area_index;
  int n_display_fr;  //  Number of frames to display (for debugging purposes can be less than num_frames).
  int num_frames;
  int* num_regions; 
  int num_vv;
  int* n_valid_pts;  //  Voronoi files plugged with extra points to restrict Voronoi region areas on outer boundaries.
  Point** pts;       //  Voronoi region (polygon) vertices.
  poly** regions;    //  Voronoi regions cardinality followed by indices of vertex points.

  void comp_line( Point s, Point t, float& m_inv, float& b );
  Point comp_midpt( Point s, Point t );
  int voronoi::expand_int_array( int* &array, int desired_size, int& array_max );
  int expand_ptset( Point* &pts, int& pt_max );
  void find_areas( int cur_frame );
  void min_max_radius( float& min, float& max, int );
  void read_delauney(char*, int);
  void read_regions(char*, int);
  void read_points(char*, int, bool);

public:
  float* max_radius;
  float* min_radius;
  int num_delauney_reg;
  voronoi();
  voronoi( char* bin_fname, int n_faces, int n_display_frames, bool FULL_SET  );
  ~voronoi();
  int add_to_ptarray(  Point pt, int cur_frame, int& ptarray_max);//, Point* &ptarray );
  void draw_center(int pt_n, int pixel_w, int pixel_h, int cur_frame, float sphere_radius );
  void draw_delauney( int cur_frame, int pixel_w, int pixel_h );
  void draw_regs( int reg_n, int pixel_w, int pixel_h, int cur_frame, float r, float g, float b, GLenum mode );
  void draw_stars( int pixel_w, int pixel_h, int cur_frame, int max_x, int max_y );
  void draw_web( int reg_n, int pixel_w, int pixel_h, int cur_frame, float r, float g, float b );
  Point find_com( int cur_frame, int reg_n );
  int find_centroid( int cur_frame, float x, float y  ); // Uses a distance table to locate the centroid point closest to the given point.
  int find_centroid( int cur_frame, Point my_pnt ); // Use when no distance table has been initialized.
  double find_datapt_dist_sq ( int cur_frame, Point pt1, int pt_n2 );
  void find_local_area( int cur_frame, int reg_n, float& avg_area, float& max_area );
  double find_polypt_dist_sq ( int cur_frame, int reg_n, int card_n1, int card_n2 );
  inline double find_dist_sq ( Point tmp1, Point tmp2 );
  float find_near_reg( int frame, Point pt, int& reg); // Only looks in valid regions
  void find_near_regs( int frame, Point pt, int& vreg, float& vdist, int& invreg, float& invdist); // Finds nearest valid and invalid regions
  void find_neighbor_regions( int cur_frame, int reg_n, int card, int* neigh_regs  );
  void free_voronoi();
  float get_area( int cur_frame, int reg_n );
  void get_area_bounds( float* &min_a, float* &max_a );
  void get_basename(string start_name, string *base_name);
  int get_cardinality(  int cur_frame, int reg_n );
  Point get_centroid_coor( int cur_frame, int position );
  int get_centroid_ind( int cur_frame, int position );// use when point coordinates of centroid are known. Looks up the id of the point.
  Point get_data_pt( int cur_frame, int pt_n );
  int* get_frame_size();
  int find_mixed_reg_index( int cur_frame, int x, int y, int region_id, float* valuey, int** reg_a);
  int* get_num_cols();
  int* get_num_rows();
  int get_num_regions( int cur_frame );
  void get_num_regions( int* num_rgs, int size );//int* get_num_regions();
  void get_num_valid_pts( int* num_v_pts, int size );
  int get_num_vv(int cur_frame);
  //float* get_poly_pt( int, int, int);
  Point get_poly_pt( int cur_frame, int reg_n, int card_n );
  void get_poly_pt_neighs( int cur_frame, int reg_n, int pt_index, Point vv_neighs [ 2 ] );
  int get_poly_pt_index( int cur_frame, int reg_n, Point the_pt );
  Point get_vv( int cur_frame, int index );
  int get_vv_index( int cur_frame, int reg_n, int card_n ); // Added for printing out the regions.
  int region_report( int reg_n );
  void init_voronoi( char* bin_fname, int n_faces, int n_display_frames, bool FULL_SET ); 
  void mark_pt(int cur_frame, int reg_n, int card_n, char the_mark);
  int update_polygon( int cur_frame, int reg_n, int new_card, int ppt_indices [ ], int new_pts_index );

};


#endif