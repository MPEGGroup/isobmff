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

/* Created for Nokia FAVS project by Tampere University of Technology */

#include <math.h>
#include "ISOMovies.h"
#include "structures.h"

#define GET16(buffer) \
	((((u8*) buffer)[0] << 8) + ((u8*) buffer)[1])

#define GET32(buffer) \
	((((u8*)buffer)[0] << 24) + (((u8*)buffer)[1] << 16) + (((u8*)buffer)[2] << 8) + (((u8*)buffer)[3]))

#define PUT32(buffer, val ) \
	((u8*) buffer)[0] = (u8) (((val) >> 24) & 0xff); \
	((u8*) buffer)[1] = (u8) (((val) >> 16) & 0xff); \
	((u8*) buffer)[2] = (u8) (((val) >> 8) & 0xff); \
	((u8*) buffer)[3] = (u8) ((val)& 0xff); \

#define PUT16(buffer, val ) \
	((u8*) buffer)[0] = (u8) (((val) >> 8) & 0xff); \
	((u8*) buffer)[1] = (u8) ((val) & 0xff);

#define PUT32(buffer, val ) \
	((u8*) buffer)[0] = (u8) (((val) >> 24) & 0xff); \
	((u8*) buffer)[1] = (u8) (((val) >> 16) & 0xff); \
	((u8*) buffer)[2] = (u8) (((val) >> 8) & 0xff); \
	((u8*) buffer)[3] = (u8) ((val)& 0xff); \


static MP4Err BitBuffer_Init(BitBuffer *bb, u8 *p, u32 length) {
	int err = MP4NoErr;

	if (length > 0x0fffffff) {
		err = MP4BadParamErr;
		goto bail;
	}

	bb->ptr = (void*)p;
	bb->length = length;

	bb->cptr = (void*)p;
	bb->cbyte = *bb->cptr;
	bb->curbits = 8;

	bb->bits_left = length * 8;

	bb->prevent_emulation = 1;
	bb->emulation_position = (bb->cbyte == 0 ? 1 : 0);

bail:
	return err;
}

static u32 ceil_log2(u32 x) {
	u32 ret = 0;
	while (x>((u32)1 << ret)) ret++;
	return ret;
}


static u32 GetBits(BitBuffer *bb, u32 nBits, MP4Err *errout) {
	MP4Err err = MP4NoErr;
	int myBits;
	int myValue;
	int myResidualBits;
	int leftToRead;

	myValue = 0;
	if (nBits>bb->bits_left) {
		err = MP4EOF;
		goto bail;
	}

	if (bb->curbits <= 0) {
		bb->cbyte = *++bb->cptr;
		bb->curbits = 8;

		if (bb->prevent_emulation != 0) {
			if ((bb->emulation_position >= 2) && (bb->cbyte == 3)) {
				bb->cbyte = *++bb->cptr;
				bb->bits_left -= 8;
				bb->emulation_position = bb->cbyte ? 0:1;
				if (nBits>bb->bits_left) {
					err = MP4EOF;
					goto bail;
				}
			} else if (bb->cbyte == 0) bb->emulation_position += 1;
			else bb->emulation_position = 0;
		}
	}

	if (nBits > bb->curbits)
		myBits = bb->curbits;
	else
		myBits = nBits;

	myValue = (bb->cbyte >> (8 - myBits));
	myResidualBits = bb->curbits - myBits;
	leftToRead = nBits - myBits;
	bb->bits_left -= myBits;

	bb->curbits = myResidualBits;
	bb->cbyte = ((bb->cbyte) << myBits) & 0xff;

	if (leftToRead > 0) {
		u32 newBits;
		newBits = GetBits(bb, leftToRead, &err);
		myValue = (myValue << leftToRead) | newBits;
	}

bail:
	if (errout) *errout = err;
	return myValue;
}

static MP4Err GetBytes(BitBuffer *bb, u32 nBytes, u8 *p) {
	MP4Err err = MP4NoErr;
	unsigned int i;

	for (i = 0; i < nBytes; i++) {
		*p++ = (u8)GetBits(bb, 8, &err);
		if (err) break;
	}

	return err;
}

static u32 read_golomb_uev(BitBuffer *bb, MP4Err *errout) {
	MP4Err err = MP4NoErr;

	u32 power = 1;
	u32 value = 0;
	u32 leading = 0;
	u32 nbits = 0;

	leading = GetBits(bb, 1, &err);  if (err) goto bail;

	while (leading == 0) {
		power = power << 1;
		nbits++;
		leading = GetBits(bb, 1, &err);  if (err) goto bail;
	}

	if (nbits > 0) {
		value = GetBits(bb, nbits, &err); if (err) goto bail;
	}

bail:
	if (errout) *errout = err;
	return (power - 1 + value);
}


