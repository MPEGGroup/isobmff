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
	DRCInstructionsBasicAtomPtr self = (DRCInstructionsBasicAtomPtr) s;
    err = MP4NoErr;
    
    logMsg(LOGLEVEL_TRACE, "Destroying atom: \"%s\"", self->name);
	if ( self == NULL )
		BAILWITHERROR( MP4BadParamErr )
        
    while (self->additionalDownMixIDs->entryCount > 0)
    {
        DRCInstructionsAdditionalDownMixID *additionalDownMixID;
        MP4GetListEntry(self->additionalDownMixIDs, 0, (char**) &additionalDownMixID);
        free (additionalDownMixID);
        MP4DeleteListEntry(self->additionalDownMixIDs, 0);
    }
    MP4DeleteLinkedList(self->additionalDownMixIDs);
    
    if ( self->super )
        self->super->destroy( s );
bail:
	TEST_RETURN( err );
    
	return;
}

static MP4Err serialize( struct MP4Atom* s, char* buffer )
{
    u8                                  tmp8;
	MP4Err                              err;
	DRCInstructionsBasicAtomPtr         self;
    
    self = (DRCInstructionsBasicAtomPtr) s;
    
    logMsg(LOGLEVEL_TRACE, "Serializing atom: \"%s\"", self->name);
	err = MP4SerializeCommonFullAtomFields( (MP4FullAtomPtr) s, buffer ); if (err) goto bail;
    buffer += self->bytesWritten;
    
    tmp8 = (self->reserved1 << 5) + (self->DRC_set_ID >> 1);
    PUT8_V(tmp8);
    
    tmp8 = (self->DRC_set_ID << 7) + ((self->DRC_location << 2) & 0x7F);
    tmp8 += self->downmix_ID >> 5;
    PUT8_V(tmp8);
    
    tmp8 = (self->downmix_ID << 3) + self->additional_dowmix_ID_count;
    PUT8_V(tmp8);
    
    assert( self->additionalDownMixIDs->entryCount == self->additional_dowmix_ID_count );
    
    for (u8 i = 0; i < self->additional_dowmix_ID_count; i++)
    {
        DRCInstructionsAdditionalDownMixID  *downMixID;
        MP4GetListEntry(self->additionalDownMixIDs, i, (char**) &downMixID);
        tmp8 = (downMixID->reserved << 7) + downMixID->additional_dowmix_ID;
        PUT8_V(tmp8);
    }
    
    PUT16(DRC_set_effect);
    
    if ((self->DRC_set_effect & (1 << 10)) == 0)
    {
        tmp8 = (self->reserved2 << 1) + self->limiter_peak_target_present;
        PUT8_V(tmp8);
        if (self->limiter_peak_target_present == 1)
        {
            PUT8(bs_limiter_peak_target);
        }
    }
    
    tmp8 = (self->reserved3 << 1) + self->DRC_set_target_loudness_present;
    PUT8_V(tmp8);
    if (self->DRC_set_target_loudness_present == 1)
    {
        tmp8 = (self->reserved4 << 4) + (self->bs_DRC_set_target_loudness_value_upper >> 2);
        PUT8_V(tmp8);
        
        tmp8 = (self->bs_DRC_set_target_loudness_value_upper << 6) + self->bs_DRC_set_target_loudness_value_lower;
        PUT8_V(tmp8);
    }
    
	assert( self->bytesWritten == self->size );
bail:
	TEST_RETURN( err );
    
	return err;
}

static MP4Err calculateSize( struct MP4Atom* s )
{
	MP4Err err;
	DRCInstructionsBasicAtomPtr self = (DRCInstructionsBasicAtomPtr) s;
	err = MP4NoErr;
	
    logMsg(LOGLEVEL_TRACE, "Calculating size for atom: \"%s\"", self->name);
	err = MP4CalculateFullAtomFieldSize( (MP4FullAtomPtr) s ); if (err) goto bail;
	
    self->size += 3;
    
    assert( self->additionalDownMixIDs->entryCount == self->additional_dowmix_ID_count );
    
    self->size += self->additional_dowmix_ID_count;
    self->size += 2;
    
    if ((self->DRC_set_effect & (1 << 10)) == 0)
    {
        self->size += 1;
        if (self->limiter_peak_target_present == 1)
        {
            self->size += 1;
        }
    }
    
    self->size += 1;
    if (self->DRC_set_target_loudness_present == 1)
    {
        self->size += 2;
    }
    
    logMsg(LOGLEVEL_TRACE, "Calculating size for atom: \"%s\", Result: size = %d", self->name, self->size);
bail:
	TEST_RETURN( err );
    
	return err;
}

