#include "io/port.h"
#include <io/ctrl_reg.h>
#include <arch/sse.h>
#include <cpuid.h>
#include <stdint.h>

struct ARC_SSEMeta {
	uint8_t sse :   1;
	uint8_t sse2 :  1;
	uint8_t sse3 :  1;
	uint8_t ssse3 : 1;
	uint8_t sse4 :  1;
	uint8_t ssse4 : 1;
};
struct ARC_SSEMeta sse_metadata;

void init_sse() {
	volatile register int eax = 0;
	volatile register int ebx = 0;
	volatile register int ecx = 0;
	volatile register int edx = 0;

	__cpuid(0x01, eax, ebx, ecx, edx);

	sse_metadata.sse   = (edx >> 25) & 1;
	sse_metadata.sse2  = (edx >> 26) & 1;
	sse_metadata.sse3  = (ecx >> 0)  & 1;
	sse_metadata.ssse3 = (ecx >> 9)  & 1;
	sse_metadata.sse4  = (ecx >> 19) & 1;
	sse_metadata.ssse4 = (ecx >> 20) & 1;

	if (*(int *)&sse_metadata != 0b111111) {
		for (;;);
	}

	read_cr0();
	read_cr4();

	cr4_reg.OSFXSR = 1;
	cr4_reg.OSXMMEXCPT = 1;

	cr0_reg.EM = 0;
	cr0_reg.MP = 0;

	set_cr0();
	set_cr4();
}
