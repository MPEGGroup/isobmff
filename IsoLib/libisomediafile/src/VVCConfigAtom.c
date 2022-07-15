/**
 * @file VVCConfigAtom.c
 * @brief VVC configuration atom implementation
 * @version 0.1
 *
 * @copyright This software module was originally developed by Apple Computer, Inc. in the course of
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
 * or derivative works. Copyright (c) 1999.
 *
 */

#include "MP4Atoms.h"
#include "MP4Descriptors.h"
#include <stdlib.h>
#include <string.h>

static void destroy(MP4AtomPtr s)
{
  MP4Err err;
  u32 i;
  ISOVVCConfigAtomPtr self;
  err  = MP4NoErr;
  self = (ISOVVCConfigAtomPtr)s;
  if(self == NULL) BAILWITHERROR(MP4BadParamErr)
  if(self->num_of_arrays)
  {
    for(i = 0; i < 7; i++)
    {
      err = MP4DeleteLinkedList(self->arrays[i].nalList);
      if(err) goto bail;
      self->arrays[i].nalList = NULL;
    }
  }
  if(self->super) self->super->destroy(s);
bail:
  TEST_RETURN(err);
  return;
}

static MP4Err serialize(struct MP4Atom *s, char *buffer)
{
  MP4Err err;
  u32 x, array_index, ui, j, cnt, helper, numByteGciLower;
  s32 i;
  ISOVVCConfigAtomPtr self = (ISOVVCConfigAtomPtr)s;
  err                      = MP4NoErr;

  err = MP4SerializeCommonFullAtomFields((MP4FullAtomPtr)s, buffer);
  if(err) goto bail;
  buffer += self->bytesWritten;

  /* reserver '11111' b + LengthSizeMinusOne(2) + ptl_present_flag(1) */
  x = (0x1f << 3) | (self->LengthSizeMinusOne << 1) | self->ptl_present_flag;
  PUT8_V(x);

  if(self->ptl_present_flag)
  {
    /* ols_idx(9) + num_sublayers(3) + constant_frame_rate(2) + chroma_format_idc(2) */
    x = (self->ols_idx << 7) | (self->num_sublayers << 4) | (self->constant_frame_rate << 2) |
        (self->chroma_format_idc);
    PUT16_V(x);

    /* bitDipth(3) + '11111'b */
    x = (self->bit_depth_minus8 << 5) | 0x1F;
    PUT8_V(x);

    /* native_ptl */
    /* VvcPTLRecord */
    {
      PUT8(native_ptl.num_bytes_constraint_info);

      x = (self->native_ptl.general_profile_idc << 1) | self->native_ptl.general_tier_flag;
      PUT8_V(x);

      PUT8(native_ptl.general_level_idc);

      x = (self->native_ptl.ptl_frame_only_constraint_flag << 7) |
          (self->native_ptl.ptl_multi_layer_enabled_flag << 6) |
          (self->native_ptl.general_constraint_info_upper);
      PUT8_V(x);

      if(self->native_ptl.num_bytes_constraint_info > 1)
      {
        numByteGciLower = self->native_ptl.num_bytes_constraint_info - 1;
        PUTBYTES(*self->native_ptl.general_constraint_info_lower, numByteGciLower);
      }

      cnt = 7;
      x   = 0;
      for(i = self->num_sublayers - 2; i >= 0; i--) /* int with u32 ? */
      {
        helper = self->native_ptl.subPTL[i].ptl_sublayer_level_present_flag << cnt;
        cnt--;
        x |= helper;
      }
      if(self->num_sublayers >= 2)
      {
        PUT8_V(x);
      }

      for(i = self->num_sublayers - 2; i >= 0; i--)
      {
        if(self->native_ptl.subPTL[i].ptl_sublayer_level_present_flag)
        {
          PUT8(native_ptl.subPTL[i].sublayer_level_idc);
        }
      }

      PUT8(native_ptl.ptl_num_sub_profiles);

      for(j = 0; j < self->native_ptl.ptl_num_sub_profiles; j++)
      {
        PUT32(native_ptl.general_sub_profile_idc[j]);
      }
    }

    PUT16(max_picture_width);

    PUT16(max_picture_height);

    PUT16(avg_frame_rate);
  }

  PUT8(num_of_arrays);

  for(array_index = 0; array_index < 7; array_index++)
  {
    u32 num_nalus;
    err = MP4GetListEntryCount(self->arrays[array_index].nalList, &num_nalus);
    if(err) goto bail;
    if(!num_nalus) continue;
    x =
      (self->arrays[array_index].array_completeness << 7) | self->arrays[array_index].NAL_unit_type;
    PUT8_V(x);
    if(self->arrays[array_index].NAL_unit_type != 13 /*DCI_NUT*/ &&
       self->arrays[array_index].NAL_unit_type != 12 /*OPI_NUT*/)
    {
      /* num_nalus */
      PUT16_V(num_nalus);
    }
    else
    {
      /* When not present, the value of numNalus is inferred to be equal to 1 */
      assert(num_nalus == 1);
      num_nalus = 1;
    }
    for(ui = 0; ui < num_nalus; ui++)
    {
      MP4Handle b;
      u32 the_size;
      err = MP4GetListEntry(self->arrays[array_index].nalList, ui, (char **)&b);
      if(err) goto bail;
      err = MP4GetHandleSize(b, &the_size);
      if(err) goto bail;
      PUT16_V(the_size);
      PUTBYTES(*b, the_size);
    }
  }

bail:
  TEST_RETURN(err);
  return err;
}

