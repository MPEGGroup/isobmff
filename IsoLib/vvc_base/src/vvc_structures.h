/*
This software module was originally developed by Apple Computer, Inc.
in the course of development of MPEG-4. 
This software module is an implementation of a part of one or 
more MPEG-4 tools as specified by MPEG-4. 
ISO/IEC gives users of MPEG-4 free license to this
software module or modifications thereof for use in hardware 
or software products claiming conformance to MPEG-4.
Those intending to use this software module in hardware or software
products are advised that its use may infringe existing patents.
The original developer of this software module and his/her company,
the subsequent editors and their companies, and ISO/IEC have no
liability for use of this software module or modifications thereof
in an implementation.
Copyright is not released for non MPEG-4 conforming
products. Apple Computer, Inc. retains full right to use the code for its own
purpose, assign or donate the code to a third party and to
inhibit third parties from using the code for non 
MPEG-4 conforming products.
This copyright notice must be included in all copies or
derivative works. Copyright (c) 1999.
*/

#ifndef __VVC_STRUCTURES_H__
#define __VVC_STRUCTURES_H__


//When a VCL NAL unit has sh_picture_header_in_slice_header_flag equal to 1 
//or is the first VCL NAL unit that follows a PH NAL unit, 
//the VCL NAL unit is the first VCL NAL unit of a picture.

typedef struct {
	u16 roll_count;
	u16 first_output_sample;
	u32 sample_offset[];
} alst_data, *alst_dataptr;

struct vvc_picture_header
{
  u8 gdr_or_irap_pic_flag, non_ref_pic_flag, gdr_pic_flag, inter_slice_allowed_flag,
    intra_slice_allowed_flag;
  u32 pic_parameter_set_id, pic_order_cnt_lsb, recovery_poc_cnt;
  u8 poc_msb_cycle_present_flag;
  u32 poc_msb_cycle_val;
  /* compute POC */
  u32 poc, poc_lsb, poc_msb, poc_msb_prev, poc_lsb_prev;
};

struct vvc_slice_header {
  struct vvc_picture_header ph;

  u32 frame_num;
  s32 poc;
	s32 poc_offset;
  u32 max_poc_lsb;
  //u32 frame_num_prev;

	u32 sample_number;
	u32 slice_segment_address;
	u8 dependent_slice;
  u8 slice_type, nal_unit_type;

	u8 is_first_slice;
	u8 sh_picture_header_in_slice_header_flag;
  u32 sh_subpic_id;

	u32 num_entry_point_offsets;
  u32 *entry_point_offset_minus1;
  u32 num_slices;
  u32 *slice_offsets;

	u8 *non_VCL_data;	
  u32 non_VCL_datalen;
};


struct vvc_sps {
  u8 sps_max_sublayers, sps_chroma_format_idc, sps_log2_ctu_size_minus5,
    sps_ptl_dpb_hrd_params_present_flag;
  u32 sps_pic_width_max_in_luma_samples, sps_pic_height_max_in_luma_samples;
  u8 sps_subpic_info_present_flag;
  u32 sps_subpic_id_len_minus1, sps_bitdepth_minus8;
  u32 sps_poc_msb_cycle_flag;
  u8 sps_entropy_coding_sync_enabled_flag, sps_entry_point_offsets_present_flag,
    sps_log2_max_pic_order_cnt_lsb_minus4, sps_poc_msb_cycle_len_minus1, sps_num_extra_ph_bytes,
    sps_num_extra_sh_bytes;
  u32 NumExtraPhBits, NumExtraShBits;

};

struct vvc_pps {
  u32 pps_num_subpics_minus1;
  u32 pps_subpic_id_len_minus1;
  u32* pps_subpic_id;
  u8 pps_rect_slice_flag;
  u32 pps_num_slices_in_pic_minus1;
};

struct vvc_stream {
	u32 used_count;
	u32 allocated_count;
	struct vvc_slice_header **header;
	struct vvc_sps sps;
	struct vvc_pps pps;
};

typedef struct 
{
  u8 subpic_id_info_flag;
  u16 num_subpic_ref_idx;
  u16* subp_track_ref_idx;
  struct
  {
    u8 subpic_id_len_minus1, start_code_emul_flag, pps_sps_subpic_id_flag, pps_id, sps_id;
    u16 subpic_id_bit_pos;
  };
} spor_box, *spor_boxPtr;

typedef struct
{
  u8 subpic_id_info_flag;
  u16 num_alternate_region_set, num_regions_minus1;
  u16 *num_regions_in_set, *alternate_region_set_id, *region_id;
  u16 **groupID;
  struct
  {
    u8 subpic_id_len_minus1, start_code_emul_flag, pps_sps_subpic_id_flag, pps_id, sps_id;
    u16 subpic_id_bit_pos;
  };
} rror_box, *rror_boxPtr;

typedef struct
{
  u16 groupID, horizontal_offset, vertical_offset, region_width, region_height;
  u16 dependency_rect_region_count;
  u16 *dependencyRectRegionGroupID;
  u8 rect_region_flag, independent_idc, full_picture, filtering_disabled, has_dependency_list;
}trif_box, *trif_boxPtr;

typedef struct
{
  u32 groupID_info_4cc;
  u16 entry_count_minus1;
  u16 *groupID;
}sulm_box, *sulm_boxPtr;

/* whole loads of bitBuffer stuff for parsing parameter sets */

typedef struct {
	u8 *ptr;
	u32 length;
	u8 *cptr;
	u8 cbyte;
	u32 curbits;
	u32 bits_left;

	u8 prevent_emulation;	/* true or false */
	u8 emulation_position;	/* 0 usually, 1 after 1 zero byte, 2 after 2 zero bytes,
													3 after 00 00 03, and the 3 gets stripped */
} BitBuffer;

struct TrackGroup {
	u32 track;
	u32 track_group_id;
};

struct ParamStruct {
	u8 inputCount;
	char **inputs;
	char *output;

	double framerate;
	u32 frameduration;
	u32 seek;
	u8 trackGroupCount;
	struct TrackGroup **trackGroups;

	u32 subsample_information;
	u32 compactSampleToGroup;
};
#endif
