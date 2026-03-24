#include "SMPTE_ST2094_50.hpp"

#include "SMPTE_ST2094_50.hpp"
#include <cstdarg>
#include <cstdio>
#include <bitset>
#include <numeric>

/* *********************************** LOCAL LOGGING FUNCTIONS *******************************************************************************************/

// Local log level constants
#define LOGLEVEL_OFF 0
#define LOGLEVEL_ERROR 1
#define LOGLEVEL_WARNING 2
#define LOGLEVEL_INFO 3
#define LOGLEVEL_DEBUG 4
#define LOGLEVEL_TRACE 5

// Global verbose level for this compilation unit
static int g_verboseLevel = LOGLEVEL_TRACE;

// Local logging function - concatenates format and outputs to stdout
static void logMsg(int logLvl, const char* format, ...) {
    // Skip if log level is higher than current verbose level
    if (logLvl > g_verboseLevel) {
        return;
    }
    
    // Print log level prefix
    switch (logLvl) {
        case LOGLEVEL_ERROR:    std::printf("Error: ");   break;
        case LOGLEVEL_WARNING:  std::printf("Warning: "); break;
        case LOGLEVEL_INFO:     std::printf("Info: ");    break;
        case LOGLEVEL_DEBUG:    std::printf("Debug: ");   break;
        case LOGLEVEL_TRACE:    std::printf("Trace: ");   break;
    }
    
    // Print formatted message
    va_list args;
    va_start(args, format);
    std::vprintf(format, args);
    va_end(args);
    
    // Print newline
    std::printf("\n");
}

/* *********************************** UTILITY FUNCTIONS *******************************************************************************************/

// Convert uint8 to heaxadecimal value
std::string uint8_to_hex(uint8_t value) {
  std::stringstream ss;
  ss << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(value);
  return ss.str();
}

// Formatted cout of a name and a value aligned for binary encoding and decoding debugging
void printDebug(const std::string& varName, uint16_t varValue, uint8_t nbBits, int verboseLevel) {
  if (verboseLevel < LOGLEVEL_TRACE) {
    return; // Only print debug info at TRACE level
  }
  
  // Explicitly set decimal output format
  std::cout << std::dec;
  
  std::cout.width(50); std::cout << varName << "=";
  std::cout.width(6); std::cout  << varValue << " | ";
  switch (nbBits) { // bitset need constant
    case 1:
      std::cout.width(16); std::cout <<  std::bitset<1>(varValue).to_string() << "\n";
      break;
    case 2:
      std::cout.width(16); std::cout <<  std::bitset<2>(varValue).to_string() << "\n";
      break;
    case 3:
      std::cout.width(16); std::cout <<  std::bitset<3>(varValue).to_string() << "\n";
      break;
    case 4:
      std::cout.width(16); std::cout <<  std::bitset<4>(varValue).to_string() << "\n";
      break;
    case 5:
      std::cout.width(16); std::cout <<  std::bitset<5>(varValue).to_string() << "\n";
      break;
    case 6:
      std::cout.width(16); std::cout <<  std::bitset<6>(varValue).to_string() << "\n";
      break;
    case 7:
      std::cout.width(16); std::cout <<  std::bitset<7>(varValue).to_string() << "\n";
      break;
    case 8:
      std::cout.width(16); std::cout <<  std::bitset<8>(varValue).to_string() << "\n";
      break;
    case 16:
      std::cout.width(16); std::cout << std::bitset<16>(varValue).to_string() << "\n";
      break;
  default:
      break;
  }
}


// Print binary data compatible with external verification tool
void printBinaryData(std::vector<uint8_t> binary_data, int verboseLevel) {
    if (verboseLevel < LOGLEVEL_TRACE) {
        return; // Only print debug info at TRACE level
    }  
    
    std::cout << "Binary data decoded, paste in external tool -> https://ccameron-chromium.github.io/agtm-demo/parse.html" << std::endl;
    const int bytesPerRow = 16; // Define how many bytes per row you want
    for (int i = 0; i < int(binary_data.size()); i++) {
        // Print hexadecimal value with 0x prefix
        std::cout << "0x" << std::noshowbase << std::hex << static_cast<int>(binary_data[i]) << ", ";
        // New line after every 'bytesPerRow' bytes
        if ((i + 1) % bytesPerRow == 0) {
            std::cout << std::endl;
        }
    }
    // Print a newline if the last row isn't complete
    if (int(binary_data.size()) % bytesPerRow != 0) {
        std::cout << std::endl;
    }
}
  

void push_boolean(struct BinaryData *payloadBinaryData, bool boolValue, const std::string& varName, int verboseLevel){
    uint8_t decValue = static_cast<uint8_t>(boolValue) ;
    payloadBinaryData->payload[payloadBinaryData->byteIdx] = payloadBinaryData->payload[payloadBinaryData->byteIdx] + (decValue << (7 - payloadBinaryData->bitIdx));
    printDebug(varName, decValue,  1, verboseLevel);

    payloadBinaryData->bitIdx++;
    if (payloadBinaryData->bitIdx == uint8_t(8)){
        payloadBinaryData->bitIdx = 0;
        payloadBinaryData->payload.push_back(0); payloadBinaryData->byteIdx++;
    } else if (payloadBinaryData->bitIdx > 8) {
        logMsg(LOGLEVEL_ERROR, "push_boolean exceeded a byte for %s", varName.c_str());
    }
}

void push_bits(struct BinaryData *payloadBinaryData, uint8_t value, uint8_t nbBits, const std::string& varName, int verboseLevel){
    payloadBinaryData->payload[payloadBinaryData->byteIdx] = payloadBinaryData->payload[payloadBinaryData->byteIdx] + (value << ( 8 - nbBits - payloadBinaryData->bitIdx));
    printDebug(varName, value,  nbBits, verboseLevel);
    payloadBinaryData->bitIdx += nbBits;
    if ( payloadBinaryData->bitIdx == 8){
        payloadBinaryData->byteIdx++;
        payloadBinaryData->bitIdx = 0;
        payloadBinaryData->payload.push_back(0);
    } else if ( payloadBinaryData->bitIdx > 8) {
        logMsg(LOGLEVEL_ERROR, "push_bits exceeded a byte for %s while trying to add %d bits", varName.c_str(), nbBits);
    }
}

void push_8bits(struct BinaryData *payloadBinaryData, uint16_t value, const std::string& varName, int verboseLevel){
    // Verify that we are at the start of a byte
    if (payloadBinaryData->bitIdx != 0){
        logMsg(LOGLEVEL_ERROR, "push_8bits called but we are not at the start of a byte");
    } else {
        payloadBinaryData->payload[payloadBinaryData->byteIdx] = uint8_t(value & 0x00FF);
        payloadBinaryData->payload.push_back(0); payloadBinaryData->byteIdx++; 
        printDebug(varName, value,  8, verboseLevel);
    }
}

void push_16bits(struct BinaryData *payloadBinaryData, uint16_t value, const std::string& varName, int verboseLevel){
    // Verify that we are at the start of a byte
    if (payloadBinaryData->bitIdx != 0){
        logMsg(LOGLEVEL_ERROR, "push_16bits called but we are not at the start of a byte");
    } else {
        payloadBinaryData->payload[payloadBinaryData->byteIdx] = uint8_t((value >> 8) & 0x00FF);
        payloadBinaryData->payload.push_back(0); payloadBinaryData->byteIdx++; 
        payloadBinaryData->payload[payloadBinaryData->byteIdx] = uint8_t((value     ) & 0x00FF);
        payloadBinaryData->payload.push_back(0); payloadBinaryData->byteIdx++; 
        printDebug(varName, value,  16, verboseLevel);
    }
}

bool pull_boolean(struct BinaryData *payloadBinaryData, const std::string& varName, int verboseLevel){
    uint8_t decValue = (payloadBinaryData->payload[payloadBinaryData->byteIdx] >> (7 - payloadBinaryData->bitIdx)) & 0x01 ;
    bool result = static_cast<bool>(decValue) ;
    payloadBinaryData->bitIdx++;
    if (payloadBinaryData->bitIdx == uint8_t(8)){        
        payloadBinaryData->byteIdx++;
        payloadBinaryData->bitIdx = 0;
    } else if (payloadBinaryData->bitIdx > 8) {
        logMsg(LOGLEVEL_ERROR, "pull_boolean exceeded a byte for %s", varName.c_str());
    }
    printDebug(varName, decValue,  1, verboseLevel);
    return result;
}

uint16_t pull_bits(struct BinaryData *payloadBinaryData, uint8_t nbBits, const std::string& varName, int verboseLevel){
    uint8_t decValue = uint8_t(payloadBinaryData->payload[payloadBinaryData->byteIdx] << payloadBinaryData->bitIdx) >> ((8 - nbBits)); 
    payloadBinaryData->bitIdx += nbBits;
    if ( payloadBinaryData->bitIdx == 8){
        payloadBinaryData->byteIdx++;
        payloadBinaryData->bitIdx = 0;
    } else if ( payloadBinaryData->bitIdx > 8) {
        logMsg(LOGLEVEL_ERROR, "pull_bits exceeded a byte for %s while trying to add %d bits", varName.c_str(), nbBits);
    }
    printDebug(varName, decValue,  nbBits, verboseLevel);
    return uint16_t(decValue);
}

