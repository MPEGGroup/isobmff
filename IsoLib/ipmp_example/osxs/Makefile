VPATH = . ../src

CFLAGS = -I. -I../src -I../../libisomediafile/osxs -I../../libisomediafile/src -O -g -Wall -ansi -pedantic -Wno-long-long

CC = cc

protectAudioMovie_src = protectAudioMovie.c

protectAudioMovie_obj := $(protectAudioMovie_src:.c=.o)

libisomediafile = ../../libisomediafile/osxs/libisomediafile/libisomediafile.a

all:	protectAudioMovie

protectAudioMovie:	$(protectAudioMovie_obj) $(libisomediafile)
	$(CC) -o protectAudioMovie $(protectAudioMovie_obj) $(libisomediafile)

%.d: %.c
	$(SHELL) -ec '$(CC) -M $(CFLAGS) $< | sed '\''s/$*.o/& $@/g'\'' > $@'

clean:
	-rm $(protectAudioMovie_obj) $(protectAudioMovie_src:.c=.d) protectAudioMovie

#
# Include all dependency files
#
-include $(protectAudioMovie_src:.c=.d)
