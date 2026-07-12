/* This software module was originally developed by Apple Computer, Inc. in the course of
development of MPEG-4. This software module is an implementation of a part of one or more MPEG-4
tools as specified by MPEG-4. ISO/IEC gives users of MPEG-4 free license to this software module or
modifications thereof for use in hardware or software products claiming conformance to MPEG-4. Those
intending to use this software module in hardware or software products are advised that its use may
infringe existing patents. The original developer of this software module and his/her company, the
subsequent editors and their companies, and ISO/IEC have no liability for use of this software
module or modifications thereof in an implementation. Copyright is not released for non MPEG-4
conforming products. Apple Computer, Inc. retains full right to use the code for its own purpose,
assign or donate the code to a third party and to inhibit third parties from using the code for non
MPEG-4 conforming products.

This copyright notice must be included in all copies or derivative works. Copyright (c) 2014.
*/

#include <nlohmann/json.hpp>
#include <fstream>
#include "hevc_tool.h"
#include "isoiff_cfen.h"

#include <stdio.h>
#include <fcntl.h>
#include <assert.h>

#include "TLibDecoder/AnnexBread.h"
#include "TLibDecoder/NALread.h"
#include "TLibDecoder/TDecEntropy.h"
#include "TLibDecoder/TDecCAVLC.h"

extern "C"
{
#include "StringUtils.h"
#include "Logger.h"
}

using namespace std;

MP4Err processHEVC_SPS(ISOIFF_HEVCDecoderConfigRecord record, InputNALUnit *nalu);

MP4Err processHEVC_NALUnits(ISOIFF_HEVCDecoderConfigRecord record, ISOIFF_HEVCItemData itemData,
                            Options *options)
{
  MP4Err err;
  u8 lengthSizeMinusOne;

  err = MP4NoErr;

  logMsg(LOGLEVEL_INFO, "Processing HEVC NAL Units.");

  ifstream bitstreamFile(options->inputFile, ifstream::in | ifstream::binary);
  InputByteStream bytestream(bitstreamFile);

  while(!!bitstreamFile)
  {
    AnnexBStats stats = AnnexBStats();
    vector<uint8_t> nalUnit;
    InputNALUnit nalu;
    MP4Handle nalData;
    u8 *buffer;

    byteStreamNALUnit(bytestream, nalu.getBitstream().getFifo(), stats);

    if(!nalu.getBitstream().getFifo().empty())
    {
      err = MP4NewHandle((u32)nalu.getBitstream().getFifo().size(), &nalData);
      if(err) goto bail;
      buffer = (u8 *)*nalData;

      for(u32 i = 0; i < nalu.getBitstream().getFifo().size(); i++)
      {
        buffer[i] = (u8)nalu.getBitstream().getFifo().at(i);
      }

      read(nalu);

      switch(nalu.m_nalUnitType)
      {
      case NAL_UNIT_VPS:
      {
        logMsg(LOGLEVEL_INFO, "VPS NAL Unit found!");

        err = ISOIFF_AddNALUnitToHEVCDecConfRec(record, nalu.m_nalUnitType, nalData);
        if(err) goto bail;
        break;
      }
      case NAL_UNIT_PPS:
      {
        logMsg(LOGLEVEL_INFO, "PPS NAL Unit found!");

        err = ISOIFF_AddNALUnitToHEVCDecConfRec(record, nalu.m_nalUnitType, nalData);
        if(err) goto bail;
        break;
      }
      case NAL_UNIT_SPS:
      {
        logMsg(LOGLEVEL_INFO, "SPS NAL Unit found!");

        err = ISOIFF_AddNALUnitToHEVCDecConfRec(record, nalu.m_nalUnitType, nalData);
        if(err) goto bail;

        err = processHEVC_SPS(record, &nalu);
        if(err) goto bail;
        break;
      }
      case NAL_UNIT_PREFIX_SEI:
      case NAL_UNIT_SUFFIX_SEI:
      {
        logMsg(LOGLEVEL_INFO, "SEI NAL Unit found!");

        err = ISOIFF_AddNALUnitToHEVCDecConfRec(record, nalu.m_nalUnitType, nalData);
        if(err) goto bail;
        break;
      }
      default:
      {
        logMsg(LOGLEVEL_INFO, "Non parameter set NAL Unit found!", nalu.m_nalUnitType);

        err = ISOIFF_AddNALUnitToHEVCItemData(itemData, nalData);
        if(err) goto bail;
        break;
      }
      }

      err = MP4DisposeHandle(nalData);
      if(err) goto bail;
    }
  }

  err = ISOIFF_GetHEVCItemDataLengthSizeMinusOne(itemData, &lengthSizeMinusOne);
  if(err) goto bail;
  record->lengthSizeMinusOne = lengthSizeMinusOne;

  logMsg(LOGLEVEL_INFO, "Processing HEVC NAL Units finished.");
bail:
  return err;
}

