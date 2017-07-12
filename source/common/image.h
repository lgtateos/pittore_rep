/*
 *	Defines for image files . . . .
 *
 *  			Paul Haeberli - 1984
 *
 */
#ifndef IMAGEDEF
#define IMAGEDEF

#if defined(_LANGUAGAE_C_PLUS_PLUS) || defined(__cplusplus)
extern "C" {
#endif

#include "stdio.h"

typedef struct {
    unsigned short	imagic;		/* stuff saved on disk . . */
    unsigned short 	type;
    unsigned short 	dim;
    unsigned short 	xsize;
    unsigned short 	ysize;
    unsigned short 	zsize;
    unsigned long 	min;
    unsigned long 	max;
    unsigned long	wastebytes;	
    char 		name[80];
    unsigned long	colormap;

    long 		file;		/* stuff used in core only */
    unsigned short 	flags;
    short		dorev;
    short		x;
    short		y;
    short		z;
    short		cnt;
    unsigned short	*ptr;
    unsigned short	*base;
    unsigned short	*tmpbuf;
    unsigned long	offset;
    unsigned long	rleend;		/* for rle images */
    unsigned long	*rowstart;	/* for rle images */
    long		*rowsize;	/* for rle images */
} IMAGE;

#define IMAGIC 	0732

/* colormap of images */
#define CM_NORMAL		0	/* file contains rows of values which are
	either RGB values (zsize == 3) or greyramp values (zsize == 1) */
#define CM_DITHERED		1
#define CM_SCREEN		2	/* file contains data which is a screen image;
	getrow returns buffer which can be displayed directly with writepixels.  */
#define CM_COLORMAP		3	/* a colormap file */

#define TYPEMASK	0xff00
#define BPPMASK		0x00ff
#define ITYPE_VERBATIM	0x0000
#define ITYPE_RLE	0x0100
#define ISRLE(type)		(((type) & 0xff00) == ITYPE_RLE)
#define ISVERBATIM(type)	(((type) & 0xff00) == ITYPE_VERBATIM)
#define BPP(type)		((type) & BPPMASK)
#define RLE(bpp)		(ITYPE_RLE | (bpp))
#define VERBATIM(bpp)		(ITYPE_VERBATIM | (bpp))
#define	IBUFSIZE(pixels)	( (pixels+(pixels>>6))<<2 )
#define	RLE_NOP		0x00

#define	ierror(p)	(((p)->flags&_IOERR)!=0)
#define	ifileno(p)	((p)->file)

IMAGE *iopen( char *, char *, ... );
IMAGE *icreate();
void iclose( IMAGE * );
void getrow( IMAGE *, short *, int, int );
void putrow( IMAGE *, short *, int, int );
unsigned short *ibufalloc();

#define	getpix(p)	(--(p)->cnt>=0? *(p)->ptr++:ifilbuf(p))
#define putpix(p,x) (--(p)->cnt>=0? ((int)(*(p)->ptr++=(unsigned)(x))):iflsbuf(p,(unsigned)(x)))

typedef struct {
	unsigned short 	type;
    	unsigned long colormap;
	int xsize, ysize;
	unsigned long *rowstart;
	long	*rowsize;
	short *pixels;     
} MEMIMAGE;

MEMIMAGE *readimage();
MEMIMAGE *readrleimage();
MEMIMAGE *newimage();

#if defined(_LANGUAGAE_C_PLUS_PLUS) || defined(__cplusplus)
}
#endif

#endif
