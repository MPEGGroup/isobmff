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
derivative works. Copyright (c) 1999, 2000.
*/
/*
	$Id: MP4Movies.c,v 1.1.1.1 2002/09/20 08:53:35 julien Exp $
*/
#include "MP4Movies.h"
#include "MP4Atoms.h"
#include "FileMappingObject.h"
#include "MP4Descriptors.h"
#include "MdatDataHandler.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef LINUX
#include <strings.h>
#endif

static ISOErr newMeta( ISOMetaAtomPtr* outMeta, u32 metaType )
{
	ISOMetaAtomPtr meta;
	MP4HandlerAtomPtr hdlr;
	char name[8];

	MP4Err ISOCreateMetaAtom( ISOMetaAtomPtr *outAtom );
	MP4Err MP4CreateHandlerAtom( MP4HandlerAtomPtr *outAtom );

	MP4Err err;
	err = MP4NoErr;
	
	err = ISOCreateMetaAtom( &meta ); if (err) goto bail;
	err = MP4CreateHandlerAtom( &hdlr ); if (err) goto bail;
	
    hdlr->handlerType = metaType;
    MP4TypeToString( metaType, name );
    hdlr->setName( (MP4AtomPtr) hdlr, name, 0 );

	err = meta->addAtom( meta, (MP4AtomPtr) hdlr ); if (err) goto bail;

	*outMeta = meta;
	
bail:
	TEST_RETURN( err );
	return err;
}

ISO_EXTERN ( ISOErr ) ISONewFileMeta( ISOMovie theMovie, u32 metaType, ISOMeta* outMeta)
{
	ISOMetaAtomPtr myMeta;
	GETMOOV( theMovie );
	err = newMeta( &myMeta, metaType); if (err) goto bail;
	
	moov->meta = (MP4AtomPtr) myMeta;
	err = myMeta->setMdat( myMeta, moov->mdat ); if (err) goto bail;
	
	*outMeta = (ISOMeta) myMeta;
	
bail:
	TEST_RETURN( err );
	return err;
}

ISO_EXTERN ( ISOErr ) ISONewMovieMeta( ISOMovie theMovie, u32 metaType, ISOMeta* outMeta )
{
	ISOMetaAtomPtr myMeta;
	GETMOVIEATOM(theMovie);
	err = newMeta( &myMeta, metaType); if (err) goto bail;
	
	err = movieAtom->addAtom( movieAtom, (MP4AtomPtr) myMeta );  if (err) goto bail;
	err = myMeta->setMdat( myMeta, moov->mdat ); if (err) goto bail;
	
	*outMeta = (ISOMeta) myMeta;
	
bail:
	TEST_RETURN( err );
	return err;
}

ISO_EXTERN ( ISOErr ) ISONewTrackMeta( ISOTrack theTrack, u32 metaType, ISOMeta* outMeta )
{
	ISOMetaAtomPtr myMeta;
	MP4TrackAtomPtr trak;
	MP4Err err;

	err = newMeta( &myMeta, metaType); if (err) goto bail;
	trak = (MP4TrackAtomPtr) theTrack;

	err = trak->addAtom( trak, (MP4AtomPtr) myMeta );  if (err) goto bail;
	err = myMeta->setMdat( myMeta, trak->mdat ); if (err) goto bail;
	
	*outMeta = (ISOMeta) myMeta;
	
bail:
	TEST_RETURN( err );
	return err;
}


