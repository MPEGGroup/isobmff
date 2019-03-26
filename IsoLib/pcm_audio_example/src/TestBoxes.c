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
 @header TestBoxes
 Test application for PCM format related boxes.
 @copyright Apple, Inc.
 @updated 2014-09-10
 @author Apple, Inc.
 @version 1.0
 */

#include <stdio.h>
#include "Logger.h"
#include "Testing.h"
extern int  logLevel;
extern FILE *logOutput;

int main(int argc, const char * argv[]) {
    int err;
    err = 0;
    logLevel  = LOGLEVEL_ERROR;

    logOutput = stdout;
    printf("\n=========================================================\n");
    printf("ISO Base Media File Format Box Tester for ISO/IEC 23003-5\n");
    printf("PCM configuration box (ISO/IEC 23003-5) and\n");
    printf("Channel layout box (ISO/IEC 14496-12)\n");
    printf("Version 0.1 (2019)");
    printf("\n=========================================================\n");

    err = testAll(100000);
    if (err) {
        printf("ERROR occured\n");
    }
    else {
        printf("Test finished\n");
    }
    return 0;
}
