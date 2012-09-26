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

#include "MP4Movies.h"
#include "MP4Descriptors.h"
#include "MP4LinkedList.h"
#include "MP4Impl.h"
#include <string.h>

MP4Err MP4NewIPMPTool( MP4Handle ipmpToolH,
                       u64 ipmpToolIdLowerPart,
                       u64 ipmpToolIdUpperPart,
                       MP4Handle altGroupInfoH,
                       MP4Handle parametricInfoH )
{
    MP4Err err;
    MP4IPMPToolPtr tool;
    err = MP4NoErr;
    tool = NULL;

    if ( ipmpToolH == NULL )
        BAILWITHERROR( MP4BadParamErr );

    /* create the IPMP Tool */
    err = MP4CreateIPMPTool( MP4IPMP_ToolTag, 
                             0, 
                             0, 
                             (MP4DescriptorPtr *) &tool );
    if (err) goto bail;
    
    /* set the id */
    tool->toolId->upperPart = ipmpToolIdUpperPart;
    tool->toolId->lowerPart = ipmpToolIdLowerPart;

    /* set the alternates */
    if ( altGroupInfoH != NULL )
    {
        u32 handleSize;
        
        err = MP4GetHandleSize( altGroupInfoH,
                                &handleSize ); if (err) goto bail;
        /* check that the handler is 128 bits aligned */
        if ( ((handleSize % 16) != 0) || (handleSize == 0) )
        {
            BAILWITHERROR( MP4BadParamErr );
        }
        else
        {
            u32 i;
            u64* altIds = (u64 *)*altGroupInfoH;
            for ( i = 0; i < handleSize/16; i++ )
            {
                MP4IPMPToolIdPtr id = (MP4IPMPToolIdPtr) calloc( 1, sizeof(MP4IPMPToolId) );
                TESTMALLOC(id);
                id->lowerPart = altIds[i*2];
                id->upperPart = altIds[i*2 + 1];
                err = MP4AddListEntry( id,  
                                       tool->alternateToolIds ); if (err) goto bail;
            }
        }
    }
    /* set the parametric description */
    if ( parametricInfoH != NULL )
    {
        u32 paramInfoSize;
        MP4IPMPByteArrayPtr paramInfo = (MP4IPMPByteArrayPtr) calloc( 1, sizeof(MP4IPMPByteArray) );
        TESTMALLOC( paramInfo );
        err = MP4GetHandleSize( parametricInfoH,
                                &paramInfoSize ); if (err) goto bail;
        paramInfo->size = paramInfoSize;
        paramInfo->data = (char*) calloc( 1, paramInfoSize );
        TESTMALLOC( paramInfo->data );
        memcpy( paramInfo->data, *parametricInfoH, paramInfoSize );
        tool->toolParamDesc = paramInfo;
    }

    /* save the tool as a handle */
    err = tool->calculateSize( (MP4DescriptorPtr)tool ); if (err) goto bail;
    err = MP4SetHandleSize( ipmpToolH, tool->size ); if (err) goto bail;
    err = tool->serialize( (MP4DescriptorPtr)tool, *ipmpToolH ); if (err) goto bail;
        
bail:
    if ( tool ) tool->destroy( (MP4DescriptorPtr) tool );
    return err;
}

MP4Err MP4AddUrlToIPMPTool( MP4Handle ipmpToolH,
                            MP4Handle urlH )
{
    MP4Err MP4CreateMemoryInputStream( char* base, u32 size, MP4InputStreamPtr *outStream );
    
    MP4Err err;
    MP4IPMPToolPtr desc;
    u32 toolSize;
    MP4InputStreamPtr is;
    MP4IPMPByteArrayPtr url;
    
    err = MP4NoErr;
    desc = NULL;   
    if ( (ipmpToolH == NULL) || (urlH == NULL) )
        BAILWITHERROR( MP4BadParamErr );
    
    /* parse the descriptor */
    err = MP4GetHandleSize( ipmpToolH,
                            &toolSize ); if (err) goto bail;
    err = MP4CreateMemoryInputStream( *ipmpToolH,
                                      toolSize,
                                      &is ); if (err) goto bail;
    is->debugging = 0;
    err = MP4ParseDescriptor( is, (MP4DescriptorPtr *) &desc ); if (err) goto bail;
    is->destroy( is );

    /* build the url */
    url = (MP4IPMPByteArrayPtr) calloc( 1, sizeof(MP4IPMPByteArray) );
    TESTMALLOC( url );
    err = MP4GetHandleSize( urlH,
                            &url->size ); if (err) goto bail;
    url->data = (char *) calloc( 1, url->size );
    TESTMALLOC( url->data );

    /* add the url to the descriptor */
    err = MP4AddListEntry( url, desc->toolUrls ); if (err) goto bail;

    /* save the descriptor as a handle */
    err = desc->calculateSize( (MP4DescriptorPtr) desc ); if (err) goto bail;
    err = MP4SetHandleSize( ipmpToolH,
                            desc->size ); if (err) goto bail;
    err = desc->serialize( (MP4DescriptorPtr) desc,
                           *ipmpToolH ); if (err) goto bail;
bail:
    if ( desc ) desc->destroy( (MP4DescriptorPtr) desc );
    return err;
}

