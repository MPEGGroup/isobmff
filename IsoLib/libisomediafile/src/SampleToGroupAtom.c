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

#include "MP4Atoms.h"
#include <stdlib.h>
#include <string.h>

#define allocation_size 8192

enum
{
  kMaxPatternLength   = 32,
  kUpperNibblePresent = 0x8000
};

/* PatternTracker is maintained as an array for each sample input.
patternStart at index k, represents the start index of the pattern which helped yield the most
efficient way to encode samples from [0, k] inclusive with patternLength, repeated sampleCount times
cumulativeMemoryCost at index k, represents the memory cost associated with most efficient way to
encode samples from [0, k] inclusive. Default considers each sample as a distinct non-pattern
prevEfficientIndex at index k, represents the end of the most efficient previous pattern/
non-pattern before k which helped yield the cumulativeMemoryCost at index k. nextEfficientIndex is
populated after back-tracking prevEfficientIndex to easily create compact sample groups
*/
typedef struct
{
  u32 patternStart;
  u32 patternLength;
  u32 sampleCount;
  s32 cumulativeMemoryCost;
  s32 prevEfficientIndex;
  s32 nextEfficientIndex;
} PatternTracker, *PatternTrackerPtr;

/*
Compacting Algorithm:
1. Run through sample group set: i=[0,n]
       a.	Expand the input samples
       b.  Initialize memory costs in tracker assuming each sample is a distinct non-pattern C[i] =
(i*indexFieldSize)
2. Run through sample group set: i=[0,n]
       a. Update memory cost considering sample i is a distinct non-pattern C[i] = min(C[i] ,
C[i-1]+ indexFieldSize) b. For pattern lengths, p=[1, 32] i. Consider patterns ending at i, from
[start=i-p, i] in the Search buffer. ii. Extend right end from [i+1, n] in the look ahead buffer
                               1. As long as the pattern repeats, keep extending right end and
update memory cost C[end] = min(C[end] , C[start-1] + p*indexFieldSize
                               2. If C[end] got modified, a more efficient pattern was found,
maintain previousEfficientIndex = start-1 for backtracking.
3. Tracker[n-1] represents most efficienct way to encode samples [0,n]. Run through tracker
backwards from last sample: i[n-1, 0]. a. Use previousEfficientIndex to populate nextEfficientIndex
to easily create compact sample groups b. Patterns with (pattern length == sample count == 1), are
combined and considered as non-pattern

Algorithm can further be optimized by making memory cost consider patternLength and sampleCount
field sizes
*/

typedef struct
{
  u32 *groupIndex;
  u32 sampleCount;
  PatternTracker *patternTracker;
  u8 groupIndexFieldSize;
} SampleGroupInfo;

static u8 GetFieldSize(u32 value, u8 localFragmentUsed)
{
  if(localFragmentUsed)
  {
    if(value < 8)
    {
      return 4;
    }
    else if(value < 128)
    {
      return 8;
    }
    else if(value < 32768)
    {
      return 16;
    }
    else
    {
      return 32;
    }
  }
  else
  {
    if(value < 16)
    {
      return 4;
    }
    else if(value < 256)
    {
      return 8;
    }
    else if(value < 65536)
    {
      return 16;
    }
    else
    {
      return 32;
    }
  }
}

static u8 SetFieldSize(u32 fieldSize)
{
  assert(fieldSize == 4 || fieldSize == 8 || fieldSize == 16 || fieldSize == 32);
  switch(fieldSize)
  {
  case 4:
    return 0;
    break;
  case 8:
    return 1;
    break;
  case 16:
    return 2;
    break;
  case 32:
    return 3;
    break;
  };
  return 0;
}

static void AppendDescriptionIndexToCompactGroup(CompressedGroupInfo *compressedGroup,
                                                 u32 descIndex, u32 groupIndex)
{
  assert(descIndex < compressedGroup->totalIndexDescriptionCount);
  compressedGroup->indexDescriptionArray[descIndex] = groupIndex;
}

static void AppendNewPatternEntry(CompressedGroupInfo *compressedGroup, u32 index,
                                  u32 patternLength, u32 sampleCount)
{
  assert(patternLength != 0);
  assert(sampleCount != 0);
  assert(index < compressedGroup->patternCount);
  /*printf("\n New pattern: length %d sampleCount %d", patternLength, sampleCount); */

  compressedGroup->patternEntries[index].patternLength = patternLength;
  compressedGroup->patternEntries[index].sampleCount   = sampleCount;
  compressedGroup->totalSampleCount += sampleCount;
}

