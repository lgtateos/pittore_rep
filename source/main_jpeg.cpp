#include "global.h"
#ifdef NAMESPACE
  #include <iostream>
#else
  #include <iostream.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <tcl.h>
#include <tk.h>

#ifdef WIN32
#include <windows.h>
#include<glut.h>
#endif

#ifdef SOLARIS
#include <GL/glut.h>
#endif

#include <GL/glu.h>
#include <GL/gl.h>

#include "bin_file.h"
#include "colour.h"e
#include "grid_file.h"
#include "highlight.h"
#include "jpeg.h"
#include "gl_debug.h"
#include "global.h"
#include "bumpmap.h"
#include "lighting.h"
#include "paint.h"
#include "random.h"
#include "reporter.h"
#include "segment.h"
#include "stroke.h"
#include "tile.h"
#include "timing.h"
#include "voronoi.h"

#include "distancetable.h"
#include "point.h"

#include "togl.h"
#include "vmap.h"

/*
#define  PIX_W 8
#define  PIX_H 8
*/
#ifndef ROUND
  #define ROUND(x) ( x > 0 ? (int)(x+.5) : (int)(x - .5) )  
#endif

#ifndef MAX
  #define MAX(x,y) ( x > y ? x : y )
#endif

# define ASPECT_RATIO 1 //1.28
/*--------------------------------------------------------------------------*/
/*  MAIN.CPP								                                */
/*    Routines to display visualization using segmention determine	        */
/*    coverage. 							                                */
/*									                                        */
/*- Modification History: --------------------------------------------------*/
/*  When:	Who:			Comments:			                            */
/*									                                        */
/*  19-Jun-01	Christopher G. Healey	Version given to Laura Tateosian    */
/*  30-Jun-01	Laura G. Tateosian	Underpainting added		                */
/*  Fall 2001   Laura G. Tateosian  Underpainting and highlights removed    */  
/*  08-Oct-01   Laura G. Tateosian	Manual togle between frames added       */
/*  12-Nov-01   Laura G. Tateosian	Underpainting and highlights readded    */
/*  16-Jul-02  	Laura G. Tateosian	Added keyboard UI		                */
/*  24-Jun-03   Laura G. Tateosian	Changed strokes to curved surfaces      */
/*  3-Jul-03    Laura G. Tateosian	Changed UI to tcl/tk                    */
/*--------------------------------------------------------------------------*/

//  Sample input: "data/weather.southamerica.pc.bin" 12 500 .3 0   0 4 6   0 

//  Module global prototypes
static void build_id( seg_s& );
static void build_cid();
void update_low_bb( float x, float y );
void choose_display_status(void);
static int choose_texture(int, int);
static void comp_coverage( int, int, float& );
void comp_highlight_size( stroke_s& );
static int comp_shape( stroke_s&, float );
static void comp_size( stroke_s&, float );
static void display_shell(GLenum mode);
static void display_obj();
void draw_color_attr( int );
static void draw_manip( void );
static void draw_manip_old( void );
static void draw_curves( void );
//static float draw_low_detail();
static void draw_mask( float );
static void draw_matte_regions();
static void draw_segs();
static void draw_sized_regions();
static void draw_complex( char mode );
static void draw_temp();
static void draw_test();
static void draw_trilevel();
void get_jtransformation( int my_view);
inline int set_orientation( Point pt, float& orientation );
inline int set_orientation( float midrot, Point pt, float& orientation );
static void pick( float x, float y );
static void process_hits( int hits, GLuint buffer [] );
static void voronoi_segment();
static int get_current_position( int, int, int, int );
static void get_region_bbox( int region_id, float ll[], float ur[] );
float get_value( int, float [] );
static int color_canvas(Point corner[], float red[], float green[], float blue[]);
static int indication_setup();
static void init_stroke_lists();
void init_maps();
void init_stuff(void);
static int interpolate( float, float, float * );
static int invalid_corner( int region_id, float* valu );
static int is_boundary ( int y, int x, int size, int region_id, float* val);
static int is_valid(int, int, float, float, float*);
int map_color( float x, float y, float color [ 3 ] );
int map_mat( int, int, stroke_s&);
static int new_frame_setup();
static void paint_borders(int);
static void paint_matte();
static void paint_matte( int, int );
void position_camera();
void print_matrix( int);
static int region_invalid( int region_id, float ll[], float ur[], float* valu );
static int region_mixed( int region_id, float ll[], float ur[], float* valu );
static int set_err( seg_s&, float );
static void set_variegated_color( stroke_s& stroke, float x, float y, float gfact, float gadd, float factor, float addend );
void setup_mapping( int, bool );
static int sort_split_regions( int, int, int, float*);
int** region_arr;
int update = 0;
int vor_var = 0;

// vars specific to jpeg closeup captures
int num; int numc, numr;
int view = -1;
int previous_view = -1;
float jtrans[2] = { 0 , 0};
float jsc[2] = {1, 1};

//Global functions for tcl/tk UI
void         idle(struct Togl *togl);
static int   capture_jpg_cb( Togl*, int, char** );
static int   dump_cb( Togl *, int, char ** );
static int   frame_cb( Togl *, int, char **);
static int   frame_change_cb( Togl *, int, char **);
static int   mapping_cb( Togl *, int, char **);
static int   pick_cb( Togl *, int, char ** );
static int   rot_cb( Togl *, int, char ** );
static int   stepup_cb( Togl *, int, char**);
static int   trans_cb( Togl *, int, char ** );
static int   zoom_cb( Togl *, int, char ** );

int     init_tcl( Tcl_Interp *);
void    togl_create_cb( Togl *);
void    togl_display_cb( Togl *);
void    togl_reshape_cb( Togl *);

//  Module global variables

#ifdef _MEM_DEBUG
//dyn_hash mem_hash_table; //for memory tracking 
#endif
ofstream outfile;       // for debugging only
int col_attr; int siz_attr; int ori_attr; int gre_attr; int con_attr; int pro_attr;
bool complex;
int count = 0;
int jpeg_count = 0;
int temp_count = 0;
float** avg_size;       //  Average stroke size in a segment
stroke_s **back_st;         // Backwash stroke list
int       *back_st_n;       // Backwash stroke number
bool BEZIER_ON = true;
int cur_seg = -1;
float clearcolor[ 3 ];
int        DEBUG = 0;
int fra; // Current frame
bool  file_open = false;      
int		   frame_change_flag;	// Init as 0; Change to 1 when frame change has just occured
int        frames;      // Number of frames of data
int fwidth; int fheight;
grid_file gf;           // grid_file to access grid formatted data
double hfactor;		// Multiplicative factor controlling highlight sensitivity and coverage
int hfactor_flag;
int* hfactor_frame_flag;
stroke_s **high_st;         // Highlight stroke list
int       *high_st_n;       // Number of strokes in the backwash
bool is_iouca;
bool is_jpeg;
int      **id;          // Expanded segment ID array
int      **cid;
bumpmap bm;
float** lowleft; float** upright;
int largest;
int LIGHTING_ON = 0;
float     *amax;         // Maximum attribute values
char mode;
float     *amin;         // Minimum attribute values
bool mucha;
string*    names;       // Attribute names  
int        num_faces;
int        num_col = -1;     // Color is the "num_col"'th feature name in attr_map.cpp
int        num_con = -1;     // Contrast is the "num_con"'th feature name in attr_map.cpp
int        num_cov = -1;     // Coverage is the "num_cov"'th feature name in attr_map.cpp
int        num_gre = -1;     // Greyscale is the "num_gre"'th feature name in attr_map.cpp
int        num_hig = -1;     // Highlight is the "num_hig"'th feature name in attr_map.cpp
int        num_ori = -1;     // Orientation is the "num_ori"'th feature name in attr_map.cpp
int        num_pro = -1;     // Proportion is the "num_pro"'th feature name in attr_map.cpp
int        num_siz = -1;     // Size is the "num_siz"'th feature name in attr_map.cpp
int        num_sha = -1;     // 
int        num_fr_to_display;
int        num_to_draw = 20;
int*       num_valid_pts;
int        seg_to_draw = 10;
float sMall = 0;//2*.75 + 1; 
int PIX_W;
int PIX_H;
int reset = 0;
ofstream   outty;
seg_s      s;           // Segment structure
int        segmentation_type; //Data property to segment on.  0 uses the first 4.  Positive ints give the attribute number.
	                          //        -1 indications segmentation on voronoi region size.
int show_corner_strokes = 1;
int show_voronoi;
stroke_s **st;          // Stroke list (struct found in common/paint.h)
int       *st_max;      // Maximum size of stroke list
int       *st_n = NULL;        // Number of strokes in list
int sty_canv;

int sty_div; int cnt_div;
int sty_reg; int cnt_reg;
int sty_med; int cnt_med;
int sty_low; int cnt_low;
int sty_lowlow; int cnt_lowlow;
int test = 0;
float threshold;
float* v;           // Temporary attr value array
voronoi* vor;
int wide_count = 0; int long_count = 0;
int ct = 0; int vr = 0; int ir = 0;
attr_map * orig_features;

bool   newmapping = false;
int** map_matrix;
//Global constants and variables for tcl/tk UI
#define Y_FOVY 		60.0  // gluLookAt camera z position depends on this
#define	EYE_Z_POS	.7025902 //.5773503;  // tan of 0.5*Y_FOVY

typedef enum {
  X_AXIS, Y_AXIS, Z_AXIS
} axis;
axis cur_axis = Z_AXIS;

float translate[ 3 ] = { 0, 0, 0};
float rotate [ 3 ] = { 0, 0, 0};
float scale [ 3 ] = { 1,1,1 };

char * oldfile;
char * newfile;

typedef enum {
  NO_DATAFILE, NEW_DATA, NEW_MAPPING, NEW_FRAME, NEW_HFACTOR, REDRAW
} status;    

status display_status = NO_DATAFILE;

typedef enum {
  ON, OFF
} toggle;
toggle brush_texture = ON;

// *********tcl/tk methods************************

int init_tcl(Tcl_Interp *tcl)
{
  char cmd[ 256 ];
  int  i;
int  num_tcl_files;
  int  rc = TCL_OK;


  char src[][ 128 ] = {
		"npv.tcl",	
		
		"vmap.tcl",
		"highlight.tcl"
	};
  char arr[1000];
	sprintf(arr, "Binary file: %s   num_faces = %d", newfile, num_faces);
  printf("%s\n", arr);
		/*  "npv.tcl",
    "vmap.tcl",
    "highlight.tcl"
  };
  num_tcl_files = 3;
  */
		num_tcl_files = 3;
  // Initialize Tcl_Interpreter with TCL, TK, and TOGL
  if ( Tcl_Init( tcl ) == TCL_ERROR || Tk_Init( tcl ) == TCL_ERROR) {
    return TCL_ERROR;
  }

  if ( Togl_Init( tcl ) == TCL_ERROR ) {
    return TCL_ERROR;
  }

  oldfile = Tcl_Alloc( 1024 );
  
  strcpy (oldfile, "" );
  //strcpy (newfile, "" );

  Tcl_LinkVar( tcl, "filename", (char *) &newfile, TCL_LINK_STRING );

  // Register standard TOGL call-back functions
  Togl_CreateFunc( togl_create_cb );
  Togl_DisplayFunc( togl_display_cb );
  Togl_ReshapeFunc( togl_reshape_cb );
  Togl_TimerFunc(idle);

  //  Register application-specific TOGL commands
  Togl_CreateCommand( "capture", capture_jpg_cb ); 
  Togl_CreateCommand( "dump", dump_cb);
  Togl_CreateCommand( "invoke_mapping_dialog", mapping_cb );
  Togl_CreateCommand( "frame_change", frame_change_cb );
  Togl_CreateCommand( "pick", pick_cb );
  Togl_CreateCommand( "rotate", rot_cb );
  Togl_CreateCommand( "stepup", stepup_cb );
  Togl_CreateCommand( "translate", trans_cb );
  Togl_CreateCommand( "zoom", zoom_cb );

  Tcl_LinkVar(tcl, "cur_axis_tcl", (char *) &cur_axis, TCL_LINK_INT);
  Tcl_LinkVar(tcl, "lighting_on_tcl", (char *) &LIGHTING_ON, TCL_LINK_INT);
  Tcl_LinkVar(tcl, "show_corner_strokes_tcl", (char *) &show_corner_strokes, TCL_LINK_INT);
  Tcl_LinkVar(tcl, "show_voronoi_tcl", (char *) &show_voronoi, TCL_LINK_INT);
  Tcl_LinkVar(tcl, "hfactor_tcl", (char *) &hfactor, TCL_LINK_DOUBLE);
	Tcl_LinkVar(tcl, "hfactor_flag_tcl", (char *) &hfactor_flag, TCL_LINK_INT);
  Tcl_LinkVar(tcl, "frame_num_tcl", (char *) &fra, TCL_LINK_INT);
  Tcl_LinkVar(tcl, "reset_tcl", (char *) &reset, TCL_LINK_INT);
  Tcl_LinkVar(tcl, "update_tcl", (char *) &update, TCL_LINK_INT);
  Tcl_LinkVar(tcl, "view_tcl", (char *) &view, TCL_LINK_INT);

  //  Source each tcl source file through the tcl interpreter
	
	for ( i = 0; i < num_tcl_files; i++) {
      sprintf(cmd, "source \"%s\"", src[i]);
      rc |= Tcl_Eval( tcl, cmd );
  }   
	
  if( DEBUG )
    outfile.open("data.txt");
  
  if ( ( rc & TCL_ERROR ) != 0 ) {
	return TCL_ERROR;
  }
  cout<<"Press left mouse button to translate"<<endl;
  cout<<"Press Shift+left mouse button to zoom "<<endl;
  cout<<"Press right mouse button to rotate" <<endl;
  
  hfactor = .3;	//  Maximum highlight coverage value; may be adjusted  
  Tcl_Eval( tcl, "vmap_init" );  
  Tcl_Eval( tcl, "hfactor_init" );


  vmap_post_init( tcl );        
  return TCL_OK;
}


void idle(struct Togl *togl)
{
  if( update ){
   if( !reset && view != previous_view ){
    get_jtransformation( view );
   }
   if (show_voronoi == 1 ){
      vor_var++;
      vor_var = vor_var%2;  //  Alternate drawing web and filled polygon regions.
   }
   Togl_PostRedisplay(togl);
   update = 0;
  }
}

static int pick_cb( Togl *togl, int argc, char *argv[] ){
  int x; int y;
  pick( atoi(argv[2]), atoi(argv[3]) );
  Togl_SwapBuffers( togl );
  return TCL_OK;
}


void togl_create_cb( Togl *togl)
{

  init_stuff();
}
/*
		int n = 0;
    int maxns = 20;

		int h = 250; 
		int w = 250;
		float* segma = new float [ maxns ];
*/

void update_low_bb(float x, float y){

	if( x < lowleft[fra][0]){
		lowleft[fra][0] = x;
	}else if( x > upright[fra][0] ){
		upright[fra][0] = x;
	}
	if( y < lowleft[fra][1] ){
		lowleft[fra][1] = y;
	}else if ( y > upright[fra][1] ){
		upright[fra][1] = y;
	}
}

void draw_color_attr( int att){
  att = 0;
  int i, j;
  float r,g,b;
  float* vl = new float (s.attr_n);
  for( i= 0; i < s.h; i++ ){
    for( j = 0; j < s.w; j++ ){
      if( is_valid( j, i, 1,1, vl ) ){
        vl[att] = ( vl[ att ] - amin[ att ] ) / ( amax[ att ] - amin[ att ] );
        get_colour(vl[att], r,g,b);
       // r = 0; g = 1; b = 1;
        glPushMatrix();
        glTranslatef( j*PIX_W, i*PIX_H, 0 );
        glScalef(PIX_W, PIX_H, 1);
        
        draw_stroke_no_tex( r, g, b);
        glPopMatrix();
      }
    }

  }
  delete [] vl;
}

