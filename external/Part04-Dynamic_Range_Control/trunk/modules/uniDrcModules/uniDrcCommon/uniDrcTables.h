/***********************************************************************************
 
 This software module was originally developed by
 
 Apple Inc.
 
 in the course of development of the ISO/IEC 23003-4 for reference purposes and its
 performance may not have been optimized. This software module is an implementation
 of one or more tools as specified by the ISO/IEC 23003-4 standard. ISO/IEC gives
 you a royalty-free, worldwide, non-exclusive, copyright license to copy, distribute,
 and make derivative works of this software module or modifications  thereof for use
 in implementations or products claiming conformance to the ISO/IEC 23003-4 standard
 and which satisfy any specified conformance criteria. Those intending to use this
 software module in products are advised that its use may infringe existing patents.
 ISO/IEC have no liability for use of this software module or modifications thereof.
 Copyright is not released for products that do not conform to the ISO/IEC 23003-4
 standard.
 
 Apple Inc. retains full right to modify and use the code for its own purpose,
 assign or donate the code to a third party and to inhibit third parties from using
 the code for products that do not conform to MPEG-related ITU Recommendations and/or
 ISO/IEC International Standards.
 
 This copyright notice must be included in all copies or derivative works.
 
 Copyright (c) ISO/IEC 2014.
 
 ***********************************************************************************/

#ifndef _UNI_DRC_TABLES_H_
#define _UNI_DRC_TABLES_H_

#ifdef __cplusplus
extern "C"
{
#endif

#define N_DELTA_TIME_CODE_TABLE_ENTRIES_MAX (512+14)

typedef struct {
    int size;
    int code;
    int value;
} DeltaTimeCodeTableEntry;

typedef struct {
    int size;
    int code;
    float value;
    int index;
} SlopeCodeTableEntry;

typedef struct {
    int size;
    int code;
    float value;
} DeltaGainCodeEntry;

typedef struct {
    DeltaTimeCodeTableEntry deltaTimeCodeTable[N_DELTA_TIME_CODE_TABLE_ENTRIES_MAX];
} Tables;
    
typedef struct {
    float ioRatio;
    float expLo;
    float expHi;
} CicpSigmoidCharacteristicParam;

typedef struct {
    float inLevel;
    float gain;
} CharacteristicNodeCoordinate;

typedef struct {
    int coordinateCount;
    CharacteristicNodeCoordinate characteristicNodeCoordinate[5];
} CicpNodeCharacteristicParam;
    
int
initTables(const int nGainValuesMax,
           Tables* tables);

int
removeTables(void);

void
generateDeltaTimeCodeTable (const int nGainValuesMax,
                            DeltaTimeCodeTableEntry* deltaTimeCodeTableItem);

void
getDeltaGainCodeTable(const int gainCodingProfile,
                      DeltaGainCodeEntry const** deltaGainCodeTable,
                      int *nEntries);

const SlopeCodeTableEntry*
getSlopeCodeTableByValue(void);

void
getSlopeCodeTableAndSize(SlopeCodeTableEntry const** slopeCodeTableEntry,
                         int* nSlopeCodeTableEntries);

float
decodeSlopeIndex(const int slopeCodeIndex);

float
decodeSlopeIndexMagnitude(const int slopeCodeIndex);


int
getDeltaTmin (const int sampleRate);

#ifdef __cplusplus
}
#endif
#endif /* _UNI_DRC_TABLES_H_ */