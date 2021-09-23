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
#include <assert.h>
#include <string.h>
#include "ISOMovies.h"
#include "vvc_structures.h"
#include "vvc.h"
#include "vvc_tools.h"

MP4_EXTERN(MP4Err) ISOAddGroupDescription(MP4Media media, u32 groupType, MP4Handle description, u32* index);
MP4_EXTERN(MP4Err) ISOMapSamplestoGroup(MP4Media media, u32 groupType, u32 group_index, s32 sample_index, u32 count );
MP4_EXTERN(MP4Err) ISONewVVCSampleDescription(MP4Track theTrack,
	MP4Handle sampleDescriptionH,
	u32 dataReferenceIndex,
	u32 length_size, MP4Handle first_sps, MP4Handle first_pps);
MP4_EXTERN(ISOErr)
ISOAddVVCSampleDescriptionPS(MP4Handle sampleEntryH, MP4Handle ps, u32 where);

static ISOErr addNaluSamples(FILE* input, ISOTrack trak, ISOMedia media, u8 trackID, u8 first_sample, struct ParamStruct *parameters, struct vvc_stream *stream) {
	ISOErr err;
	static struct vvc_slice_header *header;
	static struct vvc_sps sps;
	static struct vvc_pps pps;
	u8 sliceNal = 0;
	u32 size_temp;
	u32 slice_len[64];
	static u32 cumulativeOffset = 0;
	static u32 cumulativeSample = 0;
	ISOHandle sampleEntryH;
	ISOHandle sampleDataH;
	ISOHandle sampleDurationH;
	ISOHandle sampleSizeH;
	ISOHandle sampleOffsetH;
	ISOHandle syncSampleH = NULL;
	u8* data = NULL;
	u8* slice_data = NULL;
	static ISOHandle spsHandle;
	static ISOHandle vpsHandle;
	static ISOHandle ppsHandle;
	static u8 sps_found = 0, vps_found = 0, pps_found = 0;
	BitBuffer bb;
	u32 naltype;
	u32 frame_slices = 0;

	u32 datalen;
	u32 slice_datalen = 0;
	
	u32 sei_count = 0, aps_count = 0;
  u32 i;
  s32 j;
  ISOHandle opiHandle = NULL;
  ISOHandle dciHandle = NULL;
  ISOHandle* prefixApsHandle  = NULL;
  ISOHandle* prefixSeiHandle = NULL;

	/* On first sample */
	if (first_sample) {
		u32 i = 0;
		u32 syncLen = 0;
		memset(&pps, 0, sizeof(struct vvc_pps));
		memset(&sps, 0, sizeof(struct vvc_sps));
		cumulativeSample = 0;
		err = ISONewHandle(sizeof(u32) * 2, &syncSampleH);
		((u32*)*syncSampleH)[0] = 1;
		((u32*)*syncSampleH)[1] = 1;
		sps_found = vps_found = pps_found = 0;
	}

	err = ISOSetMediaLanguage(media, "und"); /* undetermined */
	if (err) return err;
	err = ISONewHandle(1, &sampleEntryH);

	/* Parse NAL units until slice is found */
	while (!sliceNal) {
		naltype = parseVVCNal(input, &data, &datalen);
		switch (naltype) {
    case VVC_NALU_OPI:
      err = ISONewHandle(datalen, &opiHandle);
      memcpy((*opiHandle), data, datalen);
      free(data); data = NULL;
      break;
    case VVC_NALU_DCI:
      err = ISONewHandle(datalen, &dciHandle);
      memcpy((*dciHandle), data, datalen);
      free(data); data = NULL;
      break;
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
				free(data); data = NULL;
        continue;
			}
			pps_found = 1;
			ISODisposeHandle(ppsHandle);
			err = ISONewHandle(datalen, &ppsHandle);
			memcpy((*ppsHandle), data, datalen);
			free(data); data = NULL;
			break;
    case VVC_NALU_PREFIX_APS:
      if(first_sample)
      {
        prefixApsHandle = (ISOHandle *)realloc(prefixApsHandle, sizeof(ISOHandle) * (aps_count + 1));
        err = ISONewHandle(datalen, &prefixApsHandle[aps_count]);
        memcpy((*prefixApsHandle[aps_count]), data, datalen);       
        aps_count += 1;
      }
			free(data); data = NULL;
      break;
    case VVC_NALU_PIC_HEADER:
      break;
    case VVC_NALU_PREFIX_SEI:
      if(first_sample)
      {
        prefixSeiHandle = (ISOHandle *)realloc(prefixSeiHandle, sizeof(ISOHandle) * (sei_count + 1));
        err = ISONewHandle(datalen, &prefixSeiHandle[sei_count]);
        memcpy((*prefixSeiHandle[sei_count]), data, datalen);
        sei_count += 1;
      }
			free(data); data = NULL;
      break;
    case VVC_NALU_SUFFIX_SEI:
			{
				u32 sei_type = 0;
				u32 datapos = 2;
				u32 sei_size = 0;
				while (data[datapos] == 0xff && datapos < datalen) {
					sei_type += 255;
					datapos++;
				}
				sei_type += data[datapos];
				datapos++;

				while (data[datapos] == 0xff && datapos < datalen) {
					sei_size += 255;
					datapos++;
				}
				sei_size += data[datapos];
				datapos++;
			}
			free(data); data = NULL;
			break;

		default:
			if (naltype < 11) {
				u32 layer = data[0];
				sliceNal = !layer;
				if (layer) {
					printf("NAL Layer: %d\r\n", layer);
				}
			}
			else {
				printf("Unknown NAL: %d\r\n", naltype);
				//assert(0);
				free(data); data = NULL;
			}
		}

		///* Push all slices of a sample to one chunk of data */
		if (sliceNal && cumulativeSample + frame_slices < stream->used_count - 1) {
      if(!stream->header[cumulativeSample + frame_slices + 1]->is_first_slice)
      {
				sliceNal = 0;
				slice_len[frame_slices] = datalen + 4;
				frame_slices++;
				slice_data = realloc(slice_data, slice_datalen + datalen + 4);
				memcpy(&slice_data[slice_datalen+4], data, datalen);
				PUT32(&slice_data[slice_datalen], datalen);
				slice_datalen += datalen + 4;
				free(data); data = NULL;
			}
		}
	}
	/* Special case for the last slice of the frame */
	if (frame_slices) {
		u32 i;
		slice_len[frame_slices] = datalen + 4;
		slice_data = realloc(slice_data, slice_datalen + datalen+4);
		memcpy(&slice_data[slice_datalen+4], data, datalen);
		PUT32(&slice_data[slice_datalen], datalen);
		slice_datalen += datalen + 4;
		free(data);
		data = slice_data;
		datalen = slice_datalen;

		/* Store slice data offsets to be used in sub-sample information box construction */
		stream->header[cumulativeSample]->num_slices = frame_slices + 1;
		stream->header[cumulativeSample]->slice_offsets = (u32*)malloc(stream->header[cumulativeSample]->num_slices * sizeof(u32));
		for (i = 0; i < frame_slices + 1; i++) {
			stream->header[cumulativeSample]->slice_offsets[i] = slice_len[i];
		}
	}

  /* Header data already parsed in analysis */
	header = stream->header[cumulativeSample];

	if (first_sample) {
		ISONewVVCSampleDescription(trak, sampleEntryH, 1, 4, spsHandle, ppsHandle);
    if(vps_found)
    {
      ISOAddVVCSampleDescriptionPS(sampleEntryH, vpsHandle, VVC_NALU_VPS);
		}
    if(opiHandle)
    {
      ISOAddVVCSampleDescriptionPS(sampleEntryH, opiHandle, VVC_NALU_OPI);
      err = ISODisposeHandle(opiHandle);	
		}
    if(dciHandle)
    {
      ISOAddVVCSampleDescriptionPS(sampleEntryH, dciHandle, VVC_NALU_DCI);
      err = ISODisposeHandle(dciHandle);
    }
    for(i = 0; i < aps_count; i++)
    {
      ISOAddVVCSampleDescriptionPS(sampleEntryH, prefixApsHandle[i], VVC_NALU_PREFIX_APS);
    }	
    for(i = 0; i < sei_count; i++)
    {
      ISOAddVVCSampleDescriptionPS(sampleEntryH, prefixSeiHandle[i], VVC_NALU_PREFIX_SEI);
		}	
		for(j = aps_count - 1; j >= 0; j--)
    {
      err = ISODisposeHandle(prefixApsHandle[j]);
		}
    for(j = sei_count - 1; j >= 0; j--)
    {
      err = ISODisposeHandle(prefixSeiHandle[j]);
		}
	}

	err = ISONewHandle(sizeof(u32), &sampleDurationH);
	*((u32*)*sampleDurationH) = parameters->frameduration;
	
	err = ISONewHandle(sizeof(u32), &sampleSizeH);
	err = ISONewHandle(sizeof(u32), &sampleOffsetH);

	if (header->non_VCL_datalen) {
		const u32 DATALEN_FIELD_LEN = 4;
		err = ISONewHandle(datalen + header->non_VCL_datalen + 4, &sampleDataH);		
		PUT32(*sampleDataH, datalen);
		memcpy((*sampleDataH) + DATALEN_FIELD_LEN, data, datalen);
		memcpy((*sampleDataH) + datalen + DATALEN_FIELD_LEN, header->non_VCL_data, header->non_VCL_datalen);
	} else if (frame_slices) {
		err = ISONewHandle(datalen, &sampleDataH);
		memcpy((*sampleDataH), data, datalen);
	} else {
		err = ISONewHandle(datalen + 4, &sampleDataH);
		memcpy((*sampleDataH) + 4, data, datalen);
		/* NAL units are prefixed with 4-byte length field */
		PUT32(*sampleDataH, datalen);
	}

	printf("POC: %d, cumulative: %d, offset: %d, sample_number: %d\n", stream->header[cumulativeSample]->poc + stream->header[cumulativeSample]->poc_offset, cumulativeSample,
		(((stream->header[cumulativeSample]->poc + stream->header[cumulativeSample]->poc_offset) - (cumulativeSample)+3)) * parameters->frameduration, stream->header[cumulativeSample]->sample_number);

	*(u32*)*sampleOffsetH = (((stream->header[cumulativeSample]->poc + stream->header[cumulativeSample]->poc_offset) - (stream->header[cumulativeSample]->sample_number) + 3)) * parameters->frameduration;
	cumulativeOffset += datalen;
	
	err = ISOGetHandleSize(sampleDataH,(u32*)*sampleSizeH);
	//'mdia'/'stbl'
	err = MP4AddMediaSamples(media, sampleDataH, 1,
                            sampleDurationH,
                            sampleSizeH,
                            (first_sample ? sampleEntryH : NULL), sampleOffsetH, syncSampleH);
	if (sampleEntryH) {
		err = ISODisposeHandle(sampleEntryH);
		sampleEntryH = NULL;
	}
	
	cumulativeSample += 1 + frame_slices;

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

