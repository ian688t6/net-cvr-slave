/*****************************************************************************
*  @file     debug.h
*  @brief    debug
*  Details.
*  @email    ian_yin@gcoreinc.com
*****************************************************************************/
#ifndef __DEBUG_H__
#define __DEBUG_H__
#ifdef __cplusplus
extern "C" {
#endif

#define LOG_INFO	"INFO:"
#define LOG_WARN	"WARN:"
#define LOG_ERR		"ERROR:"

#define dbg_i(fmt, ...) \
	printf(fmt, ##__VA_ARGS__)

#define dbg_w(fmt, ...) \
	printf(LOG_WARN "%s %d "fmt, __func__, __LINE__, ##__VA_ARGS__)

#define dbg_e(fmt, ...) \
	printf(LOG_ERR "%s %d "fmt, __func__, __LINE__, ##__VA_ARGS__)

#ifdef __cplusplus
}
#endif
#endif /* INCLUDE_DEBUG_H_ */
