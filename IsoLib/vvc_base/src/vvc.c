#include <math.h>
#include <assert.h>
#include <string.h>
#include "ISOMovies.h"
#include "vvc_structures.h"
#include "vvc.h"
#include "vvc_tools.h"

MP4Err vvc_parse_sps_minimal(BitBuffer* bb, struct vvc_sps* sps) {
	MP4Err err = MP4NoErr;
  u32 ui, y, ue, uvBits, CtbSizeY, tmpWidthVal, tmpHeightVal;
  s32 i;
  u8 x;

	u32 sps_num_subpics_minus1, sps_independent_subpics_flag, sps_subpic_same_size_flag,
    gci_present_flag, gci_num_reserved_bits;

	u8 ptl_sublayer_level_present_flag[8];
  u8 sps_qtbtt_dual_tree_intra_flag = 0;

	/* Get first two bytes for nal_unit_type */
  err = GetBytes(bb, 1, &x);
  err = GetBytes(bb, 1, &x);
  if(err) goto bail;
  /* SPS == 15 */
  if((x >> 3) != 15) err = MP4BadParamErr; 
	if(err) goto bail;

  err = GetBytes(bb, 1, &x);
  if(err) goto bail;
  err = GetBytes(bb, 1, &x);
  if(err) goto bail;

  sps->sps_max_sublayers									 = ((x & 0xff) >> 5) + 1;
  sps->sps_chroma_format_idc               = (x & 0x1f) >> 3;
  sps->sps_log2_ctu_size_minus5            = (x & 0x06) >> 1;
  CtbSizeY                                 = 1 << (sps->sps_log2_ctu_size_minus5 + 5);
  sps->sps_ptl_dpb_hrd_params_present_flag = x & 0x01;

	/* profile_tile_level */
  if(sps->sps_ptl_dpb_hrd_params_present_flag)
  {
    err = GetBytes(bb, 1, &x);
    if(err) goto bail;
    //config->native_ptl.general_profile_idc = (x & 0xff) >> 1;
    //config->native_ptl.general_tier_flag   = x & 0x01;

    err = GetBytes(bb, 1, &x);
    if(err) goto bail;
    //config->native_ptl.general_level_idc = x;

    x = (u8)GetBits(bb, 1, &err);
    if(err) goto bail;
    //config->native_ptl.ptl_frame_only_constraint_flag = x;

    x = (u8)GetBits(bb, 1, &err);
    if(err) goto bail;
    //config->native_ptl.ptl_multi_layer_enabled_flag = x;

    /* general_constraints_info */
    {
      /* gci_present_flag */
      x = (u8)GetBits(bb, 1, &err);
      gci_present_flag = x;
      if(gci_present_flag)
      {
        for(ui = 0;ui<10;ui++)
        {
          err = GetBytes(bb, 1, &x);
				}
        for(ui = 0; ui < x; ui++)
        {
          x = (u8)GetBits(bb, 1, &err);
				}
			}
      /* byte_alligned */
      while(bb->curbits % 8 != 0)
      {
        x = (u8)GetBits(bb, 1, &err);
        if(err) goto bail;
      }
      assert(bb->curbits % 8 == 0);
		}

    for(i = sps->sps_max_sublayers - 2; i >= 0; i--)
    {
      x = (u8)GetBits(bb, 1, &err);
      if(err) goto bail;
      ptl_sublayer_level_present_flag[i] = x;
    }
    for(ui = sps->sps_max_sublayers; ui <= 8 && sps->sps_max_sublayers > 1; ui++)
    {
      /* ptl_reserved_zero_bit, for byte aligned */
      x = (u8)GetBits(bb, 1, &err);
      if(err) goto bail;
    }
    assert(bb->curbits % 8 == 0);

    for(i = sps->sps_max_sublayers - 2; i >= 0; i--)
    {
      if(ptl_sublayer_level_present_flag[i])
      {
        /* sublayer_level_idc */
        err = GetBytes(bb, 1, &x);
        if(err) goto bail;
      }
    }

    err = GetBytes(bb, 1, &x);
    if(err) goto bail;
    //config->native_ptl.ptl_num_sub_profiles = x;

    for(ui = 0; ui < x; ui++)
    {
      y = (u32)GetBits(bb, 32, &err);
      if(err) goto bail;
      //config->native_ptl.general_sub_profile_idc[ui] = y;
    }
  }

  /* sps_gdr_enabled_flag */
  x = (u8)GetBits(bb, 1, &err);
  if(err) goto bail;
  /* sps_ref_pic_resampling_enabled_flag */
  x = (u8)GetBits(bb, 1, &err);
  if(err) goto bail;
  if(x)
  {
    /* sps_res_change_in_clvs_allowed_flag */
    x = (u8)GetBits(bb, 1, &err);
    if(err) goto bail;
  }

  /* sps_pic_width_max_in_luma_samples */
  ue = read_golomb_uev(bb, &err);
  if(err) goto bail;
  sps->sps_pic_width_max_in_luma_samples = ue;

  /* sps_pic_height_max_in_luma_samples */
  ue = read_golomb_uev(bb, &err);
  if(err) goto bail;
  sps->sps_pic_height_max_in_luma_samples = ue;

  /* sps_conformance_window_flag */
  x = (u8)GetBits(bb, 1, &err);
  if(err) goto bail;
  if(x)
  {
    for(ui = 0; ui < 4; ui++)
    {
      /* win_offset */
      ue = read_golomb_uev(bb, &err);
      if(err) goto bail;
    }
  }

  /* sps_subpic_info_present_flag */
  sps->sps_subpic_info_present_flag = (u8)GetBits(bb, 1, &err);
  if(err) goto bail;
  if(sps->sps_subpic_info_present_flag)
  {
    sps_num_subpics_minus1       = 0;
    sps_independent_subpics_flag = 0;
    sps_subpic_same_size_flag    = 0;

    /* sps_num_subpics_minus1 */
    ue = read_golomb_uev(bb, &err);
    if(err) goto bail;
    sps_num_subpics_minus1 = ue;
    if(sps_num_subpics_minus1 > 0)
    {
      /* sps_independent_subpics_flag & sps_subpic_same_size_flag */
      x = (u8)GetBits(bb, 1, &err);
      if(err) goto bail;
      sps_independent_subpics_flag = x;
      x                            = (u8)GetBits(bb, 1, &err);
      if(err) goto bail;
      sps_subpic_same_size_flag = x;
    }

    for(ui = 0; sps_num_subpics_minus1 > 0 && ui <= sps_num_subpics_minus1; ui++)
    {
      if(!sps_subpic_same_size_flag || ui == 0)
      {
        if(ui > 0 && sps->sps_pic_width_max_in_luma_samples > CtbSizeY)
        {
          tmpWidthVal = (sps->sps_pic_width_max_in_luma_samples + CtbSizeY - 1) / CtbSizeY;
          uvBits      = 0;
          while(tmpWidthVal > ((u32)1 << uvBits))
          {
            uvBits += 1;
          }
          y = GetBits(bb, uvBits, &err);
          if(err) goto bail;
        }
        if(ui > 0 && sps->sps_pic_height_max_in_luma_samples > CtbSizeY)
        {
          tmpHeightVal = (sps->sps_pic_height_max_in_luma_samples + CtbSizeY - 1) / CtbSizeY;
          uvBits       = 0;
          while(tmpHeightVal > ((u32)1 << uvBits))
          {
            uvBits += 1;
          }
          y = GetBits(bb, uvBits, &err);
          if(err) goto bail;
        }
        if(ui < sps_num_subpics_minus1 && sps->sps_pic_width_max_in_luma_samples > CtbSizeY)
        {
          tmpWidthVal = (sps->sps_pic_width_max_in_luma_samples + CtbSizeY - 1) / CtbSizeY;
          uvBits      = 0;
          while(tmpWidthVal > ((u32)1 << uvBits))
          {
            uvBits += 1;
          }
          y = GetBits(bb, uvBits, &err);
          if(err) goto bail;
        }
        if(ui < sps_num_subpics_minus1 && sps->sps_pic_height_max_in_luma_samples > CtbSizeY)
        {
          tmpHeightVal = (sps->sps_pic_height_max_in_luma_samples + CtbSizeY - 1) / CtbSizeY;
          uvBits       = 0;
          while(tmpHeightVal > ((u32)1 << uvBits))
          {
            uvBits += 1;
          }
          y = GetBits(bb, uvBits, &err);
          if(err) goto bail;
        }
      }
      if(!sps_independent_subpics_flag)
      {
        x = (u8)GetBits(bb, 2, &err);
        if(err) goto bail;
      }
    }
    /* sps_subpic_id_len_minus1 */
    ue = read_golomb_uev(bb, &err);
    if(err) goto bail;
    sps->sps_subpic_id_len_minus1 = ue;
    /* sps_subpic_id_mapping_explicitly_signalled_flag */
    x = (u8)GetBits(bb, 1, &err);
    if(err) goto bail;
    if(x)
    {
      /* sps_subpic_id_mapping_present_flag */
      x = (u8)GetBits(bb, 1, &err);
      if(err) goto bail;
      if(x)
      {
        for(ui = 0; ui <= sps_num_subpics_minus1; ui++)
        {
          /* sps_subpic_id */
          y = GetBits(bb, sps->sps_subpic_id_len_minus1 + 1, &err);
          if(err) goto bail;
        }
      }
    }
  }

  /* sps_bitdepth_minus8 */
  ue = read_golomb_uev(bb, &err);
  if(err) goto bail;
  sps->sps_bitdepth_minus8 = ue;

  sps->sps_entropy_coding_sync_enabled_flag = GetBits(bb, 1, &err); if(err) goto bail;
  sps->sps_entry_point_offsets_present_flag = GetBits(bb, 1, &err); if(err) goto bail;
  sps->sps_log2_max_pic_order_cnt_lsb_minus4 = GetBits(bb, 4, &err); if(err) goto bail;
  sps->sps_poc_msb_cycle_flag = GetBits(bb, 1, &err); if(err) goto bail;
  if(sps->sps_poc_msb_cycle_flag)
  {
    sps->sps_poc_msb_cycle_len_minus1 = read_golomb_uev(bb, &err); if(err) goto bail;
  }
  sps->sps_num_extra_ph_bytes = GetBits(bb, 2, &err); if(err) goto bail;

  sps->NumExtraPhBits = 0;
  for(ui = 0; ui < sps->sps_num_extra_ph_bytes * 8; ui++)
  {
    if(GetBits(bb, 1, &err))
    {
      sps->NumExtraPhBits++;
    }
  }

  sps->sps_num_extra_sh_bytes = GetBits(bb, 2, &err); if(err) goto bail;
  sps->NumExtraShBits = 0;
  for(ui = 0; ui < sps->sps_num_extra_sh_bytes * 8; ui++)
  {
    if(GetBits(bb, 1, &err))
    {
      sps->NumExtraShBits++;
    }
  }

  if(sps->sps_ptl_dpb_hrd_params_present_flag)
  {
    if(sps->sps_max_sublayers > 1)
      x = GetBits(bb, 1, &err); if(err) goto bail;
    for(ui = (x ? 0 : sps->sps_max_sublayers - 1); i <= sps->sps_max_sublayers - 1; i++)
    {
      ue = read_golomb_uev(bb, &err); if(err) goto bail;
      ue = read_golomb_uev(bb, &err); if(err) goto bail;
      ue = read_golomb_uev(bb, &err); if(err) goto bail;
    }
  }
  /* sps_log2_min_luma_coding_block_size_minus2 */
  ue = read_golomb_uev(bb, &err); if(err) goto bail;
  /* sps_partition_constraints_override_enabled_flag */
  x = GetBits(bb, 1, &err); if(err) goto bail;

  ue = read_golomb_uev(bb, &err); if(err) goto bail;
  ue = read_golomb_uev(bb, &err); if(err) goto bail;
  if(ue)
  {
    /* sps_log2_diff_max_bt_min_qt_intra_slice_luma */
    ue = read_golomb_uev(bb, &err); if(err) goto bail;
    ue = read_golomb_uev(bb, &err); if(err) goto bail;
  }
  if(sps->sps_chroma_format_idc != 0)
  {
    sps_qtbtt_dual_tree_intra_flag = GetBits(bb, 1, &err); if(err) goto bail; 
  }
  if(sps_qtbtt_dual_tree_intra_flag)
  {
    ue = read_golomb_uev(bb, &err); if(err) goto bail;
    ue = read_golomb_uev(bb, &err); if(err) goto bail;
    if(ue != 0)
    {
      ue = read_golomb_uev(bb, &err); if(err) goto bail;
      ue = read_golomb_uev(bb, &err); if(err) goto bail;
    }
  }
  /* sps_log2_diff_min_qt_min_cb_inter_slice */
  ue = read_golomb_uev(bb, &err); if(err) goto bail;
  /* sps_max_mtt_hierarchy_depth_inter_slice */
  ue = read_golomb_uev(bb, &err); if(err) goto bail;
  if(ue)
  {
    /* sps_log2_diff_max_bt_min_qt_inter_slice */
    ue = read_golomb_uev(bb, &err); if(err) goto bail;
    ue = read_golomb_uev(bb, &err); if(err) goto bail;
  }
  if(CtbSizeY > 32)
  {
    /* sps_max_luma_transform_size_64_flag */
    x = GetBits(bb, 1, &err); if(err) goto bail;
  }
  /* sps_transform_skip_enabled_flag */
  x = GetBits(bb, 1, &err); if(err) goto bail;
  if(x)
  {
    ue = read_golomb_uev(bb, &err); if(err) goto bail;
    x = GetBits(bb, 1, &err); if(err) goto bail;
  }
  x = GetBits(bb, 1, &err); if(err) goto bail;
  if(x)
  {
    /* sps_explicit */
    x = GetBits(bb, 1, &err); if(err) goto bail;
    x = GetBits(bb, 1, &err); if(err) goto bail;
  }
  /* sps_lfnst_enabled_flag */
  x = GetBits(bb, 1, &err); if(err) goto bail;
  if(sps->sps_chroma_format_idc != 0)
  {
    x = GetBits(bb, 1, &err); if(err) goto bail;
    x = GetBits(bb, 1, &err); if(err) goto bail;
  }

	/* todo continue parse */

bail:
  return err;
}

