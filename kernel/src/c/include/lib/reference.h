#ifndef ARC_LIB_REFERENCE_H
#define ARC_LIB_REFERENCE_H

struct ARC_Reference {
	struct ARC_Reference *prev;
	struct ARC_Reference *next;
};

#endif