uint16_t pull_8bits(struct BinaryData *payloadBinaryData, const std::string& varName, int verboseLevel){
    // Verify that we are at the start of a byte
    uint16_t decValue = 404;
    if (payloadBinaryData->bitIdx != 0){
        logMsg(LOGLEVEL_ERROR, "pull_8bits called but we are not at the start of a byte");
    } else {
        decValue = uint16_t(payloadBinaryData->payload[payloadBinaryData->byteIdx]);
    }
    printDebug(varName, decValue,  8, verboseLevel);
    payloadBinaryData->byteIdx++;
    return decValue;
}

uint16_t pull_16bits(struct BinaryData *payloadBinaryData, const std::string& varName, int verboseLevel){
    // Verify that we are at the start of a byte
    uint16_t decValue = 404;
    if (payloadBinaryData->bitIdx != 0){
        logMsg(LOGLEVEL_ERROR, "pull_16bits called but we are not at the start of a byte");
    } else {
        decValue = uint16_t(payloadBinaryData->payload[payloadBinaryData->byteIdx]) << 8; payloadBinaryData->byteIdx++;
        decValue = decValue + uint16_t(payloadBinaryData->payload[payloadBinaryData->byteIdx]); payloadBinaryData->byteIdx++;
        printDebug(varName, decValue,  16, verboseLevel);
    }
    return decValue;
}

/* *********************************** SMPTE ST 2094-50 FUNCTIONS *******************************************************************************************/
// Constructors
SMPTE_ST2094_50::SMPTE_ST2094_50(){
    keyValue = "B500900001:SMPTE-ST2094-50";

    // Application - fixed
    applicationIdentifier = 5;
    applicationVersion    = 255;

    // ProcessingWindow - fixed
    pWin.upperLeftCorner  = 0;
    pWin.lowerRightCorner = 0;
    pWin.windowNumber     = 1;

    // Initialize convenience flags
    isHeadroomAdaptiveToneMap = false;
    isReferenceWhiteToneMapping = false;
    for (uint16_t iAlt = 0; iAlt < MAX_NB_ALTERNATE; iAlt++) {
        hasSlopeParameter[iAlt] = false;
    }
    
    // Initialize verbose level to INFO (default)
    verboseLevel = LOGLEVEL_TRACE;
    g_verboseLevel = LOGLEVEL_TRACE;  // Also set the global
}

// Getters
std::vector<uint8_t>    SMPTE_ST2094_50::getPayloadData(){return payloadBinaryData.payload;}
uint32_t                SMPTE_ST2094_50::getTimeIntervalStart(){return timeI.timeIntervalStart;}
uint32_t                SMPTE_ST2094_50::getTimeintervalDuration(){return timeI.timeintervalDuration;}

// Setters
void                    SMPTE_ST2094_50::setTimeIntervalStart(uint32_t frame_start){timeI.timeIntervalStart = frame_start;}
void                    SMPTE_ST2094_50::setTimeintervalDuration(uint32_t frame_duration){timeI.timeintervalDuration = frame_duration;}
void                    SMPTE_ST2094_50::setVerboseLevel(int level){
    if (level >= LOGLEVEL_OFF && level <= LOGLEVEL_TRACE) {
        verboseLevel = level;
        g_verboseLevel = level;  // Update the global for logMsg
    } else {
        logMsg(LOGLEVEL_WARNING, "Invalid verbose level %d, keeping current level %d", level, verboseLevel);
    }
}

/* *********************************** ENCODING SECTION ********************************************************************************************/
// Read from json file the metadata items
/* *********************************** ENCODING SECTION ********************************************************************************************/
// Read from json file the metadata items
bool SMPTE_ST2094_50::decodeJsonToMetadataItems(nlohmann::json j) {
    logMsg(LOGLEVEL_DEBUG, "DECODE JSON TO METADATA ITEMS");

    if (j.is_null() || !j.is_object()) {
        logMsg(LOGLEVEL_ERROR, "Invalid JSON dictionary");
        return true;
    }

    // Check if there's a top-level SMPTEST2094_50 wrapper
    nlohmann::json rootDict = j;
    if (j.contains("SMPTEST2094_50")) {
        rootDict = j["SMPTEST2094_50"];
        if (!rootDict.is_object()) {
            logMsg(LOGLEVEL_ERROR, "SMPTEST2094_50 is not a dictionary");
            return true;
        }
    } else {logMsg(LOGLEVEL_ERROR, "SMPTEST2094_50 not found in json file");}

    
    
    // Parse top-level fields
    if (rootDict.contains("frameStart")) {
        timeI.timeIntervalStart = rootDict["frameStart"].get<uint32_t>();
    }
    
    if (rootDict.contains("frameDuration")) {
        timeI.timeintervalDuration = rootDict["frameDuration"].get<uint32_t>();
    }
    
    if (rootDict.contains("windowNumber")) {
        pWin.windowNumber = rootDict["windowNumber"].get<uint32_t>();
    }

    // Extract ColorVolumeTransform dictionary
    if (!rootDict.contains("ColorVolumeTransform") || !rootDict["ColorVolumeTransform"].is_object()) {
        logMsg(LOGLEVEL_ERROR, "ColorVolumeTransform dictionary missing or invalid");
        return true;
    }
    
    nlohmann::json cvtDict = rootDict["ColorVolumeTransform"];

    // Parse hdrReferenceWhite (required field)
    if (!cvtDict.contains("hdrReferenceWhite")) {
        logMsg(LOGLEVEL_ERROR, "hdrReferenceWhite metadata item missing");
        return true;
    }
    
    try {
        cvt.hdrReferenceWhite = cvtDict["hdrReferenceWhite"].get<float>();
    } catch (const std::exception& e) {
        logMsg(LOGLEVEL_ERROR, "Failed to parse 'hdrReferenceWhite': %s", e.what());
        return true;
    }

    // Extract HeadroomAdaptiveToneMapping dictionary (optional)
    if (!cvtDict.contains("HeadroomAdaptiveToneMapping") || !cvtDict["HeadroomAdaptiveToneMapping"].is_object()) {
        // No HATM - this is reference white tone mapping only
        isHeadroomAdaptiveToneMap = false;
        isReferenceWhiteToneMapping = false;
        return false;
    }

    nlohmann::json hatmDict = cvtDict["HeadroomAdaptiveToneMapping"];
    
    // HATM is present
    isHeadroomAdaptiveToneMap = true;

    // Parse baselineHdrHeadroom
    if (hatmDict.contains("baselineHdrHeadroom")) {
        cvt.hatm.baselineHdrHeadroom = hatmDict["baselineHdrHeadroom"].get<float>();
    }

    // Parse numAlternateImages
    if (!hatmDict.contains("numAlternateImages")) {
        // If numAlternateImages is not present, assume reference white tone mapping
        isReferenceWhiteToneMapping = true;
        return false;
    }
    
    cvt.hatm.numAlternateImages = hatmDict["numAlternateImages"].get<uint32_t>();
    isReferenceWhiteToneMapping = false;

    // Parse gainApplicationSpaceChromaticities (optional)
    if (hatmDict.contains("gainApplicationSpaceChromaticities")) {
        std::vector<float> gainAppSpaceChrom = hatmDict["gainApplicationSpaceChromaticities"].get<std::vector<float>>();
        if (gainAppSpaceChrom.size() != MAX_NB_CHROMATICITIES) {
            logMsg(LOGLEVEL_ERROR, "gainApplicationSpaceChromaticities array size (%zu) != %d", gainAppSpaceChrom.size(), MAX_NB_CHROMATICITIES);
            return true;
        }
        for (int iCh = 0; iCh < MAX_NB_CHROMATICITIES; iCh++) {
            cvt.hatm.gainApplicationSpaceChromaticities[iCh] = gainAppSpaceChrom[iCh];
        }
    }

    if (cvt.hatm.numAlternateImages >= 1) {
        // Parse alternateHdrHeadroom array
        if (hatmDict.contains("alternateHdrHeadroom")) {
            nlohmann::json alternateHdrHeadroomValue = hatmDict["alternateHdrHeadroom"];
            std::vector<float> alternateHdrHeadroomArray;
            
            // Convert to array if it's a single value
            if (alternateHdrHeadroomValue.is_number()) {
                // Single value - wrap it in an array
                alternateHdrHeadroomArray.push_back(alternateHdrHeadroomValue.get<float>());
                logMsg(LOGLEVEL_DEBUG, "alternateHdrHeadroom is a single value, converted to array");
            } else if (alternateHdrHeadroomValue.is_array()) {
                alternateHdrHeadroomArray = alternateHdrHeadroomValue.get<std::vector<float>>();
            } else {
                logMsg(LOGLEVEL_ERROR, "alternateHdrHeadroom is neither an array nor a number, skipping");
                return true;

            }
            
            if (!alternateHdrHeadroomArray.empty()) {
                // Validate array count matches numAlternateImages
                if (alternateHdrHeadroomArray.size() != cvt.hatm.numAlternateImages) {
                    logMsg(LOGLEVEL_ERROR, "alternateHdrHeadroom array count (%zu) does not match numAlternateImages (%u)",
                           alternateHdrHeadroomArray.size(), cvt.hatm.numAlternateImages);
                    return true;
                }

                // Populate alternateHdrHeadroom values
                for (uint32_t i = 0; i < cvt.hatm.numAlternateImages; i++) {
                    cvt.hatm.alternateHdrHeadroom.push_back(alternateHdrHeadroomArray[i]);
                }
            }
        }

        // Parse ColorGainFunction array
        if (!hatmDict.contains("ColorGainFunction")) {
            logMsg(LOGLEVEL_ERROR, "ColorGainFunction missing");
            return true;
        }

        nlohmann::json colorGainFunctionValue = hatmDict["ColorGainFunction"];
        std::vector<nlohmann::json> colorGainFunctionArray;
        
        // Convert to array if it's a single dictionary
        if (colorGainFunctionValue.is_object()) {
            // Single ColorGainFunction dictionary - wrap it in an array
            colorGainFunctionArray.push_back(colorGainFunctionValue);
            logMsg(LOGLEVEL_DEBUG, "ColorGainFunction is a single dictionary, converted to array");
        } else if (colorGainFunctionValue.is_array()) {
            colorGainFunctionArray = colorGainFunctionValue.get<std::vector<nlohmann::json>>();
        } else {
            logMsg(LOGLEVEL_ERROR, "ColorGainFunction is neither an array nor a dictionary");
            return true;
        }

        // Validate array count matches numAlternateImages
        if (colorGainFunctionArray.size() != cvt.hatm.numAlternateImages) {
            logMsg(LOGLEVEL_ERROR, "ColorGainFunction array count (%zu) does not match numAlternateImages (%u)",
                   colorGainFunctionArray.size(), cvt.hatm.numAlternateImages);
            return true;
        }

        // Parse each ColorGainFunction
        for (uint32_t i = 0; i < cvt.hatm.numAlternateImages; i++) {
            nlohmann::json cgfDict = colorGainFunctionArray[i];
            if (!cgfDict.is_object()) {
                logMsg(LOGLEVEL_ERROR, "ColorGainFunction[%u] is not a dictionary", i);
                return true;
            }

            ColorGainFunction cgf;

            // Parse ComponentMix
            if (!cgfDict.contains("ComponentMix") || !cgfDict["ComponentMix"].is_object()) {
                logMsg(LOGLEVEL_ERROR, "ComponentMix dictionary missing for ColorGainFunction[%u]", i);
                return true;
            }

            nlohmann::json componentMixDict = cgfDict["ComponentMix"];
            cgf.cm.componentMixRed = componentMixDict.value("componentMixRed", 0.0f);
            cgf.cm.componentMixGreen = componentMixDict.value("componentMixGreen", 0.0f);
            cgf.cm.componentMixBlue = componentMixDict.value("componentMixBlue", 0.0f);
            cgf.cm.componentMixMax = componentMixDict.value("componentMixMax", 0.0f);
            cgf.cm.componentMixMin = componentMixDict.value("componentMixMin", 0.0f);
            cgf.cm.componentMixComponent = componentMixDict.value("componentMixComponent", 0.0f);

            // Parse GainCurve
            if (!cgfDict.contains("GainCurve") || !cgfDict["GainCurve"].is_object()) {
                logMsg(LOGLEVEL_ERROR, "GainCurve dictionary missing for ColorGainFunction[%u]", i);
                return false;
            }

            nlohmann::json gainCurveDict = cgfDict["GainCurve"];

            // Parse gainCurveNumControlPoints
            if (!gainCurveDict.contains("gainCurveNumControlPoints")) {
                logMsg(LOGLEVEL_ERROR, "gainCurveNumControlPoints missing for ColorGainFunction[%u]", i);
                return false;
            }

            cgf.gc.gainCurveNumControlPoints = gainCurveDict["gainCurveNumControlPoints"].get<uint32_t>();

            // Helper lambda to handle single value or array
            auto parseControlPointArray = [&](const std::string& key, std::vector<float>& target) -> bool {
                if (!gainCurveDict.contains(key)) {
                    return false;
                }
                
                nlohmann::json value = gainCurveDict[key];
                if (value.is_array()) {
                    std::vector<float> arr = value.get<std::vector<float>>();
                    if (arr.size() != cgf.gc.gainCurveNumControlPoints) {
                        logMsg(LOGLEVEL_ERROR, "%s array count mismatch for ColorGainFunction[%u]", key.c_str(), i);
                        return false;
                    }
                    target = arr;
                } else if (value.is_number()) {
                    // Single value - replicate it
                    float singleValue = value.get<float>();
                    target.resize(cgf.gc.gainCurveNumControlPoints, singleValue);
                }
                return true;
            };

            // Parse X control points
            if (!parseControlPointArray("gainCurveControlPointX", cgf.gc.gainCurveControlPointX)) {
                logMsg(LOGLEVEL_ERROR, "Failed to parse gainCurveControlPointX for ColorGainFunction[%u]", i);
                return false;
            }

            // Parse Y control points
            if (!parseControlPointArray("gainCurveControlPointY", cgf.gc.gainCurveControlPointY)) {
                logMsg(LOGLEVEL_ERROR, "Failed to parse gainCurveControlPointY for ColorGainFunction[%u]", i);
                return true;
            }

            // Parse Slope M control points (optional)
            if (gainCurveDict.contains("gainCurveControlPointM")) {
                hasSlopeParameter[i] = true;
                if (!parseControlPointArray("gainCurveControlPointM", cgf.gc.gainCurveControlPointM)) {
                    logMsg(LOGLEVEL_WARNING, "Failed to parse gainCurveControlPointM for ColorGainFunction[%u], continuing without it", i);
                    hasSlopeParameter[i] = false;
                }
            } else {
                hasSlopeParameter[i] = false;
            }

            cvt.hatm.cgf.push_back(cgf);
        }
    }
    return false;
}