MP4Err vvc_parse_pps_minimal(BitBuffer *bb, struct vvc_pps* pps) {
	MP4Err err = MP4NoErr;
  u8 x, pps_single_slice_per_subpic_flag = 1;
	u32 ue, ui;

  u8 pps_subpic_id_mapping_present_flag, pps_no_pic_partition_flag, pps_num_exp_tile_columns_minus1,
    pps_num_exp_tile_rows_minus1;

	/* Get first two bytes for nal_unit_type */
  err = GetBytes(bb, 1, &x);
  err = GetBytes(bb, 1, &x);
  if(err) goto bail;
  /* PPS == 15 */
  if((x >> 3) != 16) err = MP4BadParamErr;
  if(err) goto bail;

	x = (u8)GetBits(bb, 6, &err); if(err) goto bail;

  x = (u8)GetBits(bb, 4, &err); if(err) goto bail;

	/* pps_mixed_nalu_types_in_pic_flag */
  x = (u8)GetBits(bb, 1, &err); if(err) goto bail;

	/* pps_pic_width_in_luma_samples */
  ue = read_golomb_uev(bb, &err); if(err) goto bail;

	/* pps_pic_height_in_luma_samples */
	ue = read_golomb_uev(bb, &err); if(err) goto bail;

	/* pps_conformance_window_flag */
	x = (u8)GetBits(bb, 1, &err); if(err) goto bail;
  if(x)
  {
    for(ui = 0; ui < 4; ui++)
    {
      /* win_offset */
			ue = read_golomb_uev(bb, &err); if(err) goto bail;		
		}	
	}

	/* pps_scaling_window_explicit_signalling_flag */
  x = (u8)GetBits(bb, 1, &err); if(err) goto bail;
  if(x)
  {
    for(ui = 0; ui < 4; ui++)
    {
      /* win_offset */
      ue = read_golomb_uev(bb, &err);
      if(err) goto bail;
    }
  }

	x = (u8)GetBits(bb, 1, &err); if(err) goto bail;
  pps_no_pic_partition_flag = (u8)GetBits(bb, 1, &err); if(err) goto bail;
  pps_subpic_id_mapping_present_flag = (u8)GetBits(bb, 1, &err); if(err) goto bail;
  if(pps_subpic_id_mapping_present_flag)
  {
    if(!pps_no_pic_partition_flag)
    {
      pps->pps_num_subpics_minus1 = read_golomb_uev(bb, &err); if(err) goto bail;
    }
    pps->pps_subpic_id_len_minus1 = read_golomb_uev(bb, &err); if(err) goto bail;
    pps->pps_subpic_id = (u32*)malloc(sizeof(u32) * (pps->pps_num_subpics_minus1 + 1));
    for(ui = 0; ui <= pps->pps_num_subpics_minus1; ui++)
    {
      pps->pps_subpic_id[ui] = GetBits(bb, pps->pps_subpic_id_len_minus1 + 1, &err);
      if(err) goto bail;
    }
  }
  if(!pps_no_pic_partition_flag)
  {
    /* pps_log2_ctu_size_minus5 */
    x = (u8)GetBits(bb, 2, &err); if(err) goto bail;
    pps_num_exp_tile_columns_minus1 = read_golomb_uev(bb, &err); if(err) goto bail;
    pps_num_exp_tile_rows_minus1 = read_golomb_uev(bb, &err); if(err) goto bail;
    for(ui = 0; ui <= pps_num_exp_tile_columns_minus1; ui++)
    {
      /* pps_tile_column_width_minus1 */
      ue = read_golomb_uev(bb, &err); if(err) goto bail;
    }
    for(ui = 0; ui <= pps_num_exp_tile_rows_minus1; ui++)
    {
      /* pps_tile_row_height_minus1 */
      ue = read_golomb_uev(bb, &err); if(err) goto bail;
    }
    if(/*NumTilesInPic > 1*/1)
    {
      x = (u8)GetBits(bb, 1, &err); if(err) goto bail;
      pps->pps_rect_slice_flag = GetBits(bb, 1, &err); if(err) goto bail;
    }
    if(pps->pps_rect_slice_flag)
    {
      pps_single_slice_per_subpic_flag = (u8)GetBits(bb, 1, &err);
      if(err) goto bail;
    }
    if(pps->pps_rect_slice_flag && !pps_single_slice_per_subpic_flag)
    {
      pps->pps_num_slices_in_pic_minus1 = read_golomb_uev(bb, &err); if(err) goto bail;
      if(pps->pps_num_slices_in_pic_minus1 > 1)
      {
        x = (u8)GetBits(bb, 1, &err); if(err) goto bail;
      }
      for(ui = 0; ui < pps->pps_num_slices_in_pic_minus1; ui++)
      {
        //todo
      }
    }

  }

	/* todo continue parse */
bail:
	return err;
}

