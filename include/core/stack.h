#ifndef SNLS_STACK_HEADER
#define SNLS_STACK_HEADER

#include <stdbool.h>
#include "general/types.h"
#include "general/references.h"

#define SPC_STACK_PUSH8    0x1
#define SPC_STACK_PUSH16   0x3

#define SPC_STACK_PULL8    0x2
#define SPC_STACK_PULL16   0x4

#define CPU_STACK_PUSH8    0x5
#define CPU_STACK_PUSH16   0x6
#define CPU_STACK_PUSH24   0x7

#define CPU_STACK_PULL8    0x8
#define CPU_STACK_PULL16   0x9
#define CPU_STACK_PULL24   0xA

typedef struct cpuStack {
   u8          mode;
   u8          bank;
   u16         address;

   void        (*push)(snCPU* cpu, u8 mode, u16 address);
   void        (*pull)(snCPU* cpu, u8 mode);
} cpuStack;

typedef struct spcStack {
   bool        mode;
   u16         address;
   snSPC*      spc;

   void        (*push)(snSPC* spc, u8 mode, u16 address);
   void        (*pull)(snSPC* spc, u8 mode);
} spcStack;

#endif