void togl_display_cb( Togl *togl)
{
  int       i; int j;                // Loop counter
  int offset = 0;
  attr_map feature;
  filereader fp;
  string base_name;
  int num_regs;
  #ifdef WIN32
  char* seg_filename = new char [512]; 
  #endif
  long t1, t2, td, dm;

  display_status = REDRAW;   // set as default status 

  choose_display_status();

  if (DEBUG) printf("Display_status = %d\n", display_status );

  switch ( display_status ) {
	  case NO_DATAFILE:
		  if (DEBUG) printf("Display status = NO_DATAFILe\n");
  		
		  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		  Togl_SwapBuffers( togl );
		  break;
	  case NEW_DATA:
  		t1 = usec_time();
		  if (DEBUG) printf("Display status = NEW_DATA\n");
      fra = 0;            // Initial setting to frame 0

	    if (!gf.open( newfile)) {
	      printf("In main.cpp: binfile failed to read file footer\n");
	    }
      file_open = true;

      //report(gf.attr_n(), gf.frame_n(), gf);
      // Create segment ID array
      alloc_seg_array( s, gf.cols(fra), gf.rows(fra) );

      //  Read number of attributes, number of frames, data field width and height
      // Set default values of s and s.attr_n = gf.attr_n(); s.w = gf.cols(fra); s.h = gf.rows(fra); 
      // s.min = 0.001;  s.num = 7;  s.den = 8;
		  init_seg( s, gf.cols(fra), gf.rows(fra), gf.attr_n(), 0.001, 7, 8); 
		  
      frames = gf.frame_n();
      v = new float [s.attr_n];
      // Allocate and initialize s.err based on the max and min of each attr over all frames.
      s.err = new float [ s.attr_n ];   // Set error to 20% of attr range
		  cout<<"Setting error now"<<endl;
		  if ( !set_err( s, 0.20) ) {
		    cerr<<"main.cpp: set_err failed"<<endl; 
		  }

		  names = new string[ s.attr_n ]; 
      lowleft = new float* [frames];
      upright = new float* [frames];
      hfactor_frame_flag = new int [frames];
		  for (i = 0; i < frames; i++ ){
			  hfactor_frame_flag[ i ] = 0;
        lowleft[ i ] = new float [ 2 ];
        upright[ i ] = new float [ 2 ];
		  }

		  orig_features = new attr_map[ s.attr_n ]; 
		  
      for ( i = 0; i < s.attr_n; i++ ) {
    	  names[ i ] = gf.attr_name( i ); 
        orig_features[i] = gf.attr_map_val(i);
			  if (DEBUG){
			    outfile<<"Name[ "<<i<<" ] = "<<names[ i ]<<endl;
    		  printf("orig_features(%d).name() = %s\n", i, (char *)orig_features[i].name() );
			  }
		  }

      map_matrix = new int*[ feature.num_feature() ];
		  for ( i = 0; i <  feature.num_feature(); i++ ) {
			  map_matrix[ i ] = new int[ s.attr_n ];
		  }

		  init_stroke_lists();
      
      num_valid_pts = new int [frames];
      if (num_fr_to_display > frames){
        num_fr_to_display  = frames;
      }
		  if( segmentation_type == -1 ){
        vor = new voronoi(); 
        vor->init_voronoi(newfile, num_faces, num_fr_to_display, false);
			  vor->get_num_valid_pts(num_valid_pts, num_fr_to_display);
		  }

      if (segmentation_type == -1){  
        region_arr = new int* [num_fr_to_display];
        lowleft[fra][0] = s.w*PIX_W;
        lowleft[fra][1] = s.h*PIX_H;
        upright[fra][0] = 0;
        upright[fra][1] = 0;

        vor->get_basename( newfile, &base_name );   // & gets the address of base_name
        sprintf( seg_filename, "%sf%d.%d.%.0f.seg.bin", (char*)base_name, fra, num_faces, threshold*100 );
        num_regs = vor->get_num_regions(fra);
        region_arr[fra] = new int [ num_regs ];
        

		if( !read_segment(s, region_arr[fra], num_regs, lowleft[fra], upright[fra], (string)seg_filename) ){
          cout<< "Begin voronoi_segment()"<<endl;
          voronoi_segment();
		  write_segment( s, region_arr[fra], num_regs, lowleft[fra], upright[fra], fra, (string)seg_filename );
          cout<< "End voronoi_segment()"<<endl;
        }
        t2 = usec_time();
        td = t2 - t1;
        td = td/1000000.0;
        dm = td/60.0;
        cout<<"Time for voronoi setup: "<<td<<" seconds (~"<<dm<<" minute(s))"<<endl;
      }else { 
        segment( offset, s, segmentation_type, gf,fra ); 
      }
      
      build_id( s );
		  setup_mapping( s.attr_n, true);
      
		  if (segmentation_type == -1 ){
         //cout<<"SEGMENT PAINTING SKIPPED FOR DEBUGGING"<<endl;
        if ( complex ){
          build_cid();
          st_max[fra] = 16;           // Initial arr size; usually grows
          st[fra] = new stroke_s[ st_max[fra] ];
          cout<<"Begin painting ..."<<endl;
          set_bezier( BEZIER_ON );
          temp_count = 0;
          wide_count = 0;
          long_count = 0;
          t1 = usec_time();
          st_n[fra] = stroke( cid, s.w * PIX_W, s.h * PIX_H, 0, st[fra],st_n[fra], st_max[fra], 1, segmentation_type);
          t2 = usec_time();
          cout<<"End painting. Curve count = "<<temp_count<<" wide_count = "<<wide_count<<" long_count = "<<long_count<<" for data dimenstion s.w = "<<s.w<< ", s.h = "<<s.h<<endl;
          cout<<"ct,vr,ir: "<<ct<<" "<<vr<<" "<<ir<<endl;
          td = t2 - t1;
          td = td/1000000.0;
          dm = td/60.0;
          cout<<"Time for stroke setup: "<<td<<" seconds (~ "<<dm<<" minute(s))"<<endl;
        }else{
             indication_setup();
        }
      }else{
		    // *****Create the stroke lists for the initial visit to a frame
		    if (!new_frame_setup()) {
  		    cerr << "New_frame_setup() of frame 0 in main unsuccessful."<<endl;
		    } else {
  		    cerr<<"New_frame_setup() was successful."<<endl;
		    } 

      }
	
		if (strcmp(newfile,"")!= 0 ) {
  		strcpy(oldfile, newfile);
		}
		break;   //end case NEW_DATA
		
	case NEW_MAPPING:
		if (DEBUG) printf("Display status = NEW_MAPPING\n");

		// delete [] back_st[fra];
		// delete [] high_st[fra];
		delete [] st[fra];
 
		st_n[fra] = 0;
		
		setup_mapping( s.attr_n, false);
    
		init_stroke_lists();           
/*
		//inseparable
		s.seg = new int *[ s.h ];         // Create segment ID array
		for ( i = 0; i < s.h; i++ ) {
			s.seg[ i ] = new int[ s.w ];
		}
		
    if (segmentation_type == -1){
      delete [] region_arr[fra];
      lowleft[fra][0] = s.w*PIX_W;
      lowleft[fra][1] = s.h*PIX_H;
      upright[fra][0] = 0;
      upright[fra][1] = 0;
      voronoi_segment();
      cout<< "ll= "<<lowleft[fra][0]<<", "<<lowleft[fra][1]<<"   ur= "<<upright[fra][0]<<", "<<upright[fra][1]<<endl;
      outty.close();
    }else {
      segment( offset, s, 0, gf,fra, vor ); 
    }

		//initialize id array which records coverage info. during painting  
		build_id( s ); 
*/  if(segmentation_type == -1 ){
      if ( complex ){
        build_cid();
        st_max[fra] = 16;           // Initial arr size; usually grows
        st[fra] = new stroke_s[ st_max[fra] ];
        cout<<"Begin painting ..."<<endl;
        set_bezier( BEZIER_ON );
        st_n[fra] = stroke( cid, s.w * PIX_W, s.h * PIX_H, 0, st[fra],st_n[fra], st_max[fra], 1, segmentation_type);
        cout<<"End painting."<<endl;
      }else{
        indication_setup();
      }
    }else{
		  // *****Create the stroke lists for the initial visit to a frame
		  if (!new_frame_setup()) {
    		  printf("New_frame_setup() of frame 0 in main unsuccessful.\n");
		  } else {
    		  printf("New_frame_setup() was successful.\n");
		  }
    }
		//end of inseparable
				
		break;  // end case NEW_MAPPING
		
	case NEW_HFACTOR: 
		if (DEBUG) printf("Display status = NEW_HFACTOR\n");
		printf("case NEW_HFACTOR: The new hfactor value = %f\n", hfactor);   ////something skrewy here!
	/*	
		if (!highlight(fra, hfactor, num_hig, s.attr_n, s.w, s.h, amin[ num_hig  ], amax[ num_hig],high_st[fra], &gf ))
		{ 
            printf("No highlight applied\n");	
		}
*/		break;
	case NEW_FRAME:
		if (DEBUG) printf("Display status = NEW_FRAME\n");
    //inseparable
	
    flush_seg( s, s.h, s.w );
    
    /*	s.seg = new int *[ s.h ];         // Create segment ID array
		for ( i = 0; i < s.h; i++ ) {
			s.seg[ i ] = new int[ s.w ];
		}
  */
		
    if (segmentation_type == -1){
      lowleft[fra][0] = s.w*PIX_W;
      lowleft[fra][1] = s.h*PIX_H;
      upright[fra][0] = 0;
      upright[fra][1] = 0;
      vor->get_basename( newfile, &base_name );   // & gets the address of base_name

      sprintf( seg_filename, "%sf%d.%d.%.0f.seg.bin", (char*)base_name, fra, num_faces, threshold*100 );
      num_regs = vor->get_num_regions(fra);
      region_arr[fra] = new int [ num_regs ];
	  if( !read_segment(s, region_arr[fra], num_regs, lowleft[fra], upright[fra], (string)seg_filename) ){
        cout<< "Begin voronoi_segment()"<<endl;
        voronoi_segment();
		write_segment( s, region_arr[fra], num_regs, lowleft[fra], upright[fra], fra, (string)seg_filename );
        cout<< "End voronoi_segment()"<<endl;
      }
      voronoi_segment();
      cout<< "ll= "<<lowleft[fra][0]<<", "<<lowleft[fra][1]<<"   ur= "<<upright[fra][0]<<", "<<upright[fra][1]<<endl;
      //outty.close();
    }else{
      //new segmentation calculation specific to each frame
		   segment( offset, s, 0, gf,fra ); 
    }
    

		//initialize id array which records coverage info. during painting  
		build_id( s ); 
    if(segmentation_type == -1 ){
     // cout<<"SEGMENT PAINTING SKIPPED FOR DEBUGGING"<<endl;
      if ( complex ){
        build_cid();
        st_max[fra] = 16;           // Initial arr size; usually grows
        st[fra] = new stroke_s[ st_max[fra] ];
        cout<<"Begin painting ..."<<endl;
        set_bezier( BEZIER_ON );
        st_n[fra] = stroke( cid, s.w * PIX_W, s.h * PIX_H, 0, st[fra],st_n[fra], st_max[fra], 1, segmentation_type);
        cout<<"End painting."<<endl;
      }else{
            indication_setup();
      }
    }else{
		  // *****Create the stroke lists for the initial visit to a frame
		  if (!new_frame_setup()) {
    		  printf("New_frame_setup() of frame 0 in main unsuccessful.\n");
		  } else {
    		  printf("New_frame_setup() was successful.\n");
		  }
    }
		//end of inseparable
    
		break;  // end of case NEW_FRAME
	case REDRAW:
    if (DEBUG) printf("Display status = REDRAW\n");
    break;
	default:   
		//do nothing
		break;
}		//end switch statement


if (display_status != NO_DATAFILE ){
  display_shell(GL_RENDER);  
	Togl_SwapBuffers( togl );
}
}								//	end routine togl_display_cb



void togl_reshape_cb( Togl *togl )
{
  float aspect;
  int w, h;

  w = Togl_Width( togl );
  h = Togl_Height( togl );

  aspect = float( w ) / float( h );

  glViewport(0, 0, w, h);

  glMatrixMode( GL_PROJECTION );
  glLoadIdentity();
 
  if ( complex ){
    glOrtho(0, fwidth, 0, fheight, -1, 1000);
  }else{
    gluPerspective(Y_FOVY, ASPECT_RATIO, 1, 10000);  
    position_camera();
  }
 /*  clearcolor[ 0 ] = 0;
  clearcolor[ 1 ] = 0;  //blue
  clearcolor[ 2 ] = .5;
  */
  /*clearcolor[ 0 ] = .4;
  clearcolor[ 1 ] = .297;  //brown
  clearcolor[ 2 ] = .246;
  */
  /*
  clearcolor[ 0 ] = .93;
  clearcolor[ 1 ] = .945;  //light blue
  clearcolor[ 2 ] = .918;
*//*
  clearcolor[ 0 ] = .77;
  clearcolor[ 1 ] = .286;  //orange
  clearcolor[ 2 ] = .0;
  */
 /* if( complex ){
    clearcolor[ 0 ] = 0;
    clearcolor[ 1 ] = 0;  
    clearcolor[ 2 ] = 0;
  }else{
   */ clearcolor[ 0 ] = .289;
    clearcolor[ 1 ] = .3125;  //grey
    clearcolor[ 2 ] = .453;
 // }

  clearcolor[ 0 ] = .4;
  clearcolor[ 1 ] = .297;  //brown
  clearcolor[ 2 ] = .246;
clearcolor[ 0 ] = .2;
  clearcolor[ 1 ] = .3;  //blue
  clearcolor[ 2 ] = .49;

  clearcolor[ 0 ] = .258;
  clearcolor[ 1 ] = .129;  //brownish green
  clearcolor[ 2 ] = 0;





    clearcolor[ 0 ] = .46;
  clearcolor[ 1 ] = .41;  //greeny-tan
  clearcolor[ 2 ] = .239;

    clearcolor[ 0 ] = 0;
  clearcolor[ 1 ] = .3;  //green
  clearcolor[ 2 ] = .137;
    clearcolor[ 0 ] = 0;
  clearcolor[ 1 ] = 0;  //black for now!!!
  clearcolor[ 2 ] = 0;

  glClearColor( clearcolor[ 0 ], clearcolor[ 1 ], clearcolor[ 2 ], 1 );


  Togl_PostRedisplay( togl );
}

static int frame_change_cb( Togl *togl, int argc, char *argv[] )

//  TOGL frame_change callback, used to the display of new frame
//  togl:  Pointer to TOGL interpreter
//  argc:  Number of arguments passed
//  argv:  Arguments passed
{
  frame_change_flag = 1;
  Togl_PostRedisplay( togl );
  return TCL_OK;
}

static int mapping_cb( Togl *togl, int argc, char *argv[] )

//  TOGL mapping callback, used to invoke the mapping (from data 
//      attributes to visual features) dialog box is requested
//  togl:  Pointer to TOGL interpreter
//  argc:  Number of arguments passed
//  argv:  Arguments passed
{
  vmap_invoke( togl, (bin_file *) &gf );

  Togl_PostRedisplay( togl );
  return TCL_OK;
}

static int rot_cb( Togl *togl, int argc, char *argv[] )

//  TOGL rotate callback, invoked to rotate the image plane about
//  the X-axis
//
//  togl:  Pointer to TOGL interpreter
//  argc:  Number of arguments passed
//  argv:  Arguments passed
{
  if ( argc != 3 ) {
      Tcl_SetResult( Togl_Interp( togl ),
                      "Wrong # args: \"pathname rotate ?angle?\"", TCL_STATIC );
      return TCL_ERROR;
  }

  rotate[cur_axis] += int( atof( argv[ 2 ] ) / 2.0 );

  Togl_PostRedisplay( togl );
  return TCL_OK;
}                   // End routine rot_cb

static int trans_cb( Togl *togl, int argc, char *argv[] )

//  TOGL trans callback, invoked to translate the image plane
//
//  togl:  Pointer to TOGL interpreter
//  argc:  Number of arguments passed
//  argv:  Arguments passed
{
  if ( argc != 4 ) {
      Tcl_SetResult( Togl_Interp( togl ),
                      "Wrong # args: \"pathname translate ?x y?\"", TCL_STATIC );
      return TCL_ERROR;
  }
  //cout<<"In trans_cb"<<endl;
  translate[ 0 ] += atof( argv[ 2 ] ) ;/// 2.0;
  translate[ 1 ] += atof( argv[ 3 ] ) ;/// 2.0;

  Togl_PostRedisplay( togl );
  return TCL_OK;
}                   // End routine trans_cb

static int zoom_cb( Togl *togl, int argc, char *argv[] )

//  TOGL zoom callback, invoked to translate the image plane
//
//  togl:  Pointer to TOGL interpreter
//  argc:  Number of arguments passed
//  argv:  Arguments passed
{
  if ( argc != 3 ) {
      Tcl_SetResult( Togl_Interp( togl ),
                      "Wrong # args: \"pathname zoom ?z?\"", TCL_STATIC );
      return TCL_ERROR;
  }
  
  float ratchet = 10.0;

  if (complex ){
    if ( scale[0] < .1 ){ 
      scale[0] = .1; scale[1] = .1;
      
    }else{
      scale[0] += atof( argv[ 2 ] ) / ratchet;
      scale[1] += atof( argv[ 2 ] ) / ratchet;
    }
  }else{
    translate[ 2 ] += atof( argv[ 2 ] ) / ratchet;
  }
  Togl_PostRedisplay( togl );
  return TCL_OK;
}                   // End routine zoom_cb

// *********end tcl/tk methods************************

// Non
static void build_id( seg_s& s )

//  This routine expands the segment ID array for the elements in
//  the dataset; this is necessary to provide more than a single
//  pixel to represent each element
//
//  s:  Segment structure
{
  int  i, j, l, m;          // Loop counters
  int  x, y;                // Lower-left corner of element tile

  id = new int *[ s.h * PIX_H ];    // Create expanded id array
  for ( i = 0; i < s.h * PIX_H; i++ ) {
      id[ i ] = new int[ s.w * PIX_W ];
  }

  for ( i = 0; i < s.h; i++ ) {
    for ( j = 0; j < s.w; j++ ) {
      x = j * PIX_W;            // Compute element position in..
      y = i * PIX_H;            // ..expanded id array

      //  Copy element's segment ID into a PIX_W by PIX_H tile in id
      //  array

      for ( l = y; l < y + PIX_H; l++ ) {
        for ( m = x; m < x + PIX_W; m++ ) {
          id[ l ][ m ] = s.seg[ i ][ j ];
        }
      }
    }
  }
}      // End routine build_id


static void build_cid()

//  This routine creates a dummy ID array so that every element in
//  the dataset can be put into one segment for stroke, but segmented
//  on voronoi regions for mapping the attributes; 
//  cid serves the same purpose as int** id used for other techniques ( allows
//  more than a single pixel to represent each element
//
//  s:  Segment structure
{
  int  i, j, l, m;          // Loop counters
  int  x, y;                // Lower-left corner of element tile

  cid = new int *[ s.h * PIX_H ];    // Create expanded id array
  for ( i = 0; i < s.h * PIX_H; i++ ) {
      cid[ i ] = new int[ s.w * PIX_W ];
  }

  for ( i = 0; i < s.h; i++ ) {
    for ( j = 0; j < s.w; j++ ) {
      x = j * PIX_W;            // Compute element position in..
      y = i * PIX_H;            // ..expanded id array

      //  Copy element's segment ID into a PIX_W by PIX_H tile in id
      //  array

      for ( l = y; l < y + PIX_H; l++ ) {
        for ( m = x; m < x + PIX_W; m++ ) {
          cid[ l ][ m ] = 0;
        }
      }
    }
  }
}      // End routine build_id


static int dump_cb( Togl *togl, int argc, char *argv[] ){
  int i;

  for( i = 0; i < frames; i++ ){
    if( st_max[i] != 0 ){  // unused frames have st_max of zero
      delete [] st[i];
      if( segmentation_type == -1){
        delete [] region_arr[i];
      }
    }
  }
  delete [] st;

  if (complex){
    for (i = 0; i < s.h*PIX_H; i++) {
      delete [] id[i];
      delete [] cid[i];
    }
    delete [] id;
    delete [] cid;
  }else{
    for (i = 0; i < s.h*PIX_H; i++) {
      delete [] id[i];
    }
    delete [] id;
  }
  delete [] region_arr;

  delete [] st_n;
  delete [] st_max;
  
  if ( segmentation_type == -1 ){
    for ( i = 0; i < 2; i++ ){
      delete [] lowleft[i];
      delete [] upright[i];
    }

    delete [] lowleft;
    delete [] upright;
  }

  for ( i = 0; i <  orig_features[0].num_feature(); i++ ) {
		delete [] map_matrix[ i ];
	}
  delete [] map_matrix;
  delete [] orig_features;
  delete [] names;
  delete [] num_valid_pts;
  delete [] hfactor_frame_flag;
  delete [] amin;
  delete [] amax;

  flush_seg_full( s );
  
  gf.close();
  s.f.close();
  if (segmentation_type == -1){
     delete vor; //free_voronoi();
  }

  if (complex){
     bm.dump_buff();
  }

  delete [] v;
 // DumpUnfreed(  );
 
  return TCL_OK;
  //    myjpeg.write_tiff( name, 3 );
}				//End routine dump_cb


static int capture_jpg_cb( Togl *togl, int argc, char *argv[] ){
  char* name = new char [512];
  // get a name for the image
  int cur_col; int cur_row;
  float quo;
  if ( view == -1 ){
    sprintf( name, "pics/defaultname%d.jpg", jpeg_count);
    jpeg_count++;
  }
  else{ 
    cur_col = view%numc;
	quo = view/numc;
    cur_row = floor(quo);
    sprintf( name, "pics/view%dcol%drow%d.jpg", view, cur_col, cur_row);
  }
/*
  if (segmentation_type == -1 ){
    string* part = new string [5];
    string base_name;
    vor->get_basename( newfile, &base_name );   // & gets the address of base_name
    base_name.token( part, 5,"/");
    if (is_jpeg){
      if( mucha ){
        sprintf( name, "study_pics/jpegs_id/%sid.jpg", (char*)part[1]);
      }else if (complex ){
        sprintf( name, "study_pics/jpegs_vc/%svc.jpg", (char*)part[1]);
      }else {
        sprintf( name, "study_pics/jpegs_ic/%sic.jpg", (char*)part[1]);
      }
    }else{
      if( mucha ){
          sprintf( name, "study_pics/npv_id/%sid.jpg", (char*)part[1]);
        }else if (complex ){
          sprintf( name, "study_pics/npv_vc/%svc.jpg", (char*)part[1]);
        }else {
          sprintf( name, "study_pics/npv_ic/%sic.jpg", (char*)part[1]);
        }
    }

  }else{
    name = "aaa_default_name.jpg";
  }
  if (argc == 3){
    name = argv[2];
  }
  */
  jpeg myjpeg;
  GLint viewport[ 4 ];

  glGetIntegerv(GL_VIEWPORT, viewport );
  myjpeg.quality( 100 );
  myjpeg.capture( 0, 0, viewport[ 2 ], viewport[ 3 ] );

  myjpeg.write( (string)name, 3 );
  printf( "%s captured.\n", name);

  return TCL_OK;
  //    myjpeg.write_tiff( name, 3 );
}				//End routine capture_jpg_cb

static int stepup_cb( Togl *togl, int argc, char *argv[] ){
  int temp = num_to_draw + 1;
  if ( temp <= num_valid_pts[fra]){
	  num_to_draw++;
  }
  temp = seg_to_draw + 1;
  if (temp <= s.n){
	  seg_to_draw++;  
  }
  Togl_PostRedisplay( togl );
  return TCL_OK;
}

void choose_display_status(){
  // This routine determines if something 
  // other than redisplaying needs to occur when the display_cb is called.
  // Alternates are:
  //  1) No data should be displayed (NO_DATAFILE) Notice the name 'NO_DATA' is defined elsewhere so can't be used here
  //  2) A new data set is being displayed; (NEW_DATA) Stroke lists must be allocated and set up
  //  3) A new highlight factor has been selected (NEW_HFACTOR) so new highlights stroke lists must be created
  //  4) A different frame is being displayed.  
  //        a) The frame has been displayed before; stroke list is already stored. (REDRAW)
  //        b) First time displaying this frame. New stroke list must be created. (NEW_FRAME)
  //  5) A new mapping has been chosen; stroke lists must be revised (NEW_MAPPING)
  //
  //
  attr_map feature;
  int i;
  // printf("hfactor = %f and the flag = %d\n", hfactor, hfactor_flag);
  // Set the display status based on values of newfile, oldfile, and other flags	
  if ( strcmp( oldfile, "" ) == 0 && strcmp( newfile, "" ) == 0 ) {
    display_status = NO_DATAFILE; 
  } else {
    if ( strcmp(newfile,oldfile)!= 0 && strcmp(newfile,"")!= 0 ) {
		  display_status = NEW_DATA;
  	
	  } else if ( hfactor_flag != 0 || hfactor_frame_flag[fra] == 1) {
	  if (hfactor_flag == 1) {
		  for ( i = 0; i <fra; i++ ){
			  hfactor_frame_flag[ i ] =1;
		  }
	  }
	  display_status = NEW_HFACTOR;
	    hfactor_flag = 0;
		  hfactor_frame_flag[fra] = 0;
	  }else if (frame_change_flag != 0 ) {
		  if ( frame_change_flag != 1 ) {
			  printf("Warning in main.cpp:  frame_change_flag should only be 0 or 1.  frame_change_flag = \n",frame_change_flag);
		  }
		  frame_change_flag = 0;
		  if (fra> frames - 1 ){ 
		   fra= 0;
		  }else if (fra < 0){
			 fra= frames - 1;
		  }
		  for (int i = 0; i < 12; i++ ){
  		  printf("In here:::::::st_max[%d] = %d\n", i, st_max[i]);
		  }
		  if (st_max[fra] == 0){ 
		    display_status = NEW_FRAME;
		  }
 	    printf("In keyboard function:  The frame number is %d\n", fra);
	  }else {
      for ( i = 0; i< s.attr_n; i++ ) {
        feature = gf.attr_map_val(i); 
        if (DEBUG = 0){
          printf("main.cpp: New feature(%d) = %s   Original feature(%d) = %s\n", i, (char *)feature.name(),i, (char*)orig_features[i].name() );
			  } DEBUG =0;
        if ( feature.name()!= orig_features[i].name() ) {
          display_status = NEW_MAPPING;
		      orig_features[ i ] =  feature;
        }
		  }
	  } 
  }
}//end of choose display status


