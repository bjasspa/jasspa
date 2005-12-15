/*{{{  about this file*/
/*
    FILE    : DPL5.C
    PROJECT : dView
    Author  : Phil Atkin, aided+abetted by Jeff Sullivan


    (C) Division Ltd 1991 / 1992 / 1993.

    The pxpl5 PAZ rpc interface

    All the nasty big/little endian stuff by Jeff

    11/2/92 PAZinit_ became PAZinit

    removed -
	    PAZgrowObject, PAZstartObject, PAZfinishObject
    added -
	    PAZopenTexture, PAZcloseTexture, PAZtextels
	    PAZreplaceGeometry
	    PAZboot

    16/2/92 PAZinitView takes an additional device arg

    4/7/92  fixed PAZdeleteScene
    9/7/92  introduced binary interface fn's
	    PAZopenBinObject, PAZopenBinPatch,
	    PAZopenBinTriStrip, PAZopenBinPolystrip,
	    PAZwriteBinVertices, PAZcloseBinObject

    28/8/93 modifications for pazpl5 - changes to texture api,
	    changes to PAZ.h (imported from pazpl5.h), new calls
	    for material support
    
*/
/*}}}  */
/*{{{  includes*/
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* VPL - changed <> to "" */
#include "paztyp5.h"
#include "pazio.h"
#include "dnet.h"
#include "vizpl5.h"
#include "pack.h"
/* VPL - added includes. */
#ifdef macintosh
/* thinkC is fascist on prototypes */
#include "dpl5.h"
#endif
#include "linkio.h"
/*}}}  */

/*{{{  variables+externs*/
short __PAZconnected = false;
char *__lastPAZop="NULL";
int860 __PAZ_triangles, __PAZ_millisecs;
int860 *__blk;
int dnetDest=255;
short __PAZ_DegAngles=1;
int dpl_verbose=0;

/*{{{  #define _idmatrix(m)*/

#define _idmatrix(m) \
m[0][0] = 1; m[0][1]=0; m[0][2]=0; m[0][3]=0; \
m[1][0] = 0; m[1][1]=1; m[1][2]=0; m[1][3]=0; \
m[2][0] = 0; m[2][1]=0; m[2][2]=1; m[2][3]=0; \
m[3][0] = 0; m[3][1]=0; m[3][2]=0; m[3][3]=1
/*}}}  */

#define max_buffer_packets 120
int buffer_packets=0;
static int860 ret;
static int860 ack_pending=0, renderer_returnValue;

extern short __my_dNet_ID;
extern void PAZidMatrix ( MATRIX m );

int __acks_per_frame=1;
/*}}}  */

/*{{{  some useful low-level access functions*/

/*{{{  static int860 ack(int860 inp)*/
static int860 ack(int860 inp)
{
  int860 l;
  extern int  __acks;
  extern long __ack_len;
  extern char __ack_mess[256];

  PAZ_ABORTCHECK;

  if (ack_pending !=0 ) {
    int t=__my_dNet_ID+1;

    if (__acks) {
      __acks=0;
      memcpy ( __blk, __ack_mess, __ack_len );
    }
    else
      D_getAcks ( (short) t, (long *)&l, (char *) __blk, (int) ack_pending );

    renderer_returnValue = __blk[1];
    ack_pending=0;
    buffer_packets=0;
  }
  if (inp)
  {
    D_getH (__my_dNet_ID+1, (long *)&l, (char *) __blk );
    buffer_packets=0;
  }

  return l;

  PAZ_EXITZERO;
}

static void check(void) 
{
  buffer_packets++;
  if (ack_pending && (buffer_packets == max_buffer_packets)) ack(0);
}

/*}}}  */
/*{{{  static int860 protocol ( int860 tag, int860 a, int860 inp )*/
static int860 protocol ( int860 tag, int860 a, int860 inp )
{
  int860 l;

  register char *b = (char *)__blk;

  PAZ_ABORTCHECK;

  check();

  /* __blk is well aligned-use fast version*/
  PUT_INT32_FAST(tag,b);
  PUT_INT32_FAST(a,b);

  D_putH (dnetDest, 8, (char *) __blk );

  if (inp) {
    l = ack(1);
    b = (char *)&(__blk[1]);
    GET_INT32_FAST(ret,b);
#ifdef VERBOSE
	PAZinform("protocol:D_getH returned length = 0x %x __blk[1] = 0x%x:"
				"ret = 0x%x.\n",l, __blk[1], ret);
#endif
    }
	else
	  ret = 0;
	  
  return ret;
  PAZ_EXITZERO;
}
/*}}}  */
/*{{{  static int860 protocol2 ( int860 tag, int860 a, int860 aa, int860 inp )*/
static int860 protocol2 ( int860 tag, int860 a, int860 aa, int860 inp )
{
  register char *b = (char *)__blk;

  PAZ_ABORTCHECK;

  check();
  /* __blk is well aligned-use fast version*/
  PUT_INT32_FAST(tag,b);
  PUT_INT32_FAST(a,b);
  PUT_INT32_FAST(aa,b);

  D_putH (dnetDest, 12, (char *) __blk );
  if (inp) 
    ack(1);
	
  b = (char *)&(__blk[1]);
  GET_INT32_FAST(ret,b);

#if 0
	PAZinform("D_getH returned length = 0x %x __blk[1] = 0x%x:"
				"ret = 0x%x.\n",l, __blk[1], ret);
#endif

  return ret;
  PAZ_EXITZERO;
}
/*}}}  */
/*{{{  static int860 string_protocol ( int860 tag, char *s )*/
static int860 string_protocol ( int860 tag, char *s )
{
  int860 l=(int860) 1+(int860) strlen(s);
  register char *b = (char *)__blk;

  PAZ_ABORTCHECK;
  check();

  PUT_INT32_FAST(tag,b);

  memcpy ( b, s, (int ) l );
  D_putH (dnetDest, l+(long)4, (char *) __blk );
  ack(1);
  b = (char *)__blk;
  GET_INT32_FAST(ret,b);

  return ret;
  PAZ_EXITZERO;
}
/*}}}  */

