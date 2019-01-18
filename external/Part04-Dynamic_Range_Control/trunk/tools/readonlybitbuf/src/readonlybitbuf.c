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

#include "readonlybitbuf.h"
#include <stdio.h>


/*------ private: -----------*/

/* returns a string of 1 */
static unsigned long GetMask(int size)
{
  return (1 << size) -1; 
}


/* reads one byte from character buffer */
/* this function has to be very secure, because it reads from the char buffer */
static unsigned int GetByte(robitbufHandle self)
{
  unsigned int tmp = 0;
  int byteSize, bitSize, bitRest;

  /* return zero if no buffer */
  if (self->charBuffer == NULL)
    return tmp;

  bitSize = self->charBufSize + self->startPos;

  byteSize = (bitSize + 7) >> 3; /* size of buffer in bytes */
  bitRest = bitSize - 8 * (byteSize - 1); /* n of bits in the last byte */

  /* check if we are inside the buffer, otherwise return 0 */
  if (self->charBufPos >= 0 && self->charBufPos < byteSize) {
  
    /* get the byte from buffer */
    tmp = self->charBuffer[self->charBufPos];
    
    /* check if the byte is full */
    if (self->charBufPos == (byteSize-1) && bitRest != 8)
      tmp &= (GetMask(bitRest) << (8-bitRest));

  }

  self->charBufPos++;
  
  return tmp;
}


/*-------------- public: -------------*/

void ROBITBUFAPI robitbuf_Init(
                               robitbufHandle self,
                               const unsigned char *extBuffer,
                               int extBufSize,
                               int startPos
                               )
{
  if (self == NULL) return;

  self->charBuffer  = NULL;
  self->charBufSize = 0;
  self->charBufPos  = 0;
  self->startPos    = startPos;

  self->cache       = 0;
  self->bitsInCache = -startPos;

  self->bitsRead    = 0;

  self->charBuffer  = extBuffer;
  self->charBufSize = (extBufSize > 0) ? extBufSize : 0;
}


void ROBITBUFAPI robitbuf_Reset(robitbufHandle self)
{
  if (self == NULL) return;

  self->charBufPos = 0;

  self->cache = 0;
  self->bitsInCache = -self->startPos;

  self->bitsRead = 0;  
}


void ROBITBUFAPI robitbuf_Copy(
                               robitbufHandle desHandle,
                               robitbufHandle refHandle
                               )
{
  if (desHandle == NULL)  return;
  if (refHandle == NULL)  return;

  desHandle->charBuffer  = refHandle->charBuffer;
  desHandle->charBufSize = refHandle->charBufSize;
  desHandle->charBufPos  = refHandle->charBufPos;
  desHandle->startPos    = refHandle->startPos;

  desHandle->cache       = refHandle->cache;
  desHandle->bitsInCache = refHandle->bitsInCache;

  desHandle->bitsRead    = refHandle->bitsRead;
}


int ROBITBUFAPI robitbuf_GetBufferSize(robitbufHandle self)
{
  if (self == NULL) return 0;

  return self->charBufSize;
}


unsigned long ROBITBUFAPI robitbuf_ReadBits(
                                            robitbufHandle self,
                                            int nBits
                                            )
{
  unsigned long tmp = 0;
  int bitsToRead, bitsToDrop, restBits;

  if (self == NULL) return 0;

  /* accelerate the routine */
  if (nBits > 32) {
    bitsToDrop = nBits - 32;
    self->bitsRead += bitsToDrop;
    self->bitsInCache -= bitsToDrop;
    nBits -= bitsToDrop;
  }
  
  while (nBits > 0) {

    /* skip bytes that we don't read */
    while (self->bitsInCache <= -8) {
      self->bitsInCache += 8;
      self->charBufPos++;
    }
    
    /* fill cache from buffer */
    if (self->bitsInCache < 16) {
      unsigned int tmpHi, tmpLo;

      self->cache <<= 16;
      tmpHi = GetByte(self);
      tmpLo = GetByte(self);
      self->cache |= ( (tmpHi << 8) | tmpLo );
      self->bitsInCache += 16;
    }

    if (nBits > self->bitsInCache)
      bitsToRead = self->bitsInCache;
    else
      bitsToRead = nBits;

    restBits = self->bitsInCache - bitsToRead;

    tmp <<= bitsToRead;
    /* get proper bits from cache */
    tmp |= ((self->cache >> restBits) & GetMask(bitsToRead)); 

    nBits -= bitsToRead;
    self->bitsInCache -= bitsToRead;

    self->bitsRead += bitsToRead;
   
  }

  return tmp;
}


void ROBITBUFAPI robitbuf_ReadBytes(
                                    robitbufHandle self,
                                    unsigned char *desBuffer,
                                    int nBytes
                                    )
{
  int i;

  if (self == NULL) return;

  for(i=0; i<nBytes; i++) {
    desBuffer[i] = (unsigned char)robitbuf_ReadBits(self, 8);
  }
}


void ROBITBUFAPI robitbuf_PushBack(
                                   robitbufHandle self,
                                   int nBits
                                   )
{
  int leftBits  = 0;
  int leftBytes = 0;

  if (self == NULL) return;

  /* rewind the buffer, don't mind if we can get out of range */

  self->bitsRead -= nBits;

  nBits += self->bitsInCache;
  self->bitsInCache = 0;

  if (nBits >= 0)
    leftBytes = nBits / 8 + 1;
  else
    leftBytes = nBits / 8;

  leftBits = leftBytes * 8 - nBits;

  self->charBufPos -= leftBytes;

  /* if set the negative value of bits in cache, the cache will be re-initialized during next read */
  self->bitsInCache -= leftBits;
 
}


void ROBITBUFAPI robitbuf_ByteAlign(robitbufHandle self)
{
  int bitsToDrop = 0;

  if (self == NULL) return;

  /* the number of bits in cache can be negative */
  if (self->bitsInCache >= 0)
    bitsToDrop = self->bitsInCache % 8;
  else
    bitsToDrop = (8 - ((-self->bitsInCache) % 8)) % 8; /* mod from negative value */

  /* trunc not needed bits from cache */
  self->bitsInCache -= bitsToDrop;

  self->bitsRead += bitsToDrop;
}

int ROBITBUFAPI robitbuf_GetBitsAvail(robitbufHandle self)
{
  if (self == NULL) return -1;

  return self->charBufSize - self->bitsRead;
}

int ROBITBUFAPI robitbuf_GetBitsRead(robitbufHandle self)
{
  if (self == NULL) return -1;

  return self->bitsRead;
}
