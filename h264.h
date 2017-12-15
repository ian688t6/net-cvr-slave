#ifndef __H264_H__
#define __H264_H__

#ifdef __cplusplus
extern "C" {
#endif

extern void h264_encode_begin(int32_t w, int32_t h);

extern int32_t h264_encode(unsigned char *data, int w, int h, unsigned char *out);

extern void h264_encode_end(void);

#ifdef __cplusplus
}
#endif

#endif /* H264_H_ */
