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
	$Id: MdatDataHandler.c,v 1.1.1.1 2002/09/20 08:53:34 julien Exp $
*/
#include "MP4Movies.h"
#include "MP4Impl.h"
#include "MP4DataHandler.h"
#include "MdatDataHandler.h"
#include <string.h>
#include <stdlib.h>

static MP4Err doOpen( struct MP4DataHandler * s,
		      struct MP4InputStreamRecord* inputStream, 
		      MP4DataEntryAtomPtr dataEntry )
{
	u64 b;
	MP4DataEntryAtomPtr d;
	
	b=inputStream->available;
	d=dataEntry;
	
	if (s==NULL) return MP4BadParamErr; else return MP4NoErr;
}

static MP4Err doClose( struct MP4DataHandler *s )
{
	free( s );
	return MP4NoErr;
}

static MP4Err copyData( struct MP4DataHandler *s, u64 offset, char *dst, u32 bytes )
{
   MP4Err err;
   MP4MdatDataHandlerPtr self = (MP4MdatDataHandlerPtr) s;
	
   err = MP4NoErr;
   if ( (self == NULL) || (self->mdat == NULL) || (self->mdat->data == NULL)
	|| (self->mdat->dataSize < (offset + bytes)) )
   {
      BAILWITHERROR( MP4BadParamErr );
   }
   else
   {
      char *src = self->mdat->data;
	  if (offset >> 32) 
		{ BAILWITHERROR( MP4NotImplementedErr ); }
      src += offset;
      memcpy( dst, src, bytes );
   }
  bail:
   TEST_RETURN( err );

   return err;
}

static MP4Err getDataSize( struct MP4DataHandler *s, u64* bytes )
{
	MP4MdatDataHandlerPtr self = (MP4MdatDataHandlerPtr) s;
	if ( (self == NULL) || (self->mdat == NULL) ) return MP4BadParamErr;
	
    *bytes = (self->mdat)->dataSize;
	return MP4NoErr;
}

MP4Err MP4CreateMdatDataHandler( MP4MediaDataAtomPtr mdat, MP4DataHandlerPtr *outDataHandler )
{
   MP4Err err;
   MP4MdatDataHandlerPtr self;
   
   err = MP4NoErr;
   self = (MP4MdatDataHandlerPtr) calloc( 1, sizeof(MP4MdatDataHandler) );
   TESTMALLOC( self );
   self->copyData  = copyData;
   self->mdat = mdat;
   self->open      = doOpen;
   self->close     = doClose;
   self->getDataSize = getDataSize;
   
   *outDataHandler = (MP4DataHandlerPtr) self;
  bail:
   TEST_RETURN( err );   
   return err;
}