// Convert metadata items to syntax elements 
void SMPTE_ST2094_50::convertMetadataItemsToSyntaxElements(){
  elm.has_custom_hdr_reference_white_flag = false;
  elm.has_adaptive_tone_map_flag = false;
  if (std::abs(cvt.hdrReferenceWhite - 203.0) > (0.5f * Q_HDR_REFERENCE_WHITE)) {
    elm.has_custom_hdr_reference_white_flag = true;
    elm.hdr_reference_white = uint16_t(cvt.hdrReferenceWhite * Q_HDR_REFERENCE_WHITE);
  }
  if (isHeadroomAdaptiveToneMap){
    elm.has_adaptive_tone_map_flag = true;
    elm.use_reference_white_tone_mapping_flag = true;
    elm.baseline_hdr_headroom = uint16_t(cvt.hatm.baselineHdrHeadroom * Q_HDR_HEADROOM  + 0.5f);
    elm.use_reference_white_tone_mapping_flag = isReferenceWhiteToneMapping;
    if (!isReferenceWhiteToneMapping){
      elm.use_reference_white_tone_mapping_flag = false;
      elm.num_alternate_images = uint16_t(cvt.hatm.numAlternateImages);

      if (cvt.hatm.cgf.size() < cvt.hatm.numAlternateImages) {
          logMsg(LOGLEVEL_ERROR, "cgf array size (%zu) is less than numAlternateImages (%u). JSON data is incomplete or malformed.", cvt.hatm.cgf.size(), cvt.hatm.numAlternateImages);
          return;
      }

      // Check if the primary combination is known
      if (
          abs(cvt.hatm.gainApplicationSpaceChromaticities[0] - 0.64  ) <  P_GAIN_APPLICATION_SPACE_CHROMATICITY &&
          abs(cvt.hatm.gainApplicationSpaceChromaticities[1] - 0.33  ) <  P_GAIN_APPLICATION_SPACE_CHROMATICITY &&
          abs(cvt.hatm.gainApplicationSpaceChromaticities[2] - 0.30  ) <  P_GAIN_APPLICATION_SPACE_CHROMATICITY &&
          abs(cvt.hatm.gainApplicationSpaceChromaticities[3] - 0.60  ) <  P_GAIN_APPLICATION_SPACE_CHROMATICITY &&
          abs(cvt.hatm.gainApplicationSpaceChromaticities[4] - 0.15  ) <  P_GAIN_APPLICATION_SPACE_CHROMATICITY &&
          abs(cvt.hatm.gainApplicationSpaceChromaticities[5] - 0.06  ) <  P_GAIN_APPLICATION_SPACE_CHROMATICITY &&
          abs(cvt.hatm.gainApplicationSpaceChromaticities[6] - 0.3127) <  P_GAIN_APPLICATION_SPACE_CHROMATICITY &&
          abs(cvt.hatm.gainApplicationSpaceChromaticities[7] - 0.3290) <  P_GAIN_APPLICATION_SPACE_CHROMATICITY){
              elm.gain_application_space_chromaticities_mode = 0;
          }
      else if (
          abs(cvt.hatm.gainApplicationSpaceChromaticities[0] - 0.68  ) <  P_GAIN_APPLICATION_SPACE_CHROMATICITY &&
          abs(cvt.hatm.gainApplicationSpaceChromaticities[1] - 0.32  ) <  P_GAIN_APPLICATION_SPACE_CHROMATICITY &&
          abs(cvt.hatm.gainApplicationSpaceChromaticities[2] - 0.265 ) <  P_GAIN_APPLICATION_SPACE_CHROMATICITY &&
          abs(cvt.hatm.gainApplicationSpaceChromaticities[3] - 0.69  ) <  P_GAIN_APPLICATION_SPACE_CHROMATICITY &&
          abs(cvt.hatm.gainApplicationSpaceChromaticities[4] - 0.15  ) <  P_GAIN_APPLICATION_SPACE_CHROMATICITY &&
          abs(cvt.hatm.gainApplicationSpaceChromaticities[5] - 0.06  ) <  P_GAIN_APPLICATION_SPACE_CHROMATICITY &&
          abs(cvt.hatm.gainApplicationSpaceChromaticities[6] - 0.3127) <  P_GAIN_APPLICATION_SPACE_CHROMATICITY &&
          abs(cvt.hatm.gainApplicationSpaceChromaticities[7] - 0.3290) <  P_GAIN_APPLICATION_SPACE_CHROMATICITY ) {
                  elm.gain_application_space_chromaticities_mode = 1;
      } else if (
          abs(cvt.hatm.gainApplicationSpaceChromaticities[0] - 0.708 ) <  P_GAIN_APPLICATION_SPACE_CHROMATICITY &&
          abs(cvt.hatm.gainApplicationSpaceChromaticities[1] - 0.292 ) <  P_GAIN_APPLICATION_SPACE_CHROMATICITY &&
          abs(cvt.hatm.gainApplicationSpaceChromaticities[2] - 0.17  ) <  P_GAIN_APPLICATION_SPACE_CHROMATICITY &&
          abs(cvt.hatm.gainApplicationSpaceChromaticities[3] - 0.797 ) <  P_GAIN_APPLICATION_SPACE_CHROMATICITY &&
          abs(cvt.hatm.gainApplicationSpaceChromaticities[4] - 0.131 ) <  P_GAIN_APPLICATION_SPACE_CHROMATICITY &&
          abs(cvt.hatm.gainApplicationSpaceChromaticities[5] - 0.046 ) <  P_GAIN_APPLICATION_SPACE_CHROMATICITY &&
          abs(cvt.hatm.gainApplicationSpaceChromaticities[6] - 0.3127) <  P_GAIN_APPLICATION_SPACE_CHROMATICITY &&
          abs(cvt.hatm.gainApplicationSpaceChromaticities[7] - 0.3290) <  P_GAIN_APPLICATION_SPACE_CHROMATICITY){
                    elm.gain_application_space_chromaticities_mode = 2;
      } else {
          elm.gain_application_space_chromaticities_mode = 3;
          for (uint16_t iCh = 0; iCh < 8; iCh++) {
              elm.gain_application_space_chromaticities[iCh] = uint16_t(cvt.hatm.gainApplicationSpaceChromaticities[iCh]* Q_GAIN_APPLICATION_SPACE_CHROMATICITY + 0.5f);
          }
      }

      // Validate that we have the required data structures
      if (cvt.hatm.numAlternateImages == 0) {
          logMsg(LOGLEVEL_ERROR, "numAlternateImages is 0, skipping the rest of metadata items");
          return;
      }

      // Loop over alternate images
      elm.has_common_component_mix_params_flag = true; // Check if all component mixing uses the same parameters
      elm.has_common_curve_params_flag = true;  // Check if all alternate have the same number of control points, x position and interpolation
      for (uint16_t iAlt = 0; iAlt < cvt.hatm.numAlternateImages; iAlt++) {
        if (fabs(cvt.hatm.alternateHdrHeadroom[iAlt] - cvt.hatm.baselineHdrHeadroom) < P_HDR_HEADROOM){ logMsg(LOGLEVEL_ERROR, "alternateHdrHeadroom[%d] cannot be equal to baselineHdrHeadroom", iAlt); return;}
          elm.alternate_hdr_headrooms[iAlt] = uint16_t(cvt.hatm.alternateHdrHeadroom[iAlt] * Q_HDR_HEADROOM + 0.5f);

          // init coefficient to 0
          for (uint16_t iCmf = 0; iCmf < MAX_NB_COMPONENT_MIXING_COEFFICIENT; iCmf++){
              elm.component_mixing_coefficient[iAlt][iCmf] = uint16_t(0);
              elm.has_component_mixing_coefficient_flag[iAlt][iCmf] = false;
          }
          // Component mixing
          if (
              abs(cvt.hatm.cgf[iAlt].cm.componentMixRed      ) < P_COMPONENT_MIXING_COEFFICIENT &&
              abs(cvt.hatm.cgf[iAlt].cm.componentMixGreen    ) < P_COMPONENT_MIXING_COEFFICIENT &&
              abs(cvt.hatm.cgf[iAlt].cm.componentMixBlue     ) < P_COMPONENT_MIXING_COEFFICIENT &&
              abs(cvt.hatm.cgf[iAlt].cm.componentMixMax- 1.0 ) < P_COMPONENT_MIXING_COEFFICIENT &&
              abs(cvt.hatm.cgf[iAlt].cm.componentMixMin      ) < P_COMPONENT_MIXING_COEFFICIENT &&
              abs(cvt.hatm.cgf[iAlt].cm.componentMixComponent) < P_COMPONENT_MIXING_COEFFICIENT){
              elm.component_mixing_type[iAlt] = 0;
          } else if (
            abs(cvt.hatm.cgf[iAlt].cm.componentMixRed      ) < P_COMPONENT_MIXING_COEFFICIENT &&
            abs(cvt.hatm.cgf[iAlt].cm.componentMixGreen    ) < P_COMPONENT_MIXING_COEFFICIENT &&
            abs(cvt.hatm.cgf[iAlt].cm.componentMixBlue     ) < P_COMPONENT_MIXING_COEFFICIENT &&
            abs(cvt.hatm.cgf[iAlt].cm.componentMixMax      ) < P_COMPONENT_MIXING_COEFFICIENT &&
            abs(cvt.hatm.cgf[iAlt].cm.componentMixMin      ) < P_COMPONENT_MIXING_COEFFICIENT &&
            abs(cvt.hatm.cgf[iAlt].cm.componentMixComponent - 1.0 ) < P_COMPONENT_MIXING_COEFFICIENT){
              elm.component_mixing_type[iAlt] = 1;
          }  else if (
            abs(cvt.hatm.cgf[iAlt].cm.componentMixRed   - (1.0 / 6.0)) < P_COMPONENT_MIXING_COEFFICIENT &&
            abs(cvt.hatm.cgf[iAlt].cm.componentMixGreen - (1.0 / 6.0)) < P_COMPONENT_MIXING_COEFFICIENT &&
            abs(cvt.hatm.cgf[iAlt].cm.componentMixBlue  - (1.0 / 6.0)) < P_COMPONENT_MIXING_COEFFICIENT &&
            abs(cvt.hatm.cgf[iAlt].cm.componentMixMax   - (1.0 / 2.0)) < P_COMPONENT_MIXING_COEFFICIENT &&
            abs(cvt.hatm.cgf[iAlt].cm.componentMixMin      ) < P_COMPONENT_MIXING_COEFFICIENT &&
            abs(cvt.hatm.cgf[iAlt].cm.componentMixComponent) < P_COMPONENT_MIXING_COEFFICIENT){
              elm.component_mixing_type[iAlt] = 2;
          } else { // Send flag to true for each non-zero coefficient
              elm.component_mixing_type[iAlt] = 3;
              uint16_t iCmf = 0;
              elm.component_mixing_coefficient[iAlt][iCmf] = uint16_t(cvt.hatm.cgf[iAlt].cm.componentMixRed       * Q_COMPONENT_MIXING_COEFFICIENT + 0.5f); iCmf++;
              elm.component_mixing_coefficient[iAlt][iCmf] = uint16_t(cvt.hatm.cgf[iAlt].cm.componentMixGreen     * Q_COMPONENT_MIXING_COEFFICIENT + 0.5f); iCmf++;
              elm.component_mixing_coefficient[iAlt][iCmf] = uint16_t(cvt.hatm.cgf[iAlt].cm.componentMixBlue      * Q_COMPONENT_MIXING_COEFFICIENT + 0.5f); iCmf++;
              elm.component_mixing_coefficient[iAlt][iCmf] = uint16_t(cvt.hatm.cgf[iAlt].cm.componentMixMax       * Q_COMPONENT_MIXING_COEFFICIENT + 0.5f); iCmf++;
              elm.component_mixing_coefficient[iAlt][iCmf] = uint16_t(cvt.hatm.cgf[iAlt].cm.componentMixMin       * Q_COMPONENT_MIXING_COEFFICIENT + 0.5f); iCmf++;
              elm.component_mixing_coefficient[iAlt][iCmf] = uint16_t(cvt.hatm.cgf[iAlt].cm.componentMixComponent * Q_COMPONENT_MIXING_COEFFICIENT + 0.5f); iCmf++;
              uint16_t sumCoefficients = 0;
              for (iCmf = 0; iCmf < MAX_NB_COMPONENT_MIXING_COEFFICIENT; iCmf++){
                  if (elm.component_mixing_coefficient[iAlt][iCmf] > 0 && elm.component_mixing_coefficient[iAlt][iCmf] <= Q_COMPONENT_MIXING_COEFFICIENT){
                      elm.has_component_mixing_coefficient_flag[iAlt][iCmf] = true;
                  } else if (elm.component_mixing_coefficient[iAlt][iCmf] == 0) {
                      elm.has_component_mixing_coefficient_flag[iAlt][iCmf] = false;
                  } else {
                      logMsg(LOGLEVEL_ERROR, "component mixing coefficient for alternate %d color %d is greater than 1.0 (%d)", iAlt, iCmf, (float)elm.component_mixing_coefficient[iAlt][iCmf] / Q_COMPONENT_MIXING_COEFFICIENT );
                  }
                  // Check if same mode as  alternate 0 and same coefficient, if not, then not common
                  if (elm.component_mixing_type[0] == 3 && (elm.component_mixing_coefficient[0][iCmf] != elm.component_mixing_coefficient[iAlt][iCmf])) {
                    elm.has_common_component_mix_params_flag = false;
                  }
                  sumCoefficients = sumCoefficients + elm.component_mixing_coefficient[iAlt][iCmf];
              }
              if (sumCoefficients != Q_COMPONENT_MIXING_COEFFICIENT) { logMsg(LOGLEVEL_WARNING, "Sum component mixing coefficient for alternate %d is not equal to 1.0, they will be scaled to 1.0 at decoding", iAlt); }
          }
          if (elm.component_mixing_type[0] != elm.component_mixing_type[iAlt]) {
            elm.has_common_component_mix_params_flag = false;
          }

          // Create syntax elements for the gain curve function
          elm.gain_curve_num_control_points_minus_1[iAlt] = cvt.hatm.cgf[iAlt].gc.gainCurveNumControlPoints - 1;
          if (elm.gain_curve_num_control_points_minus_1[0] != elm.gain_curve_num_control_points_minus_1[iAlt]){
            elm.has_common_curve_params_flag = false;
          }

          for (uint16_t iCps = 0; iCps < cvt.hatm.cgf[iAlt].gc.gainCurveNumControlPoints; iCps++){
              elm.gain_curve_control_points_x[iAlt][iCps] = uint16_t(cvt.hatm.cgf[iAlt].gc.gainCurveControlPointX[iCps] * Q_GAIN_CURVE_CONTROL_POINT_X  + 0.5f);
              if (elm.gain_curve_control_points_x[0][iCps] != elm.gain_curve_control_points_x[iAlt][iCps]){
                elm.has_common_curve_params_flag = false;
              }

              elm.gain_curve_control_points_y[iAlt][iCps] = uint16_t( abs( cvt.hatm.cgf[iAlt].gc.gainCurveControlPointY[iCps] ) * Q_GAIN_CURVE_CONTROL_POINT_Y  + 0.5f);
          }
          elm.gain_curve_use_pchip_slope_flag[iAlt] = !hasSlopeParameter[iAlt];
          if (elm.gain_curve_use_pchip_slope_flag[0] != elm.gain_curve_use_pchip_slope_flag[iAlt]){
            elm.has_common_curve_params_flag = false;
          }
          if (!elm.gain_curve_use_pchip_slope_flag[iAlt]) {
              for (uint16_t iCps = 0; iCps < cvt.hatm.cgf[iAlt].gc.gainCurveNumControlPoints; iCps++) {
                float theta = atan(cvt.hatm.cgf[iAlt].gc.gainCurveControlPointM[iCps]) * 180.0f / M_PI;
                elm.gain_curve_control_points_theta[iAlt][iCps] = uint16_t(( theta + O_GAIN_CURVE_CONTROL_POINT_THETA) * Q_GAIN_CURVE_CONTROL_POINT_THETA  + 0.5f);
              }
          }
      }
    }
  }
  }

