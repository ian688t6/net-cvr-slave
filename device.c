#include "os.h"
#include "config.h"
#include "debug.h"
#include "device.h"

#ifdef __cplusplus
extern "C" {
#endif

static device_t gdev;

static int32_t xioctl(int32_t fd, int32_t request, void *arg)
{
	int32_t r;

	do r = ioctl(fd, request, arg);
	while ((-1 == r) && (EINTR == errno));

	return r;
}

//static void process_image(const void *p)
//{
//	fputc('.', stdout);
//	fflush(stdout);
//	return;
//}

static int32_t read_frame(process_cb cb)
{
	device_t *pdev = &gdev;
	struct v4l2_buffer buf;

	switch (pdev->io_mode)
	{
	case IO_METHOD_MMAP:
		memset(&buf, 0x0, sizeof(struct v4l2_buffer));
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		if (-1 == xioctl (pdev->fd, VIDIOC_DQBUF, &buf)) {
			switch(errno)
			{
			case EAGAIN:
			case EIO:
				return GCOS_FAIL;
				break;

			default:
				dbg_e("VIDIOC_DQBUF");
				return GCOS_FAIL;
				break;
			}
		}
		assert (buf.index < pdev->n_buffers);
		//process_image (pdev->buffers[buf.index].start);
		cb(pdev->buffers[buf.index].start, pdev->buffers[buf.index].length);

		if (-1 == xioctl (pdev->fd, VIDIOC_QBUF, &buf)) {
			dbg_e ("VIDIOC_QBUF");
			return GCOS_FAIL;
		}
		break;

	case IO_METHOD_READ:
	case IO_METHOD_USERPTR:
		break;

	default:
		break;
	}

	return GCOS_SUCC;
}

static void init_mmap(void)
{
	device_t *pdev = &gdev;
	struct v4l2_requestbuffers req;

	memset(&req, 0x0, sizeof(struct v4l2_requestbuffers));
	req.count               = 4;
	req.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory              = V4L2_MEMORY_MMAP;

	if (-1 == xioctl (pdev->fd, VIDIOC_REQBUFS, &req))
	{
		if (EINVAL == errno)
		{
			dbg_e ("%s does not support " "memory mapping\n", pdev->devname);
			return ;
		}
		else
		{
			dbg_e ("VIDIOC_REQBUFS");
		}
	}

	if (req.count < 2) {
		dbg_e ("Insufficient buffer memory on %s\n", pdev->devname);
		return ;
	}

	pdev->buffers = calloc (req.count, sizeof (struct buffer));
	if (!pdev->buffers)
	{
		dbg_e ("Out of memory\n");
		return ;
	}

	for (pdev->n_buffers = 0; pdev->n_buffers < req.count; ++pdev->n_buffers) {
		struct v4l2_buffer buf;
		memset (&buf, 0x0, sizeof(struct v4l2_buffer));
		buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory      = V4L2_MEMORY_MMAP;
		buf.index       = pdev->n_buffers;
		if (-1 == xioctl (pdev->fd, VIDIOC_QUERYBUF, &buf)) {
			dbg_e ("VIDIOC_QUERYBUF");
			return ;
		}
		pdev->buffers[pdev->n_buffers].length 	= buf.length;
		pdev->buffers[pdev->n_buffers].start 	= mmap(NULL/*start anywhere*/, buf.length,
				PROT_READ | PROT_WRITE /* required */,
				MAP_SHARED /* recommended */,
				pdev->fd, buf.m.offset);
		if (MAP_FAILED == pdev->buffers[pdev->n_buffers].start) {
			dbg_e ("mmap");
			return;
		}
	}

	return;
}

void capture_loop(process_cb cb)
{
	device_t *pdev = &gdev;

	for (;;) {
		fd_set fds;
		struct timeval tv;
		int r;
		FD_ZERO (&fds);
		FD_SET (pdev->fd, &fds);

		tv.tv_sec = 2;
		tv.tv_usec = 0;
		r = select (pdev->fd + 1, &fds, NULL, NULL, &tv);

		if (-1 == r) {
			if (EINTR == errno) {
				dbg_i("capture_loop EINTR\n");
				continue;
			}
			dbg_e ("select");
			return ;
		}
		if (0 == r) {
			dbg_e ("select timeout\n");
			return ;
		}

		if (read_frame (cb))
			break;
	}

	return;
}

void capture_start(void)
{
	uint32_t i;
	enum v4l2_buf_type type;
	device_t *pdev = &gdev;

	switch(pdev->io_mode)
	{
	case IO_METHOD_MMAP:
		for (i = 0; i < pdev->n_buffers; ++i) {
			struct v4l2_buffer buf;
			memset (&buf, 0x0, sizeof(struct v4l2_buffer));
			buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			buf.memory      = V4L2_MEMORY_MMAP;
			buf.index       = i;

			if (-1 == xioctl (pdev->fd, VIDIOC_QBUF, &buf)) {
				dbg_e ("VIDIOC_QBUF");
				return ;
			}
		}

		type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		if (-1 == xioctl (pdev->fd, VIDIOC_STREAMON, &type)) {
			dbg_e ("VIDIOC_STREAMON");
			return;
		}

		break;

	case IO_METHOD_READ:
	case IO_METHOD_USERPTR:
		break;
	}

	return;
}

void capture_stop(void)
{
	enum v4l2_buf_type type;
	device_t *pdev = &gdev;

	switch(pdev->io_mode)
	{
	case IO_METHOD_READ:
	case IO_METHOD_USERPTR:
		/* Nothing to do. */
		break;

	case IO_METHOD_MMAP:
		type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		if (-1 == xioctl (pdev->fd, VIDIOC_STREAMOFF, &type))
			dbg_e ("VIDIOC_STREAMOFF");
		break;

	default:
		break;
	}

	return;
}

void device_set_io_method(io_method m)
{
	gdev.io_mode = m;
	return;
}

void device_open(char *devname)
{
	struct stat st;

	if (NULL == devname)
		return;

	strncpy(gdev.devname, devname, 16);
	if (-1 == stat(gdev.devname, &st))
	{
		dbg_e("Cannot identify '%s': %d, %s\n",
				gdev.devname, errno, strerror(errno));
		return ;
	}

	if (!S_ISCHR(st.st_mode))
	{
		dbg_e("%s is no device\n", gdev.devname);
		return ;
	}

	gdev.fd = open(gdev.devname, O_RDWR | O_NONBLOCK, 0);
	if (-1 == gdev.fd)
	{
		dbg_e("Cannot open '%s': %d, %s\n", gdev.devname, errno, strerror(errno));
		return;
	}

	return;
}

void device_init(void)
{
	struct v4l2_capability cap;
	struct v4l2_cropcap cropcap;
	struct v4l2_crop crop;
	struct v4l2_format fmt;
	uint32_t min;
	device_t *pdev = &gdev;

	pdev->n_buffers = 0;
	if (-1 == xioctl (pdev->fd, VIDIOC_QUERYCAP, &cap))
	{
		if (EINVAL == errno)
		{
			dbg_e ("%s is no V4L2 device\n",pdev->devname);
			return;
		}
		else
		{
			dbg_e ("VIDIOC_QUERYCAP");
			return;
		}
	}

	if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE))
	{
		dbg_e ("%s is no video capture device\n", pdev->devname);
		return;
	}

	switch (pdev->io_mode)
	{
	case IO_METHOD_READ:
		if (!(cap.capabilities & V4L2_CAP_READWRITE)) {
			dbg_e ("%s does not support read i/o\n", pdev->devname);
			return ;
		}
		break;
	case IO_METHOD_MMAP:
	case IO_METHOD_USERPTR:
		if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
			dbg_e ("%s does not support streaming i/o\n", pdev->devname);
			return ;
		}
		break;

	default:
		break;
	}

	memset (&cropcap, 0x0, sizeof(struct v4l2_cropcap));
	cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if (0 == xioctl (pdev->fd, VIDIOC_CROPCAP, &cropcap))
	{
		crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		crop.c = cropcap.defrect;
		/* reset to default */
		if (-1 == xioctl (pdev->fd, VIDIOC_S_CROP, &crop))
		{
			switch (errno) {
			case EINVAL:
				/*Cropping not supported.*/
				break;
			default:
				/*Errors ignored.*/
				break;
			}
		}
	}
	else
	{
		/*Errors ignored.*/
	}

	memset (&fmt, 0x0, sizeof(struct v4l2_format));
	fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.width       = PIXEL_WIDTH;
	fmt.fmt.pix.height      = PIXEL_HEIGHT;
	fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
	fmt.fmt.pix.field       = V4L2_FIELD_NONE;
	if (-1 == xioctl (pdev->fd, VIDIOC_S_FMT, &fmt)) {
		dbg_e ("VIDIOC_S_FMT");
		return ;
	}

	min = fmt.fmt.pix.width *2;
	if (fmt.fmt.pix.bytesperline <min)
		fmt.fmt.pix.bytesperline = min;
	min = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;
	if (fmt.fmt.pix.sizeimage < min)
		fmt.fmt.pix.sizeimage = min;

	switch (pdev->io_mode)
	{
	case IO_METHOD_MMAP:
		init_mmap ();
		break;

	case IO_METHOD_READ:
	case IO_METHOD_USERPTR:
		break;

	default:
		break;
	}

	return ;
}

void device_close(void)
{
	if (-1 == close(gdev.fd))
	{
		dbg_e ("close");
	}
	gdev.fd = -1;
}

void device_uninit(void)
{
	uint32_t i = 0;
	device_t *pdev = &gdev;

	switch(pdev->io_mode)
	{
	case IO_METHOD_MMAP:
		for (i = 0; i < pdev->n_buffers; ++i)
			if (-1 == munmap (pdev->buffers[i].start, pdev->buffers[i].length)) {
				dbg_e ("munmap");
				return ;
			}
		break;

	case IO_METHOD_READ:
	case IO_METHOD_USERPTR:

		break;
	}

	free(pdev->buffers);

	return;
}


#ifdef __cplusplus
}
#endif
