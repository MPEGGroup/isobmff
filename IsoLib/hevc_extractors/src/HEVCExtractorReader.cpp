/* This software module was originally developed by Apple Computer, Inc.
 * in the course of development of MPEG-4.
 * This software module is an implementation of a part of one or
 * more MPEG-4 tools as specified by MPEG-4.
 * ISO/IEC gives users of MPEG-4 free license to this
 * software module or modifications thereof for use in hardware
 * or software products claiming conformance to MPEG-4 only for evaluation and testing purposes.
 * Those intending to use this software module in hardware or software
 * products are advised that its use may infringe existing patents.
 * The original developer of this software module and his/her company,
 * the subsequent editors and their companies, and ISO/IEC have no
 * liability for use of this software module or modifications thereof
 * in an implementation.
 *
 * Copyright is not released for non MPEG-4 conforming
 * products. Apple Computer, Inc. retains full right to use the code for its own
 * purpose, assign or donate the code to a third party and to
 * inhibit third parties from using the code for non
 * MPEG-4 conforming products.
 * This copyright notice must be included in all copies or
 * derivative works. */

/* Copyright© 2017 Gesellschaft zur Foerderung der angewandten Forschung e.V.
 * acting on behalf of its Fraunhofer Institute for Telecommunications,
 * Heinrich Hertz Institute, HHI
 * All rights reserved.
*/

#include "HEVCExtractorReader.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <set>

#include <MP4TrackReader.h>

extern "C"
{
MP4_EXTERN(MP4Err) ISOGetRESVOriginalFormat(MP4Handle sampleEntryH,
                                            u32* outOrigFmt);
MP4_EXTERN(MP4Err) ISOGetRESVSampleDescriptionPS(MP4Handle sampleEntryH,
                                                 MP4Handle ps,
                                                 u32 where,
                                                 u32 index);
MP4_EXTERN(MP4Err) ISOGetHEVCSampleDescriptionPS(MP4Handle sampleEntryH,
                                                 MP4Handle ps,
                                                 u32 where,
                                                 u32 index);
MP4_EXTERN(MP4Err) ISOGetRESVLengthSizeMinusOne(MP4Handle sampleEntryH,
                                                u32* out);
}


int32_t HEVCExtractorReader::getPSfromSampleEntry(ISOHandle sampleEntryH, ISOHandle vpsH, ISOHandle spsH, ISOHandle ppsH) const
{
  ISOErr err;
  err = ISOGetRESVSampleDescriptionPS(sampleEntryH, vpsH, 32, 1);
  if(err)
  { // try to get PS from hvc1
    err = ISOGetHEVCSampleDescriptionPS(sampleEntryH, vpsH, 32, 1); if(err) { return MP4InvalidMediaErr; }
  }
  err = ISOGetRESVSampleDescriptionPS(sampleEntryH, spsH, 33, 1);
  if(err)
  {
    err = ISOGetHEVCSampleDescriptionPS(sampleEntryH, spsH, 33, 1); if(err) { return MP4InvalidMediaErr; }
  }
  err = ISOGetRESVSampleDescriptionPS(sampleEntryH, ppsH, 34, 1);
  if(err)
  {
    err = ISOGetHEVCSampleDescriptionPS(sampleEntryH, ppsH, 34, 1); if(err) { return MP4InvalidMediaErr; }
  }
  return err;
}