static MP4Err hevc_parse_sps_minimal(BitBuffer *bb, struct hevc_sps* sps) {
	MP4Err err = MP4NoErr;
	u32 i, ii;
	u8 x;
	u32 y;
	u8 sps_max_sub_layers;
	u8 sub_layer_profile_present_flag[8];
	u8 sub_layer_level_present_flag[8];

	/* Get first header byte for nal_unit_type */
	err = GetBytes(bb, 1, &x); if (err) goto bail;

	/* 33 == SPS */
	if ((x >> 1) != 33) err = MP4BadParamErr; if (err) goto bail;

	/* Skip second header byte */
	err = GetBytes(bb, 1, &x); if (err) goto bail;

	/* sps_video_parameter_set_id (4) + sps_max_sub_layers_minus1 (3) + sps_temporal_id_nesting_flag (1) */
	err = GetBytes(bb, 1, &x); if (err) goto bail;
	sps_max_sub_layers = ((x & 0xf) >> 1) + 1;

	/* profile_tier_level */
	/* general_profile_space (2) + general_tier_flag (1) + general_profile_idc (5) */
	err = GetBytes(bb, 1, &x); if (err) goto bail;

	/* general_profile_compatibility_flag[32] */
	for (i = 0; i < 4; i++) {
		err = GetBytes(bb, 1, &x); if (err) goto bail;
	}

	/* progressive_source + interlaced_source + non_packed_constraint +
	frame_only_constraint + general_reserved_zero_44bits[44] */
	for (i = 0; i < 6; i++) {
		err = GetBytes(bb, 1, &x); if (err) goto bail;
	}

	/* general_level_idc */
	err = GetBytes(bb, 1, &x); if (err) goto bail;

	/* sub_layer_profile_present_flag[i] + sub_layer_level_present_flag[i] */
	for (i = 0; i < sps_max_sub_layers - (u32)1; i++) {
		sub_layer_profile_present_flag[i] = GetBits(bb, 1, &err); if (err) goto bail;
		sub_layer_level_present_flag[i] = GetBits(bb, 1, &err); if (err) goto bail;
	}
	/* reserved_zero_2bits[ i ] */
	if (sps_max_sub_layers > 1) {
		x = GetBits(bb, 16 - (sps_max_sub_layers - 1)*2, &err); if (err) goto bail;
	}

	/* We are byte-aligned at this point */
	for (i = 0; i < sps_max_sub_layers - (u32)1; i++) {
		if (sub_layer_profile_present_flag[i]) {
			for (ii = 0; ii < 11; ii++) {
				err = GetBytes(bb, 1, &x); if (err) goto bail;
			}
		}
		if (sub_layer_level_present_flag[i]) {
			err = GetBytes(bb, 1, &x); if (err) goto bail;
		}
	}
	/* end profile_tier_level */

	/* sps_seq_parameter_set_id */
	y = read_golomb_uev(bb, &err); if (err) goto bail;
	/* chroma_format_idc */
	sps->chroma_format_idc = read_golomb_uev(bb, &err); if (err) goto bail;

	if (y == 3) {
		/* separate_colour_plane_flag */
		sps->separate_color_plane_flag = GetBits(bb, 1, &err); if (err) goto bail;
	}
	/* pic_width_in_luma_samples */
	sps->pic_width_in_luma_samples = read_golomb_uev(bb, &err); if (err) goto bail;
	
	/* pic_height_in_luma_samples */
	sps->pic_height_in_luma_samples = read_golomb_uev(bb, &err); if (err) goto bail;
	
	/* conformance_window_flag */
	x = GetBits(bb, 1, &err); if (err) goto bail;
	if (x) {
		/* conf_win_[left|right|top|bottom]_offset */
		for (i = 0; i < 4; i++) {
			x = read_golomb_uev(bb, &err); if (err) goto bail;
		}
	}

	/* bit_depth_luma_minus8 */
	y = read_golomb_uev(bb, &err); if (err) goto bail;

	/* bit_depth_chroma_minus8 */
	y = read_golomb_uev(bb, &err); if (err) goto bail;

	sps->log2_max_pic_order_cnt_lsb_minus4 = read_golomb_uev(bb, &err); if (err) goto bail;

	bail:
		return err;
}


static MP4Err hevc_parse_pps_minimal(BitBuffer *bb, struct hevc_pps* pps) {
	MP4Err err = MP4NoErr;
	u8 x;
	u32 y;

	/* Get first header byte for nal_unit_type */
	err = GetBytes(bb, 1, &x); if (err) goto bail;

	/* 34 == PPS */
 /*  if ((x >> 1) != 34) BAILWITHERROR(MP4BadParamErr); */

	/* Skip second header byte */
	err = GetBytes(bb, 1, &x); if (err) goto bail;

	/* pps_pic_parameter_set_id */
	y = read_golomb_uev(bb, &err); if (err) goto bail;

	/* pps_seq_parameter_set_id */
	y = read_golomb_uev(bb, &err); if (err) goto bail;

	/* dependent_slice_segments_enabled_flag */
	pps->dependent_slice_segments_enabled_flag = GetBits(bb, 1, &err); if (err) goto bail;

	/* output_flag_present_flag */
	pps->output_flag_present_flag = GetBits(bb, 1, &err); if (err) goto bail;

	/* num_extra_slice_header_bits */
	pps->num_extra_slice_header_bits = GetBits(bb, 3, &err); if (err) goto bail;

	/* sign_data_hiding_enabled_flag */
	GetBits(bb, 1, &err); if (err) goto bail;

	/* cabac_init_present_flag */
	GetBits(bb, 1, &err); if (err) goto bail;

	/* num_ref_idx_l0_default_active_minus1 */
	y = read_golomb_uev(bb, &err); if (err) goto bail;

	/* num_ref_idx_l1_default_active_minus1 */
	y = read_golomb_uev(bb, &err); if (err) goto bail;

	/* init_qp_minus26 */
	y = read_golomb_uev(bb, &err); if (err) goto bail;

	/* constrained_intra_pred_flag */
	GetBits(bb, 1, &err); if (err) goto bail;

	/* transform_skip_enabled_flag */
	GetBits(bb, 1, &err); if (err) goto bail;

	/* cu_qp_delta_enabled_flag */
	x = GetBits(bb, 1, &err); if (err) goto bail;
	if (x) {
		/* diff_cu_qp_delta_depth */
		y = read_golomb_uev(bb, &err); if (err) goto bail;
	}

	/* pps_cb_qp_offset */
	y = read_golomb_uev(bb, &err); if (err) goto bail;
	/* pps_cr_qp_offset */
	y = read_golomb_uev(bb, &err); if (err) goto bail;

	/* pps_slice_chroma_qp_offsets_present_flag */
	GetBits(bb, 1, &err); if (err) goto bail;
	/* weighted_pred_flag */
	GetBits(bb, 1, &err); if (err) goto bail;
	/* weighted_bipred_flag */
	GetBits(bb, 1, &err); if (err) goto bail;
	/* transquant_bypass_enabled_flag */
	GetBits(bb, 1, &err); if (err) goto bail;

	/* tiles_enabled_flag */
	pps->tiles_enabled_flag = GetBits(bb, 1, &err); if (err) goto bail;

	/* entropy_coding_sync_enabled_flag */
	GetBits(bb, 1, &err); if (err) goto bail;

	if (pps->tiles_enabled_flag) {
		/* num_tile_columns_minus1 */
		pps->num_tile_columns = read_golomb_uev(bb, &err) + 1; if (err) goto bail;

		/* num_tile_rows_minus1 */
		pps->num_tile_rows = read_golomb_uev(bb, &err) + 1; if (err) goto bail;

		/* uniform_spacing_flag */
		pps->tile_uniform_spacing_flag = GetBits(bb, 1, &err); if (err) goto bail;

		if (!pps->tile_uniform_spacing_flag) {
			s32 i;
			for (i = 0; i < pps->num_tile_columns - 1; i++) {
				pps->tile_column_width_minus1[i] = read_golomb_uev(bb, &err) + 1; if (err) goto bail;
			}

			for (i = 0; i < pps->num_tile_rows - 1; i++) {
				pps->tile_row_height_minus1[i] = read_golomb_uev(bb, &err) + 1; if (err) goto bail;
			}
			/* loop_filter_across_tiles_enabled_flag */
			GetBits(bb, 1, &err); if (err) goto bail;
		}
	}

	
bail:
	return err;
}