static MP4Err creatSporBox(MP4Media media, struct vvc_stream stream, struct ParamStruct *parameters)
{
  MP4Err err = MP4NoErr;
  spor_boxPtr spor;
  MP4Handle spor_desc = NULL;
  MP4Handle spor_temp;
  u32 sporSize      = 0;
  u32 spor_data_idx = 0;
  u32 i, spor_data, spor_idx;

  MP4NewHandle(sizeof(spor_box), &spor_temp);
  spor                      = (spor_boxPtr)*spor_temp;
  spor->subpic_id_info_flag = 1;
  spor->num_subpic_ref_idx  = 1;
  spor->subp_track_ref_idx  = (u16 *)malloc(sizeof(u16) * spor->num_subpic_ref_idx);
  for(i = 0; i < spor->num_subpic_ref_idx; i++)
  {
    spor->subp_track_ref_idx[i] = i;
  }
  if(spor->subpic_id_info_flag)
  {
    spor->subpic_id_len_minus1   = 0;
    spor->subpic_id_bit_pos      = 0;
    spor->start_code_emul_flag   = 0;
    spor->pps_sps_subpic_id_flag = 0;
    spor->sps_id                 = 0;
    spor->pps_id                 = 0;
  }

  /* caculate size */
  sporSize += 2;
  if(spor->num_subpic_ref_idx) sporSize += 2 * spor->num_subpic_ref_idx;
  if(spor->subpic_id_info_flag) sporSize += 3;

  MP4NewHandle(sporSize, &spor_desc);
  spor_data = ((spor->subpic_id_info_flag & 0x01) << 15) | (spor->num_subpic_ref_idx & 0x7fff);
  PUT16(&(*spor_desc)[spor_data_idx], spor_data);
  spor_data_idx += 2;
  for(i = 0; i < spor->num_subpic_ref_idx; i++)
  {
    spor_data = spor->subp_track_ref_idx[i];
    PUT16(&(*spor_desc)[spor_data_idx], spor_data);
    spor_data_idx += 2;
  }
  if(spor->subpic_id_info_flag)
  {
    spor_data = ((spor->subpic_id_len_minus1 & 0x0f) << 12) | (spor->subpic_id_bit_pos & 0x0fff);
    PUT16(&(*spor_desc)[spor_data_idx], spor_data);
    spor_data_idx += 2;
    if(spor->pps_sps_subpic_id_flag)
    {
      spor_data = (u8)((spor->start_code_emul_flag & 0x01) << 7) |
                  ((spor->pps_sps_subpic_id_flag & 0x01) << 6) | (spor->pps_id & 0x3f);
    }
    else
    {
      spor_data = (u8)((spor->start_code_emul_flag & 0x01) << 7) |
                  ((spor->pps_sps_subpic_id_flag & 0x01) << 6) | ((spor->sps_id & 0x3f) << 2);
    }
    PUT8(&(*spor_desc)[spor_data_idx], spor_data);
    spor_data_idx += 1;
  }
  assert(spor_data_idx == sporSize);
  /* sgpd */
  err = ISOAddGroupDescription(media, MP4_FOUR_CHAR_CODE('s', 'p', 'o', 'r'), spor_desc, &spor_idx);
  if(err) goto bail;
  /* sbgp */
  err = ISOMapSamplestoGroup(media, MP4_FOUR_CHAR_CODE('s', 'p', 'o', 'r'), spor_idx, 0, 1);
  if(err) goto bail;

  free(spor->subp_track_ref_idx);
  MP4DisposeHandle(spor_temp);
  MP4DisposeHandle(spor_desc);
bail:
  return err;
}

