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
 derivative works. Copyright (c) 2014.
 */

#include "DRCAtoms.h"
#include "Logger.h"
#include <stdlib.h>
#include <string.h>


static void destroy( MP4AtomPtr s )
{
	MP4Err err;
	DRCCoefficientUniDRCAtomPtr self = (DRCCoefficientUniDRCAtomPtr) s;
    err = MP4NoErr;
    
    logMsg(LOGLEVEL_TRACE, "Destroying atom: \"%s\"", self->name);
	if ( self == NULL )
		BAILWITHERROR( MP4BadParamErr )
        
    while (self->sequences->entryCount > 0)
    {
        DRCCoefficientUniDRCSequence *sequence;
        MP4GetListEntry(self->sequences, 0, (char**) &sequence);
        
        if (sequence->band_count > 0)
        {
            while (sequence->bandCharacteristics->entryCount > 0)
            {
                DRCCoefficientUniDRCSequenceBandCharacteristic *bandCharacteristic;
                MP4GetListEntry(sequence->bandCharacteristics, 0, (char**) &bandCharacteristic);
                free (bandCharacteristic);
                MP4DeleteListEntry(sequence->bandCharacteristics, 0);
            }
            MP4DeleteLinkedList(sequence->bandCharacteristics);
        
            while (sequence->bandIndexes->entryCount > 0)
            {
                DRCCoefficientUniDRCSequenceBandIndex *bandIndex;
                MP4GetListEntry(sequence->bandIndexes, 0, (char**) &bandIndex);
                free (bandIndex);
                MP4DeleteListEntry(sequence->bandIndexes, 0);
            }
            MP4DeleteLinkedList(sequence->bandIndexes);
        }
        free (sequence);
        MP4DeleteListEntry(self->sequences, 0);
    }
    MP4DeleteLinkedList(self->sequences);
        
    if ( self->super )
        self->super->destroy( s );
bail:
	TEST_RETURN( err );
    
	return;
}

static MP4Err serialize( struct MP4Atom* s, char* buffer )
{
    u8                                              tmp8;
    u16                                             tmp16;
	MP4Err                                          err;
    DRCCoefficientUniDRCSequence                    *sequence;
    DRCCoefficientUniDRCSequenceBandCharacteristic  *bandCharacteristic;
    DRCCoefficientUniDRCSequenceBandIndex           *bandIndex;
    
	DRCCoefficientUniDRCAtomPtr self = (DRCCoefficientUniDRCAtomPtr) s;
	err = MP4NoErr;
	
    logMsg(LOGLEVEL_TRACE, "Serializing atom: \"%s\"", self->name);
    
	err = MP4SerializeCommonFullAtomFields( (MP4FullAtomPtr) s, buffer ); if (err) goto bail;
    buffer += self->bytesWritten;
    
    assert( self->sequence_count == self->sequences->entryCount );
    
    tmp8 = (self->reserved1 << 6) + ((self->DRC_location << 1) & 0x3F);
    tmp8 += self->drc_frame_size_present;
    PUT8_V(tmp8);
    
    if (self->drc_frame_size_present == 1)
    {
        tmp16 = (self->reserved2 << 15) + self->bs_drc_frame_size;
        PUT16_V(tmp16);
    }
    
    tmp8 = (self->reserved3 << 7) + (self->delayMode << 6) + self->sequence_count;
    PUT8_V(tmp8);
    
    for (u8 i = 0; i < self->sequence_count; i++)
    {
        MP4GetListEntry(self->sequences, i, (char**) &sequence);
        tmp8 = sequence->reserved1 << 6;
        tmp8 += sequence->gain_coding_profile << 4;
        tmp8 += sequence->gain_interpolation_type << 3;
        tmp8 += sequence->full_frame << 2;
        tmp8 += sequence->time_alignment << 1;
        tmp8 += sequence->time_delta_min_present;
        PUT8_V(tmp8);
        
        if (sequence->time_delta_min_present == 1)
        {
            tmp16 = (sequence->reserved2 << 3);
            tmp16 = (tmp16 << 8) + sequence->bs_time_delta_min;
            PUT16_V(tmp16);
        }
        
        if (sequence->gain_coding_profile != 3)
        {
            tmp8 = (sequence->reserved3 << 5);
            tmp8 += (sequence->band_count << 1);
            tmp8 += sequence->drc_band_type;
            PUT8_V(tmp8);
            
            assert( sequence->bandCharacteristics->entryCount   == sequence->band_count );
            
            for (u8 j = 0; j < sequence->band_count; j++)
            {
                MP4GetListEntry(sequence->bandCharacteristics, j, (char**) &bandCharacteristic);
                tmp8 = bandCharacteristic->reserved << 7;
                tmp8 += bandCharacteristic->DRC_characteristic;
                PUT8_V(tmp8);
            }
            
            for (u8 j = 1; j < sequence->band_count; j++)
            {
                MP4GetListEntry(sequence->bandIndexes, j - 1, (char**) &bandIndex);
                
                if (sequence->drc_band_type == 1)
                {
                    tmp8 = bandIndex->reserved1 << 4;
                    tmp8 += bandIndex->crossover_freq_index;
                    PUT8_V(tmp8);
                }
                else
                {
                    tmp16 = (bandIndex->reserved2 << 2);
                    tmp16 = (tmp16 << 8) + bandIndex->start_sub_band_index;
                    PUT16_V(tmp16);
                }
            }
        }
    }
    
	assert( self->bytesWritten == self->size );
bail:
	TEST_RETURN( err );
    
	return err;
}