/*{{{  static void write_remote_count ( int32 tag, void *p, void *local, int n_bytes)*/
static void write_remote_count ( int32 tag, void *p, void *local, int n_bytes)
{
  /*
     NOTE THIS IS CALLED MORE FREQUENTLY THAN MOST -

     There is an opportunity to optimize this by
     just copying all the bytes into a big vector and squirting
     them out in one big shot

     We will need to be careful about iserver transactions coming back
     from the renderer
  */
  #if 0
  /*{{{  void D_putH ( long dest, long len, char *message )*/
  void D_putH ( long dest, long len, char *message )
  {
    extern int buffer_packets;
    unsigned char header [TXHEADLEN];
    unsigned char *h = header;
    int860 l = 4;
    int iserved=0;
  
    aux_msg[0]=0x0;
  
    /* message needs to go ... */
  
    /* INT16(4)::header::len::INT16(len)::header */
    if ((iserved==0) && ready_yet()) {
      service_link();
      iserved=1;
    }
  
    PUT_INT16( l,h);
    PUT_BYTE ( dest,h);
    PUT_BYTE ( __my_dNet_ID,h);
    PUT_BYTE ( DC_BCAST,h);
    PUT_BYTE ( 0,h);
  
    PAZ_ABORTCHECK;
    if (outRecord ( (char *) header, 6 ) == 0)
      PAZerror ("Failed to output dnet header in %s (%d)\n", __lastPAZop,
	 buffer_packets );
  
    /* now pack the 4-byte message to be len */
    h=header;
  
    PUT_INT32(len,h);
    if (outRecord ( (char *) header, 4 ) == 0)
      PAZerror ("Failed to output dnet micromessage in %s (%d)\n", __lastPAZop,
	 buffer_packets );
  
    h=header;
  
    PUT_INT16( len,h);                    /* length for rest of message */
    PUT_BYTE ( dest,h);
    PUT_BYTE ( __my_dNet_ID,h);
    PUT_BYTE ( DC_BCAST,h);
    PUT_BYTE ( 0,h);
  
    if ((iserved==0) && ready_yet()) {
      service_link();
      iserved=1;
    }
  
    if (outRecord ( (char *) header, 6 ) == 0)
      PAZerror ("Failed to output dnet mega header in %s (%d)\n", __lastPAZop,
	 buffer_packets );
  
    if (outFastRecord ( message, len ) == 0)
      PAZerror ("Failed to output dnet mega packet in %s (%d)\n", __lastPAZop,
	 buffer_packets );
  
    PAZ_EXIT;
  }
  /*}}}  */
  #endif
  int t=n_bytes+8;
  register char *_b = (char *)__blk;

  check();

  PUT_INT32_FAST((int32)(tag), _b);
  PUT_INT32_FAST((int32)(p),   _b);
  PUT_INT32_SLICE(n_bytes, local,    _b);

  D_putH ((long) dnetDest, t, (char *) __blk );
}
/*}}}  */

/*{{{  static void read_remote_count ( int32 tag, void *p, void *local, int n_bytes)*/
static void read_remote_count ( int32 tag, void *p, void *local, int n_bytes)
{
  char *_b = (char *)__blk;

  check();
  PUT_INT32_FAST((int32) tag, _b);
  PUT_INT32_FAST( ((int32)p), _b);
  D_putH ((long)dnetDest, (long) 8, (char *) __blk );

  ack(1);
  _b = (char *)__blk;
  GET_INT32_SLICE(n_bytes, local, _b);
}
/*}}}  */

#if 0
/*{{{  */
/*
  can assume decent alignment in here
*/

#define write_remote(tag,p,local,type)  \
{ \
  int l=sizeof(type); \
  int t=l+8; \
  register char *_b = (char *)__blk; \
  check();                     \
  PUT_INT32_FAST((int32)(tag),_b); \
  PUT_INT32_FAST((int32)(p),_b); \
  PUT_INT32_SLICE(l, local, _b); \
  D_putH ((long) dnetDest, t, (char *) __blk );    \
}

#define write_remote_count(tag,p,local,count) \
{ \
  int t=count; \
  register char *_b = (char *)__blk; \
  check(); \
  PUT_INT32_FAST((int32)(tag),_b); \
  PUT_INT32_FAST((int32)(p),_b); \
  PUT_INT32_SLICE((int860) t, local, _b); \
  t+=8; \
  D_putH ((long)dnetDest, t, (char *) __blk ); \
}

#define read_remote_count(tag,p,local,bytes) \
{ \
  char *_b = (char *)__blk; \
  check(); \
  PUT_INT32_FAST((int32) tag, _b); \
  PUT_INT32_FAST( ((int32)p), _b); \
  D_putH ((long)dnetDest, (long) 8, (char *) __blk ); \
  ack(1); \
  _b = (char *)__blk; \
  GET_INT32_SLICE(bytes, local, _b); \
}

#define read_remote(tag,p,local,type) read_remote_count((tag),(p),(local),sizeof(type))
/*}}}  */
#endif

/*}}}  */

/*{{{  void check_i860_msg ()*/
void check_i860_msg ()
{
  int860 l;

  extern long D_getIS ( short from, long *len, char *message );

  while (inputReady()) {
    D_getIS ( __my_dNet_ID+1, (long *)&l, (char *) __blk );
  }
}
/*}}}  */

/*{{{  scene calls*/
/* scene calls */
/*{{{  void PAZinit  ( int860 device,*/
void PAZinit  ( int860 device,
		int860 x, int860 y,
		int860 link1, int860 link2,  int860 link3,
		int860 magic, int860 magic2, int860 magic3,
		int860 renderer_id )
{
  int l;
  char *b = (char *)__blk;

  PAZ_ABORTCHECK;
  __lastPAZop="PAZinit";

  check();

  PUT_INT32_FAST (viz_init_individ,b);
  PUT_INT32_FAST (device,b);
  PUT_INT32_FAST (x,b);
  PUT_INT32_FAST (y,b);
  PUT_INT32_FAST (link1,b);
  PUT_INT32_FAST (link2,b);
  PUT_INT32_FAST (link3,b);
  PUT_INT32_FAST (magic,b);
  PUT_INT32_FAST (magic2,b);
  PUT_INT32_FAST (magic3,b);
  PUT_INT32_FAST (renderer_id,b);
  l=11*4;

  D_putH (dnetDest, (long) l, (char *) __blk );

  l = (int) ack(1);

  PAZ_EXIT;
}
/*}}}  */
/*{{{  PAZSCENE PAZcreateScene ()*/
PAZSCENE PAZcreateScene ()
{
  PAZ_ABORTCHECK;
  __lastPAZop="PAZcreateScene";


  return ( (PAZSCENE) protocol ( viz_createScene, 0, 1 ));

  PAZ_EXITNULL;
}
/*}}}  */
/*{{{  PAZSCENE PAZdeleteScene ( PAZSCENE handle )*/
PAZSCENE PAZdeleteScene ( PAZSCENE handle )
{
  PAZ_ABORTCHECK;
  __lastPAZop="ack in PAZdeleteScene";

  if (handle==NULL) return NULL;

  ack(0);
  __lastPAZop="PAZdeleteScene";

  protocol ( viz_deleteScene, (int860) handle, 1 );
  return (handle);

  PAZ_EXITZERO;
}
/*}}}  */
/*{{{  void PAZsetScene ( PAZSCENE handle )*/
void PAZsetScene ( PAZSCENE handle )
{
  PAZ_ABORTCHECK;
  __lastPAZop="PAZsetScene";

  protocol ( viz_setScene, (int860) handle, 0 );
  PAZ_EXIT;
}

