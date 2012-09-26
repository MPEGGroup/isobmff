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
	$Id: FileMappingDataHandler.h,v 1.1.1.1 2002/09/20 08:53:34 julien Exp $
*/
#ifndef INCLUDED_FILEMAPPING_DATAHANDLER_H
#define INCLUDED_FILEMAPPING_DATAHANDLER_H

#include "MP4DataHandler.h"
#include "FileMappingObject.h"

typedef struct MP4FileMappingDataHandler
{
	COMMON_DATAHANDLER_FIELDS
	struct MP4InputStreamRecord* inputStream;
	MP4DataEntryAtomPtr entryAtom;
	FileMappingObject   mappingObject;

} MP4FileMappingDataHandler, *MP4FileMappingDataHandlerPtr;

MP4Err MP4CreateFileMappingDataHandler( struct MP4InputStreamRecord* inputStream, 
										MP4DataEntryAtomPtr dataEntry, 
										MP4DataHandlerPtr *outDataHandler );

MP4Err MP4DisposeFileMappingDataHandler( MP4DataHandlerPtr dataHandler,
										MP4DataEntryAtomPtr dataEntry );

MP4Err MP4PreflightFileMappingDataHandler( struct MP4InputStreamRecord* inputStream, 
										MP4DataEntryAtomPtr dataEntry );

#endif
