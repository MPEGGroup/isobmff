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
 @header PCMAtoms
 PCMAtoms defines PCM audio specific atoms to describe PCM audio formats.
 @copyright Apple, Inc.
 @updated 2019-01-01
 @author Apple, Inc.
 @version 1.0
 */

#ifndef PCMAtoms_h
#define PCMAtoms_h

#include "MP4Atoms.h"

/* For PCM configuration box */
#define ENDIAN_FORMAT_FLAG_LITTLE   0x01

#define PCM_SAMPLE_SIZE_INT_16      16
#define PCM_SAMPLE_SIZE_INT_24      24
#define PCM_SAMPLE_SIZE_INT_32      32

#define PCM_SAMPLE_SIZE_FLOAT_32    32
#define PCM_SAMPLE_SIZE_FLOAT_64    64

/* For Channel Layout box */
#define SPEAKER_POSITION_EXPLICIT   126
#define STREAM_STRUCTURE_CHANNELS   0x1
#define STREAM_STRUCTURE_OBJECTS    0x2

/*!
 * @discussion Creates and initializes an atom structure with a certain type, memory will be allocated
 * @param atomType Type of the atom, which will be created
 * @param outAtom pointer to a structure, which will be created
 * @return An MP4Err, which is defined in libisomediafile.a; MP4NoErr if nothing fails
 */
MP4Err MP4CreatePCMAtoms ( u32 atomType, MP4AtomPtr *outAtom );

/*!
 * @discussion Reads an atom from inputStream. If type of atom not in protolist it fails.
 * @param inputStream MP4InputStreamPtr (can be found in libisomediafile.a). Data will be read from this stream.
 * @param protoList List of atoms, which are valid for being parsed. Can be NULL.
 * @param defaultAtom Prototype of parsed atom. Can be NULL.
 * @param outAtom pointer to a structure, which will be created
 * @return An MP4Err, which is defined in libisomediafile.a; MP4NoErr if nothing fails
 */
MP4Err MP4ParsePCMAtomUsingProtoList ( MP4InputStreamPtr inputStream, u32* protoList, u32 defaultAtom, MP4AtomPtr *outAtom );

/*!
 * @discussion Reads an atom from inputStream.
 * @param inputStream MP4InputStreamPtr (can be found in libisomediafile.a). Data will be read from this stream.
 * @param outAtom pointer to a structure, which will be created
 * @return A MP4Err, which is defined libisomediafile.a; MP4NoErr if nothing fails
 */
MP4Err MP4ParsePCMAtom ( MP4InputStreamPtr inputStream, MP4AtomPtr *outAtom );

#endif
