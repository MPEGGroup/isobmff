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
	$Id: MP4UserData.c,v 1.1.1.1 2002/09/20 08:53:35 julien Exp $
*/
#include "MP4Movies.h"
#include "MP4Atoms.h"
#include <string.h>

MP4_EXTERN ( MP4Err )
MP4AddUserData( MP4UserData theUserData, MP4Handle dataH, u32 userDataType, u32 *outIndex )
{
	MP4Err err;
	MP4UserDataAtomPtr udta;
	err = MP4NoErr;
	if ( (theUserData == NULL) || (dataH == NULL) || (userDataType == 0) )
	{
	    BAILWITHERROR( MP4BadParamErr );
	}
	udta = (MP4UserDataAtomPtr) theUserData;
	err = udta->addUserData( udta, dataH, userDataType, outIndex ); if (err) goto bail;
bail:
	TEST_RETURN( err );
	
	return err;
}

MP4_EXTERN ( MP4Err )
MP4GetUserDataEntryCount( MP4UserData theUserData, u32 userDataType, u32 *outCount )
{
	MP4Err err;
	MP4UserDataAtomPtr udta;
	err = MP4NoErr;
	if ( (theUserData == NULL) || (outCount == NULL) || (userDataType == 0) )
	{
	    BAILWITHERROR( MP4BadParamErr );
	}
	udta = (MP4UserDataAtomPtr) theUserData;
	if ( udta->getEntryCount == 0 )
	{
		BAILWITHERROR( MP4BadParamErr );
	}
	err = udta->getEntryCount( udta, userDataType, outCount ); if (err) goto bail;
bail:
	TEST_RETURN( err );
	
	return err;
}

MP4_EXTERN ( MP4Err )
MP4GetIndUserDataType( MP4UserData theUserData, u32 typeIndex, u32 *outType )
{
	MP4Err err;
	MP4UserDataAtomPtr udta;
	err = MP4NoErr;
	if ( (theUserData == NULL) || (outType == NULL) || (typeIndex == 0) )
	{
	    BAILWITHERROR( MP4BadParamErr );
	}
	udta = (MP4UserDataAtomPtr) theUserData;
	if ( udta->getIndType == 0 )
	{
		BAILWITHERROR( MP4BadParamErr );
	}
	err = udta->getIndType( udta, typeIndex, outType ); if (err) goto bail;
bail:
	TEST_RETURN( err );
	
	return err;
}

MP4_EXTERN ( MP4Err )
MP4GetUserDataTypeCount( MP4UserData theUserData, u32 *outCount )
{
	MP4Err err;
	MP4UserDataAtomPtr udta;
	err = MP4NoErr;
	if ( (theUserData == NULL) || (outCount == NULL) )
	{
	    BAILWITHERROR( MP4BadParamErr );
	}
	udta = (MP4UserDataAtomPtr) theUserData;
	if ( udta->getTypeCount == 0 )
	{
		BAILWITHERROR( MP4BadParamErr );
	}
	err = udta->getTypeCount( udta, outCount ); if (err) goto bail;
bail:
	TEST_RETURN( err );
	
	return err;
}

MP4_EXTERN ( MP4Err )
MP4GetUserDataItem( MP4UserData theUserData, MP4Handle dataH, u32 userDataType, u32 itemIndex )
{
	MP4Err err;
	MP4UserDataAtomPtr udta;
	err = MP4NoErr;
	if ( (theUserData == NULL) || (dataH == NULL) || (userDataType == 0) || (itemIndex == 0) )
	{
	    BAILWITHERROR( MP4BadParamErr );
	}
	udta = (MP4UserDataAtomPtr) theUserData;
	if ( udta->getItem == 0 )
	{
		BAILWITHERROR( MP4BadParamErr );
	}
	err = udta->getItem( udta, dataH, userDataType, itemIndex ); if (err) goto bail;
bail:
	TEST_RETURN( err );
	
	return err;
}

