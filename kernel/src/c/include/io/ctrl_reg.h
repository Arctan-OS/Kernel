#ifndef CTRL_REG
#define CTRL_REG

#include <global.h>

struct cr0_reg_struct {
	uint8_t PE : 1;
	uint8_t MP : 1;
	uint8_t EM : 1;
	uint8_t TS : 1;
	uint8_t ET : 1;
	uint8_t NE : 1;
	uint16_t resv1 : 10;
	uint8_t WP : 1;
	uint8_t resv2 : 1;
	uint8_t AM : 1;
	uint16_t resv3 : 7;
	uint8_t NW : 1;
	uint8_t CD : 1;
	uint8_t PG : 1;
}__attribute__((packed));
extern struct cr0_reg_struct cr0_reg;

extern uint64_t cr2_reg; // Page-Fault Linear Address

struct cr3_reg_struct {
	uint8_t resv1 : 3;
	uint8_t PWT : 1;
	uint8_t PCD : 1;
	uint16_t resv2 : 7;
	uint64_t base : 52; // Page Directory Base address >> 12
}__attribute__((packed));
extern struct cr3_reg_struct cr3_reg;

struct cr4_reg_struct {
	uint8_t VME : 1;
	uint8_t PVI : 1;
	uint8_t TSD : 1;
	uint8_t DE : 1;
	uint8_t PSE : 1;
	uint8_t PAE : 1;
	uint8_t MCE : 1;
	uint8_t PGE : 1;
	uint8_t PCE : 1;
	uint8_t OSFXSR : 1;
	uint8_t OSXMMEXCPT : 1;
	uint8_t UMIP : 1;
	uint8_t resv1 : 1;
	uint8_t VMXE : 1;
	uint8_t SMXE : 1;
	uint8_t resv2 : 1;
	uint8_t FSGSBASE : 1;
	uint8_t PCIDE : 1;
	uint8_t OSXSAVE : 1;
	uint8_t KL : 1;
	uint8_t SMEP : 1;
	uint8_t SMAP : 1;
	uint8_t PKE: 1;
	uint8_t CET: 1;
	uint8_t PKS: 1;
	uint8_t UINTR: 1;
	uint64_t resv3: 38;
}__attribute__((packed));
extern struct cr4_reg_struct cr4_reg;

extern void set_cr0();
extern void read_cr0();

extern void set_cr1();
extern void read_cr1();

extern void set_cr2();
extern void read_cr2();

extern void set_cr3();
extern void read_cr3();

extern void set_cr4();
extern void read_cr4();

#endif