static void comp_coverage( int seg_id, int attr_id, float& cov )

//  This private routine computes a coverage value for a segment
//  based on the average attribute value bound to coverage
//
//  seg_id:   ID of segment to query
//  attr_id:  ID of attribute bound to cover
//  cov:      Coverage value (0..1)
{
  int    i, j;              // Loop counters
  int    n = 0;             // Count of pixels in segment
  float  sum = 0;           // Sum of coverage attribute values

  for ( i = 0; i < s.h * PIX_H; i++ ) {
      for ( j = 0; j < s.w * PIX_W; j++ ) {
          if ( id[ i ][ j ] == seg_id ) {
              if ( interpolate( j, i, v ) ) {
                  n++;
                  sum += v[ attr_id ];
                  // cout<<v[ attr_id ] << " = v[ attr_id ]" <<endl;
              }
          }
      }
  }
  if (n!=0) {
      sum /= float( n );

      cov = 0.4 +
            ( ( sum - amin[ attr_id ] ) / ( amax[ attr_id ] - amin[ attr_id ] ) * 1.1 );
  } else {
      cov = 0; 
  }
  cov = ( cov > 1 ) ? 1 : cov;
  
}                   // End routine comp_coverage


void comp_highlight_size( stroke_s& the_stroke )
//  This routine sets the highlight stroke sizes to the average stroke 
//  size in the segment over which the stroke is drawn.
//
//  the_stroke:  The highlight stroke to be resized
{

  int seg_number;   // Segment ID number where highlight stroke is drawn
  int x, y;         //Rounded values to access integer segment ID array
  count++;

  x =  ROUND(the_stroke.p.x);
  y =  ROUND(the_stroke.p.y);
  x = ( x < 0 ) ? 0 : x;    //if x neg, set to 0
  y = ( y < 0 ) ? 0 : y;
  seg_number = id[ y ][ x ];
  the_stroke.s[ 0 ] = avg_size[ seg_number ][ 0 ];
  the_stroke.s[ 1 ] = avg_size[ seg_number ][ 1 ];
}                   // End routine comp_highlight_size


int get_current_frame(){
return fra;		 
}

static int get_current_position( int x_mult, int y_mult, float pixels_w, float pixels_h ){
float x;
float y;

x = float( x_mult )/pixels_w;
y = float( y_mult )/pixels_h;

return int(( y * s.w ) + x);
}

void get_jtransformation( int my_view ){
  float scale_size = 1;
  float min_f_ratio = 1;
  float max_d;
  float quo;

  if ( my_view >= num ){
    my_view = -1;
    view = -1;
  }
  if ( my_view == -1 ){
    // return to default (overview)
    jsc[ 0 ] = 1;
    jsc[ 1 ] = 1;
    jtrans[ 0 ] = 0;
    jtrans[ 1 ] = 0;
  }else{
    // amount to shift vertically and horizontally
    quo = view/numc;
    jtrans[ 1 ] = floor(quo);
    jtrans[ 0 ] = view%numc; 
    
    min_f_ratio = (fwidth < fheight) ? (float)fwidth/(float)fheight : (float)fheight/(float)fwidth;
    max_d = max( numr, numc );

    jsc[ 0 ] = min_f_ratio*max_d;
    jsc[ 1 ] = min_f_ratio*max_d;
  }
}

float get_value (int index, float v_array[] )
// This routine finds the average value of the attributes assigned to visual feature (normalize to a 
// value between 0 and 1).
// E.g., color may be mapped to temperature, precipitation, and wind speed.  The values
// of these attributes at the given point are contained in v_array.  Map_matrix has a value of 1
// if for each attribute to which color is assigned and 0 for all others.  
// 
// int index: the position of the visual feature in the Name array in attr_map.cpp 
// float [ ] v: array of data attribute values at a particular position
{
int i;
float sum = 0;
float count = 0;
for (i = 0 ; i < s.attr_n; i++ ) {
  sum += map_matrix[ index ][ i ]*(v_array[ i ] - amin[ i ])/(amax[ i ] - amin[ i ]);
  count += map_matrix[ index ][ i ];
}
if ( count == 0 ){
	sum = -1;
	count = 1;
}
return sum/count;
} //End routine get_value


static void init_stroke_lists()

//This routine allocates space for st, back_st, and high_st,
//the background, foreground, and highlight stroke lists and their stroke
//counts st_n, back_st_n, and high_st_n.  st_max is reused for all three
//stroke list.
//The sizes are dynamics since they depend on the number of frames in the data
//file.
{
  int    i;                 // Loop counter

  if ( st_n == NULL ){
    st_max = new int[frames];
    st_n = new int[frames];   
  }
  //back_st_n = new int[frames];
  //high_st_n = new int[frames];   

  st = new stroke_s*[frames];         //allocate foreground strokes
 // back_st = new stroke_s*[frames];        //allocate background stroke list
 // high_st = new stroke_s*[frames];    //allocate highlight stroke list
  for ( i = 0; i < frames; i++) {
      st_n[i] = 0;
      st_max[i] = 0;  // st_max will differentiate between new
      // and used frames, since it's changed
      // to 16 or greater in new_frame_setup
    //  back_st_n[i] = 0;
    //  high_st_n[i] = 0;            
  }  
}

void init_stuff()
{
  /*if ( LIGHTING_ON ){
    init_lighting();
  }*/

  glMatrixMode( GL_MODELVIEW);
  glLoadIdentity();
  
  //init_nurbs();

  int FULL_RUN = 1;
   
  if (FULL_RUN){
    init_maps();  
  } // end if FULL_RUN
  
  show_voronoi = 0;
	segmentation_type = -1;//0;
  fwidth = 970;//1100; //1255
  fheight = 970;//850; //970

  //jpeg capture specific vars
  numc = 2;
  numr = 2;
  num = numc*numr;

  PIX_W = 4;
  PIX_H = 4;
  if(complex){
    bm.alloc_buffers( fwidth, fheight );
  }else if ( mucha && !is_jpeg && !is_iouca){
    PIX_W = 8;
    PIX_H = 8;
  }
 
}

void init_maps(){
   sty_canv = 0;
  
  //Max 14 textures of size 64X256  
  sty_div = 0;
 
    if ( mucha )
      init_stroke("s3a.map"); 
      init_stroke("s5a.map"); 

      sty_reg = 2; cnt_reg = 5;
      init_stroke("s1b.map"); init_stroke("s2b.map"); init_stroke("s3b.map"); init_stroke("s4b.map"); init_stroke("s5b.map");

      cnt_med = 5;
      init_stroke("s1d.map"); init_stroke("s2d.map"); init_stroke("s3d.map"); init_stroke("s4d.map"); init_stroke("s5d.map");
          
      cnt_low = 5;
      init_stroke("s7d.map"); init_stroke("s8d.map"); init_stroke("s7d.map"); init_stroke("s8d.map"); init_stroke("s8d.map");
    }else if(complex){

      init_stroke("tear1.op.map");
      init_stroke("tear1.smooth.map");
      
      init_stroke("stroke1.op.map");
      init_stroke("stroke1.hc.map");

      init_stroke("stroke2.op.map");
      init_stroke("stroke2.map");

      init_stroke("stroke4.op.map");
      init_stroke("stroke4.hc.map");

      init_stroke("stroke8.op.map");
      init_stroke("stroke8.hc.map");

      init_stroke("stroke14.op.map");
      init_stroke("stroke14.map");

      init_stroke("stroke16.op.map");
      init_stroke("stroke16.map");
      
      init_stroke("canvas1_grey_small.map");
    
      cnt_med = 14;


    }else{
      // Initialize canvas
      init_stroke("canvas1_grey_small.map"); 
     
      sty_reg = 1; cnt_reg = 5;

      init_stroke("brush.01.map");init_stroke("brush.02.map");init_stroke("brush.03.map");init_stroke("brush.04.map");init_stroke("brush.05.map");

      cnt_med = 4;
      
      init_stroke("stroke1.map");init_stroke("stroke2.map");init_stroke("stroke3.map");init_stroke("stroke4.map");
      
      // Initialize "dry brush" strokes to be used in low detail zones.
      cnt_low = 5;
      
      init_stroke("low1.map");
      init_stroke("low2.map");
      init_stroke("low3.map");
      init_stroke("low16.map");
      init_stroke("low19.map");
  }
    
    sty_med = sty_reg + cnt_reg;
  
    sty_low = sty_med + cnt_med;
}

static int interpolate( float x, float y, float *val )

//  This routine maps an (x,y) stroke location from a tile embedded
//  within the id array into an interpolated set of attribute values
//
//  x, y:  Position of stroke
//  val:   Array for interpolated values
{
  float  dx, dy;            // Fractional position in dataset
  int    i, j, k;              // Loop counters
  int    min_x, max_x;      // X-location of corners
  int    min_y, max_y;      // Y-location of corners
  float *vals[ 6 ];            // Attribute values ag position


  for ( i = 0; i < 6; i++ ) {        // Create space for attribute values
      vals[ i ] = new float[ s.attr_n ];
  }

  dx = float( x ) / PIX_W;          // Map from tile into dataset
  dy = float( y ) / PIX_H;

  //  This is the arrangement for the various vals and dx,dy values:
  //
  //   v[2]         v[3]
  //     +------------+
  //     |            |
  // v[4]|      *     |v[5]
  //     |  val@dx,dy |
  //     +------------+
  //   v[0]         v[1]
  //

  min_x = int( floor( dx ) );       // Clamp to corner positions
  max_x = int( ceil( dx ) );
  min_y = int( floor( dy ) );
  max_y = int( ceil( dy ) );

  min_x = ( min_x < 0 ) ? 0 : min_x;
  min_y = ( min_y < 0 ) ? 0 : min_y;
  max_x = ( max_x >= s.w ) ? s.w - 1 : max_x;
  max_y = ( max_y >= s.h ) ? s.h - 1 : max_y;

  //  Grab attribute values stored at corner positions

  gf.get( ( min_y * s.w ) + min_x, fra,  vals[ 0 ] );
  gf.get( ( min_y * s.w ) + max_x, fra,  vals[ 1 ] );
  gf.get( ( max_y * s.w ) + min_x, fra,  vals[ 2 ] );
  gf.get( ( max_y * s.w ) + max_x, fra, vals[ 3 ] );

  for ( i = 0; i < 4; i++ ) {        // Quit if any corner undefined
      //for ( j = 0; j < s.attr_n; j++ ) {
     if( !gf.allowable(vals[i]) ){// if ( v[ i ][ j ] < -990 ) {
        for ( k = 0; k < 6; k++ ) {
           delete [] vals[ k ];
        }
        return 0;
     }
  }

  for ( i = 0; i < s.attr_n; i++ ) { // Compute interpolated attr values
      vals[ 4 ][ i ] =
      vals[ 0 ][ i ] + ( ( dy - min_y ) * ( vals[ 2 ][ i ] - vals[ 0 ][ i ] ) );
      vals[ 5 ][ i ] =
      vals[ 1 ][ i ] + ( ( dy - min_y ) * ( vals[ 3 ][ i ] - vals[ 1 ][ i ] ) );
      val[ i ] =
      vals[ 4 ][ i ] + ( ( dx - min_x ) * ( vals[ 5 ][ i ] - vals[ 4 ][ i ] ) );
  }

  for( i = 0; i < 6; i++ ){
    delete [] vals[ i ];
  }
  return 1;
}                   // End routine interpolate

int map_mat( int x, int y, stroke_s& stroke){
//  This routine maps an (x,y) stroke location from a tile embedded
//  within the id array into an interpolated set of attribute
//  values; those values are then used to define the stroke's colour
//  and rotation
//
//  x, y:  Position of stroke
//  st:    Stroke structure
  //float *v;					          // Interpolated value array
  float color;				        // The value assigned to color (based on data values)


  if ( !interpolate( x, y, v ) ) {  // Ask for interpolated attr vals; quits if any corner undefined
      //delete [] v;
			return 0;
  }
  color = get_value( num_col, v ); 
	if( color == -1){ 
		cerr<<"Error in map_mat in main.cpp: No attributes mapped to color. Stroke cannot be drawn."<<endl;
	}
  //Stroke color based on "color" set above with method get_value
	get_colour( color, stroke.r, stroke.g, stroke.b );
if (DEBUG){
	outfile<<"Color: "<< color<<"( stroke.r, stroke.g, stroke.b )= ( "<<stroke.r<<" , "<<stroke.g<<" , "<<stroke.b<<" )"<<endl;
	outfile<<"Attr # mapped to color = "<<num_col<<endl;
}

  stroke.rot = -990.0;  // using this as a flag to indicate the stroke is a mat one.
  
	stroke.s[ 0 ] = 4;
  stroke.s[ 1 ] = 16;
	
  return 1;
}	


static int invalid_corner( int region_id, float* valu ){
  int card = vor->get_cardinality( fra, region_id );
  Point pt;
  for( int i = 0; i < card; i++ ){
    pt = vor->get_poly_pt( fra, region_id, i );
    if ( !is_valid( pt.x, pt.y, 1, 1, valu ) ){
      return 1;
    }
  }
 return 0;
}

static int region_invalid( int region_id, float ll[], float ur[], float* valu ){
  // Routine checks if the current region contains only invalid points.
  // Returns 0 if there is at least one valid point.
  // Returns 1 if there are no valid points.
  
  int i, j;
  int tmp_reg;
  
  for( i = ll[1]; i <= ur[1]; i++ ){
    for( j = ll[0]; j <= ur [0]; j++ ){
      // get the region id
      tmp_reg = vor->find_centroid(fra, j, i );
      // we're only interested in points within our own region.
      if ( region_id == tmp_reg ){
          if ( is_valid( j, i, 1, 1, valu ) ){  // and current neighbor is valid
            return 0;
          }
      }
    }
  }
  return 1;
}


static int region_mixed( int region_id, float ll[], float ur[], float* valu ){
  // Routine checks if the current region contains only invalid points.
  // Returns 0 if there is at least one valid point.
  // Returns 1 if there are no valid points.
  
  int i, j;
  int tmp_reg;
  
  for( i = ll[1]; i <= ur[1]; i++ ){
    for( j = ll[0]; j <= ur [0]; j++ ){
      // get the region id
      tmp_reg = vor->find_centroid(fra, j, i );
      // we're only interested in points within our own region.
      if ( region_id == tmp_reg ){
          if ( !is_valid( j, i, 1, 1, valu ) ){  // and current neighbor is valid
            return 1;
          }
      }
    }
  }
  return 0;
}


static void voronoi_segment(){
  // This routine segments the data by first assigning segment ids to regions
  // Valid regions are categorized based on size and what sized regions they 
  // Invalid regions (those with invalid centers) are categorized 
  // based on what regions they abut and then checked for 
  // valid points.  Completely invalid regions are put in one segment.
  // Individual points are then given segments ids based on their region's 
  // segment id and their own validity.

  float area;
  int i, j; //counters  
  float ll[2]; float ur[2];
  int num_regs;
  int reg;  // region id holder
  Point pt;

  num_regs = vor->get_num_regions(fra);
  

  // 7 segments:
  // 0: small regions (with no large neighbors
  // 1: large regions (with no small neighbors)
  // 2: small regions with large neighbors
  // 3: large regions with small neighbors
  // 4: mixed regions (valid/invalid pts) with no small neighbors
  // 5: invalid pts in mixed regions.
  // 6: completely invalid regions.  Not painted at all. So set s.n to 6.
  s.n = 6;
  int testing = 0;
  if ( testing ){
   //  for (i = 0; i < num_valid_pts[fra]; i++ ){
   //    region_arr[fra][ i ] = 0; 
   //  }
   for( i = 0; i < s.h*.125; i++ ){
    for( j = 0; j < s.w*.125; j++ ){ 
       s.seg[ i ][ j ] = 6;
    }
   }
   for( i = s.h*.125; i<s.h; i++ ){
    for( j = s.w*.125;j<s.w; j++ ){ 
       s.seg[ i ][ j ] = 6;
    }
   }
  }else{
  //Initialize segment ids of regions with valid centers to 0 or 3 (if small) or 1 or 2 (if large).
  for (i = 0; i < num_valid_pts[fra]; i++ ){ 
    area = vor->get_area(fra, i );
    pt = vor->get_centroid_coor( fra, i );
    
    if ( area < threshold ){     // region is small
      
      if( is_boundary( pt.x, pt.y, 0, i, v ) ){
        region_arr[fra][ i ] = 3;           //small boundary region id 3
      }else{
        region_arr[fra][ i ] = 0;           //small non boundary region id 0
      }
   
    }else {                       // region is large

      if (is_boundary( pt.x, pt.y, 1, i, v ) ){
        region_arr[fra][ i ] = 2;         //large boundary region id 2
      }else{
        region_arr[fra][ i ] = 1;          //large non boundary region id 1
      }

    }
    
    if ( region_arr[fra][i] == 2 ){
      update_low_bb( pt.x*PIX_W, pt.y*PIX_H);
    }
  }  // end of categorizing regions with valid centers

  // Initialize remaining region ids 
  for ( i = num_valid_pts[fra]; i < num_regs; i++ ){ 
      region_arr[fra][ i ] = -1; 
  }

  // Categorize the rest of the regions based on the segments of their neighbors
  for ( i = num_valid_pts[fra]; i < num_regs; i++ ){ 
      get_region_bbox( i, ll, ur );
      pt = vor->get_centroid_coor( fra, i );
      // If a region has no valid points mark it as segment 6.
      if( region_invalid( i, ll, ur, v ) ){
        region_arr[fra][i] = 6;
      } else {  
        region_arr[fra][i] = vor->find_mixed_reg_index(fra,pt.x,pt.y,i,v,region_arr);//get_invalid_reg_id( pt.x, pt.y, i, v);
        if ( region_arr[fra][i] == 2 ){
          update_low_bb( pt.x*PIX_W, pt.y*PIX_H);
        }
      }
  }

  // Assign segment ids to s.seg.
  for( i = 0; i < s.h; i++ ){
    for( j = 0; j < s.w; j++ ){ 
      reg = vor->find_centroid(fra, j, i );
      if(reg == -1){ // no neighbors found
         s.seg[ i ][ j ] = 6;
      }else if( is_valid( j, i, 1, 1, v ) || region_arr[fra][reg] == 6 ) {
         get_region_bbox( reg, ll, ur );
         if ( (region_arr[fra][reg] == 1 || region_arr[fra][reg] == 2 ) && region_mixed( reg, ll, ur, v ) && invalid_corner( reg, v)  ){
           s.seg[i][j] = 4;
           update_low_bb( j*PIX_W, i*PIX_H );
         }else{
           s.seg[ i ][ j ] = region_arr[fra][reg];
           if ( region_arr[fra][reg] == 1 || region_arr[fra][reg] == 2 || region_arr[fra][reg] == 3 ){
             update_low_bb( j*PIX_W, i*PIX_H );
           }
         }

      }else {
        s.seg[ i ][ j ] = 5;
      }
    }
  }
  }
}  //end routine voronoi_segment()

int get_vecs( Point pt, float& dx, float& dy){
  float orientation;

  if ( !interpolate( pt.x, pt.y, v ) ) {  // Ask for interpolated attr vals; quits if any corner undefined
      //delete [] v; 
			return 0;
  }
  
  orientation = get_value( num_ori, v ); 
  orientation = -90.0 + ( orientation * 90.0 );
  
  if ( is_iouca ){
    if ( v[5] >=180 ){
      orientation = v[ 5 ] -  180;  // flip all to 0 to 180, since we're not showing direction, only orientation
    }else{
      orientation = v[ 5 ];
    }
  }

  return 1;
}

