#include "os.h"
#include "config.h"
#include "msg.h"
#include "debug.h"

#ifdef __cplusplus
extern "C" {
#endif

static uint32_t enable = 0;

int32_t do_record_start(msg_t *pmsg)
{
	int32_t ret = GCOS_SUCC;

	dbg_i("record start\n");
	enable = 1;

	return ret;
}

int32_t do_record_stop(msg_t *pmsg)
{
	int32_t ret = GCOS_SUCC;

	dbg_i("record stop\n");
	enable = 0;

	return ret;
}


#ifdef __cplusplus
}
#endif
