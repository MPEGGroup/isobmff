#include <math.h>
#include <assert.h>
#include <string.h>
#include "ISOMovies.h"
#include "structures.h"
#include "hevc.h"
#include "tools.h"




#define scaling_list_data() 			u32 sizeId, matrixId; \
for (sizeId = 0; sizeId < 4; sizeId++) { \
	for (matrixId = 0; matrixId < ((sizeId == 3) ? 2 : 6); matrixId++) { \
		/* scaling_list_pred_mode_flag[sizeId][matrixId] */ \
		x = GetBits(bb, 1, &err); if (err) goto bail; \
		if (!x) { \
			/* scaling_list_pred_matrix_id_delta[sizeId][matrixId] */ \
			y = read_golomb_uev(bb, &err); if (err) goto bail; \
						} else { \
			u32 coefNum = 64 < (1 << (4 + (sizeId << 1))) ? 64 : (1 << (4 + (sizeId << 1))); \
			if (sizeId > 1) { \
				/* scaling_list_dc_coef_minus8[sizeId - 2][matrixId] */ \
				y = read_golomb_uev(bb, &err); if (err) goto bail; \
									} \
			for (i = 0; i < coefNum; i++) { \
				/* scaling_list_delta_coef */ \
				y = read_golomb_uev(bb, &err); if (err) goto bail; \
									} \
						} \
			} \
}


