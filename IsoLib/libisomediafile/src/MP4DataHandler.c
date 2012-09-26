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
	$Id: MP4DataHandler.c,v 1.1.1.1 2002/09/20 08:53:35 julien Exp $
*/
#include "MP4Movies.h"
#include "MP4Impl.h"
#include "MP4DataHandler.h"
#include "FileMappingDataHandler.h"
MP4Err MP4CreateMdatDataHandler( MP4MediaDataAtomPtr mdat, MP4DataHandlerPtr *outDataHandler );

MP4Err MP4CreateDataHandler( MP4InputStreamPtr inputStream, MP4DataEntryAtomPtr dataEntry, MP4DataHandlerPtr *outDataHandler )
{
	if ((dataEntry->flags==1) && (dataEntry->mdat != NULL)) 
		return MP4CreateMdatDataHandler( (MP4MediaDataAtomPtr) dataEntry->mdat, outDataHandler );
	else
		return MP4CreateFileMappingDataHandler( inputStream, dataEntry, outDataHandler );
}

MP4Err MP4DisposeDataHandler( MP4DataHandlerPtr dataHandler, MP4DataEntryAtomPtr dataEntry )
{
	if ((dataEntry->flags==1) && (dataEntry->mdat != NULL)) 
		return dataHandler->close( dataHandler );
	else
		return MP4DisposeFileMappingDataHandler( dataHandler, dataEntry );
}

MP4Err MP4PreflightDataHandler( MP4InputStreamPtr inputStream, MP4DataEntryAtomPtr dataEntry )
{
    if ( dataEntry->flags == 1 )
    return MP4NoErr;
    return MP4PreflightFileMappingDataHandler( inputStream, dataEntry );
}