static MP4Err calculateSize( struct MP4Atom* s )
{
	MP4Err                                          err;
    DRCCoefficientUniDRCSequence                    *sequence;
	DRCCoefficientUniDRCAtomPtr                     self;
    
    self    = (DRCCoefficientUniDRCAtomPtr) s;
    
    logMsg(LOGLEVEL_TRACE, "Calculating size for atom: \"%s\"", self->name);
	err     = MP4CalculateFullAtomFieldSize( (MP4FullAtomPtr) s ); if (err) goto bail;
	
    self->size += 2;
    
    if (self->drc_frame_size_present == 1)
    {
        self->size += 2;
    }
    
    for (u8 i = 0; i < self->sequence_count; i++)
    {
        MP4GetListEntry(self->sequences, i, (char**) &sequence);
        
        self->size += 1;
        
        if (sequence->time_delta_min_present == 1)
        {
            self->size += 2;
        }
        
        if (sequence->gain_coding_profile != 3)        {
            self->size += 1;
            
            assert( sequence->bandCharacteristics->entryCount   == sequence->band_count );
            
            self->size += sequence->band_count * 1;
            
            if (sequence->drc_band_type == 1)
            {
                self->size += (sequence->band_count - 1) * 1;
            }
            else
            {
                self->size += (sequence->band_count - 1) * 2;
            }
        }
    }

    logMsg(LOGLEVEL_TRACE, "Calculating size for atom: \"%s\", Result: size = %d", self->name, self->size);
bail:
	TEST_RETURN( err );
    
	return err;
}

