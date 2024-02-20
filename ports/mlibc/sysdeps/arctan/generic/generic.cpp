#include <time.h>

namespace mlibc {

	void sys_libc_log(char const *str) {
		(void)str;
	}

	void sys_libc_panic() {
		for (;;);
	}

	int sys_tcb_set(void *arg) {
		(void)arg;
		return 0;
	}

	int sys_futex_wait(int *ptr, int expected, timespec const *time) {
		(void)ptr;
		(void)expected;
		(void)time;

		return 0;
	}

	int sys_futex_wake(int *ptr) {
		(void)ptr;

		return 0;
	}

	int sys_clock_get(int a, long *b, long *c) {
		(void)a;
		(void)b;
		(void)c;

		return 0;
	}

	void sys_exit(int code) {
		(void)code;

		for (;;);
	}

	int sys_seek(int fd, long offset, int whence, long *new_offset) {
		(void)fd;
		(void)offset;
		(void)whence;
		(void)new_offset;

		return 0;
	}

	int sys_write(int fd, void const *a, unsigned long b, long *c) {
		(void)fd;
		(void)a;
		(void)b;
		(void)c;

		return 0;
	}

	int sys_read(int fd, void *buf, unsigned long count, long *bytes_read) {
		(void)fd;
		(void)buf;
		(void)count;
		(void)bytes_read;

		return 0;
	}

	int sys_close(int fd) {
		(void)fd;

		return 0;
	}

	int sys_open(char const *name, int flags, unsigned int mode, int *fd) {
		(void)name;
		(void)flags;
		(void)mode;
		(void)fd;

		return 0;
	}

	int sys_vm_map(void *hint, unsigned long size, int prot, int flags, int fd, long offset, void **window) {
		(void)hint;
		(void)size;
		(void)prot;
		(void)flags;
		(void)fd;
		(void)offset;
		(void)window;

		return 0;
	}

	int sys_vm_unmap(void *c, unsigned long b) {
		(void)c;
		(void)b;

		return 0;
	}

	int sys_anon_allocate(unsigned long size, void **ptr) {
		(void)size;
		(void)ptr;

		return 0;
	}

	int sys_anon_free(void *ptr, unsigned long size) {
		(void)ptr;
		(void)size;

		return 0;
	}
} // namespace mlibc