static MP4Err calculateSize(struct MP4Atom *s)
{
  MP4Err err;
  ISOVVCConfigAtomPtr self = (ISOVVCConfigAtomPtr)s;
  u32 j, i, y;
  s32 x;
  err = MP4NoErr;

  err = MP4CalculateFullAtomFieldSize((MP4FullAtomPtr)s);
  if(err) goto bail;

  self->size += 1;

  if(self->ptl_present_flag)
  {
    self->size += 3;
    /* PTL */
    {
      self->size += 3;
      self->size += self->native_ptl.num_bytes_constraint_info;
      /* ptl_sublayer_level_present_flag */
      if(self->num_sublayers >= 2)
      {
        self->size += 1;
      }
      for(x = self->num_sublayers - 2; x >= 0; x--)
      {
        if(self->native_ptl.subPTL[x].ptl_sublayer_level_present_flag) self->size += 1;
      }
      /* ptl_num_sub_profiles */
      self->size += 1;
      for(y = 0; y < self->native_ptl.ptl_num_sub_profiles; y++)
      {
        self->size += 4;
      }
    }
    self->size += 6;
  }

  /* num_of_arrays */
  self->size += 1;

  if(self->num_of_arrays)
  {
    for(j = 0; j < 7; j++)
    {
      u32 num_nalus;
      err = MP4GetListEntryCount(self->arrays[j].nalList, &num_nalus);
      if(err) goto bail;
      if(!num_nalus) continue;
      self->size += 1;
      if(self->arrays[j].NAL_unit_type != 13 /*DCI_NUT*/ &&
         self->arrays[j].NAL_unit_type != 12 /*OPI_NUT*/)
      {
        self->size += 2;
      }
      else
      {
        assert(num_nalus == 1);
        num_nalus = 1;
      }

      for(i = 0; i < num_nalus; i++)
      {
        MP4Handle b;
        u32 the_size;
        /* nal_unit_length */
        self->size += 2;
        err = MP4GetListEntry(self->arrays[j].nalList, i, (char **)&b);
        if(err) goto bail;
        err = MP4GetHandleSize(b, &the_size);
        if(err) goto bail;
        self->size += the_size;
      }
    }
  }

bail:
  TEST_RETURN(err);
  return err;
}