static MP4Err creatRrorBox(MP4Media media, struct vvc_stream stream, struct ParamStruct *parameters)
{
  MP4Err err = MP4NoErr;
  rror_boxPtr rror;
  MP4Handle rror_desc = NULL;
  MP4Handle rror_temp;
  u32 rrorSize      = 0;
  u32 rror_data_idx = 0;
  u32 i, j, rror_data, rror_idx;
  u32 group_count = 0;
  static u32 group_idx = 0;

  MP4NewHandle(sizeof(rror_box), &rror_temp);
  rror                      = (rror_boxPtr)*rror_temp;
  rror->subpic_id_info_flag = 0;
  rror->num_alternate_region_set = 1; //parameters->trackGroupCount?
  rror->num_regions_in_set = (u16 *)malloc(sizeof(u16) * rror->num_alternate_region_set);
  rror->alternate_region_set_id = (u16 *)malloc(sizeof(u16) * rror->num_alternate_region_set);
  rror->groupID                 = (u16 **)malloc(sizeof(u16*) * rror->num_alternate_region_set);

  for(i = 0; i < rror->num_alternate_region_set; i++)
  {
    rror->num_regions_in_set[i] = parameters->inputCount;
    rror->alternate_region_set_id[i] = i;
    rror->groupID[i] = (u16 *)malloc(sizeof(u16) * rror->num_regions_in_set[i]);
    for(j = 0; j < rror->num_regions_in_set[i]; j++)
    {
      rror->groupID[i][j] = group_count;
      group_count += 1;
    }
  }
	rror->num_regions_minus1 = 0;
  rror->region_id          = (u16 *)malloc(sizeof(u16) * rror->num_regions_minus1);
  for(i = 0; i < rror->num_regions_minus1; i++)
  {
    rror->region_id[i] = i;
  }
  if(rror->subpic_id_info_flag)
  {
    rror->subpic_id_len_minus1   = 0;
    rror->subpic_id_bit_pos      = 0;
    rror->start_code_emul_flag   = 0;
    rror->pps_sps_subpic_id_flag = 0;
    rror->sps_id                 = 0;
    rror->pps_id                 = 0;
  }

  /* caculate size */
  rrorSize += 3;
  for(i = 0; i < rror->num_alternate_region_set; i++)
  {
    rrorSize += 4;
    for(j = 0; j < rror->num_regions_in_set[i]; j++)
    {
      rrorSize += 2;
    }
  }
  rrorSize += 2;
  for(i = 0; i < rror->num_regions_minus1; i++)
  {
    rrorSize += 2;
  }
  if(rror->subpic_id_info_flag) rrorSize += 3;

  /* write bit */
  MP4NewHandle(rrorSize, &rror_desc);
  rror_data = (rror->subpic_id_info_flag & 0x01) << 7;
  PUT8(&(*rror_desc)[rror_data_idx], rror_data);
  rror_data_idx += 1;
  PUT16(&(*rror_desc)[rror_data_idx], rror->num_alternate_region_set);
  rror_data_idx += 2;
  for(i = 0; i < rror->num_alternate_region_set; i++)
  {
    PUT16(&(*rror_desc)[rror_data_idx], rror->num_regions_in_set[i]);
    rror_data_idx += 2;
    PUT16(&(*rror_desc)[rror_data_idx], rror->alternate_region_set_id[i]);
    rror_data_idx += 2;
    for(j = 0; j < rror->num_regions_in_set[i]; j++)
    {
      PUT16(&(*rror_desc)[rror_data_idx], rror->groupID[i][j]);
      rror_data_idx += 2;
    }
  }
  PUT16(&(*rror_desc)[rror_data_idx], rror->num_regions_minus1);
  rror_data_idx += 2;
  for(i = 0; i < rror->num_regions_minus1; i++)
  {
    PUT16(&(*rror_desc)[rror_data_idx], rror->region_id[i]);
    rror_data_idx += 2;
  }
  if(rror->subpic_id_info_flag)
  {
    rror_data = ((rror->subpic_id_len_minus1 & 0x0f) << 12) | (rror->subpic_id_bit_pos & 0x0fff);
    PUT16(&(*rror_desc)[rror_data_idx], rror_data);
    rror_data_idx += 2;
    if(rror->pps_sps_subpic_id_flag)
    {
      rror_data = (u8)((rror->start_code_emul_flag & 0x01) << 7) |
                  ((rror->pps_sps_subpic_id_flag & 0x01) << 6) | (rror->pps_id & 0x3f);
    }
    else
    {
      rror_data = (u8)((rror->start_code_emul_flag & 0x01) << 7) |
                  ((rror->pps_sps_subpic_id_flag & 0x01) << 6) | ((rror->sps_id & 0x3f) << 2);
    }
    PUT8(&(*rror_desc)[rror_data_idx], rror_data);
    rror_data_idx += 1;
  }
  assert(rror_data_idx == rrorSize);

  /* sgpd */
  err = ISOAddGroupDescription(media, MP4_FOUR_CHAR_CODE('r', 'r', 'o', 'r'), rror_desc, &rror_idx); if(err) goto bail;
  /* sbgp */
  err = ISOMapSamplestoGroup(media, MP4_FOUR_CHAR_CODE('r', 'r', 'o', 'r'), group_idx, 0, stream.used_count); if(err) goto bail;
  group_idx += 1;
  free(rror->num_regions_in_set);
  free(rror->alternate_region_set_id);
  free(rror->region_id);
  free(rror->groupID);

  MP4DisposeHandle(rror_temp);
  MP4DisposeHandle(rror_desc);
bail:
  return err;
}

