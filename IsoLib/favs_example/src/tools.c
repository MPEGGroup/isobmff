#include <math.h>
#include <string.h>
#include "ISOMovies.h"
#include "structures.h"
#include "tools.h"

MP4Err BitBuffer_Init(BitBuffer *bb, u8 *p, u32 length) {
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

u32 ceil_log2(u32 x) {
	u32 ret = 0;
	while (x>((u32)1 << ret)) ret++;
	return ret;
}


u32 GetBits(BitBuffer *bb, u32 nBits, MP4Err *errout) {
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
				bb->emulation_position = bb->cbyte ? 0 : 1;
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

MP4Err GetBytes(BitBuffer *bb, u32 nBytes, u8 *p) {
	MP4Err err = MP4NoErr;
	unsigned int i;

	for (i = 0; i < nBytes; i++) {
		*p++ = (u8)GetBits(bb, 8, &err);
		if (err) break;
	}

	return err;
}

u32 read_golomb_uev(BitBuffer *bb, MP4Err *errout) {
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




int parseInput(int argc, char* argv[], struct ParamStruct *parameters) {
	u32 param;
	for (param = 1; param < (u32)argc; param++) {
		if (argv[param][0] == '-') {
			switch (argv[param][1]) {

				/* New input track */
			case 'i': {
				char *tempInput;
				u32 inputLen = 0;
				u32 inputCopy = 0;

				/* if next parameter is not present, abort this operation */
				if (argc - 1 == param) break;

				/* Allocate new pointer array and copy the old if present */
				parameters->inputs = realloc(parameters->inputs, sizeof(char*)*(parameters->inputCount + 1));

				/* Allocate new char array for the name and copy */
				inputLen = strlen(argv[param + 1]);
				tempInput = malloc(inputLen + 1);
				memcpy(tempInput, argv[param + 1], inputLen + 1);

				/* Set new input track */
				parameters->inputs[parameters->inputCount] = tempInput;
				parameters->inputCount++;

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
				parameters->output = malloc(outputLen + 1);
				memcpy(parameters->output, argv[param + 1], outputLen + 1);
				param++;
			}
				break;
				/* Framerate */
			case 'f': {
				/* if next parameter is not present, abort this operation */
				if (argc - 1 == param) break;
				parameters->framerate = atof(argv[param + 1]);
				param++;
			}
				/* Seek */
			case 's': {
				/* if next parameter is not present, abort this operation */
				if (argc - 1 == param) break;
				parameters->seek = atoi(argv[param + 1]);
				param++;
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
				newTrackGroup->track_group_id = atoi(&argv[param + 1][found + 1]);

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
				/* Compact sample to group flag */
			case 'c': {
				parameters->compactSampleToGroup = 1;
			}
				break;
				/* Long parameter name */
			case '-': {
#define LONG_PARAM_I(NAME, LEN, OUTPUT) if (strncmp(&argv[param][2], NAME, LEN) == 0) { \
					if (argv[param][LEN+2] == '=') OUTPUT = atoi(&argv[param][LEN+3]); \
							else { if (argc - 1 == param) break; param++; OUTPUT = atoi(argv[param + 1]); } \
							}
#define LONG_PARAM_F(NAME, LEN, OUTPUT) if (strncmp(&argv[param][2], NAME, LEN) == 0) { \
					if (argv[param][LEN+2] == '=') OUTPUT = atof(&argv[param][LEN+3]); \
							else { if (argc - 1 == param) break; param++; OUTPUT = atof(argv[param + 1]); } \
							}
#define LONG_PARAM_S(NAME, LEN, OUTPUT) if (strncmp(&argv[param][2], NAME, LEN) == 0) { \
				if (argv[param][LEN+2] == '=') { \
					u32 inputLen = strlen(&argv[param][LEN+3]); \
					OUTPUT = malloc(inputLen + 1); \
					memcpy(OUTPUT, &argv[param][LEN+3], inputLen + 1); \
								} else { if (argc - 1 == param) break; param++; \
					u32 inputLen = strlen(argv[param + 1]); \
					OUTPUT = malloc(inputLen + 1); \
					memcpy(OUTPUT, argv[param + 1], inputLen + 1); \
								} }
				char* temp_string = NULL;

				/* List of long parameters */
				LONG_PARAM_I("subs", 4, parameters->subsample_information);
				LONG_PARAM_I("seek", 4, parameters->seek);
				LONG_PARAM_F("framerate", 9, parameters->framerate);
				LONG_PARAM_S("input", 5, temp_string);
				if (temp_string) {
					parameters->inputs = (char **)realloc(parameters->inputs, sizeof(char*) * (parameters->inputCount + 1));
					parameters->inputs[parameters->inputCount] = temp_string;
					parameters->inputCount++;
					printf("New input %s\r\n", temp_string);
					temp_string = NULL;
				}
				LONG_PARAM_S("output", 5, parameters->output);

			}
				break;
			}
		}
	}
	return 1;
}