inline int set_orientation( Point pt, float& orientation ){
  //Finds orientation based on the values of v, a global float* holding the current attribute values
  float ori_factor = 1.0;  // jpeg npr's use mitagating factor and addend to restrict orientation range
  float ori_addend = 0.0;

  if (!is_jpeg){
    orientation = -90.0 + ( orientation * 90.0 );
  }

  if ( is_iouca ){
    if ( v[5] >=180 ){
      orientation = v[ 5 ] -  180;  // flip all to 0 to 180, since we're not showing direction, only orientation
    }else{
      orientation = v[ 5 ];
    }
	  orientation = orientation - 90; // How much the stroke should be rotated from vertical to achieve this angle.
    return 0;
  }else if ( is_jpeg ) {

    float avg;
    float area, avg_area, max_area; // jpeg npr's use region area to determine orientation

    int reg;  // jpeg npr's use region area to determine orientation

    //reg = vor->find_centroid(fra,pt.x/PIX_W,pt.y/PIX_H);
    //orientation = -90.0;  // default CW rotation 90degrees; i.e., stroke is flat on its right side
   // if( reg > - 1){
      //orientation = vor->get_area( fra, reg );
      //vor->find_local_area( fra, reg, avg_area, max_area ); // set orientation to the max value in the neighborhood
      //orientation = max_area;
   //   avg = (v[0] +v[1]+v[2])*.333333333;
  
      ori_factor = 0.3;
      ori_addend = 0.7;
      
      if ( orientation > .5 ){
        if( rand_num() > .05 ){  //.10 turtle
          // 1 flat ... 0.5 27degrees
          orientation = orientation*2 - 1; // shift [.5, 1] to [0, 1]
          orientation = ((ori_factor*(orientation))+ori_addend)*-90.0;
        }else{
          // 0.5 27 degrees ... 0 flat
          orientation = 1 - orientation*2;  // shift [0,0.5] to [0,-1] to [1,0]
          orientation = ((ori_factor*(orientation))+ori_addend)*90.0;
        }
      }else{
        if( rand_num() > .05 ){  //.15 turtle
        // 0.5 27 degrees ... 0 flat
        orientation = 1 - orientation*2;  // shift [0,0.5] to [0,-1] to [1,0]
        orientation = ((ori_factor*(orientation))+ori_addend)*90.0;
        }else{
          // 1 flat ... 0.5 27degrees
          orientation = orientation*2 - 1; // shift [.5, 1] to [0, 1]
          orientation = ((ori_factor*(orientation))+ori_addend)*-90.0;
        }

      }
     /*  if ( max_area > .4 ){
         orientation = ((ori_factor*orientation)+ori_addend)*-90.0;
         return 1;
      }else {
          orientation = ((ori_factor*orientation)+ori_addend)*90.0;
          return 0;
      }
      */
      /* if( max_area > .4){
        if ( ( rand_num() < .9) ){  //max_area > .2 && 
          orientation = ((ori_factor*orientation)+ori_addend)*-90.0;
          return 1;
        }else {
          orientation = ((ori_factor*orientation)+ori_addend)*90.0;
          return 0;
        }
      }else{
        if ( ( rand_num() < .3) ){  //max_area > .2 && 
          orientation = ((ori_factor*orientation)+ori_addend)*-90.0;
          return 1;
        }else {
          orientation = ((ori_factor*orientation)+ori_addend)*90.0;
          return 0;
        }
      }
      */
    //}
  }
  return 0;
}

inline int set_orientation( float midrot, Point pt, float& orientation ){
  //Finds orientation based on the values of v, a global float* holding the current attribute values
  if(!is_jpeg){
    orientation = -90.0 + ( orientation * 90.0 );
  }
  if ( is_iouca ){
    if ( v[5] >=180 ){
      orientation = v[ 5 ] -  180;  // flip all to 0 to 180, since we're not showing direction, only orientation
    }else{
      orientation = v[ 5 ];
    }
	  orientation = orientation - 90; // How much the stroke should be rotated from vertical to achieve this angle.
    return 0;
  }else if ( is_jpeg ) {

    float avg;
    float area, avg_area, max_area; // jpeg npr's use region area to determine orientation
    float ori_factor = .3;  // jpeg npr's use mitagating factor and addend to restrict orientation range
    float ori_addend = .7;
    int reg;  // jpeg npr's use region area to determine orientation
    float dist1; 
    float dist2; 
    float far_ori;
    float close;
    float otemp;
  /*  
    reg = vor->find_centroid(fra,pt.x/PIX_W,pt.y/PIX_H);
    orientation = -90.0;  // default CW rotation 90degrees; i.e., stroke is flat on its right side
    if( reg > - 1){
      //orientation = vor->get_area( fra, reg );
      //vor->find_local_area( fra, reg, avg_area, max_area ); // set orientation to the max value in the neighborhood
      //orientation = max_area;

      dist1 = fabs( ((ori_factor*orientation)+ori_addend)*-90.0 - midrot);
      dist2 = fabs( ((ori_factor*orientation)+ori_addend)*90.0 - midrot);

      if( dist1 > dist2 ){
       far_ori = ((ori_factor*orientation)+ori_addend)*-90.0;
        close = ((ori_factor*orientation)+ori_addend)*90.0;
      }else{
       far_ori = ((ori_factor*orientation)+ori_addend)*90.0;
        close = ((ori_factor*orientation)+ori_addend)*-90.0;
      }
      //  avg = (v[0] +v[1]+v[2])*.333333333;
      if( rand_num() < 0.3 ){
        orientation = far_ori;
        return 1;
      }else {
        orientation = close;
        return 0;
      }
      }
*/    
     
      // 1 flat ... 0.5 27degrees
      otemp = orientation*2; // shift [.5, 1] to [0, 1]
      dist1 = fabs( ((ori_factor*(otemp))+ori_addend)*-90.0 - midrot);

      // 0.5 27 degrees ... 0 flat
      otemp = 1 - orientation*2;  // shift [0,0.5] to [0,-1] to [1,0]
      dist2 = fabs(((ori_factor*(otemp))+ori_addend)*90.0 - midrot);


      if( dist1 > dist2 ){
        otemp = orientation*2 - 1; // shift [.5, 1] to [0, 1]
        far_ori = ((ori_factor*(otemp))+ori_addend)*-90.0;
        otemp = 1 - orientation*2; 
        close = ((ori_factor*(otemp))+ori_addend)*90.0;
      }else{
        otemp = 1 - orientation*2; 
        far_ori = ((ori_factor*(otemp))+ori_addend)*90.0;

        otemp = orientation*2 - 1; // shift [.5, 1] to [0, 1]
        close = ((ori_factor*(otemp))+ori_addend)*-90.0;
      }
      //  avg = (v[0] +v[1]+v[2])*.333333333;
      if( rand_num() < 0.3 ){
        orientation = far_ori;
        return 1;
      }else {
        orientation = close;
        return 0;
      }
  }
  return 0;
}

int set_orient( float rot, Point pt, float& orientation )

//  This routine maps an (x,y) stroke location from a tile embedded
//  within the id array into an interpolated attribute
//  value; this value is used to define the stroke's rotation
//
//  pt:  Point at which to find orientation
{

   if ( pt.x < 0 || pt.x/PIX_W >= s.w || pt.y < 0 || pt.y/PIX_H >=s.h  || !interpolate( pt.x, pt.y, v ) ) {  // Ask for interpolated attr vals; quits if any corner undefined
      //delete [] v; 
			return 0;
  }
  if ( !is_jpeg ){
    orientation = get_value( num_ori, v ); // get raw value of orientation
  }else{ 
    orientation = (v[0] + v[1] + v[2])/3;
  }
  set_orientation( rot, pt, orientation );

  return 1;
}


int map_color( float x, float y, float color [ 3 ] )

//  This routine maps an (x,y) location to an interpolated attribute
//  value; this value is then used to define the stroke's colour
//
//  x, y:  Position of stroke
//  color: Array to hold r,g,b
{
  float avg;
  
  float color_num;				        // The value assigned to color (based on data values)
  float factor = 1;//.666666666666667;
  float addend = 0;// .33333333333333333;
  float rand;
  float grey;
  float gfact = .5;//.666667; //.5
  float gadd = .5;//.333333; //.5
  Point pt;

  if ( !interpolate( x, y, v ) ) {  // Ask for interpolated attr vals; quits if any corner undefined
      //delete [] v; 
			return 0;
  }
  
  // Set the central color
  if (is_jpeg){
    color[ 0 ] = v[0]; color[ 1 ] = v[1]; color[ 2 ] = v[2];
  }else {
    color_num = get_value( num_col, v ); 
   
    if( color_num == -1){ 
		  cerr<<"Error in map_stroke in main.cpp: No attributes mapped to color. Stroke cannot be drawn."<<endl;
	  }
    //Stroke color based on "color" set above with method get_value
    grey = get_value( num_gre, v );
    get_colour( gfact*grey + gadd, color_num*factor + addend, color[ 0 ], color[ 1 ], color[ 2 ] );
  }
  return 1;
}


int map_stroke( float x, float y, stroke_s& stroke )

//  This routine maps an (x,y) stroke location from a tile embedded
//  within the id array into an interpolated set of attribute
//  values; those values are then used to define the stroke's colour
//  and rotation
//
//  x, y:  Position of stroke
//  st:    Stroke structure
{
  float avg;
    
  float avg_area, max_area;  
  //float *v;					          // Interpolated value array
  float color_avg;                // Average color, used in jpeg calculations
  float color;				        // The value assigned to color (based on data values)
  float diff;
  float factor = 1;//.666666666666667;
  float addend = 0;// .33333333333333333;
  float orientation;		      // The value assigned to orientation 
  float size;                 // The value assigned to size
  float rand;
  float grey;
  //int grey_num = 2; //6;//4;
  float gfact = .5;//.666667; //.5
  float gadd = .5;//.333333; //.5
  Point pt;
  int reg;
  float zigzag;
  //v = new float[ s.attr_n ];
  bool the_orient = false;
  int shape_bin = 0;
  if ( !interpolate( x, y, v ) ) {  // Ask for interpolated attr vals; quits if any corner undefined
      //delete [] v; 
			return 0;
  }
  
  stroke.type = 0x00;

  if ( complex ){
   stroke.seg_index = id[(int)y][(int)x];
  }else {
    stroke.seg_index = cur_seg;
  }

  // Set the central color
  if (is_jpeg){
    stroke.r = v[0]; stroke.g = v[1]; stroke.b = v[2];
  }else {
    color = get_value( num_col, v ); 
   
    if( color == -1){ 
		  cerr<<"Error in map_stroke in main.cpp: No attributes mapped to color. Stroke cannot be drawn."<<endl;
	  }
    //Stroke color based on "color" set above with method get_value
    grey = get_value( num_gre, v );
    get_colour( gfact*grey + gadd, color*factor + addend, stroke.r, stroke.g, stroke.b );
  }

  if (DEBUG){
	  outfile<<"Color: "<< color<<"( stroke.r, stroke.g, stroke.b )= ( "<<stroke.r<<" , "<<stroke.g<<" , "<<stroke.b<<" )"<<endl;
	  outfile<<"Attr # mapped to color = "<<num_col<<endl;
  }

  //////// Set rotation ////////
  if(!is_jpeg){
    orientation = get_value( num_ori, v );
    stroke.rot = orientation;
  }else{
    stroke.rot = (stroke.r + stroke.g + stroke.b)*.33333333333;
    if( rand_num() < 0.2){
      1- stroke.rot;
    }
  }
  
  pt.x = x;  pt.y = y;
  ct++;
  if ( set_orientation( pt, stroke.rot)){
   //stroke.r = 1; stroke.g = 0; stroke.b = 0;
    the_orient = true;
//    stroke.r = 1; stroke.g = 0; stroke.b = 0;
    vr++;
  }else{
    ir++;
  }
  //stroke.rot = 90;
  float heh; float ses; float beb;
  //Introduce strokes of contrasting color and possibly contrasting orientation
  rand = rand_num();
  zigzag = get_value( num_con, v );
  if ( complex && zigzag > .5 && rand > .95 ) {
    
    if(!is_iouca & !is_jpeg){ // no randomness is introduced into iouca orientition, since it contains orientation data; jpeg orientation is randomized inside set_orientation
      stroke.rot = 90.0 + ( orientation * -90.0 );
      stroke.type = 'z';
    }
    if ( !is_jpeg ){
      get_colour( gfact*grey + gadd*.75, color*factor + addend*.75, stroke.r, stroke.g, stroke.b );
    }else{
      //if ( stroke.r + stroke.g + stroke.b > 3 ){ 
      
        stroke.rot = -stroke.rot;
        stroke.type = 'z';
        stroke.r = stroke.r + (rand_num()-0.5)*0.1;  stroke.b = stroke.b + (rand_num()-0.5)*0.1; stroke.g = stroke.g + (rand_num()-0.5)*0.1;
        rgb_to_hsv( stroke.r, stroke.g, stroke.b, heh, ses , beb);
        beb = beb + 0.17;
        //heh = heh + (rand_num()-0.5)*2;
        hsv_to_rgb( stroke.r, stroke.g, stroke.b, heh, ses, beb);
    /*  }else{
       stroke.r = stroke.r + (rand_num()-0.5)*0.125;  stroke.b = stroke.b + (rand_num()-0.5)*0.125; stroke.g = stroke.g + (rand_num()-0.5)*0.125;
      }*/
    }
  }
  ////end of rotation and color contrast adjustments////

   //////// Compute stroke size ////////
  size = get_value( num_siz, v );
  //size = 1- size;
  comp_size( stroke, size );
  
  if ( rand_num() < .3 && complex && !is_jpeg){  // Change proportion of some strokes if num_pro's value > .4
    size = get_value( num_pro, v );
    comp_shape( stroke, size );
  }

  if (is_jpeg){  
    // stroke.rot is in [-90,90]
    /*
    size = ( stroke.rot < 0 ) ? -1*stroke.rot : stroke.rot;
    size = size*.01111111;
    comp_size(stroke, size); 
    */
   // comp_size(stroke, orientation );
    reg = vor->find_centroid(fra, x/PIX_W, y/PIX_H);
    if(  reg > - 1){
      vor->find_local_area( fra, reg, avg_area, max_area ); // set orientation to the max value in the neighborhood
        /*size = get_value( avg_area, v ); //turtle
        comp_size( stroke, size );  //turtle*/
      color_avg = (v[0] +v[1]+v[2])*.333333333;
      if ( complex ){
        if ( max_area < .3 ){  // was used at one time to suppress contrasting shapes in highly variable regions.
          if( rand_num() < .35 ){  // Change proportion of some strokes if num_pro's value > .4  //color_avg < .7 && 
            shape_bin = comp_shape( stroke, rand_num()*0.6 + 0.4 );
            stroke.type = 'd';
          }
         
       /* }else if( rand_num() < .1){
          shape_bin = comp_shape( stroke, rand_num()*0.6 + 0.4 );
          */}
      }
    }
  }
    ////end of size adjustments////

    //////// Bezier computations ////////
  if ( complex && BEZIER_ON && stroke.s[0] > sMall && stroke.type != 'z' ){
    stroke.p.x = x;
    stroke.p.y = y;
    diff = get_controlpts( stroke );
    if ( diff >= .35 ){//.56 ) {
      if ( !is_jpeg ){
        stroke.type = 'p';
      }else if (stroke.type != 'z' && stroke.type != 'd' ){
         stroke.type = 'p';
      }
       temp_count++;
    }
  }
  ////end of bezier computations////

  //////// Set variegated color. ////////
  // Default as middle of stroke color.

  stroke.rr[0] = stroke.r;  stroke.gg[0] = stroke.g;  stroke.bb[0] = stroke.b; 
  stroke.rr[1] = stroke.r;  stroke.gg[1] = stroke.g;  stroke.bb[1] = stroke.b;  
  if ( is_jpeg){

    
    //if( reg > - 1){
    //  if ( max_area > .3 &&  rand_num() < .5 ){
   //       set_variegated_color( stroke, x, y, gfact, gadd, factor, addend );
   //   }else if 
      if ( rand_num() <.9){
         set_variegated_color( stroke, x, y, gfact, gadd, factor, addend );
      }         
   // }
  }else{
       set_variegated_color( stroke, x, y, gfact, gadd, factor, addend );
  }


  // Choose a texture map
  int tex = rand_num()*(cnt_med - 1);
  stroke.tex = tex;


  return 1;
}                 // End method map_stroke

static void set_variegated_color( stroke_s& stroke, float x, float y, float gfact, float gadd, float factor,  float addend ){
  // Sets 2D arrays rr, gg, and bb based on the color on the left and right edges
  // Calculate x and y for left and right part of stroke.  
  float color; float grey;
  float dx, dy, x1, y1, x2, y2, theta;
  float rand = 0;
  float r = stroke.s[0]*0.25;
  if (complex ){
    if ( (is_jpeg && (v[0] + v[1] + v[2])/0.3333333 > 0.7) || !is_jpeg ){
      r = stroke.s[0]*5*0.125; // 5*0.125 = 5/8
    }   
  }
  
  if (is_iouca){
    dx = v[0];
    dy = v[1];
  }else{
    theta = stroke.rot*3.1415927/180;
    dx = r*cos(theta);
    dy = sqrt( ( r*r )-(dx*dx) );
  }
  
  x1 = x + dx;
  x2 = x - dx;
  if ( stroke.rot >= 0){
    y1 = y + dy;  
    y2 = y - dy;
  }else{
    y1 = y - dy;
    y2 = y + dy;
  }
  //Set left flank color 
  if ( x1 >= 0 && x1/PIX_W < s.w && y1 >=0 && y1/PIX_H < s.h && interpolate( x1, y1, v ) ){
    if (is_jpeg){
      stroke.rr[0] = v[0] + (rand_num()-.5)*.125;  stroke.gg[0] = v[1]+ (rand_num()-.5)*.125;  stroke.bb[0] = v[2]+ (rand_num()-.5)*.125;
    }else {
      color = get_value( num_col, v );
      grey = get_value( num_gre, v ); 
      if (is_jpeg && rand_num() <.9 ){
        if( complex){
          rand = 0.3*(rand_num() - 0.5 );
          factor = 1 + rand; // in this range: [.875, 1.125]
        }
         get_colour( gfact*grey + gadd , color*factor + addend, stroke.rr[0], stroke.gg[0], stroke.bb[0] );
      }else{
        get_colour( gfact*grey , color*factor , stroke.rr[0], stroke.gg[0], stroke.bb[0] );
      }
    }
  } //// else (x1,y1) is not valid point, use default middle point color.

    
  //Set right flank color
  if ( x2 >= 0 && x2/PIX_W < s.w && y2 >=0 && y2/PIX_H < s.h && interpolate( x2, y2, v ) ){
      if(is_jpeg){
        stroke.rr[1] = v[0] + (rand_num()-.5)*.125;  stroke.gg[1] = v[1] + (rand_num()-.5)*.125;  stroke.bb[1] = v[2] + (rand_num()-.5)*.125;
      }else{
         color = get_value( num_col, v );
         grey = get_value( num_gre, v ); 
         if( is_jpeg && rand_num() < .1 ){
           if( complex){
             rand = 0.3*(rand_num() - 0.5 );
             factor = 1 + rand; // in this range: [.875, 1.125]
           }  
           get_colour( gfact*grey, color*factor, stroke.rr[1], stroke.gg[1], stroke.bb[1] );
         }else{
           get_colour( gfact*grey + gadd, color*factor+addend, stroke.rr[1], stroke.gg[1], stroke.bb[1] );
         }
      }
  }// else (x2,y2) is not valid point, use default middle point color.
  
}

static int comp_shape( stroke_s& the_stroke, float value )

//  This private routine computes a starting stroke size for a
//  segment based on the average attribute value bound to coverage
//  
//  value:   attribute bound to coverage
{

//   value = ( value - amin[ attr_id ] ) / ( amax[ attr_id ] - amin[ attr_id ] );
//   s[ 0 ] = value;
//	s[ 1 ] = value/2;
//.s[ 0 ] and .s[ 1 ] used in paint.cpp in add_stroke as part of clipping process.  
//need to either replace with cp or vice versa when nurbs are drawn correctly
// *************************************************

  float fac;
  float step;
  float base;
  float rand = rand_num();
/*
  base = 3;
  step = 2;
  if ( value <= 0.333){
      the_stroke.s[0] = base;
      the_stroke.s[1] = base;
  }else if ( value <0.667 ){
    the_stroke.s[0] = base+4;
    the_stroke.s[1] = base+1;
    
  }else if ( value < .8 ){
    the_stroke.s[0] = base - 1;
    the_stroke.s[1] = base*14+2;
  }
  */
  base = 2;
  step = 2;

  if ( value <= 0.20){
    the_stroke.s[0] = base;
    the_stroke.s[1] = the_stroke.s[0] *4;
    return 0;
  }else if ( value <= 0.4 ){
    the_stroke.s[0] = base + step;
    the_stroke.s[1] = the_stroke.s[0] *4;
    return 1;
  }else if ( value <= 0.6){
    the_stroke.s[0] = base + 2*step; //6
    the_stroke.s[1] = base + 2*step; //6
    return 2;
  }else if ( value < 0.8 ){
    the_stroke.s[0] = base+14;  //14
    the_stroke.s[1] = base+8;  //6
    wide_count++;
    return 3;
  }else {
    the_stroke.s[0] = base + 1;
    the_stroke.s[1] = base*25 +2;
    long_count++;
    return 4;
  }

}


static void comp_size( stroke_s& the_stroke, float value )

