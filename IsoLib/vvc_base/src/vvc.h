#ifndef __VVC_H__
#define __VVC_H__

#define AGGREGATOR_NAL_TYPE 48
#define VVC_MAX_LAYERS 4
#define VVC_MAX_NUM_LAYER_SETS 1024

/* slice type */
enum
{
  VVC_SLICE_B       = 0,
  VVC_SLICE_P       = 1,
  VVC_SLICE_I       = 2,
  VVC_SLICE_UNKNOWN = 10,
};

/* VVC NAL unit types */
enum
{
  /* Trail N VVC slice */
  VVC_NALU_SLICE_TRAIL = 0,
  /* STSA N VVC slice */
  VVC_NALU_SLICE_STSA = 1,
  /* STSA N VVC slice */
  VVC_NALU_SLICE_RADL = 2,
  /* STSA N VVC slice */
  VVC_NALU_SLICE_RASL = 3,
  /* IDR with RADL VVC slice */
  VVC_NALU_SLICE_IDR_W_RADL = 7,
  /* IDR DLP VVC slice */
  VVC_NALU_SLICE_IDR_N_LP = 8,
  /* CRA VVC slice */
  VVC_NALU_SLICE_CRA = 9,
  /* CRA VVC slice */
  VVC_NALU_SLICE_GDR = 10,

  /* Operation Point Info */
  VVC_NALU_OPI = 12,
  /* Decode Capability Info */
  VVC_NALU_DCI = 13,
  /* Video Parameter Set */
  VVC_NALU_VID_PARAM = 14,
  /* Sequence Parameter Set */
  VVC_NALU_SEQ_PARAM = 15,
  /* Picture Parameter Set */
  VVC_NALU_PIC_PARAM = 16,
  /* APS prefix */
  VVC_NALU_APS_PREFIX = 17,
  /* APS suffix */
  VVC_NALU_APS_SUFFIX = 18,
  /* Picture Header */
  VVC_NALU_PIC_HEADER = 19,
  /* AU delimiter */
  VVC_NALU_ACCESS_UNIT = 20,
  /* End of sequence */
  VVC_NALU_END_OF_SEQ = 21,
  /* End of stream */
  VVC_NALU_END_OF_STREAM = 22,
  /* prefix SEI message */
  VVC_NALU_SEI_PREFIX = 23,
  /* suffix SEI message */
  VVC_NALU_SEI_SUFFIX = 24,
  /* Filler Data */
  VVC_NALU_FILLER_DATA = 25,
};

MP4Err vvc_parse_sps_minimal(BitBuffer *bb, struct vvc_sps* sps);
MP4Err vvc_parse_pps_minimal(BitBuffer *bb, struct vvc_pps* pps);
MP4Err vvc_parse_slice_header_minimal(BitBuffer *bb, struct vvc_poc* poc, struct vvc_slice_header* header,
struct vvc_sps* sps, struct vvc_pps* pps);
MP4Err vvc_parse_ph_minimal(BitBuffer *bb, struct vvc_picture_header *ph, struct vvc_sps *sps,
                            struct vvc_pps *pps);

u8* stripNALEmulation(u8* buffer, u32* bufferLen);
s32 parseVVCNal(FILE* input, u8** data, int* data_len);
ISOErr analyze_vvc_stream(FILE* input, struct vvc_stream* stream);

#endif
