#ifndef ARC_RESOURCE_H
#define ARC_RESOURCE_H

#include <stddef.h>

struct ARC_Reference {
	struct ARC_Resource *resource;
	int lock; // TODO: Implement lock system

	int ref_count;
};

struct ARC_Resource {
	struct ARC_Reference *reference;
	int lock; // TODO: Implement lock system

	int (*open)();
	int (*write)(void *buffer, size_t size, size_t count);
	int (*read)(void *buffer, size_t size, size_t count);
	int (*close)();
	int (*seek)(long offset, int whence);
};

#endif
