#ifndef ARC_GLOBAL_H
#define ARC_GLOBAL_H

#include "multiboot/multiboot2.h"
#include <arctan.h>
#include <mm/freelist.h>
#include <stdint.h>
#include <stddef.h>
#include <inttypes.h>

#define ARC_TERM_WIDTH 80
#define ARC_TERM_HEIGHT 25
#define ARC_TERM_CHARW 8
#define ARC_TERM_CHARH 8

#define ARC_HANG for (;;) __asm__("hlt");

#ifdef ARC_DEBUG_ENABLE

#include <interface/printf.h>

#define ARC_DEBUG_NAME_STR "[BOOTSTRAP "__FILE__"]"
#define ARC_DEBUG_NAME_SEP_STR " : "
#define ARC_DEBUG_INFO_STR "[INFO]"
#define ARC_DEBUG_WARN_STR "[WARNING]"
#define ARC_DEBUG_ERR_STR  "[ERROR]"

#define ARC_DEBUG(__level__, ...) ARC_DEBUG_##__level__(__VA_ARGS__)
#define ARC_DEBUG_INFO(...) printf(ARC_DEBUG_INFO_STR ARC_DEBUG_NAME_STR ARC_DEBUG_NAME_SEP_STR __VA_ARGS__);
#define ARC_DEBUG_WARN(...) printf(ARC_DEBUG_WARN_STR ARC_DEBUG_NAME_STR ARC_DEBUG_NAME_SEP_STR __VA_ARGS__);
#define ARC_DEBUG_ERR(...)  printf(ARC_DEBUG_ERR_STR  ARC_DEBUG_NAME_STR ARC_DEBUG_NAME_SEP_STR __VA_ARGS__);

#else

#define ARC_DEBUG(__level, ...) ;

#endif // ARC_DEBUG_ENABLE

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


extern struct ARC_FreelistMeta physical_mem;
extern uint64_t page_count;
extern void *kernel_elf;
extern struct ARC_BootMeta _boot_meta;
extern struct multiboot_tag_framebuffer *global_framebuffer;
extern uint8_t *global_kernel_font;

#endif
