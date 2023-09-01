# Arctan Operating System

End goal: A simple 64-bit operating system which is able to load programs into memory and store them to disk.

## 1. Memory Management
x86 long mode requires at least a PML4 paging table. In ArctanOS, physical and virtual memory will be handled
differently. To keep track of the physical memory, a bitmap will be used (each bit representing a 4KiB page frame).
To keep track of the virtual memory a stack will be used (each item representing a 4KiB page). An intermediate mapper will
be used, memman.c. The virtual and physical allocation and free processes will be abstracted in alloc.c, this will provide,
as of writing this, two crucial functions: malloc() and free(). There will probably be some derivatives for kernel specific
allocations and frees.

## 2. Interrupt handling
APIC

## 3. Filesystem
For the physical file system manager, a simple ext2 implementation should do. And there will be another abstraction: the VFS.
The VFS will hold critical functions that will allow the file to be opened, closed, read, written, sought, and told.

## 4. Multi-tasking
Multi-tasking will be approached in a compartmentalized fashion: front and back. The front will keep track of the tasks which
need to be done and the back will be able to determine the order in which those tasks should be done. The default will be
a simple round-robin back.

### 5. Multiple cores
Might try this.

## 6. Userland
Would be nice to have a userland.

## 7. TTY
Work out a TTY interface so user / operator can enter commands.

## 8. Additional drivers
Possibly support more devices.