static MP4Err hevc_parse_slice_header_minimal(BitBuffer *bb, struct hevc_poc* poc, struct hevc_slice_header* header,
                                              struct hevc_sps* sps, struct hevc_pps* pps) {
	MP4Err err = MP4NoErr;
	u8 x = 0;
	u32 y = 0;
	u32 nal_type;

	/* Get first header byte for nal_unit_type */
	err = GetBytes(bb, 1, &x); if (err) goto bail;

	nal_type = x >> 1;
  /* if ((x >> 1) != 34) BAILWITHERROR(MP4BadParamErr); */
	header->nal_type = nal_type;
	/* Skip second NAL header byte */
	err = GetBytes(bb, 1, &x); if (err) goto bail;

	/* first_slice_segment_in_pic_flag */
	header->first_slice_segment_in_pic_flag = GetBits(bb, 1, &err); if (err) goto bail;

	/* if (nal_unit_type >= BLA_W_LP  &&  nal_unit_type <= RSV_IRAP_VCL23) */
	if (nal_type >= 16 && nal_type <= 23) {
		/* no_output_of_prior_pics_flag */
		GetBits(bb, 1, &err); if (err) goto bail;
	}

	/* slice_pic_parameter_set_id */
	y = read_golomb_uev(bb, &err); if (err) goto bail;

	if (!header->first_slice_segment_in_pic_flag) {
		u32 PicSizeInCtbsY = (u32)(ceil((float)sps->pic_width_in_luma_samples / 64.0f)*ceil((float)sps->pic_height_in_luma_samples / 64.0f));
		if (pps->dependent_slice_segments_enabled_flag) {
			header->dependent_slice = GetBits(bb, 1, &err); if (err) goto bail;
		}
		
		/* Ceil( Log2( PicSizeInCtbsY ) ) bits */
		header->slice_segment_address = GetBits(bb, ceil_log2(PicSizeInCtbsY), &err); if (err) goto bail;
	}
		
	if (!header->dependent_slice) {
		if (pps->num_extra_slice_header_bits) {
			GetBits(bb, pps->num_extra_slice_header_bits, &err); if (err) goto bail;
		}
		/* Slice Type */
		header->slice_type = read_golomb_uev(bb, &err); if (err) goto bail;
		if (pps->output_flag_present_flag) {
			/* pic_output_flag */
			GetBits(bb, 1, &err); if (err) goto bail;
		}
		if (sps->separate_color_plane_flag) {
			/* colour_plane_id (2) */
			GetBits(bb, 2, &err); if (err) goto bail;
		}
		if (header->first_slice_segment_in_pic_flag) {
			if (nal_type != 20) {
				s32 last_order_cnt_lsb = poc->order_cnt_lsb;
				s32 max_lsb = 1 << (sps->log2_max_pic_order_cnt_lsb_minus4 + 4);
				u8 const bits = sps->log2_max_pic_order_cnt_lsb_minus4 + 4;
				/* slice_pic_order_cnt_lsb */
				if (nal_type != 19) {
					poc->order_cnt_lsb = GetBits(bb, bits, &err); if (err) goto bail;
				} else {
					poc->order_cnt_lsb = 0;
					last_order_cnt_lsb = 0;
				}

				if (poc->order_cnt_lsb < last_order_cnt_lsb &&
					last_order_cnt_lsb - poc->order_cnt_lsb > (max_lsb / 2)) {
					poc->order_cnt_msb += max_lsb;
				} else if (poc->order_cnt_lsb > last_order_cnt_lsb &&
					poc->order_cnt_lsb - last_order_cnt_lsb > (max_lsb / 2)) {
					poc->order_cnt_msb -= max_lsb;
				}
				last_order_cnt_lsb = poc->order_cnt_lsb;

			} else {
				poc->order_cnt_lsb += 1;
			}
		}
	} 
	header->poc = poc->order_cnt_msb + poc->order_cnt_lsb;

bail:
	return err;
}

static u8* stripNALEmulation(u8* buffer, u32* bufferLen) {
	u8* outBuffer;
	u32 zerocount = 0;
	u32 emulationPreventionBytes = 0;
	u32 ii = 0;
	u32 i;
	for (i = 0; i < *bufferLen; i++) {
		/* Check for emulation prevention code */
		if (zerocount == 2    &&
				buffer[i] == 0x03 &&
				i+1 < *bufferLen   &&
				buffer[i+1] <= 0x04) {
			zerocount = buffer[i + 1]?0:1;
			emulationPreventionBytes++;
			i++;
		} else if (buffer[i] == 0) {
			zerocount++;
		} else {
			zerocount = 0;
		}
	}
	outBuffer = malloc(*bufferLen - emulationPreventionBytes);

	for (i = 0, ii = 0; i < *bufferLen; i++, ii++) {
		/* Check for emulation prevention code */
		if (zerocount == 2 &&
			buffer[i] == 0x03 &&
			i + 1 < *bufferLen &&
			buffer[i + 1] <= 0x04) {
			zerocount = buffer[i + 1] ? 0 : 1;
			i++;
		} else if (buffer[i] == 0) {
			zerocount++;
		} else {
			zerocount = 0;
		}
		outBuffer[ii] = buffer[i];
	}
	*bufferLen = *bufferLen - emulationPreventionBytes;
	
	return outBuffer;
}

static int parseHEVCNal(FILE* input,u8** data, int* data_len) {
	size_t startPos;
	size_t NALStart = 0;
	size_t NALEnd = 0;
	u32 NALULen;
	u8* NALU;
	
	u8 byte;
	int zerocount;
	int frame = 0;
	char nal_header[2];

	/* Save start position */
	startPos = ftell(input);
		
	zerocount = 0;
	while (1) {
		u8 byte;
		if (!fread(&byte, 1, 1, input)) {
			return -1;
		}
		/* Search for sync */
		if (zerocount >= 2 && byte == 1) {
			/* Read NAL unit header */
			fread(nal_header, 2, 1, input);
			/* Include header in the data */
			fseek(input, -2, SEEK_CUR);      
			break;
		} else if (byte == 0) {
			zerocount++;
		} else {
			zerocount = 0;
		}
	}
	NALStart = ftell(input);

	/* Search for next sync */
	zerocount = 0;
	while (1) {
		if (!fread(&byte, 1, 1, input)) {
			zerocount = 0;
			break;
		}
		/* Sync found */
		if (zerocount >= 2 && byte == 1) {
			fseek(input, -1 - zerocount, SEEK_CUR);
			NALEnd = ftell(input);
			break;
		} else if (byte == 0) {
			zerocount++;
		} else {
			zerocount = 0;
		}
	}
	NALEnd = ftell(input);

	NALULen = NALEnd - NALStart;
	NALU = malloc(NALULen);
	fseek(input, NALStart, SEEK_SET);
	if (!fread(NALU, NALULen, 1, input)) {
		return -1;
	}
	

	/* Extract NAL unit type */
	byte = nal_header[0] >> 1;
	*data_len = NALULen;
	*data = NALU;

	return byte;
}