MP4Err processHEVC_SPS(ISOIFF_HEVCDecoderConfigRecord record, InputNALUnit *nalu)
{
  MP4Err err;
  TComSPS *sps;
  ProfileTierLevel *ptl;
  TDecEntropy m_cEntropyDecoder;
  TDecCavlc m_cCavlcDecoder;

  err = MP4NoErr;
  sps = new TComSPS();

  logMsg(LOGLEVEL_INFO, "Processing HEVC SPS NAL Unit.");

  m_cEntropyDecoder.setEntropyDecoder(&m_cCavlcDecoder);
  m_cEntropyDecoder.setBitstream(&nalu->getBitstream());
  m_cEntropyDecoder.decodeSPS(sps);

  ptl = sps->getPTL()->getGeneralPTL();

  if(sps->getVuiParametersPresentFlag())
  {
    TComVUI *vui                         = sps->getVuiParameters();
    record->min_spatial_segmentation_idc = vui->getMinSpatialSegmentationIdc();
  }

  record->general_tier_flag    = ptl->getTierFlag();
  record->general_level_idc    = ptl->getLevelIdc();
  record->chromaFormat         = sps->getChromaFormatIdc();
  record->bitDepthLumaMinus8   = sps->getBitDepth(CHANNEL_TYPE_LUMA) - 8;
  record->bitDepthChromaMinus8 = sps->getBitDepth(CHANNEL_TYPE_CHROMA) - 8;

  logMsg(LOGLEVEL_DEBUG, "General tier flag: %d", record->general_tier_flag);
  logMsg(LOGLEVEL_DEBUG, "General levle idc: %d", record->general_level_idc);
  logMsg(LOGLEVEL_DEBUG, "Chroma format: %d", record->chromaFormat);
  logMsg(LOGLEVEL_DEBUG, "Bit depth luma - 8: %d", record->bitDepthLumaMinus8);
  logMsg(LOGLEVEL_DEBUG, "Bit depth chroma - 8: %d", record->bitDepthChromaMinus8);

  if(ptl->getProgressiveSourceFlag())
  {
    logMsg(LOGLEVEL_DEBUG, "Progressive source flag is on");
    record->general_constraint_indicator_flags |= 0x800000000000;
  }
  else
  {
    logMsg(LOGLEVEL_DEBUG, "Progressive source flag is off");
  }

  if(ptl->getInterlacedSourceFlag())
  {
    logMsg(LOGLEVEL_DEBUG, "Interlaced source flag is on");
    record->general_constraint_indicator_flags |= 0x400000000000;
  }
  else
  {
    logMsg(LOGLEVEL_DEBUG, "Interlaced source flag is off");
  }

  if(ptl->getNonPackedConstraintFlag())
  {
    logMsg(LOGLEVEL_DEBUG, "Non packed constraint flag is on");
    record->general_constraint_indicator_flags |= 0x200000000000;
  }
  else
  {
    logMsg(LOGLEVEL_DEBUG, "Non packed constraint flag is off");
  }

  if(ptl->getFrameOnlyConstraintFlag())
  {
    logMsg(LOGLEVEL_DEBUG, "Frame only constraint flag is on");
    record->general_constraint_indicator_flags |= 0x100000000000;
  }
  else
  {
    logMsg(LOGLEVEL_DEBUG, "Frame only constraint is off");
  }
  delete sps;
bail:
  return err;
}

MP4Err createHEVC_ImageCollection(ISOIFF_ImageCollection *collection)
{
  MP4Err err;

  logMsg(LOGLEVEL_INFO, "Creating HEVC image collection.");
  err = ISOIFF_CreateImageCollection(collection, ISOIFF_4CC_heic, 0);
  if(err) goto bail;
  logMsg(LOGLEVEL_INFO, "Creating HEVC image collection finished.");
bail:
  return err;
}

/* Attach an nclx Colour Information (colr) property to an image item. colr is a caller policy so
 * that, for cfen, the base + enhancement inputs carry the same signaling as the cfen item (a cfen
 * input's colr, if present, shall match the derived item's colr per 6.6.2.5.1). */
static MP4Err attachNclxColr(ISOIFF_Image image, u32 primaries, u32 transfer, u32 matrix,
                             u32 fullRange)
{
  MP4Err err;
  MP4ColorInformationAtomPtr colr;
  err = MP4CreateColorInformationAtom(&colr);
  if(err) goto bail;
  colr->colour_type              = MP4ColorParameterTypeNCLX;
  colr->colour_primaries         = primaries;
  colr->transfer_characteristics = transfer;
  colr->matrix_coefficients      = matrix;
  colr->full_range_flag          = fullRange;
  err = ISOIFF_AddImageProperty(image, (MP4AtomPtr)colr, 0);
bail:
  return err;
}

