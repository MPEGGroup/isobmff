/***********************************************************************************
 
 This software module was originally developed by 
 
 Fraunhofer IIS
 
 in the course of development of the ISO/IEC 23008-3 for reference purposes and its 
 performance may not have been optimized. This software module is an implementation
 of one or more tools as specified by the ISO/IEC 23008-3 standard. ISO/IEC gives 
 you a royalty-free, worldwide, non-exclusive, copyright license to copy, distribute, 
 and make derivative works of this software module or modifications  thereof for use
 in implementations or products claiming conformance to the ISO/IEC 23008-3 standard 
 and which satisfy any specified conformance criteria. Those intending to use this 
 software module in products are advised that its use may infringe existing patents. 
 ISO/IEC have no liability for use of this software module or modifications thereof. 
 Copyright is not released for products that do not conform to the ISO/IEC 23008-3 
 standard.
 
 Fraunhofer IIS retains full right to modify and use the code for its own purpose,
 assign or donate the code to a third party and to inhibit third parties from using 
 the code for products that do not conform to MPEG-related ITU Recommendations and/or 
 ISO/IEC International Standards.
 
 This copyright notice must be included in all copies or derivative works. 
 
 Copyright (c) ISO/IEC 2013.
 
 ***********************************************************************************/

/*
  Main functionality:
  - use of external character buffer
  - allocation on stack (no internal dynamic memory allocation)
  - handle/struct can be copied for easy bitstream 'duplication'
  - no asserts, error handling
*/

#ifndef __READONLYBITBUF_H__
#define __READONLYBITBUF_H__

#ifndef ROBITBUFAPI
  #if defined(WIN32) || defined(WIN64)
    #define ROBITBUFAPI __stdcall
  #else
    #define ROBITBUFAPI
  #endif
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#if defined(WIN32) || defined(WIN64)
#pragma pack(push, 8)
#endif


/* struct definition is public, because user must know the size */
struct readonlybitbuf {
  const unsigned char* charBuffer; /* pointer to external bit buffer */
  int            charBufSize;      /* buffer size in bits */
  int            charBufPos;       /* position in char buffer */
  int            startPos;         /* start position in bits */

  unsigned long  cache;            /* cacheword */
  int            bitsInCache; 

  int            bitsRead;
};

/* Opaque declaration of bitbuffer handle  */
/*struct readonlybitbuf;*/
typedef struct readonlybitbuf  robitbuf;
typedef struct readonlybitbuf* robitbufHandle;


/*---------- methods -----------*/

/** Initializes the bitbuffer. 
    The handle has to be allocated externally.
    if set either extBuffer or extBufSize to NULL/0, the buffer will behave like an empty buffer.

    self       - pointer to bitbuffer structure
    extBuffer  - external character buffer
    extBufSize - external buffer size in bits (from the startPos bit)
    startPos   - starting read position in bits
*/
void ROBITBUFAPI robitbuf_Init(
                               robitbufHandle self,
                               const unsigned char *extBuffer,
                               int extBufSize,
                               int startPos
                               );


/** Resets the bitbuffer.

    self - pointer to bitbuffer structure
*/
void ROBITBUFAPI robitbuf_Reset(robitbufHandle self);


/** Copies the bitbuffer internal states. 
    The handle has to be allocated externally.

    desHandle - pointer to destination bitbuffer structure
    desHandle - pointer to reference bitbuffer structure
*/
void ROBITBUFAPI robitbuf_Copy(
                               robitbufHandle desHandle,
                               robitbufHandle refHandle
                               );


/** Returns the internal character buffer size. 

    self - pointer to bitbuffer structure
    
    return - size of character buffer in bits
*/
int ROBITBUFAPI robitbuf_GetBufferSize(robitbufHandle self);


/** Reads bits from buffer. 
    If we go out of range, the read operation will return zeros until we go back into the buffer.
    Does not read more than 32 bits. If requested more, it will return the last 32 bist of requested amount.
    It will ignore negative request.

    self - pointer to bitbuffer structure
    nBits - number of bits to be read

    return - requested bits
*/
unsigned long ROBITBUFAPI robitbuf_ReadBits(
                                            robitbufHandle self,
                                            int nBits
                                            );


/** Reads bytes from buffer. 
    If we go out of buffer size it will return zeros.

    self - pointer to bitbuffer structure
    desBuffer - destination character array
    nBytes - number of bytes to be read
*/
void ROBITBUFAPI robitbuf_ReadBytes(
                                    robitbufHandle self,
                                    unsigned char *desBuffer,
                                    int nBytes
                                    );


/** Pushes bits again to the buffer. 
    If we go out of range, the read operation will return zeros until we go back into the buffer.
    In case of negative request it will go forward.

    self - pointer to bitbuffer structure
    nbits - number of bits be pushed back
*/
void ROBITBUFAPI robitbuf_PushBack(
                                   robitbufHandle self,
                                   int nBits
                                   );


/** Aligns the read pointer to the next byte. 

    self - pointer to bitbuffer structure
*/
void ROBITBUFAPI robitbuf_ByteAlign(robitbufHandle self);


/** Returns the number of bits available to read operation.
    Can be negative if we move out of the buffer.

    self - pointer to bitbuffer structure

    return - number of bits
*/
int ROBITBUFAPI robitbuf_GetBitsAvail(robitbufHandle self);


/** Returns the number of bits read since initialization or reset.
    Can be negative if we move out of the buffer.

    self - pointer to bitbuffer structure

    return - number of bits
*/
int ROBITBUFAPI robitbuf_GetBitsRead(robitbufHandle self);

#if defined(WIN32) || defined(WIN64)
#pragma pack(pop)
#endif

#ifdef __cplusplus
}
#endif

#endif  /*  __READONLYBITBUF_H__   */
