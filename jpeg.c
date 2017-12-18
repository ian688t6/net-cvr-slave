#include "os.h"
#include "config.h"
#include "debug.h"
#include "jpeg.h"
#include "jpeglib.h"

#ifdef __cplusplus
extern "C" {
#endif

void jpeg_create(char *name, unsigned char *pdata, int w, int h)
{
	int32_t depth 		= 3;
	int32_t row_stride 	= 0;
	FILE 	*fp 		= NULL;
	struct jpeg_error_mgr jerr;
	struct jpeg_compress_struct jinfo;
	JSAMPROW row[1];

	jinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&jinfo);
	fp = fopen(name, "wb");
	if (NULL == fp)
	{
		dbg_e("jpg file open fail\n");
		return;
	}

	jpeg_stdio_dest(&jinfo, fp);
	jinfo.image_width = w;
	jinfo.image_height = h;
	jinfo.input_components = depth;
	jinfo.in_color_space = JCS_RGB;
	jpeg_set_defaults(&jinfo);
	jpeg_set_quality(&jinfo, 100, TRUE);
	jpeg_start_compress(&jinfo, TRUE);

    row_stride = w * 3;
    while (jinfo.next_scanline < jinfo.image_height)
    {
		row[0] = (JSAMPROW)(pdata + jinfo.next_scanline * row_stride);
		jpeg_write_scanlines(&jinfo, row, 1);
	}

    jpeg_finish_compress(&jinfo);
    jpeg_destroy_compress(&jinfo);
    fclose(fp);

	return;
}

#ifdef __cplusplus
}
#endif