//  This private routine computes a starting stroke size for a
//  segment based on the average attribute value bound to coverage
//  
//  value:   attribute bound to coverage
{

//   value = ( value - amin[ attr_id ] ) / ( amax[ attr_id ] - amin[ attr_id ] );
//   s[ 0 ] = value;
//	s[ 1 ] = value/2;
//.s[ 0 ] and .s[ 1 ] used in paint.cpp in add_stroke as part of clipping process.  
//need to either replace with cp or vice versa when nurbs are drawn correctly
// *************************************************

  float fac;
  float step;
  float base;
  float rand = rand_num();

  if ( complex ){
    base = 2;//PIX_W*.75;
    step = 4;//.5;
    if ( value <= 0.20){
      the_stroke.s[0] = base; 
    }else if ( value <= 0.4 ){
      the_stroke.s[0] = base + step;
    }else if ( value <= 0.6){
      the_stroke.s[0] = base + 2*step; 
    }else if ( value < 0.8 ){
      the_stroke.s[0] = base + 3*step;
    }else {
      the_stroke.s[0] = base + 4*step;   
    }

   the_stroke.s[1] = the_stroke.s[0] *4;
   
  } else{//is_jpeg || is_iouca){
    fac = 2;//.75;//0.25;  //PIX_W = 2
    step = 1;//.75;//3
    if ( value < 0.30 ) {
      the_stroke.s[ 0 ] = PIX_W*fac;
    } else if ( value < 0.4 ) {
      the_stroke.s[ 0 ] = PIX_W*fac + step;
    } else if ( value < 0.5 ) {
      the_stroke.s[ 0 ] = PIX_W*fac  + 2*step;
    } else if ( value < 0.6 ) {
      the_stroke.s[ 0 ] = PIX_W*fac + 3*step;
    } else {
      the_stroke.s[ 0 ] = PIX_W*fac + 4*step;
    }
    the_stroke.s[ 1 ] = the_stroke.s[ 0 ] * 4;
  }/*else{
    fac =  3; //2;
    if ( value < 0.20 ) {
      the_stroke.s[ 0 ] = PIX_W*fac;
    } else if ( value < 0.4 ) {
      the_stroke.s[ 0 ] = PIX_W*fac + 1;
    } else if ( value < 0.6 ) {
      the_stroke.s[ 0 ] = PIX_W*fac  + 2;
    } else if ( value < 0.8 ) {
      the_stroke.s[ 0 ] = PIX_W*fac + 3;
    } else {
      the_stroke.s[ 0 ] = PIX_W*fac + 4;
    }
  }*/
// *************************************************

}                  // End routine comp_size

void display_obj( int tex_num, float orientation )
{       
  float r;
	float theta;

  stroke_s stroke;

 /* if (LIGHTING_ON ){
    glEnable(GL_LIGHTING);
    glNormal3f( 0, 0, 1 );   
  }else{
    glDisable(GL_LIGHTING);
  } */

  stroke.s[0] = 2;  stroke.s[1] = 5;
  stroke.rot = 60;
  
  r = stroke.s[0]/4;

  stroke.p.x =  4;
  stroke.p.y =  2;
  
  float x1, y1, x2, y2;
  float dx = 0; 
  //theta = stroke.rot*3.1415927/180;
  //dx = r*cos(theta);
  
  float dy = sqrt( ( r*r )-(dx*dx) );
  
  x1 = stroke.p.x + dx;
  x2 = stroke.p.x - dx;
  if ( stroke.rot >= 0){
    y1 = stroke.p.y + dy;  
    y2 = stroke.p.y - dy;
  }else{
    y1 = stroke.p.x - dy;
    y2 = stroke.p.x + dy;
  }
  glPushMatrix();
  // bind textures for rendering stroke
  
  stroke.r = .98; stroke.g = .28; stroke.b = .17;
 // get_colour(.5,.5,stroke.r, stroke.g, stroke.b);
  stroke.rr[0] = 0.99607843;
  stroke.gg[0] = 0.30907908;
  stroke.bb[0] = 0.12629579;
  //stroke.rr[1] = 0.4; stroke.gg[1] = 0.8; stroke.bb[1] = 0.8;
  
  stroke.rr[1] = 0.99607843;
  stroke.gg[1] = 0.35453865;
  stroke.bb[1] = 0.23054175;
  
  stroke.rot = 90.0 + ( orientation * -90.0 );// + ( orientation * 90.0 );
  glRotatef(stroke.rot,0,0,1);
  glScalef(stroke.s[0], stroke.s[1], 1);
  start_stroke();
  if ( mode == 't' ){
    draw_stroke(1,1,1, tex_num);
  }else if ( mode == 'c'){
    draw_stroke(.1,.4,.5, tex_num );
  }else{
    draw_stroke(1,1,1, .3, tex_num );
  }
  stop_stroke();
  // draw_stroke(stroke.r,stroke.g, stroke.b, tex_num); //draw_3tone_stroke( stroke, 2);
  // }else {
  //draw_trapezoid_stroke(1,1,1,tex_num);  
  
  //}
  //draw_variegated_stroke(stroke, tex);
  // draw_stroke(.4,.8,.8, tex);
  
  glPopMatrix();
  /*
  glPushMatrix();
  start_stroke();
  glScalef(stroke.s[0], stroke.s[1], 1);
  stroke.r = 1-stroke.b; stroke.g = 1 - stroke.g; stroke.b = 1-stroke.r;
  get_colour(.8,.5,stroke.r, stroke.g, stroke.b);
  draw_3tone_stroke( stroke, 3);
  //draw_variegated_stroke(stroke, tex);
  stop_stroke();   // disable texture mapping
  glPopMatrix();
*/
/*   
 //Draw nurb strokes      
	get_colour(.05, color[ 0 ], color[ 1 ], color[ 2 ] ); 	
	glScalef( .125, .187, 1 );	  
       
	// initialize vars for drawing curve
	setup_nurbs( 4, 4,  1 );
  draw_nurbs(1, 1, 1 ); 
  //setup_nurbs( 7, 4, 1 );
  //draw_nurbs( color[ 0 ], color[ 1 ], color[ 2 ]);
*/
}


static void display_shell(GLenum mode){
  float canvas_dim;
  int num_regs;
  float r,g,b;
  float stretch;
  float max_dim;
  float white[3] = {1,1,1};
  
  
 /* if (LIGHTING_ON ){ 
    glNormal3f( 0, 0, 1 );   
  }*/

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  if (reset){
    scale[0] = 1; scale[1] = 1; scale[2] = 1;
    rotate[0] = 0; rotate[1] = 0; rotate[2] = 0;
    translate[0] = 0; translate[1] = 0; translate[2] = 0;
    jtrans[0] = 0; jtrans[1] = 0;
    jsc[0] = 1; jsc[1] = 1;
    reset = 0;
  }

 // printf( "clearcolors = (%f,%f,%f)\n", clearcolor[0] , clearcolor[1] , clearcolor[2]);
  glClear( GL_COLOR_BUFFER_BIT );//| GL_DEPTH_BUFFER_BIT);
  glFlush();
  //Translate entire scene
  glPushMatrix();
   
  glTranslatef(translate[0], translate[1], translate[2]);
  glRotatef(rotate[X_AXIS], 1, 0, 0);
  glRotatef(rotate[Y_AXIS], 0, 1, 0);
  glRotatef(rotate[Z_AXIS], 0, 0, 1);
 
  num_regs = vor->get_num_regions(fra);
  num_to_draw = num_valid_pts[fra] + ( num_regs - num_valid_pts[fra] );//*.4;

  if ( mode == GL_RENDER ){
    if(test){
      cout<<"Begin draw color attr"<<endl;
      draw_color_attr( 0 );
      cout<<"End draw color attr"<<endl;

    }else if( segmentation_type > -1){
      draw_curves();
      //draw_manip();
    }else {
      
    //   draw_segs();

    // num_to_draw = num_valid_pts[fra];
      if (mucha){
        //void draw_low_detail( bool textured, bool rectangular, int d_w, int d_h, int pix_w, int pix_h, float lowleft[ 2 ], float upright[ 2 ]  )
        canvas_dim = set_canvas_dim( vor, fra );
     //  draw_low_detail( 0, is_jpeg, s.w, s.h, PIX_W, PIX_H, lowleft[fra], upright[fra], vor, num_valid_pts[fra], fra    );
        draw_low_detail( 0, 1, s.w, s.h, PIX_W, PIX_H, lowleft[fra], upright[fra], vor, num_valid_pts[fra], fra    );
        draw_matte_regions();
        
        draw_mask( canvas_dim);
        
        draw_trilevel();
      }else if (complex) {
        //max_dim  = MAX(s.w,s.h);
        if ( fwidth/(s.w*PIX_W) < fheight/(s.h*PIX_H) ){
          stretch = ((float)fwidth)/((float)s.w*(float)PIX_W);
          glTranslatef(0, 0.5*((float)fheight - stretch*(float)s.h*(float)PIX_H), 0);
        }else{
          stretch = ((float)fheight)/((float)s.h*(float)PIX_H);
          glTranslatef(0.5*((float)fwidth - stretch*(float)s.w*(float)PIX_W), 0, 0);
        }
        glTranslatef( fwidth*0.5*(1-scale[0]), fheight*0.5*(1-scale[1]),0);

        glTranslatef( fwidth*(-jtrans[0]), fheight*(-jtrans[1]),0);

        glScalef(stretch,stretch, 1 );
        glScalef(scale[0],scale[1],1);
        glScalef( jsc[0], jsc[1], 1 );
        canvas_dim = set_canvas_dim( vor, fra );
        draw_canvas( 0, s.w, s.h, PIX_W, PIX_H, clearcolor  );
        draw_complex('c');

        if( LIGHTING_ON ){       
          bm.init_buffer( 'c' );     
          //bm.check_buffer('c', 500, 3);
         
          glClearColor(0,0,0,1);
          glClear( GL_COLOR_BUFFER_BIT );

          draw_complex('h');
          bm.init_buffer( 'h' );
    
          glClear( GL_COLOR_BUFFER_BIT );
         
          draw_canvas( 1, s.w, s.h, PIX_W, PIX_H, white  );
          draw_complex('t');
          bm.init_buffer( 't');
          
          glClearColor( clearcolor[ 0 ], clearcolor[ 1 ], clearcolor[ 2 ], 1 );
          glClear( GL_COLOR_BUFFER_BIT );
          glFlush();
        }
        glPopMatrix();
        
        if( LIGHTING_ON){
          bm.light_buffer();
        } 
              
      }else {
       //// zap canvas_dim = set_canvas_dim( vor, fra );
        //draw_low_detail( 1, is_jpeg, s.w, s.h, PIX_W, PIX_H, lowleft[fra], upright[fra], vor, num_valid_pts[fra], fra  );
      //// zap  draw_low_detail( 1, 1, s.w, s.h, PIX_W, PIX_H, lowleft[fra], upright[fra], vor, num_valid_pts[fra], fra  );
      //// zap  draw_mask( canvas_dim);
        draw_temp();
      }

      // draw_stars( PIX_W, PIX_H, fra, s.w, s.h);

      if (show_voronoi == 1){
          for (int i = 0; i < num_to_draw; i++ ){
            r = 1;  g = 0; b = 1;
            if ( i == 1200 || i == 1550 || i == 1790 ){
              g = 1;
            }
            vor->draw_web(  i, PIX_W, PIX_H,fra, r, g, b);
            vor->draw_center( i, PIX_W, PIX_H,fra, 20);
        } // end num_to_draw loop
      }

    }// end else segmentation_type < 0
  } else{ // mode == GL_SELECT, so perform picking render
      for (int i = 0; i < num_regs; i++ ){
        r = 1;  g = 0; b = 1;
        vor->draw_regs( i, PIX_W, PIX_H, fra, r,g,b, mode );
      } // end num_to_draw loop
  }
  if( !complex) {
    glPopMatrix();
  }
  

}  // end routine display_shell


static void draw_mask( float c ){
 // Draw polygonal voronoi regions as clearcolor where underlying data is invalid.  This will mask canvas areas that 
 // overlap into non-data areas.
 // 
 // c: canvas dimension used in draw_low_detail a frame around the data as wide as one canvas dimension covers any overlap 
//     the canvas may have with nondata zones
  int cardinality;
  int i, j;
  int num_regions;
  Point pt;

  glColor3f( clearcolor[0] , clearcolor[1] , clearcolor[2]);

  //Draw regions filled with clear color
  num_regions = vor->get_num_regions(fra);
  for ( i = num_valid_pts[fra]; i < num_regions; i++ ){    
    glBegin(GL_POLYGON);
      cardinality = vor->get_cardinality(fra,i);
      for (j = 0; j < cardinality; j++ ){  
        pt.x = vor->get_poly_pt(fra, i, j ).x;
        pt.y = vor->get_poly_pt(fra, i, j ).y;
        
        if (pt.x > -5 && pt.y > -5){  
          glVertex2f( pt.x*PIX_W, pt.y*PIX_H );	
        }
      }
     glEnd(); 
  }

  // Draw frame around data
  glBegin( GL_QUADS );
    glVertex3f( -c, -c, 0 );
    glVertex3f( s.w*PIX_W + c, -c, 0);
    glVertex3f( s.w*PIX_W + c, 0, 0 );
    glVertex3f( -c, 0, 0);
    
    glVertex3f( -c, s.h*PIX_H, 0 );
    glVertex3f( s.w*PIX_W + c, s.h*PIX_H, 0 );
    glVertex3f( s.w*PIX_W + c, s.h*PIX_H + c, 0 );
    glVertex3f( -c, s.h*PIX_H + c, 0 );

    glVertex3f( -c, 0, 0 );
    glVertex3f( 0, 0, 0);
    glVertex3f( 0, s.h*PIX_H, 0 );
    glVertex3f( -c, s.h*PIX_H, 0);

    glVertex3f( s.w*PIX_W, 0, 0 );
    glVertex3f( s.w*PIX_W + c, 0, 0);
    glVertex3f( s.w*PIX_W + c, s.h*PIX_H, 0 );
    glVertex3f( s.w*PIX_W, s.h*PIX_H, 0);
  glEnd();
  

}   // end routine draw_mask


static void draw_matte_regions(){
 // Draw polygonal voronoi regions categorized as 1,2, or 3, updating color at each vertex and smooth shading across the polygon
  float area;
  int cardinality;
  Point center;
  float* vc = new float [s.attr_n];
  float* v2 = new float [s.attr_n];
  int i, j;
  stroke_s my_st;
  stroke_s tmp_stroke;
  int num_regions;
  int ll [2];  int ur [2];
  Point pt1, pt2;
  //v = new float [s.attr_n];
  
 
 // cout<< " Maximum region index being drawn: "<<num_to_draw<<endl;
  //Draw regions filled as color at center
  num_regions = vor->get_num_regions(fra);
  for ( i = 0; i < num_to_draw; i++ ){
    area = vor->get_area(fra, i );
    center = vor->get_data_pt( fra, i );
    
    if(  (region_arr[fra][i] ==1 || region_arr[fra][i]==2 || region_arr[fra][i] == 3) && is_valid(center.x, center.y, 1,1,vc) ){         
        cardinality = vor->get_cardinality(fra,i);
        for (j = 0; j < cardinality; j++ ){  
          glBegin(GL_POLYGON);
            pt1.x = vor->get_poly_pt(fra, i, j ).x;
            pt1.y = vor->get_poly_pt(fra, i, j ).y;
            if ( j+1 < cardinality ){
              pt2.x = vor->get_poly_pt(fra, i, j+1).x;
              pt2.y = vor->get_poly_pt(fra, i, j+1).y;
            }else{
              pt2.x = vor->get_poly_pt(fra, i, 0).x;
              pt2.y = vor->get_poly_pt(fra, i, 0).y;
            }
            if ( is_valid(pt1.x, pt1.y, 1, 1, v) &&   is_valid(pt2.x, pt2.y, 1, 1, v2)){
              //set color for point pt1
              if (is_jpeg ){
                glColor3f( v[0], v[1], v[2] );
              }else{
                map_stroke(pt1.x*PIX_W,  pt1.y*PIX_H, tmp_stroke);
                glColor3f(tmp_stroke.r, tmp_stroke.g, tmp_stroke.b);
              }
              // draw point pt1
              glVertex2f( pt1.x*PIX_W, pt1.y*PIX_H );

              //set color for point pt2
              if (is_jpeg ){
                glColor3f( v2[0], v2[1], v2[2] );
              }else{
                map_stroke(pt2.x*PIX_W,  pt2.y*PIX_H, tmp_stroke);
                glColor3f(tmp_stroke.r, tmp_stroke.g, tmp_stroke.b);
              }
              // draw point pt2
              glVertex2f( pt2.x*PIX_W, pt2.y*PIX_H );

              
              //set color for point center
              if (is_jpeg ){
                glColor3f( vc[0], vc[1], vc[2] );
              }else{
                map_stroke(center.x*PIX_W,  center.y*PIX_H, tmp_stroke);
                glColor3f(tmp_stroke.r, tmp_stroke.g, tmp_stroke.b);
              }
              // draw point center
              glVertex2f( center.x*PIX_W, center.y*PIX_H );
           glEnd();
          }else {
        //   cout<<"Invalid point in draw_matte_regions, segment: "<<j<<"  "<<i<<"  "<<region_arr[fra][i]<<endl;
          }
        }
     
      
      
      
      
      /* 
      glBegin(GL_POLYGON);
        cardinality = vor->get_cardinality(fra,i);
        for (j = 0; j < cardinality; j++ ){  
          pt.x = vor->get_poly_pt(fra, i, j ).x;
          pt.y = vor->get_poly_pt(fra, i, j ).y;
          if ( is_valid(pt.x, pt.y, 1, 1, v) ){
            if (is_jpeg ){
              glColor3f( v[0], v[1], v[2] );
            }else{
              map_stroke(pt.x*PIX_W,  pt.y*PIX_H, tmp_stroke);
              glColor3f(tmp_stroke.r, tmp_stroke.g, tmp_stroke.b);
            }
            glVertex2f( pt.x*PIX_W, pt.y*PIX_H );	
          }else {
        //   cout<<"Invalid point in draw_matte_regions, segment: "<<j<<"  "<<i<<"  "<<region_arr[fra][i]<<endl;
          }
        }
      glEnd();
      */
/*
    pt = vor->find_com( fra, i );
    start_stroke();
    glPushMatrix();
	  glTranslatef(pt.x*PIX_W,pt.y*PIX_H,0);
    my_st.s[0] = vor->max_radius[fra]*PIX_W;
    glScalef( my_st.s[ 0 ],my_st.s[ 0 ], 1 );
    map_stroke(pt.x*PIX_W,  pt.y*PIX_H, tmp_stroke);
    draw_stroke( tmp_stroke.r, tmp_stroke.g, tmp_stroke.b, 0);
    glPopMatrix();
    stop_stroke();
    */


   } // end if 1, 2, or 3
  } // end of drawing regions filled with color
  
/*
  v = new float [s.attr_n];
  float clear_color[4] = {0, 0, .5, 1 };
  for (i = 0; i < s.h ; i++){
    for(j = 0; i < s.w; j++){
       if ( !is_valid(j, i, 1, 1, v) ){
         glPushMatrix();
         glTranslatef( j*PIX_W, i*PIX_H, 0 );
         glScalef(PIX_W, PIX_H, 1);
         draw_stroke( clear_color[0], clear_color[1], clear_color[2], clear_color[3]);
         glPopMatrix();
       }
    }
  }
 
  */
   //delete [] v;
   delete [] vc;
   delete [] v2;

/*
  num_regions = get_num_regions(fra);
  for ( i = num_valid_pts[fra]; i < num_regions; i++ ){
    
    // initialize region bounds
    ll[0] = s.w;  ll[1] = s.h;
    ur[0] = 0;  ur[0] = 0;
    cardinality = get_cardinality(fra,i);
    
    //get bounding box for region
    for( j = 0; j < cardinality; j++ ){
      pt = get_poly_pt(fra, i, j);
      if (pt.x < ll[0]){
        ll[0] = pt.x;
      }if (pt.y < ll[1]{
        pt.y = ll[1];
      }if (pt.x > ur[0]){
        ll[0] = pt.x;
      }if (pt.y > ur[1]{
        pt.y = ll[1];
      }
    }

    //check if any valid pts in region.
    if(
  }
*/
}