// Convert syntax element to finary data and write to file
void SMPTE_ST2094_50::writeSyntaxElementsToBinaryData(){
// ================================================= Convert binary data from Syntax Elements ===================================
// Initialize the binary payload structure
payloadBinaryData.byteIdx = 0;
payloadBinaryData.bitIdx  = 0;
payloadBinaryData.payload.push_back(0);

logMsg(LOGLEVEL_DEBUG, "Start SMPTE_ST2094_50::writeSyntaxElementsToBinaryData");
push_bits(&payloadBinaryData, elm.application_version, 3, "application_version", verboseLevel);
push_bits(&payloadBinaryData, elm.minimum_application_version, 3, "minimum_application_version", verboseLevel);
push_bits(&payloadBinaryData, 0, 2, "zero_2bits", verboseLevel);

push_boolean(&payloadBinaryData, elm.has_custom_hdr_reference_white_flag, "has_custom_hdr_reference_white_flag", verboseLevel);
push_boolean(&payloadBinaryData, elm.has_adaptive_tone_map_flag, "has_adaptive_tone_map_flag", verboseLevel);
push_bits(&payloadBinaryData, 0, 6, "zero_6bits", verboseLevel);

if (elm.has_custom_hdr_reference_white_flag){
    push_16bits(&payloadBinaryData, elm.hdr_reference_white, "hdr_reference_white", verboseLevel);
}

if (elm.has_adaptive_tone_map_flag) {
    push_16bits(&payloadBinaryData, elm.baseline_hdr_headroom, "baseline_hdr_headroom", verboseLevel);
    push_boolean(&payloadBinaryData, elm.use_reference_white_tone_mapping_flag, "use_reference_white_tone_mapping_flag", verboseLevel);
    if (!elm.use_reference_white_tone_mapping_flag) {
        push_bits(&payloadBinaryData, uint8_t(elm.num_alternate_images)                      , 3, "num_alternate_images", verboseLevel);
        push_bits(&payloadBinaryData, uint8_t(elm.gain_application_space_chromaticities_mode), 2, "gain_application_space_chromaticities_mode", verboseLevel);
        push_boolean(&payloadBinaryData, elm.has_common_component_mix_params_flag               , "has_common_component_mix_params_flag", verboseLevel);
        push_boolean(&payloadBinaryData, elm.has_common_curve_params_flag                       , "has_common_curve_params_flag", verboseLevel);

        if (elm.gain_application_space_chromaticities_mode == 3) {
            for (uint16_t iCh = 0; iCh < 8; iCh++) {
                push_16bits(&payloadBinaryData, elm.gain_application_space_chromaticities[iCh], "gain_application_space_chromaticities[iCh]", verboseLevel);
            }
        }

        for (uint16_t iAlt = 0; iAlt < elm.num_alternate_images; iAlt++) {
            push_16bits(&payloadBinaryData, elm.alternate_hdr_headrooms[iAlt], "alternate_hdr_headrooms[iAlt]", verboseLevel);
            // Write component mixing function parameters
            if ( iAlt == 0 || !elm.has_common_component_mix_params_flag){
                push_bits(&payloadBinaryData, uint8_t(elm.component_mixing_type[iAlt]), 2, "component_mixing_type[iAlt]", verboseLevel);
                if (elm.component_mixing_type[iAlt] == 3) {
                    // Write the flag to indicate which coefficients are signaled 
                    uint8_t value_8 = 0;
                    for (uint8_t iCm = 0; iCm < MAX_NB_COMPONENT_MIXING_COEFFICIENT; iCm++) {
                        uint8_t flagValue = static_cast<uint8_t>(elm.has_component_mixing_coefficient_flag[iAlt][iCm]);
                        value_8 = value_8 + (flagValue << (5 - iCm) );
                    }   
                    push_bits(&payloadBinaryData, value_8, 6, "has_component_mixing_coefficient_flag[iAlt]", verboseLevel);
                    // Write the coefficients 
                    for (uint8_t iCm = 0; iCm < MAX_NB_COMPONENT_MIXING_COEFFICIENT; iCm++) {
                        if (elm.has_component_mixing_coefficient_flag[iAlt][iCm]) {
                            push_16bits(&payloadBinaryData, elm.component_mixing_coefficient[iAlt][iCm], "component_mixing_coefficient[iAlt][iCm]", verboseLevel);
                        }
                    }                
                } else {
                    push_bits(&payloadBinaryData, 0, 6, "zero_6bits[iAlt][iCm]", verboseLevel);
                }
            }
            /// Write gain curve function parameters
            if ( iAlt == 0 || !elm.has_common_curve_params_flag){
                push_bits(&payloadBinaryData,    elm.gain_curve_num_control_points_minus_1[iAlt], 5, "gain_curve_num_control_points_minus_1[iAlt]", verboseLevel);
                push_boolean(&payloadBinaryData, elm.gain_curve_use_pchip_slope_flag[iAlt]      ,    "gain_curve_use_pchip_slope_flag[iAlt]", verboseLevel); 
                push_bits(&payloadBinaryData,     0                                             , 2, "zero_2bits[iAlt]", verboseLevel);                
                for (uint16_t iCps = 0; iCps < elm.gain_curve_num_control_points_minus_1[iAlt] + 1; iCps++){
                    push_16bits(&payloadBinaryData, elm.gain_curve_control_points_x[iAlt][iCps], "gain_curve_control_points_x[iAlt][iCps]", verboseLevel);
                }
            }
            for (uint16_t iCps = 0; iCps < elm.gain_curve_num_control_points_minus_1[iAlt] + 1; iCps++) {
                push_16bits(&payloadBinaryData, elm.gain_curve_control_points_y[iAlt][iCps], "gain_curve_control_points_y[iAlt][iCps]", verboseLevel);
            }
            if (!elm.gain_curve_use_pchip_slope_flag[iAlt]) {
                for (uint16_t iCps = 0; iCps < elm.gain_curve_num_control_points_minus_1[iAlt] + 1; iCps++) {
                    push_16bits(&payloadBinaryData, elm.gain_curve_control_points_theta[iAlt][iCps], "gain_curve_control_points_theta[iAlt][iCps]", verboseLevel);
                }
            }
        }
    }
    else{ // No more information need to be signaled when using Reference White Tone Mapping Operator
        push_bits(&payloadBinaryData, 0, 7, "zero_7bits", verboseLevel);
    }
}
// Verify binary is byte complete and popback last added new byte 
if (payloadBinaryData.bitIdx != 0){
    logMsg(LOGLEVEL_ERROR, "*Critical* Binary data writing did not finish with a full byte.");
} else {
    payloadBinaryData.payload.pop_back();
    logMsg(LOGLEVEL_DEBUG, "End SMPTE_ST2094_50::writeSyntaxElementsToBinaryData, payload size = %d bytes", payloadBinaryData.byteIdx);
    dbgPrintMetadataItems(); // Put here for easy comparison of logs
}
}

