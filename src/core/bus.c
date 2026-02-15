#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <unistd.h>

#include "core/bus.h"

#include "emulator/main.h"
#include "general/functions.h"
#include "general/types.h"
#include "emulator/memory.h"

/* The reason we have functions with _SPC suffixes it's cause
 * of the SPC-700 independent bus, which makes our work a little
 * more exausting, but not so hard, just need different(not so..)
 * management */

static void busReadCPU(snCPU* cpu, char mode, bool indirect, u16 offset) {
	switch (mode) {
		case ADDRESS_CONST_8:
			cpu->holder.value = ((*++cpu->PC) + offset);
		break;
		case ADDRESS_CONST_16:
			hLoAddr = (*++cpu->PC);
			hHiAddr = (*++cpu->PC);
			hAddr = (hHiAddr << 8 | hLoAddr) + offset;
			cpu->holder.value = hAddr;
		break;
		case ADDRESS_RELATIVE_8:
			cpu->holder.value = (*++cpu->PC);
			cpu->holder.ptr = (cpu->PC + (s8) cpu->holder.value) + 1;
		break;
		case ADDRESS_RELATIVE_16:
			hLoAddr = (*++cpu->PC);
			hHiAddr = (*++cpu->PC);
			hAddr = (hHiAddr << 8 | hLoAddr);
			cpu->holder.value = hAddr;
			cpu->holder.ptr = (cpu->PC + (s16) cpu->holder.value) + 1;
		break;
		case ADDRESS_ABSOLUTE_8:
			/* WARNING: in the most of the times this mode will be used
			with a offset, if not, you will probally get a seg
			fault */
			
			hAddr = (*++cpu->PC) + offset;
			cpu->resolver->get(cpu->DBR, hAddr);
			cpu->holder.ptr = &cpu->memory->bank_array[cpu->resolver->realBank][cpu->resolver->realAddr];
			cpu->holder.value = *cpu->holder.ptr;
		break;
		case ADDRESS_ABSOLUTE_16:
			if (!indirect) {
				hLoAddr = (*++cpu->PC);
				hHiAddr = (*++cpu->PC);
				hAddr = (hHiAddr << 8 | hLoAddr) + offset;
				cpu->resolver->get(cpu->DBR, hAddr);
				cpu->holder.ptr = &cpu->memory->bank_array[cpu->resolver->realBank][cpu->resolver->realAddr];
				cpu->holder.value = *cpu->holder.ptr;
			} else {
				printf("bus_error: not implemented\n");
			}
		break;
		case ADDRESS_ABSOLUTE_24:
			if (!indirect) {
				hLoAddr = (*++cpu->PC);
				hHiAddr = (*++cpu->PC);
				hBank = (*++cpu->PC);
				hAddr = (hHiAddr << 8 | hLoAddr);
				cpu->resolver->get(hBank, hAddr);
				cpu->holder.ptr = &cpu->memory->bank_array[cpu->resolver->realBank][cpu->resolver->realAddr];
				cpu->holder.value = *cpu->holder.ptr;
				} else {
					/* NOTE: i guess only jump opcode uses this mode */
					
					hBank = (*++cpu->PC);
					hAddr = ((*++cpu->PC) << 8) | (*++cpu->PC);
					cpu->resolver->get(hBank, hAddr); /* Get the real address */
					hLoAddr = cpu->memory->bank_array[cpu->resolver->realBank][cpu->resolver->realAddr];
					hHiAddr = cpu->memory->bank_array[cpu->resolver->realBank][cpu->resolver->realAddr + 1];
					hBank = cpu->memory->bank_array[cpu->resolver->realBank][cpu->resolver->realAddr + 2];
					cpu->holder.ptr = &cpu->memory->bank_array[hBank][hHiAddr << 8 | hLoAddr];
					cpu->holder.value = *cpu->holder.ptr;
				}
		break;
		case ADDRESS_DP:
			if (!indirect) {
				cpu->holder.ptr = &cpu->memory->bank_array[cpu->DBR][cpu->DP + *++cpu->PC + offset];
				cpu->holder.value = *cpu->holder.ptr;
			} else {
				hLoAddr = cpu->memory->bank_array[cpu->DBR][cpu->DP + (*++cpu->PC) + offset];
				hHiAddr = cpu->memory->bank_array[cpu->DBR][cpu->DP + (*++cpu->PC) + offset + 1];
				hAddr = hHiAddr << 8 | hLoAddr;
				cpu->resolver->get(cpu->DBR, hAddr);

				cpu->holder.ptr = &cpu->memory->bank_array[cpu->resolver->realBank][cpu->resolver->realAddr];
				cpu->holder.value = *cpu->holder.ptr;
			}
		break;
		case ADDRESS_DP_24:
			if (!indirect) {
				printf("bus_error: this mode doesnt exist ADDRESS_DP_24\n");
			} else {
				hLoAddr = cpu->memory->bank_array[cpu->DBR][cpu->DP + (*++cpu->PC) + offset];
				hHiAddr = cpu->memory->bank_array[cpu->DBR][cpu->DP + (*++cpu->PC) + offset + 1];
				hBank = cpu->memory->bank_array[cpu->DBR][cpu->DP + (*++cpu->PC) + offset + 1];
				hAddr = hHiAddr << 8 | hLoAddr;
				cpu->resolver->get(hBank, hAddr);

				cpu->holder.ptr = &cpu->memory->bank_array[cpu->resolver->realBank][cpu->resolver->realAddr];
				cpu->holder.value = *cpu->holder.ptr;
			}
		break;
	}
	return;
}