int32_t HEVCExtractorReader::getPSwithStartCodesFromTrack(uint32_t uiTrackID, std::vector<char>& rvOut) const
{
  ISOTrackReader* pTrackReader = getTrackReader(uiTrackID);
  if(!pTrackReader) { return MP4NotFoundErr; }

  ISOErr err = MP4NoErr;
  ISOHandle sampleEntryH = 0;
  ISOHandle vpsHandle = 0;
  ISOHandle spsHandle = 0;
  ISOHandle ppsHandle = 0;
  ISONewHandle(1, &sampleEntryH);
  ISONewHandle(1, &vpsHandle);
  ISONewHandle(1, &spsHandle);
  ISONewHandle(1, &ppsHandle);

  err = MP4TrackReaderGetCurrentSampleDescription(*pTrackReader, sampleEntryH);
  if(MP4NoErr==err)
  {
    err = getPSfromSampleEntry(sampleEntryH, vpsHandle, spsHandle, ppsHandle);
    if(MP4NoErr==err)
    {
      uint32_t sizeVps, sizeSps, sizePps;
      ISOGetHandleSize(vpsHandle, &sizeVps);
      ISOGetHandleSize(spsHandle, &sizeSps);
      ISOGetHandleSize(ppsHandle, &sizePps);

      char aStartCode[] = "\x00\x00\x00\x01";
      rvOut.clear();
      rvOut.insert(rvOut.end(), aStartCode, aStartCode+4);
      rvOut.insert(rvOut.end(), *vpsHandle, *vpsHandle + sizeVps);
      rvOut.insert(rvOut.end(), aStartCode, aStartCode+4);
      rvOut.insert(rvOut.end(), *spsHandle, *spsHandle + sizeSps);
      rvOut.insert(rvOut.end(), aStartCode, aStartCode+4);
      rvOut.insert(rvOut.end(), *ppsHandle, *ppsHandle + sizePps);
    }
  }
  ISODisposeHandle(sampleEntryH);
  ISODisposeHandle(vpsHandle);
  ISODisposeHandle(spsHandle);
  ISODisposeHandle(ppsHandle);
  return err;
}

int32_t HEVCExtractorReader::getLengthSizeMinusOneFlag(uint32_t uiTrackID, uint32_t& rUiFlag) const
{
  ISOErr err;
  ISOTrackReader* pTrackReader = getTrackReader(uiTrackID);
  if(!pTrackReader) { return MP4NotFoundErr; }

  ISOHandle sampleEntryH;
  err = ISONewHandle(1, &sampleEntryH);
  err = MP4TrackReaderGetCurrentSampleDescription(*pTrackReader, sampleEntryH);
  if(MP4NoErr==err)
  {
    err = ISOGetRESVLengthSizeMinusOne(sampleEntryH, &rUiFlag);
  }

  ISODisposeHandle(sampleEntryH);
  return err;
}

std::string HEVCExtractorReader::getOriginalFormat(uint32_t uiTrackID) const
{
  ISOErr err;
  std::string strRet = std::string();
  ISOTrackReader* pTrackReader = getTrackReader(uiTrackID);
  if(!pTrackReader) { return strRet; }

  ISOHandle sampleEntryH;
  ISONewHandle(1, &sampleEntryH);
  err = MP4TrackReaderGetCurrentSampleDescription(*pTrackReader, sampleEntryH);

  if(MP4NoErr==err)
  {
    uint32_t uiOrigFormat = 0;
    err = ISOGetRESVOriginalFormat(sampleEntryH, &uiOrigFormat);
    if(MP4NoErr==err)
    {
      if(uiOrigFormat==ISOHEVCSampleEntryAtomType) { strRet = "hvc1"; }
      else if(uiOrigFormat==ISOLHEVCSampleEntryAtomType) { strRet = "hvc2"; }
    }
  }
  ISODisposeHandle(sampleEntryH);
  return strRet;
}

HEVCExtractorReader::TrackIDSet HEVCExtractorReader::getRefTrackIDs(MP4Track trak) const
{
  TrackIDSet ret;
  ISOErr err;
  uint32_t uiRefTrackCnt = 0;
  MP4GetTrackReferenceCount(trak,
                            MP4_FOUR_CHAR_CODE('s', 'c', 'a', 'l'),
                            &uiRefTrackCnt);
  for(uint32_t uiRefIdx=1; uiRefIdx<=uiRefTrackCnt; ++uiRefIdx)
  {
    ISOTrack refTrak;
    err = MP4GetTrackReference(trak,
                         MP4_FOUR_CHAR_CODE('s', 'c', 'a', 'l'),
                         uiRefIdx,
                         &refTrak);
    if(err) { continue; }
    uint32_t uiTrackIDref = 0;
    err = ISOGetTrackID(refTrak, &uiTrackIDref); if(err) { continue; }
    ret.push_back(uiTrackIDref);
  }
  return ret;
}

