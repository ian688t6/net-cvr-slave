#include "os.h"
#include "config.h"
#include "msg.h"
#include "debug.h"
#include "do.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MSG_PORT		(9001)
#define MSG_FD_COUNT	(2)
static int msg_fd[MSG_FD_COUNT] = {0};

static void dumpmsg(msg_t *pmsg)
{
	if (NULL == pmsg)
	{
		return;
	}
	dbg_i("msg type: %d code %d arg0: %d arg1: %d datalen: %d\n",
			pmsg->header.type, pmsg->header.code, pmsg->header.arg0, pmsg->header.arg1, pmsg->header.datalen);

	return;
}

static int conn_check(int32_t id)
{
	msg_t msg;

	memset(&msg, 0x0, sizeof(msg_t));
	msg.header.type = MSG_TYPE_CMD;
	msg.header.code = MSG_CODE_CONN_CHECK;

	return msg_sendto(id, &msg);
}

#if 1
static int sock_create(int32_t *pfd, const char *addr, int port)
{
	int32_t fd;
	struct sockaddr_in server_addr;

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (0 > fd) {
		dbg_e("socket create fail\n");
		return GCOS_FAIL;
	}

    memset(&server_addr,0,sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if( inet_pton(AF_INET, addr, &server_addr.sin_addr) <=0 ) {
    	dbg_e("inet_pton error for %s\n",addr);
    	return GCOS_FAIL;
    }

    if(connect(fd,(struct sockaddr*)&server_addr,sizeof(server_addr))<0) {
    	dbg_e("connect sock error\n");
    	return GCOS_FAIL;
    }
    *pfd = fd;

	return GCOS_SUCC;
}
#else

#endif

static void sock_close(int fd)
{
	close(fd);
}

static void sock_destroy(void)
{
	int i = 0;

	for (i = 0; i < MSG_FD_COUNT; i ++)
	{
		sock_close(msg_fd[i]);
		msg_fd[i] = 0;
	}

	return;
}

static void bump_maxfd(int fd, int *max)
{
  if (fd > *max)
    *max = fd;
}

static void set_sock_listeners(fd_set *set, int *maxfdp)
{
	int i = 0;

	FD_ZERO(set);
	for (i = 0; i < MSG_FD_COUNT; i ++)
	{
	    FD_SET(msg_fd[i], set);
	    bump_maxfd(msg_fd[i], maxfdp);
	}

	return;
}

static int32_t msg_do(msg_t *pmsg)
{
	int32_t ret = GCOS_SUCC;

	if (0 < pmsg->header.datalen)
	{
		pmsg->payload = (uint8_t *)malloc(pmsg->header.datalen);
		if (!pmsg->payload)
		{
			dbg_e("msg payload alloc fail\n");
			return GCOS_FAIL;
		}
	}

	switch (pmsg->header.code)
	{
	case MSG_CODE_START_PREVIEW:
		ret = do_preview_start(pmsg);
		break;

	case MSG_CODE_STOP_PREVIEW:
		ret = do_preview_stop(pmsg);
		break;

	case MSG_CODE_START_RECORD:
		ret = do_record_start(pmsg);
		break;

	case MSG_CODE_STOP_RECORD:
		ret = do_record_stop(pmsg);
		break;

	case MSG_CODE_TAKE_PHOTO:
		ret = do_takephoto(pmsg);
		break;

	case MSG_CODE_PLAYBACK_VIDEO:
		ret = do_playback_video(pmsg);
		break;

	case MSG_CODE_PLAYBACK_VIDEO_PAUSE_PLAY:
		ret = do_playback_video_pause_play(pmsg);
		break;

	case MSG_CODE_PLAYBACK_VIDEO_SEEK:
		ret = do_playback_video_seek(pmsg);
		break;

	case MSG_CODE_PLAYBACK_VIDEO_STOP:
		ret = do_playback_video_stop(pmsg);
		break;

	case MSG_CODE_PLAYBACK_PHOTO:
		ret = do_playback_photo(pmsg);
		break;

	case MSG_CODE_GET_FILE_LIST:
		ret = do_file_list_get(pmsg);
		break;

	case MSG_CODE_DELETE_FILE:
		ret = do_file_delete(pmsg);
		break;

	case MSG_CODE_LOCK_FILE:
		ret = do_file_lock(pmsg);
		break;

	case MSG_CODE_UNLOCK_FILE:
		ret = do_file_unlock(pmsg);
		break;

	case MSG_CODE_FORMAT_SD:
		ret = do_format(pmsg);
		break;

	case MSG_CODE_SETTING:
		ret = do_setting(pmsg);
		break;

	case MSG_CODE_CONN_CHECK:

		break;

	default:
		break;
	}

	return ret;
}

static int32_t msg_done(int32_t *pfd, msg_t *pmsg)
{
	int32_t len = 0;
	int32_t ret = GCOS_SUCC;

	if (0 > *pfd)
	{
		dbg_e("msg done fail\n");
		return GCOS_FAIL;
	}

	len = send(*pfd, &pmsg->header, sizeof(msg_header_t), 0);
	if (len != sizeof(msg_header_t))
	{
		dbg_e("msg done send len: %d fail\n", len);
		return GCOS_FAIL;
	}
	if ((0 < pmsg->payload) && (NULL != pmsg->payload))
	{
		len = send(*pfd, pmsg->payload, pmsg->header.datalen, 0);
		if (len != pmsg->header.datalen)
		{
			dbg_e("msg done send len: %d fail\n", len);
			return GCOS_FAIL;
		}
		free(pmsg->payload);
	}
	*pfd = -1;

	if (NULL != pmsg->complete)
	{
		pmsg->complete(pmsg);
	}

	return ret;
}

static void msg_wait(fd_set *rset, int maxfd, msg_t *pmsg, int *pfd)
{
	int i		= 0;
	int i_ret	= 0;

    i_ret = select(maxfd+1, rset, 0, 0, 0);
    dbg_i("msg wait i_ret: %d\n", i_ret);

    for (i = 0; i < MSG_FD_COUNT; i ++)
    {
    	if (FD_ISSET(msg_fd[i], rset))
    	{
    		msg_recvfrom(i, (uint8_t *)pmsg, sizeof(msg_header_t));

    		dumpmsg(pmsg);

    		//Todo do msg
    		msg_do(pmsg);

    		*pfd = msg_fd[i];
    	}
    }

	return;
}

int32_t msg_recvfrom(int32_t id, uint8_t *buf, int32_t buflen)
{
	int32_t len = 0;
	int32_t ret = GCOS_SUCC;

	//Todo recv msg
	len = recv(msg_fd[id], buf, buflen, 0);
	if (len != buflen)
	{
		dbg_e("recv buf len = %d fail\n", len);
		return GCOS_FAIL;
	}

	return ret;
}

int32_t msg_sendto(int32_t id, msg_t *msg)
{
	int32_t len = 0;

	if (NULL == msg)
	{
		dbg_e("msg null");
		return GCOS_FAIL;
	}

	len = send(msg_fd[id], msg, sizeof(msg_t), 0);
	if (len != sizeof(msg_t))
	{
		dbg_e("send buf len = %d\n", len);
		return GCOS_FAIL;
	}

	if (msg->payload && (msg->header.datalen > 0))
	{
		len = send(msg_fd[id], msg->payload, msg->header.datalen, 0);
		if (len != msg->header.datalen)
		{
			dbg_e("send buf len = %d\n", len);
			return GCOS_FAIL;
		}
	}

	return (sizeof(msg_t) + msg->header.datalen);
}

int msg_conn_create(const char *addr)
{
	int i 		= 0;
	int ret 	= GCOS_SUCC;

	dbg_i("msg connection create\n");
	for (i = 0; i < MSG_FD_COUNT; i ++)
	{
		ret = sock_create(&msg_fd[i], addr, (MSG_PORT + i));
		if (GCOS_SUCC != ret)
		{
			dbg_e("socket create fail\n");
			return GCOS_FAIL;
		}
		conn_check(i);
		sleep(2);
	}

	return GCOS_SUCC;
}

void msg_conn_destroy(void)
{
	sock_destroy();
	return;
}

void msg_loop(void)
{
	int fd		= -1;
    int maxfd 	= -1;
    fd_set rset;
    msg_t msg;

	dbg_i("msg loop\n");

	for (;;)
	{
		//Todo set socket fd listeners
	    set_sock_listeners(&rset, &maxfd);

	    //Todo msg wait
	    memset(&msg, 0x0, sizeof(msg_t));
	    msg_wait(&rset, maxfd, &msg, &fd);

	    //Todo msg done
	    msg_done(&fd, &msg);
	}
	return;
}

#ifdef __cplusplus
}
#endif