/* *********************************** DECODING SECTION ********************************************************************************************/

// Decode binary data into syntax elements
void SMPTE_ST2094_50::decodeBinaryToSyntaxElements(std::vector<uint8_t> binary_data) {

  // Adapt binary data
  // Initialize the binary payload structure
  payloadBinaryData.byteIdx = 0;
  payloadBinaryData.bitIdx  = 0;
  for (int i = 0; i < int(binary_data.size()); i++) {
      payloadBinaryData.payload.push_back(binary_data[i]);
  }
  printBinaryData(binary_data,  verboseLevel);

  logMsg(LOGLEVEL_DEBUG, "Syntax Elements Decoding");
  elm.application_version = pull_bits(&payloadBinaryData, 3, "application_version", verboseLevel);
  elm.minimum_application_version = pull_bits(&payloadBinaryData, 3, "minimum_application_version", verboseLevel);
  pull_bits(&payloadBinaryData, 2, "zero_2bits", verboseLevel);

  elm.has_custom_hdr_reference_white_flag = pull_boolean(&payloadBinaryData, "has_custom_hdr_reference_white_flag", verboseLevel);
  elm.has_adaptive_tone_map_flag = pull_boolean(&payloadBinaryData, "has_adaptive_tone_map_flag", verboseLevel);
  pull_bits(&payloadBinaryData, 6, "zero_6bits", verboseLevel);

  if (elm.has_custom_hdr_reference_white_flag){
    elm.hdr_reference_white = pull_16bits(&payloadBinaryData, "hdr_reference_white", verboseLevel);
  }

  if (elm.has_adaptive_tone_map_flag) {
    elm.baseline_hdr_headroom = pull_16bits(&payloadBinaryData, "baseline_hdr_headroom", verboseLevel);  
    
    elm.use_reference_white_tone_mapping_flag = pull_boolean(&payloadBinaryData, "use_reference_white_tone_mapping_flag", verboseLevel);
    if (!elm.use_reference_white_tone_mapping_flag){
        elm.num_alternate_images = pull_bits(&payloadBinaryData, 3, "num_alternate_images", verboseLevel);
        elm.gain_application_space_chromaticities_mode = pull_bits(&payloadBinaryData, 2, "gain_application_space_chromaticities_mode", verboseLevel);
        elm.has_common_component_mix_params_flag = pull_boolean(&payloadBinaryData, "has_common_component_mix_params_flag", verboseLevel);
        elm.has_common_curve_params_flag = pull_boolean(&payloadBinaryData, "has_common_curve_params_flag", verboseLevel);  

        if (elm.gain_application_space_chromaticities_mode == 3) {
            for (uint16_t iCh = 0; iCh < 8; iCh++) {
                elm.gain_application_space_chromaticities[iCh] = pull_16bits(&payloadBinaryData, "gain_application_space_chromaticities[iCh]", verboseLevel);                
              }
        }

        for (uint16_t iAlt = 0; iAlt < elm.num_alternate_images; iAlt++) {
            elm.alternate_hdr_headrooms[iAlt] = pull_16bits(&payloadBinaryData, "alternate_hdr_headrooms[iAlt]", verboseLevel); 

            // Read component mixing function parameters - Table C.4
            if ( iAlt == 0 || !elm.has_common_component_mix_params_flag){
                elm.component_mixing_type[iAlt] = pull_bits(&payloadBinaryData, 2, "component_mixing_type[iAlt]", verboseLevel);
              if (elm.component_mixing_type[iAlt] == 3) {
                uint8_t has_component_mixing_coefficient_flag = pull_bits(&payloadBinaryData, 6, "has_component_mixing_coefficient_flag[iAlt]", verboseLevel); 
                // Decode the flags and the associated values
                for (uint8_t iCm = 0; iCm < MAX_NB_COMPONENT_MIXING_COEFFICIENT; iCm++) {
                    elm.has_component_mixing_coefficient_flag[iAlt][iCm] = bool( has_component_mixing_coefficient_flag & (0x01 << (5 - iCm) ) );
                    if (elm.has_component_mixing_coefficient_flag[iAlt][iCm]) {
                        elm.component_mixing_coefficient[iAlt][iCm] = pull_16bits(&payloadBinaryData, "component_mixing_coefficient[iAlt][iCm]", verboseLevel); 
                    } else {elm.component_mixing_coefficient[iAlt][iCm] = 0;}
                }                
              } else {
                pull_bits(&payloadBinaryData, 6, "zero_6bits[iAlt][iCm]", verboseLevel); 
              }
            } else {
                elm.component_mixing_type[iAlt] = elm.component_mixing_type[0];
                if (elm.component_mixing_type[0] == 3) {
                    elm.component_mixing_type[iAlt] = elm.component_mixing_type[0];
                    for (uint8_t iCm = 0; iCm < MAX_NB_COMPONENT_MIXING_COEFFICIENT; iCm++) {
                        elm.has_component_mixing_coefficient_flag[iAlt][iCm] = elm.has_component_mixing_coefficient_flag[iAlt][iCm];
                        elm.component_mixing_coefficient[iAlt][iCm] = elm.component_mixing_coefficient[0][iCm];
                    }
                }
            }

            // Read gain curve function parameters - table C.5
            if ( iAlt == 0 || !elm.has_common_curve_params_flag){
              elm.gain_curve_num_control_points_minus_1[iAlt] = pull_bits(&payloadBinaryData, 5, "gain_curve_num_control_points_minus_1[iAlt]", verboseLevel);
                elm.gain_curve_use_pchip_slope_flag[iAlt] = pull_boolean(&payloadBinaryData, "gain_curve_use_pchip_slope_flag[iAlt]", verboseLevel);
                pull_bits(&payloadBinaryData, 2, "zero_2bits[iAlt]", verboseLevel);           
              
              for (uint16_t iCps = 0; iCps < elm.gain_curve_num_control_points_minus_1[iAlt] + 1; iCps++){
                elm.gain_curve_control_points_x[iAlt][iCps] = pull_16bits(&payloadBinaryData, "gain_curve_control_points_x[iAlt][iCps]", verboseLevel);
              }
            } else {
              elm.gain_curve_num_control_points_minus_1[iAlt] = elm.gain_curve_num_control_points_minus_1[0];
              elm.gain_curve_use_pchip_slope_flag[iAlt]       = elm.gain_curve_use_pchip_slope_flag[0];
              for (uint16_t iCps = 0; iCps < elm.gain_curve_num_control_points_minus_1[iAlt] + 1; iCps++){
                elm.gain_curve_control_points_x[iAlt][iCps] = elm.gain_curve_control_points_x[0][iCps];
              }
            }
            for (uint16_t iCps = 0; iCps < elm.gain_curve_num_control_points_minus_1[iAlt] + 1; iCps++) {
                elm.gain_curve_control_points_y[iAlt][iCps] = pull_16bits(&payloadBinaryData, "gain_curve_control_points_y[iAlt][iCps]", verboseLevel);
            }
            if (!elm.gain_curve_use_pchip_slope_flag[iAlt]) {
              for (uint16_t iCps = 0; iCps < elm.gain_curve_num_control_points_minus_1[iAlt] + 1; iCps++) {
                elm.gain_curve_control_points_theta[iAlt][iCps] = pull_16bits(&payloadBinaryData, "gain_curve_control_points_theta[iAlt][iCps]", verboseLevel);
              }
            }
        }

    } else {pull_bits(&payloadBinaryData, 7, "zero_7bits", verboseLevel);}

}
logMsg(LOGLEVEL_DEBUG, "Syntax Elements Successfully Decoded");
}

