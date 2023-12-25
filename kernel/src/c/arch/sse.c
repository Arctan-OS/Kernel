#include <io/port.h>
#include <io/ctrl_reg.h>
#include <arch/sse.h>
#include <cpuid.h>
#include <stdint.h>
#include <framebuffer/printf.h>

struct ARC_SSEMeta {
	uint8_t sse :   1;
	uint8_t sse2 :  1;
	uint8_t sse3 :  1;
	uint8_t ssse3 : 1;
	uint8_t sse4 :  1;
	uint8_t ssse4 : 1;
}__attribute__((aligned(4)));
struct ARC_SSEMeta sse_metadata;

uint8_t float_save_buffer[512] __attribute__((aligned(16)));

void init_sse() {
	volatile register int eax = 0;
	volatile register int ebx = 0;
	volatile register int ecx = 0;
	volatile register int edx = 0;

	__cpuid(0x01, eax, ebx, ecx, edx);

	*(int *)&sse_metadata = 0;
	sse_metadata.sse   = (edx >> 25) & 1;
	sse_metadata.sse2  = (edx >> 26) & 1;
	sse_metadata.sse3  = (ecx >> 0)  & 1;
	sse_metadata.ssse3 = (ecx >> 9)  & 1;
	sse_metadata.sse4  = (ecx >> 19) & 1;
	sse_metadata.ssse4 = (ecx >> 20) & 1;

	if (*(int *)&sse_metadata != 0b111111) {
		printf("Not all SEE flags enabled (0b%08b)\n", *(uint8_t *)&sse_metadata);
	}

	read_cr0();
	read_cr4();

	cr4_reg.OSFXSR = 1;
	cr4_reg.OSXMMEXCPT = 1;

	cr0_reg.EM = 0;
	cr0_reg.MP = 0;

	set_cr0();
	set_cr4();

	if (((ecx >> 26) & 1) == 1) {
		cr4_reg.OSXSAVE = 1;
		set_cr4(); // ERROR: Causes CPU to grind to a halt
	} else {
		printf("OSXSAVE is not supported\n");
		return;
	}


	if (((ecx >> 26) & 1) == 1) {
                __asm__("fxsave %0" ::"m"(float_save_buffer));

                __asm__("push rax; push rcx; push rdx;  \
		 xor rcx, rcx;			\
                 xgetbv;                        \
                 or eax, 7;                     \
                 xsetbv;                        \
		 pop rdx; pop rcx; pop rax;"
                        :
                        :
                        :);
	}

	printf("Enabled SSE support\n");
}
