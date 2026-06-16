#ifndef SMPTE_ST2094_50_HPP
#define SMPTE_ST2094_50_HPP
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

// 3rd party headers
#include <nlohmann/json.hpp>

const float  PI_CUSTOM = 3.14159265358979323846;

const int MAX_NB_ALTERNATE = 4;
const int MAX_NB_CONTROL_POINTS = 32;
const int MAX_NB_CHROMATICITIES = 8;
const int MAX_NB_COMPONENT_MIXING_COEFFICIENT = 6;

// Compute quantization error of each float to uint16_t
const float Q_HDR_REFERENCE_WHITE = 5.0f; // Scaling 0.2 to 10000.0 range to 1-50000 (sampling of 0.2)
const float Q_HDR_HEADROOM   = 10000.0f; // Scaling 0.0 to +6.0 range to 0-60000 (sampling of 0.00001)
const float P_HDR_HEADROOM   = 0.5f / Q_HDR_HEADROOM; // Scaling 0.0 to +6.0 range to 0-60000 (sampling of 0.00001)
const float Q_GAIN_APPLICATION_SPACE_CHROMATICITY = 50000.0f; // Scaling 0.0 to +1.0 range to 0-50000 (sampling of 0.00002)
const float P_GAIN_APPLICATION_SPACE_CHROMATICITY = 0.5f /Q_GAIN_APPLICATION_SPACE_CHROMATICITY;  // Maximum quantization error of chromaticity
const float Q_COMPONENT_MIXING_COEFFICIENT = 50000.0f; // Scaling 0.0 to +1.0 range to 0-50000 (sampling of 0.00002)
const float P_COMPONENT_MIXING_COEFFICIENT = 0.5f / Q_COMPONENT_MIXING_COEFFICIENT; // Maximum quantization error of component mixing coefficient
const float Q_GAIN_CURVE_CONTROL_POINT_X = 1000.0f; // Scaling 0.0 to +64.0 range to 0-64000 (sampling of 0.0001)
const float Q_GAIN_CURVE_CONTROL_POINT_Y = 10000.0f; // Scaling 0.0 to +6.0 range to 0-60000 (sampling of 0.00001)
const float O_GAIN_CURVE_CONTROL_POINT_THETA = 90.0f; // Offset  to bring -90.0 to +90.0 range to 0-180
const float Q_GAIN_CURVE_CONTROL_POINT_THETA = 200.0f; // Scaling -90.0 to +90.0 range to 0-36000 (sampling of 0.005)

struct BinaryData{
    std::vector<uint8_t> payload;
    uint16_t byteIdx;
    uint8_t  bitIdx;
  };

struct GainCurve{
  // std::vector<uint32_t> gainCurveInterpolation; -> not in the spec currently
  uint32_t gainCurveNumControlPoints;
  std::vector<float> gainCurveControlPointX;
  std::vector<float> gainCurveControlPointY;
  std::vector<float> gainCurveControlPointM;
};

struct ComponentMix{
  float componentMixRed;
  float componentMixGreen;
  float componentMixBlue;
  float componentMixMax;
  float componentMixMin;
  float componentMixComponent;
};

struct ColorGainFunction{
  ComponentMix cm;
  GainCurve gc;
};

struct HeadroomAdaptiveToneMap{
  float baselineHdrHeadroom;
  
  uint32_t numAlternateImages;
  float gainApplicationSpaceChromaticities[MAX_NB_CHROMATICITIES];
  std::vector<float> alternateHdrHeadroom;
  std::vector<ColorGainFunction> cgf;
};

struct ColorVolumeTransform{
  float hdrReferenceWhite;
  HeadroomAdaptiveToneMap hatm;
};

struct ProcessingWindow{
  uint32_t upperLeftCorner;
  uint32_t lowerRightCorner;
  uint32_t windowNumber;
};

struct TimeInterval{
  uint32_t timeIntervalStart;
  uint32_t timeintervalDuration;
};

// Structure with the syntax elements
struct SyntaxElements {
    // smpte_st_2094_50_application_info
    uint16_t application_version;
    uint16_t minimum_application_version;

    // smpte_st_2094_50_color_volume_transform
    bool has_custom_hdr_reference_white_flag;   
    bool has_adaptive_tone_map_flag;                                               
    uint16_t hdr_reference_white ;  
    
    // smpte_st_2094_50_adaptive_tone_map
    uint16_t baseline_hdr_headroom ;   
    bool use_reference_white_tone_mapping_flag;
    uint16_t num_alternate_images;          
    uint16_t gain_application_space_chromaticities_mode;
    bool has_common_component_mix_params_flag;
    bool has_common_curve_params_flag;
    uint16_t gain_application_space_chromaticities[MAX_NB_CHROMATICITIES];
    uint16_t  alternate_hdr_headrooms[MAX_NB_ALTERNATE];                
  
    // smpte_st_2094_50_component_mixing
    uint16_t  component_mixing_type[MAX_NB_ALTERNATE];
    bool has_component_mixing_coefficient_flag[MAX_NB_ALTERNATE][MAX_NB_COMPONENT_MIXING_COEFFICIENT];
    uint16_t  component_mixing_coefficient[MAX_NB_ALTERNATE][MAX_NB_COMPONENT_MIXING_COEFFICIENT];
  
    // smpte_st_2094_50_gain_curve
    uint16_t  gain_curve_num_control_points_minus_1[MAX_NB_ALTERNATE];
    bool  gain_curve_use_pchip_slope_flag[MAX_NB_ALTERNATE];
    uint16_t  gain_curve_control_points_x[MAX_NB_ALTERNATE][MAX_NB_CONTROL_POINTS];         
    uint16_t  gain_curve_control_points_y[MAX_NB_ALTERNATE][MAX_NB_CONTROL_POINTS];            
    uint16_t  gain_curve_control_points_theta[MAX_NB_ALTERNATE][MAX_NB_CONTROL_POINTS];     
  };

class SMPTE_ST2094_50 {
public:
    SMPTE_ST2094_50(); // Constructor
    bool decodeJsonToMetadataItems(nlohmann::json j); 
    void convertMetadataItemsToSyntaxElements();
    void writeSyntaxElementsToBinaryData();

    void decodeBinaryToSyntaxElements(std::vector<uint8_t> binary_data);
    void convertSyntaxElementsToMetadataItems();
    nlohmann::json encodeMetadataItemsToJson();

    // Getters
    std::vector<uint8_t>    getPayloadData();
    uint32_t                getTimeIntervalStart();
    uint32_t                getTimeintervalDuration();
    int                     getVerboseLevel() const { return verboseLevel; }

    // Setters
    void                    setTimeIntervalStart(uint32_t frame_start);
    void                    setTimeintervalDuration(uint32_t frame_duration);
    void                    setVerboseLevel(int level);


    void dbgPrintMetadataItems();
    // Carrying mechanism information
    std::string keyValue;
    BinaryData payloadBinaryData;

private:
    uint8_t applicationIdentifier;
    uint8_t applicationVersion;
    TimeInterval timeI;
    ProcessingWindow pWin;
    ColorVolumeTransform cvt;

    // not in specification, convenience flag for implementation
    bool isHeadroomAdaptiveToneMap;   
    bool isReferenceWhiteToneMapping;
    bool hasSlopeParameter[MAX_NB_ALTERNATE];

    SyntaxElements elm;
    
    // Verbose level control
    int verboseLevel;
};

#endif // SMPTE_ST2094_50_HPP