MP4Err hevc_parse_sps_minimal(BitBuffer *bb, struct hevc_sps* sps) {
	MP4Err err = MP4NoErr;
	u32 i, ii, j;
	u8 x;
	u32 y;
	u8 sub_layer_profile_present_flag[8];
	u8 sub_layer_level_present_flag[8];
	u8 stRpsIdx;

	/* Get first header byte for nal_unit_type */
	err = GetBytes(bb, 1, &x); if (err) goto bail;

	/* 33 == SPS */
	if ((x >> 1) != 33) err = MP4BadParamErr; if (err) goto bail;

	/* Skip second header byte */
	err = GetBytes(bb, 1, &x); if (err) goto bail;

	/* sps_video_parameter_set_id (4) + sps_max_sub_layers_minus1 (3) + sps_temporal_id_nesting_flag (1) */
	err = GetBytes(bb, 1, &x); if (err) goto bail;
	sps->sps_max_sub_layers_minus1 = ((x & 0xf) >> 1);

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
	for (i = 0; i < sps->sps_max_sub_layers_minus1; i++) {
		sub_layer_profile_present_flag[i] = GetBits(bb, 1, &err); if (err) goto bail;
		sub_layer_level_present_flag[i] = GetBits(bb, 1, &err); if (err) goto bail;
	}
	/* reserved_zero_2bits[ i ] */
	if (sps->sps_max_sub_layers_minus1 > 0) {
		x = GetBits(bb, 16 - sps->sps_max_sub_layers_minus1 * 2, &err); if (err) goto bail;
	}

	/* We are byte-aligned at this point */
	for (i = 0; i < sps->sps_max_sub_layers_minus1; i++) {
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
	//config->chromaFormat = y;
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

	/* sps_sub_layer_ordering_info_present_flag */
	x = GetBits(bb, 1, &err); if (err) goto bail;
	for (i = (x ? 0 : sps->sps_max_sub_layers_minus1); i <= sps->sps_max_sub_layers_minus1; i++) {
		/* sps_max_dec_pic_buffering_minus1[i] */
		y = read_golomb_uev(bb, &err); if (err) goto bail;
		/* sps_max_num_reorder_pics[i] */
		y = read_golomb_uev(bb, &err); if (err) goto bail;
		/* sps_max_latency_increase_plus1[i] */
		y = read_golomb_uev(bb, &err); if (err) goto bail;
	}
	/* log2_min_luma_coding_block_size_minus3 */
	y = read_golomb_uev(bb, &err); if (err) goto bail;
	/* log2_diff_max_min_luma_coding_block_size */
	y = read_golomb_uev(bb, &err); if (err) goto bail;
	/* log2_min_transform_block_size_minus2 */
	y = read_golomb_uev(bb, &err); if (err) goto bail;
	/* log2_diff_max_min_transform_block_size */
	y = read_golomb_uev(bb, &err); if (err) goto bail;
	/* max_transform_hierarchy_depth_inter */
	y = read_golomb_uev(bb, &err); if (err) goto bail;
	/* max_transform_hierarchy_depth_intra */
	y = read_golomb_uev(bb, &err); if (err) goto bail;
	/* scaling_list_enabled_flag */
	x = GetBits(bb, 1, &err); if (err) goto bail;
	if (x) {
		/* sps_scaling_list_data_present_flag */
		x = GetBits(bb, 1, &err); if (err) goto bail;
		if (x) {
			scaling_list_data();
		}
	}
	/* amp_enabled_flag */
	x = GetBits(bb, 1, &err); if (err) goto bail;
	/* sample_adaptive_offset_enabled_flag */
	sps->sample_adaptive_offset_enabled_flag = GetBits(bb, 1, &err); if (err) goto bail;
	/* pcm_enabled_flag */
	x = GetBits(bb, 1, &err); if (err) goto bail;
	if (x) {
		/* pcm_sample_bit_depth_luma_minus1 */
		x = GetBits(bb, 4, &err); if (err) goto bail;
		/* pcm_sample_bit_depth_chroma_minus1 */
		x = GetBits(bb, 4, &err); if (err) goto bail;
		/* log2_min_pcm_luma_coding_block_size_minus3 */
		y = read_golomb_uev(bb, &err); if (err) goto bail;
		/* log2_diff_max_min_pcm_luma_coding_block_size */
		y = read_golomb_uev(bb, &err); if (err) goto bail;
		/* pcm_loop_filter_disabled_flag */
		x = GetBits(bb, 1, &err); if (err) goto bail;
	}
	/* num_short_term_ref_pic_sets */
	sps->num_short_term_ref_pic_sets = read_golomb_uev(bb, &err); if (err) goto bail;
	for (stRpsIdx = 0; stRpsIdx < sps->num_short_term_ref_pic_sets; stRpsIdx++) {
		u32 delta_idx_minus1 = 0;
		//short_term_ref_pic_set(i)
		u8 inter_ref_pic_set_prediction_flag = 0;
		if (stRpsIdx != 0) {
			/* inter_ref_pic_set_prediction_flag */
			inter_ref_pic_set_prediction_flag = GetBits(bb, 1, &err); if (err) goto bail;
		}
		if (inter_ref_pic_set_prediction_flag) {
			u32 k = 0;
			if (stRpsIdx == sps->num_short_term_ref_pic_sets) {
				/* delta_idx_minus1 */
				delta_idx_minus1 = read_golomb_uev(bb, &err); if (err) goto bail;
			}
			/* delta_rps_sign */
			x = GetBits(bb, 1, &err); if (err) goto bail;
			/* abs_delta_rps_minus1 */
			y = read_golomb_uev(bb, &err); if (err) goto bail;
			for (j = 0; j <= sps->num_delta_pocs[stRpsIdx - (delta_idx_minus1 + 1)]; j++) {

				/* used_by_curr_pic_flag[j] */
				u32 ref_idc = GetBits(bb, 1, &err); if (err) goto bail;
				if (!ref_idc) {
					/* use_delta_flag[j] */
					x = GetBits(bb, 1, &err); if (err) goto bail;
					ref_idc = x << 1;
				}
				if (ref_idc == 1 || ref_idc == 2) {
					k++;
				}
			}
			sps->num_delta_pocs[stRpsIdx] = k;
		} else {
			u32 num_negative_pics = 0;
			u32 num_positive_pics = 0;
			/* num_negative_pics */
			num_negative_pics = read_golomb_uev(bb, &err); if (err) goto bail;
			/* num_positive_pics */
			num_positive_pics = read_golomb_uev(bb, &err); if (err) goto bail;
			for (i = 0; i < num_negative_pics; i++) {
				/* delta_poc_s0_minus1[i] */
				y = read_golomb_uev(bb, &err); if (err) goto bail;
				/* used_by_curr_pic_s0_flag[i] */
				x = GetBits(bb, 1, &err); if (err) goto bail;
			}
			for (i = 0; i < num_positive_pics; i++) {
				/* delta_poc_s1_minus1[i] */
				y = read_golomb_uev(bb, &err); if (err) goto bail;
				/* used_by_curr_pic_s1_flag[i] */
				x = GetBits(bb, 1, &err); if (err) goto bail;
			}
			sps->num_delta_pocs[stRpsIdx] = num_negative_pics + num_positive_pics;
		}

	}
	/* long_term_ref_pics_present_flag */
	sps->long_term_ref_pics_present_flag = GetBits(bb, 1, &err); if (err) goto bail;
	if (sps->long_term_ref_pics_present_flag) {
		/* num_long_term_ref_pics_sps */
		sps->num_long_term_ref_pics_sps = read_golomb_uev(bb, &err); if (err) goto bail;
		for (i = 0; i < sps->num_long_term_ref_pics_sps; i++) {
			/* lt_ref_pic_poc_lsb_sps[i] */
			x = GetBits(bb, sps->log2_max_pic_order_cnt_lsb_minus4 + 4, &err); if (err) goto bail;
			/* used_by_curr_pic_lt_sps_flag[i] */
			x = GetBits(bb, 1, &err); if (err) goto bail;
		}
	}
	/* sps_temporal_mvp_enabled_flag */
	sps->sps_temporal_mvp_enabled_flag = GetBits(bb, 1, &err); if (err) goto bail;

	/* sps_strong_intra_smoothing_enable_flag */
	x = GetBits(bb, 1, &err); if (err) goto bail;

	/* vui_parameters_present_flag */
	x = GetBits(bb, 1, &err); if (err) goto bail;

bail:
	return err;
}


MP4Err hevc_parse_pps_minimal(BitBuffer *bb, struct hevc_pps* pps) {
	MP4Err err = MP4NoErr;
	u8 x;
	u32 y, i;

	/* Get first header byte for nal_unit_type */
	err = GetBytes(bb, 1, &x); if (err) goto bail;

	/* 34 == PPS */
	//  if ((x >> 1) != 34) BAILWITHERROR(MP4BadParamErr);

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
	pps->cabac_init_present_flag = GetBits(bb, 1, &err); if (err) goto bail;

	/* num_ref_idx_l0_default_active_minus1 */
	pps->num_ref_idx_l0_default_active_minus1 = read_golomb_uev(bb, &err); if (err) goto bail;

	/* num_ref_idx_l1_default_active_minus1 */
	pps->num_ref_idx_l1_default_active_minus1 = read_golomb_uev(bb, &err); if (err) goto bail;

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
	pps->pps_slice_chroma_qp_offsets_present_flag = GetBits(bb, 1, &err); if (err) goto bail;
	/* weighted_pred_flag */
	pps->weighted_pred_flag = GetBits(bb, 1, &err); if (err) goto bail;
	/* weighted_bipred_flag */
	pps->weighted_bipred_flag = GetBits(bb, 1, &err); if (err) goto bail;
	/* transquant_bypass_enabled_flag */
	GetBits(bb, 1, &err); if (err) goto bail;

	/* tiles_enabled_flag */
	pps->tiles_enabled_flag = GetBits(bb, 1, &err); if (err) goto bail;

	/* entropy_coding_sync_enabled_flag */
	pps->entropy_coding_sync_enabled_flag = GetBits(bb, 1, &err); if (err) goto bail;

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
				pps->tile_column_width_minus1[i] = read_golomb_uev(bb, &err); if (err) goto bail;
			}
			for (i = 0; i < pps->num_tile_rows - 1; i++) {
				pps->tile_row_height_minus1[i] = read_golomb_uev(bb, &err); if (err) goto bail;
			}
			/* loop_filter_across_tiles_enabled_flag */
			GetBits(bb, 1, &err); if (err) goto bail;
		}
	}

	/* pps_loop_filter_across_slices_enabled_flag */
	pps->pps_loop_filter_across_slices_enabled_flag = GetBits(bb, 1, &err); if (err) goto bail;
	/* deblocking_filter_control_present_flag */
	x = GetBits(bb, 1, &err); if (err) goto bail;
	if (x) {
		/* deblocking_filter_override_enabled_flag */
		pps->deblocking_filter_override_enabled_flag = GetBits(bb, 1, &err); if (err) goto bail;
		/* pps_deblocking_filter_disabled_flag */
		x = GetBits(bb, 1, &err); if (err) goto bail;
		if (!x) {
			/* pps_beta_offset_div2 */
			y = read_golomb_uev(bb, &err); if (err) goto bail;
			/* pps_tc_offset_div2 */
			y = read_golomb_uev(bb, &err); if (err) goto bail;
		}
	}
	/* pps_scaling_list_data_present_flag */
	x = GetBits(bb, 1, &err); if (err) goto bail;
	if (x) {
		scaling_list_data();
	}
	/* lists_modification_present_flag */
	pps->lists_modification_present_flag = GetBits(bb, 1, &err); if (err) goto bail;
	/* log2_parallel_merge_level_minus2 */
	y = read_golomb_uev(bb, &err); if (err) goto bail;



