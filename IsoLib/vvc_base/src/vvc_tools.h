#ifndef INCLUDED_FAVS_TOOLS_H
#define INCLUDED_FAVS_TOOLS_H

#define GET16(buffer) \
	((((u8*) buffer)[0] << 8) + ((u8*) buffer)[1])

#define GET32(buffer) \
	((((u8*)buffer)[0] << 24) + (((u8*)buffer)[1] << 16) + (((u8*)buffer)[2] << 8) + (((u8*)buffer)[3]))

#define PUT32(buffer, val ) \
	((u8*) buffer)[0] = (u8) (((val) >> 24) & 0xff); \
	((u8*) buffer)[1] = (u8) (((val) >> 16) & 0xff); \
	((u8*) buffer)[2] = (u8) (((val) >> 8) & 0xff); \
	((u8*) buffer)[3] = (u8) ((val)& 0xff); \

#define PUT16(buffer, val ) \
	((u8*) buffer)[0] = (u8) (((val) >> 8) & 0xff); \
	((u8*) buffer)[1] = (u8) ((val) & 0xff);

#define PUT8(buffer, val)                       \
  ((u8 *)buffer)[0] = (u8)((val)&0xff);

#define PUT32(buffer, val ) \
	((u8*) buffer)[0] = (u8) (((val) >> 24) & 0xff); \
	((u8*) buffer)[1] = (u8) (((val) >> 16) & 0xff); \
	((u8*) buffer)[2] = (u8) (((val) >> 8) & 0xff); \
	((u8*) buffer)[3] = (u8) ((val)& 0xff); \


MP4Err BitBuffer_Init(BitBuffer *bb, u8 *p, u32 length);

u32 ceil_log2(u32 x);

u32 GetBits(BitBuffer *bb, u32 nBits, MP4Err *errout);

MP4Err GetBytes(BitBuffer *bb, u32 nBytes, u8 *p);

u32 read_golomb_uev(BitBuffer *bb, MP4Err *errout);

int parseInput(int argc, char* argv[], struct ParamStruct *parameters);

#endif
