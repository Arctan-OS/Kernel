#include <time.h>

namespace mlibc {
	#define UINT64_T unsigned long

	struct ARC_SyscallArgs {
		UINT64_T a;
		UINT64_T b;
		UINT64_T c;
		UINT64_T d;
		UINT64_T e;
		UINT64_T f;
		UINT64_T g;
	}__attribute__((packed));


	__attribute__((sysv_abi)) int Arc_Syscall(int code, void *args) {
		(void)code;
		(void)args;

		int ret = 0;

		// RDI = code
		// RSI = args
		__asm__("syscall");
		// RAX should be return value

		return ret;
	}

	void sys_libc_log(char const *str) {
		// TODO: Implement
		(void)str;
	}

	void sys_libc_panic() {
		// TODO: Implement
		for (;;);
	}

	int sys_tcb_set(void *arg) {
		struct ARC_SyscallArgs args = { .a = (UINT64_T) arg };
		return Arc_Syscall(0x00, &args);
	}

	int sys_futex_wait(int *ptr, int expected, timespec const *time) {
		struct ARC_SyscallArgs args = { .a = (UINT64_T) ptr, .b = (UINT64_T) expected, .c = (UINT64_T) time };
		return Arc_Syscall(0x01, &args);
	}

	int sys_futex_wake(int *ptr) {
		struct ARC_SyscallArgs args = { .a = (UINT64_T) ptr };
		return Arc_Syscall(0x02, &args);
	}

	int sys_clock_get(int a, long *b, long *c) {
		struct ARC_SyscallArgs args = { .a = (UINT64_T) a, .b = (UINT64_T) b, .c = (UINT64_T) c };
		return Arc_Syscall(0x03, &args);
	}

	void sys_exit(int code) {
		struct ARC_SyscallArgs args = { .a = (UINT64_T) code };
		Arc_Syscall(0x04, &args);

		for (;;);
	}

	int sys_seek(int fd, long offset, int whence, long *new_offset) {
		struct ARC_SyscallArgs args = { .a = (UINT64_T) fd, .b = (UINT64_T) offset,
		                                .c = (UINT64_T) whence, .d = (UINT64_T) new_offset };
		return Arc_Syscall(0x05, &args);
	}

	int sys_write(int fd, void const *a, unsigned long b, long *c) {
		struct ARC_SyscallArgs args = { .a = (UINT64_T) fd, .b = (UINT64_T) a,
		                                .c = (UINT64_T) b, .d = (UINT64_T) c };

		return Arc_Syscall(0x06, &args);
	}

	int sys_read(int fd, void *buf, unsigned long count, long *bytes_read) {
		struct ARC_SyscallArgs args = { .a = (UINT64_T) fd, .b = (UINT64_T) buf,
		                                .c = (UINT64_T) count, .d = (UINT64_T) bytes_read };

		return Arc_Syscall(0x07, &args);
	}

	int sys_close(int fd) {
		struct ARC_SyscallArgs args = { .a = (UINT64_T) fd };
		return Arc_Syscall(0x08, &args);
	}

	int sys_open(char const *name, int flags, unsigned int mode, int *fd) {
		struct ARC_SyscallArgs args = { .a = (UINT64_T) name, .b = (UINT64_T) flags,
		                                .c = (UINT64_T) mode, .d = (UINT64_T) fd };
		return Arc_Syscall(0x09, &args);
	}

	int sys_vm_map(void *hint, unsigned long size, int prot, int flags, int fd, long offset, void **window) {
		struct ARC_SyscallArgs args = { .a = (UINT64_T) hint, .b = (UINT64_T) size,
		                                .c = (UINT64_T) prot, .d = (UINT64_T) flags,
		                                .e = (UINT64_T) fd, .f = (UINT64_T) offset,
		                                .g = (UINT64_T) window };
		return Arc_Syscall(0x0A, &args);
	}

	int sys_vm_unmap(void *a, unsigned long b) {
		struct ARC_SyscallArgs args = { .a = (UINT64_T) a, .b = (UINT64_T) b };
		return Arc_Syscall(0x0B, &args);
	}

	int sys_anon_allocate(unsigned long size, void **ptr) {
		struct ARC_SyscallArgs args = { .a = (UINT64_T) size, .b = (UINT64_T) ptr };
		return Arc_Syscall(0x0C, &args);
	}

	int sys_anon_free(void *ptr, unsigned long size) {
		struct ARC_SyscallArgs args = { .a = (UINT64_T) ptr, .b = (UINT64_T) size };
		return Arc_Syscall(0x0D, &args);
	}

	#undef UINT64_T
} // namespace mlibc