static void SetMemoryCostForPattern(SampleGroupInfo *sampleGroup, u32 patternLength,
                                    u32 startPatternSampleIndex, u32 endPatternSampleIndex)
{
  s32 memoryCost;
  u32 sampleCount = endPatternSampleIndex - startPatternSampleIndex + 1;

  assert(startPatternSampleIndex + patternLength <= endPatternSampleIndex);

  memoryCost = startPatternSampleIndex <= 0
                   ? sampleGroup->patternTracker[0].cumulativeMemoryCost
                   : sampleGroup->patternTracker[startPatternSampleIndex - 1].cumulativeMemoryCost;
  memoryCost += (patternLength * sampleGroup->groupIndexFieldSize);

  if(memoryCost < sampleGroup->patternTracker[endPatternSampleIndex].cumulativeMemoryCost)
  {
    PatternTracker patTrack;
    patTrack.patternStart         = startPatternSampleIndex;
    patTrack.patternLength        = patternLength;
    patTrack.sampleCount          = sampleCount;
    patTrack.cumulativeMemoryCost = memoryCost;
    patTrack.prevEfficientIndex   = startPatternSampleIndex - 1;

    sampleGroup->patternTracker[endPatternSampleIndex] = patTrack;
    /*printf("\n Pattern run of pattern length %d, sample count %d, starting @ %3d with memoryCost
     * %d and prevMostEffIndex %d", patternLength, sampleCount, startPatternSampleIndex,
     * patTrack.cumulativeMemoryCost, patTrack.prevEfficientIndex); */
  }
}

static void SetMemoryCostForNonPattern(SampleGroupInfo *sampleGroup, s32 sampleIndex)
{
  s32 memoryCost =
      (sampleIndex < 1 ? sampleGroup->patternTracker[0].cumulativeMemoryCost
                       : sampleGroup->patternTracker[sampleIndex - 1].cumulativeMemoryCost) +
      sampleGroup->groupIndexFieldSize;

  if(memoryCost < sampleGroup->patternTracker[sampleIndex].cumulativeMemoryCost)
  {
    PatternTracker patTrack;
    patTrack.patternStart         = sampleIndex;
    patTrack.patternLength        = 1;
    patTrack.sampleCount          = 1;
    patTrack.cumulativeMemoryCost = memoryCost;
    patTrack.prevEfficientIndex   = sampleIndex - 1;

    sampleGroup->patternTracker[sampleIndex] = patTrack;
    /*printf("\n Writing non-pattern with memoryCost %d and prevMostEffIndex %d ",
     * patTrack.cumulativeMemoryCost, patTrack.prevEfficientIndex);*/
  }
}

static void CombineNonPatterns(SampleGroupInfo *sampleGroup, CompressedGroupInfo *compressedGroup)
{
  s32 prevIndex                  = sampleGroup->sampleCount - 1;
  s32 nextIndex                  = sampleGroup->sampleCount;
  PatternTracker *patternTracker = sampleGroup->patternTracker;

  /* Post-process output pattern tracker. Get Pattern Count, Populate the nextIndex to navigate
   * easily*/
  while(prevIndex >= 0 && prevIndex < (s32)sampleGroup->sampleCount)
  {
    u32 consecutiveNonPatterns = 0;
    s32 i                      = prevIndex;
    while(i >= 0 && i < (s32)sampleGroup->sampleCount && (patternTracker[i].patternLength == 1) &&
          (patternTracker[i].sampleCount == 1))
    {
      consecutiveNonPatterns++;
      i = patternTracker[i].prevEfficientIndex;
    }
    if(consecutiveNonPatterns)
    {
      patternTracker[prevIndex].patternLength = consecutiveNonPatterns;
      patternTracker[prevIndex].sampleCount   = consecutiveNonPatterns;
      patternTracker[prevIndex].patternStart -= (consecutiveNonPatterns - 1);
      compressedGroup->totalIndexDescriptionCount += consecutiveNonPatterns;
      patternTracker[prevIndex].prevEfficientIndex = i;
    }
    else
    {
      compressedGroup->totalIndexDescriptionCount += patternTracker[prevIndex].patternLength;
    }

    patternTracker[prevIndex].nextEfficientIndex = nextIndex;
    nextIndex                                    = prevIndex;
    prevIndex                                    = patternTracker[prevIndex].prevEfficientIndex;

    compressedGroup->patternCount++;
  }
  compressedGroup->efficientStartIndex = nextIndex;
}