static ISOErr analyze_hevc_stream(FILE* input, struct hevc_stream* stream) {
	ISOErr err = MP4NoErr;
	ISOHandle spsHandle = NULL;
	ISOHandle vpsHandle = NULL;
	ISOHandle ppsHandle = NULL;
	u8 frameNal = 0;
	u32 size_temp;
	u8* data = NULL;
	u32 datalen;
	BitBuffer bb;
	size_t startPos;
	struct hevc_poc poc;

	memset(&poc, 0, sizeof(struct hevc_poc));
	memset(stream, 0, sizeof(struct hevc_stream));
	startPos = ftell(input);

	stream->allocated_count = 16;
	stream->header = malloc(sizeof(struct hevc_slice_header *) * stream->allocated_count);

	/* Loop whole input file */
	while (1) {

		frameNal = 0;
		/* Parse NAL units until slice is found */
		while (!frameNal) {
			s32 naltype = parseHEVCNal(input, &data, &datalen);
			if (naltype == -1) break;
			switch (naltype) {
			case 32: /* VPS */
				ISODisposeHandle(vpsHandle);
				err = ISONewHandle(datalen, &vpsHandle);
				memcpy((*vpsHandle), data, datalen);
				free(data); data = NULL;
				break;
			case 33: /* SPS */
				ISODisposeHandle(spsHandle);
				err = ISONewHandle(datalen, &spsHandle);
				memcpy((*spsHandle), data, datalen);
				free(data); data = NULL;
				break;
			case 34: /* PPS */
				ISODisposeHandle(ppsHandle);
				err = ISONewHandle(datalen, &ppsHandle);
				memcpy((*ppsHandle), data, datalen);
				free(data); data = NULL;
				break;
			default:
				if (naltype < 32) {
					frameNal = 1;
				}
			}
		}

		/* Last frame was read */
		if (!frameNal) {
			break;
		}

			/* Parse slice header data */
		{
			struct hevc_slice_header *header = malloc(sizeof(struct hevc_slice_header));
			memset(header, 0, sizeof(struct hevc_slice_header));


			/* Parse info from the headers */
			/* ToDo: get from analyzed stream data */
			/* PPS */
			err = ISOGetHandleSize(ppsHandle, &size_temp); if (err) goto bail;
			err = BitBuffer_Init(&bb, (u8*)*ppsHandle, 8 * size_temp); if (err) goto bail;

			err = hevc_parse_pps_minimal(&bb, &stream->pps); if (err) goto bail;
			/* SPS */
			err = ISOGetHandleSize(spsHandle, &size_temp); if (err) goto bail;
			err = BitBuffer_Init(&bb, (u8*)*spsHandle, 8 * size_temp); if (err) goto bail;
			err = hevc_parse_sps_minimal(&bb, &stream->sps); if (err) goto bail;
			/* Slice header */
			err = BitBuffer_Init(&bb, data, 8 * datalen); if (err) goto bail;
			err = hevc_parse_slice_header_minimal(&bb, &poc, header, &stream->sps, &stream->pps); if (err) goto bail;

			/* Double the allocated space when depleted */
			if (stream->used_count == stream->allocated_count) {
				stream->allocated_count *= 2;
				struct hevc_slice_header **temp_header = malloc(stream->allocated_count * sizeof(struct hevc_slice_header *));
				memcpy(temp_header, stream->header, sizeof(struct hevc_slice_header *)*stream->used_count);
				free(stream->header);
				stream->header = temp_header;
			}
			stream->header[stream->used_count] = header;
			stream->used_count++;
		}
	}

	/* Release memory */
	ISODisposeHandle(ppsHandle);
	ISODisposeHandle(vpsHandle);
	ISODisposeHandle(spsHandle);

	/* Rewind back to where we were at the start */
	fseek(input, startPos, SEEK_SET);

bail:
	return err;
}

MP4_EXTERN(MP4Err) ISOAddGroupDescription(MP4Media media, u32 groupType, MP4Handle description, u32* index);
MP4_EXTERN(MP4Err) ISOMapSamplestoGroup(MP4Media media, u32 groupType, u32 group_index, s32 sample_index, u32 count);
MP4_EXTERN(MP4Err) ISONewHEVCSampleDescription(MP4Track theTrack,
	MP4Handle sampleDescriptionH,
	u32 dataReferenceIndex,
	u32 length_size, MP4Handle first_sps, MP4Handle first_pps, MP4Handle first_spsext);

