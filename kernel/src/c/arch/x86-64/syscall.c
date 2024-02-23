#include <global.h>
#include <arch/x86-64/syscall.h>
#include <arch/x86-64/ctrl_regs.h>
#include <stdint.h>

extern int _syscall(uint64_t code, void *args);
int syscall_handler(uint64_t code, void *args) {
	ARC_DEBUG(INFO, "Syscall: %lX\n", code);

	return 0;
}

int Arc_InitializeSyscall() {
	uint64_t ia32_lstar = _x86_RDMSR(0xC0000082);
	ia32_lstar = (uintptr_t)_syscall;
	_x86_WRMSR(0xC0000082, ia32_lstar);

	uint64_t ia32_star = _x86_RDMSR(0xC0000081);
	uint16_t syscall_ss_cs = 0x08;
	uint16_t sysret_ss_cs = 0x18;
	ia32_star |= (uint64_t)((sysret_ss_cs << 16) | syscall_ss_cs) << 32;
	_x86_WRMSR(0xC0000081, ia32_star);

	uint64_t ia32_efer = _x86_RDMSR(0xC0000080);
	ia32_efer |= 1;
	_x86_WRMSR(0xC0000080, ia32_efer);

	ARC_DEBUG(INFO, "Installed syscalls\n");
	ARC_DEBUG(INFO, "\tEFER: 0x%lX\n", ia32_efer);
	ARC_DEBUG(INFO, "\tSTAR: 0x%lX\n", ia32_star);
	ARC_DEBUG(INFO, "\tLSTAR: 0x%lX\n", ia32_lstar);

	return 0;
}