static MP4Err createFromInputStream( MP4AtomPtr s, MP4AtomPtr proto, MP4InputStreamPtr inputStream )
{
    u32     tmp8;
    u32     tmp16;
	MP4Err  err;
	DRCInstructionsBasicAtomPtr self = (DRCInstructionsBasicAtomPtr) s;
	
    logMsg(LOGLEVEL_TRACE, "Creating DRCInstructionsBasicAtom from inputstream");
	err = MP4NoErr;
	if ( self == NULL )	BAILWITHERROR( MP4BadParamErr )
        err = self->super->createFromInputStream( s, proto, (char*) inputStream ); if ( err ) goto bail;
    
    GET16_V(tmp16);
    self->reserved1                     = tmp16 >> 13;
    self->DRC_set_ID                    = (tmp16 >> 7) & 0x003F;
    self->DRC_location                  = (tmp16 >> 2) & 0x001F;
    self->DRC_location                  = self->DRC_location << 3;
    self->DRC_location                  = self->DRC_location >> 3; // get sign right
    self->downmix_ID                    = (tmp16 << 5) & 0x007F;
    
    GET8_V(tmp8);
    self->downmix_ID                    += tmp8 >> 3;
    self->additional_dowmix_ID_count    = tmp8 & 0x07;
    
    for (u8 i = 0; i < self->additional_dowmix_ID_count; i++)
    {
        DRCInstructionsAdditionalDownMixID  *downMixID;
        downMixID = calloc(1, sizeof(DRCInstructionsAdditionalDownMixID));
        GET8_V(tmp8);
        downMixID->reserved             = tmp8 >> 7;
        downMixID->additional_dowmix_ID = tmp8 & 0x7F;
        err = MP4AddListEntry(downMixID, self->additionalDownMixIDs); if ( err ) goto bail;
    }
    
    GET16(DRC_set_effect);
    
    if ((self->DRC_set_effect & (1 << 10)) == 0)
    {
        GET8_V(tmp8);
        self->reserved2                     = tmp8 >> 1;
        self->limiter_peak_target_present   = tmp8 & 0x01;
        if (self->limiter_peak_target_present == 1)
        {
            GET8(bs_limiter_peak_target);
        }
    }
    
    GET8_V(tmp8);
    self->reserved3                         = tmp8 >> 1;
    self->DRC_set_target_loudness_present   = tmp8 & 0x01;
    if (self->DRC_set_target_loudness_present == 1)
    {
        GET16_V(tmp16);
        self->reserved4                                 = tmp16 >> 12;
        self->bs_DRC_set_target_loudness_value_upper    = (tmp16 >> 6) & 0x003F;
        self->bs_DRC_set_target_loudness_value_lower    = tmp16 & 0x003F;
    }

	assert( self->bytesRead == self->size );
bail:
	TEST_RETURN( err );
    
	return err;
}

MP4Err MP4CreateDRCInstructionsBasicAtom( DRCInstructionsBasicAtomPtr *outAtom )
{
	MP4Err err;
	DRCInstructionsBasicAtomPtr self;
	
    logMsg(LOGLEVEL_TRACE, "Initializing DRCInstructionsBasicAtom");
	self = (DRCInstructionsBasicAtomPtr) calloc( 1, sizeof(DRCInstructionsBasicAtom) );
	TESTMALLOC( self );
    
	err = MP4CreateFullAtom( (MP4AtomPtr) self );
	if ( err ) goto bail;
	self->type                      = DRCInstructionsBasicAtomType;
	self->name                      = "drc instructions basic";
	self->createFromInputStream     = (cisfunc) createFromInputStream;
	self->destroy                   = destroy;
	self->calculateSize             = calculateSize;
	self->serialize                 = serialize;
    self->reserved1                 = 0;
    self->reserved2                 = 0;
    self->reserved3                 = 0;
    self->reserved4                 = 0;
    err = MP4MakeLinkedList(&self->additionalDownMixIDs);
    
	*outAtom = self;
bail:
	TEST_RETURN( err );
    
	return err;
}
