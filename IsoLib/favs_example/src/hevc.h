#ifndef INCLUDED_FAVS_HEVC_H
#define INCLUDED_FAVS_HEVC_H

#define SLICE_B 0
#define SLICE_P 1
#define SLICE_I 2

#define AGGREGATOR_NAL_TYPE 48


MP4Err hevc_parse_sps_minimal(BitBuffer *bb, struct hevc_sps* sps);
MP4Err hevc_parse_pps_minimal(BitBuffer *bb, struct hevc_pps* pps);
MP4Err hevc_parse_slice_header_minimal(BitBuffer *bb, struct hevc_poc* poc, struct hevc_slice_header* header,
struct hevc_sps* sps, struct hevc_pps* pps);
u8* stripNALEmulation(u8* buffer, u32* bufferLen);
int parseHEVCNal(FILE* input, u8** data, int* data_len);
ISOErr analyze_hevc_stream(FILE* input, struct hevc_stream* stream);

#endif
