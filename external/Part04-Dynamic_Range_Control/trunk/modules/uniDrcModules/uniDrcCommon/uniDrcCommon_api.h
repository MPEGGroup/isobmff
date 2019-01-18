/***********************************************************************************
 
 This software module was originally developed by
 
 Apple Inc.
 
 in the course of development of the ISO/IEC 23003-4 for reference purposes and its
 performance may not have been optimized. This software module is an implementation
 of one or more tools as specified by the ISO/IEC 23003-4 standard. ISO/IEC gives
 you a royalty-free, worldwide, non-exclusive, copyright license to copy, distribute,
 and make derivative works of this software module or modifications  thereof for use
 in implementations or products claiming conformance to the ISO/IEC 23003-4 standard
 and which satisfy any specified conformance criteria. Those intending to use this
 software module in products are advised that its use may infringe existing patents.
 ISO/IEC have no liability for use of this software module or modifications thereof.
 Copyright is not released for products that do not conform to the ISO/IEC 23003-4
 standard.
 
 Apple Inc. retains full right to modify and use the code for its own purpose,
 assign or donate the code to a third party and to inhibit third parties from using
 the code for products that do not conform to MPEG-related ITU Recommendations and/or
 ISO/IEC International Standards.
 
 This copyright notice must be included in all copies or derivative works.
 
 Copyright (c) ISO/IEC 2014.
 
 ***********************************************************************************/

#ifndef _UNI_DRC_COMMON_API_H_
#define _UNI_DRC_COMMON_API_H_

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct T_UNI_DRC_CONFIG_STRUCT  *HANDLE_UNI_DRC_CONFIG;
typedef struct T_LOUDNESS_INFO_SET_STRUCT  *HANDLE_LOUDNESS_INFO_SET;
typedef struct T_UNI_DRC_GAIN_STRUCT *HANDLE_UNI_DRC_GAIN;
typedef struct T_UNI_DRC_INTERFACE_STRUCT  *HANDLE_UNI_DRC_INTERFACE;
typedef struct T_UNI_DRC_SEL_PROC_OUTPUT *HANDLE_UNI_DRC_SEL_PROC_OUTPUT;

#ifdef __cplusplus
}
#endif
#endif