static void busWrite(snCPU* cpu, u8 mode, bool indirect, u16 offset);

static void writeSNES_CPU(snCPU* cpu, u16 value) {
	printf("bus_write: writing %X to [%X][%X] %p \n", \
	       value, cpu->resolver->realBank, cpu->resolver->realAddr, cpu->holder.ptr);

	*cpu->holder.ptr = value;
};

static void writeSNESindirect_SPC(snSPC* spc, u16 offset, u16 value, u8 mode) {
	switch (mode) {
		/* 0x00 -> PC */
		/* 0x01 -> AC */
		/* 0x02 -> DP */
		/* 0x03 -> X  */
		/* 0x04 -> Y  */
		/* 0x05 -> PC + DP */
		/* 0x06 -> AC + DP */
		/* 0x07 -> X + DP */
		/* 0x07 -> Y + DP */

		case 0x00:

		break;
		case 0x02:

		break;
		case 0x03:
			spc->bus->address = (spc->X) + offset;
		break;
		case 0x05:
			spc->bus->address = ((*++spc->PC) | spc->DP) + offset;
		break;
		case 0x07:
			spc->bus->address = (spc->DP | spc->X) + offset;
		break;
		default:
			printf("spc_bus: unknown mode %X \n");
			exit(1);
		break;
	};
	printf("spc_bus_write_indirect: writing %X to %X \n", value, spc->bus->address);
	spc->bus->pointer = &spc->internalRAM[spc->bus->address];
	*spc->bus->pointer = value;
};


static void writeSNES_SPC(snSPC* spc, u16 value) {
	printf("spc_bus_write: writing %X to [%X] \n", value, spc->bus->address);
	*spc->bus->pointer = value;
};

static void readU8const_SPC(snSPC* spc, u16 offset) {
	hAddr = *(++spc->PC + offset);
};

static void readU16absolute_SPC(snSPC* spc, u16 offset) {

	hLoAddr = *(++spc->PC);
	hHiAddr = *(++spc->PC);
	hAddr = (hHiAddr << 8 | hLoAddr) + offset;
	printf("bus_spc: WARNING, code notimplemented due to SPC RAM implementation \n");
	printf("bus_spc: add it in the get Mapped Bank function \n");

};

static void readU16absoluteIndirect_SPC(snSPC* spc, u8 mode) {

	/* 0x00 -> PC */
	/* 0x01 -> AC */
	/* 0x02 -> DP */
	/* 0x03 -> X  */
	/* 0x04 -> Y  */
	/* 0x05 -> DP + X */
	/* 0x06 -> DP + Y */

	switch (mode) {
		case 0x02:
			spc->bus->value = spc->internalRAM[spc->DP];
		break;
		default:

		break;
	}

};

static void readU16const_SPC(snSPC* spc) {
	hLoAddr = *(++spc->PC);
	hHiAddr = *(++spc->PC);
	hAddr = hHiAddr << 8 | hLoAddr;
	spc->bus->value = hAddr;
}

extern void setupSPCBUS(snSPC* spc, spcBUS* bus) {
	//emulator->apu->spc->bus = cpu;
	/* We just make a callback to emulatorPtr
	 * So we can still acess something from out */
	spc->bus->address = 0x0000;
	spc->bus->value = 0xFFFF;
	spc->bus->pointer = NULL;

	bus->readU8const = readU8const_SPC;
	bus->readU16const = readU16const_SPC;
	bus->readU16absolute = readU16absolute_SPC;
	bus->readU16absoluteIndirect = readU16absoluteIndirect_SPC;
	bus->write = writeSNES_SPC;
	bus->writeIndirect = writeSNESindirect_SPC;
}

extern void setupCPUBUS(snCPU* cpu, cpuBUS* bus) {
	cpu->bus = bus;
	cpu->resolver->realAddr = 0x0000;
	cpu->resolver->realBank = 0x00;
	cpu->resolver->addrPtr = NULL;

	bus->memory = cpu->memory;
	bus->holder = &cpu->holder;
	bus->read = busReadCPU;
	bus->write = writeSNES_CPU;
}