MP4Err vvc_parse_ph_minimal(BitBuffer *bb, struct vvc_picture_header *ph, struct vvc_sps *sps,
                            struct vvc_pps *pps)
{
  MP4Err err = MP4NoErr;

  ph->gdr_or_irap_pic_flag = (u8)GetBits(bb, 1, &err); if(err) goto bail;
  ph->non_ref_pic_flag = (u8)GetBits(bb, 1, &err); if(err) goto bail;
  if(ph->gdr_or_irap_pic_flag)
  {
    ph->gdr_pic_flag = (u8)GetBits(bb, 1, &err); if(err) goto bail;
  }
  ph->inter_slice_allowed_flag = (u8)GetBits(bb, 1, &err); if(err) goto bail;
  if(ph->inter_slice_allowed_flag)
  {
    ph->intra_slice_allowed_flag = (u8)GetBits(bb, 1, &err); if(err) goto bail;
  }
  ph->pic_parameter_set_id = read_golomb_uev(bb, &err); if(err) goto bail;
  ph->pic_order_cnt_lsb = GetBits(bb, sps->sps_log2_max_pic_order_cnt_lsb_minus4 + 4, &err); if(err) goto bail;
  ph->poc_lsb = ph->pic_order_cnt_lsb;
  if(ph->gdr_pic_flag)
  {
      ph->recovery_poc_cnt = read_golomb_uev(bb, &err); if(err) goto bail;
  }