MP4Err addHEVCImageToCollection(ISOIFF_ImageCollection collection,
                                ISOIFF_HEVCDecoderConfigRecord record, ISOIFF_HEVCItemData itemData,
                                u32 width, u32 height, u32 numChannels, ISOIFF_Image *outImage)
{
  MP4Err err;
  MP4Handle imageData;
  ISOIFF_Image image;
  ISOIFF_HEVCConfigurationAtomPtr hevc;
  ISOIFF_ImageSpatialExtentsPropertyAtomPtr ispe;
  u32 i;

  logMsg(LOGLEVEL_INFO, "Adding HEVC image to collection.");

  err = MP4NoErr;
  err = MP4NewHandle(0, &imageData);
  if(err) goto bail;

  err = ISOIFF_PutHEVCItemDataIntoHandle(itemData, imageData);
  if(err) goto bail;
  err = ISOIFF_NewImage(collection, &image, ISOIFF_4CC_hvc1, imageData);
  if(err) goto bail;

  err = ISOIFF_CreateHEVCConfigurationAtom(&hevc, record);
  if(err) goto bail;
  err = ISOIFF_AddImageProperty(image, (MP4AtomPtr)hevc, 1);
  if(err) goto bail;

  err = ISOIFF_CreateImageSpatialExtentsPropertyAtom(&ispe);
  if(err) goto bail;
  ispe->image_width  = width;
  ispe->image_height = height;
  err                = ISOIFF_AddImageProperty(image, (MP4AtomPtr)ispe, 0);
  if(err) goto bail;

  {
    /* Attach a PixelInformationProperty (pixi): numChannels @ 8 bits
     * (3 for the 4:2:0 base, 1 for a mono enhancement plane). */
    ISOIFF_PixelInformationPropertyAtomPtr pixi;
    err = ISOIFF_CreatePixelInformationPropertyAtom(&pixi);
    if(err) goto bail;
    pixi->num_channels = numChannels;
    for(i = 0; i < numChannels && i < 16; i++) pixi->bits_per_channel[i] = 8;
    err = ISOIFF_AddImageProperty(image, (MP4AtomPtr)pixi, 0);
    if(err) goto bail;
  }

  err = MP4DisposeHandle(imageData);
  if(err) goto bail;

  if(outImage != NULL)
    *outImage = image; /* caller takes ownership and frees later */
  else
  {
    err = ISOIFF_FreeImage(image);
    if(err) goto bail;
  }

  logMsg(LOGLEVEL_INFO, "Adding HEVC image to collection finished.");
bail:
  return err;
}

/* Build a cfen image collection: base 4:2:0 + full-res mono Cb/Cr enhancement items, a 'cfen'
 * derived item referencing all three (channel_id 2/3/4), and an 'altr' group (base + cfen) so
 * legacy readers fall back to the 4:2:0 base. Driven by -i (base), -a (Cb), -b (Cr). */
/* Shared 'heif_cfen' interchange metadata parsed from the create-side JSON (HDRTools). */
typedef struct
{
  int loaded;
  int hasColr;
  int colour_primaries, transfer_characteristics, matrix_coefficients, full_range_flag;
  int count;
  int channel_id[16];
  int is_packed[16];
} CfenMetaCfg;

static void loadCfenMetaJson(const char *path, CfenMetaCfg *cfg)
{
  memset(cfg, 0, sizeof(*cfg));
  if(path == NULL) return;
  std::ifstream f(path);
  if(!f.is_open())
  {
    logMsg(LOGLEVEL_WARNING, "cfen: could not open metadata JSON %s", path);
    return;
  }
  try
  {
    nlohmann::json j;
    f >> j;
    if(j.contains("colr"))
    {
      const nlohmann::json &c        = j["colr"];
      cfg->hasColr                   = 1;
      cfg->colour_primaries          = c.value("colour_primaries", 1);
      cfg->transfer_characteristics  = c.value("transfer_characteristics", 1);
      cfg->matrix_coefficients       = c.value("matrix_coefficients", 1);
      cfg->full_range_flag           = c.value("full_range_flag", false) ? 1 : 0;
    }
    if(j.contains("inputs"))
    {
      for(const nlohmann::json &it : j["inputs"])
      {
        if(cfg->count >= 16) break;
        cfg->channel_id[cfg->count] = it.value("channel_id", 0);
        cfg->is_packed[cfg->count]  = it.value("is_packed", false) ? 1 : 0;
        cfg->count++;
      }
    }
    cfg->loaded = 1;
    logMsg(LOGLEVEL_INFO, "cfen: loaded interchange metadata from %s (%d input(s))", path,
           cfg->count);
  }
  catch(const std::exception &e)
  {
    logMsg(LOGLEVEL_WARNING, "cfen: failed to parse metadata JSON %s: %s", path, e.what());
  }
}

/* --- codec-specific coded-image builders (the only codec-dependent part of cfen packaging) --- */

static MP4Err buildHevcImageFromFile(ISOIFF_ImageCollection collection, const char *file, u32 w,
                                     u32 h, u32 numChannels, int hidden, ISOIFF_Image *outImage,
                                     Options *opts)
{
  MP4Err err;
  ISOIFF_HEVCDecoderConfigRecord rec;
  ISOIFF_HEVCItemData it;
  Options o;

  err = ISOIFF_CreateHEVCDecoderConfigRecord(&rec);
  if(err) goto bail;
  err = ISOIFF_CreateHEVCItemData(&it);
  if(err) goto bail;
  o           = *opts;
  o.inputFile = (char *)file;
  err         = processHEVC_NALUnits(rec, it, &o);
  if(err) goto bail;
  err = addHEVCImageToCollection(collection, rec, it, w, h, numChannels, outImage);
  if(err) goto bail;
  if(hidden)
  {
    err = ISOIFF_SetImageHidden(*outImage, 1);
    if(err) goto bail;
  }
bail:
  return err;
}