ISO_EXTERN ( ISOErr ) ISOAddMetaDataReference( ISOMeta meta, u16* out_ref, ISOHandle urlHandle, ISOHandle urnHandle )
{
	MP4Err MP4CreateDataInformationAtom( MP4DataInformationAtomPtr *outAtom );
	MP4Err MP4CreateDataReferenceAtom( MP4DataReferenceAtomPtr *outAtom );
	MP4Err MP4CreateDataEntryURLAtom( MP4DataEntryURLAtomPtr *outAtom );
	ISOMetaAtomPtr myMeta;
	MP4DataInformationAtomPtr  dinf;
	MP4DataReferenceAtomPtr    dref;

	MP4Err err;
	
	myMeta = (ISOMetaAtomPtr) meta;
	
	dinf = (MP4DataInformationAtomPtr) myMeta->dinf;
	if (dinf) {
	   dref = (MP4DataReferenceAtomPtr) dinf->dataReference;
	   if ( dref == NULL )
	   {
		  BAILWITHERROR( MP4InvalidMediaErr );
	   }
	}
	else
	{
		err = MP4CreateDataInformationAtom( &dinf ); if (err) goto bail;
		err = myMeta->addAtom( myMeta, (MP4AtomPtr) dinf ); if (err) goto bail;
	
		err = MP4CreateDataReferenceAtom( &dref ); if (err) goto bail;
		err = dinf->addAtom( dinf, (MP4AtomPtr) dref ); if (err) goto bail;
	}
	if ( urnHandle == NULL )
    {
      u32 sz;
      MP4DataEntryURLAtomPtr url;
      err = MP4CreateDataEntryURLAtom( &url ); if (err) goto bail;
      err = MP4GetHandleSize( urlHandle, &sz ); if (err) goto bail;
      url->locationLength = (u32) sz;
      url->location = (char*) calloc( 1, sz );
      memcpy( url->location, *urlHandle, sz );
      err = dref->addDataEntry( dref, (MP4AtomPtr) url ); if (err) goto bail;
    }
    else
    {
      u32 sz;
      MP4DataEntryURNAtomPtr urn;
      err = MP4CreateDataEntryURNAtom( &urn ); if (err) goto bail;
      err = MP4GetHandleSize( urlHandle, &sz ); if (err) goto bail;
      urn->locationLength = (u32) sz;
      urn->location = (char*) calloc( 1, sz );
      memcpy( urn->location, *urlHandle, sz );
      
      err = MP4GetHandleSize( urnHandle, &sz ); if (err) goto bail;
      urn->nameLength = (u32) sz;
      urn->name = (char*) calloc( 1, sz );
      memcpy( urn->name, *urnHandle, sz );
      err = dref->addDataEntry( dref, (MP4AtomPtr) urn ); if (err) goto bail;
    }
    *out_ref = dref->getEntryCount( dref );

bail:
	TEST_RETURN( err );
	return err;
}

static MP4Err requestItemID( ISOMetaAtomPtr myMeta, u16 itemID )
{
	MP4Err err;
	
	err = MP4NoErr;
	if (itemID == 0)
		BAILWITHERROR( MP4BadParamErr );

	if ( myMeta == NULL )
		BAILWITHERROR( MP4InvalidMediaErr )
	if ( itemID >= myMeta->next_item_ID )
	{
		myMeta->next_item_ID = itemID + 1;
	}
	else
	{
		u32 i;
		u32 itemCount = 0;
		ISOItemLocationAtomPtr iloc;
		iloc = (ISOItemLocationAtomPtr) myMeta->iloc;
		
		err = MP4BadParamErr;
		
		if ((iloc == NULL) || (!( iloc->itemList )))
			BAILWITHERROR( MP4NotFoundErr )

		err = MP4GetListEntryCount( iloc->itemList, &itemCount ); if (err) goto bail;
		for ( i = 0; i < itemCount; i++ )
		{
			MetaItemLocationPtr a;
			err = MP4GetListEntry( iloc->itemList, i, (char **) &a ); if (err) goto bail;
			if ( a == NULL )
				BAILWITHERROR( MP4InvalidMediaErr );
			if  (a->item_ID == itemID)
				goto bail;
		}
		err = MP4NoErr;
	}
bail:
	return err;
}

ISO_EXTERN ( ISOErr ) ISOAddMetaItem( ISOMeta meta, ISOMetaItem* outItem, u64 base_offset, u16 data_ref_index )
{
	ISOMetaAtomPtr myMeta;
	
	myMeta = (ISOMetaAtomPtr) meta;
	return ISOAddMetaItemWithID( meta, outItem, base_offset, data_ref_index, myMeta->next_item_ID );
}

ISO_EXTERN ( ISOErr ) ISOAddMetaItemWithID( ISOMeta meta, ISOMetaItem* outItem, u64 base_offset, u16 data_ref_index, u16 item_ID )
{
	MP4Err ISOCreateItemLocationAtom( ISOItemLocationAtomPtr *outAtom );
	ISOMetaAtomPtr myMeta;
	MetaItemLocationPtr item;
	ISOItemLocationAtomPtr iloc;

	MP4Err err;
	
	myMeta = (ISOMetaAtomPtr) meta;

	err = requestItemID( myMeta, item_ID ); if (err) goto bail;
	
	item = calloc( 1, sizeof(MetaItemLocation) ); 
	TESTMALLOC( item );
	
	item->item_ID     = item_ID;	
	item->base_offset = base_offset;
	item->dref_index  = data_ref_index;
	item->meta        = (MP4AtomPtr) myMeta;
	
	err = MP4MakeLinkedList( &(item->extentList) ); if (err) goto bail;
	
	iloc = (ISOItemLocationAtomPtr) myMeta->iloc;
	if (!iloc) {
		err = ISOCreateItemLocationAtom( &iloc ); if (err) goto bail;
		err = myMeta->addAtom( myMeta, (MP4AtomPtr) iloc );
	}
	err = MP4AddListEntry( (void*) item, iloc->itemList );
	
	*outItem = (ISOMetaItem) item;
	
bail:
	TEST_RETURN( err );
	return err;
}

