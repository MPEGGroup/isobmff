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

#include <stdio.h>
#include <string.h>
#include "wavIO.h"
#include "Logger.h"
#include "CompareWAVOptions.h"
#include "WAVData.h"

int main(int argc, const char * argv[]) {

    CompareWAVOptions   options;
    WAVData             wavData[N_WAV];
    int                 err;
    int                 i;
    int                 chunk;
    char*               buffer[N_WAV];
    size_t              elementsRead[N_WAV];
    unsigned long int   sampleCount[N_WAV];

    for (i=0; i<N_WAV; i++) {
        buffer[i] = NULL;
    }
    chunk = 1;
    logOutput   = stdout;

    setDefaultValues(&options);

    if (!parseArguments(argc, argv, &options))
    {
        logMsg(LOGLEVEL_ERROR, "Parsing options failed!");
        return -1;
    }

    if (options.isJustAskingForHelp) {
        return 0;
    };

    logLevel    = options.debugLevel;

    logMsg(LOGLEVEL_INFO, "CompareWAV started.");
    printOptions(&options);

    for (i=0; i<N_WAV; i++) {
        err = initWAVDataForReading(&wavData[i], options.inputFile[i]);    if (err) goto bail;
        buffer[i] = (char*) calloc(CHUNKSIZE * wavData[i].channels * wavData[i].byteDepth, 1);
        sampleCount[i] = 0;
        elementsRead[i] = 0;
    }

    if (wavData[WAV_A].channels != wavData[WAV_B].channels) {
        logMsg(LOGLEVEL_ERROR, "Channel count mismatch %d %d", wavData[WAV_A].channels, wavData[WAV_B].channels);
        err = -1;
        goto bail;
    }
    if (wavData[WAV_A].sampleRate != wavData[WAV_B].sampleRate) {
        logMsg(LOGLEVEL_ERROR, "Sample rate mismatch %d %d", wavData[WAV_A].sampleRate, wavData[WAV_B].sampleRate);
        err = -1;
        goto bail;
    }
    if (wavData[WAV_A].byteDepth != wavData[WAV_B].byteDepth) {
        logMsg(LOGLEVEL_ERROR, "ByteDepth mismatch %d %d", wavData[WAV_A].byteDepth, wavData[WAV_B].byteDepth);
        err = -1;
        goto bail;
    }
    if (wavData[WAV_A].totalSamplesPerChannel != wavData[WAV_B].totalSamplesPerChannel) {
        logMsg(LOGLEVEL_ERROR, "totalSamplesPerChannel mismatch %d %d", (int)wavData[WAV_A].totalSamplesPerChannel, (int)wavData[WAV_B].totalSamplesPerChannel);
        err = -1;
        goto bail;
    }
    if (wavData[WAV_A].formatTag != wavData[WAV_B].formatTag) {
        logMsg(LOGLEVEL_ERROR, "formatTag mismatch %d %d", wavData[WAV_A].formatTag, wavData[WAV_B].formatTag);
        err = -1;
        goto bail;
    }

    do {
        logMsg(LOGLEVEL_TRACE, "Comparing data chunk %d", chunk);
        for (i=0; i<N_WAV; i++) {
            elementsRead[i] = fread(buffer[i], wavData[i].channels * wavData[i].byteDepth, CHUNKSIZE, wavData[i].file);
            sampleCount[i] += elementsRead[i];
        }
        if (elementsRead[WAV_A] != elementsRead[WAV_B]) {
            logMsg(LOGLEVEL_ERROR, "WAV data size mismatch %d %d", (int)elementsRead[WAV_A], (int)elementsRead[WAV_B]);
            err = -1;
            goto bail;
        }

        logMsg(LOGLEVEL_TRACE, "Elements read %d %d", elementsRead[WAV_A], elementsRead[WAV_B]);
        err = memcmp(buffer[WAV_A], buffer[WAV_B], elementsRead[WAV_A] * wavData[WAV_A].channels * wavData[WAV_A].byteDepth);

        if (err) {
            logMsg(LOGLEVEL_ERROR, "WAV data mismatch");
            err = -1;
            goto bail;
        }
        chunk++;
    } while (elementsRead[WAV_A] > 0 && (elementsRead[WAV_B] > 0));

    if (sampleCount[WAV_A] != wavData[WAV_A].totalSamplesPerChannel) {
        logMsg(LOGLEVEL_ERROR, "Data size of WAV file A not matching header %d %d", wavData->byteDepth * wavData->channels * wavData->totalSamplesPerChannel, sampleCount[WAV_A]);
        err = -1;
        goto bail;
    }
    if (sampleCount[WAV_B] != wavData[WAV_B].totalSamplesPerChannel) {
        logMsg(LOGLEVEL_ERROR, "Data size of WAV file B not matching header B %d %d", wavData->byteDepth * wavData->channels * wavData->totalSamplesPerChannel, sampleCount[WAV_B]);
        err = -1;
        goto bail;
    }

    logMsg(LOGLEVEL_INFO, "WAVE file content matches");
bail:
    for (i=0; i<N_WAV; i++) {
        if (buffer[i]) {
            free(buffer[i]);
        }
        closeWAVDataFile    (&wavData[i]);
        freeWAVData         (&wavData[i]);
    }
    fflush(stdout);
    return err;
}
