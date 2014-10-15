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
derivative works. Copyright (c) 2014.
*/

/*!
 @header FileFormatDrcToInStreamDrc
 @copyright Apple
 @updated 2014-10-06
 @author Armin Trattnig
 @version 1.0
 */

#ifndef FileFormatDrcToInStreamDrc_h
#define FileFormatDrcToInStreamDrc_h

#include "DRCAtoms.h"
#include "DRCData.h"


/*!
 * @discussion Adds DRC atoms to a audio sample entry. All the DRC atoms will be created and filled with
 data found in the drc bitstream.
 * @param drcData Pointer to an initialized DRCData structure
 * @param audioSampleEntry Pointer to an initialized MP4AudioSampleEntryAtom
 * @return An MP4Err, which is defined libisomediafile.a; MP4NoErr if nothing fails
 */
MP4Err  addDRCDataToAudioSampleEntry    (DRCData *drcData, MP4AudioSampleEntryAtomPtr audioSampleEntry);

/*!
 * @discussion Creates and adds a LoundessAtom to a MP4 track.
 * @param drcData Pointer to an initialized DRCData structure
 * @param track The MP4 track
 * @return An MP4Err, which is defined libisomediafile.a; MP4NoErr if nothing fails
 */
MP4Err  addLoudnessInfoToTrackAtom      (DRCData *drcData, MP4Track track);

#endif