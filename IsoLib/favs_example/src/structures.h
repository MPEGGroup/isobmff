#ifndef INCLUDED_FAVS_STRUCTURES_H
#define INCLUDED_FAVS_STRUCTURES_H

typedef struct {
	u16 roll_count;
	u16 first_output_sample;
	u32 sample_offset[];
} alst_data, *alst_dataptr;

struct hevc_slice_header {
	s32 poc;
	s32 poc_offset;
	u32 slice_segment_address;
	u8 dependent_slice;
	u8 slice_type;
	u8 nal_type;
	u8 first_slice_segment_in_pic_flag;
};

struct hevc_poc {
	s32 order_cnt_msb;
	s32 order_cnt_lsb;
};


struct hevc_sps {
	u8 separate_color_plane_flag;
	u8 chroma_format_idc;
	u32 pic_width_in_luma_samples;
	u32 pic_height_in_luma_samples;
	u8 log2_max_pic_order_cnt_lsb_minus4;

};

struct hevc_pps {
	u8 dependent_slice_segments_enabled_flag;
	u8 num_extra_slice_header_bits;
	u8 output_flag_present_flag;
	u8 tiles_enabled_flag;
	u8 num_tile_columns;
	u8 num_tile_rows;
	u8 tile_uniform_spacing_flag;
	u16 tile_column_width_minus1[16];
	u16 tile_row_height_minus1[16];
};

struct hevc_stream {
	u32 used_count;
	u32 allocated_count;
	struct hevc_slice_header **header;
	struct hevc_sps sps;
	struct hevc_pps pps;
};
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
};


#endif
