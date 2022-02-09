/**
 * @file test_mebx.cpp
 * @author Dimitri Podborski
 * @brief Multiplexed Metadata testing
 * @version 0.1
 * @date 2022-02-07
 * 
 * @copyright This software module was originally developed by Apple Computer, Inc. in the course of
 * development of MPEG-4. This software module is an implementation of a part of one or more MPEG-4
 * tools as specified by MPEG-4. ISO/IEC gives users of MPEG-4 free license to this software module
 * or modifications thereof for use in hardware or software products claiming conformance to MPEG-4.
 * Those intending to use this software module in hardware or software products are advised that its
 * use may infringe existing patents. The original developer of this software module and his/her
 * company, the subsequent editors and their companies, and ISO/IEC have no liability for use of
 * this software module or modifications thereof in an implementation. Copyright is not released for
 * non MPEG-4 conforming products. Apple Computer, Inc. retains full right to use the code for its
 * own purpose, assign or donate the code to a third party and to inhibit third parties from using
 * the code for non MPEG-4 conforming products. This copyright notice must be included in all copies
 * or derivative works. Copyright (c) 1999.
 *
 */

#include <catch.hpp>
#include <ISOMovies.h>
#include <MP4Atoms.h>

TEST_CASE("mebx")
{
  MP4Err err;

  SECTION("Open")
  {
    ISOMovie cMovieBox;
    err = MP4OpenMovieFile(&cMovieBox, "/Users/podborski/Library/CloudStorage/Box-Box/Dolby/files/iphone13_DV_cinematic.MOV", MP4OpenMovieDebug);
    REQUIRE(err == MP4NoErr);
  }
}