MP4Err MP4NewIPMPToolListDescriptor( MP4Handle ipmpToolListDescrH )
{
    MP4Err err;
    MP4IPMPToolListDescriptorPtr toolList;
    toolList = NULL;
    err = MP4NoErr;
    if ( ipmpToolListDescrH == NULL )
        BAILWITHERROR( MP4BadParamErr );
    
    /* create the ipmp tool list */
    err = MP4CreateIPMPToolListDescriptor( MP4IPMP_ToolListDescriptorTag,
                                           0,
                                           0,
                                           (MP4DescriptorPtr *) &toolList );
    if (err) goto bail;

    /* save the ipmp tool list as a handle */
    err = toolList->calculateSize( (MP4DescriptorPtr)toolList ); if (err) goto bail;
    err = MP4SetHandleSize( ipmpToolListDescrH, toolList->size ); if (err) goto bail;
    err = toolList->serialize( (MP4DescriptorPtr)toolList,
                               *ipmpToolListDescrH ); if (err) goto bail;

bail:
    if ( toolList ) toolList->destroy( (MP4DescriptorPtr) toolList );
    return err;
}

MP4Err MP4AddToolToIPMPToolList( MP4Handle ipmpToolListDescrH,
                                 MP4Handle ipmpToolH )             
{
    MP4Err MP4CreateMemoryInputStream( char *base, u32 size, MP4InputStreamPtr *outStream );

    MP4Err err;
    u32 toolListSize, toolSize;
    MP4InputStreamPtr is;
    MP4IPMPToolListDescriptorPtr toolListDesc;
    MP4DescriptorPtr tool;

    toolListDesc = NULL;
    err = MP4NoErr;
    if ( (ipmpToolListDescrH == NULL) || (ipmpToolH == NULL) )
        BAILWITHERROR( MP4BadParamErr );
    
    /* parse the ipmpToolListDescriptor */
    err = MP4GetHandleSize( ipmpToolListDescrH, 
                            &toolListSize ); if (err) goto bail;
    err = MP4CreateMemoryInputStream( *ipmpToolListDescrH,
                                      toolListSize,
                                      &is ); if (err) goto bail;
    is->debugging = 0;
    err = MP4ParseDescriptor( is, (MP4DescriptorPtr *) &toolListDesc ); if (err) goto bail;
    is->destroy( is );
    if ( toolListDesc == NULL )
        BAILWITHERROR( MP4BadParamErr );

    /* parse the tool */
    err = MP4GetHandleSize( ipmpToolH, &toolSize ); if (err) goto bail;
    err = MP4CreateMemoryInputStream( *ipmpToolH,
                                      toolSize,
                                      &is ); if (err) goto bail;
    is->debugging = 0;
    err = MP4ParseDescriptor( is, &tool ); if (err) goto bail;
    is->destroy( is );
    
    /* add the tool to the ipmp tool list */
    err = toolListDesc->addDescriptor( (MP4DescriptorPtr) toolListDesc,
                                       tool ); if (err) goto bail;
    
    /* save the tool list as a handle */
    err = toolListDesc->calculateSize( (MP4DescriptorPtr) toolListDesc ); if (err) goto bail;
    err = MP4SetHandleSize( ipmpToolListDescrH,
                            toolListDesc->size ); if (err) goto bail;
    err = toolListDesc->serialize( (MP4DescriptorPtr) toolListDesc,
                                    *ipmpToolListDescrH ); if (err) goto bail;

bail:
    if ( toolListDesc ) toolListDesc->destroy( (MP4DescriptorPtr) toolListDesc );
    return err;
}


