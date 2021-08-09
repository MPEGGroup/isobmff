//
#ifndef __VVCC__
#define __VVCC__

#include "MP4Atoms.h"
#include "MP4Descriptors.h"
#include <stdlib.h>
#include <string.h>

#if VVCC
static MP4Err createFromInputStream(MP4AtomPtr s, MP4AtomPtr proto, MP4InputStreamPtr inputStream)
{
  MP4Err err;
  ISOVVCConfigAtomPtr self = (ISOVVCConfigAtomPtr)s;
  u32 x, count, array_index;

  err = MP4NoErr;
  if(self == NULL) BAILWITHERROR(MP4BadParamErr)
  err = self->super->createFromInputStream(s, proto, (char *)inputStream);
  if(err) goto bail;

  GET8_V(x);
  self->LengthSizeMinusOne = (x & 0x07) >> 1;
  self->ptl_present_flag   = x & 0x01;

  if(self->ptl_present_flag)
  {
    GET16_V(x);
    self->ols_idx = (x & 0xffff) >> 7;
    self->num_sublayers = (x & 0x007f) >> 4;
    self->constant_frame_rate = (x & 0x000f) >> 2;
    self->chroma_format_idc   = x & 0x0003;

    //PTL recoder
    {
      GET8_V(x);
      self->native_ptl.num_bytes_constraint_info = x & 0x3f;

      GET8_V(x);
      self->native_ptl.general_profile_idc = (x & 0xff) >> 1;
      self->native_ptl.general_tier_flag   = x & 0x01;

      GET8_V(x);
      self->native_ptl.general_level_idc = x;

      GET8_V(x);
      self->native_ptl.ptl_frame_only_constraint_flag = (x & 0xff) >> 7;
      self->native_ptl.ptl_multi_layer_enabled_flag   = (x & 0x7f) >> 6;

      //if(self->native_ptl.num_bytes_constraint_info == 1)
      //{
      //  self->native_ptl.general_constraint_info = 0;
      //}

      self->native_ptl.general_constraint_info = x & 0x3f;
      for(u32 i = 1; i < self->native_ptl.num_bytes_constraint_info; i++)
      {
        self->native_ptl.general_constraint_info <<= 8;
        GET8_V(x);
        // out of range pj??
        self->native_ptl.general_constraint_info |= x;
      }

      GET8_V(x);
      int cnt = 7;
      for(int i = self->num_sublayers - 2; i >= 0; i--)
      {
        u8 helper = 0x01;
        helper <<= cnt;
        cnt--;
        self->native_ptl.subPTL[i].ptl_sublayer_level_present_flag = x & helper; 
      }

      for(int i = self->num_sublayers - 2; i >= 0; i--)
      {
        if(self->native_ptl.subPTL[i].ptl_sublayer_level_present_flag)
        {
          GET8(native_ptl.subPTL[i].sublayer_level_idc);
        }
      }

      GET8(native_ptl.ptl_num_sub_profiles);
      for(u32 j = 0; j < self->native_ptl.ptl_num_sub_profiles; j++)
      {
        GET32(native_ptl.subPTL[j].general_sub_profile_idc);
      }   
    }

    GET8_V(x);
    self->bit_depth_minus8 = (x & 0xff) >> 5;

    GET16(max_picture_width);
    GET16(max_picture_height);
    GET16(avg_frame_rate);
  }
    
  GET8(num_of_arrays);

  for(array_index = 0; array_index < self->num_of_arrays; array_index++)
  {
    GET8_V(x);
    self->arrays[array_index].array_completeness = (x & 0x80) ? 1 : 0;
    self->arrays[array_index].NALtype            = x & 0x1f;
    err = MP4MakeLinkedList(&self->arrays[array_index].nalList);
    if(err) goto bail;

    if(self->arrays[array_index].NALtype != 13 /*DCI_NUT*/ && self->arrays[array_index].NALtype != 12 /*OPI_NUT*/)
    {
      GET16_V(count);
    }
    else
    {
      count = 1;
    }
    u32 i;
    for(i = 0; i < count; i++)
    {
      MP4Handle b;
      u32 the_size;

      GET16_V(the_size);
      err = MP4NewHandle(the_size, &b);
      if(err) goto bail;

      GETBYTES_V_MSG(the_size, *b, "NAL");
      err = MP4AddListEntry((void *)b, self->arrays[array_index].nalList);
      if(err) goto bail;
    }
  }

bail:
  TEST_RETURN(err);

  return err;
}

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
    for(i = 0; i < self->num_of_arrays; i++)
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

