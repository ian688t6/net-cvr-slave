#ifndef __H264_H__
#define __H264_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*done_cb)(uint8_t * pframe, int32_t len);

extern void h264_encode_begin(int32_t w, int32_t h);

extern void h264_encode(uint8_t *data, int32_t w, int32_t h, done_cb done);

extern void h264_encode_end(void);

#ifdef __cplusplus
}
#endif

#endif /* H264_H_ */
