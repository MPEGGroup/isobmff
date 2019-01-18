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

#include <stdio.h>
#include <math.h>
#include "uniDrcCommon.h"
#include "uniDrc.h"
#include "uniDrcTables.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Huffman code table for codingMode 1 */
const DeltaGainCodeEntry deltaGainCodeTableByValue[] =
{
    {  4, 0x000 /*          0000 */, -2.000f },
    {  9, 0x039 /*   0 0011 1001 */, -1.875f },
    { 11, 0x0E2 /* 000 1110 0010 */, -1.750f },
    { 11, 0x0E3 /* 000 1110 0011 */, -1.625f },
    { 10, 0x070 /*  00 0111 0000 */, -1.500f },
    { 10, 0x1AC /*  01 1010 1100 */, -1.375f },
    { 10, 0x1AD /*  01 1010 1101 */, -1.250f },
    {  9, 0x0D5 /*   0 1101 0101 */, -1.125f },
    {  7, 0x00F /*      000 1111 */, -1.000f },
    {  7, 0x034 /*      011 0100 */, -0.875f },
    {  7, 0x036 /*      011 0110 */, -0.750f },
    {  6, 0x019 /*       01 1001 */, -0.625f },
    {  5, 0x002 /*        0 0010 */, -0.500f },
    {  5, 0x00F /*        0 1111 */, -0.375f },
    {  3, 0x001 /*           001 */, -0.250f },
    {  2, 0x003 /*            11 */, -0.125f },
    {  3, 0x002 /*           010 */,  0.000f },
    {  2, 0x002 /*            10 */,  0.125f },
    {  6, 0x018 /*       01 1000 */,  0.250f },
    {  6, 0x006 /*       00 0110 */,  0.375f },
    {  7, 0x037 /*      011 0111 */,  0.500f },
    {  8, 0x01D /*     0001 1101 */,  0.625f },
    {  9, 0x0D7 /*   0 1101 0111 */,  0.750f },
    {  9, 0x0D4 /*   0 1101 0100 */,  0.875f },
    {  5, 0x00E /*        0 1110 */,  1.000f },
    
};
/* Huffman code table for codingMode 1 */
const DeltaGainCodeEntry deltaGainCodeTableBySize[] =
{
    {  2, 0x003 /*            11 */, -0.125f },
    {  2, 0x002 /*            10 */,  0.125f },
    {  3, 0x001 /*           001 */, -0.250f },
    {  3, 0x002 /*           010 */,  0.000f },
    {  4, 0x000 /*          0000 */, -2.000f },
    {  5, 0x002 /*        0 0010 */, -0.500f },
    {  5, 0x00F /*        0 1111 */, -0.375f },
    {  5, 0x00E /*        0 1110 */,  1.000f },
    {  6, 0x019 /*       01 1001 */, -0.625f },
    {  6, 0x018 /*       01 1000 */,  0.250f },
    {  6, 0x006 /*       00 0110 */,  0.375f },
    {  7, 0x00F /*      000 1111 */, -1.000f },
    {  7, 0x034 /*      011 0100 */, -0.875f },
    {  7, 0x036 /*      011 0110 */, -0.750f },
    {  7, 0x037 /*      011 0111 */,  0.500f },
    {  8, 0x01D /*     0001 1101 */,  0.625f },
    {  9, 0x039 /*   0 0011 1001 */, -1.875f },
    {  9, 0x0D5 /*   0 1101 0101 */, -1.125f },
    {  9, 0x0D7 /*   0 1101 0111 */,  0.750f },
    {  9, 0x0D4 /*   0 1101 0100 */,  0.875f },
    { 10, 0x070 /*  00 0111 0000 */, -1.500f },
    { 10, 0x1AC /*  01 1010 1100 */, -1.375f },
    { 10, 0x1AD /*  01 1010 1101 */, -1.250f },
    { 11, 0x0E2 /* 000 1110 0010 */, -1.750f },
    { 11, 0x0E3 /* 000 1110 0011 */, -1.625f }
    
};

static const int kNumDeltaGainValuesTable = sizeof(deltaGainCodeTableBySize)/sizeof(deltaGainCodeTableBySize[0]);


