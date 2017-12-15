#ifndef __DEVICE_H__
#define __DEVICE_H__

#ifdef __cplusplus
extern "C" {
#endif

#define PIXEL_WIDTH 	(640)
#define PIXEL_HEIGHT 	(480)

typedef enum
{
	IO_METHOD_READ,
	IO_METHOD_MMAP,
	IO_METHOD_USERPTR,
} io_method;

struct buffer
{
	void	*start;
	size_t  length;
};

typedef struct
{
	char devname[16];
	int32_t fd;
	io_method io_mode;
	int32_t n_buffers;
	struct buffer *buffers;
} device_t;

typedef void (*process_cb)(unsigned char * psrc, int32_t len);

extern void capture_loop(process_cb cb);

extern void capture_start(void);

extern void capture_stop(void);

extern void device_set_io_method(io_method m);

extern void device_open(char *devname);

extern void device_close(void);

extern void device_init(void);

extern void device_uninit(void);

#ifdef __cplusplus
}
#endif
#endif /* DEVICE_H_ */