HEVCExtractorReader::TrackIDSet HEVCExtractorReader::getHvc1TrackIDs() const
{
  return m_vHvc1TrackIDs;
}

HEVCExtractorReader::TrackIDSet HEVCExtractorReader::getHvc2TrackIDs() const
{
  return m_vHvc2TrackIDs;
}

MP4TrackReader* HEVCExtractorReader::getTrackReader(uint32_t uiTrackID) const
{
  TrackReaderPtrMap::const_iterator it = m_mTrackReaders.find(uiTrackID);
  if(m_mTrackReaders.end()==it) { return 0; }
  return it->second;
}

uint32_t HEVCExtractorReader::getCurrentSampleNr(uint32_t uiTrackID) const
{
  ISOTrackReader* pTrackReader = getTrackReader(uiTrackID);
  if(!pTrackReader) { return 0; }
  uint32_t uiCurrentSampleNr;
  MP4TrackReaderGetCurrentSampleNumber(*pTrackReader, &uiCurrentSampleNr);
  return uiCurrentSampleNr;
}

uint32_t HEVCExtractorReader::getNextSampleNr(uint32_t uiTrackID) const
{
  ISOTrackReader* pTrackReader = getTrackReader(uiTrackID);
  if(!pTrackReader) { return 0; }
  MP4TrackReaderPtr reader;
  reader = (MP4TrackReaderPtr) *pTrackReader;
  return reader->nextSampleNumber;
}

std::vector<char> HEVCExtractorReader::getNextSample()
{
  if(isHvc2Track(m_uiSelectedTrackID))
  {
    return getNextAUResolveExtractors();
  }
  return getNextAUwithoutExtractors();
}

int32_t HEVCExtractorReader::getNextAU(uint32_t uiTrackID,
                                 ISOHandle sampleH,
                                 uint32_t* pUiSize)
{
  ISOTrackReader* pTrackReader = getTrackReader(uiTrackID);
  if(!pTrackReader) { return MP4NotFoundErr; }

  uint32_t sampleFlags;
  int32_t cts, dts;
  return MP4TrackReaderGetNextAccessUnit(*pTrackReader, sampleH, pUiSize, &sampleFlags, &cts, &dts);
}

std::vector<char> HEVCExtractorReader::getNextAUwithoutExtractors()
{
  ISOErr err;
  std::vector<char> vRet;
  uint32_t uiSize = 0;
  ISOHandle sampleH;
  ISONewHandle(0, &sampleH);
  err = getNextAU(m_uiSelectedTrackID, sampleH, &uiSize);
  if(MP4NoErr==err)
  {
    vRet.insert(vRet.begin(), *sampleH, *sampleH+uiSize);
  }
  ISODisposeHandle(sampleH);
  return vRet;
}

