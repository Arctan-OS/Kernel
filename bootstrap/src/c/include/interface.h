#ifndef INTERFACE_H
#define INTERFACE_H

#include <stdint.h>
#include <stdarg.h>

#ifdef E9HACK
#define E9_HACK(c) outb(0xE9, c);
#else
#define E9_HACK(c) ;
#endif

extern void outb(uint16_t port, uint8_t value);

void putc(char c);
void puts(char *s);
int putn(uint32_t val, uint8_t base, int depth);
void printf(const char *form, ...);

#endif