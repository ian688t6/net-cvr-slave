#include "os.h"
#include "config.h"
#include "msg.h"
#include "debug.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
	int32_t recorder_time;
	int32_t voice;
} setting_t;

static void dumpsetting(setting_t *psetting)
{
	dbg_i("record time: %d voice: %d\n", psetting->recorder_time, psetting->voice);
	return;
}

int32_t do_setting(msg_t *pmsg)
{
	int32_t ret = GCOS_SUCC;
	setting_t *psetting;

	if ((0 < pmsg->header.datalen) && (pmsg->payload))
	{
		msg_recvfrom(0, pmsg->payload, pmsg->header.datalen);
		psetting = (setting_t *)pmsg->payload;
		dumpsetting(psetting);
	}

	return ret;
}


#ifdef __cplusplus
}
#endif