/*}}}  */
/*{{{  void PAZsetBackGND ( float r,  float g, float b )*/
void PAZsetBackGND ( float r,  float g, float b )
{
  char *bptr = (char *)__blk;
  
  PAZ_ABORTCHECK;
  __lastPAZop="PAZsetBackGND";

  check();

  PUT_INT32_FAST(viz_setBackGND,bptr);
  PUT_FLOAT_FAST(r,bptr);
  PUT_FLOAT_FAST(g,bptr);
  PUT_FLOAT_FAST(b,bptr);

  
  D_putH ( dnetDest, 4*sizeof(int860), (char *) __blk );
  PAZ_EXIT;
}
/*}}}  */
/*}}}  */

/*{{{  object / geometry access calls*/

/*{{{  void PAZopenObject(void)*/
void PAZopenObject(void)
{
  PAZ_ABORTCHECK;
    __lastPAZop="ack in PAZopenObject";


  ack(0);
    __lastPAZop="PAZopenObject";

  string_protocol ( viz_streamObject, "hey, ho, lets go!" );

  PAZ_EXIT;
}
/*}}}  */
/*{{{  void PAZwriteObject(const char *format, ...)*/
void PAZwriteObject(const char *format, ...)
{
  va_list arg_ptr;
  char buf[1024];     /* grandma, what a big buf youve got */

  PAZ_ABORTCHECK;
    __lastPAZop="PAZwriteObject";


  va_start(arg_ptr, format);
  vsprintf(buf, format, arg_ptr);

  if (string_protocol ( viz_fileString, buf ) != viz_fileString) {
    printf ( (char *) (__blk+1)); fflush(stdout);
  }

  PAZ_EXIT;
}
/*}}}  */
/*{{{  PAZOBJECT PAZcloseObject(void)*/
OBJECT *PAZcloseObject(void)
{
  PAZ_ABORTCHECK;
    __lastPAZop="PAZcloseObject";


  if (string_protocol ( viz_fileClose, "Hi" ) != viz_fileClose) {
    printf ( (char *) (__blk+1)); fflush(stdout);
    return (0);
  }
  else {
  
    char *b=(char *) &__blk[1];
    GET_INT32_FAST ( ret, b );
    return (PAZOBJECT) ret;
  }

  PAZ_EXITZERO;
}
/*}}}  */

/*{{{  void PAZopenBinObject()*/
void PAZopenBinObject( void )
{
  PAZ_ABORTCHECK;
    __lastPAZop="ack in PAZopenBinObj";


  ack(0);
    __lastPAZop="PAZopenBinObj";

  protocol ( viz_openBinGeometry, 0, 0 );

  PAZ_EXIT;
}
/*}}}  */
/*{{{  void PAZopenBinLOD( float in, float out )*/
void PAZopenBinLOD( float in, float out )
{
  int *inp =(int *) &in;
  int *outp=(int *) &out;

  PAZ_ABORTCHECK;
    __lastPAZop="PAZopenBinLOD";

  protocol2 ( viz_openBinLOD, *inp, *outp, 0 );

  PAZ_EXIT;
}
/*}}}  */
/*{{{  void PAZopenBinPatch( surface *surf )*/
void PAZopenBinPatch( surface *surf )
{
  PAZ_ABORTCHECK;
    __lastPAZop="PAZopenBinPatch";

  write_remote_count (viz_openBinPatch, surf, surf, sizeof(surface) );
  PAZ_EXIT;
}
/*}}}  */
/*{{{  void PAZopenBinPmesh( surface *surf )*/
void PAZopenBinPmesh( surface *surf )
{
  PAZ_ABORTCHECK;
    __lastPAZop="PAZopenBinPmesh";

  write_remote_count (viz_openBinPmesh, surf, surf, sizeof(surface) );
  PAZ_EXIT;
}
/*}}}  */
/*{{{  void PAZopenBinSmesh( surface *surf )*/
void PAZopenBinSmesh( surface *surf )
{
  PAZ_ABORTCHECK;
    __lastPAZop="PAZopenBinSmesh";

  write_remote_count (viz_openBinSmesh, surf, surf, sizeof(surface) );
  PAZ_EXIT;
}
/*}}}  */
/*{{{  void PAZopenBinMaterial( MATERIAL *m )*/
void PAZopenBinMaterial( MATERIAL *m )
{
  PAZ_ABORTCHECK;
    __lastPAZop="PAZopenBinMaterial";

  write_remote_count (viz_openBinMaterial, m, m, sizeof(MATERIAL ));
  PAZ_EXIT;
}
/*}}}  */
/*{{{  void PAZopenBinTristrip( int vertices )*/
void PAZopenBinTristrip( int vertices )
{
  PAZ_ABORTCHECK;
    __lastPAZop="PAZopenBinTristrip";

  protocol2 ( viz_openBinStrip, 0, vertices, 0 );
  PAZ_EXIT;
}
/*}}}  */
/*{{{  void PAZopenBinPolystrip( int vertices )*/
void PAZopenBinPolystrip( int vertices )
{
  PAZ_ABORTCHECK;
    __lastPAZop="PAZopenBinPolystrip";

  protocol2 ( viz_openBinStrip, 1, vertices, 0 );
  PAZ_EXIT;
}
/*}}}  */
/*{{{  void PAZwriteBinVertices( float *bulk, int vertices, int floats_per_vertex )*/
void PAZwriteBinVertices ( float *bulk, int vertices, int floats_per_vertex )
{
  /*
     the format of the bulk vertices is required to match exactly the
     format required inside the renderer -

     normals  cooked  textured  floats per vertex       meaning

	0        0        0            3                x,y,z
	0        0        1            5                x,y,z,u,v
	0        1        0            6                x,y,z,r,g,b
	0        1        1            8                x,y,z,r,g,b,u,v
	1        0        0            6                x,y,z,dx,dy,dz
	1        0        1            8                x,y,z,dx,dy,dz,u,v
	1        1        0            N/A
	1        1        1            N/A

     this is NOT checked at present ! ! !
     all the floats are just passed thru

    */
  int bytesPerVert=sizeof(float) * floats_per_vertex;
  int bytesPerSquirt=240;
  int vertsPerSquirt=bytesPerSquirt/bytesPerVert;
  int left;
  char *cbulk=(char *) bulk;

  PAZ_ABORTCHECK;
    __lastPAZop="PAZwriteBinVertices";

  for (left=vertices; left > vertsPerSquirt; left-=vertsPerSquirt) {
    int hop=vertsPerSquirt*bytesPerVert;
    write_remote_count ( viz_binGeometry, vertsPerSquirt, cbulk, hop );
    cbulk+=hop;
    ack(1);
  }
  if (left) {
    write_remote_count (viz_binGeometry, left, cbulk, left*bytesPerVert);
    ack(1);
  }

  PAZ_EXIT;
}
/*}}}  */
/*{{{  OBJECT *PAZcloseBinObject(void)*/
OBJECT *PAZcloseBinObject(void)
{
  PAZ_ABORTCHECK;
    __lastPAZop="PAZcloseBinObject";

  return (OBJECT *) protocol ( viz_closeBinGeometry, 0, 1 );
  PAZ_EXITZERO;
}
/*}}}  */

