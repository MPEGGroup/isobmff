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
	DRCCoefficientBasicAtomPtr self = (DRCCoefficientBasicAtomPtr) s;
    err = MP4NoErr;
    
    logMsg(LOGLEVEL_TRACE, "Destroying atom: \"%s\"", self->name);
	if ( self == NULL )
		BAILWITHERROR( MP4BadParamErr )
        
        if ( self->super )
            self->super->destroy( s );
bail:
	TEST_RETURN( err );
    
	return;
}

static MP4Err serialize( struct MP4Atom* s, char* buffer )
{
    u8      tmp8;
	MP4Err  err;
	DRCCoefficientBasicAtomPtr self = (DRCCoefficientBasicAtomPtr) s;
	err = MP4NoErr;
	
    logMsg(LOGLEVEL_TRACE, "Serializing atom: \"%s\"", self->name);
    
	err = MP4SerializeCommonFullAtomFields( (MP4FullAtomPtr) s, buffer ); if (err) goto bail;
    buffer += self->bytesWritten;
    
    tmp8 = (self->reserved << 4) + ((self->DRC_location >> 1) & 0x0F);
    PUT8_V(tmp8);
    tmp8 = (self->DRC_location << 7) + self->DRC_characteristic;
    PUT8_V(tmp8);
    
	assert( self->bytesWritten == self->size );
bail:
	TEST_RETURN( err );
    
	return err;
}

static MP4Err calculateSize( struct MP4Atom* s )
{
	MP4Err err;
	DRCCoefficientBasicAtomPtr self = (DRCCoefficientBasicAtomPtr) s;
	err = MP4NoErr;
	
    logMsg(LOGLEVEL_TRACE, "Calculating size for atom: \"%s\"", self->name);
	err = MP4CalculateFullAtomFieldSize( (MP4FullAtomPtr) s ); if (err) goto bail;
	
    self->size += 2;
    
    logMsg(LOGLEVEL_TRACE, "Calculating size for atom: \"%s\", Result: size = %d", self->name, self->size);
bail:
	TEST_RETURN( err );
    
	return err;
}

static MP4Err createFromInputStream( MP4AtomPtr s, MP4AtomPtr proto, MP4InputStreamPtr inputStream )
{
    u32         tmp8;
	MP4Err      err;
	DRCCoefficientBasicAtomPtr self = (DRCCoefficientBasicAtomPtr) s;
	
    logMsg(LOGLEVEL_TRACE, "Creating DRCCoefficientBasicAtom from inputstream");
	err = MP4NoErr;
	if ( self == NULL )	BAILWITHERROR( MP4BadParamErr )
        err = self->super->createFromInputStream( s, proto, (char*) inputStream ); if ( err ) goto bail;
    
    GET8_V(tmp8);
    self->reserved              = tmp8 >> 4;
    self->DRC_location          = (tmp8 & 0x0F) << 1;
    GET8_V(tmp8);
    self->DRC_location          += tmp8 >> 7;
    self->DRC_location          = self->DRC_location << 3;
    self->DRC_location          = self->DRC_location >> 3; // get sign right
    self->DRC_characteristic    = tmp8 & 0x7F;
    
	assert( self->bytesRead == self->size );
bail:
	TEST_RETURN( err );
    
	return err;
}

MP4Err MP4CreateDRCCoefficientBasicAtom( DRCCoefficientBasicAtomPtr *outAtom )
{
	MP4Err err;
	DRCCoefficientBasicAtomPtr self;
	
    logMsg(LOGLEVEL_TRACE, "Initializing DRCCoefficientBasicAtom");
	self = (DRCCoefficientBasicAtomPtr) calloc( 1, sizeof(DRCCoefficientBasicAtom) );
	TESTMALLOC( self );
    
	err = MP4CreateFullAtom( (MP4AtomPtr) self );
	if ( err ) goto bail;
	self->type                      = DRCCoefficientBasicAtomType;
	self->name                      = "drc coefficient basic";
	self->createFromInputStream     = (cisfunc) createFromInputStream;
	self->destroy                   = destroy;
	self->calculateSize             = calculateSize;
	self->serialize                 = serialize;
    self->reserved                  = 0;
    
	*outAtom = self;
bail:
	TEST_RETURN( err );
    
	return err;
}
