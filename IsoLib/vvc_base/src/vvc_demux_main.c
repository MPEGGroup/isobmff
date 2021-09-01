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
#include <string.h>
#include "ISOMovies.h"
#include "vvc_structures.h"
#include "vvc.h"
#include "vvc_tools.h"

MP4_EXTERN(MP4Err) ISOGetHEVCSampleDescriptionPS(MP4Handle sampleEntryH, MP4Handle ps, u32 where, u32 index);
MP4_EXTERN(MP4Err) ISOGetGroupDescription(MP4Media media, u32 groupType, u32 index, MP4Handle description);
MP4_EXTERN(MP4Err) MP4MediaTimeToSampleNum(MP4Media theMedia, u64 mediaTime, u32 *outSampleNum, 
                                           u64 *outSampleCTS, u64 *outSampleDTS, s32 *outSampleDuration);
MP4_EXTERN(MP4Err) MP4GetSubSampleInformationEntryFromTrack(MP4Track theTrack, u32* flags, u32 *entry_count, u32 **sample_delta,
	u32 **subsample_count, u32 ***subsample_size_array, u32 ***subsample_priority_array,
	u32 ***subsample_discardable_array);

MP4Err writeVvccNalus(FILE *out, ISOHandle  sampleEntryH)
{
  MP4Err err          = MP4NoErr;
  const char syncCode[] = "\0\0\0\1";
  const char syncCode2[] = "\0\0\1";

	ISOHandle spsHandle = NULL;
  ISOHandle vpsHandle = NULL;
  ISOHandle ppsHandle = NULL;
  ISOHandle opiHandle = NULL;
  ISOHandle dciHandle = NULL;
  ISOHandle apsHandle[10];
  ISOHandle seiHandle[10];

	u32 ui, num_nalu = 0, aps_count = 0, sei_count = 0;
  u32 sampleSize;
  s32 i;

	err = ISOGetVVCNaluNums(sampleEntryH, VVC_NALU_VPS, &num_nalu); if(err) goto bail;
  if(num_nalu)
  {
    err = ISONewHandle(1, &vpsHandle); if(err) goto bail;
    err = ISOGetVVCSampleDescriptionPS(sampleEntryH, vpsHandle, VVC_NALU_VPS, 1); if(err) goto bail;
    err = ISOGetHandleSize(vpsHandle, &sampleSize); if(err) goto bail;
    fwrite(&syncCode, 4, 1, out); fwrite(*vpsHandle, sampleSize, 1, out);
  }

  err = ISONewHandle(1, &spsHandle); err = ISONewHandle(1, &ppsHandle); if(err) goto bail;
  err = ISOGetVVCSampleDescriptionPS(sampleEntryH, spsHandle, VVC_NALU_SPS, 1); if(err) goto bail;
  err = ISOGetHandleSize(spsHandle, &sampleSize); if(err) goto bail;
  fwrite(&syncCode, 4, 1, out); fwrite(*spsHandle, sampleSize, 1, out);
  err = ISOGetVVCSampleDescriptionPS(sampleEntryH, ppsHandle, VVC_NALU_PPS, 1); if(err) goto bail;
  err = ISOGetHandleSize(ppsHandle, &sampleSize); if(err) goto bail;
  fwrite(&syncCode, 4, 1, out); fwrite(*ppsHandle, sampleSize, 1, out);

  err = ISOGetVVCNaluNums(sampleEntryH, VVC_NALU_OPI, &num_nalu); if(err) goto bail;
  if(num_nalu)
  {
    err = ISONewHandle(1, &opiHandle); if(err) goto bail;
    err = ISOGetVVCSampleDescriptionPS(sampleEntryH, opiHandle, VVC_NALU_OPI, 1); if(err) goto bail;
    err = ISOGetHandleSize(opiHandle, &sampleSize); if(err) goto bail;
    fwrite(&syncCode2, 3, 1, out); fwrite(*opiHandle, sampleSize, 1, out);
  }

  err = ISOGetVVCNaluNums(sampleEntryH, VVC_NALU_DCI, &num_nalu);
  if(err) goto bail;
  if(num_nalu)
  {
    err = ISONewHandle(1, &dciHandle); if(err) goto bail;
    err = ISOGetVVCSampleDescriptionPS(sampleEntryH, dciHandle, VVC_NALU_DCI, 1); if(err) goto bail;
    err = ISOGetHandleSize(dciHandle, &sampleSize);
    if(err) goto bail; fwrite(&syncCode, 4, 1, out);
    fwrite(&syncCode2, 3, 1, out); fwrite(*dciHandle, sampleSize, 1, out);
  }

  err = ISOGetVVCNaluNums(sampleEntryH, VVC_NALU_SEI_PREFIX, &sei_count); if(err) goto bail;
  for(ui = 0; ui < sei_count; ui++)
  {
    err = ISONewHandle(1, &seiHandle[ui]); if(err) goto bail;
    err = ISOGetVVCSampleDescriptionPS(sampleEntryH, seiHandle[ui], VVC_NALU_SEI_PREFIX, ui + 1); if(err) goto bail;
  }
  if(sei_count)
  {
    for(ui = 0; ui < sei_count; ui++)
    {
      err = ISOGetHandleSize(seiHandle[ui], &sampleSize); if(err) goto bail;
      fwrite(&syncCode2, 3, 1, out); fwrite(*seiHandle[ui], sampleSize, 1, out);
    }
  }

	err = ISOGetVVCNaluNums(sampleEntryH, VVC_NALU_APS_PREFIX, &aps_count);
  if(err) goto bail;
  for(ui = 0; ui < aps_count; ui++)
  {
    err = ISONewHandle(1, &apsHandle[ui]); if(err) goto bail;
    err = ISOGetVVCSampleDescriptionPS(sampleEntryH, apsHandle[ui], VVC_NALU_APS_PREFIX, ui + 1); if(err) goto bail;
  }
  if(aps_count)
  {
    for(ui = 0; ui < aps_count; ui++)
    {
      err = ISOGetHandleSize(apsHandle[ui], &sampleSize); if(err) goto bail;
      fwrite(&syncCode, 4, 1, out); fwrite(*apsHandle[ui], sampleSize, 1, out);
    }
  }

  err = ISODisposeHandle(spsHandle);
  err = ISODisposeHandle(ppsHandle);
  if(vpsHandle) err = ISODisposeHandle(vpsHandle);
  if(dciHandle) err = ISODisposeHandle(dciHandle);
  if(opiHandle) err = ISODisposeHandle(opiHandle);
  for(i = aps_count - 1; i >= 0; i--)
    err = ISODisposeHandle(apsHandle[i]);
  for(i = sei_count - 1; i >= 0; i--)
    err = ISODisposeHandle(seiHandle[i]);

bail:
  return err;
}

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

	const char syncCodeZeroByte[] = "\0\0\0\1";
  const char syncCode[] = "\0\0\1";
	char *outSampleName = malloc(128);
	ISOHandle sampleEntryH;
	err = ISONewHandle(1, &sampleEntryH); if (err) goto bail;
	err = ISONewHandle(1, &alst_desc); if (err) goto bail;

	err = ISOOpenMovieFile(&moov, filename, MP4OpenMovieDebug); if (err) goto bail;
	err = MP4GetMovieTrackCount(moov, &trackCount); if (err) goto bail;

	printf("trackCount: %d\r\n", trackCount);

	/* Loop all the tracks in the container, starting from 1 */
	for (trackNumber = 1; trackNumber < trackCount + 1; trackNumber++) {
		FILE *out;

		MP4GenericAtom subs = NULL;
		u32 sampleSize;
		u32 alst_target = 0;
		u32 pics_rasl_skipped = 0;
		u32 pics_start_skipped = 0;
		u32 pics_written = 0;

		sprintf(outSampleName, "out_track_%d.266", trackNumber);
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
		{
			u32 flags;
			u32 entry_count;
			u32* sample_delta = NULL;
			u32* subsample_count = NULL;
			u32** subsample_size_array = NULL;
			u32** subsample_priority_array = NULL;
			u32** subsample_discardable_array = NULL;

			if (MP4GetSubSampleInformationEntryFromTrack(trak, &flags, &entry_count, &sample_delta, &subsample_count, &subsample_size_array, &subsample_priority_array, &subsample_discardable_array) == MP4NoErr) {
				printf("Subsample found: flags: %x, entries: %d\r\n", flags, entry_count);
				for (i = 0; i < entry_count; i++) {
					u32 j;
					printf("[%2d] Subsamples: ", i+1);
					for (j = 0; j < (u32)subsample_count[i]; j++) {						
						printf("%d ", subsample_size_array[i][j]);
					}
					printf("\r\n");
				}

				/* Cleanup */
				/* ToDo: fix */
				for (i = 0; i < entry_count; i++) {
					if ((u32)subsample_count[i]) {
						free(subsample_size_array[i]);        subsample_size_array[i] = NULL;
						free(subsample_priority_array[i]);    subsample_priority_array[i] = NULL;
						//free(subsample_discardable_array[i]);	subsample_discardable_array[i] = NULL;
					}
				}
				free(subsample_size_array);        subsample_size_array = NULL;
				free(subsample_priority_array);    subsample_priority_array = NULL;
				//free(subsample_discardable_array); subsample_discardable_array = NULL;

				free(sample_delta);                sample_delta = NULL;
				free(subsample_count);             subsample_count = NULL;
			}
		}

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
		
		err = writeVvccNalus(out, sampleEntryH); if(err) goto bail;

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
					u32 sampleSize = 0;
					u32 sampleOffsetBytes = 0;
          u8 nalType           = 0;
					
					MP4GetHandleSize(sampleH, &sampleSize);
					/* ToDo: only RADL slices have short start code */
					/* Handle multiple NAL units in a sample */
					while (sampleOffsetBytes < sampleSize)
					{
						u32 boxSize = GET32(*sampleH + sampleOffsetBytes);
						u32 byteoffset = 4;									
						// sample
            nalType = ((u8*)(*sampleH + sampleOffsetBytes + 4))[1] >> 3;
            printf("OutNAL: %d\r\n", nalType);
            //if(nalType >= 14 && nalType <= 16)
            if(nalType > 11)
							fwrite(&syncCodeZeroByte[0], 4, 1, out);
            else
              fwrite(&syncCode[0], 3, 1, out);
            fwrite(*sampleH + sampleOffsetBytes + byteoffset, boxSize, 1, out);
						sampleOffsetBytes += boxSize + byteoffset;
					}
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

	parseInput(argc, argv, &parameters);

	/* We need inputs */
	if (!parameters.inputCount) {
		fprintf(stderr, "Usage: hevc_demuxer -i <inputFile>\r\n");
		fprintf(stderr, "            --input, -i <filename>: Input file\r\n");
		fprintf(stderr, "            --seek,-s <frame> seek to frame\r\n");
		exit(1);
	}


	playMyMovie(&parameters, parameters.inputs[0]);

	cleanParameters(&parameters);

	return 1;
}