static MP4Err createFromInputStream(MP4AtomPtr s, MP4AtomPtr proto, MP4InputStreamPtr inputStream)
{
  MP4Err err;
  ISOVVCConfigAtomPtr self = (ISOVVCConfigAtomPtr)s;
  u32 x, num_nalus, array_index, j, numBytesGciLower, ui;
  s32 i;
  u8 helper;
  u32 nalTypeLut[7] = {15, 16, 14, 12, 13, 17, 23};
  u8 nalType;

  err = MP4NoErr;
  if(self == NULL) BAILWITHERROR(MP4BadParamErr);
  err = self->super->createFromInputStream(s, proto, (char *)inputStream);
  if(err) goto bail;

  GET8_V(x);
  self->LengthSizeMinusOne = (x & 0x06) >> 1;
  self->ptl_present_flag   = x & 0x01;
  DEBUG_SPRINTF("LengthSizeMinusOne = %u", self->LengthSizeMinusOne);
  DEBUG_SPRINTF("ptl_present_flag = %u", self->ptl_present_flag);

  if(self->ptl_present_flag)
  {
    GET16_V(x);
    self->ols_idx             = (x & 0xff80) >> 7;
    self->num_sublayers       = (x & 0x0070) >> 4;
    self->constant_frame_rate = (x & 0x000c) >> 2;
    self->chroma_format_idc   = x & 0x0003;

    GET8_V(x);
    self->bit_depth_minus8 = (x & 0xe0) >> 5;

    DEBUG_SPRINTF("ols_idx = %u", self->ols_idx);
    DEBUG_SPRINTF("num_sublayers = %u", self->num_sublayers);
    DEBUG_SPRINTF("constant_frame_rate = %u", self->constant_frame_rate);
    DEBUG_SPRINTF("chroma_format_idc = %u", self->chroma_format_idc);
    DEBUG_SPRINTF("bit_depth_minus8 = %u", self->bit_depth_minus8);

    /* PTL recoder */
    {
      GET8_V(x);
      self->native_ptl.num_bytes_constraint_info = x & 0x3f;

      GET8_V(x);
      self->native_ptl.general_profile_idc = (x & 0xfe) >> 1;
      self->native_ptl.general_tier_flag   = x & 0x01;
      DEBUG_SPRINTF("general_profile_idc = %u", self->native_ptl.general_profile_idc);
      DEBUG_SPRINTF("general_tier_flag = %u", self->native_ptl.general_tier_flag);

      GET8(native_ptl.general_level_idc);

      GET8_V(x);
      self->native_ptl.ptl_frame_only_constraint_flag = (x & 0x80) >> 7;
      self->native_ptl.ptl_multi_layer_enabled_flag   = (x & 0x40) >> 6;
      DEBUG_SPRINTF("ptl_frame_only_constraint_flag = %u",
                    self->native_ptl.ptl_frame_only_constraint_flag);
      DEBUG_SPRINTF("ptl_multi_layer_enabled_flag = %u",
                    self->native_ptl.ptl_multi_layer_enabled_flag);

      self->native_ptl.general_constraint_info_upper = x & 0x3f;
      if(self->native_ptl.num_bytes_constraint_info > 1)
      {
        numBytesGciLower = self->native_ptl.num_bytes_constraint_info - 1;
        err = MP4NewHandle(numBytesGciLower, &self->native_ptl.general_constraint_info_lower);
        if(err) goto bail;
        /* byte_aligned() */
        GETBYTES_V_MSG(numBytesGciLower, *self->native_ptl.general_constraint_info_lower,
                       "general_constraint_info_lower");
      }

      if(self->num_sublayers >= 2)
      {
        GET8_V(x);
      }
      helper = 0x80;
      for(i = self->num_sublayers - 2; i >= 0; i--)
      {
        self->native_ptl.subPTL[i].ptl_sublayer_level_present_flag = (x & helper) ? 1 : 0;
        helper >>= 1;
      }

      for(i = self->num_sublayers - 2; i >= 0; i--)
      {
        if(self->native_ptl.subPTL[i].ptl_sublayer_level_present_flag)
        {
          GET8(native_ptl.subPTL[i].sublayer_level_idc);
        }
      }

      GET8(native_ptl.ptl_num_sub_profiles);
      for(j = 0; j < self->native_ptl.ptl_num_sub_profiles; j++)
      {
        GET32(native_ptl.general_sub_profile_idc[j]);
      }
    }

    GET16(max_picture_width);
    GET16(max_picture_height);
    GET16(avg_frame_rate);
  }

  GET8(num_of_arrays);

  for(ui = 0; ui < self->num_of_arrays; ui++)
  {
    GET8_V(x);
    nalType     = x & 0x1f;
    array_index = ui;
    for(j = 0; j < 7; j++)
    {
      if(nalType == nalTypeLut[j])
      {
        array_index = j;
        break;
      }
      if(j == 6)
      {
        /* unknown nalu type */
        assert(0);
      }
    }
    self->arrays[array_index].array_completeness = (x & 0x80) ? 1 : 0;
    self->arrays[array_index].NAL_unit_type      = x & 0x1f;
    DEBUG_SPRINTF("NAL_unit_type = %u", self->arrays[array_index].NAL_unit_type);
    err = MP4MakeLinkedList(&self->arrays[array_index].nalList);
    if(err) goto bail;

    if(self->arrays[array_index].NAL_unit_type != 13 /*DCI_NUT*/ &&
       self->arrays[array_index].NAL_unit_type != 12 /*OPI_NUT*/)
    {
      GET16_V(num_nalus);
    }
    else
    {
      num_nalus = 1;
    }

    for(j = 0; j < num_nalus; j++)
    {
      MP4Handle nal_unit;
      u32 nal_unit_length;

      GET16_V(nal_unit_length);
      err = MP4NewHandle(nal_unit_length, &nal_unit);
      if(err) goto bail;
      GETBYTES_V_MSG(nal_unit_length, *nal_unit, "NAL");
      err = MP4AddListEntry((void *)nal_unit, self->arrays[array_index].nalList);
      if(err) goto bail;
    }
  }

bail:
  TEST_RETURN(err);
  return err;
}

