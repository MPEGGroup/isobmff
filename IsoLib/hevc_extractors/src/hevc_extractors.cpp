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
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <fstream>

#include "HEVCExtractorReader.h"

class SimpleCmdOptionParser
{
  typedef std::vector<std::string> OptList;
public:
  SimpleCmdOptionParser(int32_t argc, char** argv)
  {
    for(int32_t i=1; i<argc; ++i) m_strOptions.push_back(std::string(argv[i]));
  }
  bool isSet(std::string strOpt)
  {
    return std::find(m_strOptions.begin(), m_strOptions.end(), strOpt) != m_strOptions.end();
  }
  std::string getOption(std::string strOpt, std::string strDefault=std::string())
  {
    std::string strRet;
    if(isSet(strOpt))
    {
      OptList::iterator it = std::find(m_strOptions.begin(), m_strOptions.end(), strOpt); it++;
      if(it != m_strOptions.end()) { if((*it)[0] != '-') { strRet = *it; } }
    }
    if(strRet.empty() && !strDefault.empty()) { strRet = strDefault; }
    return strRet;
  }
private:
  OptList m_strOptions;
};


int main(int argc, char** argv)
{
  // options
  SimpleCmdOptionParser cOptions(argc, argv);
  bool        bListTrackIDs     = false;
  uint32_t    uiSelectedTrackID = 0;
  std::string strInputFile;
  std::string strOutputFile     = "out.265";

  if(cOptions.isSet("-h"))
  {
    std::cout << "Extractors reference software version 0.1, Input contribution m41627 for 120th MPEG meeting\n"
                 "  -h              this help text\n"
                 "  -i inputFile    input ISOBMFF file (OMAF compliant)\n"
                 "  -o outputFile   output HEVC bitstream file (default: out.265)\n"
                 "  -l              list all tarckIDs\n"
                 "  -t trackID      select a trackID from which the bitstream is extracted\n" << std::endl;
    return 0;
  }
  strInputFile  = cOptions.getOption("-i");
  strOutputFile = cOptions.getOption("-o", strOutputFile);
  bListTrackIDs = cOptions.isSet("-l");

  std::string strTrackID = cOptions.getOption("-t");
  uiSelectedTrackID = atoi(strTrackID.c_str());

  // cmd options checks
  if(strInputFile.empty())
  {
    std::cerr << "No input file specified." << std::endl;
    return -1;
  }
  if(!bListTrackIDs && (strOutputFile.empty() || uiSelectedTrackID==0))
  {
    std::cerr << "Check command line parameters. Output file and trackID should be valid" << std::endl;
    return -1;
  }

  HEVCExtractorReader cExtractor;
  ISOErr err;
  err = cExtractor.init(strInputFile, true);
  if(err)
  {
    std::cerr << "could not initialize extractor player: err=" << err << std::endl;
    return -1;
  }

  // -l option is set: list all trackIDs and original formats
  if(bListTrackIDs)
  {
    std::cout << "list all trackIDs:\n";
    std::vector<uint32_t> vHvc1TrackIds = cExtractor.getHvc1TrackIDs();
    std::vector<uint32_t> vHvc2TrackIds = cExtractor.getHvc2TrackIDs();
    if(0<vHvc1TrackIds.size())
    {
      std::cout << "'hvc1' trackIDs:" << std::endl;
      for(uint32_t i=0; i<vHvc1TrackIds.size();++i)
      {
        std::cout << "                 " << vHvc1TrackIds[i] << std::endl;
      }
    }
    if(0<vHvc2TrackIds.size())
    {
      std::cout << "'hvc2' trackIDs:" << std::endl;
      for(uint32_t i=0; i<vHvc2TrackIds.size();++i)
      {
        std::cout << "                 " << vHvc2TrackIds[i];
        std::cout << "  depend on: " << cExtractor.getAllDependencies(vHvc2TrackIds[i]) << std::endl;
      }
    }
    return 0;
  }

  if(!cExtractor.isHvc2Track(uiSelectedTrackID))
  {
    std::cout << "selected trackID=" << uiSelectedTrackID << " is not hvc2" << std::endl;
  }
  cExtractor.setSelectedTrackID(uiSelectedTrackID);

  std::ofstream cBitstream(strOutputFile.c_str(), std::ios_base::binary);

  // get ParameterSets
  std::vector<char>  vParamSets;
  cExtractor.getPSwithStartCodesFromTrack(uiSelectedTrackID, vParamSets);
  cBitstream.write(vParamSets.data(), vParamSets.size());

  std::vector<char> vSample;
  do
  {
    vSample = cExtractor.getNextAUResolveExtractors();
    if(vSample.size()==0)
    { // try to get an AU from hvc1 track
      vSample = cExtractor.getNextAUwithoutExtractors();
    }
    if(vSample.size()>0)
    {
      cExtractor.replaceLFwithSC(vSample);
      cBitstream.write(vSample.data(), vSample.size());
    }
  }while(vSample.size()>0 );

  cBitstream.close();
  return 0;
}