/* Codec dispatch for building one cfen input image. New coded formats plug in here; everything
 * below (the cfen item, dimg, altr, properties) is codec-independent. */
static MP4Err buildCfenInputImage(ISOIFF_ImageCollection collection, const char *file,
                                  const char *codec, u32 w, u32 h, u32 numChannels, int hidden,
                                  ISOIFF_Image *outImage, Options *opts)
{
  if(codec == NULL || codec[0] == 0 || strcmp(codec, "hevc") == 0)
    return buildHevcImageFromFile(collection, file, w, h, numChannels, hidden, outImage, opts);
  logMsg(LOGLEVEL_ERROR, "cfen: unsupported input codec '%s'", codec);
  return MP4BadParamErr;
}

MP4Err processWriteModeCfen(Options *options)
{
  MP4Err err;
  ISOIFF_ImageCollection collection;
  ISOIFF_Image images[3];
  ISOIFF_Image altGroup[2];
  ISOIFF_Image cfen;
  ISOIFF_CfenInput cins[3];
  ISOIFF_ImageSpatialExtentsPropertyAtomPtr cispe;
  ISOIFF_PixelInformationPropertyAtomPtr cpixi;
  CfenMetaCfg mcfg;
  const char *files[3];
  u32 nch[3]    = { 3, 1, 1 };
  int hidden[3] = { 0, 1, 1 };
  int i;
  u32 cp = 1, tc = 1, mc = 1, fr = 0; /* nclx colr, BT.709 / limited defaults */
  u32 w = (u32)options->width, h = (u32)options->height;

  err = MP4NoErr;
  logMsg(LOGLEVEL_INFO, "Processing cfen (3-input) write mode..");

  /* Optional: read colr + channel_ids from the shared interchange metadata (HDRTools create side). */
  loadCfenMetaJson(options->metaFile, &mcfg);
  if(mcfg.loaded && mcfg.hasColr)
  {
    cp = mcfg.colour_primaries; tc = mcfg.transfer_characteristics;
    mc = mcfg.matrix_coefficients; fr = mcfg.full_range_flag;
  }

  files[0] = options->inputFile; /* base 4:2:0 */
  files[1] = options->enhFileU;  /* Cb (mono)  */
  files[2] = options->enhFileV;  /* Cr (mono)  */

  /* A cfen file has a derived (cfen) primary item, so it uses the 'mif2' structural brand (which
   * supports a derived primary with an 'altr' fallback). 'mif1'/'heic' are not used: 'mif1'
   * requires the primary to be independently coded, and 'heic' pulls in 'mif1'. */
  err = ISOIFF_CreateImageCollectionWithBrands(&collection, ISOIFF_4CC_mif2, 0, 0); if(err) goto bail;

  /* Build each coded input image via the codec dispatch. Base is the displayable fallback;
   * enhancement inputs are hidden (6.6.2.5.1). Codec is HEVC for now (see buildCfenInputImage). */
  for(i = 0; i < 3; i++)
  {
    err = buildCfenInputImage(collection, files[i], "hevc", w, h, nch[i], hidden[i], &images[i],
                              options);
    if(err) goto bail;
    /* colr on inputs is optional (6.6.2.5.1). Attach it only to the base, which is a displayable
     * 'altr' alternative; when present it must match the cfen item's colr, so use the same values.
     * The hidden mono enhancement inputs carry no colr. */
    if(i == 0)
    {
      err = attachNclxColr(images[i], cp, tc, mc, fr);
      if(err) goto bail;
    }
  }

  cins[0].is_packed_flag = 0; cins[0].channel_id = 2; /* Y  */
  cins[1].is_packed_flag = 0; cins[1].channel_id = 3; /* Cb */
  cins[2].is_packed_flag = 0; cins[2].channel_id = 4; /* Cr */

  /* Override channel_ids / is_packed from the interchange metadata when provided. */
  if(mcfg.loaded && mcfg.count == 3)
  {
    for(i = 0; i < 3; i++)
    {
      cins[i].channel_id     = (u8)mcfg.channel_id[i];
      cins[i].is_packed_flag = (u8)mcfg.is_packed[i];
    }
  }

  err = ISOIFF_CreateColourFormatEnhancementItem(collection, images, cins, 3, &cfen);
  if(err) goto bail;

  /* cfen item properties: ispe + pixi(3ch) + colr(nclx). */
  err = ISOIFF_CreateImageSpatialExtentsPropertyAtom(&cispe); if(err) goto bail;
  cispe->image_width = w; cispe->image_height = h;
  err = ISOIFF_AddImageProperty(cfen, (MP4AtomPtr)cispe, 0); if(err) goto bail;

  err = ISOIFF_CreatePixelInformationPropertyAtom(&cpixi); if(err) goto bail;
  cpixi->num_channels = 3;
  cpixi->bits_per_channel[0] = 8; cpixi->bits_per_channel[1] = 8; cpixi->bits_per_channel[2] = 8;
  err = ISOIFF_AddImageProperty(cfen, (MP4AtomPtr)cpixi, 0); if(err) goto bail;

  err = attachNclxColr(cfen, cp, tc, mc, fr); if(err) goto bail;

  err = ISOIFF_SetImageAsCover(cfen); if(err) goto bail; /* cfen is the primary/displayable item */

  /* altr: the cfen item and the base 4:2:0 are alternatives. Per 6.6.2.5.1 the cfen item is listed
   * first (preferred) and the backward-compatible 4:2:0 base second. Both are non-hidden (an 'altr'
   * group must not mix hidden and non-hidden items). */
  altGroup[0] = cfen; altGroup[1] = images[0];
  err = ISOIFF_AddImagesToAlternativeGroup(collection, altGroup, 2, 1); if(err) goto bail;

  err = ISOIFF_FreeImage(cfen); if(err) goto bail;
  for(i = 0; i < 3; i++) { err = ISOIFF_FreeImage(images[i]); if(err) goto bail; }

  err = ISOIFF_WriteCollectionToFile(collection, options->outputFile); if(err) goto bail;
  logMsg(LOGLEVEL_INFO, "cfen write mode finished.");
bail:
  return err;
}