  GetBits(bb, sps->NumExtraPhBits, &err); if(err) goto bail;

  if(sps->sps_poc_msb_cycle_flag)
  {
    ph->poc_msb_cycle_present_flag = (u8)GetBits(bb, 1, &err); if(err) goto bail;
    if(ph->poc_msb_cycle_present_flag)
    {
      ph->poc_msb_cycle_val = GetBits(bb, sps->sps_poc_msb_cycle_len_minus1 + 1, &err);
      if(err) goto bail;
    }
  }

	/* todo continue parse */
bail:
  return err;
}

void vvc_compute_poc(struct vvc_slice_header* header) {
  u32 max_poc_lsb = header->max_poc_lsb;

  if(header->ph.poc_msb_cycle_present_flag)
  {
    header->ph.poc_msb = header->ph.poc_msb_cycle_val;
  }
  else
  {
    if((header->ph.poc_lsb < header->ph.poc_lsb_prev) &&
       (header->ph.poc_lsb_prev - header->ph.poc_lsb >= max_poc_lsb / 2))
      header->ph.poc_msb = header->ph.poc_msb_prev + max_poc_lsb;
    else if((header->ph.poc_lsb > header->ph.poc_lsb_prev) &&
            (header->ph.poc_lsb - header->ph.poc_lsb_prev > max_poc_lsb / 2))
      header->ph.poc_msb = header->ph.poc_msb_prev - max_poc_lsb;
    else
      header->ph.poc_msb = header->ph.poc_msb_prev;
  }

  header->ph.poc = header->ph.poc_msb + header->ph.poc_lsb;
  header->poc    = header->ph.poc;
}

