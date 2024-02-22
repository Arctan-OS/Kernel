#include <stdint.h>
#include <time.h>

namespace mlibc {
	#define SYSCALL(code, args, ret) \
		__asm__("mov %1, %%edi; \
                         mov %2, %%rsi; \
	                 syscall" : "=a"(ret) : "a"(code), "g"(args) : "rdi", "rsi");

	struct ARC_SyscallArgs {
		uint64_t a;
		uint64_t b;
		uint64_t c;
		uint64_t d;
		uint64_t e;
		uint64_t f;
		uint64_t g;
	}__attribute__((packed));

	void sys_libc_log(char const *str) {
		struct ARC_SyscallArgs args;
		args.a = (uint64_t) str;

		int ret = 0;
		SYSCALL(0x0E, &args, ret);
	}

	void sys_exit(int code) {
		struct ARC_SyscallArgs args;
		args.a = (uint64_t) code;

		int ret = 0;
		SYSCALL(0x04, &args, ret);

		for (;;);
	}

	void sys_libc_panic() {
		sys_libc_log("mlibc: panic!\n");
		sys_exit(-UINT16_MAX);

		for (;;);
	}

	int sys_tcb_set(void *arg) {
		struct ARC_SyscallArgs args;
		args.a = (uint64_t) arg;

		int ret = 0;
		SYSCALL(0x00, &args, ret);

		return ret;
	}

	int sys_futex_wait(int *ptr, int expected, timespec const *time) {
		struct ARC_SyscallArgs args;
		args.a = (uint64_t) ptr;
		args.b = (uint64_t) expected;
		args.c = (uint64_t) time;

		int ret = 0;
		SYSCALL(0x01, &args, ret);

		return ret;
	}

	int sys_futex_wake(int *ptr) {
		struct ARC_SyscallArgs args;
		args.a = (uint64_t) ptr;

		int ret = 0;
		SYSCALL(0x02, &args, ret);

		return ret;
	}

	int sys_clock_get(int a, long *b, long *c) {
		struct ARC_SyscallArgs args;
		args.a = (uint64_t) a;
		args.b = (uint64_t) b;
		args.c = (uint64_t) c;

		int ret = 0;
		SYSCALL(0x03, &args, ret);

		return ret;
	}

	int sys_seek(int fd, long offset, int whence, long *new_offset) {
		struct ARC_SyscallArgs args;
		args.a = (uint64_t) fd;
		args.b = (uint64_t) offset;
		args.c = (uint64_t) whence;
		args.d = (uint64_t) new_offset;

		int ret = 0;
		SYSCALL(0x05, &args, ret);

		return ret;
	}

	int sys_write(int fd, void const *a, unsigned long b, long *c) {
		struct ARC_SyscallArgs args;
		args.a = (uint64_t) fd;
		args.b = (uint64_t) a;
		args.c = (uint64_t) b;
		args.d = (uint64_t) c;

		int ret = 0;
		SYSCALL(0x06, &args, ret);

		return ret;
	}

	int sys_read(int fd, void *buf, unsigned long count, long *bytes_read) {
		struct ARC_SyscallArgs args;
		args.a = (uint64_t) fd;
		args.b = (uint64_t) buf;
		args.c = (uint64_t) count;
		args.d = (uint64_t) bytes_read;

		int ret = 0;
		SYSCALL(0x07, &args, ret);

		return ret;
	}

	int sys_close(int fd) {
		struct ARC_SyscallArgs args;
		args.a = (uint64_t) fd;

		int ret = 0;
		SYSCALL(0x08, &args, ret);

		return ret;
	}

	int sys_open(char const *name, int flags, unsigned int mode, int *fd) {
		struct ARC_SyscallArgs args;
		args.a = (uint64_t) name;
		args.b = (uint64_t) flags;
		args.c = (uint64_t) mode;
		args.d = (uint64_t) fd;

		int ret = 0;
		SYSCALL(0x09, &args, ret);

		return ret;
	}

	int sys_vm_map(void *hint, unsigned long size, int prot, int flags, int fd, long offset, void **window) {
		struct ARC_SyscallArgs args;
		args.a = (uint64_t) hint;
		args.b = (uint64_t) size;
		args.c = (uint64_t) prot;
		args.d = (uint64_t) flags;
		args.e = (uint64_t) fd;
		args.f = (uint64_t) offset;
		args.g = (uint64_t) window;

		int ret = 0;
		SYSCALL(0x0A, &args, ret);

		return ret;
	}

	int sys_vm_unmap(void *a, unsigned long b) {
		struct ARC_SyscallArgs args;
		args.a = (uint64_t) a;
		args.b = (uint64_t) b;

		int ret = 0;
		SYSCALL(0x0B, &args, ret);

		return ret;
	}

	int sys_anon_allocate(unsigned long size, void **ptr) {
		struct ARC_SyscallArgs args;
		args.a = (uint64_t) size;
		args.b = (uint64_t) ptr;

		int ret = 0;
		SYSCALL(0x0C, &args, ret);

		return ret;
	}

	int sys_anon_free(void *ptr, unsigned long size) {
		struct ARC_SyscallArgs args;
		args.a = (uint64_t) ptr;
		args.b = (uint64_t) size;

		int ret = 0;
		SYSCALL(0x0D, &args, ret);

		return ret;
	}
} // namespace mlibc