/* Extract a single HEVC image's Annex-B bitstream (parameter sets from hvcC + coded slice). */
static MP4Err extractHevcBitstreamFromImage(ISOIFF_Image image, MP4Handle bitstreamH)
{
  MP4Err err;
  u32 j, numProps;
  MP4GenericAtom *properties = NULL;
  ISOIFF_HEVCDecoderConfigRecord record = NULL;

  err = ISOIFF_CreateHEVCDecoderConfigRecord(&record);
  if(err) goto bail;
  /* getHEVCBitstreamFromImage appends; reset the handle so we get only this image's bitstream. */
  err = MP4SetHandleSize(bitstreamH, 0);
  if(err) goto bail;
  err = ISOIFF_GetImageProperties(image, &properties, &numProps);
  if(err) goto bail;
  for(j = 0; j < numProps; j++)
  {
    MP4AtomPtr property = (MP4AtomPtr)properties[j];
    if(property->type == ISOIFF_4CC_hvcC)
    {
      err = ISOIFF_GetHEVCDecoderConfigRecordFromProperty(property, &record);
      if(err) goto bail;
    }
  }
  err = getHEVCBitstreamFromImage(image, record, bitstreamH);
  if(err) goto bail;

bail:
  if(properties) free(properties);
  if(record) ISOIFF_FreeHEVCDecoderConfigRecord(record);
  return err;
}

/* Best-effort read of an item's ispe (w,h), pixi (num_channels), and colr (nclx CICP) properties.
 * ispe/pixi come back as unknown atoms (payload after the 8-byte box header); colr is structured. */
typedef struct
{
  u32 width, height, num_channels;
  int hasColr;
  u32 colour_primaries, transfer_characteristics, matrix_coefficients, full_range_flag;
} CfenItemProps;

static MP4Err readCfenItemProps(ISOIFF_Image image, CfenItemProps *ip)
{
  MP4Err err;
  MP4GenericAtom *props = NULL;
  u32 n = 0, j;

  memset(ip, 0, sizeof(*ip));
  err = ISOIFF_GetImageProperties(image, &props, &n);
  if(err) goto bail;
  for(j = 0; j < n; j++)
  {
    MP4AtomPtr p = (MP4AtomPtr)props[j];
    if(p->type == ISOIFF_4CC_ispe)
    {
      MP4UnknownAtomPtr u = (MP4UnknownAtomPtr)p;
      u8 *d               = (u8 *)u->data;
      if(u->dataSize >= 12)
      {
        ip->width  = (d[4] << 24) | (d[5] << 16) | (d[6] << 8) | d[7];
        ip->height = (d[8] << 24) | (d[9] << 16) | (d[10] << 8) | d[11];
      }
    }
    else if(p->type == ISOIFF_4CC_pixi)
    {
      MP4UnknownAtomPtr u = (MP4UnknownAtomPtr)p;
      u8 *d               = (u8 *)u->data;
      if(u->dataSize >= 5) ip->num_channels = d[4];
    }
    else if(p->type == MP4ColorInformationAtomType)
    {
      MP4ColorInformationAtomPtr c = (MP4ColorInformationAtomPtr)p;
      ip->hasColr                  = 1;
      ip->colour_primaries         = c->colour_primaries;
      ip->transfer_characteristics = c->transfer_characteristics;
      ip->matrix_coefficients      = c->matrix_coefficients;
      ip->full_range_flag          = c->full_range_flag;
    }
  }
bail:
  if(props) free(props);
  return err;
}

/* cfen reconstruction (extraction stage): locate the 'cfen' item, parse its ColourFormatEnhancement
 * box, follow the 'dimg' inputs in order, and write each input's HEVC bitstream to
 * "<outBase>.ch<channel_id>.hevc". Decoding + 4:4:4 assembly is done downstream (no in-lib decoder).
 * Sets *foundCfen to 1 if a cfen item was present. */