static ISOErr addNaluSamples(FILE* input, ISOTrack trak, ISOMedia media, u8 trackID, u8 first_sample, struct ParamStruct *parameters, struct hevc_stream *stream) {
	ISOErr err;
	static struct hevc_slice_header header;
	static struct hevc_sps sps;
	static struct hevc_pps pps;
	static struct hevc_poc poc;
	u8 frameNal = 0;
	static u32 cumulativeOffset = 0;
	static u32 cumulativeSample = 0;
	ISOHandle sampleEntryH;
	ISOHandle sampleDataH;
	ISOHandle sampleDurationH;
	ISOHandle sampleSizeH;
	ISOHandle sampleOffsetH;
	ISOHandle syncSampleH = NULL;
	u8* data = NULL;
	static ISOHandle spsHandle;
	static ISOHandle vpsHandle;
	static ISOHandle ppsHandle;
	u32 naltype;

	u32 datalen;
	
	/* On first sample */
	if (first_sample) {
		u32 i = 0;
		u32 syncLen = 0;
		memset(&poc, 0, sizeof(struct hevc_poc));
		memset(&header, 0, sizeof(struct hevc_slice_header));
		memset(&pps, 0, sizeof(struct hevc_pps));
		memset(&sps, 0, sizeof(struct hevc_sps));
		cumulativeSample = 0;
		err = ISONewHandle(sizeof(u32) * 2, &syncSampleH);
		((u32*)*syncSampleH)[0] = 1;
		((u32*)*syncSampleH)[1] = 1;
	}

	err = ISOSetMediaLanguage(media, "und"); /* undetermined */
	if (err) return err;
	err = ISONewHandle(1, &sampleEntryH);

	/* Parse NAL units until slice is found */
	while (!frameNal) {
		naltype = parseHEVCNal(input, &data, &datalen);
		switch (naltype) {
		case 32: /* VPS */
			ISODisposeHandle(vpsHandle);
			err = ISONewHandle(datalen, &vpsHandle);
			memcpy((*vpsHandle), data, datalen);
			free(data); data = NULL;
			break;
		case 33: /* SPS */
			ISODisposeHandle(spsHandle);
			err = ISONewHandle(datalen, &spsHandle);
			memcpy((*spsHandle), data, datalen);
			free(data); data = NULL;
			break;
		case 34: /* PPS */
			ISODisposeHandle(ppsHandle);
			err = ISONewHandle(datalen, &ppsHandle);
			memcpy((*ppsHandle), data, datalen);
			free(data); data = NULL;
			break;
		default:
			if (naltype < 32) {
				frameNal = 1;
			}
		}
	}

	/* Get the header info from analysis results */
	header = *stream->header[cumulativeSample];

	ISONewHEVCSampleDescription(trak, sampleEntryH, 1, 1, spsHandle, ppsHandle, vpsHandle);
	err = ISONewHandle(sizeof(u32), &sampleDurationH);
	*((u32*)*sampleDurationH) = parameters->frameduration;
	err = ISONewHandle(datalen+4, &sampleDataH);
	
	err = ISONewHandle(sizeof(u32), &sampleSizeH);
	err = ISONewHandle(sizeof(u32), &sampleOffsetH);


	memcpy((*sampleDataH)+4, data, datalen);
	/* NAL units are prefixed with 4-byte length field */
	(*sampleDataH)[0] = (datalen >> 24) & 0xff;
	(*sampleDataH)[1] = (datalen >> 16) & 0xff;
	(*sampleDataH)[2] = (datalen >> 8) & 0xff;
	(*sampleDataH)[3] = (datalen) & 0xff;

	*(u32*)*sampleOffsetH = ( ( (stream->header[cumulativeSample]->poc + stream->header[cumulativeSample]->poc_offset) -
		                          (cumulativeSample)+3) ) * parameters->frameduration;

	cumulativeOffset += datalen;
	
	err = ISOGetHandleSize(sampleDataH,(u32*)*sampleSizeH);
	err = MP4AddMediaSamples(media, sampleDataH, 1,
                            sampleDurationH,
                            sampleSizeH,
                            (first_sample ? sampleEntryH : NULL), sampleOffsetH, syncSampleH);
	if (sampleEntryH) {
		err = ISODisposeHandle(sampleEntryH);
		sampleEntryH = NULL;
	}
	cumulativeSample++;
	if (syncSampleH) err = ISODisposeHandle(syncSampleH);

	err = ISODisposeHandle(sampleDataH);
	err = ISODisposeHandle(sampleSizeH);
	err = ISODisposeHandle(sampleDurationH);
	free(data);
	return err;
}

MP4_EXTERN(MP4Err) MP4AddAtomToTrack(MP4Track theTrack, MP4GenericAtom the_atom);
MP4_EXTERN(MP4Err) MP4AddTrackGroup(MP4Track theTrack, u32 groupID, u32 dependencyType);
ISO_EXTERN(ISOErr)
ISOSetMovieBrand(ISOMovie theMovie, u32 brand, u32 minorversion);

