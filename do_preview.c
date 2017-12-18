#include "os.h"
#include "config.h"
#include "msg.h"
#include "debug.h"
#include "device.h"
#include "h264.h"

#ifdef __cplusplus
extern "C" {
#endif

static pthread_t pid = -1;

#ifdef DEBUG
static void dumpframe(uint8_t *pframe, int32_t len)
{
	int i = 0;

	for (i = 0; i < len; i += 8)
	{
		dbg_i("%02x %02x %02x %02x %02x %02x %02x %02x\n",
				pframe[i + 0], pframe[i + 1], pframe[i + 2], pframe[i + 3],
				pframe[i + 4], pframe[i + 5], pframe[i + 6], pframe[i + 7]);
	}

	return ;
}
#endif

static void cleanup(void *data)
{
	dbg_i("cleanup\n");
	/* Todo stop capturing */
	capture_stop();

	/* Todo h264 encode end */
	h264_encode_end();

	/* Todo uninit device */
	device_uninit();

	/* Todo close device */
	device_close();
}

static int32_t send_frame(uint8_t *pframe, int32_t len)
{
	msg_t msg;
	int32_t ret = 0;

	memset(&msg, 0x0, sizeof(msg_t));
	msg.header.code = MSG_CODE_START_PREVIEW;
	msg.header.type = MSG_TYPE_RESP;
	msg.header.datalen = len;
	msg.payload = pframe;

	ret = msg_sendto(0, &msg);
	if (ret == GCOS_FAIL)
	{
		dbg_e("send frame fail retlen: %d\n", ret);
		return GCOS_FAIL;
	}
	return GCOS_SUCC;
}

static void process_frame(unsigned char *pdata, int len)
{
	int32_t framelen 	= 0;
	uint8_t *pframe 	= (uint8_t *) malloc(sizeof(uint8_t) * PIXEL_WIDTH * PIXEL_HEIGHT * 1.5);
	if (NULL == pframe)
	{
		dbg_e("pframe alloc fail\n");
		return;
	}

	memset(pframe, 0x0, sizeof(uint8_t) * PIXEL_WIDTH * PIXEL_HEIGHT * 1.5);

	framelen = h264_encode(pdata, PIXEL_WIDTH, PIXEL_HEIGHT, pframe);
//	dumpframe(pframe, framelen);
	send_frame(pframe, framelen);
	free(pframe);
	usleep(15*1000);
//	dbg_i("time: %lldus\n", L2 - L1);
	return ;
}

static void *preview(void *arg)
{
	pthread_cleanup_push(cleanup, NULL);

	/* Todo open device */
	device_open("/dev/video0");

	/* Todo init device */
	device_init();

	/* Todo h264 encode begin */
	h264_encode_begin(PIXEL_WIDTH, PIXEL_HEIGHT);

	/* Todo start capturing */
	capture_start();

	/* Todo loop */
	capture_loop(process_frame, CAPTURE_LOOP);

	pthread_cleanup_pop(0);

	return NULL;
}

int32_t do_preview_start(msg_t *pmsg)
{
	int32_t ret = GCOS_SUCC;

	if (GCOS_SUCC != pthread_create(&pid ,NULL, preview, NULL))
	{
		return GCOS_FAIL;
	}

	return ret;
}

int32_t do_preview_stop(msg_t *pmsg)
{
	int32_t ret = GCOS_SUCC;

	pthread_cancel(pid);

	return ret;
}

#ifdef __cplusplus
}
#endif