MP4Err reconstructCfen(ISOIFF_ImageCollection collection, const char *outBase, u32 *foundCfen)
{
  MP4Err err;
  u32 numCfen = 0, numInputs = 0, parsedCount = 0, i;
  ISOIFF_Image *cfenImages = NULL;
  ISOIFF_Image *inputs = NULL;
  MP4Handle cfenDataH = NULL;
  MP4Handle bitstreamH = NULL;
  ISOIFF_CfenInput cins[16];
  char paths[16][1024];
  const char *codecs[16];
  CfenItemProps inProps[16];
  CfenItemProps cfenProps;

  err = MP4NoErr;
  if(foundCfen) *foundCfen = 0;

  err = ISOIFF_GetAllImagesWithType(collection, ISOIFF_4CC_cfen, &cfenImages, &numCfen);
  if(err) goto bail;
  if(numCfen == 0) goto bail; /* not a cfen file */
  if(foundCfen) *foundCfen = 1;

  /* Parse the ColourFormatEnhancement box (channel_id per input, in dimg order). */
  err = MP4NewHandle(0, &cfenDataH);
  if(err) goto bail;
  err = ISOIFF_GetImageData(cfenImages[0], cfenDataH);
  if(err) goto bail;
  err = ISOIFF_ParseColourFormatEnhancementData(cfenDataH, cins, 16, &parsedCount);
  if(err) goto bail;

  /* Input images, ordered by the cfen item's 'dimg' references. */
  err = ISOIFF_GetImagesOfImageWithType(cfenImages[0], ISOIFF_4CC_dimg, &inputs, &numInputs);
  if(err) goto bail;
  if(numInputs != parsedCount) BAILWITHERROR(MP4BadDataErr);

  logMsg(LOGLEVEL_INFO, "cfen: %d input(s)", numInputs);

  err = MP4NewHandle(0, &bitstreamH);
  if(err) goto bail;

  for(i = 0; i < numInputs; i++)
  {
    u32 sz, itype;
    const char *ext;
    FILE *f;

    /* Codec-agnostic dispatch: choose the extractor by the input item's coding type.
     * The cfen container itself is codec-independent; only the per-item bitstream extraction is
     * codec-specific, so new coded item types plug in here without touching the cfen logic. */
    err = ISOIFF_GetImageType(inputs[i], &itype);
    if(err) goto bail;
    if(itype == ISOIFF_4CC_hvc1)
    {
      err = extractHevcBitstreamFromImage(inputs[i], bitstreamH);
      if(err) goto bail;
      codecs[i] = "hevc";
      ext       = "hevc";
    }
    else
    {
      logMsg(LOGLEVEL_ERROR, "cfen: unsupported input item coding type 0x%08x", itype);
      BAILWITHERROR(MP4BadParamErr);
    }

    err = MP4GetHandleSize(bitstreamH, &sz);
    if(err) goto bail;
    snprintf(paths[i], sizeof(paths[i]), "%s.ch%d.%s", outBase, cins[i].channel_id, ext);
    f = fopen(paths[i], "wb");
    if(f == NULL) BAILWITHERROR(MP4IOErr);
    fwrite(*bitstreamH, 1, sz, f);
    fclose(f);
    err = readCfenItemProps(inputs[i], &inProps[i]);
    if(err) goto bail;
    logMsg(LOGLEVEL_INFO, "  wrote %s (channel_id=%d, %d bytes)", paths[i], cins[i].channel_id, sz);
  }

  /* Emit the shared interchange metadata (read side): describes the HEIF cfen signaling so a
   * decoder + HDRTools can assemble the final image. Same schema HDRTools writes on the forward
   * side (see ECFIConvert *.heif.json). Written to "<outBase>.heif.json". */
  err = readCfenItemProps(cfenImages[0], &cfenProps);
  if(err) goto bail;
  {
    char jpath[1024];
    nlohmann::json j;
    j["format"]  = "heif_cfen";
    j["version"] = 1;
    j["source"]  = "isoiff_tool";
    j["output"]  = { {"width", cfenProps.width}, {"height", cfenProps.height} };
    if(cfenProps.hasColr)
      j["colr"] = { {"colour_type", "nclx"},
                    {"colour_primaries", cfenProps.colour_primaries},
                    {"transfer_characteristics", cfenProps.transfer_characteristics},
                    {"matrix_coefficients", cfenProps.matrix_coefficients},
                    {"full_range_flag", cfenProps.full_range_flag != 0} };
    j["inputs"] = nlohmann::json::array();
    for(i = 0; i < numInputs; i++)
      j["inputs"].push_back({ {"role", i == 0 ? "base" : "enhancement"},
                              {"channel_id", cins[i].channel_id},
                              {"is_packed", cins[i].is_packed_flag != 0},
                              {"codec", codecs[i]},
                              {"file", paths[i]},
                              {"width", inProps[i].width},
                              {"height", inProps[i].height},
                              {"num_channels", inProps[i].num_channels} });

    snprintf(jpath, sizeof(jpath), "%s.heif.json", outBase);
    {
      std::ofstream jf(jpath);
      if(!jf.is_open()) BAILWITHERROR(MP4IOErr);
      jf << j.dump(2) << std::endl;
    }
    logMsg(LOGLEVEL_INFO, "cfen: wrote metadata %s", jpath);
  }

bail:
  if(bitstreamH) MP4DisposeHandle(bitstreamH);
  if(cfenDataH) MP4DisposeHandle(cfenDataH);
  if(inputs) free(inputs);
  if(cfenImages) free(cfenImages);
  return err;
}

