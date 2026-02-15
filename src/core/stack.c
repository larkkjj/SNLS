#include <stdio.h>

#include "general/types.h"
#include "core/cpu.h"
#include "core/apu.h"

static void pushSPC(snSPC* spc, u16 address) {
   switch (spc->stack.mode) {
      case SPC_STACK_PUSH8:
         spc->internalRAM[spc->SP] = address;
         --spc->SP;
      break;
      case SPC_STACK_PUSH16:
         hLoByte = (u8) address;
         hHiByte = (u8) (address << 8);
         spc->internalRAM[spc->SP] = hLoByte;
         spc->internalRAM[--spc->SP] = hHiByte;
         --spc->SP;
      break;
      default:
         printf("spc_stack: error \n");
   }
};

static void pullSPC(snSPC* spc) {
   switch (spc->stack.mode) {
      case SPC_STACK_PULL8:

      break;
      case SPC_STACK_PULL16:

      break;
   }
};

static void pushCPU(snCPU* cpu, u16 address) {
   switch (cpu->stack.mode) {
      case CPU_STACK_PUSH8:
         cpu->memory->bank_array[cpu->DBR][cpu->SP] = address;
         --cpu->SP;
      break;
      case CPU_STACK_PUSH16:
         hLoByte = (u8) address;
         hHiByte = (u8) (address << 8);
         cpu->memory->bank_array[cpu->DBR][cpu->SP] = hLoByte;
         cpu->memory->bank_array[cpu->DBR][--cpu->SP] = hHiByte;
         --cpu->SP;
      break;
      case CPU_STACK_PUSH24:

      break;
      default:
         printf("stack_cpu: error \n");
   }
};

static void pullCPU(snCPU* cpu) {
   switch (cpu->stack.mode) {
      case CPU_STACK_PULL8:
         hAddr = cpu->memory->bank_array[cpu->DBR][++cpu->SP];
         cpu->holder.value = hAddr;
         --cpu->SP;
      break;
      case CPU_STACK_PULL16:
         hLoAddr = cpu->memory->bank_array[cpu->DBR][++cpu->SP];
         hHiAddr = cpu->memory->bank_array[cpu->DBR][++cpu->SP];
         cpu->holder.value = (hHiAddr << 8 | hLoAddr);
         cpu->SP -= 2;
      break;
      case CPU_STACK_PULL24:
      break;
   }
};

extern void setupCPUstack(cpuStack* stack) {
   stack->push = pushCPU;
   stack->pull = pullCPU;
};

extern void setupSPCstack(spcStack* stack, snSPC* spc) {
   stack->push = pushSPC;
   stack->pull = pullSPC;
   stack->spc = spc;
};
