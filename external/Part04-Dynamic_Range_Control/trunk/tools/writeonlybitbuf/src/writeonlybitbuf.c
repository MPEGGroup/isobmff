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

#include "writeonlybitbuf.h"

#include <stdio.h>

/*------ private: -----------*/

/* returns a string of 1 */
static unsigned long GetMask(int size)
{
  return (1 << size) -1; 
}


/* returns the charecter for ByteAlignSet */
static unsigned char getSetValue(int value, int nBits)
{
  int tmp = 0;
  
  if(value){
    tmp = GetMask(nBits);
  }

  return (unsigned char) tmp;
}


/* writes one byte of data to array */
static void WriteByte(wobitbufHandle self,
                      unsigned char data)
{
  
  if(self->bitPos){
    int restBits      = self->bitPos;
    int bitsToFill    = 8 - restBits; /* bits to fill till Byte-border */
    unsigned char bs  = self->charBuffer[self->charBufPos];
    unsigned char tmp = '\0';

    /* fill Bits till Byte-border */
    tmp |= (data >> restBits);
    bs  >>= bitsToFill;  /* zero Bits we     */
    bs  <<= bitsToFill;  /* want to write to */
    bs  |= tmp;
    self->charBuffer[self->charBufPos] = bs;
    self->charBufPos++;
    
    /* fill rest of Bits */
    tmp = (data << bitsToFill);
    bs  = self->charBuffer[self->charBufPos] & (unsigned char) GetMask(bitsToFill);
    bs  |= tmp;
    self->charBuffer[self->charBufPos] = bs;
  } else {
    self->charBuffer[self->charBufPos] = data;
    self->charBufPos++;
  }
  
  self->bitsWritten += 8;
}
  
/* writes less than 1 Byte to array */
static void WriteBitsUnderEight(wobitbufHandle self,
                                unsigned char data,
                                int nbits)
{
  
  unsigned char bs;
  unsigned char tmp = '\0';
  
  if(self->bitPos + nbits >= 8){
    
    int bitsToFill    = 8 - self->bitPos; /* bits to fill till Byte-border */
    bs  = self->charBuffer[self->charBufPos];

    /* fill Bits till Byte-border */
    tmp |= ( (data >> (nbits - bitsToFill)) & GetMask(bitsToFill) );
    bs  >>= bitsToFill;  /* zero Bits we     */
    bs  <<= bitsToFill;  /* want to write to */
    bs  |= tmp;
    self->charBuffer[self->charBufPos] = bs;
    self->charBufPos++;
    self->bitPos = 0;
    nbits -= bitsToFill;
    self->bitsWritten += bitsToFill;
  }
  
  if(nbits){
    
    unsigned char help_bs;
    int offset    = self->bitPos;
    
    bs = self->charBuffer[self->charBufPos];
    tmp          = '\0';

    /* fill rest of Bits */
    tmp = (data & (unsigned char) GetMask(nbits)) << (8 - nbits - offset);
    help_bs = bs & (unsigned char) GetMask(8 - nbits - offset);
    bs >>= (8 - offset);
    bs <<= (8 - offset);
    bs  |= (tmp | help_bs);
    self->charBuffer[self->charBufPos] = bs;
    self->bitPos += nbits;
    self->bitsWritten += nbits;
  }
}


/*-------------- public: -------------*/

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
                              )
{

	int error = 0;

	if(self == NULL || extBuffer == NULL || extBufSize <= 0 || startPos < 0){
		error = 1;
	}

	if(!error){
		self->charBuffer	= extBuffer;
		self->charBufSize	= extBufSize;

    self->charBufPos	= startPos >> 3;
    self->bitPos		  = startPos % 8;

    self->startPos		= startPos;
    self->endPos      = startPos + (extBufSize -1);

    self->bitsWritten = 0;
	}

	return error;
}



/** Resets the bitbuffer.
    This function resets the write-pointer of
    the buffer. All other values remain the same.
    Does not clear/zero the bitbuffer itself.

    self - pointer to bitbuffer structure
*/
void WOBITBUFAPI wobitbuf_Reset(wobitbufHandle self){

	if(self != NULL){
		self->charBufPos	= self->startPos >> 3;
		self->bitPos		  = self->startPos % 8;
    self->bitsWritten = 0;
	}
}



/** Copies the bitbuffer internal states. 
    The handle has to be allocated externally.

    desHandle - pointer to destination bitbuffer structure
    refHandle - pointer to reference bitbuffer structure
*/
void WOBITBUFAPI wobitbuf_Copy(
                               wobitbufHandle desHandle,
                               wobitbufHandle refHandle
                               )
{

	if(desHandle != NULL && refHandle != NULL){
		desHandle->charBuffer	  = refHandle->charBuffer;
		desHandle->charBufSize	= refHandle->charBufSize;
    
    desHandle->charBufPos	  = refHandle->charBufPos;
		desHandle->bitPos	      = refHandle->bitPos;

		desHandle->startPos		  = refHandle->startPos;
    desHandle->endPos       = refHandle->endPos;

    desHandle->bitsWritten  = refHandle->bitsWritten;
	}
}



