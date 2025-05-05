#ifndef ARC_CONFIG_H
#define ARC_CONFIG_H

// The number of processor descriptors to statically allocate
// TODO: Get rid of this
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
#define ARC_PMM_BUDDY_RATIO 1/2
// If this is set to 1 them the first 0x10000 bytes of memory 
// is reserved for legacy DMA (i.e. a floppy controller). If it 
// set to 0, then the memory will be used as part of general
// allocation
// NOTE: This is a kernel option. A bootstrapper should always
//       pass a free region < 0x10000 as low memory, and anything
//       above as high memory
#define ARC_SEPARATE_LOW_MEM 1
// The number of file descriptors a single process can have
#define ARC_PROCESS_FILE_LIMIT 1024
// The standard size of a memory buffer
#define ARC_STD_BUFF_SIZE (size_t)0x1000


#define ARC_E9_PORT 0x3F8

#endif
