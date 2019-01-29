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

/*!
 @header Testing
Testing provides functions to test PCMAtoms and PCM related generic Atoms from libisomediafile.a
 @copyright Apple, Inc.
 @updated 2019-01-01
 @author Apple, Inc.
 @version 1.0
 */

#ifndef Testing_h
#define Testing_h

#include "MP4Atoms.h"
#include "PCMAtoms.h"


/*!
 * @discussion Calls every test function defined in this header multiple times
 * @param iterations Determines how often the test iteration is called
 */
MP4Err testAll (u32 iterations);

/*!
 * @discussion Assigns random values to the fields of a ChannelLayoutAtom and serializes to a buffer.
 The buffer is then parsed the ChannelLayoutAtom is recreated and every field will be checked.
 */
MP4Err testChannelLayoutAtom (void);

/*!
 * @discussion Tests a PCMConfigAtom
 */
MP4Err testPCMConfigAtom(void);

#endif