static MP4Err createFromInputStream( MP4AtomPtr s, MP4AtomPtr proto, MP4InputStreamPtr inputStream )
{
    u32         tmp8;
    u32         tmp16;
	MP4Err      err;
	DRCCoefficientUniDRCAtomPtr self = (DRCCoefficientUniDRCAtomPtr) s;
	
	err = MP4NoErr;
    
    logMsg(LOGLEVEL_TRACE, "Creating DRCCoefficientUniDRCAtom from inputstream");
	if ( self == NULL )	BAILWITHERROR( MP4BadParamErr )
        err = self->super->createFromInputStream( s, proto, (char*) inputStream ); if ( err ) goto bail;
    
    GET8_V(tmp8);
    self->reserved1                 = tmp8 >> 6;
    self->DRC_location              = (tmp8 >> 1) & 0x1F;
    self->DRC_location              = self->DRC_location << 3;
    self->DRC_location              = self->DRC_location >> 3; // get sign right
    self->drc_frame_size_present    = tmp8 & 0x01;
    
    if (self->drc_frame_size_present == 1)
    {
        GET16_V(tmp16);
        self->reserved2             = tmp16 >> 15;
        self->bs_drc_frame_size     = tmp16 & 0x7FFF;
    }
    
    GET8_V(tmp8);
    self->reserved3                 = tmp8 >> 7;
    self->delayMode                 = (tmp8 >> 6) & 0x01;
    self->sequence_count            = tmp8 & 0x3F;
    
    for (u8 i = 0; i < self->sequence_count; i++)
    {
        DRCCoefficientUniDRCSequence *sequence;
        sequence = calloc(1, sizeof(DRCCoefficientUniDRCSequence));
        
        GET8_V(tmp8);
        sequence->reserved1                 = tmp8 >> 6;
        sequence->gain_coding_profile       = (tmp8 >> 4) & 0x03;
        sequence->gain_interpolation_type   = (tmp8 >> 3) & 0x01;
        sequence->full_frame                = (tmp8 >> 2) & 0x01;
        sequence->time_alignment            = (tmp8 >> 1) & 0x01;
        sequence->time_delta_min_present    = tmp8 & 0x01;
        
        if (sequence->time_delta_min_present == 1)
        {
            GET16_V(tmp16);
            sequence->reserved2             = tmp16 >> 11;
            sequence->bs_time_delta_min     = tmp16 & 0x07FF;
        }
        
        if (sequence->gain_coding_profile != 3)
        {
            GET8_V(tmp8);
            sequence->reserved3             = tmp8 >> 5;
            sequence->band_count            = (tmp8 >> 1) & 0x0F;
            sequence->drc_band_type         = tmp8 & 0x01;
            
            err = MP4MakeLinkedList(&sequence->bandCharacteristics); if ( err ) goto bail;
            for (u8 j = 0; j < sequence->band_count; j++)
            {
                DRCCoefficientUniDRCSequenceBandCharacteristic  *bandCharacteristic;
                bandCharacteristic = calloc(1, sizeof(DRCCoefficientUniDRCSequenceBandCharacteristic));
                GET8_V(tmp8);
                bandCharacteristic->reserved            = tmp8 >> 7;
                if (bandCharacteristic->reserved != 0)
                {
                    logMsg(LOGLEVEL_ERROR, "Reserved should alwayse be 0!");
                }
                bandCharacteristic->DRC_characteristic  = tmp8 & 0x7F;
                err = MP4AddListEntry(bandCharacteristic, sequence->bandCharacteristics); if ( err ) goto bail;
            }
            
            err = MP4MakeLinkedList(&sequence->bandIndexes); if ( err ) goto bail;
            for (u8 j = 1; j < sequence->band_count; j++)
            {
                DRCCoefficientUniDRCSequenceBandIndex *bandIndex;
                bandIndex = calloc(1, sizeof(DRCCoefficientUniDRCSequenceBandIndex));
                if (sequence->drc_band_type == 1)
                {
                    GET8_V(tmp8);
                    bandIndex->reserved1            = tmp8 >> 4;
                    bandIndex->crossover_freq_index = tmp8 & 0x0F;
                }
                else
                {
                    GET16_V(tmp16);
                    bandIndex->reserved2            = tmp16 >> 10;
                    bandIndex->start_sub_band_index = tmp16 & 0x03FF;
                }
                err = MP4AddListEntry(bandIndex, sequence->bandIndexes); if ( err ) goto bail;
            }
        }
        err = MP4AddListEntry(sequence, self->sequences); if ( err ) goto bail;
    }

	assert( self->bytesRead == self->size );
bail:
	TEST_RETURN( err );
    
	return err;
}

MP4Err MP4CreateDRCCoefficientUniDRCAtom( DRCCoefficientUniDRCAtomPtr *outAtom )
{
	MP4Err err;
	DRCCoefficientUniDRCAtomPtr self;
	
    logMsg(LOGLEVEL_TRACE, "Initializing DRCCoefficientUniDRCAtom");
	self = (DRCCoefficientUniDRCAtomPtr) calloc( 1, sizeof(DRCCoefficientUniDRCAtom) );
	TESTMALLOC( self );
    
	err = MP4CreateFullAtom( (MP4AtomPtr) self );
	if ( err ) goto bail;
	self->type                      = DRCCoefficientUniDRCAtomType;
	self->name                      = "drc coefficient uniDrc";
	self->createFromInputStream     = (cisfunc) createFromInputStream;
	self->destroy                   = destroy;
	self->calculateSize             = calculateSize;
	self->serialize                 = serialize;
    self->reserved1                 = 0;
    self->reserved2                 = 0;
    self->reserved3                 = 0;
    
    err = MP4MakeLinkedList(&self->sequences); if ( err ) goto bail;
    
	*outAtom = self;
bail:
	TEST_RETURN( err );
    
	return err;
}
