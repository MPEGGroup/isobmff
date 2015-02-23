# $Id: Makefile,v 1.2 1999/11/20 08:19:15 mc Exp $

VPATH = . ../src ../../src

CFLAGS = -I. -I../../libisomediafile/linux -I../../libisomediafile/src -O -g -Wall

CC = gcc

FavsSample_common_src = ../src/hevc.c ../src/tools.c
FavsSample_muxer_src = ../src/mux_main.c
FavsSample_demuxer_src = ../src/demux_main.c

FavsSample_obj := $(FavsSample_common_src:.c=.o)
FavsSample_muxer_obj := $(FavsSample_muxer_src:.c=.o)
FavsSample_demuxer_obj := $(FavsSample_demuxer_src:.c=.o)

libisomediafile = ../../libisomediafile/linux/libisomediafile/libisomediafile.a

all:	FavsSample_muxer FavsSample_demuxer

FavsSample_muxer:	$(FavsSample_muxer_obj) $(FavsSample_obj) $(libisomediafile)
	$(CC) -o hevc_muxer $(FavsSample_muxer_obj) $(FavsSample_obj) $(libisomediafile) -lm
	
FavsSample_demuxer:	$(FavsSample_demuxer_obj) $(FavsSample_obj) $(libisomediafile)
	$(CC) -o hevc_demuxer $(FavsSample_demuxer_obj) $(FavsSample_obj) $(libisomediafile) -lm

%.d: %.c
	$(SHELL) -ec '$(CC) -M $(CFLAGS) $< | sed '\''s/$*.o/& $@/g'\'' > $@'

#
# Include all dependency files
#
-include $(FavsSample_common_src:.c=.d)
-include $(FavsSample_muxer_src:.c=.d)
-include $(FavsSample_demuxer_src:.c=.d)