const DeltaGainCodeEntry deltaGainCodeTableProfile2BySize[] =
{
    { 3,	0x007,	-0.125f },
    { 4,	0x00C,	-0.625f },
    { 4,	0x009,	-0.500f },
    { 4,	0x005,	-0.375f },
    { 4,	0x003,	-0.250f },
    { 4,	0x001,	 0.000f },
    { 4,	0x00B,	 0.125f },
    { 5,	0x011,	-0.875f },
    { 5,	0x00E,	-0.750f },
    { 5,	0x005,	 0.250f },
    { 5,	0x004,	 0.375f },
    { 5,	0x008,	 0.500f },
    { 5,	0x000,	 0.625f },
    { 5,	0x00D,	 0.750f },
    { 5,	0x00F,	 0.875f },
    { 5,	0x010,	 1.000f },
    { 5,	0x01B,	 1.125f },
    { 6,	0x02B,	-1.250f },
    { 6,	0x028,	-1.125f },
    { 6,	0x002,	-1.000f },
    { 6,	0x012,	 1.250f },
    { 6,	0x018,	 1.375f },
    { 6,	0x029,	 1.500f },
    { 7,	0x06A,	-4.000f },
    { 7,	0x054,	-1.750f },
    { 7,	0x068,	-1.625f },
    { 7,	0x026,	-1.500f },
    { 7,	0x006,	-1.375f },
    { 7,	0x032,	 1.625f },
    { 8,	0x0D2,	-2.250f },
    { 8,	0x0AB,	-2.125f },
    { 8,	0x0AA,	-2.000f },
    { 8,	0x04F,	-1.875f },
    { 8,	0x04E,	 1.750f },
    { 8,	0x0D7,	 1.875f },
    { 8,	0x00E,	 2.000f },
    { 9,	0x1AD,	-3.625f },
    { 9,	0x1AC,	-3.375f },
    { 9,	0x1A6,	-3.250f },
    { 9,	0x0CD,	-3.125f },
    { 9,	0x0CE,	-2.750f },
    { 9,	0x1A7,	-2.625f },
    { 9,	0x01F,	-2.500f },
    { 9,	0x0CC,	-2.375f },
    { 10,	0x03C,	-3.500f },
    { 10,	0x19E,	-3.000f },
    { 10,	0x19F,	-2.875f },
    { 11,	0x07A,	-3.875f },
    { 11,	0x07B,	-3.750f }
};
const DeltaGainCodeEntry deltaGainCodeTableProfile2ByValue[] =
{
    { 7,	0x06A,	-4.000f },
    { 11,	0x07A,	-3.875f },
    { 11,	0x07B,	-3.750f },
    { 9,	0x1AD,	-3.625f },
    { 10,	0x03C,	-3.500f },
    { 9,	0x1AC,	-3.375f },
    { 9,	0x1A6,	-3.250f },
    { 9,	0x0CD,	-3.125f },
    { 10,	0x19E,	-3.000f },
    { 10,	0x19F,	-2.875f },
    { 9,	0x0CE,	-2.750f },
    { 9,	0x1A7,	-2.625f },
    { 9,	0x01F,	-2.500f },
    { 9,	0x0CC,	-2.375f },
    { 8,	0x0D2,	-2.250f },
    { 8,	0x0AB,	-2.125f },
    { 8,	0x0AA,	-2.000f },
    { 8,	0x04F,	-1.875f },
    { 7,	0x054,	-1.750f },
    { 7,	0x068,	-1.625f },
    { 7,	0x026,	-1.500f },
    { 7,	0x006,	-1.375f },
    { 6,	0x02B,	-1.250f },
    { 6,	0x028,	-1.125f },
    { 6,	0x002,	-1.000f },
    { 5,	0x011,	-0.875f },
    { 5,	0x00E,	-0.750f },
    { 4,	0x00C,	-0.625f },
    { 4,	0x009,	-0.500f },
    { 4,	0x005,	-0.375f },
    { 4,	0x003,	-0.250f },
    { 3,	0x007,	-0.125f },
    { 4,	0x001,	0.000f },
    { 4,	0x00B,	0.125f },
    { 5,	0x005,	0.250f },
    { 5,	0x004,	0.375f },
    { 5,	0x008,	0.500f },
    { 5,	0x000,	0.625f },
    { 5,	0x00D,	0.750f },
    { 5,	0x00F,	0.875f },
    { 5,	0x010,	1.000f },
    { 5,	0x01B,	1.125f },
    { 6,	0x012,	1.250f },
    { 6,	0x018,	1.375f },
    { 6,	0x029,	1.500f },
    { 7,	0x032,	1.625f },
    { 8,	0x04E,	1.750f },
    { 8,	0x0D7,	1.875f },
    { 8,	0x00E,	2.000 }
};

