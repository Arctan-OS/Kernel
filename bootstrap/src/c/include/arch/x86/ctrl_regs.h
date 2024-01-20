#ifndef ARC_X86_CTRL_REGS_H
#define ARC_X86_CTRL_REGS_H

#include <stdint.h>

extern uint64_t _x86_CR0;
extern uint64_t _x86_CR1;
extern uint64_t _x86_CR2;
extern uint64_t _x86_CR3;
extern uint64_t _x86_CR4;

extern void _x86_getCR0();
extern void _x86_setCR0();

extern void _x86_getCR1();
extern void _x86_setCR1();

extern void _x86_getCR2();
extern void _x86_setCR2();

extern void _x86_getCR3();
extern void _x86_setCR3();

extern void _x86_getCR4();
extern void _x86_setCR4();

#endif