ISO_EXTERN ( ISOErr ) ISOAddItemExtent( ISOMetaItem item, MP4Handle data )
{
	MetaExtentLocationPtr extent;
	ISOMetaAtomPtr myMeta;
	MP4DataInformationAtomPtr dinf;
	MetaItemLocationPtr myItem;
	u32 size, extents;
	MP4Err err;
	
	extent = calloc( 1, sizeof(MetaExtentLocation) ); 
	TESTMALLOC( extent );
	
	myItem = (MetaItemLocationPtr) item;
	myMeta = (ISOMetaAtomPtr) myItem->meta;
	
	err = MP4GetHandleSize( data, &size ); if (err) goto bail;
	extent->extent_length = size;

	if (myItem->dref_index) {
		dinf = (MP4DataInformationAtomPtr) myMeta->dinf;
		if (!dinf) { BAILWITHERROR( ISOBadParamErr ); }
		
		err = dinf->getOffset( dinf, myItem->dref_index, &(extent->extent_offset) ); if (err) goto bail;
		err = dinf->addSamples( dinf, 1, myItem->dref_index, data ); if (err) goto bail;
	}
	else
	{
		MP4MediaDataAtomPtr mdat;
		mdat = (MP4MediaDataAtomPtr) myMeta->mdat;
		extent->extent_offset = mdat->dataSize;
		err = mdat->addData( mdat, data ); if (err) goto bail;
	}
	err = MP4GetListEntryCount( myItem->extentList, &extents ); if (err) goto bail;
	if (extents==0) 
		myItem->base_offset += extent->extent_offset;

	err = MP4AddListEntry( (void*) extent, myItem->extentList );
	
bail:
	TEST_RETURN( err );
	return err;
}

ISO_EXTERN ( ISOErr ) ISOAddItemExtentReference( ISOMetaItem item, u64 offset, u64 length )
{
	MetaExtentLocationPtr extent;
	ISOMetaAtomPtr myMeta;
	MP4DataInformationAtomPtr dinf;
	MetaItemLocationPtr myItem;
	MP4Err err;
	u32 extents;
	
	extent = calloc( 1, sizeof(MetaExtentLocation) ); 
	TESTMALLOC( extent );
	
	myItem = (MetaItemLocationPtr) item;
	myMeta = (ISOMetaAtomPtr) myItem->meta;
	
	extent->extent_length = length;
	extent->extent_offset = offset;
	
	if (myItem->dref_index) {
		dinf = (MP4DataInformationAtomPtr) myMeta->dinf;
		if (!dinf) { BAILWITHERROR( ISOBadParamErr ); }
	}
	else { BAILWITHERROR( ISOBadParamErr ); }

	err = MP4GetListEntryCount( myItem->extentList, &extents ); if (err) goto bail;
	if (extents==0) 
		myItem->base_offset += extent->extent_offset;

	err = MP4AddListEntry( (void*) extent, myItem->extentList );
	
bail:
	TEST_RETURN( err );
	return err;
}

ISO_EXTERN ( ISOErr ) ISOAddPrimaryData( ISOMeta meta, u32 box_type, MP4Handle data, u8 is_full_atom )
{
	ISOMetaAtomPtr myMeta;
	MP4UnknownAtomPtr myAtom;
	char * dataPtr;
	u32 len;
	
	MP4Err err;
	err = MP4NoErr;
	
	myMeta = (ISOMetaAtomPtr) meta;
	
	err = MP4CreateUnknownAtom( &myAtom ); if (err) goto bail;
	
	err = MP4GetHandleSize( data, &len ); if (err) goto bail;
	if (is_full_atom) len += 4;
	
	dataPtr = calloc( 1, len ); TESTMALLOC( dataPtr );
	myAtom->data     = dataPtr;
	myAtom->dataSize = len;
	myAtom->type     = box_type;
	
	if (is_full_atom) { dataPtr[0] = dataPtr[1] = dataPtr[2] = dataPtr[3] = 0; dataPtr += 4; len -= 4; }
	memcpy( dataPtr, *data, len );
	
	err = myMeta->addAtom( myMeta, (MP4AtomPtr) myAtom ); if (err) goto bail;
	
bail:
	TEST_RETURN( err );
	return err;
}

ISO_EXTERN ( ISOErr ) ISOGetPrimaryData( ISOMeta meta, u32 box_type, MP4Handle data, u8 is_full_atom )
{
	ISOMetaAtomPtr myMeta;
	
	MP4Err err;
	err = MP4NoErr;
	
	myMeta = (ISOMetaAtomPtr) meta;
	
	err = myMeta->getData( myMeta, box_type, data, is_full_atom ); if (err) goto bail;
	
bail:
	TEST_RETURN( err );
	return err;
}