static const int kNumDeltaGainValuesTableProfile2 = sizeof(deltaGainCodeTableProfile2BySize)/sizeof(deltaGainCodeTableProfile2BySize[0]);

static const SlopeCodeTableEntry slopeCodeTableEntryByValue[] =
{
  {  6, 0x018, -3.0518f,  0 },
  {  8, 0x042, -1.2207f,  1 },
  {  7, 0x032, -0.4883f,  2 },
  {  5, 0x00A, -0.1953f,  3 },
  {  5, 0x009, -0.0781f,  4 },
  {  5, 0x00D, -0.0312f,  5 },
  {  2, 0x000,  -0.005f,  6 },
  {  1, 0x001,     0.0f,  7 },
  {  4, 0x007,   0.005f,  8 },
  {  5, 0x00B,  0.0312f,  9 },
  {  6, 0x011,  0.0781f, 10 },
  {  9, 0x087,  0.1953f, 11 },
  {  9, 0x086,  0.4883f, 12 },
  {  7, 0x020,  1.2207f, 13 },
  {  7, 0x033,  3.0518f, 14 },
};

static const SlopeCodeTableEntry slopeCodeTableEntryBySize[] =
{
  {  1, 0x001,     0.0f,  7 },
  {  2, 0x000,  -0.005f,  6 },
  {  4, 0x007,   0.005f,  8 },
  {  5, 0x00A, -0.1953f,  3 },
  {  5, 0x009, -0.0781f,  4 },
  {  5, 0x00D, -0.0312f,  5 },
  {  5, 0x00B,  0.0312f,  9 },
  {  6, 0x018, -3.0518f,  0 },
  {  6, 0x011,  0.0781f, 10 },
  {  7, 0x032, -0.4883f,  2 },
  {  7, 0x020,  1.2207f, 13 },
  {  7, 0x033,  3.0518f, 14 },
  {  8, 0x042, -1.2207f,  1 },
  {  9, 0x087,  0.1953f, 11 },
  {  9, 0x086,  0.4883f, 12 },
};

static const int kNumSlopeValuesTable = sizeof(slopeCodeTableEntryBySize)/sizeof(slopeCodeTableEntryBySize[0]);

const float downmixCoeff[] =
{
     0.0f,
    -0.5f,
    -1.0f,
    -1.5f,
    -2.0f,
    -2.5f,
    -3.0f,
    -3.5f,
    -4.0f,
    -4.5f,
    -5.0f,
    -5.5f,
    -6.0f,
    -7.5f,
    -9.0f,
    -1000.0f
};

const float downmixCoeffLfe[] =
{
    10.0f,
    6.0f,
    4.5f,
    3.0f,
    1.5f,
    0.0f,
    -1.5f,
    -3.0f,
    -4.5f,
    -6.0f,
    -10.0f,
    -15.0f,
    -20.0f,
    -30.0f,
    -40.0f,
    -1000.0f
};
    
#if AMD1_SYNTAX
const float channelWeight[] =
{
    10.0f,
    6.0f,
    4.5f,
    3.0f,
    1.5f,
    0.0f,
    -1.5f,
    -3.0f,
    -4.5f,
    -6.0f,
    -10.0f,
    -15.0f,
    -20.0f,
    -30.0f,
    -40.0f,
    -1000.0f
};
#endif /* AMD1_SYNTAX */
    
#if MPEG_D_DRC_EXTENSION_V1
const float downmixCoeffV1[] =
{
    10.00f,
    6.00f,
    4.50f,
    3.00f,
    1.50f,
    0.00f,
    -0.50f,
    -1.00f,
    -1.50f,
    -2.00f,
    -2.50f,
    -3.00f,
    -3.50f,
    -4.00f,
    -4.50f,
    -5.00f,
    -5.50f,
    -6.00f,
    -6.50f,
    -7.00f,
    -7.50f,
    -8.00f,
    -9.00f,
    -10.00f,
    -11.00f,
    -12.00f,
    -15.00f,
    -20.00f,
    -25.00f,
    -30.00f,
    -40.00f,
    -100000.0f
};

