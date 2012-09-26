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
	$Id: MJ2ColorSpecificationAtom.c,v 1.1.1.1 2002/09/20 08:53:34 julien Exp $
*/

#include "MJ2Atoms.h"
#include <stdlib.h>
#include <string.h>

static void destroy( MP4AtomPtr s )
{
	ISOErr err = ISONoErr;
	MJ2ColorSpecificationAtomPtr self = (MJ2ColorSpecificationAtomPtr) s;
	
	if ( self == NULL )
		BAILWITHERROR( ISOBadParamErr )
		
	if ( self->profile )
	{
		free( self->profile );
		self->profile = NULL;
	}

	if ( self->super )
		self->super->destroy( s );
bail:
	TEST_RETURN( err );

	return;
}

static ISOErr serialize( struct MP4Atom* s, char* buffer )
{
	ISOErr err;
	MJ2ColorSpecificationAtomPtr self = (MJ2ColorSpecificationAtomPtr) s;
	
	err = ISONoErr;
	
	err = MP4SerializeCommonBaseAtomFields( s, buffer ); if (err) goto bail;
    buffer += self->bytesWritten; 
    
    PUT8( method );
    PUT8( precedence );
    PUT8( approx );
    
    if ( self->method == MJ2EnumeratedColorSpace )
    {
    	PUT32( enumCS );
    } 
    else if ( self->method == MJ2RestrictedICCProfile )
    {
  	 	PUTBYTES( self->profile, self->profileSize );
    }
 
	assert( self->bytesWritten == self->size );
bail:
	TEST_RETURN( err );

	return err;
}

static ISOErr calculateSize( struct MP4Atom* s )
{
	ISOErr err;
	MJ2ColorSpecificationAtomPtr self = (MJ2ColorSpecificationAtomPtr) s;
	err = ISONoErr;
	
	err = MP4CalculateBaseAtomFieldSize( s ); if (err) goto bail;
	self->size += 3 * sizeof(u8);
	
    if ( self->method == MJ2EnumeratedColorSpace )
    {
    	self->size += sizeof(u32);
    } 
    else if ( self->method == MJ2RestrictedICCProfile )
    {
    	self->size += self->profileSize;
    }

bail:
	TEST_RETURN( err );

	return err;
}

static ISOErr createFromInputStream( MP4AtomPtr s, MP4AtomPtr proto, MP4InputStreamPtr inputStream )
{
	ISOErr err;
	MJ2ColorSpecificationAtomPtr self = (MJ2ColorSpecificationAtomPtr) s;
	
	err = ISONoErr;
	if ( self == NULL )
		BAILWITHERROR( ISOBadParamErr )
	err = self->super->createFromInputStream( s, proto, (char*) inputStream ); if ( err ) goto bail;

	GET8( method );
	GET8( precedence );
	GET8( approx );
	
	if ( self->method == MJ2EnumeratedColorSpace )
    {
		GET32( enumCS );
    } 
    else if ( self->method == MJ2RestrictedICCProfile )
    {
		self->profileSize = self->size - self->bytesRead;
		self->profile = (char*) malloc( self->profileSize );
		TESTMALLOC( self->profile );
		GETBYTES( self->profileSize, profile );
    }

bail:
	TEST_RETURN( err );

	return err;
}

ISOErr MJ2CreateColorSpecificationAtom( MJ2ColorSpecificationAtomPtr *outAtom )
{
	ISOErr err;
	MJ2ColorSpecificationAtomPtr self;
	
	self = (MJ2ColorSpecificationAtomPtr) calloc( 1, sizeof( MJ2ColorSpecificationAtom ) );
	TESTMALLOC( self );

	err = MP4CreateBaseAtom( (MP4AtomPtr) self );
	if ( err ) goto bail;
	
	self->type					= MJ2ColorSpecificationAtomType;
	self->name					= "JPEG 2000 color specification atom";
	self->destroy				= destroy;
	self->createFromInputStream = (cisfunc) createFromInputStream;
	self->calculateSize			= calculateSize;
	self->serialize				= serialize;
	self->method				= MJ2EnumeratedColorSpace;
	*outAtom = self;
bail:
	TEST_RETURN( err );

	return err;
}