MP4_EXTERN ( MP4Err )
MP4DeleteUserDataItem( MP4UserData theUserData, u32 userDataType, u32 itemIndex )
{
	MP4Err err;
	MP4UserDataAtomPtr udta;
	err = MP4NoErr;
	if ( (theUserData == NULL) || (userDataType == 0) || (itemIndex == 0) )
	{
	    BAILWITHERROR( MP4BadParamErr );
	}
	udta = (MP4UserDataAtomPtr) theUserData;
	if ( udta->deleteItem == 0 )
	{
		BAILWITHERROR( MP4BadParamErr );
	}
	err = udta->deleteItem( udta, userDataType, itemIndex ); if (err) goto bail;
bail:
	TEST_RETURN( err );
	
	return err;
}

MP4_EXTERN ( MP4Err )
MP4NewUserData( MP4UserData *outUserData )
{
	MP4Err err;
	MP4AtomPtr udta;
	err = MP4NoErr;
	err = MP4CreateAtom( MP4UserDataAtomType, &udta ); if (err) goto bail;
	*outUserData = (MP4UserData) udta;
bail:
	TEST_RETURN( err );
	
	return err;
}

MP4_EXTERN ( MP4Err )
MP4NewForeignAtom( MP4GenericAtom *outAtom, u32 atomType, MP4Handle atomPayload )
{
	MP4Err err;
	MP4AtomPtr the_atom;
	MP4UnknownAtomPtr self;
	
	err = MP4NoErr;
	err = MP4CreateUnknownAtom((MP4UnknownAtomPtr *) &the_atom ); if (err) goto bail;
	self = (MP4UnknownAtomPtr) the_atom;
	self->type = atomType;
	
	err = MP4GetHandleSize( atomPayload, &(self->dataSize) ); if (err) goto bail;
	
	self->data = (char *) malloc( self->dataSize );
	if (!(self->data)) { err = MP4NoMemoryErr; goto bail; }
	memcpy( self->data, *atomPayload, self->dataSize );
	
	*outAtom = (MP4GenericAtom) the_atom;
bail:
	TEST_RETURN( err );
	
	return err;
}

MP4_EXTERN ( MP4Err )
MP4GetForeignAtom( MP4GenericAtom atom, u32* atomType, u8 the_uuid[16], MP4Handle atomPayload )
{
	MP4Err err;
	MP4AtomPtr the_atom;
	MP4UnknownAtomPtr self;
	
	err = MP4NoErr;
	the_atom = (MP4AtomPtr) atom;
	self = (MP4UnknownAtomPtr) the_atom;
	*atomType = self->type;
	
	err = MP4SetHandleSize( atomPayload, self->dataSize ); if (err) goto bail;
	
	memcpy( *atomPayload, self->data, self->dataSize );
	if ((self->type == MP4ExtendedAtomType) && the_uuid) 
		memcpy( the_uuid, &(self->uuid), 16 );
bail:
	TEST_RETURN( err );
	
	return err;
}

MP4_EXTERN ( MP4Err )
MP4NewUUIDAtom( MP4GenericAtom *outAtom, u8 the_uuid[16], MP4Handle atomPayload )
{
	MP4Err err;
	MP4AtomPtr the_atom;
	MP4UnknownAtomPtr self;
	
	err = MP4NoErr;
	err = MP4CreateAtom( MP4ExtendedAtomType, &the_atom ); if (err) goto bail;
	self = (MP4UnknownAtomPtr) the_atom;
	self->type = MP4ExtendedAtomType;

	err = MP4GetHandleSize( atomPayload, &(self->dataSize) ); if (err) goto bail;
	
	self->data = (char*) malloc( self->dataSize );
	if (!(self->data)) { err = MP4NoMemoryErr; goto bail; }
	memcpy( self->data, *atomPayload, self->dataSize );
	
	memcpy( &(self->uuid), the_uuid, 16 );
	
	*outAtom = (MP4GenericAtom) the_atom;
bail:
	TEST_RETURN( err );
	
	return err;
}

