#ifndef _COMMON_H_
#define _COMMON_H_

#include "config.h"
#include <rpc/types.h>

#ifdef __cplusplus
extern "C" {
#endif



/*
 * Defines
 */
#define LEFT                (0)
#define RIGHT               (1)

#ifndef __int8_t_defined
# define __int8_t_defined
typedef signed char     int8_t;
typedef short int       int16_t;
typedef int         int32_t;
# if __WORDSIZE == 64
typedef long int        int64_t;
# else
__extension__
typedef long long int       int64_t;
# endif
#endif

/* Unsigned.  */
typedef unsigned char       uint8_t;
typedef unsigned short int  uint16_t;
#ifndef __uint32_t_defined
typedef unsigned int        uint32_t;
# define __uint32_t_defined
#endif
#if __WORDSIZE == 64
typedef unsigned long int   uint64_t;
#else
__extension__
typedef unsigned long long int  uint64_t;
#endif

/*
 * Typedefs
 */
typedef enum systemModes_tag {
    mModeBC127 = 0x00,
    mModeXBMC = 0x01
} systemModes_t;


#ifdef __cplusplus
}
#endif

#endif /* _COMMON_H_ */