ISO_EXTERN ( ISOErr ) ISOGetPrimaryItemData( ISOMeta meta, u32 box_type, MP4Handle data, u8 is_full_atom )
{
	MP4Err err;
	
	err = ISOGetPrimaryData( meta, box_type, data, is_full_atom );
	if (err == MP4NotFoundErr)
	{
		u64 boff;
		u16 ID;
		ISOMetaItem item;
		err = ISOGetPrimaryItemID( meta, &ID ); if (err) goto bail;
		err = ISOFindItemByID( meta, &item, ID ); if (err) goto bail;
		err = ISOGetItemData( item, data, &boff ); if (err) goto bail;
	}
	
bail:
	TEST_RETURN( err );
	return err;
}

ISO_EXTERN ( ISOErr ) ISOSetPrimaryItem( ISOMeta meta, ISOMetaItem item )
{
	MP4Err err;
	ISOMetaAtomPtr myMeta;
	ISOPrimaryItemAtomPtr pitm;
	MetaItemLocationPtr myItem;
	
	err = MP4NoErr;
	
	myMeta = (ISOMetaAtomPtr) meta;
	myItem = (MetaItemLocationPtr) item;
	
	err = ISOCreatePrimaryItemAtom( &pitm ); if (err) goto bail;
	pitm->item_ID = myItem->item_ID;

	err = myMeta->addAtom( myMeta, (MP4AtomPtr) pitm ); if (err) goto bail;
	
bail:
	TEST_RETURN( err );
	return err;
}

ISO_EXTERN ( ISOErr ) ISOGetPrimaryItemID( ISOMeta meta, u16 *ID )
{
	MP4Err err;
	ISOMetaAtomPtr myMeta;
	ISOPrimaryItemAtomPtr pitm;
	
	err = MP4NoErr;
	
	myMeta = (ISOMetaAtomPtr) meta;
	pitm = (ISOPrimaryItemAtomPtr) myMeta->pitm;
	if (pitm) {
		*ID = pitm->item_ID;
	} else BAILWITHERROR( MP4NotFoundErr );
	
bail:
	TEST_RETURN( err );
	return err;
}

ISO_EXTERN ( ISOErr ) ISOGetItemID( ISOMetaItem item, u16* ID )
{
	MP4Err err;
	MetaItemLocationPtr myItem;
	
	err = MP4NoErr;
	
	myItem = (MetaItemLocationPtr) item;
	if (item) *ID = myItem->item_ID;
	else { BAILWITHERROR(MP4BadParamErr); }
	
bail:
	TEST_RETURN( err );
	return err;
}


ISO_EXTERN ( ISOErr ) ISOSetItemInfo( ISOMetaItem item, u16 protection_index, char* name, char* content_type, char* content_encoding )
{
	MP4Err ISOCreateItemInfoAtom( ISOItemInfoAtomPtr *outAtom );
	MP4Err ISOCreateItemInfoEntryAtom( ISOItemInfoEntryAtomPtr *outAtom );
	ISOMetaAtomPtr myMeta;
	MetaItemLocationPtr myItem;
	ISOItemInfoAtomPtr iinf;
	ISOItemInfoEntryAtomPtr infe;
	u32 sz;
	
	MP4Err err;
	err = MP4NoErr;
	
	myItem = (MetaItemLocationPtr) item;
	myMeta = (ISOMetaAtomPtr) myItem->meta;
	
	iinf = (ISOItemInfoAtomPtr) myMeta->iinf;
	if (!iinf) {
		err = ISOCreateItemInfoAtom( &iinf ); if (err) goto bail;
		err = myMeta->addAtom( myMeta, (MP4AtomPtr) iinf); if (err) goto bail;
	}
	err = ISOCreateItemInfoEntryAtom( &infe ); if (err) goto bail;
	err = iinf->addAtom( iinf, (MP4AtomPtr) infe); if (err) goto bail;
	
	if (name) {
		sz = strlen( name );
		infe->item_name = (char*) calloc( 1, sz );
		memcpy( infe->item_name, name, sz );
	} else {
		infe->item_name = (char*) calloc( 1, 1 );
		(infe->item_name)[0] = '\0';
	}

	if (content_encoding) {
		sz = strlen( content_encoding );
		infe->content_encoding = (char*) calloc( 1, sz );
		memcpy( infe->content_encoding, content_encoding, sz );
	} else {
		infe->content_encoding = (char*) calloc( 1, 1 );
		(infe->content_encoding)[0] = '\0';
	}
	
	if (content_type) {
		sz = strlen( content_type );
		infe->content_type = (char*) calloc( 1, sz );
		memcpy( infe->content_type, content_type, sz );
	} else {
		infe->content_type = (char*) calloc( 1, 1 );
		(infe->content_type)[0] = '\0';
	}
	
	infe->item_ID = myItem->item_ID;
	infe->protection_index = protection_index;
	
bail:
	TEST_RETURN( err );
	return err;
}

