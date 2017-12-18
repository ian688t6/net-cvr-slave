#ifndef __DEVICE_H__
#define __DEVICE_H__

#ifdef __cplusplus
extern "C" {
#endif

#define PIXEL_WIDTH 	(640)
#define PIXEL_HEIGHT 	(480)

typedef enum
{
	CAPTURE_ONCE = 0,
	CAPTURE_LOOP,
} capture_method;

struct buffer
{
	void	*start;
	size_t  length;
};

typedef struct
{
	uint8_t	 *frame;
	uint32_t length;
} current_frame_t;

typedef struct
{
	char devname[16];
	int32_t fd;
	capture_method mode;
	int32_t n_buffers;
	current_frame_t current_frame;
	struct buffer *buffers;
} device_t;

typedef void (*process_cb)(unsigned char * psrc, int32_t len);

extern current_frame_t *capture_once(void);

extern void capture_loop(process_cb cb, capture_method);

extern void capture_start(void);

extern void capture_stop(void);

extern void device_open(char *devname);

extern void device_close(void);

extern void device_init(void);

extern void device_uninit(void);

#ifdef __cplusplus
}
#endif
#endif /* DEVICE_H_ */