/*{{{  void PAZopenGeometry ( PAZOBJECT o,*/
void PAZopenGeometry ( OBJECT *o,
		       int LODindex,
		       int patchIndex,
		       int floats_per_vert )
{
  register char *b=(char *) __blk;

  PAZ_ABORTCHECK;
    __lastPAZop="PAZopenGeometry";

  ack(0);

  PUT_INT32_FAST ((int860) viz_openGeometry, b);
  PUT_INT32_FAST ((int860) o, b);
  PUT_INT32_FAST ((int860) LODindex, b);
  PUT_INT32_FAST ((int860) patchIndex, b);
  PUT_INT32_FAST ((int860) floats_per_vert, b);

  D_putH ( dnetDest, 5*4, (char *) __blk );

  return;

  PAZ_EXIT;
}

/*}}}  */
/*{{{  void PAZwriteGeometry ( float *mesh, int n_floats )*/
void PAZwriteGeometry ( float *mesh, int n_floats )
{
  register char *b=(char *) __blk;
  int860 *imesh=(int860 *) mesh;
  int t;

  PAZ_ABORTCHECK;
  __lastPAZop="PAZwriteGeometry";
  check();


  PUT_INT32_FAST ((int860) viz_geometry, b);
  PUT_INT32_FAST ((int860) n_floats, b);
  PUT_INT32_SLICE(n_floats<<2, imesh,b);

  t=(n_floats<<2) + 8;
  D_putH ( (long) dnetDest, (long)t, (char *) __blk );
/*  ack(1); */

  return;

  PAZ_EXIT;
}

/*}}}  */
/*{{{  void PAZcloseGeometry ()*/
void PAZcloseGeometry ()
{
  register char *b=(char *) __blk;
  __lastPAZop="PAZcloseGeometry";

  PAZ_ABORTCHECK;
  check();

  PUT_INT32_FAST ((int860) viz_closeGeometry, b);
  D_putH ( dnetDest, 4, (char *) __blk );

  return;

  PAZ_EXIT;
}

/*}}}  */

/*{{{  PAZOBJECT PAZcreateObject ( char *s )*/
PAZOBJECT PAZcreateObject ( char *s, char *(*resolve_texture)(char *) )
{
  /* return ( (PAZOBJECT) stream_file (s)); */
  return ((PAZOBJECT) _read (s, resolve_texture));
}
/*}}}  */

/*{{{  int860 PAZdeleteObject ( PAZOBJECT o )*/
int860 PAZdeleteObject ( PAZOBJECT o )
{
  PAZ_ABORTCHECK;
    __lastPAZop="PAZdeleteObj";
  
  ack(0);

  return (protocol ( viz_deleteObject, (int860) o, 1));
  PAZ_EXITZERO;
}
/*}}}  */
/*{{{  void PAZreadBound ( PAZOBJECT o, POINT* bound )*/
void PAZreadBound ( PAZOBJECT o, POINT* bound )
{
  PAZ_ABORTCHECK;
    __lastPAZop="PAZreadBound";

  read_remote_count(viz_readBounds,o,bound, 8 * sizeof(POINT));
  PAZ_EXIT;
}
/*}}}  */
/*}}}  */
/*{{{  materials*/
/*{{{  PAZMATERIAL PAZdeleteMaterial ( PAZMATERIAL mtl )*/
PAZMATERIAL PAZdeleteMaterial ( PAZMATERIAL mtl )
{
  PAZMATERIAL remote;

  if (mtl==NULL) return NULL;

  remote=mtl->name;

  PAZ_ABORTCHECK;
    __lastPAZop="PAZdeleteMaterial";


  free(mtl);
  
  remote = (PAZMATERIAL) protocol ( viz_deleteMaterial, (int860) remote, 1 );
  return mtl;

  PAZ_EXITZERO;

}
/*}}}  */
/*{{{  void PAZreadMaterial ( PAZMATERIAL mtl )*/
void PAZreadMaterial ( PAZMATERIAL mtl )
{
  PAZMATERIAL remote;

  PAZ_ABORTCHECK;

  if (mtl==NULL) return;
  remote=mtl->name;

  __lastPAZop="PAZreadMaterial";
 
  read_remote_count ( viz_readMaterial, remote, mtl, sizeof(MATERIAL ));

  mtl->name=remote;
  PAZ_EXIT;

}
/*}}}  */
/*{{{  void PAZwriteMaterial ( PAZMATERIAL mtl )*/
void PAZwriteMaterial ( PAZMATERIAL mtl )
{

  PAZ_ABORTCHECK;
  __lastPAZop="PAZwriteMaterial";

  write_remote_count ( viz_writeMaterial, mtl->name, mtl, sizeof(MATERIAL ));
  check();

  PAZ_EXIT;
}
/*}}}  */
/*{{{  PAZMATERIAL PAZcreateMaterial ()*/
PAZMATERIAL PAZcreateMaterial ()
{
  PAZMATERIAL local;
  PAZ_ABORTCHECK;
  __lastPAZop="PAZcreateMaterial";

  local=(PAZMATERIAL) malloc(sizeof(MATERIAL));
  
  if (local==NULL) {
    PAZerror ("malloc fail in createMaterial\n" );
    return(NULL);
  }

  local->name = (PAZMATERIAL) protocol ( viz_createMaterial, 0, 1 );
  
  PAZreadMaterial ( local );

  return (local);
  PAZ_EXITNULL;
}
/*}}}  */
/*}}}  */
/*{{{  texture calls*/
static TEXTURE *__tex=NULL;

