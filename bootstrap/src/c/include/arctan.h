#ifndef ARC_ARCTAN_H
#define ARC_ARCTAN_H

#define ARC_HHDM_VADDR   0xFFFFC00000000000 // 192 TiB

#include <stdint.h>
#include <stddef.h>

struct ARC_KernMeta {

}__attribute__((packed));

struct ARC_BootMeta {
	uint32_t mb2i; // Physical address of MBI2 structure
	uint32_t pmm_state; // Physical pointer to the state of the bootstrapper's PMM (of type struct ARC_FreelsitMeta)
	struct Arc_KernMeta *state; // State of the last kernel
}__attribute__((packed));

#endif
