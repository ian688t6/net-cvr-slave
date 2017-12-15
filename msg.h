#ifndef MSG_H_
#define MSG_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef enum
{
	MSG_TYPE_CMD 	= 1,

	MSG_TYPE_RESP	= 2,
} msg_type_e;

typedef enum
{
	MSG_CODE_START_PREVIEW 				= 1,

	MSG_CODE_STOP_PREVIEW				= 2,

	MSG_CODE_START_RECORD				= 3,

	MSG_CODE_STOP_RECORD				= 4,

	MSG_CODE_TAKE_PHOTO					= 5,

	MSG_CODE_PLAYBACK_VIDEO				= 6,

	MSG_CODE_PLAYBACK_VIDEO_PAUSE_PLAY	= 7,

	MSG_CODE_PLAYBACK_VIDEO_SEEK		= 8,

	MSG_CODE_PLAYBACK_VIDEO_STOP		= 9,

	MSG_CODE_PLAYBACK_PHOTO				= 10,

	MSG_CODE_GET_FILE_LIST				= 11,

	MSG_CODE_FORMAT_SD					= 12,

	MSG_CODE_SETTING					= 13,

	MSG_CODE_DELETE_FILE				= 14,

	MSG_CODE_LOCK_FILE					= 15,

	MSG_CODE_UNLOCK_FILE				= 16,

	MSG_CODE_CONN_CHECK					= 17,
} msg_code_e;

typedef enum
{
	MSG_SAMPLE_INFO 	= 0,

	MSG_SAMPLE_VIDEO	= 1,

	MSG_SAMPLE_AUDIO	= 2,

	MSG_SAMPLE_END		= 3,
} msg_sample_type_e;

typedef enum
{
	MSG_RET_OK = 0,

	MSG_RET_INVAL,

	MSG_RET_TIMEOUT,

	MSG_RET_SD_ERR,

	MSG_RET_UNKNOWN_ERR,
} msg_ret_type_e;

typedef struct
{
	uint16_t type;
	uint16_t code;
	uint32_t arg0;
	uint32_t arg1;
	uint32_t datalen;
} msg_header_t;

typedef struct
{
	msg_header_t	header;
	uint8_t 		*payload;
	void (*complete)(void *pmsg);
} msg_t;

extern int32_t msg_sendto(int32_t id, msg_t *msg);

extern int32_t msg_recvfrom(int32_t id, uint8_t *buf, int32_t buflen);

extern void msg_loop(void);

extern int msg_conn_create(const char *addr);

extern void msg_conn_destroy(void);

#ifdef __cplusplus
}
#endif
#endif /* MSG_H_ */
