# $Id: Makefile,v 1.2 1999/11/20 08:19:15 mc Exp $

VPATH = . ../src ../../src

CFLAGS = -I. -I../../libisomediafile/linux -I../../libisomediafile/src -O -g -Wall

CC = gcc

FavsSample_src = ../src/main.c ../src/hevc.c ../src/tools.c

FavsSample_obj := $(FavsSample_src:.c=.o)

libisomediafile = ../../libisomediafile/linux/libisomediafile/libisomediafile.a

all:	FavsSample

FavsSample:	$(FavsSample_obj) $(libisomediafile)
	$(CC) -o favssample $(FavsSample_obj) $(libisomediafile) -lm

%.d: %.c
	$(SHELL) -ec '$(CC) -M $(CFLAGS) $< | sed '\''s/$*.o/& $@/g'\'' > $@'

#
# Include all dependency files
#
-include $(FavsSample_src:.c=.d)