static MP4Err creatTrifBox(MP4Media media, struct vvc_stream stream)
{
  MP4Err err          = MP4NoErr;
	MP4Handle trif_desc = NULL;
  trif_boxPtr trif;
  MP4Handle trif_temp;
  u32 trifSize      = 0;
  u32 trif_data_idx = 0;
  u32 i, trif_data, trif_idx;
  /* Define spor parameters */
  MP4NewHandle(sizeof(trif_box), &trif_temp);
  trif                      = (trif_boxPtr)*trif_temp;
  trif->groupID             = 0;
  trif->rect_region_flag    = 1;
  trif->independent_idc     = 1;
  trif->full_picture        = 0;
  trif->filtering_disabled  = 0;
  trif->has_dependency_list = 0;
  if(!trif->full_picture)
  {
    trif->horizontal_offset = 0;
    trif->vertical_offset   = 0;
  }
  trif->region_width  = stream.sps.sps_pic_width_max_in_luma_samples;
  trif->region_height = stream.sps.sps_pic_height_max_in_luma_samples;
  if(trif->has_dependency_list)
  {
    trif->dependency_rect_region_count = 0;
    trif->dependencyRectRegionGroupID =
      (u16 *)malloc(sizeof(u16) * trif->dependency_rect_region_count);
    for(i = 1; i <= trif->dependency_rect_region_count; i++)
    {
      trif->dependencyRectRegionGroupID[i - 1] = 0;
    }
  }
  /* caculate size */
  trifSize += 3;
  if(trif->rect_region_flag)
  {
    if(!trif->full_picture)
    {
      trifSize += 4;
    }
    trifSize += 4;
    if(trif->has_dependency_list)
    {
      trifSize += 2 + 2 * trif->dependency_rect_region_count;
    }
  }

  MP4NewHandle(trifSize, &trif_desc);
  PUT16(&(*trif_desc)[trif_data_idx], trif->groupID);
  trif_data_idx += 2;
  if(!trif->rect_region_flag)
  {
    trif_data = (u8)(trif->rect_region_flag << 7);
    PUT8(&(*trif_desc)[trif_data_idx], trif_data);
    trif_data_idx += 1;
  }
  else
  {
    trif_data = (u8)(trif->rect_region_flag << 7) | ((trif->independent_idc & 0x03) << 5) |
                (trif->full_picture << 4) | (trif->filtering_disabled << 3) |
                (trif->has_dependency_list << 2);
    PUT8(&(*trif_desc)[trif_data_idx], trif_data);
    trif_data_idx += 1;
    if(!trif->full_picture)
    {
      PUT16(&(*trif_desc)[trif_data_idx], trif->horizontal_offset);
      trif_data_idx += 2;
      PUT16(&(*trif_desc)[trif_data_idx], trif->vertical_offset);
      trif_data_idx += 2;
    }
    PUT16(&(*trif_desc)[trif_data_idx], trif->region_width);
    trif_data_idx += 2;
    PUT16(&(*trif_desc)[trif_data_idx], trif->region_height);
    trif_data_idx += 2;
    if(trif->has_dependency_list)
    {
      PUT16(&(*trif_desc)[trif_data_idx], trif->dependency_rect_region_count);
      trif_data_idx += 2;
      for(i = 1; i <= trif->dependency_rect_region_count; i++)
      {
        PUT16(&(*trif_desc)[trif_data_idx], trif->dependencyRectRegionGroupID[i - 1]);
        trif_data_idx += 2;
      }
    }
  }
  assert(trifSize == trif_data_idx);
  /* sgpd */
  err = ISOAddGroupDescription(media, MP4_FOUR_CHAR_CODE('t', 'r', 'i', 'f'), trif_desc, &trif_idx);
  if(err) goto bail;
  /* sbgp */
  err = ISOMapSamplestoGroup(media, MP4_FOUR_CHAR_CODE('t', 'r', 'i', 'f'), trif_idx, -1, 1);
  if(err) goto bail;

  free(trif->dependencyRectRegionGroupID);
  MP4DisposeHandle(trif_temp);
  MP4DisposeHandle(trif_desc);
bail:
  return err;
}