bail:
	return err;
}


MP4Err hevc_parse_slice_header_minimal(BitBuffer *bb, struct hevc_poc* poc, struct hevc_slice_header* header,
struct hevc_sps* sps, struct hevc_pps* pps) {
	MP4Err err = MP4NoErr;
	u8 x = 0;
	u8 slice_temporal_mvp_enabled_flag = 0;
	u8 slice_deblocking_filter_disabled_flag = 0;
	u32 y = 0, i;
	u32 nal_type;

	/* Get first header byte for nal_unit_type */
	err = GetBytes(bb, 1, &x); if (err) goto bail;

	nal_type = x >> 1;
	// if ((x >> 1) != 34) BAILWITHERROR(MP4BadParamErr);
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
	header->dependent_slice = 0;
	if (!header->first_slice_segment_in_pic_flag) {
		u32 PicSizeInCtbsY = (u32)ceil(sps->pic_width_in_luma_samples / 64.0)*ceil(sps->pic_height_in_luma_samples / 64.0);
		if (pps->dependent_slice_segments_enabled_flag) {
			header->dependent_slice = GetBits(bb, 1, &err); if (err) goto bail;
		}

		/* Ceil( Log2( PicSizeInCtbsY ) ) bits */
		header->slice_segment_address = GetBits(bb, ceil_log2(PicSizeInCtbsY), &err); if (err) goto bail;
		/* printf("Slice segment address: %d, dependent segment: %d\n", header->slice_segment_address, header->dependent_slice); */
	}