MP4Err vvc_parse_slice_header_minimal(BitBuffer *bb, struct vvc_slice_header *header,
                                      struct vvc_sps *sps, struct vvc_pps *pps)
{
	MP4Err err = MP4NoErr;
	u8 x;
	u8 slice_temporal_mvp_enabled_flag = 0;
	u8 slice_deblocking_filter_disabled_flag = 0;
	u32 i;
	u32 nal_unit_type;

  /* for POC compute */
  header->max_poc_lsb = 1 << (sps->sps_log2_max_pic_order_cnt_lsb_minus4 + 4);

	/* Get first header byte for nal_unit_type */
	err = GetBytes(bb, 1, &x); if(err) goto bail;
	err = GetBytes(bb, 1, &x); if(err) goto bail;

	nal_unit_type = x >> 3;
	header->nal_unit_type = nal_unit_type;

  if(nal_unit_type == VVC_NALU_SLICE_IDR_W_RADL || nal_unit_type == VVC_NALU_SLICE_IDR_N_LP ||
     nal_unit_type == VVC_NALU_SLICE_CRA)
  {
    header->slice_type = VVC_SLICE_I;
  }

  header->sh_picture_header_in_slice_header_flag = (u8)GetBits(bb, 1, &err); if(err) goto bail;
  if(header->sh_picture_header_in_slice_header_flag)
  {
    //printf("be first slice\r\n");
    header->is_first_slice = 1;
    /* Picture header in slice header incomplete support */
    return vvc_parse_ph_minimal(bb, &header->ph, sps, pps);
  }
  if(sps->sps_subpic_info_present_flag)
  {
    /* sh_subpic_id */
    header->sh_subpic_id = GetBits(bb, sps->sps_subpic_id_len_minus1 + 1, &err);
    if(err) goto bail;
  }