static MP4Err creatSulmBox(MP4Media media) {
	MP4Err err = MP4NoErr;
  MP4Handle sulm_desc = NULL;
  sulm_boxPtr sulm;
  MP4Handle sulm_temp;
  u32 sulmSize      = 0;
  u32 sulm_data_idx = 0;
  u32 i,sulm_idx;
  /* Define sulm parameters */
  MP4NewHandle(sizeof(sulm_box), &sulm_temp);
  sulm                     = (trif_boxPtr)*sulm_temp;
  sulm->groupID_info_4cc   = MP4_FOUR_CHAR_CODE('t', 'r', 'i', 'f');
  sulm->entry_count_minus1 = 3; // num of subpicture
  sulm->groupID            = (u16 *)malloc(sizeof(u16) * (sulm->entry_count_minus1 + 1));
  for(i = 0; i <= sulm->entry_count_minus1; i++)
  {
    sulm->groupID[i] = i + 1;
  }
  /* caculate size */
  sulmSize += 6;
  sulmSize += 2 * (sulm->entry_count_minus1 + 1);

  MP4NewHandle(sulmSize, &sulm_desc);
  PUT32(&(*sulm_desc)[sulm_data_idx], sulm->groupID_info_4cc);
  sulm_data_idx += 4;
  PUT16(&(*sulm_desc)[sulm_data_idx], sulm->entry_count_minus1);
  sulm_data_idx += 2;
  for(i = 0; i <= sulm->entry_count_minus1; i++)
  {
    PUT16(&(*sulm_desc)[sulm_data_idx], sulm->groupID[i]);
    sulm_data_idx += 2;
  }
  assert(sulmSize == sulm_data_idx);
  /* sgpd */
  err = ISOAddGroupDescription(media, MP4_FOUR_CHAR_CODE('s', 'u', 'l', 'm'), sulm_desc, &sulm_idx);
  if(err) goto bail;
  /* sbgp */
  err = ISOMapSamplestoGroup(media, MP4_FOUR_CHAR_CODE('s', 'u', 'l', 'm'), sulm_idx, 1, 1);
  if(err) goto bail;

  free(sulm->groupID);
  MP4DisposeHandle(sulm_temp);
  MP4DisposeHandle(sulm_desc);
bail:
  return err;
}

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
  initialObjectDescriptorID = 0;
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

	ISOSetMovieBrand(moov, MP4_FOUR_CHAR_CODE('m', 'p', '4', '1'), 0);
	ISOSetMovieCompatibleBrand(moov, MP4_FOUR_CHAR_CODE('v', 'v', 'c', '1'));
  ISOSetMovieCompatibleBrand(moov, MP4_FOUR_CHAR_CODE('i', 's', 'o', 'm'));
  ISOSetMovieCompatibleBrand(moov, MP4_FOUR_CHAR_CODE('i', 's', 'o', '8'));

	for (trackID = 1; trackID < (u32)parameters->inputCount + 1; trackID++) {
	
		FILE *input = fopen(parameters->inputs[trackID - 1], "rb");
		struct vvc_stream stream;
		MP4GenericAtom subs;
		u32 i;
		s32 poc_offset = 0;
		s32 largest_poc = 0;
		u32 slices = 0;
		u32 sampleNumber = 0;
		if (!input) continue;

		err = analyze_vvc_stream(input, &stream); if (err) goto bail;
		printf("Found: %d slices\r\n", stream.used_count);

		/* Analyze POC numbers for discontinuety */
		for (frameCounter = 1; frameCounter < stream.used_count; frameCounter++) {
      if(!stream.header[frameCounter]->is_first_slice) continue;
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

    // merge base track reference: 'tref' -> 'subp' only add this box in the fitst main track
		//		correspond to the sample grop 'spor' 
		// extraction base track reference: 'tref' -> 'recr' add these box in all the track
		//		correspond to the sample grop 'rror' 
    // using MP4AddTrackReferenceWithID();
    //u32 refid;
    //for(i = 1; i <= parameters->inputCount; i++)
    //{
    //  err = MP4AddTrackReferenceWithID(trak, i, MP4_FOUR_CHAR_CODE('r', 'e', 'c', 'r'), &refid);
    //}

		/* mdia */
		err = ISONewTrackMedia(trak, &media, ISOVisualHandlerType, 30000, NULL); if (err) goto bail;

		err = ISOBeginMediaEdits(media); if (err) goto bail;
		/* Add each slice as a new sample */
		for (frameCounter = 0; frameCounter < stream.used_count; frameCounter++) {

			stream.header[frameCounter]->sample_number = sampleNumber;
      if(!stream.header[frameCounter]->is_first_slice) continue;
			err = addNaluSamples(input, trak, media, trackID, frameCounter == 0, parameters, &stream); if (err) goto bail;
			sampleNumber++;
		}
		err = ISOGetMediaDuration(media, &mediaDuration); if (err) goto bail;
		err = ISOEndMediaEdits(media);  if (err) goto bail;/* Calculate duration */
	
		//err = creatSporBox(media, stream, parameters); if(err) goto bail;
		//err = creatTrifBox(media, stream); if(err) goto bail;
		//err = creatSulmBox(media); if(err) goto bail;
    //err = creatRrorBox(media, stream, parameters); if(err) goto bail;
	
		for (i = 0; i < stream.used_count; i++) {
			free(stream.header[i]); stream.header[i] = NULL;
		}
		free(stream.header);
		stream.header = NULL;
	}

  /* sub::trackgroup 'trgr' -> 'alte' (trackid, track_group_id) */
	//for (trackGroup = 0; trackGroup < (u32)parameters->trackGroupCount; trackGroup++) {
	//	err = ISOGetMovieIndTrack(moov, parameters->trackGroups[trackGroup]->track, &trak); if (err) goto bail;
	//	err =  MP4AddTrackGroup(trak, parameters->trackGroups[trackGroup]->track_group_id, MP4_FOUR_CHAR_CODE('a', 'l', 't', 'e'));
	//	if (err) goto bail;
	//}

	err = ISOWriteMovieToFile(moov, filename); if (err) goto bail;
	if (alst_desc) ISODisposeHandle(alst_desc);
	err = ISODisposeMovie(moov); if (err) goto bail;
bail:
	return err;
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
	/* Set default parameters */
	parameters.framerate = 30.0;
	parameters.compactSampleToGroup = 0;
	parseInput(argc, argv, &parameters);
	parameters.frameduration = (u32)((30000.0 / parameters.framerate)+0.5);

	/* We need inputs */
	if (!parameters.inputCount) {
		fprintf(stderr, "Usage:\r\n");
		fprintf(stderr, "vvc_muxer -i <inputFile> -g <TrackID>:<GroupID> -o <outputFile> -f <framerate(float)>\r\n");
		fprintf(stderr, "    --input, -i <filename>: Input file, can be multiple\r\n");
		fprintf(stderr, "    -g <TrackID>:<GroupID> :Add track group, can be multiple\r\n");
		fprintf(stderr, "    --output, -o <filename> Output file (MP4)\r\n");
		fprintf(stderr, "    --framerate, -f <framerate> set framerate, default 30\r\n");
		exit(1);
	}

	printf("Using framerate: %.2f\r\n", parameters.framerate);
	if(parameters.compactSampleToGroup) printf("Using compact sample to group\r\n");

	createMyMovie(&parameters);
	
	cleanParameters(&parameters);

	return 1;
}