static void FindPatternsStartingAtIndex(SampleGroupInfo *sampleGroup, u32 sampleIndex)
{
  u32 p;
  assert(sampleIndex < sampleGroup->sampleCount);

  /* Update memoryCost in case current sample is considered as a non-pattern*/
  SetMemoryCostForNonPattern(sampleGroup, sampleIndex);

  for(p = 1; p <= kMaxPatternLength; p++)
  {
    if(sampleIndex < p) break;

    if(sampleGroup->groupIndex[sampleIndex] == sampleGroup->groupIndex[sampleIndex - p])
    {
      u32 startPatternSampleIndex = sampleIndex - p;
      u32 i                       = sampleIndex;
      while(i < sampleGroup->sampleCount &&
            sampleGroup->groupIndex[i] == sampleGroup->groupIndex[i - p])
      {
        SetMemoryCostForPattern(sampleGroup, p, startPatternSampleIndex, i);
        i++;
      }

      /* Pattern ended at i - 1.
      printf("\n %3d: pattern run ended", i-1);
      break; */
    }
  }
}

static void InitializeSampleGroupInput(MP4SampletoGroupAtomPtr self, SampleGroupInfo *sampleGroup)
{
  u32 i, startValue;
  PatternTracker patternTrackerEntry;
  sampleGroup->groupIndex          = self->group_index;
  sampleGroup->sampleCount         = self->sampleCount;
  sampleGroup->groupIndexFieldSize = self->compressedGroup.indexFieldSize;
  sampleGroup->patternTracker =
      (PatternTracker *)malloc(sizeof(PatternTracker) * self->sampleCount);

  memset(sampleGroup->patternTracker, 0, sizeof(PatternTracker) * self->sampleCount);

  /* Expand sample group indices, get maximum field size to represent each group index*/
  startValue = self->group_index[0];

  for(i = 1; i < self->sampleCount; i++)
  {
    if(self->group_index[i - 1] == self->group_index[i])
    {
      self->group_index[i - 1] = startValue;
    }
    else
    {
      startValue = self->group_index[i];
    }
  }
  self->group_index[self->sampleCount - 1] = startValue;

  /* Initialize efficiencies assuming each input sample is a distinct non-pattern*/
  patternTrackerEntry.patternLength = patternTrackerEntry.sampleCount = 1;

  for(i = 0; i < self->sampleCount; i++)
  {
    patternTrackerEntry.patternStart         = i;
    patternTrackerEntry.prevEfficientIndex   = i - 1;
    patternTrackerEntry.cumulativeMemoryCost = (i + 1) * sampleGroup->groupIndexFieldSize;
    sampleGroup->patternTracker[i]           = patternTrackerEntry;
  }
}

