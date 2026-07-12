/* This software module was originally developed by Apple Computer, Inc. in the course of
 * development of MPEG-4. This software module is an implementation of a part of one or more MPEG-4
 * tools as specified by MPEG-4. ISO/IEC gives users of MPEG-4 free license to this software module
 * or modifications thereof for use in hardware or software products claiming conformance to MPEG-4.
 * Those intending to use this software module in hardware or software products are advised that its
 * use may infringe existing patents. The original developer of this software module and his/her
 * company, the subsequent editors and their companies, and ISO/IEC have no liability for use of
 * this software module or modifications thereof in an implementation. Copyright is not released for
 * non MPEG-4 conforming products. Apple Computer, Inc. retains full right to use the code for its
 * own purpose, assign or donate the code to a third party and to inhibit third parties from using
 * the code for non MPEG-4 conforming products. This copyright notice must be included in all copies
 * or derivative works.
 */

#include "isoiff_cfen.h"
#include <stdlib.h>
#include <string.h>

MP4Err ISOIFF_CreateColourFormatEnhancementData(MP4Handle dataH, const ISOIFF_CfenInput *inputs,
                                                u32 count)
{
  MP4Err err;
  u32 i, size;
  u8 *p;

  err = MP4NoErr;
  if(dataH == NULL || inputs == NULL || count == 0) BAILWITHERROR(MP4BadParamErr)

  /* version(1) + per input: (reserved:7|is_packed_flag:1)(1) + channel_id(1) for non-packed */
  size = 1;
  for(i = 0; i < count; i++)
  {
    if(inputs[i].is_packed_flag) BAILWITHERROR(MP4BadParamErr) /* packed layout: TODO */
    size += 2;
  }

  err = MP4SetHandleSize(dataH, size);
  if(err) goto bail;

  p    = (u8 *)*dataH;
  *p++ = 0; /* version */
  for(i = 0; i < count; i++)
  {
    *p++ = (u8)(inputs[i].is_packed_flag & 0x01); /* reserved(7)=0 | is_packed_flag */
    *p++ = inputs[i].channel_id;
  }

bail:
  return err;
}

MP4Err ISOIFF_ParseColourFormatEnhancementData(MP4Handle dataH, ISOIFF_CfenInput *inputs,
                                               u32 maxCount, u32 *outCount)
{
  MP4Err err;
  u32 size, pos, n;
  u8 *p;

  err = MP4NoErr;
  if(dataH == NULL || inputs == NULL || outCount == NULL) BAILWITHERROR(MP4BadParamErr)

  err = MP4GetHandleSize(dataH, &size);
  if(err) goto bail;
  if(size < 1) BAILWITHERROR(MP4BadDataErr)

  p = (u8 *)*dataH;
  if(p[0] != 0) BAILWITHERROR(MP4BadDataErr) /* version */
  pos = 1;
  n   = 0;
  while(pos < size)
  {
    u8 packed = p[pos] & 0x01;
    pos += 1;
    if(packed) BAILWITHERROR(MP4BadParamErr) /* packed layout: TODO */
    if(pos >= size) BAILWITHERROR(MP4BadDataErr)
    if(n >= maxCount) BAILWITHERROR(MP4BadParamErr)
    inputs[n].is_packed_flag = 0;
    inputs[n].channel_id     = p[pos];
    pos += 1;
    n += 1;
  }
  *outCount = n;

bail:
  return err;
}

MP4Err ISOIFF_CreateColourFormatEnhancementItem(ISOIFF_ImageCollection collection,
                                                ISOIFF_Image *inputImages,
                                                const ISOIFF_CfenInput *inputs, u32 count,
                                                ISOIFF_Image *outCfen)
{
  MP4Err err;
  u32 i;
  MP4Handle dataH   = NULL;
  ISOIFF_Image cfen = NULL;

  err = MP4NoErr;
  if(collection == NULL || inputImages == NULL || inputs == NULL || count == 0 || outCfen == NULL)
    BAILWITHERROR(MP4BadParamErr)

  err = MP4NewHandle(0, &dataH);
  if(err) goto bail;
  err = ISOIFF_CreateColourFormatEnhancementData(dataH, inputs, count);
  if(err) goto bail;

  err = ISOIFF_NewImage(collection, &cfen, ISOIFF_4CC_cfen, dataH);
  if(err) goto bail;

  /* Order the inputs via 'dimg' references (first == base input image). */
  for(i = 0; i < count; i++)
  {
    err = ISOIFF_AddImageRelation(cfen, inputImages[i], ISOIFF_4CC_dimg);
    if(err) goto bail;
  }

  *outCfen = cfen;

bail:
  if(dataH) MP4DisposeHandle(dataH);
  return err;
}