/*{{{  void PAZopenTexture ( int s_size, int t_size, int mode, char *name )*/
void PAZopenTexture ( int s_size, int t_size, int mode, char *name )
{
  PAZ_ABORTCHECK;
    __lastPAZop="PAZopenTexture";

  if (__tex==NULL) {
    register char *b=(char *) __blk;
    int860 ret;

    PAZ_ABORTCHECK;

    ack(0);

    PUT_INT32_FAST ((int860) viz_texture, b);
    PUT_INT32_FAST ((int860) s_size, b);
    PUT_INT32_FAST ((int860) t_size, b);
    PUT_INT32_FAST ((int860) mode, b);
    PUT_INT32_FAST ((int860) 0, b);

    memcpy ( b, name, 16 );
    b[15]=0x0;

    D_putH ( dnetDest, (5*4)+16, (char *) __blk );
      ack(1);
	
    b = (char *)&(__blk[1]);
    GET_INT32_FAST(ret,b);

    __tex=(TEXTURE *) ret;

    PAZ_EXIT;
  }
}
/*}}}  */
/*{{{  void PAZtexels ( int860 *map, int texels )*/
void PAZtexels ( int860 *map, int texels )
{
  int860 l;
  register char *b=(char *) __blk;

  PAZ_ABORTCHECK;
    __lastPAZop="PAZtexels";

  if (__tex) {
    int t;

    check();
    PUT_INT32_FAST ( viz_texels, b );
    PUT_INT32_FAST ( texels, b );
    t=texels<<2;
    PUT_INT32_SLICE ( t, map, b );
    t=8+(texels<<2);
    D_putH ((int860) dnetDest, (int860) t, (char *) __blk );

    l=ack(1);
  }
  PAZ_EXIT;
}
/*}}}  */
/*{{{  void PAZfxtexels ( int860 *map, int texels )*/
void PAZfxtexels ( int860 *map, int texels )
{
  int860 l;
  int t;
  register char *b=(char *) __blk;

  PAZ_ABORTCHECK;

    __lastPAZop="PAZfxtexels";


  check();
  PUT_INT32_FAST ( viz_ani_texels, b );
  PUT_INT32_FAST ( texels, b );
  t=texels<<2;
  PUT_INT32_SLICE ( t, map, b );
  t=8+(texels<<2);
  D_putH ((int860) dnetDest, (int860) t, (char *) __blk );

  l=ack(1);
  PAZ_EXIT;
}
/*}}}  */
/*{{{  TEXTURE *PAZcloseTexture ()*/
TEXTURE *PAZcloseTexture ()
{
  TEXTURE *rettex=__tex;

  PAZ_ABORTCHECK;

  __tex=NULL;

  return rettex;

  PAZ_EXITZERO;
}
/*}}}  */

/*{{{  TEXTURE *PAZloadTexture ( int860 *store, int s_size, int t_size, int mode, char *name )*/
TEXTURE *PAZloadTexture ( int860 *map, int s_size, int t_size, int mode, char *name )
{
  /*
     under all current implementations, textures must be a power
     of 2 on each side, and must be >= 64x64. So I can safely just
     load up the whole thing 64 at a time
  */
  int860 texels;

  PAZopenTexture ( s_size, t_size, mode, name );

  for (texels=(int860) (s_size)*(int860) (t_size); texels!=0; texels-=64 ) {
    PAZtexels ( map, 64 );
    map+=64;
  }
  return PAZcloseTexture();

}
/*}}}  */
/*{{{  static TEXTURE *doPAZcreateTexture ( char *fname, int mode, char *texname )*/
static TEXTURE *doPAZcreateTexture ( char *fname, int mode, char *texname )
{
  FILE *fp=fopen(fname, "rb" );
  TEXTURE *tex=(TEXTURE *) malloc(sizeof(TEXTURE));
  int860 *texptr= (int860 *) malloc(128*4);
  long l_edge;
  int edge;

  if (fp==NULL) {
    printf ("failed to open file %s\n", fname );
    return NULL;
  }
  else {
    printf ("opened file %s\n", fname );
    fseek ( fp, 0L, SEEK_END );
    l_edge=ftell(fp);
    fseek ( fp, 0L, SEEK_SET );

    edge=128;

    if      (l_edge == 16384L) edge=64;
    else if (l_edge == 65536L) edge=128;
    else if (l_edge == 262144L) edge=256;
    else 
    {
	printf("Bad texture file size 0x%x\n",l_edge);
	return NULL;
    }

  }

  if (texptr) {
    long texels=edge*edge;

    PAZopenTexture ( edge, edge, mode, texname );

    while (texels) {
	fread ( texptr, 32, 4, fp );
	PAZtexels ( texptr, 32 );
	texels-=32;
    }

    tex->name=PAZcloseTexture();

    free(texptr);
  }
  else {
    printf ("Failed to malloc texture space\n" );
    free(tex);
    tex=NULL;
  }

  fclose (fp);

  return tex;

}

/*}}}  */

/*{{{  support for deferred texture loading*/
typedef struct s_texread {
  struct s_texread *next;
  char texname [TEXTURE_NAME_LENGTH];
  int mode;
  int texture_loaded;
  TEXTURE *PAZtexture;
} texture_item;

static texture_item *texture_head=NULL;
/*}}}  */

/*{{{  TEXTURE *PAZcreateTexture ( char *fname, int mode, char *texname )*/
TEXTURE *PAZcreateTexture ( char *fname, int mode, char *texname )
{
  texture_item *texture=texture_head;

  while (texture) {
    if (strcmp(texture->texname, texname) == 0) {
      printf ("returning found texture %s\n", texture->texname );
      return texture->PAZtexture;
    }
    texture=texture->next;
  }
  texture=(texture_item *) malloc (sizeof(texture_item));

  texture->next=texture_head;
  texture_head =texture;

  strncpy ( texture->texname, texname, TEXTURE_NAME_LENGTH-1 );

  texture->mode=mode;

  texture->PAZtexture=doPAZcreateTexture ( fname, mode, texname );

  return texture->PAZtexture;
}
/*}}}  */