const float eqSlopeTable [] = {
    -32.0f,  /* 0 */
    -24.0f,  /* 1 */
    -18.0f,  /* 2 */
    -12.0f,  /* 3 */
    -7.0f,   /* 4 */
    -4.0f,   /* 5 */
    -2.0f,   /* 6 */
    -1.0f,   /* 7 */
    1.0f,    /* 8 */
    2.0f,    /* 9 */
    4.0f,    /* 10 */
    7.0f,    /* 11 */
    12.0f,   /* 12 */
    18.0f,   /* 13 */
    24.0f,   /* 14 */
    32.0f    /* 15 */
};


const float eqGainDeltaTable[] = {
    -22.0f,  /* 0 */
    -16.0f,  /* 1 */
    -13.0f,  /* 2 */
    -11.0f,  /* 3 */
    -9.0f,  /* 4 */
    -7.0f,  /* 5 */
    -6.0f,  /* 6 */
    -5.0f,  /* 7 */
    -4.0f,  /* 8 */
    -3.0f,  /* 9 */
    -2.5f,  /* 10 */
    -2.0f,  /* 11 */
    -1.5f,  /* 12 */
    -1.0f,  /* 13 */
    -0.5f,  /* 14 */
    0.0f,  /* 15 */
    0.5f,  /* 16 */
    1.0f,  /* 17 */
    1.5f,  /* 18 */
    2.0f,  /* 19 */
    2.5f,  /* 20 */
    3.0f,  /* 21 */
    4.0f,  /* 22 */
    5.0f,  /* 23 */
    6.0f,  /* 24 */
    7.0f,  /* 25 */
    9.0f,  /* 26 */
    11.0f,  /* 27 */
    13.0f,  /* 28 */
    16.0f,  /* 29 */
    22.0f,  /* 30 */
    32.0f,  /* 31 */
};


const float zeroPoleRadiusTable[] = {
    0.00000000E+00f,
    7.57409621E-11f,
    7.47451079E-09f,
    7.37623509E-08f,
    3.37872933E-07f,
    1.05439995E-06f,
    2.61370951E-06f,
    5.55702854E-06f,
    1.05878771E-05f,
    1.85806475E-05f,
    3.05868707E-05f,
    4.78395414E-05f,
    7.17558214E-05f,
    1.03938342E-04f,
    1.46175269E-04f,
    2.00439375E-04f,
    2.68886099E-04f,
    3.53850890E-04f,
    4.57845890E-04f,
    5.83555840E-04f,
    7.33833469E-04f,
    9.11694835E-04f,
    1.12031354E-03f,
    1.36301492E-03f,
    1.64327072E-03f,
    1.96469179E-03f,
    2.33102194E-03f,
    2.74613220E-03f,
    3.21401190E-03f,
    3.73876374E-03f,
    4.32459544E-03f,
    4.97581391E-03f,
    5.69681637E-03f,
    6.49208482E-03f,
    7.36617809E-03f,
    8.32372531E-03f,
    9.36941616E-03f,
    1.05079999E-02f,
    1.17442720E-02f,
    1.30830696E-02f,
    1.45292655E-02f,
    1.60877611E-02f,
    1.77634824E-02f,
    1.95613634E-02f,
    2.14863531E-02f,
    2.35434026E-02f,
    2.57374570E-02f,
    2.80734543E-02f,
    3.05563174E-02f,
    3.31909470E-02f,
    3.59822176E-02f,
    3.89349759E-02f,
    4.20540236E-02f,
    4.53441292E-02f,
    4.88100089E-02f,
    5.24563305E-02f,
    5.62877022E-02f,
    6.03086725E-02f,
    6.45237267E-02f,
    6.89372867E-02f,
    7.35536888E-02f,
    7.83772022E-02f,
    8.34120139E-02f,
    8.86622295E-02f,
    9.41318572E-02f,
    9.98248383E-02f,
    1.05744988E-01f,
    1.11896060E-01f,
    1.18281692E-01f,
    1.24905407E-01f,
    1.31770656E-01f,
    1.38880774E-01f,
    1.46238968E-01f,
    1.53848350E-01f,
    1.61711931E-01f,
    1.69832602E-01f,
    1.78213134E-01f,
    1.86856180E-01f,
    1.95764288E-01f,
    2.04939872E-01f,
    2.14385241E-01f,
    2.24102572E-01f,
    2.34093949E-01f,
    2.44361281E-01f,
    2.54906416E-01f,
    2.65731007E-01f,
    2.76836663E-01f,
    2.88224846E-01f,
    2.99896836E-01f,
    3.11853856E-01f,
    3.24096978E-01f,
    3.36627185E-01f,
    3.49445283E-01f,
    3.62551987E-01f,
    3.75947863E-01f,
    3.89633417E-01f,
    4.03608948E-01f,
    4.17874694E-01f,
    4.32430804E-01f,
    4.47277188E-01f,
    4.62413728E-01f,
    4.77840215E-01f,
    4.93556231E-01f,
    5.09561300E-01f,
    5.25854886E-01f,
    5.42436182E-01f,
    5.59304416E-01f,
    5.76458573E-01f,
    5.93897760E-01f,
    6.11620665E-01f,
    6.29626155E-01f,
    6.47912800E-01f,
    6.66479111E-01f,
    6.85323536E-01f,
    7.04444408E-01f,
    7.23839939E-01f,
    7.43508339E-01f,
    7.63447523E-01f,
    7.83655465E-01f,
    8.04130018E-01f,
    8.24868977E-01f,
    8.45869958E-01f,
    8.67130578E-01f,
    8.88648331E-01f,
    9.10420537E-01f,
    9.32444632E-01f,
    9.54717815E-01f,
    9.77237225E-01f
};

