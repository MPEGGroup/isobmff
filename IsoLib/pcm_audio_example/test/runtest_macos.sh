#!/bin/tcsh
setenv WAVTOOLS ../macosx/WAV_conversion/build/Debug
$WAVTOOLS/WAV_to_MP4 -i sine.wav -o sine.mp4 -d 1
$WAVTOOLS/MP4_to_WAV -i sine.mp4 -o sine_out.wav -d 1
$WAVTOOLS/CompareWAV -a sine.wav -b sine_out.wav -d 3
