#ifndef ARC_GLOBAL_H
#define ARC_GLOBAL_H

#include <inttypes.h>
#include <stdint.h>
#include <arctan.h>

#define ARC_HANG for (;;) __asm__("hlt");

extern struct ARC_BootMeta *Arc_BootMeta;

#define ASSERT(cond) if (!(cond)) {					\
			printf("Assertion %s failed (%s:%d)\n", #cond, __FILE__, __LINE__); \
			for (;;); \
		     }
#define ALIGN(v, a) ((v + (a - 1)) & ~(a - 1))

#define max(a, b) \
        ({ __typeof__ (a) _a = (a); \
        __typeof__ (b) _b = (b); \
        _a > _b ? _a : _b; })

#define min(a, b) \
        ({ __typeof__ (a) _a = (a); \
        __typeof__ (b) _b = (b); \
        _a < _b ? _a : _b; })


#endif