static MP4Err calculateSize(struct MP4Atom *s)
{
  MP4Err err;
  ISOVVCConfigAtomPtr self = (ISOVVCConfigAtomPtr)s;
  u32 i, ii;
  err = MP4NoErr;

  err = MP4CalculateFullAtomFieldSize((MP4FullAtomPtr)s);
  if(err) goto bail;
  // 5 + 2 + 1 + num_of_arrays(8)
  self->size += 2;

  if(self->ptl_present_flag)
  {
    self->size += 3;
    self->size += self->native_ptl.num_bytes_constraint_info;
    {
      self->size += 1;
      for(int x = self->num_sublayers - 2; x >= 0; x--)
      {
        if(self->native_ptl.subPTL->ptl_sublayer_level_present_flag) self->size += 1;
      }
      for(u32 y = 0; y < self->native_ptl.ptl_num_sub_profiles; y++)
      {
        self->size += 4;
      }
    }
    self->size += 6; 
  }

  if(self->num_of_arrays)
  {
    for(i = 0; i < self->num_of_arrays; i++)
    {
      self->size += 1;

      u32 count;
      err = MP4GetListEntryCount(self->arrays[i].nalList, &count);
      if(err) goto bail;

      if(self->arrays[i].NALtype != 13 /*DCI_NUT*/ && self->arrays[i].NALtype != 12 /*OPI_NUT*/)
      {
        self->size += 2;
      }
      else
      {
        assert(count == 1);
        count = 1;
      }

      for(ii = 0; ii < count; ii++)
      {
        MP4Handle b;
        u32 the_size;
        err = MP4GetListEntry(self->arrays[i].nalList, ii, (char **)&b);
        if(err) goto bail;
        err = MP4GetHandleSize(b, &the_size);
        if(err) goto bail;
        self->size += 2 + the_size;
      }
    }
  }

bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err serialize(struct MP4Atom* s, char* buffer) 
{
  MP4Err err;
  u32 x, array_index;
  ISOVVCConfigAtomPtr self = (ISOVVCConfigAtomPtr)s;
  err = MP4NoErr;

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
    // VvcPTLRecord
    {
      PUT8(native_ptl.num_bytes_constraint_info);

      x = (self->native_ptl.general_profile_idc << 1) | self->native_ptl.general_tier_flag;
      PUT8_V(x);
      
      PUT8(native_ptl.general_level_idc);

      x = (self->native_ptl.ptl_frame_only_constraint_flag << 7) |
          (self->native_ptl.ptl_multi_layer_enabled_flag << 6);
      int gciNum = 8*self->native_ptl.num_bytes_constraint_info - 2;
      gciNum -= 6;
      u32 h1 = (self->native_ptl.general_constraint_info >> gciNum) & 0xff;
      x |= h1;
      PUT8_V(x);
      while(gciNum > 0)
      {
        x = (self->native_ptl.general_constraint_info >> gciNum) & 0xff;
        PUT8_V(x);
        gciNum -= 8;
      }

      int cnt = 7;
      x       = 0;
      for(int i = self->num_sublayers - 2; i >= 0; i--) //int with u32 ?
      {
        u32 helper = (self->native_ptl.subPTL[i].ptl_sublayer_level_present_flag << cnt) & 0xff;
        cnt--;
        x |= helper;
      }
      PUT8_V(x);

      for(int i = self->num_sublayers - 2; i >= 0; i--)
      {
        if(self->native_ptl.subPTL[i].ptl_sublayer_level_present_flag)
        {
          PUT8(native_ptl.subPTL[i].sublayer_level_idc);
        }
      }

      PUT8(native_ptl.ptl_num_sub_profiles);

      for(u32 j = 0; j < self->native_ptl.ptl_num_sub_profiles; j++)
      {
        PUT32(native_ptl.subPTL[j].general_sub_profile_idc);
      }
    }

    PUT16(max_picture_width);

    PUT16(max_picture_height);

    PUT16(avg_frame_rate);
  }

  PUT8(num_of_arrays);

  for(array_index = 0; array_index < self->num_of_arrays; array_index++)
  {
    u32 count;
    err = MP4GetListEntryCount(self->arrays[array_index].nalList, &count);
    if(err) goto bail;
    x = (self->arrays[array_index].array_completeness << 7) | self->arrays[array_index].NALtype;
    PUT8_V(x);
    if(self->arrays[array_index].NALtype != 13 /*DCI_NUT*/ && self->arrays[array_index].NALtype != 12 /*OPI_NUT*/)
    {
      /* num_nalus */
      PUT16_V(count);
    }
    else
    {
      // When not present, the value of numNalus is inferred to be equal to 1
      assert(count == 1);
      count = 1;
    }
    u32 i;
    for(i = 0; i < count; i++)
    {
      MP4Handle b;
      u32 the_size;
      err = MP4GetListEntry(self->arrays[array_index].nalList, i, (char **)&b);
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

  for(i = 0; i < 8; i++) //pj??
  {
    if(self->arrays[i].NALtype == nalu)
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

  for(i = 0; i < self->num_of_arrays; i++)
  {
    if(self->arrays[i].NALtype == nalu)
    {
      err = MP4GetListEntry(self->arrays[i].nalList, index - 1, (char **)&b);
      if(err) goto bail;
      break;
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

#endif

#if VVCC
MP4Err MP4CreateVVCConfigAtom(ISOVVCConfigAtomPtr *outAtom)
{
  MP4Err err;
  ISOVVCConfigAtomPtr self;
  u32 i;

  self = (ISOVVCConfigAtomPtr)calloc(1, sizeof(ISOVVCConfigAtom));
  TESTMALLOC(self);

  err = MP4CreateFullAtom((MP4AtomPtr)self);
  if(err) goto bail;
  self->type                  = ISOVVCConfigAtomType;
  self->name                  = "VVCConfig";
  self->createFromInputStream = (cisfunc)createFromInputStream;
  self->destroy               = destroy;
  self->calculateSize         = calculateSize;
  self->serialize             = serialize;
  self->addParameterSet       = addParameterSet;
  self->getParameterSet       = getParameterSet;

  for(i = 0; i < 8; i++)
  {
    err = MP4MakeLinkedList(&self->arrays[i].nalList);
    if(err) goto bail;
    self->arrays[i].NALtype            = 14 + i;
    self->arrays[i].array_completeness = 1;
  }

  *outAtom = self;
bail:
  TEST_RETURN(err);

  return err;
}
#endif

#endif // __VVCC__





