#include "os.h"
#include "debug.h"
#include "config.h"
#include "msg.h"

#ifdef __cplusplus
extern "C" {
#endif

int main(int argc, char *argv[])
{
	int32_t ret = GCOS_SUCC;
	char *addr = argv[1];

	/* Todo msg connection create */
	ret = msg_conn_create(addr);
	if (GCOS_SUCC != ret)
	{
		dbg_e("msg connection create fail\n");
		return GCOS_FAIL;
	}
	/* Todo msg loop */
	msg_loop();

	/* Todo msg connection destroy */
	msg_conn_destroy();

	return 0;
}

#ifdef __cplusplus
}
#endif