static void CreateCompactSampleGroups(MP4SampletoGroupAtomPtr self)
{
  u32 j, patternIndex, descIndex, nextIndex, sampleCount;
  SampleGroupInfo sampleGroup;
  CompressedGroupInfoPtr compressedGroup;
  PatternTrackerPtr patternTracker;
  if(self->compressedGroup.isSampleGroupCompressed) return;

  InitializeSampleGroupInput(self, &sampleGroup);

  compressedGroup                         = &self->compressedGroup;
  compressedGroup->patternLengthFieldSize = 4;
  compressedGroup->sampleCountFieldSize   = 4;
  compressedGroup->indexFieldSize         = sampleGroup.groupIndexFieldSize;

  /* Repeatedly process, find and update efficiencies at each sample index*/
  for(j = 0; j < self->sampleCount; j++)
  {
    FindPatternsStartingAtIndex(&sampleGroup, j);
  }

  CombineNonPatterns(&sampleGroup, compressedGroup);

  if(compressedGroup->patternEntries == NULL)
  {
    compressedGroup->patternEntries = (MP4CompactSampleToGroupPatternEntryPtr)malloc(
        sizeof(MP4CompactSampleToGroupPatternEntry) * compressedGroup->patternCount);
  }
  if(compressedGroup->indexDescriptionArray == NULL)
  {
    compressedGroup->indexDescriptionArray =
        (u32 *)malloc(sizeof(u32) * compressedGroup->totalIndexDescriptionCount);
  }

  /*printf("\n The pattern tracker details are");
  for(int i = 0; i < self->sampleCount; i++)
  {
          printf("\n Index %d Start: %d  PL: %d  SC: %d  Eff: %d  PrevIndex: %d NextIndex: %d ", i,
  patternTracker[i].start, patternTracker[i].patternLength, patternTracker[i].sampleCount,
  patternTracker[i].cumulativeMemoryCost, patternTracker[i].prevEfficientIndex,
  patternTracker[i].nextEfficientIndex);
  }*/

  patternTracker = sampleGroup.patternTracker;
  descIndex      = 0;
  nextIndex      = compressedGroup->efficientStartIndex;
  for(patternIndex = 0; patternIndex < compressedGroup->patternCount; patternIndex++)
  {
    u32 patternLength = patternTracker[nextIndex].patternLength;
    u8 fieldSize      = GetFieldSize(patternLength, 0);
    if(fieldSize > compressedGroup->patternLengthFieldSize)
      compressedGroup->patternLengthFieldSize = fieldSize;

    sampleCount = patternTracker[nextIndex].sampleCount;
    fieldSize   = GetFieldSize(sampleCount, 0);
    if(fieldSize > compressedGroup->sampleCountFieldSize)
      compressedGroup->sampleCountFieldSize = fieldSize;

    AppendNewPatternEntry(compressedGroup, patternIndex, patternLength, sampleCount);

    assert(patternTracker[nextIndex].patternStart < self->sampleCount);
    assert(patternTracker[nextIndex].patternStart + patternLength - 1 < self->sampleCount);

    for(j = 0; j < patternLength; j++)
    {
      u32 groupIndex = sampleGroup.groupIndex[patternTracker[nextIndex].patternStart + j];
      if(self->fragmentLocalIndexPresent)
      {
        groupIndex = groupIndex | (1 << (self->compressedGroup.indexFieldSize - 1));
      }
      AppendDescriptionIndexToCompactGroup(compressedGroup, descIndex, groupIndex);
      descIndex++;
    }
    nextIndex = patternTracker[nextIndex].nextEfficientIndex;
  }

  /* In case only patternLength or sampleCount field size is 4, ensure pattern entry is always byte
   * aligned*/
  if((compressedGroup->patternLengthFieldSize + compressedGroup->sampleCountFieldSize) % 8)
  {
    if(compressedGroup->patternLengthFieldSize == 4) compressedGroup->patternLengthFieldSize = 8;
    else
      compressedGroup->sampleCountFieldSize = 8;
  }

  assert(descIndex == compressedGroup->totalIndexDescriptionCount);
  self->compressedGroup.isSampleGroupCompressed = 1;

  free(sampleGroup.patternTracker);
  sampleGroup.patternTracker = NULL;
}

static void destroy(MP4AtomPtr s)
{
  MP4SampletoGroupAtomPtr self;
  self = (MP4SampletoGroupAtomPtr)s;
  if(self == NULL) return;
  if(self->group_index != NULL)
  {
    free(self->group_index);
    self->group_index = NULL;
  }

  if(self->compressedGroup.patternEntries != NULL)
  {
    free(self->compressedGroup.patternEntries);
    self->compressedGroup.patternEntries = NULL;
  }

  if(self->compressedGroup.indexDescriptionArray != NULL)
  {
    free(self->compressedGroup.indexDescriptionArray);
    self->compressedGroup.indexDescriptionArray = NULL;
  }

  if(self->super) self->super->destroy(s);
}