MP4Err getHEVCImages(ISOIFF_ImageCollection collection, ISOIFF_Image **images,
                     ISOIFF_HEVCDecoderConfigRecord **decoderConfigs, u32 *numberOfImagesFound)
{
  MP4Err err;
  u32 i, j;

  logMsg(LOGLEVEL_INFO, "Requesting HEVC images from collection.");

  err = MP4NoErr;
  err = ISOIFF_GetAllImagesWithType(collection, ISOIFF_4CC_hvc1, images, numberOfImagesFound);
  if(err) goto bail;

  logMsg(LOGLEVEL_INFO, "Number of images found: %d", *numberOfImagesFound);

  *decoderConfigs = (ISOIFF_HEVCDecoderConfigRecord *)calloc(
    *numberOfImagesFound, sizeof(struct ISOIFF_HEVCDecoderConfigRecordS));

  for(i = 0; i < *numberOfImagesFound; i++)
  {
    MP4GenericAtom *properties;
    u32 numberOfPropertiesFound;

    properties = NULL;
    err        = ISOIFF_GetImageProperties((*images)[i], &properties, &numberOfPropertiesFound);
    if(err) goto bail;

    if(numberOfPropertiesFound == 0) BAILWITHERROR(MP4BadDataErr);

    for(j = 0; j < numberOfPropertiesFound; j++)
    {
      MP4AtomPtr property;
      property = (MP4AtomPtr)properties[j];
      if(property->type == ISOIFF_4CC_hvcC)
      {
        err = ISOIFF_GetHEVCDecoderConfigRecordFromProperty(property, &(*decoderConfigs[i]));
        if(err) goto bail;
      }
      else if(property->type == ISOIFF_4CC_ispe)
      {
        err = ISOIFF_ParseImageSpatialExtends(property);
        if(err) goto bail;
      }
    }

    free(properties);
  }

bail:
  return err;
}

MP4Err ISOIFF_GetHEVCDecoderConfigRecordFromProperty(MP4AtomPtr property,
                                                     ISOIFF_HEVCDecoderConfigRecord *decoderConfig)
{
  MP4Err err;
  MP4Handle boxH = NULL;
  MP4Handle recH = NULL;
  u32 boxSize;
  err = MP4NoErr;

  /* The hvcC property may be parsed either as a structured HEVCConfigurationBox or as an unknown
   * atom depending on the atom factory. Serialize it back to bytes and strip the 8-byte box header
   * to recover the HEVCDecoderConfigurationRecord, which is robust to both representations. */
  boxSize = (u32)property->size;
  if(boxSize <= 8) BAILWITHERROR(MP4BadDataErr);

  err = MP4NewHandle(boxSize, &boxH);
  if(err) goto bail;
  err = property->serialize(property, *boxH);
  if(err) goto bail;

  err = MP4NewHandle(boxSize - 8, &recH);
  if(err) goto bail;
  memcpy(*recH, (u8 *)*boxH + 8, boxSize - 8);

  err = ISOIFF_CreateHEVCDecConfRecFromHandle(recH, decoderConfig);
  if(err) goto bail;

bail:
  if(boxH) MP4DisposeHandle(boxH);
  if(recH) MP4DisposeHandle(recH);
  return err;
}