const float zeroPoleAngleTable[] = {
    0.00000000E+00f,
    6.90533966E-04f,
    7.31595252E-04f,
    7.75098170E-04f,
    8.21187906E-04f,
    8.70018279E-04f,
    9.21752258E-04f,
    9.76562500E-04f,
    1.03463193E-03f,
    1.09615434E-03f,
    1.16133507E-03f,
    1.23039165E-03f,
    1.30355455E-03f,
    1.38106793E-03f,
    1.46319050E-03f,
    1.55019634E-03f,
    1.64237581E-03f,
    1.74003656E-03f,
    1.84350452E-03f,
    1.95312500E-03f,
    2.06926386E-03f,
    2.19230869E-03f,
    2.32267015E-03f,
    2.46078330E-03f,
    2.60710909E-03f,
    2.76213586E-03f,
    2.92638101E-03f,
    3.10039268E-03f,
    3.28475162E-03f,
    3.48007312E-03f,
    3.68700903E-03f,
    3.90625000E-03f,
    4.13852771E-03f,
    4.38461738E-03f,
    4.64534029E-03f,
    4.92156660E-03f,
    5.21421818E-03f,
    5.52427173E-03f,
    5.85276202E-03f,
    6.20078536E-03f,
    6.56950324E-03f,
    6.96014624E-03f,
    7.37401807E-03f,
    7.81250000E-03f,
    8.27705542E-03f,
    8.76923475E-03f,
    9.29068059E-03f,
    9.84313320E-03f,
    1.04284364E-02f,
    1.10485435E-02f,
    1.17055240E-02f,
    1.24015707E-02f,
    1.31390065E-02f,
    1.39202925E-02f,
    1.47480361E-02f,
    1.56250000E-02f,
    1.65541108E-02f,
    1.75384695E-02f,
    1.85813612E-02f,
    1.96862664E-02f,
    2.08568727E-02f,
    2.20970869E-02f,
    2.34110481E-02f,
    2.48031414E-02f,
    2.62780130E-02f,
    2.78405849E-02f,
    2.94960723E-02f,
    3.12500000E-02f,
    3.31082217E-02f,
    3.50769390E-02f,
    3.71627223E-02f,
    3.93725328E-02f,
    4.17137454E-02f,
    4.41941738E-02f,
    4.68220962E-02f,
    4.96062829E-02f,
    5.25560260E-02f,
    5.56811699E-02f,
    5.89921445E-02f,
    6.25000000E-02f,
    6.62164434E-02f,
    7.01538780E-02f,
    7.43254447E-02f,
    7.87450656E-02f,
    8.34274909E-02f,
    8.83883476E-02f,
    9.36441923E-02f,
    9.92125657E-02f,
    1.05112052E-01f,
    1.11362340E-01f,
    1.17984289E-01f,
    1.25000000E-01f,
    1.32432887E-01f,
    1.40307756E-01f,
    1.48650889E-01f,
    1.57490131E-01f,
    1.66854982E-01f,
    1.76776695E-01f,
    1.87288385E-01f,
    1.98425131E-01f,
    2.10224104E-01f,
    2.22724680E-01f,
    2.35968578E-01f,
    2.50000000E-01f,
    2.64865774E-01f,
    2.80615512E-01f,
    2.97301779E-01f,
    3.14980262E-01f,
    3.33709964E-01f,
    3.53553391E-01f,
    3.74576769E-01f,
    3.96850263E-01f,
    4.20448208E-01f,
    4.45449359E-01f,
    4.71937156E-01f,
    5.00000000E-01f,
    5.29731547E-01f,
    5.61231024E-01f,
    5.94603558E-01f,
    6.29960525E-01f,
    6.67419927E-01f,
    7.07106781E-01f,
    7.49153538E-01f,
    7.93700526E-01f,
    8.40896415E-01f,
    8.90898718E-01f,
    9.43874313E-01f,
    1.00000000E+00f
};