// Convert the syntax elements to Metadata Items as described in Clause C.3
void SMPTE_ST2094_50::convertSyntaxElementsToMetadataItems(){

  // get mandatory metadata
  if (elm.has_custom_hdr_reference_white_flag) {
    cvt.hdrReferenceWhite = float(elm.hdr_reference_white) / Q_HDR_REFERENCE_WHITE;
  } else {
    cvt.hdrReferenceWhite = 203.0;
  }

  // Get Optional metadata items
  if (elm.has_adaptive_tone_map_flag) {
    isHeadroomAdaptiveToneMap = true;
    HeadroomAdaptiveToneMap hatm;
    cvt.hatm.baselineHdrHeadroom = float(elm.baseline_hdr_headroom) / Q_HDR_HEADROOM;
    if (elm.use_reference_white_tone_mapping_flag) {
      isReferenceWhiteToneMapping = true;
      cvt.hatm.numAlternateImages = 2;
      // BT.2020 primaries
      cvt.hatm.gainApplicationSpaceChromaticities[0] = 0.708;
      cvt.hatm.gainApplicationSpaceChromaticities[1] = 0.292;
      cvt.hatm.gainApplicationSpaceChromaticities[2] = 0.17 ;
      cvt.hatm.gainApplicationSpaceChromaticities[3] = 0.797;
      cvt.hatm.gainApplicationSpaceChromaticities[4] = 0.131;
      cvt.hatm.gainApplicationSpaceChromaticities[5] = 0.046;
      cvt.hatm.gainApplicationSpaceChromaticities[6] = 0.3127;
      cvt.hatm.gainApplicationSpaceChromaticities[7] = 0.3290;
      // Compute alternate headroom
      cvt.hatm.alternateHdrHeadroom.push_back(0.0);
      float headroom_to_anchor_ratio = std::min(cvt.hatm.baselineHdrHeadroom / log2(1000.0/203.0), 1.0);
      float h_alt_1 = log2(8.0/3.0) * headroom_to_anchor_ratio;
      cvt.hatm.alternateHdrHeadroom.push_back(h_alt_1);
  
      // Constant parameter across alternate images
      float kappa  = 0.65;
      float x_knee = 1;
      float x_max  = pow(2.0, cvt.hatm.baselineHdrHeadroom);
      for (uint16_t iAlt = 0;  iAlt  < cvt.hatm.numAlternateImages; iAlt++){
        hasSlopeParameter[iAlt]     = true;
        // Component mixing is maxRGB
        ColorGainFunction cgf;
        cgf.cm.componentMixRed = 0.0;
        cgf.cm.componentMixGreen = 0.0;
        cgf.cm.componentMixBlue = 0.0;
        cgf.cm.componentMixMax = 1.0;
        cgf.cm.componentMixMin = 0.0;
        cgf.cm.componentMixComponent = 0.0; 
        cgf.gc.gainCurveNumControlPoints = 8;
  
        // Inner vector for push_back
        std::vector<float> inner_gainCurveControlPointX;
        std::vector<float> inner_gainCurveControlPointY;
        std::vector<float> inner_gainCurveControlPointM;
  
        // Compute the control points parameter depending on the alternate headroom
        float  y_white = 1.0;
        if (iAlt == 0) {
          y_white = 1 - (0.5 * headroom_to_anchor_ratio );
        }
        float y_knee = y_white;
        float y_max  = pow(2.0, cvt.hatm.alternateHdrHeadroom[iAlt]);
        float x_mid = (1.0 - kappa) * x_knee + kappa * (x_knee * y_max / y_knee);
        float y_mid = (1.0 - kappa) * y_knee + kappa * y_max;
        // Compute Quadratic Beziers coefficients
        float a_x = x_knee - 2 * x_mid + x_max;
        float a_y = y_knee - 2 * y_mid + y_max;
        float b_x = 2 * x_mid - 2 * x_knee;
        float b_y = 2 * y_mid - 2 * y_knee;
        float c_x = x_knee;
        float c_y = y_knee;
  
        for (uint16_t iCps = 0; iCps < cgf.gc.gainCurveNumControlPoints; iCps++) {
          // Compute the control points
          float t = float(iCps) / (float(cgf.gc.gainCurveNumControlPoints) - 1.0);
          float t_square = t * t;
          float x = a_x * t_square + b_x * t + c_x;
          float y = a_y * t_square + b_y * t + c_y;
          float m = (2.0 * a_y * t + b_y) / (2 * a_x * t + b_x);
          float slope = atan( (x * m - y) / (log(2) * x * y) );
          cgf.gc.gainCurveControlPointX.push_back(x);
          cgf.gc.gainCurveControlPointY.push_back(log2(y / x));
          cgf.gc.gainCurveControlPointM.push_back(slope / PI_CUSTOM * 180.0);
        }
        cvt.hatm.cgf.push_back(cgf);
      }
    } else {
      cvt.hatm.numAlternateImages = elm.num_alternate_images;
      if (elm.gain_application_space_chromaticities_mode == 0){
        cvt.hatm.gainApplicationSpaceChromaticities[0] = 0.64;
        cvt.hatm.gainApplicationSpaceChromaticities[1] = 0.33; 
        cvt.hatm.gainApplicationSpaceChromaticities[2] = 0.3; 
        cvt.hatm.gainApplicationSpaceChromaticities[3] = 0.6;
        cvt.hatm.gainApplicationSpaceChromaticities[4] = 0.15; 
        cvt.hatm.gainApplicationSpaceChromaticities[5] = 0.06;
        cvt.hatm.gainApplicationSpaceChromaticities[6] = 0.3127;
        cvt.hatm.gainApplicationSpaceChromaticities[7] = 0.329;
      } else if (elm.gain_application_space_chromaticities_mode == 1){
        cvt.hatm.gainApplicationSpaceChromaticities[0] = 0.68; 
        cvt.hatm.gainApplicationSpaceChromaticities[1] = 0.32; 
        cvt.hatm.gainApplicationSpaceChromaticities[2] = 0.265; 
        cvt.hatm.gainApplicationSpaceChromaticities[3] = 0.69; 
        cvt.hatm.gainApplicationSpaceChromaticities[4] = 0.15; 
        cvt.hatm.gainApplicationSpaceChromaticities[5] = 0.06; 
        cvt.hatm.gainApplicationSpaceChromaticities[6] = 0.3127; 
        cvt.hatm.gainApplicationSpaceChromaticities[7] = 0.329; 
      } else if (elm.gain_application_space_chromaticities_mode == 2){
        cvt.hatm.gainApplicationSpaceChromaticities[0] = 0.708;
        cvt.hatm.gainApplicationSpaceChromaticities[1] = 0.292;
        cvt.hatm.gainApplicationSpaceChromaticities[2] = 0.17;
        cvt.hatm.gainApplicationSpaceChromaticities[3] = 0.797;
        cvt.hatm.gainApplicationSpaceChromaticities[4] = 0.131;
        cvt.hatm.gainApplicationSpaceChromaticities[5] = 0.046;
        cvt.hatm.gainApplicationSpaceChromaticities[6] = 0.3127;
        cvt.hatm.gainApplicationSpaceChromaticities[7] = 0.329;
      } else if (elm.gain_application_space_chromaticities_mode == 3){
          for (uint16_t iCh = 0; iCh < 8; iCh++) {
            cvt.hatm.gainApplicationSpaceChromaticities[iCh] = float(elm.gain_application_space_chromaticities[iCh]) / Q_GAIN_APPLICATION_SPACE_CHROMATICITY;
          }
      } else {
        logMsg(LOGLEVEL_ERROR, "gain_application_space_primaries=%d not defined", elm.gain_application_space_chromaticities_mode);
      }
      for (uint16_t iAlt = 0;  iAlt  < cvt.hatm.numAlternateImages; iAlt++){
        cvt.hatm.alternateHdrHeadroom.push_back(float(elm.alternate_hdr_headrooms[iAlt]) / Q_HDR_HEADROOM);
        // init k_params to zero and replace the one that are not
        ColorGainFunction cgf;
        cgf.cm.componentMixRed = 0.0;
        cgf.cm.componentMixGreen = 0.0;
        cgf.cm.componentMixBlue = 0.0;
        cgf.cm.componentMixMax= 0.0;
        cgf.cm.componentMixMin = 0.0;
        cgf.cm.componentMixComponent = 0.0;
        if (elm.component_mixing_type[iAlt] == 0){
          cgf.cm.componentMixMax   = 1.0;
        } else if (elm.component_mixing_type[iAlt] == 1){
          cgf.cm.componentMixComponent   = 1.0;
        } else if (elm.component_mixing_type[iAlt] == 2){
          cgf.cm.componentMixRed   = 1.0 / 6.0;
          cgf.cm.componentMixGreen = 1.0 / 6.0;
          cgf.cm.componentMixBlue  = 1.0 / 6.0;
          cgf.cm.componentMixMax   = 1.0 / 2.0;
        } else if (elm.component_mixing_type[iAlt] == 3){
          cgf.cm.componentMixRed       = 0.0f;
          cgf.cm.componentMixGreen     = 0.0f;
          cgf.cm.componentMixBlue      = 0.0f;
          cgf.cm.componentMixMax       = 0.0f;
          cgf.cm.componentMixMin       = 0.0f;
          cgf.cm.componentMixComponent = 0.0f;
          if (elm.component_mixing_type[iAlt] == 0){ cgf.cm.componentMixMax = 1.0f;}
          else if (elm.component_mixing_type[iAlt] == 1){ cgf.cm.componentMixComponent = 1.0f;}
          else if (elm.component_mixing_type[iAlt] == 2){ 
            cgf.cm.componentMixMax = 1.0f / 2.0f; 
            cgf.cm.componentMixRed = 1.0f / 6.0f; 
            cgf.cm.componentMixGreen = 1.0f / 6.0f; 
            cgf.cm.componentMixBlue = 1.0f / 6.0f;}
          else if (elm.component_mixing_type[iAlt] == 3) {
              // Compute sum of component
              float sumComponent = 0.0f;
              for (int k = 0; k < MAX_NB_COMPONENT_MIXING_COEFFICIENT; k++) {
                sumComponent = float(elm.component_mixing_coefficient[iAlt][k]);
              }
              if (sumComponent != Q_COMPONENT_MIXING_COEFFICIENT){
                logMsg(LOGLEVEL_WARNING, "Sum component mixing coefficient for alternate %d is not equal to 1.0, they will be scaled to 1.0.", iAlt);
              }
              for (int k = 0; k < MAX_NB_COMPONENT_MIXING_COEFFICIENT; k++) {
                  float value = 0.0f;
                  if (elm.has_component_mixing_coefficient_flag[iAlt][k]) {
                      value = float(elm.component_mixing_coefficient[iAlt][k]) / sumComponent;
                  }
                  switch (k) {
                      case 0: cgf.cm.componentMixRed = value; break;
                      case 1: cgf.cm.componentMixGreen = value; break;
                      case 2: cgf.cm.componentMixBlue = value; break;
                      case 3: cgf.cm.componentMixMax = value; break;
                      case 4: cgf.cm.componentMixMin = value; break;
                      case 5: cgf.cm.componentMixComponent = value; break;
                  }
              }
          }
        } else {
          logMsg(LOGLEVEL_ERROR, "mix_encoding[%d]=%d not defined", iAlt, elm.component_mixing_type[iAlt]);
        }
        cgf.gc.gainCurveNumControlPoints = elm.gain_curve_num_control_points_minus_1[iAlt] + 1;
        // Determine the sign of the gain coefficients based on headrooms difference
        float sign = 1.0;
        if (cvt.hatm.baselineHdrHeadroom > cvt.hatm.alternateHdrHeadroom[iAlt])
        {
            sign = -1.0;
        }
        for (uint16_t iCps = 0; iCps < cgf.gc.gainCurveNumControlPoints; iCps++) {
          cgf.gc.gainCurveControlPointX.push_back(float(elm.gain_curve_control_points_x[iAlt][iCps]) / Q_GAIN_CURVE_CONTROL_POINT_X);
          cgf.gc.gainCurveControlPointY.push_back(sign * float(elm.gain_curve_control_points_y[iAlt][iCps]) / Q_GAIN_CURVE_CONTROL_POINT_Y);
          if (!elm.gain_curve_use_pchip_slope_flag[iAlt]) {
            hasSlopeParameter[iAlt]     = true;
            float theta = float(elm.gain_curve_control_points_theta[iAlt][iCps]) / Q_GAIN_CURVE_CONTROL_POINT_THETA - O_GAIN_CURVE_CONTROL_POINT_THETA;
            cgf.gc.gainCurveControlPointM.push_back( tan(theta * M_PI / 180.0f) );
          }
        }
        cvt.hatm.cgf.push_back(cgf);
        }
    }
  }
}

