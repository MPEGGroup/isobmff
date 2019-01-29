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
derivative works. Copyright (c) 2019.
*/

#include "Logger.h"
#include "PCMFormatData.h"
#include "Testing.h"

#include <time.h>
#include <stdlib.h>
#include <math.h>

#define CHECK( pt1, pt2, varname ) \
if (pt1->varname != pt2->varname) { \
logMsg(LOGLEVEL_ERROR, "Comparing %s->%s failed! Original: %d, Parsed: %d", #pt1, #varname, pt1->varname, pt2->varname); \
err = MP4BadDataErr; goto bail; }

u16  getTestInt(u8 length)
{
    u8 boundaryTest;
    
    boundaryTest = rand() % 5;
    if (boundaryTest == 0)
        return 0;
    
    if (boundaryTest == 4)
        return (pow(2, length) -1);
    
    return rand() % ((int) pow(2, length));
}


MP4Err testChannelLayoutAtom()
{
    MP4Err                      err;
    MP4ChannelLayoutAtomPtr     atom;
    MP4Handle                   dataH;
    u32                         size;
    char                        *buffer;
    MP4AtomPtr                  parsedAtom;
    MP4ChannelLayoutAtomPtr     parsedLayoutAtom;
    MP4InputStreamPtr           is;
    
    err = MP4NoErr;
    MP4CreateChannelLayoutAtom(&atom);

    atom->version          = getTestInt(1);
    if (atom->version == 0) {
        atom->stream_structure          = getTestInt(1) + 1;
        if (atom->stream_structure & 1) /* channelStructured */
        {
            atom->definedLayout          = getTestInt(8);
            if (atom->definedLayout == 0)
            {
                atom->channelCount          = getTestInt(4); //actually u16
                for (u8 i = 0; i < atom->channelCount; i++)
                {
                    MP4ChannelLayoutDefinedLayout *definedLayoutStruct;
                    definedLayoutStruct = calloc(1, sizeof(MP4ChannelLayoutDefinedLayout));

                    definedLayoutStruct->speaker_position = getTestInt(8);
                    if (definedLayoutStruct->speaker_position == SPEAKER_POSITION_EXPLICIT)
                    {
                        definedLayoutStruct->elevation = getTestInt(7);
                        definedLayoutStruct->azimuth = getTestInt(8);
                    }
                    else
                    {
                        definedLayoutStruct->speaker_position = getTestInt(7);
                    }
                    MP4AddListEntry(definedLayoutStruct, atom->definedLayouts);
                }
            }
            else
            {
                atom->omittedChannelsMap = getTestInt(16);
                atom->omittedChannelsMap = atom->omittedChannelsMap << 16;
                atom->omittedChannelsMap += getTestInt(16);
                atom->omittedChannelsMap = atom->omittedChannelsMap << 16;
                atom->omittedChannelsMap += getTestInt(16);
                atom->omittedChannelsMap = atom->omittedChannelsMap << 16;
                atom->omittedChannelsMap += getTestInt(16);
            }
        }

        if (atom->stream_structure & 2) /* objectStructured */
        {
            atom->object_count += getTestInt(8);
        }
    }
    else { // version > 0
        atom->stream_structure          = getTestInt(1) + 1;
        atom->formatOrdering            = getTestInt(2);
        if (atom->formatOrdering > 2) atom->formatOrdering  = 2;
        do {
            atom->baseChannelCount = getTestInt(8);
        }
        while (atom->baseChannelCount == 0);
        if (atom->stream_structure & STREAM_STRUCTURE_CHANNELS) /* channelStructured */
        {
            atom->definedLayout          = getTestInt(8);
            if (atom->definedLayout == 0)
            {
                do {
                    atom->layoutChannelCount = getTestInt(8);
                }
                while (atom->layoutChannelCount > atom->baseChannelCount || atom->layoutChannelCount == 0);
                for (u8 i = 0; i < atom->layoutChannelCount; i++)
                {
                    MP4ChannelLayoutDefinedLayout *definedLayoutStruct;
                    definedLayoutStruct = calloc(1, sizeof(MP4ChannelLayoutDefinedLayout));

                    definedLayoutStruct->speaker_position = getTestInt(8);
                    if (definedLayoutStruct->speaker_position == SPEAKER_POSITION_EXPLICIT)
                    {
                        definedLayoutStruct->azimuth = getTestInt(8);
                        definedLayoutStruct->elevation = getTestInt(16);
                    }
                    else {
                        definedLayoutStruct->azimuth = 0;
                        definedLayoutStruct->elevation = 0;
                    }
                    MP4AddListEntry(definedLayoutStruct, atom->definedLayouts);
                }
            }
            else
            {
                atom->channelOrderDefinition = getTestInt(3);
                atom->omittedChannelsPresent = getTestInt(1);
                if(atom->omittedChannelsPresent) {
                    atom->omittedChannelsMap = getTestInt(16);
                    atom->omittedChannelsMap = atom->omittedChannelsMap << 16;
                    atom->omittedChannelsMap += getTestInt(16);
                    atom->omittedChannelsMap = atom->omittedChannelsMap << 16;
                    atom->omittedChannelsMap += getTestInt(16);
                    atom->omittedChannelsMap = atom->omittedChannelsMap << 16;
                    atom->omittedChannelsMap += getTestInt(16);
                }
            }
        }
    }
    atom->calculateSize((MP4AtomPtr) atom);
    size    = atom->size;
    err     = MP4NewHandle( 0, &dataH );          if (err) goto bail;
    err     = MP4SetHandleSize( dataH, size );  if (err) goto bail;
    
    buffer  = (char *) *dataH;
    atom->serialize((MP4AtomPtr) atom, buffer);
    
    err = MP4CreateMemoryInputStream( *dataH, size, &is );      if (err) goto bail;
    is->debugging = 0;
    err = MP4ParsePCMAtom( is, &parsedAtom);
    if (err) {
        logMsg(LOGLEVEL_ERROR, "(MP4ParsePCMAtom) Error parsing PCM atom");
        goto bail;
    }
    if (parsedAtom->type != MP4ChannelLayoutAtomType)
    {
        logMsg(LOGLEVEL_ERROR, "(MP4ChannelLayoutAtom) Parsed Atomtype invalid!");
        err = MP4BadDataErr; goto bail;
    }
    
    parsedLayoutAtom = (MP4ChannelLayoutAtomPtr) parsedAtom;

    CHECK(atom, parsedLayoutAtom, version);
    if (atom->version == 0) {
        CHECK(atom, parsedLayoutAtom, stream_structure);
        if (atom->stream_structure & STREAM_STRUCTURE_CHANNELS)
        {
            CHECK(atom, parsedLayoutAtom, definedLayout);
            if (atom->definedLayout == 0)
            {
                for (u8 i = 0; i < atom->channelCount; i++)
                {
                    MP4ChannelLayoutDefinedLayout *definedLayoutStructOrig;
                    MP4ChannelLayoutDefinedLayout *definedLayoutStructParsed;
                    MP4GetListEntry(atom->definedLayouts, i, (char**) &definedLayoutStructOrig);
                    MP4GetListEntry(parsedLayoutAtom->definedLayouts, i, (char**) &definedLayoutStructParsed);
                    CHECK(definedLayoutStructOrig, definedLayoutStructParsed, speaker_position);
                    if (definedLayoutStructOrig->speaker_position == SPEAKER_POSITION_EXPLICIT)
                    {
                        CHECK(definedLayoutStructOrig, definedLayoutStructParsed, elevation);
                        CHECK(definedLayoutStructOrig, definedLayoutStructParsed, azimuth);
                    }
                }
            }
            else
            {
                CHECK(atom, parsedLayoutAtom, omittedChannelsMap);
            }
        }

        if (atom->stream_structure & STREAM_STRUCTURE_OBJECTS)
        {
            CHECK(atom, parsedLayoutAtom, object_count);
        }
    }
    else { // version > 0
        CHECK(atom, parsedLayoutAtom, stream_structure);
        CHECK(atom, parsedLayoutAtom, formatOrdering);
        CHECK(atom, parsedLayoutAtom, baseChannelCount);
        if (atom->stream_structure & STREAM_STRUCTURE_CHANNELS)
        {
            CHECK(atom, parsedLayoutAtom, definedLayout);
            if (atom->definedLayout == 0)
            {
                CHECK(atom, parsedLayoutAtom, layoutChannelCount);
                for (u8 i = 0; i < atom->layoutChannelCount; i++)
                {
                    MP4ChannelLayoutDefinedLayout *definedLayoutStructOrig;
                    MP4ChannelLayoutDefinedLayout *definedLayoutStructParsed;
                    MP4GetListEntry(atom->definedLayouts, i, (char**) &definedLayoutStructOrig);
                    MP4GetListEntry(parsedLayoutAtom->definedLayouts, i, (char**) &definedLayoutStructParsed);
                    CHECK(definedLayoutStructOrig, definedLayoutStructParsed, speaker_position);
                    if (definedLayoutStructOrig->speaker_position == SPEAKER_POSITION_EXPLICIT)
                    {
                        CHECK(definedLayoutStructOrig, definedLayoutStructParsed, elevation);
                        CHECK(definedLayoutStructOrig, definedLayoutStructParsed, azimuth);
                    }
                }
            }
            else
            {
                CHECK(atom, parsedLayoutAtom, omittedChannelsPresent);
                if (atom->omittedChannelsPresent) {
                    CHECK(atom, parsedLayoutAtom, omittedChannelsMap);
                }
            }
        }
    }
    atom->destroy((MP4AtomPtr) atom);
    parsedAtom->destroy((MP4AtomPtr) parsedAtom);
    free (is);
    free (buffer);
    
    logMsg(LOGLEVEL_TRACE, "MP4ChannelLayoutAtom  test successfull!");
bail:
    return err;
}

MP4Err testPCMConfigAtom()
{
    MP4Err                      err;
    MP4PCMConfigAtomPtr         atom;
    MP4Handle                   dataH;
    u32                         size;
    char                        *buffer;
    MP4AtomPtr                  parsedAtom;
    MP4PCMConfigAtomPtr         parsedPCMAtom;
    MP4InputStreamPtr           is;
    PCMFormatData               pcmFormatData;
    
    err = MP4NoErr;
    initPCMFormatData(&pcmFormatData);
    MP4CreatePCMConfigAtom(&atom);
    
    atom->format_flags          = getTestInt(8);
    atom->PCM_sample_size       = getTestInt(8);

    atom->calculateSize((MP4AtomPtr) atom);
    size    = atom->size;
    err     = MP4NewHandle( 0, &dataH );        if (err) goto bail;
    err     = MP4SetHandleSize( dataH, size );  if (err) goto bail;
    
    buffer  = (char *) *dataH;
    atom->serialize((MP4AtomPtr) atom, buffer);
    
    err = MP4CreateMemoryInputStream( *dataH, size, &is );      if (err) goto bail;
    is->debugging = 0;
    err = MP4ParsePCMAtom( is, &parsedAtom) ; if (err) goto bail;
    
    if (parsedAtom->type != MP4PCMConfigAtomType)
    {
        logMsg(LOGLEVEL_ERROR, "(testPCMConfigAtom) Parsed Atomtype invalid!");
        err = MP4BadDataErr; goto bail;
    }
    
    parsedPCMAtom = (MP4PCMConfigAtomPtr) parsedAtom;
    
    CHECK(atom, parsedPCMAtom, format_flags);
    CHECK(atom, parsedPCMAtom, PCM_sample_size);
    
    atom->destroy((MP4AtomPtr) atom);
    parsedAtom->destroy((MP4AtomPtr) parsedAtom);
    free (is);
    free (buffer);
    
    logMsg(LOGLEVEL_TRACE, "PCMConfigAtom test successfull!");
bail:
    return err;
}

MP4Err testAll(u32 iterations)
{
    MP4Err err = MP4NoErr;
    srand((unsigned int) time(NULL));
    for (u32 i = 0; i < iterations; i++)
    {
        err = testChannelLayoutAtom();  if (err) goto bail;
        err = testPCMConfigAtom();      if (err) goto bail;
    }
bail:
    return err;
}