  if(pps->pps_rect_slice_flag)
  {
    //assert(0);
    /* tile parsing not supported */
    //Ceil( Log2 ( NumTilesInPic ) )
  }

  for(i = 0; i < sps->NumExtraShBits; i++)
  {
    x = (u8)GetBits(bb, 1, &err); if(err) goto bail;
  }
  if(header->ph.inter_slice_allowed_flag)
  {
    header->slice_type = read_golomb_uev(bb, &err); if(err) goto bail;
  }

  /* todo continue parse */

bail:
	return err;
}

s32 parseVVCNal(FILE *input, u8 **data, int *data_len)
{
  size_t startPos;
  size_t NALStart = 0;
  size_t NALEnd   = 0;
  u32 NALULen;
  u8 *NALU;

  u8 byte;
  int zerocount;
  int frame = 0;
  u8 nal_header[2];

  // Save start position
  startPos = ftell(input);

  // Search for sync
  zerocount = 0;
  while(1)
  {
    u8 byte;
    if(!fread(&byte, 1, 1, input))
    {
      return -1;
    }
    /* Search for sync */
    if(zerocount >= 2 && byte == 1)
    {
      /* Read NAL unit header */
      fread(nal_header, 2, 1, input);
      /* Include header in the data */
      fseek(input, -2, SEEK_CUR);
      break;
    }
    else if(byte == 0)
    {
      zerocount++;
    }
    else
    {
      zerocount = 0;
    }
  }
  NALStart = ftell(input);

  // Search for next sync
  zerocount = 0;
  while(1)
  {
    if(!fread(&byte, 1, 1, input))
    {
      zerocount = 0;
      break;
    }
    // Sync found
    if(zerocount >= 2 && byte == 1)
    {
      // fread(&byte, 1, 1, input);
      fseek(input, -1 - zerocount, SEEK_CUR);
      NALEnd = ftell(input);
      break;
    }
    else if(byte == 0)
    {
      zerocount++;
    }
    else
    {
      zerocount = 0;
    }
  }
  NALEnd = ftell(input);

  NALULen = NALEnd - NALStart;
  NALU    = (u8 *)malloc(NALULen);
  fseek(input, NALStart, SEEK_SET);
  if(!fread(NALU, NALULen, 1, input))
  {
    return -1;
  }

  /* Extract NAL unit type */
  byte      = nal_header[1] >> 3;
  *data_len = NALULen;
  *data     = NALU;

  return byte;
}

