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

/*!
 @header isoiff_cfen
 Colour format enhancement derived image item ('cfen'), ISO/IEC 23008-12 AMD1 clause 6.6.2.5.
 Write path (minimal): non-packed inputs only; packed grid layout is future work.
 */

#ifndef isoiff_cfen_h
#define isoiff_cfen_h

#include "isoiff.h"

#ifdef __cplusplus
extern "C"
{
#endif

enum
{
  ISOIFF_4CC_cfen = MP4_FOUR_CHAR_CODE('c', 'f', 'e', 'n'),
  ISOIFF_4CC_dimg = MP4_FOUR_CHAR_CODE('d', 'i', 'm', 'g')
};

/*!
 * @typedef ISOIFF_CfenInput
 * @brief One per-input entry of a ColourFormatEnhancement box (6.6.2.5.2).
 *   Minimal write path supports non-packed inputs only (is_packed_flag == 0).
 *   channel_id per Table 2: 0=unused, 1=unspecified, 2=Y/R, 3=Cb/G, 4=Cr/B, 5=Alpha, 6=Depth.
 */
typedef struct ISOIFF_CfenInput
{
  u8 is_packed_flag; /* only 0 supported for now */
  u8 channel_id;
} ISOIFF_CfenInput;

/*!
 * @discussion Builds the ColourFormatEnhancement box payload (the 'cfen' item body) into dataH.
 *   reference_count is implicit (== count) per spec and is not stored in the box.
 * @param dataH  MP4Handle (created with MP4NewHandle) that will receive the serialized bytes
 * @param inputs Array of per-input entries, in dimg reference order
 * @param count  Number of inputs (>= 1)
 */
MP4Err ISOIFF_CreateColourFormatEnhancementData(MP4Handle dataH, const ISOIFF_CfenInput *inputs,
                                                u32 count);

/*!
 * @discussion Parses a ColourFormatEnhancement box payload (the 'cfen' item body) into an array of
 *   per-input entries. Non-packed inputs only. reference_count is derived from the payload length.
 * @param dataH     MP4Handle holding the serialized box bytes
 * @param inputs    Output array (caller-allocated) of per-input entries, in dimg reference order
 * @param maxCount  Capacity of the inputs array
 * @param outCount  Receives the number of inputs parsed
 */
MP4Err ISOIFF_ParseColourFormatEnhancementData(MP4Handle dataH, ISOIFF_CfenInput *inputs,
                                               u32 maxCount, u32 *outCount);

/*!
 * @discussion Creates a 'cfen' derived image item whose body is the ColourFormatEnhancement box,
 *   and adds a 'dimg' reference from the cfen item to each input image (in order). The caller is
 *   responsible for attaching the cfen item's ispe/pixi/colr properties and for marking it as the
 *   primary/cover item.
 * @param collection  The image collection
 * @param inputImages Array of input images, in dimg reference order (first is the base)
 * @param inputs      Per-input ColourFormatEnhancement entries (same order/count as inputImages)
 * @param count       Number of inputs (>= 1)
 * @param outCfen     Receives the created cfen image item
 */
MP4Err ISOIFF_CreateColourFormatEnhancementItem(ISOIFF_ImageCollection collection,
                                                ISOIFF_Image *inputImages,
                                                const ISOIFF_CfenInput *inputs, u32 count,
                                                ISOIFF_Image *outCfen);

#ifdef __cplusplus
}
#endif

#endif
