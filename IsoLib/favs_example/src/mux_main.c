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
#include "hevc.h"
#include "tools.h"

MP4_EXTERN(MP4Err) ISOAddGroupDescription(MP4Media media, u32 groupType, MP4Handle description, u32* index);
MP4_EXTERN(MP4Err) ISOMapSamplestoGroup(MP4Media media, u32 groupType, u32 group_index, s32 sample_index, u32 count);
MP4_EXTERN(MP4Err) ISONewHEVCSampleDescription(MP4Track theTrack,
	MP4Handle sampleDescriptionH,
	u32 dataReferenceIndex,
	u32 length_size, MP4Handle first_sps, MP4Handle first_pps, MP4Handle first_spsext);

static ISOErr addNaluSamples(FILE* input, ISOTrack trak, ISOMedia media, u8 trackID, u8 first_sample, struct ParamStruct *parameters, struct hevc_stream *stream) {
	ISOErr err;
	static struct hevc_slice_header *header;
	static struct hevc_sps sps;
	static struct hevc_pps pps;
	static struct hevc_poc poc;
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
	
	/* On first sample */
	if (first_sample) {
		u32 i = 0;
		u32 syncLen = 0;
		memset(&poc, 0, sizeof(struct hevc_poc));
		memset(&pps, 0, sizeof(struct hevc_pps));
		memset(&sps, 0, sizeof(struct hevc_sps));
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
		naltype = parseHEVCNal(input, &data, &datalen);
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
		case 40: /* Suffix SEI */
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
			if (naltype < 32) {
				u32 layer = data[1] >> 3;
				sliceNal = !layer;
				if (layer) {
					/*printf("NAL Layer: %d\r\n", layer);*/
				}
			}
			else {
				printf("Unknown NAL: %d\r\n", naltype);
				assert(0);
				free(data); data = NULL;
			}
		}

		/* Push all slices of a sample to one chunk of data */
		if (sliceNal && cumulativeSample + frame_slices < stream->used_count - 1) {
			if (!stream->header[cumulativeSample + frame_slices + 1]->first_slice_segment_in_pic_flag) {
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
		ISONewHEVCSampleDescription(trak, sampleEntryH, 1, 1, spsHandle, ppsHandle, vpsHandle);
	}
	err = ISONewHandle(sizeof(u32), &sampleDurationH);
	*((u32*)*sampleDurationH) = parameters->frameduration;
	
	err = ISONewHandle(sizeof(u32), &sampleSizeH);
	err = ISONewHandle(sizeof(u32), &sampleOffsetH);

	/* Special case for aggregators */
	if (header->aggregator_datalen) {
		const u32 DATALEN_FIELD_LEN = 4;
		/* Construct SHVC header by using NAL header data with nal_unit_type changed */		
		u16 NALU_header = ((AGGREGATOR_NAL_TYPE << 1) << 8) | (header->aggregator_header & 0xff);
		err = ISONewHandle(datalen + header->aggregator_datalen + 10, &sampleDataH);		
		PUT32(*sampleDataH, datalen);
		memcpy((*sampleDataH) + DATALEN_FIELD_LEN, data, datalen);

		/* Append aggregator data after slice data */		
		PUT32((*sampleDataH) + datalen + DATALEN_FIELD_LEN, header->aggregator_datalen + 2);
		PUT16((*sampleDataH) + datalen + DATALEN_FIELD_LEN + DATALEN_FIELD_LEN, NALU_header);
		memcpy((*sampleDataH) + datalen + DATALEN_FIELD_LEN + DATALEN_FIELD_LEN + 2, header->aggregator_data, header->aggregator_datalen);
	} else if (frame_slices) {
		err = ISONewHandle(datalen, &sampleDataH);
		memcpy((*sampleDataH), data, datalen);
	} else {
		err = ISONewHandle(datalen + 4, &sampleDataH);
		memcpy((*sampleDataH) + 4, data, datalen);
		/* NAL units are prefixed with 4-byte length field */
		PUT32(*sampleDataH, datalen);
	}

	/*printf("POC: %d, cumulative: %d, offset: %d, sample_number: %d\n", stream->header[cumulativeSample]->poc + stream->header[cumulativeSample]->poc_offset, cumulativeSample,
		(((stream->header[cumulativeSample]->poc + stream->header[cumulativeSample]->poc_offset) - (cumulativeSample)+3)) * parameters->frameduration, stream->header[cumulativeSample]->sample_number);*/
	*(u32*)*sampleOffsetH = (((stream->header[cumulativeSample]->poc + stream->header[cumulativeSample]->poc_offset) - (stream->header[cumulativeSample]->sample_number) + 3)) * parameters->frameduration;
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
MP4_EXTERN(MP4Err) MP4AddSubSampleInformationToTrack(MP4Track theTrack, MP4GenericAtom *subs);
MP4_EXTERN(MP4Err) MP4AddSubSampleInformationEntry(MP4GenericAtom subsample, u32 sample_delta,
	u32 subsample_count, MP4Handle subsample_size_array, MP4Handle subsample_priority_array,
	MP4Handle subsample_discardable_array);
MP4_EXTERN(MP4Err) MP4SetSubSampleInformationFlags(MP4GenericAtom subsample, u32 flags);
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
	ISOSetMovieBrand(moov, MP4_FOUR_CHAR_CODE('i', 's', 'o', '6'), 0);

	for (trackID = 1; trackID < (u32)parameters->inputCount + 1; trackID++) {
	
		FILE *input = fopen(parameters->inputs[trackID - 1], "rb");
		struct hevc_stream stream;
		MP4GenericAtom subs;
		u32 i;
		s32 poc_offset = 0;
		s32 largest_poc = 0;
		u32 slices = 0;
		u32 sampleNumber = 0;
		if (!input) continue;

		err = analyze_hevc_stream(input, &stream); if (err) goto bail;
		printf("Found: %d slices\r\n", stream.used_count);

		/* Analyze POC numbers for discontinuety */
		for (frameCounter = 1; frameCounter < stream.used_count; frameCounter++) {
			if (!stream.header[frameCounter]->first_slice_segment_in_pic_flag) continue;
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

		err = MP4AddTrackToMovieIOD(trak); if (err) goto bail;
		err = ISONewTrackMedia(trak, &media, ISOVisualHandlerType, 30000, NULL); if (err) goto bail;

		/* Add sub-sample information box */
		/* TODO: only add when needed */
		if (parameters->subsample_information == 4 /* Slice based sub-sample */ 
			|| parameters->subsample_information == 2 /* Tile based sub-sample*/) {
			err = MP4AddSubSampleInformationToTrack(trak, &subs); if (err) goto bail;
			err = MP4SetSubSampleInformationFlags(subs, parameters->subsample_information ); if (err) goto bail;
		}

		err = ISOBeginMediaEdits(media); if (err) goto bail;
		/* Add each slice as a new sample */
		for (frameCounter = 0; frameCounter < stream.used_count; frameCounter++) {

			stream.header[frameCounter]->sample_number = sampleNumber;
			if (!stream.header[frameCounter]->first_slice_segment_in_pic_flag) continue;
			err = addNaluSamples(input, trak, media, trackID, frameCounter == 0, parameters, &stream); if (err) goto bail;

			/* Handle slice sub-sample information */
			if (parameters->subsample_information == 4 /* Slice based sub-sample */  && stream.header[frameCounter]->num_slices) {
				MP4Handle subsample_size_array = NULL;
				MP4Handle subsample_priority_array = NULL;
				MP4Handle subsample_discardable_array = NULL;
				MP4NewHandle(stream.header[frameCounter]->num_slices * sizeof(u32), &subsample_size_array);
				MP4NewHandle(stream.header[frameCounter]->num_slices * sizeof(u32), &subsample_priority_array);
				MP4NewHandle(stream.header[frameCounter]->num_slices * sizeof(u32), &subsample_discardable_array);
				for (i = 0; i < stream.header[frameCounter]->num_slices; i++) {
					((u32*)*subsample_size_array)[i] = stream.header[frameCounter]->slice_offsets[i];
					((u32*)*subsample_priority_array)[i] = 0;
					((u32*)*subsample_discardable_array)[i] = 0;
				}
				err = MP4AddSubSampleInformationEntry(subs, frameCounter?1:0, stream.header[frameCounter]->num_slices,
					subsample_size_array, subsample_priority_array, subsample_discardable_array); if (err) goto bail;

				MP4DisposeHandle(subsample_size_array);
				MP4DisposeHandle(subsample_priority_array);
				MP4DisposeHandle(subsample_discardable_array);
			}

			/* Handle tile sub-sample information */
			if (parameters->subsample_information == 2 /* Tile based sub-sample*/ && stream.header[frameCounter]->num_entry_point_offsets) {
				MP4Handle subsample_size_array = NULL;
				MP4Handle subsample_priority_array = NULL;
				MP4Handle subsample_discardable_array = NULL;
				MP4NewHandle(stream.header[frameCounter]->num_entry_point_offsets * sizeof(u32), &subsample_size_array);
				MP4NewHandle(stream.header[frameCounter]->num_entry_point_offsets * sizeof(u32), &subsample_priority_array);
				MP4NewHandle(stream.header[frameCounter]->num_entry_point_offsets * sizeof(u32), &subsample_discardable_array);
				for (i = 0; i < stream.header[frameCounter]->num_entry_point_offsets; i++) {
					((u32*)*subsample_size_array)[i] = stream.header[frameCounter]->entry_point_offset_minus1[i] + 1;
					((u32*)*subsample_priority_array)[i] = 0;
					((u32*)*subsample_discardable_array)[i] = 0;
				}
				err = MP4AddSubSampleInformationEntry(subs, frameCounter ? 1 : 0, stream.header[frameCounter]->num_entry_point_offsets,
					subsample_size_array, subsample_priority_array, subsample_discardable_array); if (err) goto bail;

				MP4DisposeHandle(subsample_size_array);
				MP4DisposeHandle(subsample_priority_array);
				MP4DisposeHandle(subsample_discardable_array);
			}
			sampleNumber++;
		}
		err = ISOGetMediaDuration(media, &mediaDuration); if (err) goto bail;
		err = ISOEndMediaEdits(media);  if (err) goto bail;/* Calculate duration */

		/* Create "rap " sample description and group */
		ISOAddGroupDescription(media, MP4_FOUR_CHAR_CODE('r', 'a', 'p', ' '), rap_desc, &rap_desc_index);
		for (frameCounter = 1; frameCounter < stream.used_count; frameCounter++) {
			/* Mark RAP frames (CRA/BLA/IDR/IRAP) to the group */
			if (stream.header[frameCounter]->first_slice_segment_in_pic_flag &&
				  stream.header[frameCounter]->nal_type >= 16 && stream.header[frameCounter]->nal_type <= 23) {
					ISOMapSamplestoGroup(media, MP4_FOUR_CHAR_CODE('r', 'a', 'p', ' '), rap_desc_index, stream.header[frameCounter]->sample_number, 1);
			}
		}

		/* Create an alternative startup sequence */
		{
			u32 start_slice = 0;
			u32 leading_pic_count = 0;      
			/* Map RASL picture count from the headers, search second RAP */
			for (frameCounter = 1; frameCounter < stream.used_count; frameCounter++) {
				if (!stream.header[frameCounter]->first_slice_segment_in_pic_flag) continue;
				if (stream.header[frameCounter]->nal_type >= 16 && stream.header[frameCounter]->nal_type <= 23) {
						start_slice = frameCounter;
						break;
				}
			}
			frameCounter++;
			/* Count RASL slices following */
			for (; frameCounter < stream.used_count; frameCounter++) {
				if (!stream.header[frameCounter]->first_slice_segment_in_pic_flag) continue;
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
					if (!stream.header[frameCounter]->first_slice_segment_in_pic_flag) continue;
					/* Mark RAP before RASL pics and one picture after them to this group */
					/* This enabled alternative startup sequence to use only the RAP and the picture after RASL in decoding */
					/* and skip all the RASL pictures */
					if (((stream.header[frameCounter]->nal_type < 6 || stream.header[frameCounter]->nal_type > 9) &&
							(stream.header[frameCounter + 1]->nal_type >= 6 && stream.header[frameCounter + 1]->nal_type <= 9))
							||
							((stream.header[frameCounter - 1]->nal_type >= 6 && stream.header[frameCounter - 1]->nal_type <= 9) &&
							(stream.header[frameCounter]->nal_type < 6 || stream.header[frameCounter]->nal_type > 9))
							) {
						ISOMapSamplestoGroup(media, MP4_FOUR_CHAR_CODE('a', 'l', 's', 't'), alst_desc_index, stream.header[frameCounter]->sample_number, 1);
					}
				}
				MP4DisposeHandle(alst_temp);
				MP4DisposeHandle(alst_desc);
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
	parseInput(argc, argv, &parameters);
	parameters.frameduration = (u32)((30000.0 / parameters.framerate)+0.5);

	/* We need inputs */
	if (!parameters.inputCount) {
		fprintf(stderr, "Usage: hevc_muxer -i <inputFile> -g <TrackID>:<GroupID> -o <outputFile> -f <framerate(float)>\r\n");
		fprintf(stderr, "            --input, -i <filename>: Input file, can be multiple\r\n");
		fprintf(stderr, "            -g <TrackID>:<GroupID> :Add track group, can be multiple\r\n");
		fprintf(stderr, "            --output, -o <filename> Output file (MP4)\r\n");
		fprintf(stderr, "            --framerate, -f <framerate> set framerate, default 30\r\n");
		fprintf(stderr, "            --subs <type> enable subsample information, 4 = slice, 2 = tile\r\n");
		exit(1);
	}

	printf("Using framerate: %.2f\r\n", parameters.framerate);

	createMyMovie(&parameters);
	
	cleanParameters(&parameters);

	return 1;
}