ISO_EXTERN ( ISOErr ) ISOGetFileMeta( ISOMovie theMovie,  ISOMeta* meta, u32 inMetaType, u32* outMetaType)
{
	ISOMetaAtomPtr myMeta;
	MP4HandlerAtomPtr hdlr;
	
	GETMOOV( theMovie );
	myMeta = (ISOMetaAtomPtr) moov->meta;
	
	if (!myMeta) { BAILWITHERROR( MP4BadParamErr ); }
	hdlr = (MP4HandlerAtomPtr) myMeta->hdlr;
	if (!hdlr) { BAILWITHERROR( MP4BadDataErr ); }
	
	if ((inMetaType != 0) && (hdlr->handlerType != inMetaType)) { BAILWITHERROR( MP4BadParamErr ); }
	if (outMetaType) *outMetaType = hdlr->handlerType;
	if (meta)        *meta = (ISOMeta) myMeta;
	
bail:
	TEST_RETURN( err );
	return err;
}

ISO_EXTERN ( ISOErr ) ISOGetMovieMeta( ISOMovie theMovie, ISOMeta* meta, u32 inMetaType, u32* outMetaType )
{
	ISOMetaAtomPtr myMeta;
	MP4HandlerAtomPtr hdlr;
	
	GETMOVIEATOM(theMovie);
	myMeta = (ISOMetaAtomPtr) movieAtom->meta;
	
	if (!myMeta) { BAILWITHERROR( MP4BadParamErr ); }
	hdlr = (MP4HandlerAtomPtr) myMeta->hdlr;
	if (!hdlr) { BAILWITHERROR( MP4BadDataErr ); }
	
	if ((inMetaType != 0) && (hdlr->handlerType != inMetaType)) { BAILWITHERROR( MP4BadParamErr ); }
	if (outMetaType) *outMetaType = hdlr->handlerType;
	if (meta)        *meta = (ISOMeta) myMeta;
	
bail:
	TEST_RETURN( err );
	return err;
}

ISO_EXTERN ( ISOErr ) ISOGetTrackMeta( ISOTrack theTrack, ISOMeta* meta, u32 inMetaType, u32* outMetaType )
{
	MP4Err err;
	ISOMetaAtomPtr myMeta;
	MP4HandlerAtomPtr hdlr;
	MP4TrackAtomPtr trak;
	
	err = MP4NoErr;
	
	trak = (MP4TrackAtomPtr) theTrack;
	myMeta = (ISOMetaAtomPtr) trak->meta;
	
	if (!myMeta) { BAILWITHERROR( MP4BadParamErr ); }
	hdlr = (MP4HandlerAtomPtr) myMeta->hdlr;
	if (!hdlr) { BAILWITHERROR( MP4BadDataErr ); }
	
	if ((inMetaType != 0) && (hdlr->handlerType != inMetaType)) { BAILWITHERROR( MP4BadParamErr ); }
	if (outMetaType) *outMetaType = hdlr->handlerType;
	if (meta)        *meta = (ISOMeta) myMeta;
	
bail:
	TEST_RETURN( err );
	return err;
}

ISO_EXTERN ( ISOErr ) ISOFindItemByID( ISOMeta meta, ISOMetaItem* item, u16 ID )
{
	MP4Err err;
	ISOMetaAtomPtr myMeta;
	ISOItemLocationAtomPtr iloc;
	u32 i, item_total;
	
	err = MP4NoErr;
	myMeta = (ISOMetaAtomPtr) meta;

	if (myMeta == NULL)
		BAILWITHERROR( MP4BadParamErr )
	iloc = (ISOItemLocationAtomPtr) myMeta->iloc;
	*item = NULL;
	
	if (iloc == NULL)
		BAILWITHERROR( MP4NotFoundErr )
	
	*item = NULL;
	
	if ( iloc->itemList )
	{
		err = MP4GetListEntryCount( iloc->itemList, &item_total ); if (err) goto bail;
		
		for ( i = 0; i < item_total; i++ )
		{
			MetaItemLocationPtr a;
			err = MP4GetListEntry( iloc->itemList, i, (char **) &a ); if (err) goto bail;
			
			if (a->item_ID == ID) {
				a->meta = (MP4AtomPtr) myMeta;
				*item = (ISOMetaItem) a;
				err = MP4NoErr;
				goto bail;
			}
		}
	} else BAILWITHERROR( MP4BadDataErr );
	err = MP4NotFoundErr;
	
bail:
	TEST_RETURN( err );

	return err;
}

