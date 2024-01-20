#ifndef ARC_INTERFACE_COMPOSITOR_H
#define ARC_INTERFACE_COMPOSITOR_H

#include <interface/framebuffer.h>

struct ARC_CompositeMode {
	int lock;
};

int Arc_BinaryComposite(struct ARC_CompositeMode *mode, struct ARC_FramebufferMeta *master, struct ARC_FramebufferMeta *addition);

#endif