MP4Err MP4NewIPMPDescriptorPointer( MP4Handle ipmpDescPtrH,
                                    u8 ipmpDescriptorId,
                                    u16 ipmpToolDescrId )
{
    MP4Err err;
    MP4IPMPDescriptorPointerPtr desc;

    err = MP4NoErr;
    desc = NULL;
    /* check the handle */
    if ( ipmpDescPtrH == NULL )
        BAILWITHERROR( MP4BadParamErr );

    /* check the IDs */
    if ( ipmpDescriptorId != 0xFF ) /* ipmp hooks */
    {
        if ( ipmpToolDescrId != 0 )
            BAILWITHERROR( MP4BadParamErr );
    }
    else /* ipmp extensions */
    {
        if ( ipmpToolDescrId == 0 )
            BAILWITHERROR( MP4BadParamErr );
    }

    /* create the descriptor */
    err = MP4CreateIPMPDescriptorPointer( MP4IPMP_DescriptorPointerTag,
                                          0,
                                          0,
                                          (MP4DescriptorPtr *) &desc ); if (err) goto bail;
    desc->ipmpDescriptorId = ipmpDescriptorId;
    desc->ipmpToolDescriptorId = ipmpToolDescrId;

    /* serialize it as a handle */
    err = desc->calculateSize( (MP4DescriptorPtr) desc ); if (err) goto bail;
    err = MP4SetHandleSize( ipmpDescPtrH, desc->size ); if (err) goto bail;
    err = desc->serialize( (MP4DescriptorPtr) desc,
                            *ipmpDescPtrH ); if (err) goto bail;
bail:
    if ( desc ) desc->destroy( (MP4DescriptorPtr) desc );
    return err;
}


MP4Err MP4NewIPMPToolDescriptor( MP4Handle ipmpToolDescH,
                                 u16 ipmpToolDescrId,
                                 u64 ipmpToolIdLowerPart,
                                 u64 ipmpToolIdUpperPart,
                                 MP4Handle ipmpInitializeH )
{
    MP4Err MP4CreateMemoryInputStream( char *base, u32 size, MP4InputStreamPtr *outStream );

    MP4Err err;
    MP4IPMPToolDescriptorPtr desc;

    err = MP4NoErr;
    desc = NULL;
    if ( (ipmpToolDescH == NULL) || (ipmpToolDescrId == 0) )
        BAILWITHERROR( MP4BadParamErr );

    /* create the tool descriptor */
    err = MP4CreateIPMPToolDescriptor( MP4IPMP_ToolDescriptorTag,
                                       0,
                                       0,
                                       (MP4DescriptorPtr *)  &desc ); if (err) goto bail;
    desc->ipmpToolDescriptorId = ipmpToolDescrId;
    desc->ipmpToolId->lowerPart = ipmpToolIdLowerPart;
    desc->ipmpToolId->upperPart = ipmpToolIdUpperPart;

    /* if there's some init data, add it to the descriptor */
    if ( ipmpInitializeH != NULL )
    {
        MP4InputStreamPtr is;
        MP4IPMPInitializePtr init;
        u32 initSize;

        err = MP4CreateIPMPInitialize( &init ); if (err) goto bail;
        err = MP4GetHandleSize( ipmpInitializeH,
                                &initSize ); if (err) goto bail;
        err = MP4CreateMemoryInputStream( *ipmpInitializeH,
                                          initSize,
                                          &is ); if (err) goto bail;
        is->debugging = 0;
        err = init->createFromInputStream( init, is ); if (err) goto bail;
        is->destroy( is );
        desc->initData = init;
    }

    /* save it as handle */
    err = desc->calculateSize( (MP4DescriptorPtr) desc ); if (err) goto bail;
    err = MP4SetHandleSize( ipmpToolDescH,
                            desc->size ); if (err) goto bail;
    err = desc->serialize( (MP4DescriptorPtr) desc,
                            *ipmpToolDescH ); if (err) goto bail;

bail:
    if ( desc ) desc->destroy( (MP4DescriptorPtr) desc );
    return err;
}