ISO_EXTERN ( ISOErr ) ISOFindItemByName( ISOMeta meta, ISOMetaItem* item, char* name, u8 exact_case )
{
	MP4Err err;
	ISOMetaAtomPtr myMeta;
	ISOItemInfoAtomPtr iinf;
	u32 i, item_total;
	
	err = MP4NoErr;
	myMeta = (ISOMetaAtomPtr) meta;

	if (myMeta == NULL)
		BAILWITHERROR( MP4BadParamErr )
	iinf = (ISOItemInfoAtomPtr) myMeta->iinf;
	*item = NULL;
	
	if (iinf == NULL) 
		BAILWITHERROR( MP4NotFoundErr )
	
	if ( iinf->atomList )
	{
		err = MP4GetListEntryCount( iinf->atomList, &item_total ); if (err) goto bail;
		
		for ( i = 0; i < item_total; i++ )
		{
			ISOItemInfoEntryAtomPtr infe;
			err = MP4GetListEntry( iinf->atomList, i, (char **) &infe ); if (err) goto bail;
			
			if (( exact_case && (strcmp(name,infe->item_name)==0)) ||
			    (!exact_case && (strcasecmp(name,infe->item_name)==0)) )
				return ISOFindItemByID(meta, item, (u16) infe->item_ID);
		}
	} else BAILWITHERROR( MP4BadDataErr )
	err = MP4NotFoundErr;

bail:
	TEST_RETURN( err );

	return err;
}

ISO_EXTERN ( ISOErr ) ISOGetItemData( ISOMetaItem item, MP4Handle data, u64* base_offset )
{
	MP4Err err;
	ISOMetaAtomPtr myMeta;
	MetaItemLocationPtr myItem;
	MP4DataHandlerPtr dhlr;

	err = MP4NoErr;
	myItem = (MetaItemLocationPtr) item;
	if (myItem == NULL)
		BAILWITHERROR( MP4BadParamErr )
	
	myMeta = (ISOMetaAtomPtr) myItem->meta;
	if (myMeta == NULL)
		BAILWITHERROR( MP4BadDataErr )

	if (myMeta->dataEntryIndex != myItem->dref_index)
	{
		if ( myMeta->dataHandler )
		{
			err = myMeta->closeDataHandler( myMeta ); if (err) goto bail;
		}
		
		err = myMeta->openDataHandler( myMeta, myItem->dref_index ); if (err) goto bail;
	}
		
	dhlr = (MP4DataHandlerPtr) myMeta->dataHandler;
	if ( dhlr == NULL )
	{
	  BAILWITHERROR( MP4InvalidMediaErr );
	}

	err = MP4SetHandleSize( data, 0 ); if (err) goto bail;

	*base_offset = myItem->base_offset;
	
	if ( myItem->extentList )
	{
		u32 list2Size;
		u32 datasize, j;
		
		err = MP4GetListEntryCount( myItem->extentList, &list2Size ); if (err) goto bail;
		
		datasize = 0;
		
		for ( j = 0; j < list2Size; j++ )
		{
			MetaExtentLocationPtr b;
			u32 length;
			u64 fileSize;
			
			err = MP4GetListEntry( myItem->extentList, j, (char **) &b ); if (err) goto bail;
			
			if (b->extent_length==0) {
				err = dhlr->getDataSize( dhlr, &fileSize ); if (err) goto bail;
				if (b->extent_offset) BAILWITHERROR( MP4BadDataErr );
				/* the docs say that length==0 means the whole external file; it should probably say
				   from the extent_offset to end of file */
				if (fileSize >> 32) BAILWITHERROR( MP4NoLargeAtomSupportErr );
				length = (u32) fileSize;
			}
			else {
				if (b->extent_length >> 32) BAILWITHERROR( MP4NoLargeAtomSupportErr );
				length = (u32) b->extent_length;
			}
			
			err = MP4SetHandleSize( data, datasize+length ); if (err) goto bail;
			err = dhlr->copyData( dhlr, b->extent_offset, (*data)+datasize, length ); if (err) goto bail;
			if (j==0) *base_offset -= b->extent_offset;
			/* it used to be at extent_offset in the source, but as is it the first thing in the output handle, it's now at 0 */
			datasize += length;
		}
	}
	
bail:
	TEST_RETURN( err );

	return err;
}

