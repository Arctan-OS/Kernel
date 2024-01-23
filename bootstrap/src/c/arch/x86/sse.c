
#include <arch/x86/ctrl_regs.h>
#include <global.h>
#include <arch/x86/sse.h>

extern void _osxsave_support();

void init_sse(int ecx, int edx) {
	// Check for SSE2
	if (((edx >> 25) & 1) == 0 && ((edx >> 26) & 1) == 0
	    && (ecx & 1) == 0 && ((ecx >> 9) & 1) == 0) {
		ARC_DEBUG(INFO, "No SSE/SSE2/SSE3/SSSE3 extensions\n");
		return;
	}

	_x86_getCR0();
	_x86_CR0 &= ~(1 << 2); // Disable x87 FPU emulation
	_x86_CR0 |= (1 << 1); //
	_x86_setCR0();

	_x86_getCR4();
	_x86_CR4 |= (1 << 9); // OSFXSR
	_x86_CR4 |= (1 << 10); // OSXMMEXCPT
	_x86_setCR4();

	if (((ecx >> 27) & 1) == 1) {
		_x86_CR4 |= (1 << 18); // OSXSAVE support
		_x86_setCR4();
		_osxsave_support();
	}

	// Set MXCSR Register
	ARC_DEBUG(INFO, "Enabled SSE support\n");
}