MP4Err getHEVCBitstreamFromImage(ISOIFF_Image image, ISOIFF_HEVCDecoderConfigRecord decoderConfig,
                                 MP4Handle bitstreamH)
{
  u32 i;
  u8 *buffer;
  MP4Err err;
  MP4Handle naluH;
  MP4Handle imgDatH;
  MP4Handle startCodePrefixH;
  MP4Handle zeroByteH;
  MP4LinkedList vpsNALUnits;
  MP4LinkedList spsNALUnits;
  MP4LinkedList ppsNALUnits;
  MP4LinkedList prefixSEI_NALUnits;
  MP4LinkedList suffixSEI_NALUnits;
  MP4LinkedList imageNALUnits;
  ISOIFF_HEVCItemData hevcItemData;

  logMsg(LOGLEVEL_INFO, "Creating HEVC bitstream from image.");

  err = MP4MakeLinkedList(&vpsNALUnits);
  if(err) goto bail;
  err = MP4MakeLinkedList(&spsNALUnits);
  if(err) goto bail;
  err = MP4MakeLinkedList(&ppsNALUnits);
  if(err) goto bail;
  err = MP4MakeLinkedList(&imageNALUnits);
  if(err) goto bail;
  err = MP4MakeLinkedList(&prefixSEI_NALUnits);
  if(err) goto bail;
  err = MP4MakeLinkedList(&suffixSEI_NALUnits);
  if(err) goto bail;

  err = MP4NewHandle(3, &startCodePrefixH);
  if(err) goto bail;
  buffer    = (u8 *)*startCodePrefixH;
  buffer[0] = 0;
  buffer[1] = 0;
  buffer[2] = 1;

  err = MP4NewHandle(1, &zeroByteH);
  if(err) goto bail;
  buffer    = (u8 *)*zeroByteH;
  buffer[0] = 0;

  err = ISOIFF_GetNALUnitsWithTypeFromHEVCDecConf(decoderConfig, NAL_UNIT_VPS, vpsNALUnits);
  if(err) goto bail;
  err = ISOIFF_GetNALUnitsWithTypeFromHEVCDecConf(decoderConfig, NAL_UNIT_SPS, spsNALUnits);
  if(err) goto bail;
  err = ISOIFF_GetNALUnitsWithTypeFromHEVCDecConf(decoderConfig, NAL_UNIT_PPS, ppsNALUnits);
  if(err) goto bail;
  err = ISOIFF_GetNALUnitsWithTypeFromHEVCDecConf(decoderConfig, NAL_UNIT_PREFIX_SEI,
                                                  prefixSEI_NALUnits);
  if(err) goto bail;
  err = ISOIFF_GetNALUnitsWithTypeFromHEVCDecConf(decoderConfig, NAL_UNIT_SUFFIX_SEI,
                                                  suffixSEI_NALUnits);
  if(err) goto bail;

  if(vpsNALUnits->entryCount > 0)
  {
    err = MP4GetListEntry(vpsNALUnits, 0, (char **)&naluH);
    if(err) goto bail;
    err = MP4HandleCat(bitstreamH, zeroByteH);
    if(err) goto bail;
    err = MP4HandleCat(bitstreamH, startCodePrefixH);
    if(err) goto bail;
    err = MP4HandleCat(bitstreamH, naluH);
    if(err) goto bail;
  }

  if(spsNALUnits->entryCount > 0)
  {
    err = MP4GetListEntry(spsNALUnits, 0, (char **)&naluH);
    if(err) goto bail;
    err = MP4HandleCat(bitstreamH, zeroByteH);
    if(err) goto bail;
    err = MP4HandleCat(bitstreamH, startCodePrefixH);
    if(err) goto bail;
    err = MP4HandleCat(bitstreamH, naluH);
    if(err) goto bail;
  }

  if(ppsNALUnits->entryCount > 0)
  {
    err = MP4GetListEntry(ppsNALUnits, 0, (char **)&naluH);
    if(err) goto bail;
    err = MP4HandleCat(bitstreamH, zeroByteH);
    if(err) goto bail;
    err = MP4HandleCat(bitstreamH, startCodePrefixH);
    if(err) goto bail;
    err = MP4HandleCat(bitstreamH, naluH);
    if(err) goto bail;
  }

  for(i = 0; i < prefixSEI_NALUnits->entryCount; i++)
  {
    err = MP4GetListEntry(prefixSEI_NALUnits, i, (char **)&naluH);
    if(err) goto bail;
    err = MP4HandleCat(bitstreamH, startCodePrefixH);
    if(err) goto bail;
    err = MP4HandleCat(bitstreamH, naluH);
    if(err) goto bail;
  }

  err = MP4NewHandle(0, &imgDatH);
  if(err) goto bail;
  err = ISOIFF_GetImageData(image, imgDatH);
  if(err) goto bail;

  err =
    ISOIFF_CreateHEVCItemDataFromHandle(imgDatH, &hevcItemData, decoderConfig->lengthSizeMinusOne);
  if(err) goto bail;
  err = ISOIFF_GetNALUDataHandlesFromHEVCItemData(hevcItemData, imageNALUnits);
  if(err) goto bail;

  for(i = 0; i < imageNALUnits->entryCount; i++)
  {
    err = MP4GetListEntry(imageNALUnits, i, (char **)&naluH);
    if(err) goto bail;
    err = MP4HandleCat(bitstreamH, startCodePrefixH);
    if(err) goto bail;
    err = MP4HandleCat(bitstreamH, naluH);
    if(err) goto bail;
  }

  for(i = 0; i < suffixSEI_NALUnits->entryCount; i++)
  {
    err = MP4GetListEntry(suffixSEI_NALUnits, i, (char **)&naluH);
    if(err) goto bail;
    err = MP4HandleCat(bitstreamH, startCodePrefixH);
    if(err) goto bail;
    err = MP4HandleCat(bitstreamH, naluH);
    if(err) goto bail;
  }

  err = MP4DisposeHandle(startCodePrefixH);
  if(err) goto bail;
  err = MP4DisposeHandle(imgDatH);
  if(err) goto bail;
  err = MP4DisposeHandle(zeroByteH);
  if(err) goto bail;

  err = MP4DeleteLinkedList(vpsNALUnits);
  if(err) goto bail;
  err = MP4DeleteLinkedList(spsNALUnits);
  if(err) goto bail;
  err = MP4DeleteLinkedList(ppsNALUnits);
  if(err) goto bail;
  err = MP4DeleteLinkedList(imageNALUnits);
  if(err) goto bail;
  err = MP4DeleteLinkedList(prefixSEI_NALUnits);
  if(err) goto bail;
  err = MP4DeleteLinkedList(suffixSEI_NALUnits);
  if(err) goto bail;

  err = ISOIFF_FreeHEVCItemData(hevcItemData);
  if(err) goto bail;
bail:
  return err;
}