ISOErr createMyMovie(struct ParamStruct *parameters) {
	ISOErr err;
	ISOMovie moov;
	u32 frameCounter = 0;
	u32 trackID = 0;
	u32 trackGroup = 0;
	ISOTrack trak;
	ISOMedia media;
	u32 rap_desc_index;
	u32 alst_desc_index;
	MP4Handle rap_desc;  
	MP4Handle alst_desc = NULL;
	u32 initialObjectDescriptorID;
	u8 OD_profileAndLevel;
	u8 scene_profileAndLevel;
	u8 audio_profileAndLevel;
	u8 visual_profileAndLevel;
	u8 graphics_profileAndLevel;
	u64 mediaDuration;
	char *filename = parameters->output ? parameters->output : "mov.mp4";
	err = ISONoErr;
	initialObjectDescriptorID = 1;
	OD_profileAndLevel        = 0xff; /* none required */
	scene_profileAndLevel     = 0xff; /* none required */
	audio_profileAndLevel     = 0xff; /* none required */
	visual_profileAndLevel    = 0xff; /* none required */
	graphics_profileAndLevel  = 0xff; /* none required */
	err = MP4NewMovie(&moov,
										initialObjectDescriptorID,
										OD_profileAndLevel,
										scene_profileAndLevel,
										audio_profileAndLevel,
										visual_profileAndLevel,
										graphics_profileAndLevel);  

	MP4NewHandle(1, &rap_desc); /* Allocate one byte for rap */

	/* Set correct brand for the new features */
	ISOSetMovieBrand(moov, MP4_FOUR_CHAR_CODE('i', 's', 'o', '6'), 0);

	for (trackID = 1; trackID < (u32)parameters->inputCount + 1; trackID++) {
	
		FILE *input = fopen(parameters->inputs[trackID - 1], "rb");
		struct hevc_stream stream;
		u32 i;
		s32 poc_offset = 0;
		s32 largest_poc = 0;
		if (!input) continue;

		err = analyze_hevc_stream(input, &stream); if (err) goto bail;
		printf("Found: %d slices\r\n", stream.used_count);

		/* Analyze POC numbers for discontinuety */
		for (frameCounter = 1; frameCounter < stream.used_count; frameCounter++) {
			if (stream.header[frameCounter]->poc == 0) {
				u32 frameCounterPoc = frameCounter+1;
				s32 smallest = 0;
				for (; frameCounterPoc < stream.used_count; frameCounterPoc++) {
					if (stream.header[frameCounterPoc]->poc < smallest) smallest = stream.header[frameCounterPoc]->poc;
					if (stream.header[frameCounterPoc]->poc > stream.header[frameCounter]->poc) break;
				}
				poc_offset = -(smallest - 1) + largest_poc;
			}
			stream.header[frameCounter]->poc_offset = poc_offset;
			if (stream.header[frameCounter]->poc + stream.header[frameCounter]->poc_offset > largest_poc) {
				largest_poc = stream.header[frameCounter]->poc + stream.header[frameCounter]->poc_offset;
			}
		}

		/* Create a new trak to the movie */
		err = ISONewMovieTrackWithID(moov, ISONewTrackIsVisual, trackID, &trak); if (err) goto bail;

		err = ISONewTrackMedia(trak, &media, ISOVisualHandlerType, 30000, NULL); if (err) goto bail;

		err = ISOBeginMediaEdits(media); if (err) goto bail;

		/* Add each slice as a new sample */
		for (frameCounter = 0; frameCounter < stream.used_count; frameCounter++) {
			err = addNaluSamples(input, trak, media, trackID, frameCounter == 0, parameters, &stream); if (err) goto bail;
		}

		err = ISOGetMediaDuration(media, &mediaDuration); if (err) goto bail;
		err = ISOEndMediaEdits(media);  if (err) goto bail;/* Calculate duration */

		/* Create "rap " sample description and group */
		ISOAddGroupDescription(media, MP4_FOUR_CHAR_CODE('r', 'a', 'p', ' '), rap_desc, &rap_desc_index);
		for (frameCounter = 1; frameCounter < stream.used_count; frameCounter++) {
			/* Mark RAP frames (CRA/BLA/IDR/IRAP) to the group */
			if (stream.header[frameCounter]->nal_type >= 16 && stream.header[frameCounter]->nal_type  <= 23) {
				ISOMapSamplestoGroup(media, MP4_FOUR_CHAR_CODE('r', 'a', 'p', ' '), rap_desc_index, frameCounter, 1);
			}
		}

		/* Create an alternative startup sequence */
		{
			u32 start_slice = 0;
			u32 leading_pic_count = 0;      
			/* Map RASL picture count from the headers, search second RAP */
			for (frameCounter = 1; frameCounter < stream.used_count; frameCounter++) {
				if (stream.header[frameCounter]->nal_type >= 16 && stream.header[frameCounter]->nal_type <= 23) {
						start_slice = frameCounter;
						break;
				}
			}
			frameCounter++;
			/* Count RASL slices following */
			for (; frameCounter < stream.used_count; frameCounter++) {
				if (stream.header[frameCounter]->nal_type == 8 || stream.header[frameCounter]->nal_type == 9) {
					leading_pic_count++;
				} else if (stream.header[frameCounter]->nal_type == 6 || stream.header[frameCounter]->nal_type == 7) {
					leading_pic_count++;
				} else {
					break;
				}       
			}
			/* If RASL slices are present, we can use alternative startup sequences */
			if (leading_pic_count) {
				alst_dataptr alst;
				MP4Handle alst_temp;
				/* Define alst parameters */
				MP4NewHandle(2 + 2 + 4 * 2, &alst_temp);
				alst = (alst_dataptr)*alst_temp;
				alst->roll_count = 2; alst->first_output_sample = 1;
				alst->sample_offset[0] = leading_pic_count * parameters->frameduration; alst->sample_offset[1] = 0;
				/* Write alst parameters to the description using correct endianness */
				MP4NewHandle(2 + 2 + 4 * 2, &alst_desc);				
				PUT16(&(*alst_desc)[0], alst->roll_count); /* We need the picture before and after RASL pictures */
				PUT16(&(*alst_desc)[2], alst->first_output_sample); /* Output the first picture */
				PUT32(&(*alst_desc)[4], alst->sample_offset[0]); /* Push decoding time forward */
				PUT32(&(*alst_desc)[8], alst->sample_offset[1]);
				ISOAddGroupDescription(media, MP4_FOUR_CHAR_CODE('a', 'l', 's', 't'), alst_desc, &alst_desc_index);

				for (frameCounter = start_slice; frameCounter < stream.used_count-1; frameCounter++) {
					/* Mark RAP before RASL pics and one picture after them to this group */
					/* This enabled alternative startup sequence to use only the RAP and the picture after RASL in decoding */
					/* and skip all the RASL pictures */
					if (((stream.header[frameCounter]->nal_type < 6 || stream.header[frameCounter]->nal_type > 9) &&
							(stream.header[frameCounter + 1]->nal_type >= 6 && stream.header[frameCounter + 1]->nal_type <= 9))
							||
							((stream.header[frameCounter - 1]->nal_type >= 6 && stream.header[frameCounter - 1]->nal_type <= 9) &&
							(stream.header[frameCounter]->nal_type < 6 || stream.header[frameCounter]->nal_type > 9))
							) {
						ISOMapSamplestoGroup(media, MP4_FOUR_CHAR_CODE('a', 'l', 's', 't'), alst_desc_index, frameCounter, 1);
					}
				}
				MP4DisposeHandle(alst_temp);
			}
		}

		err = ISOInsertMediaIntoTrack(trak, 0, 0, mediaDuration, 1); if (err) goto bail;
		
		for (i = 0; i < stream.used_count; i++) {
			free(stream.header[i]); stream.header[i] = NULL;
		}
		free(stream.header);
		stream.header = NULL;
	}

	/* Add each track group to the traks */
	for (trackGroup = 0; trackGroup < (u32)parameters->trackGroupCount; trackGroup++) {
		err = ISOGetMovieIndTrack(moov, parameters->trackGroups[trackGroup]->track, &trak); if (err) goto bail;
		err =  MP4AddTrackGroup(trak, parameters->trackGroups[trackGroup]->track_group_id, MP4_FOUR_CHAR_CODE('m', 's', 'r', 'c'));
		if (err) goto bail;
	}


	err = ISOWriteMovieToFile(moov, filename); if (err) goto bail;
	if (alst_desc) ISODisposeHandle(alst_desc);
	ISODisposeHandle(rap_desc);
	err = ISODisposeMovie(moov); if (err) goto bail;
bail:
	return err;
}

MP4_EXTERN(MP4Err) ISOGetHEVCSampleDescriptionPS(MP4Handle sampleEntryH, MP4Handle ps, u32 where, u32 index);
MP4_EXTERN(MP4Err) ISOGetGroupDescription(MP4Media media, u32 groupType, u32 index, MP4Handle description);
MP4_EXTERN(MP4Err) MP4MediaTimeToSampleNum(MP4Media theMedia, u64 mediaTime, u32 *outSampleNum, 
                                           u64 *outSampleCTS, u64 *outSampleDTS, s32 *outSampleDuration);