	if (!header->dependent_slice) {
		u32 numRpsCurrTempList = 0;

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
		/* if( nal_unit_type  !=  IDR_W_RADL  &&  nal_unit_type  !=  IDR_N_LP ) */
		if (nal_type != 20) {
			s32 last_order_cnt_lsb = poc->order_cnt_lsb;
			s32 max_lsb = 1 << (sps->log2_max_pic_order_cnt_lsb_minus4 + 4);
			u8 const bits = sps->log2_max_pic_order_cnt_lsb_minus4 + 4;
			/* slice_pic_order_cnt_lsb */
			if (nal_type != 19) {
				u8 short_term_ref_pic_set_sps_flag = 0;
				poc->last_rap++;
				poc->order_cnt_lsb = GetBits(bb, bits, &err); if (err) goto bail;

				/* short_term_ref_pic_set_sps_flag */
				short_term_ref_pic_set_sps_flag = GetBits(bb, 1, &err); if (err) goto bail;
				if (!short_term_ref_pic_set_sps_flag) {
					/* short_term_ref_pic_set( stRpsIdx ) */
					u8 inter_ref_pic_set_prediction_flag = 0;
					u8 stRpsIdx = sps->num_short_term_ref_pic_sets;
					u32 j;
					if (stRpsIdx != 0) {
						/* inter_ref_pic_set_prediction_flag */
						inter_ref_pic_set_prediction_flag = GetBits(bb, 1, &err); if (err) goto bail;
					}
					if (inter_ref_pic_set_prediction_flag) {
						s32 delta_idx_minus1 = 0;
						if (stRpsIdx == sps->num_short_term_ref_pic_sets) {
							/* delta_idx_minus1 */
							delta_idx_minus1 = read_golomb_uev(bb, &err); if (err) goto bail;
						}
						/* delta_rps_sign */
						x = GetBits(bb, 1, &err); if (err) goto bail;
						/* abs_delta_rps_minus1 */
						y = read_golomb_uev(bb, &err); if (err) goto bail;
						for (j = 0; j <= sps->num_delta_pocs[stRpsIdx - (delta_idx_minus1 + 1)]; j++) {
							/* used_by_curr_pic_flag[j] */
							x = GetBits(bb, 1, &err); if (err) goto bail;
							if (x) numRpsCurrTempList++;
							if (!x) {
								/* use_delta_flag[j] */
								x = GetBits(bb, 1, &err); if (err) goto bail;
							}
						}
					} else {
						u32 num_negative_pics = 0;
						u32 num_positive_pics = 0;
						/* num_negative_pics */
						num_negative_pics = read_golomb_uev(bb, &err); if (err) goto bail;
						/* num_positive_pics */
						num_positive_pics = read_golomb_uev(bb, &err); if (err) goto bail;
						for (i = 0; i < num_negative_pics; i++) {
							/* delta_poc_s0_minus1[i] */
							y = read_golomb_uev(bb, &err); if (err) goto bail;
							/* used_by_curr_pic_s0_flag[i] */
							x = GetBits(bb, 1, &err); if (err) goto bail;
							if (x) numRpsCurrTempList++;
						}
						for (i = 0; i < num_positive_pics; i++) {
							/* delta_poc_s1_minus1[i] */
							y = read_golomb_uev(bb, &err); if (err) goto bail;
							/* used_by_curr_pic_s1_flag[i] */
							x = GetBits(bb, 1, &err); if (err) goto bail;
							if (x) numRpsCurrTempList++;
						}
						sps->num_delta_pocs[stRpsIdx] = num_negative_pics + num_positive_pics;
					}
				} else if (sps->num_short_term_ref_pic_sets > 1) {
					u8 bits = 0;
					while ((1 << bits) < sps->num_short_term_ref_pic_sets) {
						bits++;
					}
					if (bits) {
						x = GetBits(bb, bits, &err); if (err) goto bail;
					}
				}

				if (sps->long_term_ref_pics_present_flag) {
					u8 num_long_term_sps = 0;
					u8 num_long_term_pics = 0;
					if (sps->num_long_term_ref_pics_sps > 0) {
						/* num_long_term_sps */
						num_long_term_sps = read_golomb_uev(bb, &err); if (err) goto bail;
					}
					/* num_long_term_pics */
					num_long_term_pics = read_golomb_uev(bb, &err); if (err) goto bail;
					for (i = 0; i < num_long_term_sps + num_long_term_pics; i++) {
						if (i < num_long_term_sps) {
							if (sps->num_long_term_ref_pics_sps > 1) {
								/* lt_idx_sps[i] */
								x = GetBits(bb, bits, &err); if (err) goto bail;
							}
						} else {
							/* poc_lsb_lt[i] */
							x = GetBits(bb, ceil_log2(sps->num_long_term_ref_pics_sps), &err); if (err) goto bail;
							/* used_by_curr_pic_lt_flag[i] */
							x = GetBits(bb, 1, &err); if (err) goto bail;
							if (x) numRpsCurrTempList++;
						}
						/* delta_poc_msb_present_flag[i] */
						x = GetBits(bb, 1, &err); if (err) goto bail;
						if (x) {
							/* delta_poc_msb_cycle_lt[i] */
							y = read_golomb_uev(bb, &err); if (err) goto bail;
						}
					}
				}

				if (sps->sps_temporal_mvp_enabled_flag) {
					/* slice_temporal_mvp_enabled_flag */
					slice_temporal_mvp_enabled_flag = GetBits(bb, 1, &err); if (err) goto bail;
				}



			} else {
				if (header->first_slice_segment_in_pic_flag) {
					poc->order_cnt_lsb = 0;
					last_order_cnt_lsb = 0;
					poc->last_rap = 0;
				}
			}

			/* Calculate POC only on first slice segment */
			if (header->first_slice_segment_in_pic_flag) {
				if (poc->order_cnt_lsb < last_order_cnt_lsb &&
					last_order_cnt_lsb - poc->order_cnt_lsb >(max_lsb / 2)) {
					poc->order_cnt_msb += max_lsb;
				} else if (poc->order_cnt_lsb > last_order_cnt_lsb &&
					poc->order_cnt_lsb - last_order_cnt_lsb > (max_lsb / 2)) {
					poc->order_cnt_msb -= max_lsb;
				}
				last_order_cnt_lsb = poc->order_cnt_lsb;
			}
		} else {
			if (header->first_slice_segment_in_pic_flag)
				poc->order_cnt_lsb += 1;
		}

		if (sps->sample_adaptive_offset_enabled_flag) {
			/* slice_sao_luma_flag */
			sps->slice_sao_luma_flag = GetBits(bb, 1, &err); if (err) goto bail;
			/* slice_sao_chroma_flag */
			sps->slice_sao_chroma_flag = GetBits(bb, 1, &err); if (err) goto bail;
		}
		if (header->slice_type == SLICE_P || header->slice_type == SLICE_B) {
			u8 num_ref_idx_l0_active_minus1 = pps->num_ref_idx_l0_default_active_minus1;
			u8 num_ref_idx_l1_active_minus1 = pps->num_ref_idx_l1_default_active_minus1;

			/* num_ref_idx_active_override_flag */
			x = GetBits(bb, 1, &err); if (err) goto bail;
			if (x) {
				/* num_ref_idx_l0_active_minus1 */
				num_ref_idx_l0_active_minus1 = read_golomb_uev(bb, &err); if (err) goto bail;
				if (header->slice_type == SLICE_B) {
					/* num_ref_idx_l1_active_minus1 */
					num_ref_idx_l1_active_minus1 = read_golomb_uev(bb, &err); if (err) goto bail;
				}
			}
			if (pps->lists_modification_present_flag  &&  poc->last_rap > 1) {
				/* ref_pic_lists_modification() */
				u32 temp_pic = 0;
				u32 bits = 0;
				while (numRpsCurrTempList >>= 1) {
					bits++;
				}

				/* ref_pic_list_modification_flag_l0 */
				x = GetBits(bb, 1, &err); if (err) goto bail;
				if (x) {

					for (temp_pic = 0; temp_pic < num_ref_idx_l0_active_minus1; temp_pic++) {
						/* list_entry_l0 */
						x = GetBits(bb, bits, &err); if (err) goto bail;
					}
				}
				if (header->slice_type == SLICE_B) {
					/* ref_pic_list_modification_flag_l1 */
					x = GetBits(bb, 1, &err); if (err) goto bail;
					if (x) {
						for (temp_pic = 0; temp_pic < num_ref_idx_l1_active_minus1; temp_pic++) {
							/* list_entry_l1 */
							x = GetBits(bb, bits, &err); if (err) goto bail;
						}
					}
				}
			}
			if (header->slice_type == SLICE_B) {
				/* mvd_l1_zero_flag */
				x = GetBits(bb, 1, &err); if (err) goto bail;
			}
			if (pps->cabac_init_present_flag) {
				/* cabac_init_flag */
				x = GetBits(bb, 1, &err); if (err) goto bail;
			}
			if (slice_temporal_mvp_enabled_flag) {
				u8 collocated_from_l0_flag = 0;
				if (header->slice_type == SLICE_B) {
					/* collocated_from_l0_flag */
					collocated_from_l0_flag = GetBits(bb, 1, &err); if (err) goto bail;
				}
				if ((collocated_from_l0_flag  &&  num_ref_idx_l0_active_minus1 > 0) ||
					(!collocated_from_l0_flag  &&  num_ref_idx_l1_active_minus1 > 0)) {
					/* collocated_ref_idx */
					y = read_golomb_uev(bb, &err); if (err) goto bail;
				}
			}
			if ((pps->weighted_pred_flag  &&  header->slice_type == SLICE_P) ||
				(pps->weighted_bipred_flag  &&  header->slice_type == SLICE_B)) {
				printf("pred_weight_table() not implemented!\r\n");
				assert(0);
				exit(1);
				/* pred_weight_table()*/
			}
			/* five_minus_max_num_merge_cand */
			y = read_golomb_uev(bb, &err); if (err) goto bail;
		}

		/* slice_qp_delta */
		y = read_golomb_uev(bb, &err); if (err) goto bail;
		//printf("Slice QP Delta: %d\n", y);
		if (pps->pps_slice_chroma_qp_offsets_present_flag) {
			/* slice_cb_qp_offset */
			y = read_golomb_uev(bb, &err); if (err) goto bail;
			/* slice_cr_qp_offset */
			y = read_golomb_uev(bb, &err); if (err) goto bail;
		}
		if (pps->deblocking_filter_override_enabled_flag) {
			/* deblocking_filter_override_flag */
			x = GetBits(bb, 1, &err); if (err) goto bail;
			if (x) {
				/* slice_deblocking_filter_disabled_flag */
				slice_deblocking_filter_disabled_flag = GetBits(bb, 1, &err); if (err) goto bail;
				if (!slice_deblocking_filter_disabled_flag) {
					/* slice_beta_offset_div2 */
					y = read_golomb_uev(bb, &err); if (err) goto bail;
					/* slice_tc_offset_div2 */
					y = read_golomb_uev(bb, &err); if (err) goto bail;
				}
			}
		}
		if (pps->pps_loop_filter_across_slices_enabled_flag &&
			(sps->slice_sao_luma_flag || sps->slice_sao_chroma_flag ||
			!slice_deblocking_filter_disabled_flag)) {
			/* slice_loop_filter_across_slices_enabled_flag */
			x = GetBits(bb, 1, &err); if (err) goto bail;
		}
	}
	if (pps->tiles_enabled_flag || pps->entropy_coding_sync_enabled_flag) {
		/* num_entry_point_offsets */
		header->num_entry_point_offsets = read_golomb_uev(bb, &err); if (err) goto bail;
		if (header->num_entry_point_offsets > 0) {
			header->entry_point_offset_minus1 = (u32 *)malloc(header->num_entry_point_offsets * sizeof(u32));
			u32 offset_len_minus1 = read_golomb_uev(bb, &err); if (err) goto bail;
			for (i = 0; i < header->num_entry_point_offsets; i++) {
				header->entry_point_offset_minus1[i] = GetBits(bb, offset_len_minus1 + 1, &err); if (err) goto bail;
			}
		}
	}

