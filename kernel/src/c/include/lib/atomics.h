#ifndef ARC_LIB_ATOMICS_H
#define ARC_LIB_ATOMICS_H

#include <stdint.h>
#include <stdatomic.h>

struct ARC_GenericSpinlock {
	uint32_t lock;
	uint64_t code;
}__attribute__((packed));

#define ARC_GENERIC_LOCK(__lock__) \
	while (atomic_flag_test_and_set_explicit(__lock__, memory_order_acquire)) __builtin_ia32_pause();
#define ARC_GENERIC_UNLOCK(__lock__) \
	while (atomic_flag_clear_explicit(__lock__, memory_order_release));

#endif