/*}}}  */
/*{{{  pixels*/
/*{{{  void PAZpixels ( int860 *pixels, int x, int y, int n_pixels )*/
void PAZpixels ( int860 *pixels, int x, int y, int n_pixels )
{
  int860 l;
  int t;
  register char *b=(char *) __blk;

  PAZ_ABORTCHECK;
    __lastPAZop="PAZpixels";

  check();

  PUT_INT32_FAST ( viz_readPixels, b );
  PUT_INT32_FAST ( x, b );
  PUT_INT32_FAST ( y, b );
  PUT_INT32_FAST ( n_pixels, b );

  D_putH ((int860) dnetDest, (int860) 16, (char *) __blk );

  l=ack(1);

  b = (char *)__blk;
  b+=4;
  GET_INT32_SLICE ( n_pixels<<2, pixels, b);

  PAZ_EXIT;
}
/*}}}  */
/*}}}  */
/*{{{  fx calls*/
/*{{{  void PAZsfx ( int860 fx_type, float qual, MATRIX m )*/
void PAZsfx ( int860 fx_type, float qual, MATRIX m )
{
  int860 l;
  int t, i;
  register char *b=(char *) __blk;

  PAZ_ABORTCHECK;
  __lastPAZop="PAZsfx";

  check();

  PUT_INT32_FAST ( viz_sfx, b );
  PUT_INT32_FAST ( fx_type, b );
  PUT_FLOAT_FAST ( qual,    b );

  for (i=0; i<4; i++ ) {
    PUT_FLOAT_FAST ( m[i][0], b );
    PUT_FLOAT_FAST ( m[i][1], b );
    PUT_FLOAT_FAST ( m[i][2], b );
    PUT_FLOAT_FAST ( m[i][3], b );
  }

  D_putH ((int860) dnetDest, 19 * sizeof(float), (char *) __blk );

  check();

  PAZ_EXIT;
}
/*}}}  */
/*{{{  void PAZfog ( int860 enable,*/
void PAZfog ( int860 enable,
	      float  neer,
	      float  phar,
	      float  fr,
	      float  fg,
	      float  fb )
{
  int860 l;
  int t, i;
  register char *b=(char *) __blk;

  PAZ_ABORTCHECK;
  __lastPAZop="PAZfog";

  check();

  PUT_INT32_FAST ( viz_fog, b );
  PUT_INT32_FAST ( enable,  b );
  PUT_FLOAT_FAST ( neer,    b );
  PUT_FLOAT_FAST ( phar,    b );
  PUT_FLOAT_FAST ( fr,    b );
  PUT_FLOAT_FAST ( fg,    b );
  PUT_FLOAT_FAST ( fb,    b );

  D_putH ((int860) dnetDest, 7 * sizeof(float), (char *) __blk );

  check();

  PAZ_EXIT;
}
/*}}}  */
/*}}}  */
/*{{{  instance calls*/
/*{{{  PAZINST PAZdeleteInstance ( PAZINST inst )*/
PAZINST PAZdeleteInstance ( PAZINST inst )
{
  PAZINST remote;

  if (inst == NULL) return NULL;
  if (inst->daddy)  return NULL;
  if (inst->link)   return NULL;
  if (inst->nest)   return NULL;

  remote=inst->name;

  PAZ_ABORTCHECK;
    __lastPAZop="PAZdeleteInstance";
  
  ack(0);

  free(inst);

  remote = (PAZINST) protocol ( viz_deleteInstance, (int860) remote, 1 );
  return (inst);

  PAZ_EXITZERO;
}
/*}}}  */
/*{{{  void PAZreadInstance ( PAZINST inst )*/
void PAZreadInstance ( PAZINST inst )
{
  PAZINST remote=inst->name,
	  daddy=inst->daddy,
	  nest=inst->nest,
	  link=inst->link;

  PAZ_ABORTCHECK;
    __lastPAZop="PAZreadInstance";


  read_remote_count (viz_readInstance, remote, inst, sizeof(INSTANCE ));

  inst->name=remote;
  inst->nest=nest;
  inst->link=link;
  inst->daddy=daddy;

  PAZ_EXIT;
}
/*}}}  */
/*{{{  void PAZwriteInstance ( PAZINST inst )*/
void PAZwriteInstance ( PAZINST inst )
{
  PAZINST remote   =inst->name,
	  rd, daddy=inst->daddy,
	  rn, nest =inst->nest,
	  rl, link =inst->link;   
	  
  PAZ_ABORTCHECK;
    __lastPAZop="PAZwriteInstance";

  /*{{{  fix up to remote pointers before writing*/
  rd=daddy;
  rn=nest;
  rl=link;
  
  if (nest !=NULL) inst->nest  = nest ->name;
  if (daddy!=NULL) inst->daddy = daddy->name;
  if (link !=NULL) inst->link  = link ->name;
  /*}}}  */

  write_remote_count (viz_writeInstance, remote, inst, sizeof(INSTANCE ));
  check();

  /*{{{  fix back*/
  inst->nest =rn;
  inst->link =rl;
  inst->daddy=rd;
  
  /*}}}  */

  PAZ_EXIT;

}
/*}}}  */
/*{{{  PAZINST PAZcreateInstance ()*/
PAZINST PAZcreateInstance (void)
{
  PAZINST local=(PAZINST) malloc(sizeof(INSTANCE));

  PAZ_ABORTCHECK;
    __lastPAZop="PAZcreateInstance";


  if (local==NULL) {
    PAZerror ("malloc fail in createInstance\n" );
    return(NULL);
  }

  local->name = (PAZINST) protocol ( viz_createInstance, 0, 1 );
  
  PAZreadInstance ( local );

  local->daddy=NULL;
  local->link=NULL;
  local->nest=NULL;
  
  return (local);

  PAZ_EXITZERO;
}
/*}}}  */

/*{{{  void PAZwriteMatrix ( PAZINST inst )*/
void PAZwriteMatrix ( PAZINST inst )
{
  PAZ_ABORTCHECK;
  __lastPAZop="PAZwriteMatrix";

  write_remote_count ( viz_writeMatrix, inst->name, inst->f, sizeof(MATRIX ));
  check();

  PAZ_EXIT;
}
/*}}}  */