ISO_EXTERN ( ISOErr ) ISOGetItemInfo( ISOMetaItem item, u16* protection_index, char* name, char* content_type, char* content_encoding )
{
	MP4Err err;
	ISOMetaAtomPtr myMeta;
	ISOItemInfoAtomPtr iinf;
	u32 i, item_total;
	MetaItemLocationPtr myItem;
	
	err = MP4NoErr;
	myItem = (MetaItemLocationPtr) item;
	if (myItem == NULL)
		BAILWITHERROR( MP4BadParamErr )
	
	myMeta = (ISOMetaAtomPtr) myItem->meta;
	if (myMeta == NULL)
		BAILWITHERROR( MP4BadDataErr )

	iinf = (ISOItemInfoAtomPtr) myMeta->iinf;
	
	if (iinf == NULL)
		BAILWITHERROR( MP4NotFoundErr )
	
	if ( iinf->atomList )
	{
		err = MP4GetListEntryCount( iinf->atomList, &item_total ); if (err) goto bail;
		
		for ( i = 0; i < item_total; i++ )
		{
			ISOItemInfoEntryAtomPtr infe;
			err = MP4GetListEntry( iinf->atomList, i, (char **) &infe ); if (err) goto bail;
			if (infe->item_ID == myItem->item_ID) {
				u32 sz;
				if (name && infe->item_name) {
					sz = strlen( infe->item_name );
					memcpy( name, infe->item_name, sz+1 );
				} else if (name) name[0] = 0;

				if (content_encoding && infe->content_encoding) {
					sz = strlen( infe->content_encoding );
					memcpy( content_encoding, infe->content_encoding, sz+1 );
				} else if (content_encoding) content_encoding[0] = 0;

				if (content_type && infe->content_type) {
					sz = strlen( infe->content_type );
					memcpy( content_type, infe->content_type, sz+1 );
				} else if (content_type) name[0] = 0;
				
				if (protection_index) *protection_index = infe->protection_index;
				err = MP4NoErr;
				goto bail;
			}
		}
	} else BAILWITHERROR( MP4BadDataErr )
	err = MP4NotFoundErr;
	
bail:
	TEST_RETURN( err );

	return err;
}

ISO_EXTERN ( MP4Err ) 
ISOCheckMetaDataReferences( ISOMeta meta )
{
	MP4Err err;
	ISOMetaAtomPtr myMeta;
	MP4DataInformationAtomPtr  dinf;
	MP4DataReferenceAtomPtr    dref;
	MP4DataEntryAtomPtr		   dataEntryAtom;
	u32 count, dataEntryIndex;
	
	err = MP4NoErr;
   	myMeta = (ISOMetaAtomPtr) meta;

	dinf = (MP4DataInformationAtomPtr) myMeta->dinf;
	if (!dinf) goto bail;
	
	dref = (MP4DataReferenceAtomPtr) dinf->dataReference;
	if ( dref == NULL )
	{
	  BAILWITHERROR( MP4InvalidMediaErr );
	}

	count = dref->getEntryCount( dref );
	for ( dataEntryIndex = 1; dataEntryIndex <= count; dataEntryIndex++ )
	{
		err = dref->getEntry( dref, dataEntryIndex, &dataEntryAtom ); if (err) goto bail;
		if ( dataEntryAtom == NULL )
			BAILWITHERROR( MP4InvalidMediaErr )	
		err = MP4PreflightDataHandler( myMeta->inputStream, dataEntryAtom ); if (err) goto bail;
	}
	
bail:
   TEST_RETURN( err );

   return err;
}

ISO_EXTERN ( MP4Err )
ISONewMetaProtection( ISOMeta meta, u32 sch_type, u32 sch_version, char* sch_url, u16* protection_index )
{
	MP4Err err;
	
	ISOMetaAtomPtr myMeta;
	MP4SecurityInfoAtomPtr sinf;
	MP4SecuritySchemeAtomPtr schm;
	MP4SchemeInfoAtomPtr schi;
	ISOItemProtectionAtomPtr ipro;
	u32 j;
	
	char* sch_url_copy = NULL;
	
	err = MP4NoErr;
   	myMeta = (ISOMetaAtomPtr) meta;

	err = MP4CreateSchemeInfoAtom( &schi ); if (err) goto bail;
	
	err = MP4CreateSecuritySchemeAtom( &schm ); if (err) goto bail;
	schm->scheme_type    = sch_type;
	schm->scheme_version = sch_version;
	
	if (sch_url) {
		sch_url_copy = (char*) calloc( 1, strlen(sch_url)+1 );
	    TESTMALLOC( sch_url_copy );
	    memcpy( sch_url_copy, sch_url, strlen(sch_url)+1 );
		schm->scheme_url = sch_url_copy; sch_url_copy = NULL;
	}
	else schm->scheme_url = NULL;

	err = MP4CreateSecurityInfoAtom( &sinf ); if (err) goto bail;
	sinf->MP4SecurityScheme = (MP4AtomPtr) schm; schm = NULL;
	sinf->MP4SchemeInfo     = (MP4AtomPtr) schi; schi = NULL;
	
	ipro = (ISOItemProtectionAtomPtr) myMeta->ipro;
	if (!ipro) {
		err = ISOCreateItemProtectionAtom( &ipro ); if (err) goto bail;
		err = myMeta->addAtom( myMeta, (MP4AtomPtr) ipro ); if (err) goto bail;
	}
	err = MP4GetListEntryCount( ipro->atomList, &j ); if (err) goto bail;
	*protection_index = j+1;
	
	err = ipro->addAtom( ipro, (MP4AtomPtr) sinf ); if (err) goto bail;	
			
bail:
	if (schm) schm->destroy( (MP4AtomPtr) schm );
	if (schi) schi->destroy( (MP4AtomPtr) schi );
	if (sch_url_copy) free( sch_url_copy );
	
	TEST_RETURN( err );

	return err;
}