ISOErr playMyMovie(struct ParamStruct *parameters, char *filename) {

	s32 i;
	u32 trackNumber;
	ISOErr err;
	u32 handlerType;
	ISOMovie moov;
	ISOTrack trak;
	u32 trackCount = 0;
	u32 trackGroups = 0;
	u32 trackGroupID = 0;
	u32 mediaTimeScale = 0;
	u32 totalSamples = 0;
	u64 duration = 0;
	ISOMedia media;
	ISOTrackReader reader;
	ISOHandle sampleH;
	ISOHandle decoderConfigH;

	ISOHandle alst_desc;
	ISOHandle alst_struct = NULL;
	alst_dataptr alst = NULL;
	ISOHandle alst_index;
	ISOHandle rap_index;
	s32 alst_start = -1;

	const char syncCode[] = "\0\0\0\1";
	char *outSampleName = malloc(128);
	ISOHandle sampleEntryH;
	err = ISONewHandle(1, &sampleEntryH); if (err) goto bail;
	err = ISONewHandle(1, &alst_desc); if (err) goto bail;

	err = ISOOpenMovieFile(&moov, filename, MP4OpenMovieNormal); if (err) goto bail;
	err = MP4GetMovieTrackCount(moov, &trackCount); if (err) goto bail;

	printf("trackCount: %d\r\n", trackCount);

	/* Loop all the tracks in the container, starting from 1 */
	for (trackNumber = 1; trackNumber < trackCount + 1; trackNumber++) {
		FILE *out;
		ISOHandle spsHandle = NULL;
		ISOHandle vpsHandle = NULL;
		ISOHandle ppsHandle = NULL;
		u32 sampleSize;
		u32 alst_target = 0;
		u32 pics_rasl_skipped = 0;
		u32 pics_start_skipped = 0;
		u32 pics_written = 0;

		sprintf(outSampleName, "out_track_%d.265", trackNumber);
		out = fopen(outSampleName, "wb");
		printf("Track ID %d\r\n", trackNumber);
		err = ISOGetMovieIndTrack(moov, trackNumber, &trak);

		/* Check if track group box exists and print the group if found */
		MP4_EXTERN(MP4Err) MP4GetTrackGroup(MP4Track theTrack, u32 groupType, u32 *outGroupId);
		if (MP4GetTrackGroup(trak, MP4_FOUR_CHAR_CODE('m', 's', 'r', 'c'), &trackGroupID) == MP4NoErr) {
			printf("Found trackGroup (Group ID: %d)\r\n", trackGroupID);
		}

		
		err = ISOGetTrackMedia(trak, &media); if (err) goto bail;
		err = ISOGetMediaHandlerDescription(media, &handlerType, NULL); if (err) goto bail;
		err = ISONewHandle(0, &decoderConfigH); if (err) goto bail;
		err = ISOCreateTrackReader(trak, &reader); if (err) goto bail;
		err = ISONewHandle(0, &sampleH); if (err) goto bail;

		/* Handle alternative startup sequence, 'rap ' is also required */
		if (ISOGetGroupDescription(media, MP4_FOUR_CHAR_CODE('r', 'a', 'p', ' '), 1, alst_desc) == MP4NoErr) {
			if (ISOGetGroupDescription(media, MP4_FOUR_CHAR_CODE('a', 'l', 's', 't'), 1, alst_desc) == MP4NoErr) {
				u32 roll_count;
				err = ISOGetHandleSize(alst_desc, &sampleSize); if (err) goto bail;
				err = ISONewHandle(sampleSize, &alst_struct); if (err) goto bail;
				alst = (alst_dataptr)*alst_struct;
				alst->roll_count = GET16(&(*alst_desc)[0]);
				alst->first_output_sample = GET16(&(*alst_desc)[2]);
				for (roll_count = 0; roll_count < alst->roll_count; roll_count++) {
					alst->sample_offset[roll_count] = GET32(&(*alst_desc)[4 + roll_count * 4]);
				}
			}
		}

		/* Get sample description from the trak */
		err = MP4TrackReaderGetCurrentSampleDescription(reader, sampleEntryH); if (err) goto bail;
		/* Allocate handles for parameter sets */
		err = ISONewHandle(1, &vpsHandle); err = ISONewHandle(1, &spsHandle); err = ISONewHandle(1, &ppsHandle);
		if (err) goto bail;
		/* Grab parameter sets from the sample description */
		err = ISOGetHEVCSampleDescriptionPS(sampleEntryH, vpsHandle, 32, 1); if (err) goto bail;
		err = ISOGetHEVCSampleDescriptionPS(sampleEntryH, spsHandle, 33, 1); if (err) goto bail;
		err = ISOGetHEVCSampleDescriptionPS(sampleEntryH, ppsHandle, 34, 1); if (err) goto bail;

		ISOGetMediaTimeScale(media, &mediaTimeScale);
		ISOGetMediaSampleCount(media, &totalSamples);    
		ISOGetMediaDuration(media, &duration);

		printf("numberofSamples %u, duration: %u\r\n", totalSamples, duration);

		err = ISONewHandle(totalSamples*sizeof(u32), &alst_index); if (err) goto bail;
		err = ISONewHandle(totalSamples*sizeof(u32), &rap_index); if (err) goto bail;

		/* Extract alternative startup sequences related groupings */
		if (alst) {
			u32 group_index = 0;
			u32 i;
			printf("Alternative startup sequences found!\r\n");
			for (i = 0; i < totalSamples; i++) {
				ISOGetSampletoGroupMap(media, MP4_FOUR_CHAR_CODE('r', 'a', 'p', ' '), i + 1, &((u32*)*rap_index)[i]);
				ISOGetSampletoGroupMap(media, MP4_FOUR_CHAR_CODE('a', 'l', 's', 't'), i + 1, &((u32*)*alst_index)[i]);
			}
		}

		/* Write parameter sets in the beginning of the trak output file */
		err = ISOGetHandleSize(vpsHandle, &sampleSize); if (err) goto bail;
		fwrite(&syncCode, 4, 1, out); fwrite(*vpsHandle, sampleSize, 1, out);

		err = ISOGetHandleSize(spsHandle, &sampleSize); if (err) goto bail;
		fwrite(&syncCode, 4, 1, out); fwrite(*spsHandle, sampleSize, 1, out);

		err = ISOGetHandleSize(ppsHandle, &sampleSize); if (err) goto bail;
		fwrite(&syncCode, 4, 1, out); fwrite(*ppsHandle, sampleSize, 1, out);
		
		/* Handle the case when seek parameter is given and alst is present */
		if (parameters->seek && alst) {
			/* scale given time in ms to mediatimescale */
			u64 mediatime = (parameters->seek * mediaTimeScale) / 1000, sampleCTS, sampleDTS;
			u32 sampleDuration;
			s32 i;
			
			err = MP4MediaTimeToSampleNum(media, mediatime, &alst_target, &sampleCTS, &sampleDTS, &sampleDuration); if (err) goto bail;
			
			for (i = alst_target - 1; i >= 0; i--) {
				if (((u32*)*rap_index)[i] && ((u32*)*alst_index)[i]) {
					alst_start = i + 1;
					break;
				}
			}
			printf("RAP: %d\n", alst_start);
		}

		for (i = 1;;i++) { /* play every frame */
			u32 unitSize;
			s32 cts;
			s32 dts;
			u32 sampleFlags;
			static u32 roll_count = 0;
			
			err = MP4TrackReaderGetNextAccessUnit(reader, sampleH,
				&unitSize, &sampleFlags,
				&cts, &dts);
			if (err) {
				if (err == ISOEOF)
					err = ISONoErr;
				break;
			}
			
			/* ALST needs special handling, if alst_start if the default -1, this should write everything to a file */
			if (i >= alst_start) {
				if (alst_start == -1 || roll_count == alst->roll_count || ((u32*)*alst_index)[i - 1]) {
					/* ToDo: only RADL slices have short start code */
					fwrite(&syncCode[0], 4, 1, out);
					fwrite(*sampleH + 4, unitSize - 4, 1, out);
					
					pics_written++;
					
					if (alst && roll_count < alst->roll_count) roll_count++;
				} else {
					pics_rasl_skipped++;
				}
			} else {
				pics_start_skipped++;
			}
		}
		if (pics_rasl_skipped) printf("Skipped %d RASL and %d pictures from the start, written %d pictures, %d pictures to decode until seek position\n", pics_rasl_skipped, pics_start_skipped, pics_written, alst_target - pics_start_skipped - pics_rasl_skipped);
		fclose(out);
		err = ISODisposeHandle(rap_index);
		if (alst_struct) err = ISODisposeHandle(alst_struct);
		err = ISODisposeHandle(alst_index);
		err = ISODisposeHandle(alst_desc);
		err = ISODisposeHandle(sampleH);
		err = ISODisposeHandle(decoderConfigH);
		err = ISODisposeTrackReader(reader);
	}
	free(outSampleName);
	err = ISODisposeMovie(moov);
bail:
	return err;
}