/** Returns the internal character buffer size. 
    If self == NULL the function returns 0.
      
    self - pointer to bitbuffer structure
    
    return - size of character buffer in bits
*/
int WOBITBUFAPI wobitbuf_GetBufferSize(wobitbufHandle self)
{
  
  int bufsize = 0;

	if(self != NULL){
		bufsize = self->charBufSize;
	}
	
	return bufsize;
	
}



/** Writes bits to buffer. 
    If we go out of range, the write operation returns an error.
    If the function is called with nBits greater than 32 or below
    1, the write-operation is ignored.

    self  - pointer to bitbuffer structure
    data  - data to be written
    nBits - number of bits to be written

    return - error code, zero for success
*/
int WOBITBUFAPI wobitbuf_WriteBits(
                                   wobitbufHandle self,
                                   unsigned int data,
                                   int nBits
                                   )
{

  int error = 0;
    
  if(self == NULL || (wobitbuf_GetBitsAvail(self) - nBits < 0)){
    error = 1;
  }

  if(!error && nBits > 0 && nBits <= 32){

    while (nBits >= 8){
      unsigned char tmp = '\0';
      nBits += -8;
      tmp |= (unsigned char) (data >> nBits);
      WriteByte(self, tmp);
    }

    if (nBits){
      unsigned char tmp = (unsigned char) (data & GetMask(nBits));
      WriteBitsUnderEight(self, tmp, nBits);
    }
  }

  return error;
}



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
                                    )
{
  
  int byteCount;
  int error = 0;
  
  if(self == NULL || (wobitbuf_GetBitsAvail(self) - (nBytes << 3) < 0)){
    error = 1;
  }
  
  if(!error){
      for(byteCount=0; byteCount<nBytes; byteCount++){
        WriteByte(self, srcBuffer[byteCount]);
      }
  }
  
  return error;
}



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
                              )
{
	int error = 0;
	
	if(self != NULL){
		
    int newPos = ((self->charBufPos << 3) + self->bitPos) + nBits;
               /*------------current positon-------------*/

		if(newPos <= self->endPos && newPos >= self->startPos){
			self->charBufPos	= newPos >> 3;
			self->bitPos		  = newPos % 8;
		}
		else
		{
			error = 1;
		}
	}
	else
	{
		error = 1;
	}

	return error;
}



/** Aligns the write pointer to the next byte with respect to the start position. 
    The skipped adat area is not touched.

    self - pointer to bitbuffer structure
*/
void WOBITBUFAPI wobitbuf_ByteAlign(wobitbufHandle self)
{
	if (self != NULL && wobitbuf_GetBitsAvail(self) > 8){
		    
    int startbitpos = self->startPos % 8;
    int bitPos      = self->bitPos;
    int bitsToWrite = (8 - bitPos) + startbitpos;
    
    if(self->bitPos > (self->startPos % 8)){
      self->charBufPos++;
    }

    self->bitPos      = (self->startPos % 8);
    self->bitsWritten += bitsToWrite;
	}
}


/** Aligns the read pointer to the next byte with respect to the start position. 
    The skipped adat area is set to 'value'.

    self  - pointer to bitbuffer structure
    value - can be 0 or 1
*/
void WOBITBUFAPI wobitbuf_ByteAlignSet(wobitbufHandle self, int value)
{
  if (self != NULL && wobitbuf_GetBitsAvail(self) > 8){
    
    int startbitpos = self->startPos % 8;
    int bitPos      = self->bitPos;
    int bitsToWrite = (8 - bitPos) + startbitpos;

    if(bitsToWrite != 8){
      WriteBitsUnderEight(self, getSetValue(value, bitsToWrite), bitsToWrite);
    }
  }
}

/** Returns the number of bits available to write operation.
    This function returns the number of bits between current
    write-pointer-position and end-of-buffer.

    self - pointer to bitbuffer structure

    return - number of bits
*/
int WOBITBUFAPI wobitbuf_GetBitsAvail(wobitbufHandle self)
{
	
  int bitsAvail = 0;
  
  if (self != NULL){
		bitsAvail = self->charBufSize - wobitbuf_GetBufPos(self);
  }
	
  return bitsAvail;
}



/** Returns the current write-pointer-position.
    This function returns the current write-pointer-position with respect 
    to the start position. 
    
    self - pointer to bitbuffer structure

    return - number of bits
*/
int WOBITBUFAPI wobitbuf_GetBufPos(wobitbufHandle self)
{

  int bitsFilled = 0;
  
  if (self != NULL){
		bitsFilled = ((self->charBufPos<<3) + self->bitPos) - self->startPos;
		
	}
	
  return bitsFilled;
}


/** Returns the number of bits written since initialization or reset.
        
    self - pointer to bitbuffer structure

    return - number of bits
*/
int WOBITBUFAPI wobitbuf_GetBitsWritten(wobitbufHandle self)
{
  return self->bitsWritten;
}