nlohmann::json SMPTE_ST2094_50::encodeMetadataItemsToJson() {
  nlohmann::json j;

  j["frame_start"] = timeI.timeIntervalStart;
  j["frame_duration"] = timeI.timeintervalDuration;

  j["hdrReferenceWhite"] = cvt.hdrReferenceWhite;
  if (isHeadroomAdaptiveToneMap) {
    j["baselineHdrHeadroom"] = cvt.hatm.baselineHdrHeadroom;
    if (!isReferenceWhiteToneMapping) {
      j["numAlternateImages"] = cvt.hatm.numAlternateImages;
      j["gainApplicationSpaceChromaticities"] = cvt.hatm.gainApplicationSpaceChromaticities;

      if (cvt.hatm.numAlternateImages > 0) {
        j["alternateHdrHeadroom"] = cvt.hatm.alternateHdrHeadroom;

        std::vector<float> componentMixRed;
        std::vector<float> componentMixGreen;
        std::vector<float> componentMixBlue;
        std::vector<float> componentMixMax;
        std::vector<float> componentMixMin;
        std::vector<float> componentMixComponent;

        std::vector<uint32_t> gainCurveNumControlPoints;
        std::vector<std::vector<float>> gainCurveControlPointX_ptr;
        std::vector<std::vector<float>> gainCurveControlPointY_ptr;
        std::vector<std::vector<float>> gainCurveControlPointT_ptr;
        // "Unnesting" the variables: Accessing individual members of each struct
        for (size_t i = 0; i < cvt.hatm.cgf.size(); i++) { // Range-based for loop for convenience
          componentMixRed.push_back(cvt.hatm.cgf[i].cm.componentMixRed);
          componentMixGreen.push_back(cvt.hatm.cgf[i].cm.componentMixGreen);
          componentMixBlue.push_back(cvt.hatm.cgf[i].cm.componentMixBlue);
          componentMixMax.push_back(cvt.hatm.cgf[i].cm.componentMixMax);
          componentMixMin.push_back(cvt.hatm.cgf[i].cm.componentMixMin);
          componentMixComponent.push_back(cvt.hatm.cgf[i].cm.componentMixComponent);

          gainCurveNumControlPoints.push_back(cvt.hatm.cgf[i].gc.gainCurveNumControlPoints);
          std::vector<float> gainCurveControlPointX;
          std::vector<float> gainCurveControlPointY;
          std::vector<float> gainCurveControlPointT;
          for (uint32_t p = 0; p < cvt.hatm.cgf[i].gc.gainCurveNumControlPoints; p++){
            gainCurveControlPointX.push_back(cvt.hatm.cgf[i].gc.gainCurveControlPointX[p]);
            gainCurveControlPointY.push_back(cvt.hatm.cgf[i].gc.gainCurveControlPointY[p]);
          }
          for (size_t p = 0; p < cvt.hatm.cgf[i].gc.gainCurveControlPointM.size(); p++){
            gainCurveControlPointT.push_back(cvt.hatm.cgf[i].gc.gainCurveControlPointM[p]);
          }
          gainCurveControlPointX_ptr.push_back(gainCurveControlPointX);
          gainCurveControlPointY_ptr.push_back(gainCurveControlPointY);
          gainCurveControlPointT_ptr.push_back(gainCurveControlPointT);

        }

        j["componentMixRed"]        = componentMixRed;
        j["componentMixGreen"]      = componentMixGreen;
        j["componentMixBlue"]       = componentMixBlue;
        j["componentMixMax"]        = componentMixMax;
        j["componentMixMin"]        = componentMixMin;
        j["componentMixComponent"]  = componentMixComponent;

        j["gainCurveNumControlPoints"] = gainCurveNumControlPoints;
        j["gainCurveControlPointX"]    = gainCurveControlPointX_ptr;
        j["gainCurveControlPointY"]    = gainCurveControlPointY_ptr;
        j["gainCurveControlPointX"]    = gainCurveControlPointT_ptr;
      }
    }
  }
return j;
}