std::vector<char> HEVCExtractorReader::getNextAUResolveExtractors()
{
  ISOErr err;
  std::vector<char> vRet;
  if(!isHvc2Track(m_uiSelectedTrackID)) { return vRet; }
  uint32_t uiSize = 0;
  uint32_t uiNextSample = getNextSampleNr(m_uiSelectedTrackID);
  ISOHandle sampleH;
  ISONewHandle(0, &sampleH);

  // TODO: remove it when seeking is implemented
  TrackIDSet rUsedTrackIds;
  rUsedTrackIds.push_back(m_uiSelectedTrackID);

  err = getNextAU(m_uiSelectedTrackID, sampleH, &uiSize);
  if(err || uiSize==0) { return vRet; }

  uint32_t uiNalStart = 0;
  while(uiNalStart<uiSize)
  {
    uint32_t uiCurNalPtr = uiNalStart + 4; // current nalu pointer
    int32_t uiNalSize;
    uiNalSize = ((*(*sampleH+uiNalStart)&0xff)   << 24) |
                ((*(*sampleH+uiNalStart+1)&0xff) << 16) |
                ((*(*sampleH+uiNalStart+2)&0xff) << 8)  |
                ( *(*sampleH+uiNalStart+3)&0xff);
    uiNalStart += 4 + uiNalSize; // next nalu start pos

    //parse nalu header
    uint32_t uiNalType = (*(*sampleH+uiCurNalPtr) & 0x7e)>>1;
    if(49 == uiNalType)
    {
      // extractor NALU found / now iterate over all constructors
      uint32_t uiConstrStart = 2;
      bool bEndOfNalU = false;
      bool bNaluLengthRewrite = false;
      int32_t iNaluLengthIdx = -1;
      uint32_t uiNaluLengthCorrect = 0;
      do
      {
        uint32_t uiConstrType = *(*sampleH+uiCurNalPtr+uiConstrStart++);
        if(0 == uiConstrType) // sample constructor
        {
          if(0>iNaluLengthIdx) { iNaluLengthIdx = vRet.size(); }
          uint32_t uiTrackRefIdx = *(*sampleH+uiCurNalPtr+uiConstrStart++);
          int32_t  iSampleOffset = *(*sampleH+uiCurNalPtr+uiConstrStart++);
          uint32_t uiDataOffset  = 0;
          uint32_t uiDataLength  = 0;

          // set data_offset and data_length fields dependent on length_size_minus_one flag
          uint32_t uiLengthSizeMinusOne = m_mLenSizeMinOne.at(m_uiSelectedTrackID);

          switch(uiLengthSizeMinusOne)
          {
          case 0:
            uiDataOffset = *(*sampleH+uiCurNalPtr+uiConstrStart++);
            uiDataLength = *(*sampleH+uiCurNalPtr+uiConstrStart++);
            break;
          case 1:
            uiDataOffset = (((*(*sampleH+uiCurNalPtr+uiConstrStart))&0xff) << 8) |
                           ( (*(*sampleH+uiCurNalPtr+uiConstrStart+1))&0xff);
            uiConstrStart += 2;
            uiDataLength = (((*(*sampleH+uiCurNalPtr+uiConstrStart))&0xff) << 8) |
                           ( (*(*sampleH+uiCurNalPtr+uiConstrStart+1))&0xff);
            uiConstrStart += 2;
            break;
          case 2:
              uiDataOffset = (((*(*sampleH+uiCurNalPtr+uiConstrStart))&0xff) << 16)   |
                             (((*(*sampleH+uiCurNalPtr+uiConstrStart+1))&0xff) << 8)  |
                             ( (*(*sampleH+uiCurNalPtr+uiConstrStart+2))&0xff);
              uiConstrStart += 3;
              uiDataLength = (((*(*sampleH+uiCurNalPtr+uiConstrStart))&0xff) << 16) |
                             (((*(*sampleH+uiCurNalPtr+uiConstrStart+1))&0xff) << 8)  |
                             ( (*(*sampleH+uiCurNalPtr+uiConstrStart+2))&0xff);
              uiConstrStart += 3;
              break;
          case 3:
            uiDataOffset = (((*(*sampleH+uiCurNalPtr+uiConstrStart))&0xff) << 24)   |
                           (((*(*sampleH+uiCurNalPtr+uiConstrStart+1))&0xff) << 16) |
                           (((*(*sampleH+uiCurNalPtr+uiConstrStart+2))&0xff) << 8)  |
                           ( (*(*sampleH+uiCurNalPtr+uiConstrStart+3))&0xff);
            uiConstrStart += 4;
            uiDataLength = (((*(*sampleH+uiCurNalPtr+uiConstrStart))&0xff) << 24)   |
                           (((*(*sampleH+uiCurNalPtr+uiConstrStart+1))&0xff) << 16) |
                           (((*(*sampleH+uiCurNalPtr+uiConstrStart+2))&0xff) << 8)  |
                           ( (*(*sampleH+uiCurNalPtr+uiConstrStart+3))&0xff);
            uiConstrStart += 4;
            break;
          default:
            return vRet;
          }

          // get trackID of the track we depend on in this sample constructor
          uint32_t uiRefTrackID = m_mHvc2Dependencies[m_uiSelectedTrackID].at(uiTrackRefIdx-1);

          rUsedTrackIds.push_back(uiRefTrackID);

          // get the correct sample number
          if(0!=iSampleOffset)
          {
            // we have to change the sample number of the reference track
            uint32_t uiNextSampleCorrected = uiNextSample + iSampleOffset;
            setNextSampleNr(uiRefTrackID, uiNextSampleCorrected);
          }
          else
          {
            setNextSampleNr(uiRefTrackID, uiNextSample);
          }

          // now read the sample from referenced track
          ISOHandle sampleFromConstrH;
          ISONewHandle(0, &sampleFromConstrH);
          uint32_t uiRefSampleSize;
          err = getNextAU(uiRefTrackID, sampleFromConstrH, &uiRefSampleSize);
          if(err) {assert(0); ISODisposeHandle(sampleFromConstrH); continue; }

          // check the sample size and if data_offset and data_length values are correct
          if(0==uiDataLength)
          { // uiDataOffset shall refer to the beginning of a NALU length field and the entire NALU is copied
            uiDataLength = uiRefSampleSize;
          }
          if(uiDataOffset+uiDataLength != uiRefSampleSize)
          {
            if(static_cast<uint64_t>(uiDataOffset)+uiDataLength > uiRefSampleSize)
            {
              //When data_offset + data_length is greater than the size of the sample, the bytes from the byte
              // pointed to by data_offset until the end of the sample, inclusive, are copied
              uiDataLength = uiRefSampleSize - uiDataOffset;
              bNaluLengthRewrite = true;
            }
            else { assert(0); } // this should not happen if the file is OMAF compliant
          }
          // now get the right portion of the sample and write it to the output vector
          vRet.insert(vRet.end(), *sampleFromConstrH+uiDataOffset, *sampleFromConstrH+uiDataOffset+uiDataLength);
          uiNaluLengthCorrect += uiDataLength;
          ISODisposeHandle(sampleFromConstrH); // clean
          bEndOfNalU = (uiConstrStart >= uiNalSize);
        }
        else if(2 == uiConstrType) // inline constructor
        {
          if(0 > iNaluLengthIdx) { iNaluLengthIdx = vRet.size(); }
          uint32_t uiLengthInline = *(*sampleH+uiCurNalPtr+uiConstrStart++);

          // write data from inline constructor to the output vector
          vRet.insert(vRet.end(),
                      *sampleH+uiCurNalPtr+uiConstrStart,
                      *sampleH+uiCurNalPtr+uiConstrStart+uiLengthInline);
          uiNaluLengthCorrect += uiLengthInline;
          uiConstrStart += uiLengthInline;
          bEndOfNalU = (uiConstrStart >= uiNalSize);
        }
        else
        { // unknown constructor
          assert(0);
          bEndOfNalU = true;
          continue;
        }
      }while(!bEndOfNalU);

      if(bNaluLengthRewrite)
      {
        uiNaluLengthCorrect -= 4;
        setLengthField(vRet, iNaluLengthIdx, uiNaluLengthCorrect);
      }

      uint32_t uiNaluLengthParsed = getLengthField(vRet, iNaluLengthIdx);

      if(uiNaluLengthCorrect<uiNaluLengthParsed)
      {
        // Resolution of an extractor may result in a reconstructed payload for which there are fewer
        // bytes than what is indicated in the NALUnitLength of the first NAL in that reconstructed payload.
        // In such cases, readers shall assume that only a single NAL unit was reconstructed by the
        // extractors, and shall rewrite the NALUnitLength of that NAL to the appropriate value
        uiNaluLengthCorrect -= 4;
        setLengthField(vRet, iNaluLengthIdx, uiNaluLengthCorrect);
      }
    }
    else
    {
      std::cout << " NALU is not of type 49 (Extractors) copy the entire NALU\n";
      vRet.insert(vRet.end(),
                  *sampleH+uiCurNalPtr-4,
                  *sampleH+uiCurNalPtr-4+uiNalSize);
      setLengthField(vRet, vRet.size()-uiNalSize, uiNalSize-4); // replace the length field with payload length only
    }
  }
  skipNotUsedAUs(rUsedTrackIds);
  ISODisposeHandle(sampleH);
  return vRet;
}