int parseInput(int argc, char* argv[], struct ParamStruct *parameters) {
	u32 param;
	for (param = 1; param < (u32)argc; param++) {
		if (argv[param][0] == '-') {
			switch (argv[param][1]) {

			/* New input track */
			case 'i': {
					char **tempInputs = parameters->inputs;
					char *tempInput;
					u32 inputLen = 0;
					u32 inputCopy = 0;

					/* if next parameter is not present, abort this operation */
					if (argc - 1 == param) break;

					/* Allocate new pointer array and copy the old if present */
					parameters->inputs = malloc(sizeof(char*)*(parameters->inputCount+1));
					if (tempInputs) {
						for (inputCopy = 0; inputCopy < parameters->inputCount; inputCopy++) {
							parameters->inputs[inputCopy] = tempInputs[inputCopy];
						}
					}
					/* Allocate new char array for the name and copy */
					inputLen = strlen(argv[param + 1]);
					tempInput = malloc(inputLen + 1);
					memcpy(tempInput, argv[param + 1], inputLen + 1);

					/* Set new input track */
					parameters->inputs[parameters->inputCount] = tempInput;
					parameters->inputCount++;

					if (tempInputs) free(tempInputs);
					/* Skip next parameter since if was used as the input name */
					param++;
				}
				break;
			/* Output file */
			case 'o': {
					u32 outputLen;
					/* if next parameter is not present, abort this operation */
					if (argc - 1 == param) break;

					/* Allocate memory and copy the file name */
					outputLen = strlen(argv[param + 1]);
					parameters->output = malloc(outputLen+1);
					memcpy(parameters->output, argv[param + 1], outputLen + 1);
				}
				break;
			/* Framerate */
			case 'f': {        
				/* if next parameter is not present, abort this operation */
				if (argc - 1 == param) break;
				parameters->framerate = atof(argv[param + 1]);
			}
			/* Seek */
			case 's': {
				/* if next parameter is not present, abort this operation */
				if (argc - 1 == param) break;
				parameters->seek = atoi(argv[param + 1]);
			}
				break;
			/* Track groups */
			case 'g': {
				u32 outputLen;
				u32 i;
				u32 found = 0;
				u32 groupCopy;
				struct TrackGroup *newTrackGroup;

				/* Store original trackGroup list pointer */
				struct TrackGroup **tempGroups = parameters->trackGroups;

				/* if next parameter is not present, abort this operation */
				if (argc - 1 == param) break;

				/* Allocate memory and copy the file name */
				outputLen = strlen(argv[param + 1]);
				/* Search for delimiter ':' */
				for (i = 0; i < outputLen; i++) {
					if (argv[param + 1][i] == ':' && i != 0 && i != outputLen - 1) {
						/* Store current position */
						found = i;
						/* Set to null to separate the two ID's */
						argv[param + 1][i] = 0;
						break;
					}
				}
				if (!found) break;

				/* Grab tracks from the parameter <TrackID>:<GroupID> */
				newTrackGroup = malloc(sizeof(struct TrackGroup));
				newTrackGroup->track = atoi(&argv[param + 1][0]);
				newTrackGroup->track_group_id = atoi(&argv[param + 1][found+1]);

				/* Allocate more space for the trackGroup structs and copy pointers from the original */
				parameters->trackGroups = malloc(sizeof(struct TrackGroup*)*(parameters->trackGroupCount + 1)); 
				if (tempGroups) {
					for (groupCopy = 0; groupCopy < parameters->trackGroupCount; groupCopy++) {
						parameters->trackGroups[groupCopy] = tempGroups[groupCopy];
					}
				}
				/* Insert the new trackGroup */
				parameters->trackGroups[parameters->trackGroupCount] = newTrackGroup;
				parameters->trackGroupCount++;
				param++;

				/* Free original list data */
				if (tempGroups) free(tempGroups);
			}
				break;
			/* Long parameter name */
			case '-': {

				}
				break;
			}
		}

	}
	return 1;
}


int cleanParameters(struct ParamStruct *parameters) {
	u32 i;
	if (parameters->inputCount) {
		for (i = 0; i < parameters->inputCount; i++) {
			free(parameters->inputs[i]);
		}
		free(parameters->inputs);
	}

	free(parameters->output);

	if (parameters->trackGroupCount) {
		for (i = 0; i < parameters->trackGroupCount; i++) {
			free(parameters->trackGroups[i]);
		}
		free(parameters->trackGroups);
	}
	return 1;
}



int main(int argc, char* argv[])
{
	struct ParamStruct parameters;
	memset(&parameters, 0, sizeof(struct ParamStruct));
	parameters.framerate = 30.0;
	parseInput(argc, argv, &parameters);
	parameters.frameduration = (u32)((30000.0 / parameters.framerate)+0.5);

	/* We need inputs */
	if (!parameters.inputCount) {
		fprintf(stderr, "Usage: program -i <inputFile> -g <TrackID>:<GroupID> -o <outputFile> -f <framerate(float)>\r\n");
		fprintf(stderr, "            -i <filename>: Input file, can be multiple\r\n");
		fprintf(stderr, "            -g <TrackID>:<GroupID> :Add track group, can be multiple\r\n");
		fprintf(stderr, "            -o <filename> Output file (MP4)\r\n");
		fprintf(stderr, "            -f <framerate> set framerate, default 30\r\n");
		fprintf(stderr, "            -s <frame> seek to position on milliseconds\r\n");
		exit(1);
	}

	printf("Using framerate: %.2f\r\n", parameters.framerate);

	createMyMovie(&parameters);
	
	playMyMovie(&parameters, parameters.output ? parameters.output : "mov.mp4");

	cleanParameters(&parameters);

	return 1;
}