/* *********************************** DEBUGGING SECTION *******************************************************************************************/
// Print the metadata item 
void SMPTE_ST2094_50::dbgPrintMetadataItems() {
    // Only print at DEBUG level or higher
    if (verboseLevel < LOGLEVEL_DEBUG) {
        return;
    }

    logMsg(LOGLEVEL_DEBUG, "Start SMPTE_ST2094_50::dbgPrintMetadataItems");
    std::cout <<"windowNumber=" << pWin.windowNumber << "\n";
    std::cout <<"hdrReferenceWhite=" << cvt.hdrReferenceWhite << "\n";
    if ( isHeadroomAdaptiveToneMap)
    {
      std::cout <<"baselineHdrHeadroom=" << cvt.hatm.baselineHdrHeadroom << "\n";
      if ( !isReferenceWhiteToneMapping) {
          std::cout <<"numAlternateImages=" << cvt.hatm.numAlternateImages << "\n";
          
          std::cout << "gainApplicationSpaceChromaticities=[" ;
          for (float val : cvt.hatm.gainApplicationSpaceChromaticities) {
              std::cout << val << ", ";
          }
          std::cout << "]" << std::endl;
          
          for (uint32_t iAlt = 0; iAlt < cvt.hatm.numAlternateImages; iAlt++) {
              std::cout <<"alternateHdrHeadroom=" << cvt.hatm.alternateHdrHeadroom[iAlt] << "\n";
              std::cout <<"componentMixRed=" << cvt.hatm.cgf[iAlt].cm.componentMixRed << "\n";
              std::cout <<"componentMixGreen=" << cvt.hatm.cgf[iAlt].cm.componentMixGreen << "\n";
              std::cout <<"componentMixBlue=" << cvt.hatm.cgf[iAlt].cm.componentMixBlue << "\n";
              std::cout <<"componentMixMax=" << cvt.hatm.cgf[iAlt].cm.componentMixMax << "\n";
              std::cout <<"componentMixMin=" << cvt.hatm.cgf[iAlt].cm.componentMixMin << "\n";
              std::cout <<"componentMixComponent=" << cvt.hatm.cgf[iAlt].cm.componentMixComponent << "\n";
              
              std::cout <<"gainCurveNumControlPoints=" << cvt.hatm.cgf[iAlt].gc.gainCurveNumControlPoints << "\n";
              
              std::cout << "gainCurveControlPointX=[" << std::endl;
              for (float val : cvt.hatm.cgf[iAlt].gc.gainCurveControlPointX) {
                  std::cout << val << ", ";
              }
              std::cout << "]" << std::endl;
              
              std::cout << "gainCurveControlPointY=[" << std::endl;
              for (float val : cvt.hatm.cgf[iAlt].gc.gainCurveControlPointY) {
                  std::cout << val << ", ";
              }
              std::cout << "]" << std::endl;
              
              std::cout << "gainCurveControlPointM=[" << std::endl;
              for (float val : cvt.hatm.cgf[iAlt].gc.gainCurveControlPointM) {
                  std::cout << val << ", ";
              }
              std::cout << "]" << std::endl;
          }
      }
    }
    logMsg(LOGLEVEL_DEBUG, "End SMPTE_ST2094_50::dbgPrintMetadataItems");
  }