std::string HEVCExtractorReader::getAllDependencies(uint32_t uiTrackID) const
{
  std::stringstream ss;

  TrackIDSetMap::const_iterator it = m_mHvc2Dependencies.find(uiTrackID);
  if(it==m_mHvc2Dependencies.end()) return std::string();

  TrackIDSet vIDs = it->second;
  for(uint32_t i=0; i<vIDs.size(); ++i)
  {
    ss << vIDs[i] << ",";
  }
  return ss.str();
}

void HEVCExtractorReader::setNextSampleNr(uint32_t uiTrackID, uint32_t uiNextSampleNr)
{
  ISOTrackReader* pTrackReader = getTrackReader(uiTrackID);
  if(!pTrackReader) { return; }
  MP4TrackReaderPtr reader;
  reader = (MP4TrackReaderPtr) *pTrackReader;
  reader->nextSampleNumber = uiNextSampleNr;
}

void HEVCExtractorReader::setSelectedTrackID(uint32_t uiTrackID)
{
  m_uiSelectedTrackID = uiTrackID;
}

int32_t HEVCExtractorReader::init(std::string strFileName, bool bForce)
{
  ISOErr err;
  err = ISOOpenMovieFile(&m_cMovieBox, strFileName.c_str(), MP4OpenMovieNormal); if(err) { return err; }

  // iterate over all tracks
  uint32_t uiTrackCnt = 0;
  ISOGetMovieTrackCount(m_cMovieBox, &uiTrackCnt);
  for(uint32_t i=1; i<=uiTrackCnt; ++i)
  {
    ISOTrack trak;
    err = ISOGetMovieIndTrack( m_cMovieBox, i, &trak); if(err) { continue; }

    // get trackID
    uint32_t uiTrackID = 0;
    ISOGetTrackID(trak, &uiTrackID);

    // init track readers
    ISOTrackReader*  pReader = new ISOTrackReader;
    err = ISOCreateTrackReader(trak, pReader);
    if(err)
    {
      MP4DisposeTrackReader(*pReader);
      delete pReader;
      continue;
    }
    m_mTrackReaders[uiTrackID] = pReader;

    // get LengthSizeMinusOne flags
    uint32_t uiLenSizeMinOne = m_uiDefLenSizeMinOne;
    if(m_uiDefLenSizeMinOne==0)
    {
      err = getLengthSizeMinusOneFlag(uiTrackID, uiLenSizeMinOne);
      if(err)
      {
        std::cerr << "could not get lensizeminusone flag: "<< err << std::endl;
        if(!bForce) continue;
        uiLenSizeMinOne = 3;
      }
    }
    m_mLenSizeMinOne[uiTrackID] = uiLenSizeMinOne;

    /// TODO: check why first NextSampleNumber is 4 and not 1
    setNextSampleNr(uiTrackID, getCurrentSampleNr(uiTrackID)+1);

    // get original format
    std::string strOriginalFormat = getOriginalFormat(uiTrackID);
    if(strOriginalFormat.find("hvc1")!=std::string::npos)
    {
      m_vHvc1TrackIDs.push_back(uiTrackID);
    }
    else if(strOriginalFormat.find("hvc2")!=std::string::npos)
    {
      m_vHvc2TrackIDs.push_back(uiTrackID);
      m_mHvc2Dependencies[uiTrackID] = getRefTrackIDs(trak);
    }
    else // original format not found, asume that we have a hvc1 track
    {
      m_vHvc1TrackIDs.push_back(uiTrackID);
    }
  }
  m_bInitialized = true;
  return MP4NoErr;
}