ISO_EXTERN ( MP4Err )
ISOAddMetaProtectionInfo( ISOMeta meta, u16 protection_index, MP4GenericAtom schi_atom  )
{
	MP4Err err;
	
	ISOMetaAtomPtr myMeta;
	MP4SecurityInfoAtomPtr sinf;
	MP4SchemeInfoAtomPtr schi;
	ISOItemProtectionAtomPtr ipro;

	err = MP4NoErr;
   	myMeta = (ISOMetaAtomPtr) meta;
	ipro = (ISOItemProtectionAtomPtr) myMeta->ipro;
	if (!ipro) BAILWITHERROR( MP4BadParamErr );
	
	err = MP4GetListEntry( ipro->atomList, protection_index-1, (char **) &sinf ); if (err) goto bail;
	schi = (MP4SchemeInfoAtomPtr) sinf->MP4SchemeInfo;
	if (!schi) BAILWITHERROR( MP4BadParamErr );
	err = schi->addAtom( schi, (MP4AtomPtr) schi_atom ); if (err) goto bail;
	
bail:
	TEST_RETURN( err );
	
	return err;
}

ISO_EXTERN ( MP4Err )
ISOGetMetaProtection( ISOMeta meta, u16 protection_index, u32* sch_type, u32* sch_version, char* sch_url )
{
	MP4Err err;
	
	ISOMetaAtomPtr myMeta;
	MP4SecurityInfoAtomPtr sinf;
	MP4SecuritySchemeAtomPtr schm;
	ISOItemProtectionAtomPtr ipro;
	
	err = MP4NoErr;
   	myMeta = (ISOMetaAtomPtr) meta;

	ipro = (ISOItemProtectionAtomPtr) myMeta->ipro;
	if (!ipro) BAILWITHERROR( MP4BadParamErr);
	err  = MP4GetListEntry( ipro->atomList, protection_index-1, (char **) &sinf ); if (err) goto bail;
	
	schm = (MP4SecuritySchemeAtomPtr) sinf->MP4SecurityScheme;

	*sch_type    = schm->scheme_type;
	*sch_version = schm->scheme_version;

	if (sch_url && schm->scheme_url) {
	    memcpy( sch_url, schm->scheme_url, strlen(schm->scheme_url)+1 );
	}
	
bail:	
	TEST_RETURN( err );

	return err;
}

ISO_EXTERN ( MP4Err )
ISOGetMetaProtectionInfo( ISOMeta meta, u16 protection_index, u32 atom_type, MP4GenericAtom* schi_atom  )
{
	MP4Err err;
	
	ISOMetaAtomPtr myMeta;
	MP4SecurityInfoAtomPtr sinf;
	MP4SchemeInfoAtomPtr schi;
	ISOItemProtectionAtomPtr ipro;
	u32 item_total, i;
	
	err = MP4NoErr;
	*schi_atom = NULL;
   	myMeta = (ISOMetaAtomPtr) meta;
	ipro = (ISOItemProtectionAtomPtr) myMeta->ipro;
	if (!ipro) BAILWITHERROR( MP4BadParamErr );
	
	err = MP4GetListEntry( ipro->atomList, protection_index-1, (char **) &sinf ); if (err) goto bail;
	schi = (MP4SchemeInfoAtomPtr) sinf->MP4SchemeInfo;
	if (!schi) BAILWITHERROR( MP4BadParamErr );
	
	if ( schi->atomList )
	{
		err = MP4GetListEntryCount( schi->atomList, &item_total ); if (err) goto bail;
		
		for ( i = 0; i < item_total; i++ )
		{
			MP4AtomPtr a;
			err = MP4GetListEntry( schi->atomList, i, (char **) &a ); if (err) goto bail;
			
			if (a->type == atom_type) {
				*schi_atom = (MP4GenericAtom) a;
				err = MP4NoErr;
				goto bail;
			}
		}
	} else BAILWITHERROR( MP4BadDataErr )
	err = MP4NotFoundErr;
	
bail:
	TEST_RETURN( err );
	
	return err;
}