static void draw_complex(char mode){
  int i; int j;
  int index = 0;
  stroke_s tmp_stroke;
  tmp_stroke.r = 1;
  tmp_stroke.g = 1;
  tmp_stroke.b = 1;
  float dist;
  int tex; 

  //printf("number of strokes: %d\n", st_n[fra]);
  // draw detail areas
  for ( i = 0; i < st_n[fra]; i++){

    // Assign one of the 'regular' stroke textures as default
   // tex = choose_texture( sty_reg, cnt_reg + cnt_med );

    //Assign one of the indication stroke textures
    index = st[fra][i].seg_index;

    if (index > -1){
      
      start_stroke();
      glPushMatrix();
	    glTranslatef(st[fra][i].p.x,st[fra][i].p.y,0);

	    if (st[fra][i].rot > -900 ){
		    glRotatef( st[fra][i].rot, 0, 0, 1 );
	    }
      
      glScalef( st[fra][i].s[ 0 ],st[fra][i].s[ 1 ], 1 );
      /* if ( rand_num() < .5){
            BEZIER_ON = false;
      }*/

      if( mode == 'c' ){ 
       // tex = rand_num()*(cnt_med - 1);
       // st[fra][i].tex = tex;
        if (st[fra][i].tex%2 == 1 ){
          st[fra][i].tex--;
        }
        if (BEZIER_ON && st[fra][i].s[0] > sMall && st[fra][i].type == 'p'){
          draw_bezier( st[fra][i], st[fra][i].tex );
        }else{
          draw_3tone_stroke( st[fra][i], st[fra][i].tex );//draw_stroke(st[fra][i].r, st[fra][i].g, st[fra][i].b, st[fra][i].tex);//
        }
      }else if ( mode == 'h' ){
        if ( BEZIER_ON && st[fra][i].type == 'p'){
          tmp_stroke.r = st[fra][i].r;   tmp_stroke.g = st[fra][i].g;   tmp_stroke.b = st[fra][i].b;
          st[fra][i].r = 1; st[fra][i].g = 1; st[fra][i].b = 1;
          draw_bezier(st[fra][i], .3, st[fra][i].tex );
          st[fra][i].r = tmp_stroke.r; st[fra][i].g = tmp_stroke.g;  st[fra][i].b = tmp_stroke.b;
        }else{
          draw_stroke( 1,1,1, .3, st[fra][i].tex);
        }

      }else if(mode == 't'){
        if ( BEZIER_ON && st[fra][i].type == 'p' ){
          tmp_stroke.r = st[fra][i].r;   tmp_stroke.g = st[fra][i].g;   tmp_stroke.b = st[fra][i].b;
          //st[fra][i].r = .7; st[fra][i].g = .7; st[fra][i].b = .7; //for creating heightfield image for ppt 
		  st[fra][i].r = 1; st[fra][i].g = 1; st[fra][i].b = 1; 
          draw_bezier(st[fra][i], st[fra][i].tex + 1 );
          st[fra][i].r = tmp_stroke.r; st[fra][i].g = tmp_stroke.g;  st[fra][i].b = tmp_stroke.b;
        }else{
          draw_stroke( 1,1,1, st[fra][i].tex + 1);
        }
      }else {
        draw_stroke( st[fra][i].r, st[fra][i].g, st[fra][i].b, st[fra][i].tex);
      }
      glPopMatrix();
      stop_stroke();
      
    }
    BEZIER_ON = true;
  }// end of drawing detail areas
}  // end routine draw_complex

static void draw_trilevel(){
  int draw = 1;
  int i; int j;
  int index = 0;
  stroke_s tmp_stroke;
  int count = 0;
  int tex; 
/*
  //Draw test canvas patches
  start_stroke();
  tex = 0;

  map_stroke(400,  700, tmp_stroke);
  glTranslatef( 350, 650, 0);
  glScalef( 100, 100, 1);
  //draw_stroke( 0, 0, 0.5, tex); //dark blue
//  draw_stroke( tmp_stroke.r, tmp_stroke.g, tmp_stroke.b, tex);

  glScalef( .01, .01, 1);
  tex = 1;

  map_stroke(400,  800, tmp_stroke);
  glTranslatef( 0, 100,0);
  glScalef(100,100,1);
  //draw_stroke(0, 0, 0.5, tex);
// draw_stroke( tmp_stroke.r, tmp_stroke.g, tmp_stroke.b, tex);

  glScalef( .0100, .0100, 1);

  glTranslatef( -350, -750,0);

  stop_stroke(); 
*/
  float dist;
  float ran;
  // draw detail areas
  for ( i = 0; i < st_n[fra]; i++){
    draw = 1;
    ran = rand_num();

    if( st[fra][i].type == 'c' && !show_corner_strokes ){
      draw = 0;
    }
    // Assign one of the 'regular' stroke textures as default
    
    
    tex = choose_texture( sty_reg, cnt_reg );
    
    if ( index == 0 ){
      dist = (st[fra][i].rr[0]-st[fra][i].rr[1])*(st[fra][i].rr[0]-st[fra][i].rr[1]) + (st[fra][i].gg[0]-st[fra][i].gg[1])*(st[fra][i].gg[0]-st[fra][i].gg[1]) + (st[fra][i].bb[0]-st[fra][i].bb[1])*(st[fra][i].bb[0]-st[fra][i].bb[1]);
      if ( dist > .001 ){
        ran = rand_num();
        if (ran > 0.5) {
          tex = 0;
        }else{
           tex = 1; 
        }
      }
    }
    //Assign one of the indication stroke textures
    index = st[fra][i].seg_index;

    if (index > -1){
      if( index == 1 ){
        tex = choose_texture( sty_low, cnt_low );
      }else if ( index == 2){
        tex = choose_texture( sty_low, cnt_low );
      }else if (index == 3){
        tex = choose_texture (sty_med, cnt_med);   
      }
      
      if(brush_texture == ON ){ //&& index != 4 && index != 5 ){
        start_stroke();
      }
      
      if( draw && index != 5 && index != 4){ 
        glPushMatrix();
	      glTranslatef(st[fra][i].p.x,st[fra][i].p.y,0);

	      if (st[fra][i].rot > -900 ){
		      glRotatef( st[fra][i].rot, 0, 0, 1 );
	      }
	      glScalef( st[fra][i].s[ 0 ],st[fra][i].s[ 1 ], 1 );
      //   cout<<"stroke x, y = "<<st[fra][i].p.x<<" "<<st[fra][i].p.y<<"  "<<i<<"  "<<st[fra][i].r<<"  "<<st[fra][i].g<<"  "<<st[fra][i].b<<" "<<endl;
        if (index == 0){
          draw_variegated_stroke( st[fra][i], tex);     
        }else{
          if ( index == 1 || index == 2 ){
            if (rand_num() < 1.1) { //0.8 ){
              draw_stroke( st[fra][i].r, st[fra][i].g, st[fra][i].b, tex);
            }
          }else{
              draw_stroke( st[fra][i].r, st[fra][i].g, st[fra][i].b, tex);
          }
        }
      		count++;
        glPopMatrix();
     // }
      }
      
        if ( brush_texture == ON ){//&& index != 4 && index != 6){
          stop_stroke();
        }

        if ( 0 ){//{index == 5 ){//|| index == 4 ){  // clearcolored strokes for mixed regions (untextured)
          glPushMatrix();
	        glTranslatef(st[fra][i].p.x,st[fra][i].p.y,0);
          // HACK HACK 
          if ( index == 4 ){
            glScalef( st[fra][i].s[ 0 ] + (st[fra][i].s[ 0 ]),  st[fra][i].s[ 1 ], 1 );
          }else{
            glScalef( st[fra][i].s[ 0 ],  st[fra][i].s[ 1 ], 1 );
          }
          //draw_stroke( st[fra][i].r, st[fra][i].g, st[fra][i].b, tex);
          draw_stroke_no_tex( st[fra][i].r, st[fra][i].g, st[fra][i].b);
          count++;
          glPopMatrix();
        }
    }
  }// end of drawing detail areas
}  // end routine draw_trilevel


static void draw_temp(){
  int draw = 1;
  int i; int j;
  int index = 0;
  stroke_s tmp_stroke;
  int count = 0;
  int tex; 

  float ran;
  // draw detail areas
  for ( i = 0; i < st_n[fra]; i++){
    draw = 1;
    ran = rand_num();
   
    if( st[fra][i].type == 'c' && !show_corner_strokes ){
      draw = 0;
    }
    // Assign one of the 'regular' stroke textures as default
    
    
    tex = choose_texture( sty_reg, cnt_reg + cnt_med );

    //Assign one of the indication stroke textures
    index = st[fra][i].seg_index;

    if (index > -1){
      if ( index  == 1 || index == 2 ){
        tex = choose_texture( sty_low, cnt_low );
     /* if( index == 1 ){
        tex = choose_texture( sty_lowlow, cnt_lowlow );
      }else if ( index == 2){
        tex = choose_texture( sty_low, cnt_low );
        */
      }else if (index == 3){
        tex = choose_texture (sty_reg, cnt_med + cnt_reg);   //(sty_med, cnt_med); 
        //st[fra][i].r =1; st[fra][i].g = 1; st[fra][i].b = 1;
      }
      
      if ( i == largest){
    //    tex = 0;
      }
      
	    if (st[fra][i].rot < -900 ){
			    brush_texture = OFF;
	    }
      if(brush_texture == ON ){ //&& index != 4 && index != 5 ){
        start_stroke();
      }
      
      if( draw && index!= 4 && index != 5 ){  
        glPushMatrix();
	      glTranslatef(st[fra][i].p.x,st[fra][i].p.y,0);

	      if (st[fra][i].rot > -900 ){
		      glRotatef( st[fra][i].rot, 0, 0, 1 );
	      }
	     glScalef( st[fra][i].s[ 0 ],st[fra][i].s[ 1 ], 1 ); ////zap

      //   cout<<"stroke x, y = "<<st[fra][i].p.x<<" "<<st[fra][i].p.y<<"  "<<i<<"  "<<st[fra][i].r<<"  "<<st[fra][i].g<<"  "<<st[fra][i].b<<" "<<endl;
        if ( index == 1 || index == 2 ){
         // if (rand_num() < 1.1) {//.8){
            draw_stroke( st[fra][i].r, st[fra][i].g, st[fra][i].b, tex);
          ///// zap draw_bezier(  st[fra][i], tex);
         // }
        }else{
           draw_stroke( st[fra][i].r, st[fra][i].g, st[fra][i].b, tex);
          // zap draw_bezier( st[fra][i], tex);
        }
      		count++;
        glPopMatrix();
     // }
      }
      
        if ( brush_texture == ON ){//&& index != 4 && index != 6){
        stop_stroke();
	    }	
	    if (st[fra][i].rot < -900 ){
			    brush_texture = ON;
      }
    }
  }// end of drawing detail areas
}

static int choose_texture(int style, int numb){
  float ran;
  float top;
  bool found = 0;
  ran = rand_num();  
  int tex = numb - 1;  //default to highest tex index
  float part = 1.0/(float)numb;
  for ( int i = 0; i < numb && !found; i++ ){
    top = part + (float)i*part;
    if ( ran < top ){
      tex = i;
      found = 1;
    }
  }

  tex = tex + style;
  return tex;
}

static void draw_sized_regions(){
  int count = 0;
  int i, j;
  int index;
  float max_area = 0;
  float r,g,b;
  int tmp_reg;

  // Draw detail levels as blocks of color, red, bluegreen, blue
  for ( i = 0; i < s.h*PIX_H ; i++ ){
	  for (j = 0; j < s.w*PIX_W ; j++ ){
	    index = id[ i ][ j ];
  	 if(index <= seg_to_draw ){
	    if ( max_area >=0){ // if negative value, don't draw
      // if (j >= 52 && j <=55 && i>=41 && i <= 44){
        // tmp_reg = vor->find_centroid(fra, j, i );
        if (index == s.n - 2){ 
          //if (j >= 208 && j <=211 && i>=164 && i <= 167){
          //outty<<j <<"  "<<i<<endl;
          r = .5;
          g = .5;
          b = 0;
        }
 
       
        /* }else if( is_boundary( j , i, id, tmp_reg, v ) ){
          count++;	
          r = 1.0;
          g = 0;
			    b = 0;
		    }else*/
        
        else if( max_area < threshold ){
          
          r = 0.0;
          g = 1;
			    b = 1;
        }else{
			    r = 0;
          g = 0;
			    b = 1;
        }
		    glColor3f(r,g,b);
        glTranslatef(j,i,0);
        draw_stroke( r,g,b);
        glTranslatef(-j,-i,0);
      }//endif max_area >= 0
     }
    }
  }
//	} //end if < segs_to_draw
}  //end routine draw_sized_regions

static void draw_test(){
  glColor3f(1,1,1); //white
  glutSolidCube(40);
}

static void draw_segs (){
// This routine is for debugging.  It shows the segments as distinct colors.
  
  // The voronoi segmenting was decided as follows 
  // 0 if the current point is in a small region which does not border a large region
  // 1 if the current point is in a large region which does not border a small region
  // 2 if the current point is in a large region which borders a small region
  // 3 if the current point is in a small region which borders a large region
  // 4 if the current point is valid but its region has invalid points and it does not border a small region.
  // 5 if the current point is invalid, but its region contains valid points
  // 6 if the current point is in an entirely invalid region.

  int i, j;         // counters
  int index = 0;    // segment id holder
  float r,g,b;      // colors
  
  //int reg; float area;
  //int floori; int floorj;
  for ( i = 0; i < s.h*PIX_H ; i++ ){
 //for ( i = 500; i < 1000 ; i++ ){
  // floori = floor((float)i/(float)PIX_H);
    for (j = 0; j < s.w*PIX_W ; j++ ){
  //  for (j = 500; j < 1000 ; j++ ){
			index = id[ i ][ j ];
	    // if(index <= seg_to_draw ){
      get_colour( (float)index/(float)s.n*.5 +.5, r, g, b );

      /* floorj = floor((float)j/(float)PIX_W);
      reg = vor->find_centroid(fra, floorj, floori );
      area = vor->get_area(fra, tmp_reg );
      */ 



      if ( index == 4 ){
        r = 1;
        g = 0;
        b = 0;
      } else if ( index == 5 ){
        r = 0;
        g = 1;
        b = 0;
      } else if ( index == 6 ){
        r = 0;
        g = 0;
        b = 1;
      }
     start_stroke();
     glPushMatrix(); 
     glTranslatef(j,i,0);
      
     glScalef(PIX_W, PIX_H, 1);
     draw_stroke( r,g,b, 1);
      
      //  cout<<"r = "<<r<<" g = "<<g<<" b = "<< b<<" j = "<<j<<" i = "<<i<<endl;
      
     glPopMatrix();
     stop_stroke();
    //}

    }
  }
}// end routine draw_segs

static void get_region_bbox( int region_id, float ll [] , float ur [] ){
  // This routine check's the bounding polygon's vertices for a region in order to find the outer 
  // extremes of the region.
 
  int cardinality;
  int j;
  float corner_x; float corner_y;

  // Initialize bounding box to amax and amin
  ll[0] = s.w;  ll[1] = s.h;
  ur[0] = 0;    ur[1] = 0;

  cardinality = vor->get_cardinality(fra,region_id);
  for (j = 0; j < cardinality; j++ ){  
    // Get x value of current polygon vertex.
    corner_x = vor->get_poly_pt(fra, region_id, j ).x;
    
    if ( corner_x < ll[0] ){
      ll[0] = corner_x;
    }
    if ( corner_x > ur[0] ){
      ur[0] = corner_x;
    }

    // Get y value of current polygon vertex.
    corner_y = vor->get_poly_pt(fra, region_id, j ).y;
    if ( corner_y < ll[1] ){
      ll[1] = corner_y;
    }
    if ( corner_y > ur[1] ){
      ur[1] = corner_y;
    }
  } // end j loop.
}

static int is_boundary ( int x, int y, int size, int region_id, float* val){
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
  float nb_reg_area;          // neighbor's region's area
  int nb_reg_id;              // neighbor's region's id
  int rc;

  int    off[ 8 ][ 2 ] = {		// Offset of each neighbour
    { -1,  1 }, {  0,  1 }, {  1,  1 },
    { -1,  0 },             {  1,  0 },
    { -1, -1 }, {  0, -1 }, {  1, -1 }
  };

  rc = 0;

  cardinality = vor->get_cardinality(fra,region_id);
  for (j = 0; j < cardinality; j++ ){  
    corner_x = vor->get_poly_pt(fra, region_id, j ).x;
    corner_y = vor->get_poly_pt(fra, region_id, j ).y;
    for( i = 0; i < 8; i++ ) {		// For all neighbours
	    cur_x = corner_x - off[ i ][ 0 ]; // set cur_x to next neighbor's x position 
      cur_y = corner_y - off[ i ][ 1 ];
      
	    // if outside of dataset boundaries
      if ( cur_x < 0 || cur_x >= s.w  || cur_y < 0 || cur_y >= s.h ){
        continue;
      }
    
      nb_reg_id = vor->find_centroid(fra, cur_x, cur_y );
      
      if ( nb_reg_id == region_id ){  // lives in my region. I'm not interested.
        continue;
      }
      if ( nb_reg_id > num_valid_pts[fra] ){  // region has invalid center. Area unknown, so can't be used as bounding region.
        continue;
      }
      nb_reg_area = vor->get_area(fra, nb_reg_id );

	  if ( size == 0 ){                            // size is small
        if( nb_reg_area >= threshold ) {
		      return 1;
        }
      }else{                   // size is large
        if( nb_reg_area < threshold ) {
          return 1;
        } //end else
      }
    }// end i loop 
  } // end j loop

  return rc;

}   // end routine is_boundary


 void position_camera(){
    float zdist;
    float data_w;
    float data_h;

 
  if ( strcmp( newfile, "" ) == 0 || !file_open ) {
      //Default dimensions before file is open
			data_w = 500;
      data_h = 500;
      zdist = 700;
      //gluOrtho2D(-data_w*PIX_W*.5, data_w*PIX_W*.5, -data_h*PIX_H*.5, data_h*PIX_H*.5);
      //gluOrtho2D(0, data_w*PIX_W, 0, data_h*PIX_H);
      // gluOrtho2D(-data_w*PIX_W*.5, data_w*PIX_W*.5, -data_h*PIX_H*.5, data_h*PIX_H*.5);
  } else {
     data_w = s.w;  //moraine_w = 416  south_america_w= 94;   //asia_w = 251;  //north_america_w = 116;
     data_h = s.h;  //moraine_h = 312  south_america_h = 137; //asia_h = 184;  //north_america_h = 52; 
   
  }
    
    //Look at takes triplets for each of: where camera is placed, where it is aimed, which way is up
    if ( data_w > data_h ){
      zdist = (0.5*data_w*PIX_W)/EYE_Z_POS + (5 * PIX_W);
    }else {
      zdist = (0.5*data_h*PIX_H)/EYE_Z_POS + (5 * PIX_H);
    }
    gluLookAt(  data_w*PIX_W*.5, data_h*PIX_H*.5, zdist,   data_w*PIX_W*.5, data_h*PIX_H*.5, -1,   0, 1, 0   );  
    //  for gluOrtho gluLookAt( data_w*PIX_W*.5, data_h*PIX_H*.5, 1 ,data_w*PIX_W*.5, data_h*PIX_H*.5, 0, 0, 1, 0 ); 
 }

void pick( float x, float y ){
  
  GLuint selectBuf[512];
  int hits;
  int viewport[4];

  glGetIntegerv(GL_VIEWPORT, viewport);

  glSelectBuffer(512, selectBuf);
  glRenderMode(GL_SELECT);

  glInitNames();
  glPushName(0);

  glMatrixMode( GL_PROJECTION );
  glPushMatrix();
  glLoadIdentity();
  
  //Create a 5x5 pixel picking region near cursor location
  gluPickMatrix( 
    (GLdouble)x, (GLdouble)(viewport[3] - y), 5.0, 5.0, viewport );
  
  if (complex ){
    glOrtho( 0, fwidth, 0, fheight, -1, 1000);
  }else{
    gluPerspective( Y_FOVY, ASPECT_RATIO, 1, 10000);
    position_camera();
  }

  display_shell( GL_SELECT );

  glMatrixMode( GL_PROJECTION );
  glPopMatrix();
  glFlush();

  hits = glRenderMode( GL_RENDER );
  process_hits( hits, selectBuf );
  
  
}   //end routine pick


void process_hits( int hits, GLuint buffer [] ){
  int i, j, k;
  float area, avg_area, max_area;
  int card;
  int* neigh;
  int reg_n;
  Point pt;
  
  cout<<"# of hits = "<<hits<<endl;

  GLuint names, *ptr;
  ptr = (GLuint *) buffer;
  
  for( i = 0; i < hits; i++ ){
    names = *ptr;
    cout<<"The number of names for this hit = "<<names<<endl;
    ptr++;
    // Print amin and amax z values for the hits found at this point
    printf(" z1 is %g;", (float) *ptr/0xffffffff); 
    ptr++;   
    printf(" z2 is %g\n", (float) *ptr/0xffffffff); 
    ptr++;
    for( j = 0; j < names; j++ ){
      reg_n = *ptr;
      pt = vor->get_data_pt(fra, reg_n);
      
      printf("\n *************** begin region %d\n", reg_n);
      printf("Region number = %d; center = (%f,%f)\n", reg_n, pt.x, pt.y );      
      card = vor->get_cardinality( fra, reg_n );
      
      printf("\nPoly has %d points:\n", card );
      for ( k = 0; k < card; k++ ){
        pt = vor->get_poly_pt( fra, reg_n, k );
        printf("p[%d] = (%.2f,%.2f);  ", k, pt.x, pt.y);
        if ( k > 0 && (k+1)%2 == 0 ){
          printf("\n");
        }
      }
      area = vor->get_area( fra, reg_n);
      vor->find_local_area( fra, reg_n, avg_area, max_area );
      printf("\n\nArea = %f; Avg area = %f; Max area = %f",area, avg_area, max_area);
      neigh = new int [ card ];
      vor->find_neighbor_regions( fra, reg_n, card, neigh );
      printf("\n\n\nNeighbor region numbers: \n");
      for ( k = 0; k < card; k++ ){
        pt = vor->get_poly_pt( fra, reg_n, k );
        printf("n[%d] = %d;  ", k, neigh[k]);
        if ( k > 0 && (k+1)%2 == 0 ){
          printf("\n");
        }
      }
      printf("\n *************** end region %d\n\n", reg_n);
      delete [] neigh;
      ptr++;
    }
  }
}  //process_hits