int32_t HEVCExtractorReader::reInit()
{
  if(!m_bInitialized) return -1;
  ISOErr err;

  // clear old Track readers
  for(TrackReaderPtrMap::iterator it = m_mTrackReaders.begin();
          it != m_mTrackReaders.end(); ++it)
  {
    MP4DisposeTrackReader(*(it->second));
    delete it->second;
  }
  m_mTrackReaders.clear();

  uint32_t uiTrackCnt = 0;
  ISOGetMovieTrackCount(m_cMovieBox, &uiTrackCnt);
  for(uint32_t i=1; i<=uiTrackCnt; ++i)
  {
    ISOTrack trak;
    err = ISOGetMovieIndTrack( m_cMovieBox, i, &trak); if(err) { continue; }

    // get trackID
    uint32_t uiTrackID = 0;
    ISOGetTrackID(trak, &uiTrackID);

    // init track readers
    ISOTrackReader*  pReader = new ISOTrackReader;
    err = ISOCreateTrackReader(trak, pReader);
    if(err)
    {
      MP4DisposeTrackReader(*pReader);
      delete pReader;
      continue;
    }
    m_mTrackReaders[uiTrackID] = pReader;
    setNextSampleNr(uiTrackID, getCurrentSampleNr(uiTrackID)+1);
  }
  return 0;
}

