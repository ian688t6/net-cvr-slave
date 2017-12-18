#include "os.h"
#include "config.h"
#include "msg.h"
#include "debug.h"
#include "device.h"
#include "jpeg.h"

#ifdef __cplusplus
extern "C" {
#endif

static uint32_t picid = 0;

static void yuv2rgb(const unsigned char *im_yuv, unsigned char *dst)
{
    const int size = PIXEL_WIDTH * PIXEL_HEIGHT;
    unsigned char Y = 0;
    unsigned char U = 0;
    unsigned char V = 0;
    int B = 0;
    int G = 0;
    int R = 0;
    int i;
    for(i = 0; i < size; ++i){
        if(!(i & 1)){
            U = im_yuv[2 * i + 1];
            V = im_yuv[2 * i + 3];
        }
        Y = im_yuv[2 * i];
        B = Y + 1.773 * (U - 128);
        G = Y - 0.344 * (U - 128) - (0.714 * (V - 128));
        R = Y + 1.403 * (V - 128);
        if(B > UCHAR_MAX){
            B = UCHAR_MAX;
        }
        if(G > UCHAR_MAX){
            G = UCHAR_MAX;
        }
        if(R > UCHAR_MAX){
            R = UCHAR_MAX;
        }
        dst[3*i] = B;
        dst[3*i+1] = G;
        dst[3*i+2] = R;
    }
	return;
}

static void process_frame(unsigned char *pdata, int len)
{
	uint8_t *picbuf = NULL;
	char picname[32] = {0};

	if (NULL == pdata)
	{
		dbg_e("process_frame pdata null\n");
		return;
	}

	picbuf = (uint8_t *)malloc(PIXEL_WIDTH * PIXEL_HEIGHT * 3);
	if (NULL == picbuf)
	{
		dbg_e("alloc picbuf fail\n");
		return;
	}
	memset(picbuf, 0x0, sizeof(PIXEL_WIDTH * PIXEL_HEIGHT * 3));

	yuv2rgb((const uint8_t *)pdata, picbuf);
	snprintf(picname, sizeof(picname), "img%d.jpg", picid ++);
	jpeg_create(picname, picbuf, PIXEL_WIDTH, PIXEL_HEIGHT);

	free(picbuf);

	return;
}

int32_t do_takephoto(msg_t *pmsg)
{
	int32_t ret = GCOS_SUCC;
	current_frame_t *pframe = NULL;

	pframe = capture_once();
	if (pframe->frame)
	{
		dbg_e("capture photo once fail\n");
		return GCOS_FAIL;
	}
	process_frame(pframe->frame, pframe->length);

	return ret;
}

#ifdef __cplusplus
}
#endif
