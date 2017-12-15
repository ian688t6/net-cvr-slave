#ifndef __DO_H__
#define __DO_H__
#ifdef __cplusplus
extern "C" {
#endif

extern int32_t do_preview_start(msg_t *pmsg);

extern int32_t do_preview_stop(msg_t *pmsg);

extern int32_t do_record_start(msg_t *pmsg);

extern int32_t do_record_stop(msg_t *pmsg);

extern int32_t do_takephoto(msg_t *pmsg);

extern int32_t do_playback_video(msg_t *pmsg);

extern int32_t do_playback_video_pause_play(msg_t *pmsg);

extern int32_t do_playback_video_seek(msg_t *pmsg);

extern int32_t do_playback_video_stop(msg_t *pmsg);

extern int32_t do_playback_photo(msg_t *pmsg);

extern int32_t do_file_list_get(msg_t *pmsg);

extern int32_t do_file_delete(msg_t *pmsg);

extern int32_t do_file_lock(msg_t *pmsg);

extern int32_t do_file_unlock(msg_t *pmsg);

extern int32_t do_format(msg_t *pmsg);

extern int32_t do_setting(msg_t *pmsg);

#ifdef __cplusplus
}
#endif
#endif /* DO_H_ */
