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
/*
	$Id: PaddingBitsAtom.c,v 1.1.1.1 2002/09/20 08:53:35 julien Exp $
*/

#include "MP4Atoms.h"
#include <stdlib.h>
#include <string.h>

static void destroy( MP4AtomPtr s )
{
	MP4Err err;
	MP4PaddingBitsAtomPtr self;
	err = MP4NoErr;
	self = (MP4PaddingBitsAtomPtr) s;
	if ( self == NULL )	BAILWITHERROR( MP4BadParamErr )
	if ( self->pads )
	{
		free( self->pads );
		self->pads = NULL;
	}
	if ( self->super )
		self->super->destroy( s );
bail:
	TEST_RETURN( err );

	return;
}

static MP4Err addSamplePads( struct MP4PaddingBitsAtom *self, u32 sampleCount, MP4Handle padsH )
{
	MP4Err err;
	u8 *PaddingBits;
	u32 padsCount;
	u8 *newpads;
	u8 defaultpad;
	u8 dodefault;
	u32 i;
	
	err = MP4NoErr;
	
	dodefault =1;
	defaultpad=0;
	PaddingBits = NULL;
	
	if (padsH!=NULL) 
	{
	  PaddingBits = (u8 *) *padsH; 
	  err = MP4GetHandleSize( padsH, &padsCount ); if (err) goto bail;
	  switch (padsCount) {
	  	case 0:  break;
	  	case 1:  defaultpad=*PaddingBits; break;
	  	default: dodefault=0; assert( sampleCount==padsCount ); break;
	  }
	}
	
	newpads = (u8*) calloc( sampleCount + self->sampleCount + 1, sizeof(u8) );
		/* we add so we can easily pack up (see below) */
	TESTMALLOC( newpads );
	if ( self->sampleCount > 0) 
	  memcpy( newpads, self->pads, self->sampleCount );
	
	if (dodefault==0) {
	  memcpy( newpads + self->sampleCount, PaddingBits, padsCount );
	}
	else
	{
	  for (i=0; i<sampleCount; i++) 
	    newpads[i + self->sampleCount] = defaultpad;
	}

	if ( self->sampleCount > 0) free( self->pads );
	self->pads = newpads;
	
	self->sampleCount += sampleCount;
bail:
	TEST_RETURN( err );

	return err;
}

static MP4Err getPaddingBits( MP4AtomPtr s, u32 sampleNumber, u8 *outPad )
{
	MP4Err err;
	MP4PaddingBitsAtomPtr self = (MP4PaddingBitsAtomPtr) s;
	
	err = MP4NoErr;
	if ( (self == NULL) || (outPad == NULL) || (sampleNumber > self->sampleCount) || (sampleNumber == 0) )
		BAILWITHERROR( MP4BadParamErr )
	
	if (self->pads == NULL) *outPad = 0; 
	else 
	  *outPad = self->pads[sampleNumber - 1];
	
	
bail:
	TEST_RETURN( err );

	return err;
}

static MP4Err serialize( struct MP4Atom* s, char* buffer )
{
	MP4Err err;
	u32 i;
	MP4PaddingBitsAtomPtr self = (MP4PaddingBitsAtomPtr) s;
	err = MP4NoErr;
	
	err = MP4SerializeCommonFullAtomFields( (MP4FullAtomPtr) s, buffer ); if (err) goto bail;
    buffer += self->bytesWritten;
    PUT32( sampleCount );
	for ( i = 0; i < ((self->sampleCount+1)/2); i++ )
	{
		PUT8_V( (self->pads[i*2] & 7) | ((self->pads[i*2+1] & 7)<<4) );
	}
	assert( self->bytesWritten == self->size );
bail:
	TEST_RETURN( err );

	return err;
}

static MP4Err calculateSize( struct MP4Atom* s )
{
	MP4Err err;
	MP4PaddingBitsAtomPtr self = (MP4PaddingBitsAtomPtr) s;
	err = MP4NoErr;
	
	err = MP4CalculateFullAtomFieldSize( (MP4FullAtomPtr) s ); if (err) goto bail;
	self->size += ((self->sampleCount+1)/2) + 4;
bail:
	TEST_RETURN( err );

	return err;
}


static MP4Err createFromInputStream( MP4AtomPtr s, MP4AtomPtr proto, MP4InputStreamPtr inputStream )
{
	MP4Err err;
	u32 entries;
	u8* p;
	MP4PaddingBitsAtomPtr self = (MP4PaddingBitsAtomPtr) s;
	
	err = MP4NoErr;
	if ( self == NULL )	BAILWITHERROR( MP4BadParamErr )
	err = self->super->createFromInputStream( s, proto, (char*) inputStream ); if ( err ) goto bail;
	
	GET32( sampleCount );
	
	self->pads = (u8 *) calloc( self->sampleCount + 1, sizeof(u8) );
	TESTMALLOC( self->pads );
	p = self->pads;
	for ( entries = 0; entries < self->sampleCount; entries = entries + 2 )
	{
		u32 pad;
		GET8_V_MSG( pad, NULL );
		*p = pad & 7; p++; pad = pad >> 4;
		*p = pad & 7; p++;
	}
	
bail:
	TEST_RETURN( err );

	return err;
}

MP4Err MP4CreatePaddingBitsAtom( MP4PaddingBitsAtomPtr *outAtom )
{
	MP4Err err;
	MP4PaddingBitsAtomPtr self;
	
	self = (MP4PaddingBitsAtomPtr) calloc( 1, sizeof(MP4PaddingBitsAtom) );
	TESTMALLOC( self )

	err = MP4CreateFullAtom( (MP4AtomPtr) self );
	if ( err ) goto bail;
	self->type = MP4PaddingBitsAtomType;
	self->name                = "padding bits";
	self->createFromInputStream = (cisfunc) createFromInputStream;
	self->destroy             = destroy;
	self->getSamplePad        = getPaddingBits;
	self->calculateSize       = calculateSize;
	self->serialize           = serialize;
	self->addSamplePads       = addSamplePads;
	*outAtom = self;
bail:
	TEST_RETURN( err );

	return err;
}
