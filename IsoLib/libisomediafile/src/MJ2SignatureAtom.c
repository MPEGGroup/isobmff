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
	$Id: MJ2SignatureAtom.c,v 1.1.1.1 2002/09/20 08:53:34 julien Exp $
*/

#include "MJ2Atoms.h"
#include <stdlib.h>
#include <string.h>

static void destroy( MP4AtomPtr s )
{
	ISOErr err = ISONoErr;
	MJ2JPEG2000SignatureAtomPtr self = (MJ2JPEG2000SignatureAtomPtr) s;
	
	if ( self == NULL )
		BAILWITHERROR( ISOBadParamErr )
	
	if ( self->super )
		self->super->destroy( s );
		
bail:
	TEST_RETURN( err );

	return;
}

static ISOErr serialize( struct MP4Atom* s, char* buffer )
{
	ISOErr err;
	MJ2JPEG2000SignatureAtomPtr self = (MJ2JPEG2000SignatureAtomPtr) s;
	err = ISONoErr;
	
	err = MP4SerializeCommonBaseAtomFields( s, buffer ); if (err) goto bail;
    buffer += self->bytesWritten;
    PUTBYTES( &self->signature, MJ2JPEG2000SignatureSize );
	assert( self->bytesWritten == self->size );
bail:
	TEST_RETURN( err );

	return err;
}

static ISOErr calculateSize( struct MP4Atom* s )
{
	ISOErr err;
	MJ2JPEG2000SignatureAtomPtr self = (MJ2JPEG2000SignatureAtomPtr) s;
	err = ISONoErr;
	
	err = MP4CalculateBaseAtomFieldSize( s ); if (err) goto bail;
	self->size += MJ2JPEG2000SignatureSize;
bail:
	TEST_RETURN( err );

	return err;
}


static ISOErr createFromInputStream( MP4AtomPtr s, MP4AtomPtr proto, MP4InputStreamPtr inputStream )
{
	ISOErr err;
	MJ2JPEG2000SignatureAtomPtr self = (MJ2JPEG2000SignatureAtomPtr) s;
	
	err = ISONoErr;
	if ( self == NULL )
		BAILWITHERROR( MP4BadParamErr )
	err = self->super->createFromInputStream( s, proto, (char*) inputStream );

	GET32( signature );	
bail:
	TEST_RETURN( err );

	return err;
}

ISOErr MJ2CreateSignatureAtom( MJ2JPEG2000SignatureAtomPtr *outAtom )
{
	ISOErr err;
	MJ2JPEG2000SignatureAtomPtr self;
	
	self = (MJ2JPEG2000SignatureAtomPtr) calloc( 1, sizeof(MJ2JPEG2000SignatureAtom) );
	TESTMALLOC( self );

	err = MP4CreateBaseAtom( (MP4AtomPtr) self );
	if ( err ) goto bail;
	
	self->type                	= MJ2JPEG2000SignatureAtomType;
	self->name               	= "JPEG 2000 signature atom";
	self->destroy             	= destroy;
	self->createFromInputStream = (cisfunc) createFromInputStream;
	self->calculateSize         = calculateSize;
	self->serialize             = serialize;	
	self->signature             = MJ2JPEG2000Signature;
	*outAtom = self;
bail:
	TEST_RETURN( err );

	return err;
}