const float shapeFilterY1BoundLfTable[][3] = {
    {-0.994f,-0.996f, -1.0f},
    {-0.99f, -0.995f, -0.999f},
    {-0.98f, -0.989f, -0.996f},
    {-0.97f, -0.983f, -0.994f},
};

const float shapeFilterY1BoundHfTable[][3] = {
    {0.15f,  0.75f, 1.05f  },
    {0.43f,  0.87f, 1.07f},
    {0.60f,  0.92f, 1.07f},
    {0.80f,  1.00f, 1.06f},
    {0.90f,  1.04f, 1.073f},
};

const float shapeFilterGainOffsetLfTable[][3] = {
    {3.0f, 2.0f, 1.2f},
    {3.0f, 2.0f, 1.5f},
    {3.0f, 2.0f, 2.0f},
    {3.0f, 2.0f, 2.0f},
};

const float shapeFilterGainOffsetHfTable[][3] = {
    {4.5f, 6.0f, 3.5f},
    {3.7f, 4.0f, 2.7f},
    {3.0f, 3.5f, 2.0f},
    {2.0f, 2.5f, 1.5f},
    {1.5f, 2.0f, 1.31f},
};

const float shapeFilterRadiusLfTable[] = {
    0.988f,
    0.98f,
    0.96f,
    0.94f,
};

const float shapeFilterRadiusHfTable[] = {
    0.45f,
    0.40f,
    0.35f,
    0.30f,
    0.30f
};

const float shapeFilterCutoffFreqNormHfTable[] = {
    0.15f,
    0.20f,
    0.25f,
    0.35f,
    0.45f
};

const CicpSigmoidCharacteristicParam cicpSigmoidCharacteristicParamTable[] = {
    {0.0f, 9.0f, 12.0f},
    {0.2f, 9.0f, 12.0f},
    {0.4f, 9.0f, 12.0f},
    {0.6f, 9.0f, 12.0f},
    {0.8f, 6.0f, 8.0f},
    {1.0f, 5.0f, 6.0f}
};

/*
 const cicpNodeCharacteristicParamTable[] = {
 {5, {-53.0f, 6.0f}, {-41.0f, 0.0f}, {-21.0f, 0.0f}, {-11.0f, -5.0f}, {9.0f, -24.0f}},
 {5, {-43.0f, 6.0f}, {-31.0f, 0.0f}, {-26.0f, 0.0f}, {-16.0f, -5.0f}, {4.0f, -24.0f}},
 {4, {-65.0f, 12.0f}, {-41.0f, 0.0f}, {-21.0f, 0.0f}, {9.0f, -15.0f}},
 {5, {-55.0f, 12.0f}, {-31.0f, 0.0f}, {-26.0f, 0.0f}, {-16.0f, -5.0f}, {4.0f, -24.0f}},
 {5, {-50.0f, 15.0f}, {-31.0f, 0.0f}, {-26.0f, 0.0f}, {-16.0f, -5.0f}, {4.0f, -24.0f}}
 };
 */