	header->poc = poc->order_cnt_msb + poc->order_cnt_lsb;
	/*printf("POC: %d\n\n", header->poc); */

bail:
	return err;
}

u8* stripNALEmulation(u8* buffer, u32* bufferLen) {
	u8* outBuffer;
	u32 zerocount = 0;
	u32 emulationPreventionBytes = 0;
	u32 ii = 0;
	u32 i;
	for (i = 0; i < *bufferLen; i++) {
		/* Check for emulation prevention code */
		if (zerocount == 2 &&
			buffer[i] == 0x03 &&
			i + 1 < *bufferLen   &&
			buffer[i + 1] <= 0x04) {
			zerocount = buffer[i + 1] ? 0 : 1;
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

int parseHEVCNal(FILE* input, u8** data, int* data_len) {
	size_t startPos;
	size_t NALStart = 0;
	size_t NALEnd = 0;
	u32 NALULen;
	u8* NALU;

	u8 byte;
	int zerocount;
	int frame = 0;
	char nal_header[2];

	//Save start position
	startPos = ftell(input);

	//Search for sync
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

	//Search for next sync
	zerocount = 0;
	while (1) {
		if (!fread(&byte, 1, 1, input)) {
			zerocount = 0;
			break;
		}
		//Sync found
		if (zerocount >= 2 && byte == 1) {
			//fread(&byte, 1, 1, input);
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

	//free(NALU);

	return byte;
}


ISOErr analyze_hevc_stream(FILE* input, struct hevc_stream* stream) {
	ISOErr err = MP4NoErr;
	ISOHandle spsHandle = NULL;
	ISOHandle vpsHandle = NULL;
	ISOHandle ppsHandle = NULL;
	u8 frameNal = 0;
	u32 size_temp;
	u8* data = NULL;
	u8 *slicedata = NULL;
	u32 slicedatalen = 0;
	u32 datalen;
	BitBuffer bb;
	size_t startPos;
	struct hevc_poc poc;
	static u8 sps_found = 0, vps_found = 0, pps_found = 0;

	memset(&poc, 0, sizeof(struct hevc_poc));
	memset(stream, 0, sizeof(struct hevc_stream));
	startPos = ftell(input);

	stream->allocated_count = 16;
	stream->header = malloc(sizeof(struct hevc_slice_header *) * stream->allocated_count);



	/* Loop whole input file */
	while (1) {
		long peek_origin = 0;
		struct hevc_slice_header *header = malloc(sizeof(struct hevc_slice_header));
		memset(header, 0, sizeof(struct hevc_slice_header));

		frameNal = 0;
		/* Parse NAL units until slice (and the next) is found */
		while (frameNal != 2) {
			s32 naltype = parseHEVCNal(input, &data, &datalen);
			/*printf("NAL type: %d\r\n", naltype);*/
			if (naltype == -1 && frameNal) {
				fseek(input, peek_origin, SEEK_SET);
				break;
			}
			if (naltype == -1) break;
			switch (naltype) {
			case 32: /* VPS */
				if (vps_found) {
					free(data); data = NULL;
					/*printf("Another VPS\r\n");*/
					continue;
				}
				vps_found = 1;
				ISODisposeHandle(vpsHandle);
				err = ISONewHandle(datalen, &vpsHandle);
				memcpy((*vpsHandle), data, datalen);
				free(data); data = NULL;
				break;
			case 33: /* SPS */
				if (sps_found) {
					header->aggregator_data = (u8*)realloc(header->aggregator_data, header->aggregator_datalen + datalen + 4);
					memcpy(&header->aggregator_data[header->aggregator_datalen + 4], data, datalen);
					PUT32(&header->aggregator_data[header->aggregator_datalen], datalen);
					header->aggregator_datalen += datalen + 4;

					free(data); data = NULL;
					/*printf("Another SPS\r\n");*/
					continue;
				}
				sps_found = 1;
				ISODisposeHandle(spsHandle);
				err = ISONewHandle(datalen, &spsHandle);
				memcpy((*spsHandle), data, datalen);
				free(data); data = NULL;
				break;
			case 34: /* PPS */
				if (pps_found) {
					header->aggregator_data = (u8*)realloc(header->aggregator_data, header->aggregator_datalen + datalen + 4);
					memcpy(&header->aggregator_data[header->aggregator_datalen + 4], data, datalen);
					PUT32(&header->aggregator_data[header->aggregator_datalen], datalen);
					header->aggregator_datalen += datalen + 4;

					free(data); data = NULL;
					/*printf("Another PPS\r\n");*/
					continue;
				}
				pps_found = 1;
				ISODisposeHandle(ppsHandle);
				err = ISONewHandle(datalen, &ppsHandle);
				memcpy((*ppsHandle), data, datalen);
				free(data); data = NULL;
				break;
			default:
				if (naltype < 32) {
					u32 layer = data[1] >> 3;
					if (!frameNal) {
						slicedata = data;
						slicedatalen = datalen;
					}
					frameNal += !layer;
					if (layer) {
						header->aggregator_header = GET16(data);
						/*printf("Push aggregator layer 1\r\n");*/
						header->aggregator_data = (u8*)realloc(header->aggregator_data, header->aggregator_datalen + datalen + 4);
						memcpy(&header->aggregator_data[header->aggregator_datalen + 4], data, datalen);
						PUT32(&header->aggregator_data[header->aggregator_datalen], datalen);
						header->aggregator_datalen += datalen + 4;
						free(data); data = NULL;
					}
				} else {
					if (header->aggregator_datalen) {
						header->aggregator_data = (u8*)realloc(header->aggregator_data, header->aggregator_datalen + datalen + 4);
						memcpy(&header->aggregator_data[header->aggregator_datalen + 4], data, datalen);
						PUT32(&header->aggregator_data[header->aggregator_datalen], datalen);
						header->aggregator_datalen += datalen + 4;
					}
					free(data); data = NULL;
				}
			}

			if (frameNal != 2) {
				peek_origin = ftell(input);
			}
		}

		/* Last frame was read */
		if (!frameNal) {
			break;
		}

		fseek(input, peek_origin, SEEK_SET);

		/* Parse slice header and parameter set data */

		/* PPS */
		err = ISOGetHandleSize(ppsHandle, &size_temp); if (err) goto bail;
		err = BitBuffer_Init(&bb, (u8*)*ppsHandle, 8 * size_temp); if (err) goto bail;
		err = hevc_parse_pps_minimal(&bb, &stream->pps); if (err) goto bail;
		/* SPS */
		err = ISOGetHandleSize(spsHandle, &size_temp); if (err) goto bail;
		err = BitBuffer_Init(&bb, (u8*)*spsHandle, 8 * size_temp); if (err) goto bail;
		err = hevc_parse_sps_minimal(&bb, &stream->sps); if (err) goto bail;
		/* Slice header */
		err = BitBuffer_Init(&bb, slicedata, 8 * slicedatalen); if (err) goto bail;
		err = hevc_parse_slice_header_minimal(&bb, &poc, header, &stream->sps, &stream->pps); if (err) goto bail;

		/* Double the allocated space when depleted */
		if (stream->used_count == stream->allocated_count) {
			stream->allocated_count *= 2;
			stream->header = realloc(stream->header, stream->allocated_count * sizeof(struct hevc_slice_header *));				
		}
		stream->header[stream->used_count] = header;
		stream->used_count++;

	}

	/* Release memory */
	ISODisposeHandle(ppsHandle);
	ISODisposeHandle(vpsHandle);
	ISODisposeHandle(spsHandle);

	free(data); data = NULL;

	/* Rewind back to where we were at the start */
	fseek(input, startPos, SEEK_SET);

bail:
	return err;
}