static int sort_split_regions( int x, int y, int region_id, float* valu){
  // This routine identifies boundary regions ( regions bourdaring regions in a different size category.  
  // Sets seg[y][x] to current region id  if the current region is NOT yet identified as a boundary but has a valid center
  // Returns 0 if the current position neighbors a small region with id of 0
  // Returns 3 if the current position neighbors a small region with id of 3
  // Returns 4 if the current region has an invalid center but also contains valid points and has no small neighboring regions
 
  //
  // This routine should only be called if segmentation_type == -1  

  int i;                      // counter

  int cur_x; int cur_y;     // current point ( for looping through neighbors )
  int nb_reg_id;            // neighbor's region's id
  int nb_seg_id;            // id of the segment to which the neighbor belongs

  int    off[ 8 ][ 2 ] = {		// Offset of each neighbour
    { -1,  1 }, {  0,  1 }, {  1,  1 },
    { -1,  0 },             {  1,  0 },
    { -1, -1 }, {  0, -1 }, {  1, -1 }
  };

  for( i = 0; i < 8; i++ ) {		// For all neighbours
	  cur_x = x - off[ i ][ 0 ]; // set cur_x to next neighbor's x position 
    cur_y = y - off[ i ][ 1 ];
    
	  // if outside of dataset boundaries
    if ( cur_x < 0 || cur_x >= s.w  || cur_y < 0 || cur_y >= s.h ){
      continue;
    }
    
    nb_seg_id = s.seg[ cur_y ][ cur_x ];
    if ( nb_seg_id == -1 ){
      nb_reg_id = vor->find_centroid(fra, cur_x, cur_y );
      nb_seg_id = region_arr[ fra ][ nb_reg_id ];
    }
    
    if ( nb_seg_id == 0 || nb_seg_id == 3){
      return nb_seg_id;
    } else {
      nb_reg_id = vor->find_centroid(fra, cur_x, cur_y );
      if ( nb_reg_id == region_id && nb_seg_id != 7) {
        s.seg[y][x] = 7;
        return sort_split_regions( cur_x, cur_y, nb_reg_id, valu );
      }
    }
  }

  return 4; //return region_arr[fra][region_id];

}   // end routine sort_split_regions


static void draw_manip()

//  GLUT callback to handle canvas display
{
  int    i; int j;                 // Loop counter
  stroke_s the_stroke;

	for ( i = 0; i < st_n[fra]; i++){
    
		if (st[fra][i].rot < -900 ){
				brush_texture = OFF;
				//			 cout<<"stroke x, y = "<<st[fra][i].p.x<<" "<<st[fra][i].p.y<<endl;
		}
		if(brush_texture == ON ){
      start_stroke();
    }

		glPushMatrix();

		glTranslatef(st[fra][i].p.x,st[fra][i].p.y,0);
		//outfile<<"the rotation is: "<<the_stroke.rot<<", "<<j<<", and "<<i<<endl;
		if (st[fra][i].rot > -900 ){
			glRotatef( st[fra][i].rot, 0, 0, 1 );
		}
		glScalef( st[fra][i].s[ 0 ],st[fra][i].s[ 1 ], 1 );
		draw_stroke( st[fra][i].r, st[fra][i].g, st[fra][i].b );
			
		glPopMatrix();
		
		if ( brush_texture == ON ){
      stop_stroke();
		}	
		if (st[fra][i].rot < -900 ){
				brush_texture = ON;
		}
	}
}                 // End routine draw_manip


static void draw_manip_old()

//  GLUT callback to handle canvas display
{
  int    i; int j;                 // Loop counter
  stroke_s the_stroke;

//difference from curved code
	  glColor3f(1,1,1); //white
//	glutSolidCube(40);

/*

  for ( i = 0; i < s.h*PIX_H; i++ ){
	for (j = 0; j < s.w*PIX_W; j++ ){
		
		cout<<"####### got to beginning of manip "<<j<<" "<<i<<endl;
				position = get_current_position( j, i );
				 	cout<<"after get_current_pos )))))))))))manip "<<position<<endl;
	      gf.get( position,fra, v );
				map_mat(j,i,the_stroke);
        
		//	 cout<<"value = "<<value<<endl;
				
				glColor3f(the_stroke.r,the_stroke.g,the_stroke.b);
        glTranslatef(j,i,0);
				glScalef( the_stroke.s[ 0 ],the_stroke.s[ 1 ], 1 );
        draw_stroke(the_stroke.r,the_stroke.g,the_stroke.b);
				glTranslatef(-j,-i,0);
		//	 cout<<"j,i = "<<j<<" "<<i<<endl;
		// }
	}
	}
*/
//    glScalef( zoom, zoom, 1 );
/*
for ( i = 0; i < gf.rows(fra); i++){
	for ( j = 0; j < gf.cols(fra); j++){
		if (map_stroke(j*PIX_H,i*PIX_W,the_stroke)){
			glPushMatrix();
		//	glTranslatef(the_stroke.p.x, the_stroke.p.y, 0 );
			//outfile<<"the_stroke.p.x, the_stroke.p.y = "<<the_stroke.p.x<<", "<< the_stroke.p.y<<endl;
			//outfile<<"j, i = "<<j<<" , "<<i<<endl;
			glTranslatef(j*PIX_H,i*PIX_W,0);
			//outfile<<"the rotation is: "<<the_stroke.rot<<", "<<j<<", and "<<i<<endl;
			glRotatef( the_stroke.rot, 0, 0, 1 );
			glScalef( the_stroke.s[ 0 ], the_stroke.s[ 1 ], 1 );
			draw_stroke( the_stroke.r, the_stroke.g, the_stroke.b );
			

			glPopMatrix();
		}
		
}
}
*/
// difference from curved code
	for ( i = 0; i < st_n[fra]; i++){
    
		if (st[fra][i].rot < -900 ){
				brush_texture = OFF;
				//			 cout<<"stroke x, y = "<<st[fra][i].p.x<<" "<<st[fra][i].p.y<<endl;
		}
		if(brush_texture == ON ){
      start_stroke();
    }

//		for ( i = 0; i < 3; i++ ) {
		glPushMatrix();
//bookmark
		glTranslatef(st[fra][i].p.x,st[fra][i].p.y,0);
		//outfile<<"the rotation is: "<<the_stroke.rot<<", "<<j<<", and "<<i<<endl;
		if (st[fra][i].rot > -900 ){
			glRotatef( st[fra][i].rot, 0, 0, 1 );
		}
		glScalef( st[fra][i].s[ 0 ],st[fra][i].s[ 1 ], 1 );
		draw_stroke( st[fra][i].r, st[fra][i].g, st[fra][i].b );
			
		glPopMatrix();
		
		if ( brush_texture == ON ){
      stop_stroke();
		}	
		if (st[fra][i].rot < -900 ){
				brush_texture = ON;
		}
	}
		

  //draw the underpainting 
/*
  for( i = 0; i < back_st_n[fra]; i++ ) {
  //for ( i = 0; i < 5; i++ ) {
      if (brush_texture == ON ) {
          start_stroke();
      }
	cerr<<"main.cpp:  draw_manip() i = "<<i <<endl;
      glPushMatrix();
      glTranslatef( back_st[fra][ i ].p.x, back_st[fra][ i ].p.y, 0 );
      glRotatef( back_st[fra][ i ].rot, 0, 0, 1 );
      // glScalef( back_st[fra][ i ].s[ 0 ], back_st[fra][ i ].s[ 1 ], 1 );
      //  setup_nurbs(7,4,.1);
	cerr<<" I got here before setup_nurbs"<<endl;
      setup_nurbs( back_st[fra][ i ].cp[ 0 ], back_st[fra][ i ].cp[ 1 ], 0.1 );    
	cerr<<" I got here after setup_nurbs"<<endl;
      draw_nurbs( .74, .87, 1 );
	cerr<<" I got here before draw_stroke"<<endl;
      //draw_stroke( .74, .87, 1 );  //hardcoded underpainting color
//		cerr<<" I got here after draw_stroke"<<endl;
      glPopMatrix();
      if (brush_texture == ON ) {
          stop_stroke();
      }
  }*/

/*  //draw the overpainting 
// for( i = 0; i < st_n[fra]; i++ ) {
  for( i = 0; i < 40; i++ ) {
  if(brush_texture == ON )
  {
    start_stroke();
  }
      
  glPushMatrix();
  glTranslatef( st[fra][ i ].p.x, st[fra][ i ].p.y, 0 );
  glRotatef( st[fra][ i ].rot, 0, 0, 1 );
  // glScalef( st[fra][ i ].s[ 0 ], st[fra][ i ].s[ 1 ], 1 );
  //setup_nurbs(7,4,0.1);
  //setup_nurbs( st[fra][ i ].cp[ 0 ], st[fra][ i ].cp[ 1 ] , 0.1 );	
  //draw_nurbs( st[fra][ i ].r, st[fra][ i ].g, st[fra][ i ].b );
  draw_stroke( st[fra][ i ].r, st[fra][ i ].g, st[fra][ i ].b );
  glPopMatrix();
  if(brush_texture == ON )
  {
    stop_stroke();
  }
}
*/
/*
//draw the highlight strokes
for( i = 0; i < high_st_n[fra]; i++ ) {
  if ( high_st[fra][i].r != -1 ){  //Is this the problem?????????????????
    if(brush_texture == ON )
  {
    start_stroke();
  }			
    glPushMatrix();
    glTranslatef( high_st[fra][i].p.x, high_st[fra][i].p.y, 0 );
    glRotatef( high_st[fra][i].rot, 0, 0, 1 );
    // glScalef( high_st[fra][i].s[0], high_st[fra][i].s[1], 1 );
    //setup_nurbs(7,4,0.1);
    setup_nurbs( high_st[fra][i].cp[ 0 ], high_st[fra][i].cp[ 1 ], 0.1 );	
    draw_nurbs( high_st[fra][i].r, high_st[fra][i].g, high_st[fra][i].b );
    //draw_stroke( 1, 1, 0 );
    //draw_stroke( high_st[fra][i].r, high_st[fra][i].g, high_st[fra][i].b );
    glPopMatrix();
    if(brush_texture == ON )
  {
    stop_stroke();
  }
  }  
}
*/
//   glPopMatrix(); 

}                 // End routine draw_manip_old


static void draw_curves()

//  GLUT callback to handle canvas display
{
  int    i; int j;                 // Loop counter
  stroke_s the_stroke;

//difference from curved code
/*
  for ( i = 0; i < s.h*PIX_H; i++ ){
	for (j = 0; j < s.w*PIX_W; j++ ){
		
				position = get_current_position( j, i );
	      gf.get( position,fra, v );
				map_mat(j,i,the_stroke);
        
		//	 cout<<"value = "<<value<<endl;
				
				glColor3f(the_stroke.r,the_stroke.g,the_stroke.b);
        glTranslatef(j,i,0);
				glScalef( the_stroke.s[ 0 ],the_stroke.s[ 1 ], 1 );
        draw_stroke(the_stroke.r,the_stroke.g,the_stroke.b);
				glTranslatef(-j,-i,0);
		//	 cout<<"j,i = "<<j<<" "<<i<<endl;
		// }
	}
	}
*/
  
//    glScalef( zoom, zoom, 1 );
/*
for ( i = 0; i < gf.rows(fra); i++){
	for ( j = 0; j < gf.cols(fra); j++){
		if (map_stroke(j*PIX_H,i*PIX_W,the_stroke)){
			glPushMatrix();
		//	glTranslatef(the_stroke.p.x, the_stroke.p.y, 0 );
			//outfile<<"the_stroke.p.x, the_stroke.p.y = "<<the_stroke.p.x<<", "<< the_stroke.p.y<<endl;
			//outfile<<"j, i = "<<j<<" , "<<i<<endl;
			glTranslatef(j*PIX_H,i*PIX_W,0);
			//outfile<<"the rotation is: "<<the_stroke.rot<<", "<<j<<", and "<<i<<endl;
			glRotatef( the_stroke.rot, 0, 0, 1 );
			glScalef( the_stroke.s[ 0 ], the_stroke.s[ 1 ], 1 );
			draw_stroke( the_stroke.r, the_stroke.g, the_stroke.b );
			

			glPopMatrix();
		}
		
}
}
*/
// difference from curved code
	for ( i = 0; i < st_n[fra]; i++){
    
		if (st[fra][i].rot < -900 ){
				brush_texture = OFF;
				//			 cout<<"stroke x, y = "<<st[fra][i].p.x<<" "<<st[fra][i].p.y<<endl;
		}
		if(brush_texture == ON ){
      start_stroke();
    }

		glPushMatrix();
		glTranslatef(st[fra][i].p.x,st[fra][i].p.y,0);
    outfile<<"st[fra][i].p.x, st[fra][i].p.y = "<<st[fra][i].p.x<<", "<< st[fra][i].p.y<<endl;
		//outfile<<"the rotation is: "<<the_stroke.rot<<", "<<j<<", and "<<i<<endl;
		if (st[fra][i].rot > -900 ){
			glRotatef( st[fra][i].rot, 0, 0, 1 );
		}
		glScalef( st[fra][i].s[ 0 ],st[fra][i].s[ 1 ], 1 );
//		setup_nurbs( st[fra][ i ].cp[ 0 ], st[fra][ i ].cp[ 1 ], 0.1 );    
//    draw_nurbs( .74, .87, 1 );
			
		glPopMatrix();
		
		if ( brush_texture == ON ){
      stop_stroke();
		}	
		if (st[fra][i].rot < -900 ){
				brush_texture = ON;
		}
	}
		
/*
  //draw the underpainting 

  for( i = 0; i < back_st_n[fra]; i++ ) {
  //for ( i = 0; i < 5; i++ ) {
      if (brush_texture == ON ) {
          start_stroke();
      }
	cerr<<"main.cpp:  draw_manip() i = "<<i <<endl;
      glPushMatrix();
      glTranslatef( back_st[fra][ i ].p.x, back_st[fra][ i ].p.y, 0 );
      glRotatef( back_st[fra][ i ].rot, 0, 0, 1 );
      // glScalef( back_st[fra][ i ].s[ 0 ], back_st[fra][ i ].s[ 1 ], 1 );
      //  setup_nurbs(7,4,.1);
	cerr<<" I got here before setup_nurbs"<<endl;
      setup_nurbs( back_st[fra][ i ].cp[ 0 ], back_st[fra][ i ].cp[ 1 ], 0.1 );    
	cerr<<" I got here after setup_nurbs"<<endl;
      draw_nurbs( .74, .87, 1 );
	cerr<<" I got here before draw_stroke"<<endl;
      //draw_stroke( .74, .87, 1 );  //hardcoded underpainting color
//		cerr<<" I got here after draw_stroke"<<endl;
      glPopMatrix();
      if (brush_texture == ON ) {
          stop_stroke();
      }
  }
*/
/*  //draw the overpainting 
// for( i = 0; i < st_n[fra]; i++ ) {
  for( i = 0; i < 40; i++ ) {
  if(brush_texture == ON )
  {
    start_stroke();
  }
      
  glPushMatrix();
  glTranslatef( st[fra][ i ].p.x, st[fra][ i ].p.y, 0 );
  glRotatef( st[fra][ i ].rot, 0, 0, 1 );
  // glScalef( st[fra][ i ].s[ 0 ], st[fra][ i ].s[ 1 ], 1 );
  //setup_nurbs(7,4,0.1);
  //setup_nurbs( st[fra][ i ].cp[ 0 ], st[fra][ i ].cp[ 1 ] , 0.1 );	
  //draw_nurbs( st[fra][ i ].r, st[fra][ i ].g, st[fra][ i ].b );
  draw_stroke( st[fra][ i ].r, st[fra][ i ].g, st[fra][ i ].b );
  glPopMatrix();
  if(brush_texture == ON )
  {
    stop_stroke();
  }
}
*/
/*
//draw the highlight strokes
for( i = 0; i < high_st_n[fra]; i++ ) {
  if ( high_st[fra][i].r != -1 ){  //Is this the problem?????????????????
    if(brush_texture == ON )
  {
    start_stroke();
  }			
    glPushMatrix();
    glTranslatef( high_st[fra][i].p.x, high_st[fra][i].p.y, 0 );
    glRotatef( high_st[fra][i].rot, 0, 0, 1 );
    // glScalef( high_st[fra][i].s[0], high_st[fra][i].s[1], 1 );
    //setup_nurbs(7,4,0.1);
    setup_nurbs( high_st[fra][i].cp[ 0 ], high_st[fra][i].cp[ 1 ], 0.1 );	
    draw_nurbs( high_st[fra][i].r, high_st[fra][i].g, high_st[fra][i].b );
    //draw_stroke( 1, 1, 0 );
    //draw_stroke( high_st[fra][i].r, high_st[fra][i].g, high_st[fra][i].b );
    glPopMatrix();
    if(brush_texture == ON )
  {
    stop_stroke();
  }
  }  
}
*/
}                 // End routine draw_curves


static void paint_borders(int reg_type){
  // Place a stroke at the center of each region of reg_type.  Designed for large regions.
  
  int cardinality;
  int i; int j;
  Point pt;
  int count = 0;
  float dist;
  float mindist = (PIX_W*2 + 20)*(PIX_W*2 + 20);
  int neigh;
  Point neigh_pt;
  int nostroke = 0;
  int num_regs;
  
  num_regs = vor->get_num_regions(fra);
  
  for( i = 0; i < num_valid_pts[fra] ; i++ ){

    //  Check if matches the type of region being painted
    if ( region_arr[fra][ i ] == reg_type ){
      cardinality = vor->get_cardinality( fra, i );
     
      // For each corner of the polygon, try to place a stroke.
      for ( j = 0; j < cardinality; j++ ){
        nostroke = 0;
        pt = vor->get_poly_pt( fra, i, j );
        if ( pt.mark_flag == 'c' ){  
          nostroke = 1;  // A stroke has already been placed here.
        }else{
          if ( j - 1 > -1 ){ neigh = j - 1;}else{ neigh = cardinality - 1;}
          
          dist = vor->find_polypt_dist_sq( fra, i, neigh, j );
          neigh_pt = vor->get_poly_pt(fra, i, neigh);
          if ( dist < mindist && neigh_pt.mark_flag == 'c' ){
            nostroke = 1;  // very close neighbor has a stroke already
          }else{
            //  Check other neighbor.
            if ( j + 1 < cardinality ){ neigh = j + 1;}else{ neigh = 0; }
              dist = vor->find_polypt_dist_sq( fra, i, neigh, j );
              neigh_pt = vor->get_poly_pt(fra, i, neigh);
              if ( dist < mindist && neigh_pt.mark_flag == 'c' ){
                nostroke = 1;  // Other close neighbor has a stroke already.
              }
          }
        }

        // If no stroke here or close to here already, paint stroke here.
        if ( !nostroke ){
            if ( st_n[fra] == st_max[fra] ){
              expand_st( st[fra], st_max[fra] );
            }
                   
            if (!map_stroke( pt.x*PIX_W, pt.y*PIX_H, st[fra][st_n[fra] ] ) )
            {
              //outty<<"In main.cpp: paint_borders call to map_stroke failed."<<endl;
            }else{
              st[fra][st_n[fra] ].p.x = pt.x*PIX_W;
              st[fra][st_n[fra] ].p.y = pt.y*PIX_H;
              vor->mark_pt(fra, i, j, 'c');
              st[fra][st_n[fra] ].type = 'c';
              st_n[fra]++;
            }
        }
      }
     

    //  pt = vor->get_centroid_coor(fra, i );
    //  outty<<pt.x<<" "<<pt.y<<"  "<<i;
    

      // Paint a stroke at the center of mass of this region.
      if ( st_n[fra] == st_max[fra] ){
        expand_st( st[fra], st_max[fra] );
      }

      pt = vor->find_com(fra, i);
      
      if (!map_stroke( pt.x*PIX_W, pt.y*PIX_H, st[fra][st_n[fra] ] ) )
      {
        //outty<<"In main.cpp: paint_borders call to map_stroke failed."<<endl;
      } else {
          st[fra][st_n[fra] ].p.x = pt.x*PIX_W;
          st[fra][st_n[fra] ].p.y = pt.y*PIX_H;

          if ( vor->get_area(fra, i ) == 1){
            //st[fra][st_n[fra] ].s[0] = st[fra][st_n[fra] ].s[1];
            largest = st_n[fra];
          }
          //outty<<"  "<<st_n[fra]<<"  "<<st[fra][st_n[fra]].r<<"  "<<st[fra][st_n[fra]].g<<"  "<<st[fra][st_n[fra]].b<<" "<<st[fra][st_n[fra]].p.x<<"  "<<st[fra][st_n[fra]].p.y<<endl;
          st_n[fra]++;
      }
    }

  }
  //outty<<"count  "<<count<<endl;

}  // end routine paint_borders