#endif  /* MPEG_D_DRC_EXTENSION_V1 */

int
initTables(const int nGainValuesMax,
           Tables* tables)
{
    generateDeltaTimeCodeTable (nGainValuesMax,
                                tables->deltaTimeCodeTable);
    return(0);
}

int
removeTables(void)
{

    return(0);
}

const SlopeCodeTableEntry*
getSlopeCodeTableByValue(void)
{
    return(&(slopeCodeTableEntryByValue[0]));
}

void
getSlopeCodeTableAndSize(SlopeCodeTableEntry const** slopeCodeTableEntry,
                         int* nSlopeCodeTableEntries)
{
    *slopeCodeTableEntry = &(slopeCodeTableEntryBySize[0]);
    *nSlopeCodeTableEntries = kNumSlopeValuesTable;
}


void
getDeltaGainCodeTable(const int gainCodingProfile,
                      DeltaGainCodeEntry const** deltaGainCodeTable,
                      int *nEntries)
{
    if (gainCodingProfile==GAIN_CODING_PROFILE_CLIPPING)
    {
        *deltaGainCodeTable = deltaGainCodeTableProfile2BySize;
        *nEntries = kNumDeltaGainValuesTableProfile2;
    }
    else
    {
        *deltaGainCodeTable = deltaGainCodeTableBySize;
        *nEntries = kNumDeltaGainValuesTable;
    }
}

/*
 Generation of the delta time code table
 This table is not really needed because it would be more efficient to decode only the first few bits
 of the code (prefix) and then decode the remaining bits directly into the value with a linear relation: value = code + offset
*/

void
generateDeltaTimeCodeTable (const int nGainValuesMax,
                            DeltaTimeCodeTableEntry* deltaTimeCodeTableItem)
{
    int n, k;

    int Z = 1;
    while((1<<Z)<2*nGainValuesMax)
    {
      Z++;
    }

    /* first entry: not used. Fill with dummy values. */
    deltaTimeCodeTableItem[0].size = -1;
    deltaTimeCodeTableItem[0].code = -1;
    deltaTimeCodeTableItem[0].value = -1; 
    
    deltaTimeCodeTableItem[1].size = 2;
    deltaTimeCodeTableItem[1].code = 0x0;
    deltaTimeCodeTableItem[1].value = 1;
    for (n=0; n<4; n++)
    {
        deltaTimeCodeTableItem[n+2].size = 4;
        deltaTimeCodeTableItem[n+2].code = 0x4 + n;
        deltaTimeCodeTableItem[n+2].value = n+2;
    }
    for (n=0; n<8; n++)
    {
        deltaTimeCodeTableItem[n+6].size = 5;
        deltaTimeCodeTableItem[n+6].code = 0x10 + n;
        deltaTimeCodeTableItem[n+6].value = n+6;
    }

    /* Large timeDelta values are coded with wordlength 2+Z, where Z is only as short as it must be to represent all possible values */
    k = 2*nGainValuesMax-14+1;
    for (n=0; n<k; n++)
    {
        deltaTimeCodeTableItem[n+14].size = 2+Z;
        deltaTimeCodeTableItem[n+14].code = (0x3<<Z) + n;
        deltaTimeCodeTableItem[n+14].value = n+14;
    }

}

float
decodeSlopeIndex(const int slopeCodeIndex)
{
    const SlopeCodeTableEntry* slopeCodeTable = getSlopeCodeTableByValue();
    return slopeCodeTable[slopeCodeIndex].value;
}

float
decodeSlopeIndexMagnitude(const int slopeCodeIndex)
{
    const SlopeCodeTableEntry* slopeCodeTable = getSlopeCodeTableByValue();
    return (float)fabs((double)slopeCodeTable[slopeCodeIndex].value);
}

int
getDeltaTmin (const int sampleRate)
{
    int lowerBound = (int) (0.5f + 0.0005f * sampleRate);
    int result = 1;
    if (sampleRate < 1000)
    {
        fprintf(stderr, "Error: audio sample rates below 1000 Hz are not supported: %d.\n", sampleRate);
        return(UNEXPECTED_ERROR);
    }
    while (result <= lowerBound) result = result << 1;
    return result;
}
    
#ifdef __cplusplus
}
#endif /* __cplusplus */



