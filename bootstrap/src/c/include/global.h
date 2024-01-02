#ifndef ARC_GLOBAL_H
#define ARC_GLOBAL_H

#include <stdint.h>
#include <stddef.h>
#include <inttypes.h>

#ifdef ARC_DEBUG_ENABLE

#define ARC_DEBUG_NAME_STR "[BOOTSTRAP "__FILE__"]"
#define ARC_DEBUG_NAME_SEP_STR " : "
#define ARC_DEBUG_INFO_STR "[INFO]"
#define ARC_DEBUG_WARN_STR "[WARNING]"
#define ARC_DEBUG_ERR_STR  "[ERROR]"

#define ARC_DEBUG(__level__, __params__) ARC_DEBUG_##__level__(__params__)
#define ARC_DEBUG_INFO(__params__) printf(ARC_DEBUG_INFO_STR ARC_DEBUG_NAME_STR ARC_DEBUG_NAME_SEP_STR __params__);
#define ARC_DEBUG_WARN(__params__) printf(ARC_DEBUG_WARN_STR ARC_DEBUG_NAME_STR ARC_DEBUG_NAME_SEP_STR __params__);
#define ARC_DEBUG_ERR(__params__)  printf(ARC_DEBUG_ERR_STR  ARC_DEBUG_NAME_STR ARC_DEBUG_NAME_SEP_STR __params__);

#else

#define ARC_DEBUG(__level, __params__) ;

#endif // ARC_DEBUG_ENABLE

#endif
