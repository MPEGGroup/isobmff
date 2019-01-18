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

#ifndef __WRITEONLYBITBUF_H__
#define __WRITEONLYBITBUF_H__

#ifndef WOBITBUFAPI
  #if defined (WIN32)
    #define WOBITBUFAPI __stdcall
  #else
    #define WOBITBUFAPI
  #endif
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#if defined (WIN32)
#pragma pack(push, 8)
#endif


/* struct definition is public, because user must know the size */
struct writeonlybitbuf {
  unsigned char* charBuffer;  /* pointer to external bit buffer */
  int            charBufSize; /* buffer size in bits */                    
  int            charBufPos;  /* byte-position in char buffer             |----  byte ----||<- charBufPos       */
  int            bitPos;      /* bit-position in byte of char buffer      |x|x|x|x|x|x|x|x||x|x|x|x|x|x|x|x|
                                                                                                 |<- bitPos     */
  int            startPos;    /* start position in bits */
  int            endPos;      /* end position in bits */

  int            bitsWritten; /* number of written bits since last init/reset; */
};

typedef struct writeonlybitbuf  wobitbuf;
typedef struct writeonlybitbuf* wobitbufHandle;


/*---------- methods -----------*/

/** Initializes the bitbuffer. 
    The handle has to be allocated externally.
    If set either extBuffer or extBufSize to NULL/0, the buffer will behave like an empty buffer.

    self       - pointer to bitbuffer structure
    extBuffer  - external character buffer
    extBufSize - external buffer size in bits (from the startPos bit)
    startPos   - starting write position in bits

	  return	   - error if extBuffer == NULL || self == NULL || extBuffSize <= 0 || startPos < 0
*/
int WOBITBUFAPI wobitbuf_Init(
                              wobitbufHandle self,
                              unsigned char *extBuffer,
                              int extBufSize,
                              int startPos
                              );


/** Resets the bitbuffer.
    This function resets the write-pointer of
    the buffer. All other values remain the same.
    Does not clear/zero the bitbuffer itself.

    self - pointer to bitbuffer structure
*/
void WOBITBUFAPI wobitbuf_Reset(wobitbufHandle self);


/** Copies the bitbuffer internal states. 
    The handle has to be allocated externally.

    desHandle - pointer to destination bitbuffer structure
    refHandle - pointer to reference bitbuffer structure
*/
void WOBITBUFAPI wobitbuf_Copy(
                               wobitbufHandle desHandle,
                               wobitbufHandle refHandle
                               );


/** Returns the internal character buffer size. 
    If self == NULL the function returns 0.
      
    self - pointer to bitbuffer structure
    
    return - size of character buffer in bits
*/
int WOBITBUFAPI wobitbuf_GetBufferSize(wobitbufHandle self);


/** Writes bits to buffer. 
    If we go out of range, the write operation returns an error.
    If the function is called with nBits greater than 32 or below
    1, the write-request is ignored.

    self  - pointer to bitbuffer structure
    data  - data to be written
    nBits - number of bits to be written

    return - error code, zero for success
*/
int WOBITBUFAPI wobitbuf_WriteBits(
                                   wobitbufHandle self,
                                   unsigned int data,
                                   int nBits
                                   );


/** Writes bytes to buffer. 
    If we go out of buffer size an error is returned.

    self - pointer to bitbuffer structure
    srcBuffer - external source character array
    nBytes - number of bytes to be written
*/
int WOBITBUFAPI wobitbuf_WriteBytes(
                                    wobitbufHandle self,
                                    unsigned char *srcBuffer,
                                    int nBytes
                                    );


/** Moves write pointer.
    If we go out of range, an error is returned.
    In case of negative request it will go backward.

    self - pointer to bitbuffer structure
    nbits - number of bits to move

	  return - error (1) if (self == NULL || out of range)
*/
int WOBITBUFAPI wobitbuf_Seek(
                              wobitbufHandle self,
                              int nBits
                              );


/** Aligns the read pointer to the next byte with respect to the start position. 
    The skipped adat area is not touched.

    self - pointer to bitbuffer structure
*/
void WOBITBUFAPI wobitbuf_ByteAlign(wobitbufHandle self);


/** Aligns the read pointer to the next byte with respect to the start position. 
    The skipped adat area is set to 'value'.

    self  - pointer to bitbuffer structure
    value - can be 0 or 1
*/
void WOBITBUFAPI wobitbuf_ByteAlignSet(wobitbufHandle self, int value);


/** Returns the number of bits available to write operation.
    This function returns the number of bits between current
    write-pointer-position and end-of-buffer.

    self - pointer to bitbuffer structure

    return - number of bits
*/
int WOBITBUFAPI wobitbuf_GetBitsAvail(wobitbufHandle self);


/** Returns the current write-pointer-position.
    This function returns the current write-pointer-position with respect 
    to the start position. 
    
    self - pointer to bitbuffer structure

    return - number of bits
*/
int WOBITBUFAPI wobitbuf_GetBufPos(wobitbufHandle self);


/** Returns the number of bits written since initialization or reset.
        
    self - pointer to bitbuffer structure

    return - number of bits
*/
int WOBITBUFAPI wobitbuf_GetBitsWritten(wobitbufHandle self);


#if defined(WIN32)
#pragma pack(pop)
#endif

#ifdef __cplusplus
}
#endif

#endif  /*  __WRITEONLYBITBUF_H__   */