static MP4Err ensureSize(struct MP4SampletoGroupAtom *self, u32 newSize)
{
  MP4Err err;

  err = MP4NoErr;

  if(newSize > self->allocatedSize)
  {
    self->allocatedSize += allocation_size;
    if(newSize > self->allocatedSize) self->allocatedSize = newSize;

    if(self->group_index != NULL)
      self->group_index = (u32 *)realloc(self->group_index, self->allocatedSize);
    else
      self->group_index = (u32 *)calloc(self->allocatedSize, 1);

    TESTMALLOC(self->group_index);
  }

bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err addSamples(struct MP4SampletoGroupAtom *self, u32 count)
{
  MP4Err err;
  u32 *p;
  u32 j;

  err = MP4NoErr;
  err = ensureSize(self, (self->sampleCount + count) * sizeof(u32));
  if(err) goto bail;

  p = &((self->group_index)[self->sampleCount]);
  for(j = 0; j < count; j++)
    *p++ = 0;
  self->sampleCount += count;

bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err mapSamplestoGroup(struct MP4SampletoGroupAtom *self, u32 group_index,
                                s32 sample_index, u32 count)
{
  MP4Err err;
  u32 i;
  u32 *p;

  err = MP4NoErr;

  if(sample_index < 0)
  {
    p = &((self->group_index)[self->sampleCount + sample_index]);
    if(count > ((u32)(-sample_index)))
    {
      err = MP4BadParamErr;
      goto bail;
    }
  }
  else
  {
    p = &((self->group_index)[sample_index]);
    if(count + sample_index > self->sampleCount)
    {
      err = MP4BadParamErr;
      goto bail;
    }
  }
  for(i = 0; i < count; i++)
    *p++ = group_index;

  if(GetFieldSize(group_index, self->fragmentLocalIndexPresent) >
     self->compressedGroup.indexFieldSize)
    self->compressedGroup.indexFieldSize =
        GetFieldSize(group_index, self->fragmentLocalIndexPresent);

bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err changeSamplestoGroupType(struct MP4SampletoGroupAtom *self,
                                       sampleToGroupType_t sampleToGroupType)
{
  MP4Err err;

  if(self == NULL) BAILWITHERROR(MP4BadParamErr)

  self->sampleToGroupType = sampleToGroupType;
  switch(self->sampleToGroupType)
  {
  case SAMPLE_GROUP_NORMAL:
    self->type = MP4SampletoGroupAtomType;
    self->name = "sample to group";
    break;
  case SAMPLE_GROUP_COMPACT:
    self->type = MP4CompactSampletoGroupAtomType;
    self->name = "compact sample to group";
    break;
  case SAMPLE_GROUP_AUTO:
    self->type = 0;
    self->name = "auto sample to group";
    break;
  default:
    self->type = MP4SampletoGroupAtomType;
    self->name = "(default) sample to group";
    break;
  }

bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err getSampleGroupMap(struct MP4SampletoGroupAtom *self, u32 sampleNumber,
                                u32 *groupIndex)
{
  MP4Err err;

  err = MP4NoErr;

  if(sampleNumber < 1) BAILWITHERROR(MP4BadParamErr);
  if(sampleNumber > self->sampleCount) *groupIndex = 0;
  else
    *groupIndex = (self->group_index)[sampleNumber - 1];

bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err GetData(MP4AtomPtr s, MP4InputStreamPtr inputStream, u8 fieldSize, u32 *upperNibble,
                      u32 *outValue)
{
  MP4Err err                   = MP4NoErr;
  MP4SampletoGroupAtomPtr self = (MP4SampletoGroupAtomPtr)s;
  u32 readData                 = 0;

  if(fieldSize != 4 && fieldSize != 8 && fieldSize != 16 && fieldSize != 32)
    BAILWITHERROR(MP4BadParamErr)

  switch(fieldSize)
  {
  case 4:
    if(*upperNibble & kUpperNibblePresent)
    {
      *outValue    = *upperNibble & 0xF;
      *upperNibble = 0;
    }
    else
    {
      GET8_V_MSG(readData, NULL);
      *upperNibble = (readData & 0xF) | kUpperNibblePresent;
      *outValue    = readData >> 4;
    }
    break;
  case 8:
    GET8_V_MSG(readData, NULL);
    *outValue = readData;
    break;
  case 16:
    GET16_V_MSG(readData, NULL);
    *outValue = readData;
    break;
  case 32:
    GET32_V_MSG(readData, NULL);
    *outValue = readData;
    break;
  }
bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err PackData(MP4SampletoGroupAtomPtr self, char **bufferPtr, u8 fieldSize,
                       u8 nonByteBoundary, u32 value)
{
  MP4Err err = MP4NoErr;
  u32 previousByte;
  char *buffer = *bufferPtr;
  if(fieldSize != 4 && fieldSize != 8 && fieldSize != 16 && fieldSize != 32)
    BAILWITHERROR(MP4BadParamErr)

  switch(fieldSize)
  {
  case 4:
    /* Read the previous byte and append the new nibble to it*/
    if(nonByteBoundary)
    {
      buffer = buffer - 1;
      self->bytesWritten -= 1;
      previousByte = *(u8 *)buffer;
      value        = previousByte | (value & 0xF);
    }
    else
    {
      value = value << 4;
    }
    PUT8_V(value);
    break;
  case 8:
    PUT8_V(value);
    break;
  case 16:
    PUT16_V(value);
    break;
  case 32:
    PUT32_V(value);
    break;
  }
  *bufferPtr = buffer;
bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err serialize(struct MP4Atom *s, char *buffer)
{
  MP4Err err;
  u32 i, cur_index, cur_count, entryCount;

  MP4SampletoGroupAtomPtr self = (MP4SampletoGroupAtomPtr)s;
  err                          = MP4NoErr;

  if(self->type == MP4CompactSampletoGroupAtomType)
  {
    self->flags = SetFieldSize(self->compressedGroup.indexFieldSize) |
                  (SetFieldSize(self->compressedGroup.sampleCountFieldSize) << 2) |
                  (SetFieldSize(self->compressedGroup.patternLengthFieldSize) << 4) |
                  (self->fragmentLocalIndexPresent ? 0x80 : 0);
  }

  err = MP4SerializeCommonFullAtomFields((MP4FullAtomPtr)s, buffer);
  if(err) goto bail;
  buffer += self->bytesWritten;

  PUT32(grouping_type);
  /* Grouping type parameter is disabled. If enabled, will need to pack an additional byte to
   * represent it*/
  PUT32(entryCount);

  if(self->type == MP4SampletoGroupAtomType)
  {
    cur_index  = (self->group_index)[0];
    cur_count  = 1;
    entryCount = 0;

    for(i = 1; i < self->sampleCount; i++)
    {
      if((self->group_index)[i - 1] != (self->group_index)[i])
      {
        PUT32_V(cur_count);
        PUT32_V(cur_index);
        cur_count = 1;
        cur_index = (self->group_index)[i];
        entryCount++;
      }
      else
        cur_count++;
    }
    PUT32_V(cur_count);
    PUT32_V(cur_index);
    entryCount++;

    assert(entryCount == self->entryCount);
  }
  else
  {
    printf(" \n Field sizes of pattern is %d , sampleCount is %d , indexDescription is %d ",
           self->compressedGroup.patternLengthFieldSize, self->compressedGroup.sampleCountFieldSize,
           self->compressedGroup.indexFieldSize);

    printf("\n Pattern count %d covering %d total samples ", self->compressedGroup.patternCount,
           self->compressedGroup.totalSampleCount);

    for(i = 0; i < self->compressedGroup.patternCount; i++)
    {
      printf("\n Pattern length %d for %d samples ",
             self->compressedGroup.patternEntries[i].patternLength,
             self->compressedGroup.patternEntries[i].sampleCount);

      /* Pattern entry is ensured to start at a byte boundary*/
      PackData(self, &buffer, self->compressedGroup.patternLengthFieldSize, 0,
               self->compressedGroup.patternEntries[i].patternLength);

      PackData(self, &buffer, self->compressedGroup.sampleCountFieldSize,
               (self->compressedGroup.patternLengthFieldSize == 4),
               self->compressedGroup.patternEntries[i].sampleCount);
    }

    /* Index descriptor array is ensured to start at a byte boundary*/
    printf("\n Index Descriptors array is: ");
    for(i = 0; i < self->compressedGroup.totalIndexDescriptionCount; i++)
    {
      printf(" %d ", self->compressedGroup.indexDescriptionArray[i]);
      PackData(self, &buffer, self->compressedGroup.indexFieldSize, (i & 1),
               self->compressedGroup.indexDescriptionArray[i]);
    }
  }

  assert(self->bytesWritten == self->size);
bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err calculateSize(struct MP4Atom *s)
{
  MP4Err err;
  MP4SampletoGroupAtomPtr self = (MP4SampletoGroupAtomPtr)s;
  u32 entryCount, i, normalSize, compactSize, sizeInBits;

  err = MP4NoErr;

  err = MP4CalculateFullAtomFieldSize((MP4FullAtomPtr)s);
  if(err) goto bail;

  if(self->sampleToGroupType != SAMPLE_GROUP_AUTO)
  {
    if(self->type == MP4SampletoGroupAtomType)
    {
      entryCount = 1;
      for(i = 1; i < (self->sampleCount); i++)
      {
        if((self->group_index)[i - 1] != (self->group_index)[i]) entryCount++;
      }
      self->size += (entryCount * 8) + 8;
      self->entryCount = entryCount;
    }
    else if(self->type == MP4CompactSampletoGroupAtomType)
    {
      CreateCompactSampleGroups(self);

      /* If grouping type parameter is enabled in flags, will need to add an additional byte*/
      self->size += 8;

      sizeInBits =
          (self->compressedGroup.patternCount * (self->compressedGroup.patternLengthFieldSize +
                                                 self->compressedGroup.sampleCountFieldSize));
      sizeInBits +=
          (self->compressedGroup.totalIndexDescriptionCount * self->compressedGroup.indexFieldSize);
      self->size = self->size + (sizeInBits + 4) / 8;

      self->entryCount = self->compressedGroup.patternCount;
    }
  }
  else
  {
    normalSize = compactSize = self->size;
    /* normal */
    entryCount = 1;
    for(i = 1; i < (self->sampleCount); i++)
    {
      if((self->group_index)[i - 1] != (self->group_index)[i]) entryCount++;
    }
    normalSize += (entryCount * 8) + 8;
    /* compact */
    CreateCompactSampleGroups(self);
    compactSize += 8;
    sizeInBits =
        (self->compressedGroup.patternCount * (self->compressedGroup.patternLengthFieldSize +
                                               self->compressedGroup.sampleCountFieldSize));
    sizeInBits +=
        (self->compressedGroup.totalIndexDescriptionCount * self->compressedGroup.indexFieldSize);
    compactSize += (sizeInBits + 4) / 8;

    /* pick compact only if the size is smaller */
    if(normalSize <= compactSize)
    {
      self->changeSamplestoGroupType(self, SAMPLE_GROUP_NORMAL);
      self->size       = normalSize;
      self->entryCount = entryCount;
    }
    else
    {
      self->changeSamplestoGroupType(self, SAMPLE_GROUP_COMPACT);
      self->size       = compactSize;
      self->entryCount = self->compressedGroup.patternCount;
    }
  }

bail:
  TEST_RETURN(err);

  return err;
}

static MP4Err createFromInputStream(MP4AtomPtr s, MP4AtomPtr proto, MP4InputStreamPtr inputStream)
{
  MP4Err err;
  u32 i, j, size, flags, upperNibble;
  u32 *p;
  MP4CompactSampleToGroupPatternEntryPtr patternEntries;
  MP4SampletoGroupAtomPtr self = (MP4SampletoGroupAtomPtr)s;
  char typeString[8];
  char msgString[80];
  u8 indexFieldSize, countFieldSize, patternFieldSize;
  char groupingTypeParamPresent;

  err = MP4NoErr;
  if(self == NULL) BAILWITHERROR(MP4BadParamErr)
  err = self->super->createFromInputStream(s, proto, (char *)inputStream);
  if(err) goto bail;
  flags = self->flags;

  if(self->type == MP4SampletoGroupAtomType)
  {
    self->sampleToGroupType = 0;
    GET32(grouping_type);
    MP4TypeToString(self->grouping_type, typeString);
    sprintf(msgString, " grouping type is '%s'", typeString);
    inputStream->msg(inputStream, msgString);

    GET32(entryCount);
    size = 0;

    for(i = 0; i < self->entryCount; i++)
    {
      u32 count, index;
      GET32_V_MSG(count, NULL);
      GET32_V_MSG(index, NULL);
      sprintf(msgString, " entry %d, count %d index %d", i + 1, count, index);
      inputStream->msg(inputStream, msgString);

      err = ensureSize(self, (size + count) * sizeof(u32));
      if(err) goto bail;

      p = &((self->group_index)[size]);
      for(j = 0; j < count; j++)
        *p++ = index;
      size += count;
    }
    self->sampleCount = size;
  }
  else
  {
    self->sampleToGroupType = 1;
    indexFieldSize          = 4 << ((flags & 0x3) >> 0);
    if(indexFieldSize != 4 && indexFieldSize != 8 && indexFieldSize != 16 && indexFieldSize != 32)
      BAILWITHERROR(MP4BadParamErr)

    countFieldSize = 4 << ((flags & 0xC) >> 2);
    if(countFieldSize != 4 && countFieldSize != 8 && countFieldSize != 16 && countFieldSize != 32)
      BAILWITHERROR(MP4BadParamErr)

    patternFieldSize = 4 << ((flags & 0x30) >> 4);
    if(patternFieldSize != 4 && patternFieldSize != 8 && patternFieldSize != 16 &&
       patternFieldSize != 32)
      BAILWITHERROR(MP4BadParamErr)

    /* Ensure each pattern entry is always byte-aligned*/
    if((patternFieldSize + countFieldSize) % 8) BAILWITHERROR(MP4BadParamErr)

    self->fragmentLocalIndexPresent = (flags & 0x80);
    groupingTypeParamPresent        = (flags & 0x40);

    sprintf(msgString, "field size for index %d , sample count %d pattern length %d ",
            indexFieldSize, countFieldSize, patternFieldSize);
    inputStream->msg(inputStream, msgString);

    GET32(grouping_type);
    MP4TypeToString(self->grouping_type, typeString);
    sprintf(msgString, "grouping type is '%s'", typeString);
    inputStream->msg(inputStream, msgString);

    if(groupingTypeParamPresent)
    {
      GET32(groupingTypeParameter);
    }

    GET32(entryCount);

    patternEntries = malloc(self->entryCount * sizeof(MP4CompactSampleToGroupPatternEntry));
    TESTMALLOC(patternEntries);
    upperNibble = 0;

    for(i = 0; i < self->entryCount; i++)
    {
      u32 sampleCount, patternLength;

      GetData(s, inputStream, patternFieldSize, &upperNibble, &patternLength);
      patternEntries[i].patternLength = patternLength;

      GetData(s, inputStream, countFieldSize, &upperNibble, &sampleCount);
      patternEntries[i].sampleCount = sampleCount;
      self->sampleCount += sampleCount;

      sprintf(msgString, "pattern entry %d: pattern length->%d sample count->%d ", i + 1,
              patternLength, sampleCount);
      inputStream->msg(inputStream, msgString);
    }

    err = ensureSize(self, self->sampleCount * sizeof(u32));
    if(err) goto bail;
    p = &((self->group_index)[0]);

    sprintf(msgString, "Index description values are:");
    inputStream->msg(inputStream, msgString);
    for(i = 0; i < self->entryCount; i++)
    {
      u32 *patternStart = p;
      u32 mask =
          self->fragmentLocalIndexPresent ? 0xFFFFFFFF >> (32 - indexFieldSize + 1) : 0xFFFFFFFF;
      for(j = 0; j < patternEntries[i].sampleCount; j++)
      {
        if(j < patternEntries[i].patternLength)
        {
          u32 indexValue;
          GetData(s, inputStream, indexFieldSize, &upperNibble, &indexValue);
          indexValue = indexValue & mask;
          *p++       = indexValue;
          sprintf(msgString + (i * 2), "%d ", indexValue);
        }
        else
        {
          *p++ = *(patternStart + (j % patternEntries[i].patternLength));
        }
      }
    }
    inputStream->msg(inputStream, msgString);

    free(patternEntries);
    patternEntries = NULL;
  }

bail:
  TEST_RETURN(err);

  return err;
}

MP4Err MP4CreateSampletoGroupAtom(MP4SampletoGroupAtomPtr *outAtom,
                                  sampleToGroupType_t sampleToGroupType)
{
  MP4Err err;
  MP4SampletoGroupAtomPtr self;

  self = (MP4SampletoGroupAtomPtr)calloc(1, sizeof(MP4SampletoGroupAtom));
  TESTMALLOC(self)

  err = MP4CreateFullAtom((MP4AtomPtr)self);
  if(err) goto bail;
  self->sampleToGroupType = sampleToGroupType;
  switch(self->sampleToGroupType)
  {
  case SAMPLE_GROUP_NORMAL:
    self->type = MP4SampletoGroupAtomType;
    self->name = "sample to group";
    break;
  case SAMPLE_GROUP_COMPACT:
    self->type = MP4CompactSampletoGroupAtomType;
    self->name = "compact sample to group";
    break;
  case SAMPLE_GROUP_AUTO:
    self->type = 0;
    self->name = "auto sample to group";
    break;
  default:
    self->type = MP4SampletoGroupAtomType;
    self->name = "(default) sample to group";
    break;
  }
  self->createFromInputStream     = (cisfunc)createFromInputStream;
  self->destroy                   = destroy;
  self->calculateSize             = calculateSize;
  self->serialize                 = serialize;
  self->mapSamplestoGroup         = mapSamplestoGroup;
  self->addSamples                = addSamples;
  self->changeSamplestoGroupType  = changeSamplestoGroupType;
  self->getSampleGroupMap         = getSampleGroupMap;
  self->group_index               = NULL;
  self->sampleCount               = 0;
  self->flags                     = 0;
  self->fragmentLocalIndexPresent = 0;
  memset(&self->compressedGroup, 0, sizeof(CompressedGroupInfo));

  *outAtom = self;
bail:
  TEST_RETURN(err);

  return err;
}
