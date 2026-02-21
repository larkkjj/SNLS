#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "general/types.h"
#include "core/cpu.h"
#include "core/apu.h"

static void pushSPC(snSPC* spc, u8 mode, u16 address) {
   switch (mode) {
      case SPC_STACK_PUSH8:
         spc->internalRAM[spc->SP] = address;
      break;
      case SPC_STACK_PUSH16:
         hLoByte = (u8) address;
         hHiByte = (u8) (address >> 8);
         spc->internalRAM[spc->SP] = hLoByte;
         spc->internalRAM[--spc->SP] = hHiByte;
      break;
      default:
         printf("spc_stack: unknown usage \n");
   }
};

static void pullSPC(snSPC* spc, u8 mode) {
   switch (mode) {
      case SPC_STACK_PULL8:

      break;
      case SPC_STACK_PULL16:

      break;
      default:
         printf("spc_stack: unknown usage\n");
   }
};

static void pushCPU(snCPU* cpu, u8 mode, u32 address) {
   switch (mode) {
      case CPU_STACK_PUSH8:
         cpu->memory->bank_array[cpu->DBR][cpu->SP] = (u8) address;
         cpu->SP--;
      break;
      case CPU_STACK_PUSH16:
         hLoAddr = (u8) (address);
         hHiAddr = (u8) (address >> 8);
         printf("%02X %02X\n", hLoAddr, hHiAddr);
         cpu->memory->bank_array[cpu->DBR][cpu->SP] = hHiAddr;
         cpu->memory->bank_array[cpu->DBR][--cpu->SP] = hLoAddr;
         --cpu->SP;
      break;
      case CPU_STACK_PUSH24:
         hLoAddr = (u8) (address);
         hHiAddr = (u8) (address >> 8);
         hBank = (u8) (address >> 16);
         printf("%02X %02X\n", hLoAddr, hHiAddr);
         cpu->memory->bank_array[cpu->DBR][cpu->SP] = hHiAddr;
         cpu->memory->bank_array[cpu->DBR][--cpu->SP] = hLoAddr;
         cpu->memory->bank_array[cpu->DBR][--cpu->SP] = hBank;
         --cpu->SP;
      break;
      default:
         printf("stack_cpu: unknown usage \n");
         exit(1);
   }
};

static void pullCPU(snCPU* cpu, u8 mode) {
   switch (mode) {
      case CPU_STACK_PULL8:
         hAddr = cpu->memory->bank_array[cpu->DBR][++cpu->SP];
         cpu->holder.value = hAddr;
      break;
      case CPU_STACK_PULL16:
         hLoAddr = cpu->memory->bank_array[cpu->DBR][++cpu->SP];
         hHiAddr = cpu->memory->bank_array[cpu->DBR][++cpu->SP];
         cpu->holder.value = (hHiAddr << 8 | hLoAddr);
      break;
      case CPU_STACK_PULL24:
         cpu->holder.value = (hHiAddr << 8 | hLoAddr);
      break;
      default:
         printf("stack_cpu: unknown usage\n");
         exit(1);
   }
};

extern void setupCPUstack(cpuStack* stack) {
   printf("stack_cpu_setup: init\n");
   stack->push = pushCPU;
   stack->pull = pullCPU;
   printf("stack_cpu_setup: done\n");
};

extern void setupSPCstack(spcStack* stack, snSPC* spc) {
   stack->push = pushSPC;
   stack->pull = pullSPC;
   stack->spc = spc;
};