MP4Err MP4NewIPMPInitialize( MP4Handle ipmpInitializeH,
                             u8 controlPoint,
                             u8 sequenceCode )
{
    MP4Err err;
    MP4IPMPInitializePtr init;

    init = NULL;
    err = MP4NoErr;
    if ( (ipmpInitializeH == NULL) || (controlPoint == 0xFF) )
        BAILWITHERROR( MP4BadParamErr );
    /* create the structure */
    err = MP4CreateIPMPInitialize( &init ); if (err) goto bail;
    init->controlPointCode = controlPoint;
    init->sequenceCode = sequenceCode;
    
    /* save it as a handle */
    err = init->calculateSize( init ); if (err) goto bail;
    err = MP4SetHandleSize( ipmpInitializeH,
                            init->size ); if (err) goto bail;
    err = init->serialize( init, 
                           *ipmpInitializeH ); if (err) goto bail;

bail:
    if ( init ) init->destroy( init );
    return err;
}

MP4Err MP4AddIPMPDataToIPMPInitialize( MP4Handle ipmpInitializeH,
                                       MP4Handle ipmpDataH )
{
    MP4Err MP4CreateMemoryInputStream( char* base, u32 size, MP4InputStreamPtr* outStream );
    
    MP4Err err;
    MP4IPMPInitializePtr init;
    MP4IPMPXDataPtr data;
    MP4InputStreamPtr is;
    u32 initSize;
    u32 dataSize;

    err = MP4NoErr;
    init = NULL;
    if ( (ipmpInitializeH == NULL) || (ipmpDataH == NULL) )
        BAILWITHERROR( MP4BadParamErr );
    /* build the MP4IPMPInitialize structure */
    err = MP4CreateIPMPInitialize( &init );if (err) goto bail;
    err = MP4GetHandleSize( ipmpInitializeH,
                            &initSize ); if (err) goto bail;
    err = MP4CreateMemoryInputStream( *ipmpInitializeH,
                                      initSize,
                                      &is ); if (err) goto bail;
    is->debugging = 0;
    err = init->createFromInputStream( init, is ); if (err) goto bail;
    is->destroy( is );
    
    /* parse the data */
    err = MP4GetHandleSize( ipmpDataH,
                            &dataSize ); if (err) goto bail;
    err = MP4CreateMemoryInputStream( *ipmpDataH,
                                      dataSize,
                                      &is ); if (err) goto bail;
    is->debugging = 0;
    err = MP4ParseIPMPXData( is, &data ); if (err) goto bail;

    /* add the data to the init structure */
    err = init->addIPMPXData( init, data ); if (err) goto bail;

    /* save it as a handle */
    err = init->calculateSize( init ); if (err) goto bail;
    err = MP4SetHandleSize( ipmpInitializeH,
                            init->size ); if (err) goto bail;
    err = init->serialize( init, 
                           *ipmpInitializeH ); if (err) goto bail;   
bail:
    if ( init ) init->destroy( init );
    return err;
}

MP4Err MP4AddIPMPDataToIPMPToolDescriptor( MP4Handle ipmpToolDescrH,
                                           MP4Handle ipmpDataH )
{
    MP4Err MP4CreateMemoryInputStream( char* base, u32 size, MP4InputStreamPtr* outStream );
    
    MP4Err err;
    MP4IPMPToolDescriptorPtr desc;
    MP4IPMPXDataPtr data;
    MP4InputStreamPtr is;
    u32 descSize;
    u32 dataSize;

    err = MP4NoErr;
    desc = NULL;
    if ( (ipmpToolDescrH == NULL) || (ipmpDataH == NULL) )
        BAILWITHERROR( MP4BadParamErr );
    /* parse the descriptor */
    err = MP4GetHandleSize( ipmpToolDescrH,
                            &descSize ); if (err) goto bail;
    err = MP4CreateMemoryInputStream( *ipmpToolDescrH,
                                      descSize,
                                      &is ); if (err) goto bail;
    is->debugging = 0;
    err = MP4ParseDescriptor( is, (MP4DescriptorPtr *) &desc );if (err) goto bail;
    is->destroy( is );
    
    /* parse the data */
    err = MP4GetHandleSize( ipmpDataH,
                            &dataSize ); if (err) goto bail;
    err = MP4CreateMemoryInputStream( *ipmpDataH,
                                      dataSize,
                                      &is ); if (err) goto bail;
    is->debugging = 0;
    err = MP4ParseIPMPXData( is, &data ); if (err) goto bail;

    /* add the data to the descriptor */
    err = desc->addIPMPXData( (MP4DescriptorPtr) desc, data ); if (err) goto bail;

    /* save it as a handle */
    err = desc->calculateSize( (MP4DescriptorPtr)desc ); if (err) goto bail;
    err = MP4SetHandleSize( ipmpToolDescrH,
                            desc->size ); if (err) goto bail;
    err = desc->serialize( (MP4DescriptorPtr) desc, 
                           *ipmpToolDescrH ); if (err) goto bail;   
bail:
    if ( desc ) desc->destroy( (MP4DescriptorPtr) desc );
    return err;
}

