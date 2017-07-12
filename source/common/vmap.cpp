#include "vmap.h"

/*--------------------------------------------------------------------------*/
/*  VMAP.CPP								    */
/*    Routines to maintain visual mappings for each attribute		    */
/*									    */
/*- Modfication History ----------------------------------------------------*/
/*  When:	Who:			Comments:			    */
/*									    */
/*  05-Jun-00	Christopher G. Healey	Initial implementation		    */
/*  28-Jun-03	Christopher G. Healey	Changed vmap_invoke to take pointer */
/*					to bin_file, rather than reference  */
/*					to grid_file (more flexible)	    */
/*--------------------------------------------------------------------------*/


int vmap_invoke( Togl *togl, bin_file *bf )

  //  Procedure to invoke the vmap dialog; return 0 on Cancel, 1, on
  //  OK
  //
  //  togl:  Pointer to TOGL interpreter
  //  bf:    Pointer to binary file (or subclass of bin_file)
{
  char            cmd[ 256 ];		// Tcl command string
  int             i;			// Loop counter
  attr_map        m;			// Current attribute map value
  string    nm;			// Current attribute name
  Tcl_Interp      *tcl;			// Pointer to tcl interpreter

  tcl = Togl_Interp( togl );

  Tcl_Eval( tcl, "vmap_flush" );	// Reset, initialize dialog
  for( i = 0; i < bf->attr_n(); i++ ) {
    m = bf->attr_map_val( i );
    nm = bf->attr_name( i );
    sprintf( cmd, "vmap_add_attr %s %d", (char *) nm, m.vf_get() );
    Tcl_Eval( tcl, cmd );
  }
  Tcl_Eval( tcl, "vmap" );		// Invoke vmap dialog
  if ( atoi( tcl->result ) == 0 ) {	// If Cancel, don't update mappings
    cerr<<"The data-feature mapping has been cancelled with result: "<<tcl->result<<endl;
    return 0;
  }

for( i = 0; i < bf->attr_n(); i++ ) {	// Update attribute mappings
    sprintf( cmd, "lindex $vmap_visual %d", i );
    Tcl_Eval( tcl, cmd );
     
    m.vf_set( atoi( tcl->result ) );
    bf->attr_map_val( i, m );
  }

  return 1;
}					// End method vmap_invoke


void vmap_post_init( Tcl_Interp *tcl )

  //  Procedure to initialize vmap dialog and popup menu; this routine
  //  must be called AFTER the tcl interpreter is initialized and the
  //  C++ class routines are available, hence the name ...post_init()
  //
  //  tcl:  Pointer to tcl interpreter
{
  int DEBUG = 0;
  char      cmd[ 256 ];			// Tcl command string
  int       i;				// Loop counter
  attr_map  temp_map;			// Temporary map object
  
  printf("In vmap__post_init:  num features = %d\n",temp_map.num_feature() ); 
  
  for( i = 0; i < temp_map.num_feature(); i++ ) {
   
   sprintf( cmd, "vmap_add_feat %s", (char *) temp_map.name( i ) );
   Tcl_Eval( tcl, cmd );
   
   if(DEBUG)
     printf( "vmap.cpp: vmap_post_init initializing feature(%d) = %s\n",i,(char*) temp_map.name(i) );
  }
}                                       // End routine vmap_post_init
