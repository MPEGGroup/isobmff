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
#include "PCMAtoms.h"
#include "Logger.h"

MP4Err MP4CreatePCMAtoms ( u32 atomType, MP4AtomPtr *outAtom )
{
    MP4Err      err;
	MP4AtomPtr  newAtom;
    char        typeString[8];
	
	err = MP4NoErr;
	
    MP4TypeToString( atomType, typeString );
    
    logMsg(LOGLEVEL_TRACE, "MP4CreatePCMAtoms called with type: \"%s\"", typeString);
    
	switch ( atomType )
	{
            
        case AudioIntegerPCMSampleEntryType:
        case AudioFloatPCMSampleEntryType:
            err = MP4CreateAudioSampleEntryAtom ( (MP4AudioSampleEntryAtomPtr*) &newAtom );
            break;
            
		case MP4PCMConfigAtomType:
			err = MP4CreatePCMConfigAtom ( (MP4PCMConfigAtomPtr*) &newAtom );
			break;
			
		default:
			err = MP4CreateAtom( atomType, &newAtom );
			break;
	}
	if ( err == MP4NoErr )
    {
		*outAtom = newAtom;
        logMsg(LOGLEVEL_TRACE, "Created atom with name: \"%s\"", newAtom->name);
    }
	return err;
}

MP4Err MP4ParsePCMAtomUsingProtoList ( MP4InputStreamPtr inputStream, u32* protoList, u32 defaultAtom, MP4AtomPtr *outAtom )
{
    MP4AtomPtr      atomProto;
	MP4Err          err;
	long            bytesParsed;
	MP4Atom	        protoAtom;
	MP4AtomPtr      newAtom;
	char            typeString[ 8 ];
	char            msgString[ 80 ];
	u64             beginAvail;
	u64             consumedBytes;
	u32             useDefaultAtom;

	atomProto = &protoAtom;
	err = MP4NoErr;
	bytesParsed = 0L;
	
	if ((inputStream == NULL) || (outAtom == NULL) )
		BAILWITHERROR( MP4BadParamErr )
        *outAtom = NULL;
	beginAvail = inputStream->available;
	useDefaultAtom = 0;
	inputStream->msg( inputStream, "{" );
	inputStream->indent++;
	err = MP4CreateBaseAtom( atomProto ); if ( err ) goto bail;
	
	atomProto->streamOffset = inputStream->getStreamOffset( inputStream );
    
	/* atom size */
	err = inputStream->read32( inputStream, &atomProto->size, NULL ); if ( err ) goto bail;
 	if ( atomProto->size == 0 ) {
		/* BAILWITHERROR( MP4NoQTAtomErr )  */
		u64 the_size;
		the_size = inputStream->available + 4;
		if (the_size >> 32) {
			atomProto->size = 1;
			atomProto->size64  = the_size + 8;
		}
		else atomProto->size = (u32) the_size;
	}
	if ((atomProto->size != 1) && ((atomProto->size - 4) > inputStream->available))
		BAILWITHERROR( MP4BadDataErr )
    bytesParsed += 4L;
	
	sprintf( msgString, "atom size is %d", atomProto->size );
	inputStream->msg( inputStream, msgString );
    
	/* atom type */
	err = inputStream->read32( inputStream, &atomProto->type, NULL ); if ( err ) goto bail;
	bytesParsed += 4L;
	MP4TypeToString( atomProto->type, typeString );
	sprintf( msgString, "atom type is '%s'", typeString );
	inputStream->msg( inputStream, msgString );
	if ( atomProto->type == MP4ExtendedAtomType )
	{
		err = inputStream->readData( inputStream, 16, (char*) atomProto->uuid, NULL );	if ( err ) goto bail;
		bytesParsed += 16L;
	}
	
	/* large atom */
	if ( atomProto->size == 1 )
	{
		u32 size;
		err = inputStream->read32( inputStream, &size, NULL ); if ( err ) goto bail;
		/* if ( size )
         BAILWITHERROR( MP4NoLargeAtomSupportErr ) */
		atomProto->size64 = size;
        atomProto->size64 <<= 32;
		err = inputStream->read32( inputStream, &size, NULL ); if ( err ) goto bail;
		atomProto->size64 |= size;
		atomProto->size = 1;
		bytesParsed += 8L;
	}
    
	atomProto->bytesRead = (u32) bytesParsed;
	if ((atomProto->size != 1) && ( ((long) atomProto->size) < bytesParsed ))
		BAILWITHERROR( MP4BadDataErr )
        if ( protoList )
        {
            while ( *protoList  )
            {
                if ( *protoList == atomProto->type )
                    break;
                protoList++;
            }
            if ( *protoList == 0 )
            {
                useDefaultAtom = 1;
            }
        }

	err = MP4CreatePCMAtoms( useDefaultAtom ? defaultAtom : atomProto->type, &newAtom ); if ( err ) goto bail;
	sprintf( msgString, "atom name is '%s'", newAtom->name );
	inputStream->msg( inputStream, msgString );
    err = newAtom->createFromInputStream( newAtom, atomProto, (char*) inputStream ); if ( err ) goto bail;
	consumedBytes = beginAvail - inputStream->available;
	if ((atomProto->size != 1 ) && ( consumedBytes != atomProto->size ))
	{
		sprintf( msgString, "##### atom size is %d but parse used %lld bytes ####", atomProto->size, consumedBytes );
		inputStream->msg( inputStream, msgString );
		if (consumedBytes < atomProto->size) {
			u32 x;
			u32 i;
			for (i=0; i<(atomProto->size)-consumedBytes; i++) inputStream->read8(inputStream, &x, NULL );
		}
	}
	else if ((atomProto->size == 1 ) && ( consumedBytes != atomProto->size64 ))
	{
		sprintf( msgString, "##### atom size is %lld but parse used %lld bytes ####", atomProto->size64, consumedBytes );
		inputStream->msg( inputStream, msgString );
		if (consumedBytes < atomProto->size64) {
			u32 x;
			u64 i;
			for (i=0; i<(atomProto->size64)-consumedBytes; i++) inputStream->read8(inputStream, &x, NULL );
		}
	}
	*outAtom = newAtom;
	inputStream->indent--;
	inputStream->msg( inputStream, "}" );
bail:
	TEST_RETURN( err );
    
	return err;
}

MP4Err MP4ParsePCMAtom ( MP4InputStreamPtr inputStream, MP4AtomPtr *outAtom )
{
    logMsg(LOGLEVEL_TRACE, "Parsing Atom");
    return  MP4ParsePCMAtomUsingProtoList( inputStream, NULL, 0, outAtom  );
}