/*{{{  PAZINST PAZsectPixel ( POINT sect, float x, float y, int dot )*/
PAZINST PAZsectPixel ( POINT sect, float x, float y, int dot )
{
  PAZ_ABORTCHECK;
  __lastPAZop="PAZsectPixel";
  
  {
    int860 ret;
    register char *_b = (char *)__blk;
    check();

    PUT_INT32_FAST(viz_sectPixel,_b);
    PUT_FLOAT_FAST(x,_b);
    PUT_FLOAT_FAST(y,_b);
    PUT_INT32_FAST(dot,_b);

    D_putH (dnetDest, 16, (char *) __blk );

    _b = (char *)__blk;

    ack(1);

    GET_INT32_FAST ( ret, _b );

    GET_INT32_FAST ( ret, _b );
    GET_FLOAT_FAST ( sect[0], _b );
    GET_FLOAT_FAST ( sect[1], _b );
    GET_FLOAT_FAST ( sect[2], _b );

    return (PAZINST) ret;
  }


  PAZ_EXITZERO;
}
/*}}}  */
/*{{{  PAZINST PAZsectVector ( POINT sect,*/
PAZINST PAZsectVector ( POINT sect,
			float x0, float y0, float z0,
			float x1, float y1, float z1 )
{
  PAZ_ABORTCHECK;
  __lastPAZop="PAZsectVector";
  
  {
    int860 ret;
    register char *_b = (char *)__blk;
    check();

    PUT_INT32_FAST(viz_sectVector,_b);
    PUT_FLOAT_FAST(x0,_b);
    PUT_FLOAT_FAST(y0,_b);
    PUT_FLOAT_FAST(z0,_b);
    PUT_FLOAT_FAST(x1,_b);
    PUT_FLOAT_FAST(y1,_b);
    PUT_FLOAT_FAST(z1,_b);

    D_putH (dnetDest, 28, (char *) __blk );

    _b = (char *)__blk;

    ack(1);

    GET_INT32_FAST ( ret, _b );

    GET_INT32_FAST ( ret, _b );
    GET_FLOAT_FAST ( sect[0], _b );
    GET_FLOAT_FAST ( sect[1], _b );
    GET_FLOAT_FAST ( sect[2], _b );

    return (PAZINST) ret;
  }


  PAZ_EXITZERO;
}
/*}}}  */

/*{{{  heirarchy support*/
/*
    make VERY sure we dont shaft remote pointers
*/

/*{{{  INSTANCE *PAZlink ( INSTANCE *parent, INSTANCE *chain )*/
INSTANCE *PAZlink ( INSTANCE *parent, INSTANCE *chain )
{
  PAZ_ABORTCHECK;
  __lastPAZop="PAZlink";

  if (parent->link == NULL) {
    parent->link=chain;

    chain->daddy=parent;
 
    protocol2 ( viz_link, (int860) parent->name, (int860) chain->name, 0 );

    return(chain);
  }
  else
    return(NULL);

  PAZ_EXITZERO;
}
/*}}}  */
/*{{{  INSTANCE *PAZnest ( INSTANCE *parent, INSTANCE *child )*/
INSTANCE *PAZnest ( INSTANCE *parent, INSTANCE *child )
{
  PAZ_ABORTCHECK;
    __lastPAZop="PAZnest";


  if (parent->nest == NULL) {
    parent->nest=child;

    child->daddy=parent;
    protocol2 ( viz_nest, (int860) parent->name, (int860) child->name, 0 );

    return(child);
  }
  else
    return(NULL);

  PAZ_EXITZERO;
}
/*}}}  */

/*{{{  INSTANCE *PAZunlink ( INSTANCE *item )*/
INSTANCE *PAZunlink ( INSTANCE *item )
{
  INSTANCE *parent;
  if (item==NULL) return NULL;
  else parent=item->daddy;

  PAZ_ABORTCHECK;
    __lastPAZop="PAZunlink";


  if (parent != NULL) {
    if (parent->link == item) {
      parent->link=NULL;
      item->daddy=NULL;

      protocol ( viz_unlink, (int860) item->name, 0 );
    }
  }
  return (parent);

  PAZ_EXITZERO;
}
/*}}}  */
/*{{{  INSTANCE *PAZunnest ( INSTANCE *item )*/
INSTANCE *PAZunnest ( INSTANCE *item )
{
  INSTANCE *parent;

  if (item==NULL) return NULL;
  else parent=item->daddy;

  PAZ_ABORTCHECK;
    __lastPAZop="PAZunnest";

  

  if (parent != NULL) {
    if (parent->nest == item) {
      parent->nest=NULL;
      item->daddy=NULL;

      protocol ( viz_unnest, (int860) item->name, 0 );
    }
  }
  return (parent);

  PAZ_EXITZERO;
}
/*}}}  */

/*{{{  static void _deleteTree ( PAZINST root )*/
static void _deleteTree ( PAZINST root )
{
  if (root) {
    _deleteTree ( root->nest );
    _deleteTree ( root->link );
    free ( root );
  }
}
/*}}}  */
/*{{{  void PAZdeleteTree ( PAZINST root )*/
PAZINST PAZdeleteTree ( PAZINST root )
{
  PAZ_ABORTCHECK;
    __lastPAZop="PAZdeleteTree";


  if (root==NULL)
    return NULL;
  else {
    PAZINST remote=root->name;

    _deleteTree ( root );
    ack(0);
    protocol ( viz_deleteTree, (int860) remote, 1 );

    return root;
  }
  PAZ_EXITZERO;
}
/*}}}  */

/*}}}  */
/*}}}  */
/*{{{  lighting calls*/
/*{{{  PAZLIGHT PAZdeleteLight ( PAZLIGHT light )*/
PAZLIGHT PAZdeleteLight ( PAZLIGHT light )
{
  PAZLIGHT remote;

  if (light == NULL) return NULL;

  remote=light->name;
    __lastPAZop="PAZdeleteLight";


  PAZ_ABORTCHECK;

  free(light);
  
  remote = (PAZLIGHT) protocol ( viz_deleteLight, (int860) remote, 1 );

  return light;

  PAZ_EXITZERO;
}
/*}}}  */
/*{{{  void PAZreadLight ( PAZLIGHT light )*/
void PAZreadLight ( PAZLIGHT light )
{
  PAZLIGHT remote=light->name;

  PAZ_ABORTCHECK;
  __lastPAZop="PAZreadLight";

  read_remote_count (viz_readLight, remote, light, sizeof(LIGHTSOURCE ));
  light->name=remote;

  PAZ_EXIT;
}
/*}}}  */
/*{{{  void PAZwriteLight ( PAZLIGHT light )*/
void PAZwriteLight ( PAZLIGHT light )
{
  PAZ_ABORTCHECK;
    __lastPAZop="PAZwriteLight";

  write_remote_count (viz_writeLight, light->name, light, sizeof(LIGHTSOURCE ));
  check();

  PAZ_EXIT;
}
/*}}}  */
/*{{{  PAZLIGHT PAZcreateLight ()*/
PAZLIGHT PAZcreateLight ()
{
  PAZLIGHT local=(PAZLIGHT) malloc(sizeof(LIGHTSOURCE));

  PAZ_ABORTCHECK;
    __lastPAZop="PAZcreateLight";


  if (local==NULL) {
    PAZerror("malloc fail in createLight\n" );
    return(NULL);
  }

  local->name = (PAZLIGHT) protocol ( viz_createLight, 0, 1 );
  PAZreadLight ( local );

  return (local);

  PAZ_EXITZERO;
}
/*}}}  */

