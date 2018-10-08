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

#include "ISOMovies.h"
#include <stdint.h>
#include <vector>
#include <map>
#include <string>

class HEVCExtractorReader
{
public:
////////////////////////////////// TYPES //////////////////////////////////
  typedef std::vector<uint32_t>               TrackIDSet;
  typedef std::map<uint32_t, TrackIDSet>      TrackIDSetMap;      // hvc2_trackID | set(hvc1_trackIDs)
  typedef std::map<uint32_t, ISOTrackReader*> TrackReaderPtrMap;  // trackID | Reader
  typedef std::map<uint32_t, uint32_t>        LenSizeMinOneMap;   // trackID | lengthsize-1

  ////////////////////////////////// METHODS //////////////////////////////////
  ///
  /// \brief getPSwithStartCodesFromTrack returns the parameter sets of selected track
  /// \param uiTrackID which track to select
  /// \param rvOut output vector
  /// \return 0 on success
  ///
  int32_t           getPSwithStartCodesFromTrack(uint32_t uiTrackID,
                                                 std::vector<char>& rvOut)  const;
  ///
  /// \brief getOriginalFormat returns the original_format string
  /// \param uiTrackID which track to select
  /// \return original_format string
  ///
  std::string       getOriginalFormat(uint32_t uiTrackID)             const;
  ///
  /// \brief getHvc1TrackIDs return all 'hvc1' trackIDs
  /// \return vector off all 'hvc1' trackIDs
  ///
  TrackIDSet        getHvc1TrackIDs()                                 const;
  ///
  /// \brief getHvc2TrackIDs return all 'hvc2' trackIDs
  /// \return off all 'hvc2' trackIDs
  ///
  TrackIDSet        getHvc2TrackIDs()                                 const;
  ///
  /// \brief getCurrentSampleNr
  /// \param uiTrackID which track to select
  /// \return current sample number
  ///
  uint32_t          getCurrentSampleNr(uint32_t uiTrackID)            const;
  ///
  /// \brief getNextSampleNr
  /// \param uiTrackID which track to select
  /// \return next sample number
  ///
  uint32_t          getNextSampleNr(uint32_t uiTrackID)               const;
  ///
  /// \brief getNextSample return next AU (automatically select if extractors needs to be resolved or not)
  /// \return
  ///
  std::vector<char> getNextSample();
  ///
  /// \brief getNextAUwithoutExtractors return next AU without resolving extractors
  /// \return data
  ///
  std::vector<char> getNextAUwithoutExtractors();
  ///
  /// \brief getNextAUResolveExtractors return next AU while resolving extractors
  /// \return data
  ///
  std::vector<char> getNextAUResolveExtractors();
  ///
  /// \brief getAllDependencies
  /// \param uiTrackID which track to select
  /// \return string containing all referenced trackIDs
  ///
  std::string       getAllDependencies(uint32_t uiTrackID)            const;

  ///
  /// \brief setNextSampleNr
  /// \param uiTrackID which track to select
  /// \param uiNextSampleNr next sample number
  ///
  void              setNextSampleNr(uint32_t uiTrackID, uint32_t uiNextSampleNr);
  ///
  /// \brief setSelectedTrackID selects a track
  /// \param uiTrackID which track to select
  ///
  void              setSelectedTrackID(uint32_t uiTrackID);
  ///
  /// \brief init initialization
  /// \param strFileName OMAF compliant ISOBMFF file
  /// \param bForce if set, try to initialize non OMAF compliant files as well
  /// \return 0 on success
  ///
  int32_t           init(std::string strFileName, bool bForce=false);
  ///
  /// \brief reInit flush all trackReaders and reinitialize
  /// \return 0 on success
  ///
  int32_t           reInit();
  ///
  /// \brief isHvc2Track
  /// \param uiTrackID which track to select
  /// \return true if the selected track is 'hvc2'
  ///
  bool              isHvc2Track(uint32_t uiTrackID) const;
  ///
  /// \brief replaceLFwithSC replaces all length fields LFs with start codes 0x00 0x00 0x00 0x01
  /// \param rvSample sample with data where LFs should be replaced
  ///
  void              replaceLFwithSC(std::vector<char>& rvSample) const;

  ///
  /// \brief setDefaultLenSizeMinOne this function is a temporary bugfix, this information should be parsed from the file
  /// \param uiDefaultVal default value for all tracks in the file. if set to 0 parsing from the config record is forced
  ///
  void setDefaultLenSizeMinOne(uint32_t uiDefaultVal) { m_uiDefLenSizeMinOne = uiDefaultVal; }

private:
  int32_t           getPSfromSampleEntry(ISOHandle sampleEntryH,
                                         ISOHandle vpsH,
                                         ISOHandle spsH,
                                         ISOHandle ppsH)              const;
  int32_t           getLengthSizeMinusOneFlag(uint32_t uiTrackID,
                                              uint32_t& rUiFlag)      const;
  TrackIDSet        getRefTrackIDs(ISOTrack trak)                     const;
  ISOTrackReader*   getTrackReader(uint32_t uiTrackID)                const;
  int32_t           getNextAU(uint32_t uiTrackID, ISOHandle sampleH, uint32_t* pUiSize);
  uint32_t          getLengthField(std::vector<char>& rvSample, uint32_t uiPointer) const;
  void              setLengthField(std::vector<char>& rvSample, uint32_t uiPointer, uint32_t uiSize) const;

  // this is a hack function since we don't have seeking implemented now
  // TODO: implement seeking and remove this method
  void              skipNotUsedAUs(TrackIDSet& rUsedTrackIds);

////////////////////////////////// MEMBERS //////////////////////////////////
private:
  bool              m_bInitialized;     // initialized flag
  ISOMovie          m_cMovieBox;        // Movie Box
  TrackIDSet        m_vHvc1TrackIDs;    // trackIDs of all 'hvc1' tracks
  TrackIDSet        m_vHvc2TrackIDs;    // trackIDs of all 'hvc2' tracks
  TrackIDSetMap     m_mHvc2Dependencies;// all dependencies of each 'hvc2' track
  uint32_t          m_uiSelectedTrackID;// trackID of selected track
  TrackReaderPtrMap m_mTrackReaders;    // one track reader per track
  LenSizeMinOneMap  m_mLenSizeMinOne;   // lenght_size_minus_one for each track
  uint32_t          m_uiDefLenSizeMinOne; // default lenght_size_minus_one value todo: remove it after bugfix in libisomendiafile
};
