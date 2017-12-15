#include "os.h"
#include "config.h"
#include "debug.h"
#include "device.h"
#include "x264.h"
#include "h264.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
	x264_param_t 	param;
	x264_picture_t	picture;
	x264_nal_t		*nal;
	x264_t			*x264;
} h264_t;

static h264_t g_h264;

static void x264_setimg(x264_image_t *img, int32_t w, int32_t h, unsigned char *data)
{
	int32_t i = 0, j = 0;
	int32_t idx_y = 0, idx_u = 0, idx_v = 0;
	unsigned char *y 	= NULL;
	unsigned char *u 	= NULL;
	unsigned char *v 	= NULL;

	if (NULL == img)
	{
		return ;
	}

	y = img->plane[0];
	u = img->plane[1];
	v = img->plane[2];

	for (j = 0; j < h * 2; j ++)
	{
		for (i = 0;  i < w; i += 4)
		{
            *(y + (idx_y++)) = *(data + w * j + i);
            *(y + (idx_y++)) = *(data + w * j + i + 2);
            if(j % 2 == 0)
            {
                *(u + (idx_u++)) = *(data + w * j + i + 1);
                *(v + (idx_v++)) = *(data + w * j + i + 3);
            }
		}
	}

	return;
}

void h264_encode_begin(int32_t w, int32_t h)
{
	h264_t *ph264 = &g_h264;

	x264_param_default(&ph264->param);
	ph264->param.i_threads = X264_SYNC_LOOKAHEAD_AUTO;
	ph264->param.i_width = w;
	ph264->param.i_height = h;
	ph264->param.i_frame_total = 0;
	ph264->param.i_keyint_max = 10;
	ph264->param.rc.i_lookahead = 0;
	ph264->param.i_bframe = 5;
	ph264->param.b_open_gop = 0;
	ph264->param.i_bframe_pyramid = 0;
	ph264->param.i_bframe_adaptive = X264_B_ADAPT_TRELLIS;
	ph264->param.rc.i_bitrate =1024 * 10;
	ph264->param.i_fps_num =25;
	ph264->param.i_fps_den = 1;

	x264_param_apply_profile(&ph264->param, "baseline");
	if ((ph264->x264 = x264_encoder_open(&ph264->param)) == 0) {
		return;
	}
	/* Create a new pic */
	x264_picture_alloc(&ph264->picture, X264_CSP_I420, ph264->param.i_width,
			ph264->param.i_height);
	ph264->picture.img.i_csp = X264_CSP_I420;
	ph264->picture.img.i_plane = 3;

	return;
}

int32_t h264_encode(unsigned char *data, int w, int h, unsigned char *out)
{
	int32_t i_nal 		= 0;
	int32_t n_nal 		= 0;
	int32_t i_retlen 	= 0;
	int32_t frame_size 	= 0;
	h264_t *ph264 = &g_h264;
	x264_picture_t pic_out;

	x264_setimg(&ph264->picture.img, w, h, data);
	frame_size = x264_encoder_encode(ph264->x264, &ph264->nal, &n_nal, &ph264->picture, &pic_out);
	if (frame_size > 0)
	{
		for (i_nal = 0; i_nal < n_nal; i_nal ++)
		{
			memcpy(out, ph264->nal[i_nal].p_payload, ph264->nal[i_nal].i_payload);
			out += ph264->nal[i_nal].i_payload;
			i_retlen += ph264->nal[i_nal].i_payload;
		}
	}
//	dbg_i("h264 encode frame_size: %d n_nal: %d retlen: %d\n", frame_size, n_nal, i_retlen);

	return frame_size;
}

void h264_encode_end(void)
{
	h264_t *ph264 = &g_h264;

	x264_picture_clean(&ph264->picture);
	x264_encoder_close(ph264->x264);

	return;
}


#ifdef __cplusplus
}
#endif