/*{{{  void PAZinitLight ( PAZLIGHT v, int860 type,      NOT YET*/
void PAZinitLight ( PAZLIGHT v, int860 type,
		    float a, float b, float c, float x, float y, float z )
{
/*
  printf ("DPL _ PAZinitLight\n"); fflush(stdout);
*/
  v->positional=type;
  v->position[0]=x;
  v->position[1]=y;
  v->position[2]=z;
  v->colour[0]=a;
  v->colour[1]=b;
  v->colour[2]=c;
}
/*}}}  */
/*}}}  */
/*{{{  viewing calls*/
/*{{{  PAZVIEW PAZdeleteView ( PAZVIEW view )*/
PAZVIEW PAZdeleteView ( PAZVIEW view )
{
  PAZVIEW remote;

  if (view==NULL) return NULL;

  remote=view->name;

  PAZ_ABORTCHECK;
    __lastPAZop="PAZdeleteView";


  free(view);
  
  remote = (PAZVIEW) protocol ( viz_deleteView, (int860) remote, 1 );
  return view;

  PAZ_EXITZERO;

}
/*}}}  */
/*{{{  void PAZreadView ( PAZVIEW view )*/
void PAZreadView ( PAZVIEW view )
{
  PAZVIEW remote=view->name;

  PAZ_ABORTCHECK;
    __lastPAZop="PAZreadView";

 
  read_remote_count (viz_readView, remote, view, sizeof(VIEW ));


#ifdef VERBOSE
	PAZinform ("Read view, buffer_A=0x%lx buffer_B=0x%lx buffer_flip=0x%lx\n",
     (int860) view->buffer_A,
     (int860) view->buffer_B,
     (int860) view->buffer_flip);
#endif
  view->name=remote;
  PAZ_EXIT;

}
/*}}}  */
/*{{{  void PAZwriteView ( PAZVIEW view )*/
void PAZwriteView ( PAZVIEW view )
{

  PAZ_ABORTCHECK;
    __lastPAZop="PAZwriteView";

 
  write_remote_count (viz_writeView, view->name, view, sizeof(VIEW ));
  check();

  PAZ_EXIT;
}
/*}}}  */
/*{{{  void PAZwriteMatrix ( PAZINST inst )*/
void PAZwriteViewMatrix ( PAZVIEW inst )
{
  PAZ_ABORTCHECK;
  __lastPAZop="PAZwriteMatrix";

  write_remote_count ( viz_writeViewMat, inst->name, inst->f, sizeof(MATRIX ));
  check();

  PAZ_EXIT;
}
/*}}}  */
/*{{{  PAZVIEW PAZcreateView ()*/
PAZVIEW PAZcreateView ()
{
  PAZVIEW local;
  PAZ_ABORTCHECK;
    __lastPAZop="PAZcreateView";


  local=(PAZVIEW) malloc(sizeof(VIEW));
  
  if (local==NULL) {
    PAZerror ("malloc fail in createView\n" );
    return(NULL);
  }

  local->name = (PAZVIEW) protocol ( viz_createView, 0, 1 );
  
  PAZreadView ( local );

  return (local);
  PAZ_EXITNULL;
}
/*}}}  */

/*{{{  void PAZinitView ( PAZVIEW v, float d, float near, float far, float x, float y,*/
void PAZinitView ( PAZVIEW v, float d, float neer, float phar, float x, float y,
			 float aspect, int device )
{
  v->d=d;
  v->enable=1;
  v->yon=phar;
  v->hither=neer;

  v->screen_width=x;
  v->screen_half_width=x/2;

  v->screen_height=y;
  v->screen_half_height=y/2;

  v->device=device;

  v->shift_x=0;
  v->shift_y=0;

  v->zscale=neer / d;
  v->zmin  =neer / phar;

  v->zmunge=1.0f / (1.0f - v->zmin);
  v->aspect_ratio = aspect;

  _idmatrix ( v->f );
  _idmatrix ( v->b );

}
/*}}}  */
/*}}}  */
/*{{{  control calls*/
/*{{{  void PAZerrStatus ()*/
int860 PAZerrStatus ( char *message )
{
  int860 stat;
  char *str=(char *) (__blk+2);

  PAZ_ABORTCHECK;

  stat = protocol ( viz_errStatus, 0, 1 );

  memcpy ( message, str, 1+strlen(str));

  return stat;
 
  PAZ_EXITZERO;
}
/*}}}  */
/*{{{  long PAZversion ( char *version_string )*/
long PAZversion ( char *version_string )
{
  int major=2;
  int minor=0;

  if (version_string != NULL)
    strcpy ( version_string, "PAZ v 2.0 3-10-92\0" );

  major=(major<<8) | minor;

  return (long) major;
}
/*}}}  */
/*{{{  void PAZrenderScene ()*/


void PAZrenderScene ()
{
  PAZ_ABORTCHECK;
    __lastPAZop="ack in PAZrenderScene";


  ack(0);
    __lastPAZop="PAZrenderScene";


  __PAZ_triangles=renderer_returnValue&0xffff;
  __PAZ_millisecs=(renderer_returnValue>>16)&0xffff;

  protocol ( viz_renderScene, 0, 0 );
  buffer_packets=0;

  ack_pending=__acks_per_frame;

  PAZ_EXIT;
}
/*}}}  */

/*{{{  int860 PAZpollAck(void)*/
int860 PAZpollAck(void)
{
  PAZ_ABORTCHECK;
 
  if (ack_pending)
    return ((int860) inputReady());
  else
    return (1);

  PAZ_EXITZERO;
}
/*}}}  */
/*}}}  */
