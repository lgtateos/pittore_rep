//  COLOUR.H
//    Prototypes for COLOUR.CPP

#ifndef _colour_h
#define _colour_h

void  get_colour(			// Get colour along LUV colour spiral
        float,				//  Parametric t (0=PB .. 1=RP)
        float&,				//  R (red) for resulting colour
        float&,				//  G (green) for resulting colour
        float&				//  B (blue) for resulting colour
      );
void  get_colour(			// Get colour w/specific L and hue
        float,				//  Parametric L (0=black .. 1=white)
        float,				//  Parametric t (0=PB .. 1=RP)
        float&,				//  R (red) for resulting colour
        float&,				//  G (green) for resulting colour
        float&				//  B (blue) for resulting colour
      );
void  get_grey(				// Get grey along LUV grey ramp
        float,				//  Parametric t (0=black .. 1=white)
        float&,				//  R (red) for resulting colour
        float&,				//  G (green) for resulting colour
        float&				//  B (blue) for resulting colour
      );


void hsv_to_rgb( float& r, float& g, float& b, float h, float s, float v );
void rgb_to_hsv( float r, float g, float b, float& h, float& s, float& v );

#endif
