#ifndef ARC_CONFIG_H
#define ARC_CONFIG_H

// The number of processor descriptors to statically allocate
#define ARC_MAX_PROCESSORS 16
// The number of ticks, or timer interrupts, that are to happen
// in one second
#define ARC_TICK_FREQUENCY 1000
// The number of ticks that constitute a single timeslice for
// the scheduler
#define ARC_TICKS_PER_TIMESLICE 1000
// Used in the calculation for the total number of pages the
// physical buddy allocator should use 
// (buddy pages = total pages * ARC_PMM_BUDDY_RATIO)
#define ARC_PMM_BUDDY_RATIO 3/4

#endif