bool HEVCExtractorReader::isHvc2Track(uint32_t uiTrackID) const
{
  return !(m_vHvc2TrackIDs.end()==std::find(m_vHvc2TrackIDs.begin(),m_vHvc2TrackIDs.end(),uiTrackID));
}

void HEVCExtractorReader::replaceLFwithSC(std::vector<char>& rvSample) const
{
  if(4 > rvSample.size()) return;
  uint32_t uiPointer = 0;
  do
  {
    uint32_t uiSize = getLengthField(rvSample, uiPointer);
    setLengthField(rvSample, uiPointer, 1);
    uiPointer += uiSize+4;
  }while(uiPointer<rvSample.size());
}

uint32_t HEVCExtractorReader::getLengthField(std::vector<char>& rvSample, uint32_t uiPointer) const
{
  if(uiPointer+4 > rvSample.size()) return 0;
  return ((rvSample[uiPointer]&0xff)   << 24) |
         ((rvSample[uiPointer+1]&0xff) << 16) |
         ((rvSample[uiPointer+2]&0xff) << 8)  |
      ( rvSample[uiPointer+3]&0xff);
}

void HEVCExtractorReader::setLengthField(std::vector<char>& rvSample, uint32_t uiPointer, uint32_t uiSize) const
{
  if(uiPointer+4 > rvSample.size()) return;
  rvSample[uiPointer]   = (uiSize>>24)&0xff;
  rvSample[uiPointer+1] = (uiSize>>16)&0xff;
  rvSample[uiPointer+2] = (uiSize>>8)&0xff;
  rvSample[uiPointer+3] = (uiSize)&0xff;
}

void HEVCExtractorReader::skipNotUsedAUs(TrackIDSet& rUsedTrackIds)
{
  // just to be sure that we dont skip multiple samples from the same track
  std::set<uint32_t> uniqueUsedIDs, uniqueNotUsedIDs;
  for(TrackIDSet::iterator it=rUsedTrackIds.begin(); it!=rUsedTrackIds.end(); it++)
  {
    uniqueUsedIDs.insert(*it);
  }

  // get all unique not used trackIDs
  for(TrackIDSet::iterator it=m_vHvc1TrackIDs.begin(); it!=m_vHvc1TrackIDs.end(); it++)
  { // hvc1 tracks
    if(uniqueUsedIDs.find(*it) == uniqueUsedIDs.end()) { uniqueNotUsedIDs.insert(*it); }
  }
  for(TrackIDSet::iterator it=m_vHvc2TrackIDs.begin(); it!=m_vHvc2TrackIDs.end(); it++)
  { // hvc2 tracks
    if(uniqueUsedIDs.find(*it) == uniqueUsedIDs.end()) { uniqueNotUsedIDs.insert(*it); }
  }

  // skip AUs
  ISOErr err;
  ISOHandle sampleSkip;
  ISONewHandle(0, &sampleSkip);
  uint32_t uiSize = 0;
  for(std::set<uint32_t>::iterator it=uniqueNotUsedIDs.begin(); it!=uniqueNotUsedIDs.end(); it++)
  {
    err = getNextAU(*it, sampleSkip, &uiSize);
    if(err) {assert(0); continue; }
  }
  ISODisposeHandle(sampleSkip);
}