ISOErr analyze_vvc_stream(FILE* input, struct vvc_stream* stream) {
	ISOErr err = MP4NoErr;
	ISOHandle spsHandle = NULL;
	ISOHandle vpsHandle = NULL;
	ISOHandle ppsHandle = NULL;
  ISOHandle phHandle  = NULL;
  ISOHandle otherHandle = NULL;
	u8 frameNal = 0;
	u32 size_temp;
	u8* data = NULL;
	u8 *slicedata = NULL;
	u32 slicedatalen = 0;
	u32 datalen;
	BitBuffer bb;
	size_t startPos;
	static u8 sps_found = 0, vps_found = 0, pps_found = 0;
  u8 ph_found = 0, is_first_slice = 0, find_first_IDR_frame = 0, poc_reset = 0;

	memset(stream, 0, sizeof(struct vvc_stream));
	startPos = ftell(input);

	stream->allocated_count = 16;
	stream->header = malloc(sizeof(struct vvc_slice_header *) * stream->allocated_count);

	/* Loop whole input file */
	while (1) {
		long peek_origin = 0;
		struct vvc_slice_header *header = malloc(sizeof(struct vvc_slice_header));
		memset(header, 0, sizeof(struct vvc_slice_header));

		frameNal = 0;
		/* Parse NAL units until slice (and the next) is found */
		while (frameNal != 2) {
			s32 naltype = parseVVCNal(input, &data, &datalen);
			//printf("NAL type: %d\r\n", naltype);
			if (naltype == -1 && frameNal) {
				fseek(input, peek_origin, SEEK_SET);
				break;
			}
			if (naltype == -1) break;
			switch (naltype) {
      case VVC_NALU_VPS:
				if (vps_found) {
					free(data); data = NULL;
          continue;
				}
				vps_found = 1;
				ISODisposeHandle(vpsHandle);
				err = ISONewHandle(datalen, &vpsHandle);
				memcpy((*vpsHandle), data, datalen);
				free(data); data = NULL;
				break;
      case VVC_NALU_SPS:
				if (sps_found) {
          ISODisposeHandle(spsHandle);
          err = ISONewHandle(datalen, &spsHandle);
          memcpy((*spsHandle), data, datalen);

					header->non_VCL_data = (u8*)realloc(header->non_VCL_data, header->non_VCL_datalen + datalen + 4);
					memcpy(&header->non_VCL_data[header->non_VCL_datalen + 4], data, datalen);
					PUT32(&header->non_VCL_data[header->non_VCL_datalen], datalen);
					header->non_VCL_datalen += datalen + 4;
					free(data); data = NULL;
          continue;
				}
				sps_found = 1;
				ISODisposeHandle(spsHandle);
				err = ISONewHandle(datalen, &spsHandle);
				memcpy((*spsHandle), data, datalen);
				free(data); data = NULL;
				break;
      case VVC_NALU_PPS:
				if (pps_found) {
          ISODisposeHandle(ppsHandle);
          err = ISONewHandle(datalen, &ppsHandle);
          memcpy((*ppsHandle), data, datalen);

					header->non_VCL_data = (u8*)realloc(header->non_VCL_data, header->non_VCL_datalen + datalen + 4);
					memcpy(&header->non_VCL_data[header->non_VCL_datalen + 4], data, datalen);
					PUT32(&header->non_VCL_data[header->non_VCL_datalen], datalen);
					header->non_VCL_datalen += datalen + 4;
					free(data); data = NULL;
          continue;
				}
				pps_found = 1;
				ISODisposeHandle(ppsHandle);
				err = ISONewHandle(datalen, &ppsHandle);
				memcpy((*ppsHandle), data, datalen);
				free(data); data = NULL;
				break;
      case VVC_NALU_PIC_HEADER:
        if(ph_found)
        {
          /* There should not be multiple PHs in one frame */
          assert(0);
        }
        ph_found = 1;
        ISODisposeHandle(phHandle);
        err = ISONewHandle(datalen, &phHandle);
        memcpy((*phHandle), data, datalen);
        free(data);
        data = NULL;
        break;
      case VVC_NALU_SLICE_IDR_W_RADL:
      case VVC_NALU_SLICE_IDR_N_LP:
        find_first_IDR_frame = 1;
        poc_reset            = 1;
			default:
        /* VCL */
				if (naltype < 11) 
        {
					u32 layer = data[0];
					if (!frameNal) {
            //printf("copy slice data\r\n");
						slicedata = data;
						slicedatalen = datalen;
					}
					frameNal += !layer;
					if (layer) {
            //todo
					}
				} 
        else 
         /* Non-VCL */
        {
          /* before find the first frame, the non-VCL nalus are store in the vvcC box */
          if(find_first_IDR_frame)
          {
            header->non_VCL_data =
              (u8 *)realloc(header->non_VCL_data, header->non_VCL_datalen + datalen + 4);
            memcpy(&header->non_VCL_data[header->non_VCL_datalen + 4], data, datalen);
            PUT32(&header->non_VCL_data[header->non_VCL_datalen], datalen);
            header->non_VCL_datalen += datalen + 4;
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
    /* SPS */
		err = ISOGetHandleSize(spsHandle, &size_temp); if (err) goto bail;
		err = BitBuffer_Init(&bb, (u8*)*spsHandle, 8 * size_temp); if (err) goto bail;
		err = vvc_parse_sps_minimal(&bb, &stream->sps); if (err) goto bail;
		/* PPS */
		err = ISOGetHandleSize(ppsHandle, &size_temp); if (err) goto bail;
		err = BitBuffer_Init(&bb, (u8*)*ppsHandle, 8 * size_temp); if (err) goto bail;
		err = vvc_parse_pps_minimal(&bb, &stream->pps); if (err) goto bail;
		/* Slice header */
		err = BitBuffer_Init(&bb, slicedata, 8 * slicedatalen); if (err) goto bail;
		err = vvc_parse_slice_header_minimal(&bb, header, &stream->sps, &stream->pps); if (err) goto bail;
    /* Picture header */
    if(ph_found)
    {
      ph_found       = 0;
      is_first_slice = 1;
      err = BitBuffer_Init(&bb, slicedata, 8 * slicedatalen); if(err) goto bail;
      err = vvc_parse_ph_minimal(&bb, &header->ph, &stream->sps, &stream->pps); if(err) goto bail;
      /* the slice after the PH is the first slice of one frame */
      header->is_first_slice = 1;
      is_first_slice         = 0;
    }
    /* compute POC */
    if(header->is_first_slice)
    {
      if(poc_reset)
      {
        poc_reset = 0;
        header->ph.poc_lsb_prev = 0;
        header->ph.poc_msb_prev = 0;
      }
      else
      {
        header->ph.poc_lsb_prev = stream->header[stream->used_count - 1]->ph.poc_lsb;
        header->ph.poc_msb_prev = stream->header[stream->used_count - 1]->ph.poc_msb;
      }
      vvc_compute_poc(header);
    }

		/* Double the allocated space when depleted */
		if (stream->used_count == stream->allocated_count) {
			stream->allocated_count *= 2;
			stream->header = realloc(stream->header, stream->allocated_count * sizeof(struct vvc_slice_header *));				
		}

		stream->header[stream->used_count] = header;
		stream->used_count++;

	}

	/* Release memory */
	ISODisposeHandle(ppsHandle);
	ISODisposeHandle(vpsHandle);
	ISODisposeHandle(spsHandle);
  ISODisposeHandle(phHandle);

	free(data); data = NULL;

	/* Rewind back to where we were at the start */
	fseek(input, startPos, SEEK_SET);

bail:
	return err;
}