static MP4Err addParameterSet(struct ISOVVCConfigAtom *self, MP4Handle ps, u32 nalu)
{
  MP4Err err;
  MP4Handle b;
  u32 the_size;
  u32 i;

  err = MP4NoErr;
  err = MP4GetHandleSize(ps, &the_size);
  if(err) goto bail;
  err = MP4NewHandle(the_size, &b);
  if(err) goto bail;
  memcpy(*b, *ps, the_size);

  for(i = 0; i < 7; i++)
  {
    if(self->arrays[i].NAL_unit_type == nalu)
    {
      u32 nalCount = 0;
      err          = MP4GetListEntryCount(self->arrays[i].nalList, &nalCount);
      if(err) goto bail;
      if(!nalCount)
      {
        self->num_of_arrays++;
      }
      err = MP4AddListEntry((void *)b, self->arrays[i].nalList);
      if(err) goto bail;
      break;
    }
    if(i == 6)
    {
      BAILWITHERROR(MP4BadParamErr);
    }
  }

bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err getParameterSet(struct ISOVVCConfigAtom *self, MP4Handle ps, u32 nalu, u32 index)
{
  MP4Err err;
  MP4Handle b = NULL;
  u32 the_size;
  u32 i;

  err = MP4NoErr;

  for(i = 0; i < 7; i++)
  {
    if(self->arrays[i].NAL_unit_type == nalu)
    {
      err = MP4GetListEntry(self->arrays[i].nalList, index - 1, (char **)&b);
      if(err) goto bail;
      break;
    }
    if(i == 6)
    {
      BAILWITHERROR(MP4BadParamErr);
    }
  }

  err = MP4GetHandleSize(b, &the_size);
  if(err) goto bail;
  err = MP4SetHandleSize(ps, the_size);
  if(err) goto bail;
  memcpy(*ps, *b, the_size);

bail:
  TEST_RETURN(err);

  return err;
}

MP4Err MP4CreateVVCConfigAtom(ISOVVCConfigAtomPtr *outAtom)
{
  MP4Err err;
  ISOVVCConfigAtomPtr self;
  u32 i;
  /*
    There is a set of arrays to carry initialization non-VCL NAL units. The NAL unit types are
    restricted to indicate OPI(12), DCI(13), VPS(14), SPS(15), PPS(16), prefix APS(17), and prefix
    SEI(23) NAL units only. NAL unit types that are reserved in ISO/IEC 23090-3 and in this
    specification may acquire a definition in the future specification, and readers should ignore
    arrays with reserved or unpermitted values of NAL unit type.
  */
  u32 nalType[7] = {15, 16, 14, 12, 13, 17, 23};
  self           = (ISOVVCConfigAtomPtr)calloc(1, sizeof(ISOVVCConfigAtom));
  TESTMALLOC(self);

  err = MP4CreateFullAtom((MP4AtomPtr)self);
  if(err) goto bail;
  self->type                  = ISOVVCConfigAtomType;
  self->name                  = "VvcConfigurationBox";
  self->createFromInputStream = (cisfunc)createFromInputStream;
  self->destroy               = destroy;
  self->calculateSize         = calculateSize;
  self->serialize             = serialize;
  self->addParameterSet       = addParameterSet;
  self->getParameterSet       = getParameterSet;

  for(i = 0; i < 7; i++)
  {
    err = MP4MakeLinkedList(&self->arrays[i].nalList);
    if(err) goto bail;
    self->arrays[i].array_completeness = 1;
    self->arrays[i].NAL_unit_type      = nalType[i];
    self->arrays[i].array_completeness = 0;
  }

  *outAtom = self;
bail:
  TEST_RETURN(err);
  return err;
}
