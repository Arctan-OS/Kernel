#ifndef ARC_INTERFACE_FRAMEBUFFER_H
#define ARC_INTERFACE_FRAMEBUFFER_H

struct ARC_FramebufferMeta {
	void *vaddr;
	void *paddr;
	int width;
	int height;
	int bpp;
	int lock;
};

int Arc_CreateFramebuffer(struct ARC_FramebufferMeta *framebuffer);

#endif
