# $Id: Makefile,v 1.2 2014/10/14 at Exp $

VPATH = . ../src

CFLAGS = -I. -I../src -I../../libisomediafile/linux -I../../libisomediafile/src -I../../Part04-Dynamic_Range_Control/trunk/tools/readonlybitbuf/include -I../../Part04-Dynamic_Range_Control/trunk/tools/writeonlybitbuf/include -I../../Part04-Dynamic_Range_Control/trunk/tools/wavIO/include -I../../Part04-Dynamic_Range_Control/trunk/modules/uniDrcModules/uniDrcBitstreamDecoderLib/include -I../../Part04-Dynamic_Range_Control/trunk/modules/uniDrcModules/uniDrcCommon -O -g -Wall -w -std=c99

CC = gcc

SOURCES = $(wildcard ../src/*.c)
OBJECTS = $(SOURCES:.c=.o)
DRC_to_MP4_OBJECTS = $(filter-out ../src/MP4_to_DRC.o,$(OBJECTS))
MP4_to_DRC_OBJECTS = $(filter-out ../src/DRC_to_MP4.o,$(OBJECTS))


libisomediafile = ../../libisomediafile/linux/libisomediafile/libisomediafile.a
libreadonlybitbuf = ../../Part04-Dynamic_Range_Control/trunk/tools/readonlybitbuf/lib/Linux_x86_64_O0/libreadonlybitbuf.a
libwriteonlybitbuf = ../../Part04-Dynamic_Range_Control/trunk/tools/writeonlybitbuf/lib/Linux_x86_64_D_O0/libwriteonlybitbuf.a
libwavIO = ../../Part04-Dynamic_Range_Control/trunk/tools/wavIO/lib/Linux_x86_64_O0/libwavIO.a
libuniDrcBitstreamDecoderLib = ../../Part04-Dynamic_Range_Control/trunk/modules/uniDrcModules/uniDrcBitstreamDecoderLib/lib/Linux_x86_64_D_O0/libuniDrcBitstreamDecoder.a


all:	MP4_to_DRC DRC_to_MP4

DRC_to_MP4:	$(DRC_to_MP4_OBJECTS) $(libisomediafile) $(libreadonlybitbuf) $(libwavIO) $(libuniDrcBitstreamDecoderLib)
	$(CC) $(CFLAGS) -o DRC_to_MP4 $(DRC_to_MP4_OBJECTS) $(libisomediafile) $(libwriteonlybitbuf) $(libreadonlybitbuf) $(libwavIO) $(libuniDrcBitstreamDecoderLib) -lm

MP4_to_DRC:	$(MP4_to_DRC_OBJECTS) $(libisomediafile) $(libwriteonlybitbuf) $(libwavIO) $(libuniDrcBitstreamDecoderLib)
	$(CC) $(CFLAGS) -o MP4_to_DRC $(MP4_to_DRC_OBJECTS) $(libisomediafile) $(libwriteonlybitbuf) $(libreadonlybitbuf) $(libwavIO) $(libuniDrcBitstreamDecoderLib) -lm

%.d: %.c
	$(SHELL) -ec '$(CC) -M $(CFLAGS) $< | sed '\''s/$*.o/& $@/g'\'' > $@'

