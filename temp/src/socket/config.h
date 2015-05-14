#ifndef _CONFIG_H_
#define _CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Defines
 */
#define DEBUG                   (1)
#define MAX_COMMANDS            (5)
#define BUTTONS_NB              (10)
#define ENCODERS_NB             (5)
#define ACTION_MAX_SIZE         (500)
#define LINE_MAX_SIZE           (500)

#define XBMCBUILTIN_CMD         "xbmcbuiltin_"
#define XBMCBUTTON_CMD          "KB_"
#define BC127_CMD               "BC127_"


#define SYSTEM_MODE_TOGGLE      "system_mode_toggle"

#if DEBUG
#define print                   printf
#else
#define print                   (void)
#endif

#ifdef __cplusplus
}
#endif

#endif /* _CONFIG_H_ */
