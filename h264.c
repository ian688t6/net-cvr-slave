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

static void convert(x264_image_t *img, int32_t w, int32_t h, unsigned char *pdata)
{
    int32_t is_u = 1;
	int32_t i = 0, j = 0;
    int32_t idx_y = 0, idx_u = 0, idx_v = 0;
	uint8_t *y 	= NULL, *u = NULL, *v = NULL;

	if (NULL == img)
	{
		return ;
	}

	y = img->plane[0];
	u = img->plane[1];
	v = img->plane[2];

	for(i = 0; i < (2 * w * h); i += 2)
	{
		*(y+ idx_y) = *(pdata + i);
		idx_y++;
	}

	for(i = 0; i < h; i += 2)
	{
		for(j= (i* w * 2 + 1); j < (i * w * 2+ w * 2); j += 2)
		{
			if(is_u)
			{
				*(u + idx_u) = *(pdata+j);
				idx_u ++;
				is_u = 0;
			}
			else
			{
				*(v + idx_v) = *(pdata+j);
				idx_v ++;
				is_u = 1;
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
	ph264->param.rc.i_lookahead = 0;
	ph264->param.i_fps_num = 30;
	ph264->param.i_fps_den = 1;
	ph264->param.i_csp = X264_CSP_I420;
//	ph264->param.i_keyint_max = X264_KEYINT_MAX_INFINITE;
//	ph264->param.i_frame_total = 0;
//	ph264->param.i_bframe = 5;
//	ph264->param.b_open_gop = 0;
//	ph264->param.i_bframe_pyramid = 0;
//	ph264->param.i_bframe_adaptive = X264_B_ADAPT_TRELLIS;
//	ph264->param.rc.i_bitrate =1024 * 10;

	x264_param_apply_profile(&ph264->param, x264_profile_names[0]);
	if ((ph264->x264 = x264_encoder_open(&ph264->param)) == 0) {
		return;
	}
	/* Create a new pic */
	x264_picture_alloc(&ph264->picture, ph264->param.i_csp, ph264->param.i_width,
			ph264->param.i_height);
	ph264->picture.img.i_csp 	= ph264->param.i_csp;
	ph264->picture.img.i_plane 	= 3;
	ph264->picture.i_pts = 0;

	return;
}

void h264_encode(uint8_t *data, int32_t w, int32_t h, done_cb done)
{
	int32_t nal 		= 0;
	int32_t nalcnt 		= 0;
	int32_t retlen 		= 0;
	int32_t frame_size 	= 0;
	uint8_t *pout		= NULL;
	uint8_t *pframe		= NULL;
	h264_t *ph264 = &g_h264;
	x264_picture_t pic_out;

	if ((NULL == data) || (NULL == done))
		return;

	convert(&ph264->picture.img, w, h, data);

	ph264->picture.i_pts ++;
	frame_size = x264_encoder_encode(ph264->x264, &ph264->nal, &nalcnt, &ph264->picture, &pic_out);
	if (0 >= frame_size)
	{
		dbg_w("frame_size negative or zero %d\n", frame_size);
		return ;
	}
	pframe = (uint8_t *)malloc(frame_size);
	if (NULL == pframe)
	{
		dbg_e("out frame alloc fail\n");
		return;
	}
	pout = pframe;
	memset(pout, 0x0, frame_size);
	for (nal = 0; nal < nalcnt; nal ++)
	{
		memcpy(pout, ph264->nal[nal].p_payload, ph264->nal[nal].i_payload);
		pout += ph264->nal[nal].i_payload;
		retlen += ph264->nal[nal].i_payload;
	}

	done(pframe, retlen);
	free(pframe);
//	dbg_i("h264 encode frame_size: %d n_nal: %d retlen: %d\n", frame_size, n_nal, i_retlen);

	return ;
}

void h264_encode_end(void)
{
	h264_t *ph264 = &g_h264;

	x264_picture_clean(&ph264->picture);
	x264_encoder_close(ph264->x264);
	memset(ph264, 0x0, sizeof(h264_t));

	return;
}


#ifdef __cplusplus
}
#endif
