
### makefile for static library uniDrcBitstreamDecoderLib

GENERALROOT=../../../../
MAKEROOT=../..

-include $(GENERALROOT)/general/makefile.all

ifeq "$(MAKELEVEL)" "0"

ifeq "$(MPEGH)" "1"
DEFFLAGS += -DMPEG_H
endif

ifeq "$(AMD1)" "1"
DEFFLAGS += -DAMD1
endif

SRCS += uniDrcBitstreamDecoder.c uniDrcParser.c uniDrcTables.c

SRCDIRS += ../src ../../uniDrcCommon

INCL += -I../include -I../../uniDrcCommon

# readonlybitbuf includes
INCL += -I../../../../tools/readonlybitbuf/include

COMLIBS += -lreadonlybitbuf

OPTIONS = SRCDIRS="$(SRCDIRS)" INCL="$(INCL)" INCLDEP=1 DEFFLAGS="$(DEFFLAGS)"

# create one absolute static archive library
OPTIONS += SHAREDLIB=-1

all: $(GENERALROOT)/general/makefile.all libuniDrcBitstreamDecoder$(LIB_SUFFIX)

libuniDrcBitstreamDecoder$(LIB_SUFFIX):
	@$(MAKE) TARGET=$@ $(OPTIONS) SRCS="$(SRCS)" COMLIBS="$(COMLIBS)" library

clean:
	@$(MAKE) TARGET=libuniDrcBitstreamDecoder$(LIB_SUFFIX) SRCS="$(SRCS)" clean

clean_all:
	@$(MAKE) TARGET=libuniDrcBitstreamDecoder$(LIB_SUFFIX) SRCS="$(SRCS)" COMLIBS="$(COMLIBS)" clean


endif