MP4Err MP4NewIPMPToolDescriptorUpdate( MP4Handle ipmpToolDescrUpdateH )
{
    MP4Err err;
    MP4IPMPToolDescriptorUpdatePtr cmd;

    cmd = NULL;
    err = MP4NoErr;
    if ( ipmpToolDescrUpdateH == NULL )
        BAILWITHERROR( MP4BadParamErr );
    
    /* create the command */
    err = MP4CreateIPMPToolDescriptorUpdate( MP4IPMP_ToolDescriptorUpdateTag,
                                             0,
                                             0,
                                             (MP4DescriptorPtr *) &cmd ); if (err) goto bail;
    /* save it as a handle */
    err = cmd->calculateSize( (MP4DescriptorPtr) cmd ); if (err) goto bail;
    err = MP4SetHandleSize( ipmpToolDescrUpdateH,
                            cmd->size );
    err = cmd->serialize( (MP4DescriptorPtr) cmd,
                          *ipmpToolDescrUpdateH ); if (err) goto bail;
bail:
    if ( cmd ) cmd->destroy( (MP4DescriptorPtr) cmd );
    return err;
}

MP4Err MP4AddIPMPToolDescriptorToUpdate( MP4Handle ipmpToolDescrUpdateH,
                                         MP4Handle ipmpToolDescrH )
{
    MP4Err MP4CreateMemoryInputStream( char *base, u32 size, MP4InputStreamPtr *outStream );

    MP4Err err;
    MP4IPMPToolDescriptorUpdatePtr cmd;
    MP4InputStreamPtr is;
    MP4IPMPToolDescriptorPtr desc;
    u32 cmdSize;
    u32 descSize;

    err = MP4NoErr;
    cmd = NULL;
    if ( (ipmpToolDescrUpdateH == NULL) || (ipmpToolDescrH == NULL) )
        BAILWITHERROR( MP4BadParamErr );

    /* parse the command */
    err = MP4GetHandleSize( ipmpToolDescrUpdateH,
                            &cmdSize ); if (err) goto bail;
    err = MP4CreateMemoryInputStream( *ipmpToolDescrUpdateH,
                                      cmdSize,
                                      &is ); if (err) goto bail;
    is->debugging = 0;
    err = MP4ParseCommand( is, (MP4DescriptorPtr *) &cmd ); if (err) goto bail;
    is->destroy( is );
    
    /* parse the descriptor */
    err = MP4GetHandleSize( ipmpToolDescrH,
                            &descSize ); if (err) goto bail;
    err = MP4CreateMemoryInputStream( *ipmpToolDescrH,
                                      descSize,
                                      &is ); if (err) goto bail;
    is->debugging = 0;
    err = MP4ParseDescriptor( is, (MP4DescriptorPtr *) &desc ); if (err) goto bail;
    is->destroy( is );

    /* add the descriptor to the command */
    err = cmd->addDescriptor( (MP4DescriptorPtr) cmd,
                              (MP4DescriptorPtr) desc ); if (err) goto bail;

    /* serialize the command as a handle */
    err = cmd->calculateSize( (MP4DescriptorPtr) cmd ); if (err) goto bail;
    err = MP4SetHandleSize( ipmpToolDescrUpdateH,
                            cmd->size ); if (err) goto bail;
    err = cmd->serialize( (MP4DescriptorPtr) cmd, 
                          *ipmpToolDescrUpdateH ); if (err) goto bail;
bail:
    if ( cmd ) cmd->destroy( (MP4DescriptorPtr) cmd );
    return err;
}

