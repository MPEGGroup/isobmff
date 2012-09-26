/*
This software module was originally developed by Apple Computer, Inc. and Ericsson Research
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
products. The copyright owners retain full right to use the code for its own
purpose, assign or donate the code to a third party and to
inhibit third parties from using the code for non 
MPEG-4 conforming products.
This copyright notice must be included in all copies or
derivative works. Copyright (c) Apple Computer and Telefonaktiebolaget LM Ericsson 2001
*/
/*
	$Id: AMRSpecificInfoAtom.c,v 1.2 2001/10/25 15:53:24 erapefh Exp $
*/

#include "MP4Atoms.h"
#include <stdlib.h>

static void destroy( MP4AtomPtr s )
{
	MP4Err err;
	MP4AMRSpecificInfoAtomPtr self;
	err = MP4NoErr;
	self = (MP4AMRSpecificInfoAtomPtr) s;
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
	MP4Err err;
	MP4AMRSpecificInfoAtomPtr self = (MP4AMRSpecificInfoAtomPtr) s;
	err = MP4NoErr;
	
	err = MP4SerializeCommonBaseAtomFields( s, buffer ); if (err) goto bail;
    buffer += self->bytesWritten;
	PUT32(vendor );
	PUT8( decoder_version );
	PUT16(mode_set );
	PUT8( mode_change_period );
	PUT8( frames_per_sample );

	assert( self->bytesWritten == self->size );
bail:
	TEST_RETURN( err );

	return err;
}

static MP4Err calculateSize( struct MP4Atom* s )
{
	MP4Err err;
	MP4AMRSpecificInfoAtomPtr self = (MP4AMRSpecificInfoAtomPtr) s;
	err = MP4NoErr;
	
	err = MP4CalculateBaseAtomFieldSize( s ); if (err) goto bail;
	self->size += 4+2+(3*1);
bail:
	TEST_RETURN( err );

	return err;
}

static MP4Err createFromInputStream( MP4AtomPtr s, MP4AtomPtr proto, MP4InputStreamPtr inputStream )
{
	MP4Err err;
	MP4AMRSpecificInfoAtomPtr self = (MP4AMRSpecificInfoAtomPtr) s;
	
	err = MP4NoErr;
	if ( self == NULL )
		BAILWITHERROR( MP4BadParamErr )
	err = self->super->createFromInputStream( s, proto, (char*) inputStream ); if ( err ) goto bail;

	GET32(vendor );
	GET8( decoder_version );
	GET16(mode_set );
	GET8( mode_change_period );
	GET8( frames_per_sample );
	
bail:
	TEST_RETURN( err );

	return err;
}

MP4Err MP4CreateAMRSpecificInfoAtom( MP4AMRSpecificInfoAtomPtr *outAtom )
{
	MP4Err err;
	MP4AMRSpecificInfoAtomPtr self;
	
	self = (MP4AMRSpecificInfoAtomPtr) calloc( 1, sizeof(MP4AMRSpecificInfoAtom) );
	TESTMALLOC( self );

	err = MP4CreateBaseAtom( (MP4AtomPtr) self );
	if ( err ) goto bail;
	self->type =                MP4AMRSpecificInfoAtomType;
	self->name                  = "AMR specific info";
	self->createFromInputStream = (cisfunc) createFromInputStream;
	self->destroy               = destroy;
	self->calculateSize         = calculateSize;
	self->serialize             = serialize;
	*outAtom = self;
bail:
	TEST_RETURN( err );

	return err;
}