static void paint_matte( ){
  int i, j;
  int position;
  //float* v = new float [ s.attr_n ];
  for ( i = 0; i < s.h; i++ ){
    for ( j = 0; j < s.w; j++ ){
      
      if ( st_n[ fra ]== st_max[ fra ] ){
        expand_st( st[ fra ], st_max[ fra ]);
      }

      position = j + ( i * s.w ) ;

      gf.get( position, fra, v );
      // Set color (clearcolor, if type clear; based on data, if matte.)
      st[fra][ st_n[ fra ]].r = v[ 0 ];
      st[fra][ st_n[ fra ]].g = v[ 1 ];
      st[fra][ st_n[ fra ]].b = v[ 2 ];
      
      // Set or reset other stroke properties to make it square and upright.
      st[fra][ st_n[ fra ]].seg_index = 4;
      st[fra][ st_n[ fra ]].p.x = j*PIX_W;
      st[fra][ st_n[ fra ]].p.y = i*PIX_H;
      st[fra][ st_n[ fra ]].s[ 0 ] = PIX_W;
      st[fra][ st_n[ fra ]].s[ 1 ] = PIX_H;
      st[fra][ st_n[ fra ]].rot = 0;

      st_n[fra]++;
    }
  }
  //delete v;
}  // end routine paint_matte()

static void paint_matte( int matt, int clear ){
  int i, j;
  for ( i = 0; i < s.h; i++ ){
    for ( j = 0; j < s.w; j++ ){
      if( s.seg[ i ][ j ] == matt || s.seg[ i ][ j ] == clear){
        if ( st_n[ fra ]== st_max[ fra ] ){
          expand_st( st[ fra ], st_max[ fra ]);
        }
        // Set color (clearcolor, if type clear; based on data, if matte.)
        if( s.seg[ i ][ j ] == clear ){
            st[fra][ st_n[ fra ]].r = clearcolor[ 0 ];
            st[fra][ st_n[ fra ]].g = clearcolor[ 1 ];
            st[fra][ st_n[ fra ]].b = clearcolor[ 2 ];
        }else {
           map_stroke( j*PIX_W, i*PIX_H, st[fra][ st_n[ fra ]]);
        }
        // Set or reset other stroke properties to make it square and upright.
        st[fra][ st_n[ fra ]].seg_index = s.seg[ i ][ j ];
        st[fra][ st_n[ fra ]].p.x = j*PIX_W;
        st[fra][ st_n[ fra ]].p.y = i*PIX_H;
        st[fra][ st_n[ fra ]].s[ 0 ] = PIX_W;
        st[fra][ st_n[ fra ]].s[ 1 ] = PIX_H;
        st[fra][ st_n[ fra ]].rot = 0;

        st_n[fra]++;
      }
    }
  }
}  // end routine paint_matte(int matte, int clear)


static int indication_setup()
//  This routine calls the stroke() routine for the high variability regions (0 and 3)
//  It calls the paint borders routine for regions with low variability (1 and 2)
//  It creates square strokes for strokes coming from mixed regions (4 and 6)
//  This creates a stroke list for the frame.  
//  
//  This routine should only be called upon initial entry of a frame and when segmentation_type == -1.
//  After this the stroke lists are saved and redisplayed.

// Voronoi segmenting:
// 0 if the current point is in a small region which does not border a large region
// Segment 0 is painted with stroke() and cov = 1.
// 1 if the current point is in a large region which does not border a small region
// Segment 1 is painted with paint_borders()
// 2 if the current point is in a large region which borders a small region
// Segment 2 is painted with paint_borders()
// 3 if the current point is in a small region which borders a large region
// Segment 3 is painted with stroke() and cov < 1.
// 4 if the current point is valid but its region has invalid points and it does not border a small region.
// Segment 4 is painted with unoriented untextured square strokes.
// 5 if the current point is invalid, but its region contains valid points
// Segment 5 is painted with unoriented untextured square strokes colored as clearcolor to match segment 5.
// 6 if the current point is in an entirely invalid region. Not painted.

{
  float cov = 1;       //  Coverage between 0 and 1 for a segment

  if ( segmentation_type != -1 ){
    cerr<<"Routine indication_setup should only be called when segmentation_type = -1"<<endl;
    return -1;
  }
  //outty.open("middle.txt");

  st_max[fra] = 16;           // Initial arr size; usually grows

  st[fra] = new stroke_s[ st_max[fra] ];
  
  // Paint segments 4 and 6 with unoriented untextured square strokes.  Use data to color strokes in 4.  Use clearcolor for strokes in 6.
  // A point is in 4 if it is valid but its region has invalid points and it does not border a small region.
  // A point is in 5 if the current point is invalid, but its region contains valid points
  if (is_jpeg){
  //  paint_matte();
  }else{ 
    paint_matte(4,5);  
  }

  // Paint segment 1 with paint_borders()
  // A point is in 1 if it is in a large region which does not border a small region
  cur_seg = 1;
  printf(" Segment number %d of %d is being painted\n", cur_seg, s.n ); 
  paint_borders(1); // give one stroke to the center of each large nonborder region

  // Paint segment 2 with paint_borders()
  // A point is in 2 if it is in a large region which borders a small region
  cur_seg = 2;  // map_stroke sets the stroke seg_index to cur_seg
  printf(" Segment number %d of %d is being painted\n", cur_seg, s.n ); 

  paint_borders(2);  // give one stroke to the center of each large-sized border region

  //outty.close();



  
  // Paint segment 0 with stroke() and cov = 1.
  // Points are in segment 0 if they are in a small region which does not border a large region
  cur_seg = 0;
  printf(" Segment number %d of %d is being painted\n", cur_seg, s.n ); 
  st_n[fra] = stroke( id, s.w * PIX_W, s.h * PIX_H, 0, st[fra],st_n[fra], st_max[fra], cov, segmentation_type);
  
  
  // Paint segment 3 with stroke() and cov < 1.
  // Points are in segment 3 if they are in a small region which borders a large region
  cov = 0.98;
  cur_seg = 3;
  printf(" segment number %d of %d is being painted\n", cur_seg, s.n ); 
  st_n[fra] = stroke( id, s.w* PIX_W, s.h * PIX_H, 3, st[fra],st_n[fra], st_max[fra],cov, segmentation_type);
  

  /*
  for ( i = 0; i < s.h; i++ ) {
    delete [] s.seg[ i ];
  }
  delete [] s.seg;*/
  /*  for (i = 0; i < s.h*PIX_H; i++) {
    delete [] id[i];
  }
  delete [] id;
  */
return 1;
}                   // End rountine indication_setup

static int new_frame_setup()
//  This routine sets up the segmentation and calls the stroke routine which
//  creates a stroke list for the frame.  It is only called upon during
//  initial entry of a frame.  After that the stroke lists are saved and
//  redisplayed.
{
  float cov = .8;       //  Coverage between 0 and 1 for a segment
  int   i;     //  Loop counter
	
avg_size = new float*[ s.n ];     // Creat array to hold avg
for ( i = 0;  i < s.n; i++ ) {     // stroke size of each segment
  avg_size[ i ] = new float[ 2 ]; // used for highlight sizes
}

st_max[fra] = 16;           // Initial arr size; usually grows

st[fra] = new stroke_s[ st_max[fra] ];

for ( i = 0; i < s.n; i++ ) {

    printf(" segment number %d of %d is being painted\n", i, s.n );

	// ****************Replace when finished debugging ****************
  //	comp_coverage( i, 1, cov );  // the second parameter assigns an attribute to coverage
  //  cov = .8;
	//  Testing purposes
  //    comp_size( i, 4, size );
  //    printf( "cov = %5.3f, \n", cov );
  //    printf( "size[0] = %d, size [1] = %2d: \n", int( size[ 0 ] ), int( size[ 1 ] ) );


	st_n[fra] = stroke( id, s.w * PIX_W, s.h * PIX_H, i, st[fra],st_n[fra], st_max[fra], cov, segmentation_type);
  cout<<"st_n = "<<st_n[fra]<<endl;
	find_avg_size( avg_size[ i ] );     //avg size in each segment for highlight stroke dimensions

	
	//cout<<i<<" = i "<<avg_size[ i ][0] << " = bla0 "<<avg_size[ i ][1]<<" = bla1 "<<endl;
  //potential problem:  some segs don't have strokes; find_avg_size sets those avg sizes to 0
}

//Create a set of strokes (back_st) to draw the backwash. 
//Using 100% coverage, set by the penultimate parameter in stroke
//Using same segmentation as foreground strokes, on ALL the properties
/*back_st_n[fra] = 0;  
st_max[fra] = 16;
back_st[fra] = new stroke_s[ st_max[fra]];
   for ( i = 0; i < s.n; i++) {
for ( i = 0; i < 1; i++) {
  back_st_n[fra] = stroke(id, s.w * PIX_W, s.h * PIX_H, i, back_st[fra], back_st_n[fra], st_max[fra], 1);
}
*/
// cout<<"the value of hfactor now is................EWERWERWERWERWERWER    "<<hfactor<<endl;   ////something skrewy here!

/*if (!highlight(fra, hfactor, num_hig, s.attr_n, s.w, s.h, amin[ num_hig  ], amax[ num_hig],high_st[fra], &gf ))
  { 
       	cerr<<"No highlight applied"<<endl;
  }
*/   
for ( i = 0; i < s.n; i++ ) {
  delete [] avg_size[ i ];
}
delete [] avg_size;

/*
for ( i = 0; i < s.h; i++ ) {
  delete [] s.seg[ i ];
}
delete [] s.seg; */
/*  for (i = 0; i < s.h*PIX_H; i++) {
  delete [] id[i];
}
delete [] id;
*/
return 1;

}                   // End rountine new_frame_setup


void print_matrix( int mat_type){
float mat[16];

//glGetFloatv(GL_MODELVIEW_MATRIX, mat);
glGetFloatv(GL_PROJECTION_MATRIX, mat);
printf("::::MODEL_VIEW_MATRIX::::\n");
printf("( %f %f %f %f )\n",mat[0],mat[1],mat[2],mat[3]);
printf("( %f %f %f %f )\n",mat[4],mat[5],mat[6],mat[7]);
printf("( %f %f %f %f )\n",mat[8],mat[9],mat[10],mat[11]);
printf("( %f %f %f %f )\n",mat[12],mat[13],mat[14],mat[15]);
}


static int set_err( seg_s& s, float p )

//  This routine probes the input file, computing the min-max range
//  for each attribute, then setting the allowable error to some
//  percentage of that range
//
//  s:    Segment info structure
//  err:  Error cutoff array
//  p:    Range percentage
{
  int     i;             // Loop counter
  int     rc = 0;           // Temporary return code

  amax = new float[ s.attr_n ];      // Create global amin, amax arrays
  amin = new float[ s.attr_n ];

  gf.min_max( amin, amax );

  for ( i = 0; i < s.attr_n; i++ ) { // Set cutoff error values
      s.err[ i ] = ( amax[ i ] - amin[ i ] ) * p;
  }

  
  return 1;
}                   // End routine set_err

void setup_mapping(  int num_attr, bool clamp_attr ) 
// This routine initializes map_matrix to reflect the current mapping.  Each entry map_matrix[ i ][ j ]is set 
// to 0 or 1.  An entry of 1 indicates a mapping from the jth attribute to the ith feature.
//
// num_attr: number of data attributes
{  
int i, j;  //counters
attr_map feature;
int num_features = feature.num_feature();  //number of visual features, as set in attr_map.h
DEBUG = 1;
for (i = 0; i < num_features ; i++ ){
  for ( j = 0 ; j < num_attr ; j++ ){
    if ( feature.name( i ) == gf.attr_map_val( j ).name() && !clamp_attr ){
		    map_matrix[ i ][ j ] = 1;
		} else {
		  map_matrix[ i ][ j ] = 0;
    }
  }
}
DEBUG = 0;

/// HACK

if ( clamp_attr && !is_jpeg){
  map_matrix[0][col_attr] = 1; // colour
  map_matrix[1][siz_attr] = 1; // size
  map_matrix[4][ori_attr] = 1; // orientation
  map_matrix[5][gre_attr] = 1; // greyscale
  map_matrix[9][con_attr] = 1; // contrast
  map_matrix[10][pro_attr] = 1; // proportion
}
/* //South America
map_matrix[1][0] = 1;//cloud_cover -> size  (first number is vis feature, second is data attr)
map_matrix[0][2] = 1;//mean_temp -> colour
map_matrix[4][5] = 1;//wind_speed -> orientation
*/

/*
// asia 500
map_matrix[1][3] = 1;//precipitation -> size  
map_matrix[0][8] = 1;//max temp -> colour
map_matrix[4][7] = 1;//vapor pressure -> orientation
*/

/*
// Iouca 3125
map_matrix[1][4] = 1;//pressure -> size  
map_matrix[0][3] = 1;//density -> colour
map_matrix[4][2] = 1;//magnitude -> orientation
*/

if ( is_jpeg ){
// Map all three colors components to both size and orientation
  map_matrix[1][0] = 1;// red-> size
  map_matrix[1][1] = 1;//green -> size
  map_matrix[1][2] = 1;// blue-> size
  map_matrix[4][0] = 1;// red-> orientation
  map_matrix[4][1] = 1;// green -> orientation
  map_matrix[4][2] = 1;//blue -> orientation
  map_matrix[9][con_attr] = 1; // contrast
  map_matrix[10][pro_attr] = 1; // proportion
}
/*

::::::MAP_MATRIX::::::
num_attr = 11
0  0  1  0  0  0  0  0  0  0  0
1  0  0  0  0  0  0  0  0  0  0
0  0  0  0  0  0  0  0  0  0  0
0  0  0  0  0  0  0  0  0  0  0   num_features = 13
0  0  0  0  0  1  0  0  0  0  0
0  0  0  0  0  0  0  0  0  0  0
0  0  0  0  0  0  0  0  0  0  0
0  0  0  0  0  0  0  0  0  0  0
0  0  0  0  0  0  0  0  0  0  0
0  0  0  0  0  0  0  0  0  0  0
0  0  0  0  0  0  0  0  0  0  0
0  0  0  0  0  0  0  0  0  0  0
0  0  0  0  0  0  0  0  0  0  0


features[ 0 ].name(0) = Colour
features[ 0 ].name(1) = Size
features[ 0 ].name(2) = Density
features[ 0 ].name(3) = Regularity
features[ 0 ].name(4) = Orientation
features[ 0 ].name(5) = Greyscale
features[ 0 ].name(6) = Flicker
features[ 0 ].name(7) = Direction
features[ 0 ].name(8) = Velocity
features[ 0 ].name(9) = X
features[ 0 ].name(10) = Y
features[ 0 ].name(11) = Z
features[ 0 ].name(12) = None

Name[ 0 ] = cloud_cover
Name[ 1 ] = frost_freq
Name[ 2 ] = mean_temp
Name[ 3 ] = precip
Name[ 4 ] = radiation
Name[ 5 ] = w_speed
Name[ 6 ] = wet_day_freq
Name[ 7 ] = vapor_press
Name[ 8 ] = max_temp
Name[ 9 ] = min_temp
Name[ 10 ] = temp_range
*/
if (DEBUG){
  printf("::::::MAP_MATRIX::::::\n");
  for (i = 0; i < num_features ; i++ ){
    for ( j = 0 ; j < num_attr ; j++ ){
printf( " %d ", map_matrix[ i ][ j ]);
if ( j == num_attr - 1 )	printf("\n");
    }
  }
}
				
DEBUG = 1;
for ( j = 0 ; j < num_features ; j++ ){
  if(DEBUG)
    printf("feature.name(%d) = %s\n", j, (char*)feature.name(j) );
  if ( feature.name(j) == "Colour") {
    num_col = j;  
  }else if ( feature.name(j) == "Contrast") {
    num_con = j;
  }else if ( feature.name(j) == "Highlight") {
    num_hig = j;
  }else if ( feature.name(j) == "Size") {
    num_siz = j;
  }else if ( feature.name(j) == "Orientation") {
    num_ori = j;
  }else if ( feature.name(j) == "Coverage") {
    num_cov = j;
  }else if ( feature.name(j) == "Greyscale") {
    num_gre = j;    
  }else if ( feature.name(j) == "Proportion") {
    num_pro = j;
  }else{
    if (DEBUG)
      printf("The visual feature feature.name(%d) = %s has not been implemented\n", j,(char*)feature.name(j) );
  } DEBUG;
}  
DEBUG = 0;
} //end routine setup_mapping


static int is_valid(int x_mult, int y_mult, float pix_wide, float pix_hi, float* val){
// x_mult and y_mult are multiples of x and y respectively
// x and y are retrieved by dividing out pix_wide and pix_hi, the factors introduced to 
// represent each data point with more than one pixel.
  int position;
  int rc;

  //Check for out of bounds x,y coordinates.
  if (  x_mult < 0 || x_mult >= s.w*pix_wide || y_mult < 0 || y_mult >= s.h*pix_hi ){
    return 0;
  }

  position = get_current_position( x_mult, y_mult, pix_wide, pix_hi );
	gf.get( position,fra, val);
  rc = gf.allowable( val );

  return rc;
}  // is_valid



int main( int argc, char *argv[] )
{  

  if ( argc < 13 ){
    cerr<< "Usage:  pittore BIN_FILE num_fr_to_display #_faces threshold is_jpeg  col_attr gre_attr size_attr ori_attr con_attr pro_attr mucha\n"<<endl;
    cerr<< "BINFILE: name of binary file to be visualized"<<endl;
    cerr<< "num_fr_to_display: number of frames to display (must be smaller than number of frames in dataset)"<<endl;
    cerr<< "#_faces: Approximate number of faces in reduced mesh file"<<endl;
    cerr<< "threshold: large region size cutoff"<<endl;
    cerr<< "col_attr: data attribute used to assign color"<<endl;
    cerr<< "gre_attr: data attribute used to assign greyscale"<<endl;
    cerr<< "siz_attr: data attribute used to assign size"<<endl;
    cerr<< "ori_attr: data attribute used to assign orientation"<<endl;
    cerr<< "con_attr: data attribute used to assign contrast"<<endl;
    cerr<< "pro_attr: data attribute used to assign proportion"<<endl;
    cerr<< "is_jpeg: flag to indication binfile is based on a jpeg file."<<endl;

    cerr<<"example input1: weather.southamerica6_7_10_f600/weather.southamerica.pc.bin 12 600 .5 w 6 7 10   ic "<<endl;
    cerr<<"example input2: data/butterfly.pc.bin 1 1200 .4 j 0 1 2   m "<<endl;
    cerr<<"weather.southamerica6_7_10_f600/weather.southamerica.pc.bin 12 600 .3 w   6 7 10   m"<<endl;
    cerr<<"weather.asia2_4_6_f1200/asia.pc.bin 12 1200 .3 w   2 4 6   m"<<endl;
    cerr<<"moraine0_1_2_f1200/moraine.pc.bin 1 1200 .5 j   0 1 2   m"<<endl;
    cerr<<"weather.us2_6_7_f200/weather.us.pc.bin 12 200 .3 w 2 6 7   m"<<endl;
    cerr<<"orcas0_1_2_f1200/orcas.pc.bin 1 1200 .5 j   0 1 2   m"<<endl;
    cerr<<"Iouca.cropped2_3_4_f100/Iouca.cropped.pc.bin 1 100 .6 i   2 3 4   m"<<endl;
    for( int i = 0; i < argc; i++ ){
      cerr<< "Argv[i] = "<<argv[i]<<"   i = "<<i<<endl;
    }
    return 0;
  }
  newfile = Tcl_Alloc( 1024 );

  newfile = argv[1];
  num_fr_to_display  = atoi(argv[2]);
 
  num_faces = atoi(argv[3]);
  threshold = atof(argv[4]);

  is_jpeg = false;
  is_iouca = false;
  if ( (string) argv[5] == "j" ){
    is_jpeg = true;
  }else if ( (string) argv[5] == "i" ){
    is_iouca = true;
  }

  col_attr = atoi(argv[6]);
  gre_attr = atoi(argv[7]);
  siz_attr = atoi(argv[8]);
  ori_attr = atoi(argv[9]);
  con_attr = atoi(argv[10]);
  pro_attr = atoi(argv[11]);
  mucha = 0;
  complex = 0;
  if ( (string)argv[12] == "m" ) {
    mucha = 1;
  }else if ( (string)argv[12] == "c" ) {
    complex = 1;
  }

  argc = 1;
	Tk_Main( argc, argv, init_tcl);  // call init_tcl which starts everything going.
  return 0;
}
