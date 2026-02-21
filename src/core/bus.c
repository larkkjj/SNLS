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

static void busReadCPU(snCPU* cpu, char mode, char assign_mode, u16 offset) {
	cpu->holder.value = 0;
	cpu->holder.ptr = NULL;
	switch (mode) {
		case ADDRESS_CONST:
			switch (assign_mode) {
				case ASSIGN_NONE:
					cpu->holder.value = *(++cpu->PC);
					return;
				break;
				case ASSIGN_INDEX:
					if (cpu->flags.Index) {
						cpu->holder.value = *(++cpu->PC);
					} else {
						hLoAddr = *(++cpu->PC);
						hHiAddr = *(++cpu->PC);
						cpu->holder.value = (hHiAddr << 8 | hLoAddr);
					}
					return;
				break;
				case ASSIGN_ACCUMULATOR:
					if (cpu->flags.Accumulator) {
						cpu->holder.value = *(++cpu->PC);
					} else {
						hLoAddr = *(++cpu->PC);
						hHiAddr = *(++cpu->PC);
						cpu->holder.value = (hHiAddr << 8 | hLoAddr);
					}
					return;
				break;
			}
		break;
		case ADDRESS_RELATIVE_8:
			cpu->holder.value = (*++cpu->PC);
			cpu->holder.ptr = (cpu->PC + (s8) cpu->holder.value) + 1;
			return;
		break;
		case ADDRESS_RELATIVE_16:
			hLoAddr = (*++cpu->PC);
			hHiAddr = (*++cpu->PC);
			cpu->holder.value = (hHiAddr << 8 | hLoAddr);
			cpu->holder.ptr = (cpu->PC + (s16) cpu->holder.value) + 1;
			return;
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
			hLoAddr = (*++cpu->PC);
			hHiAddr = (*++cpu->PC);
			hAddr = (hHiAddr << 8 | hLoAddr) + offset;
			cpu->resolver->get(cpu->DBR, hAddr);
			cpu->holder.ptr = &cpu->memory->bank_array[cpu->resolver->realBank][cpu->resolver->realAddr];
			printf("%X %X %X \n", hAddr, cpu->resolver->realBank, cpu->resolver->realAddr);
		break;
		case ADDRESS_ABSOLUTE_16_INDIRECT:
			hLoAddr = (*++cpu->PC);
			hHiAddr = (*++cpu->PC);
			hAddr = (hHiAddr << 8 | hLoAddr) + offset;
			cpu->resolver->get(cpu->DBR, hAddr);
			cpu->resolver->get(cpu->resolver->realBank, cpu->resolver->realAddr);
		break;
		case ADDRESS_ABSOLUTE_24:
				hLoAddr = (*++cpu->PC);
				hHiAddr = (*++cpu->PC);
				hBank = (*++cpu->PC);
				hAddr = (hHiAddr << 8 | hLoAddr);
				cpu->resolver->get(hBank, hAddr);
				cpu->holder.ptr = &cpu->memory->bank_array[cpu->resolver->realBank][cpu->resolver->realAddr];
				cpu->holder.value = *cpu->holder.ptr;
		break;
		case ADDRESS_ABSOLUTE_24_INDIRECT:
				hLoAddr = (*++cpu->PC);
				hHiAddr = (*++cpu->PC);
				hBank = (*++cpu->PC);
				hAddr = (hHiAddr << 8 | hLoAddr);
				cpu->resolver->get(hBank, hAddr); /* Get the real address */
				cpu->holder.ptr = &cpu->memory->bank_array[cpu->resolver->realBank][cpu->resolver->realAddr];
				cpu->holder.value = *cpu->holder.ptr;
		break;
		case ADDRESS_DP:
				cpu->holder.ptr = &cpu->memory->bank_array[cpu->DBR][cpu->DP + *++cpu->PC + offset];
				cpu->holder.value = *cpu->holder.ptr;
		break;
		case ADDRESS_DP_INDIRECT:
				hAddr = cpu->DP + *++cpu->PC + offset;
				cpu->resolver->get(cpu->DBR, hAddr);
				hLoAddr = cpu->memory->bank_array[cpu->resolver->realBank][cpu->resolver->realAddr];
				hHiAddr = cpu->memory->bank_array[cpu->resolver->realBank][cpu->resolver->realAddr + 1];
				hAddr = hHiAddr << 8 | hLoAddr;
				cpu->holder.ptr = &cpu->memory->bank_array[cpu->resolver->realBank][cpu->resolver->realAddr];
				cpu->holder.value = *cpu->holder.ptr;
		break;
		case ADDRESS_DP_24:
				printf("bus_error: this mode doesnt exist ADDRESS_DP_24\n");
		break;
		case ADDRESS_DP_24_INDIRECT:
				hAddr = (*(++cpu->PC)) + cpu->DP + offset;
				cpu->resolver->get(cpu->DBR, hAddr);

				hLoAddr = cpu->memory->bank_array[cpu->resolver->realBank][cpu->resolver->realAddr];
				hHiAddr = cpu->memory->bank_array[cpu->resolver->realBank][cpu->resolver->realAddr + 1];
				cpu->resolver->realBank = cpu->memory->bank_array[cpu->resolver->realBank][cpu->resolver->realAddr + 2];
				cpu->resolver->realAddr = (hHiAddr << 8 | hLoAddr);
				cpu->resolver->get(cpu->resolver->realBank, cpu->resolver->realAddr);

				cpu->holder.ptr = &cpu->memory->bank_array[cpu->resolver->realBank][cpu->resolver->realAddr];
		break;
	}

	switch (assign_mode) {
		case ASSIGN_NONE:
			if (!cpu->flags.Emulation) {
					cpu->holder.value = *cpu->holder.ptr;
			} else {
				cpu->holder.value = *(cpu->holder.ptr + 1);
				cpu->holder.value <<= 8;
				cpu->holder.value |= *(cpu->holder.ptr);
			}
			return;
		break;
		case ASSIGN_ACCUMULATOR:
			if (!cpu->flags.Accumulator) {
				/* i'm tired of using variables to assign bro */
				cpu->holder.value = *(cpu->holder.ptr + 1);
				cpu->holder.value <<= 8;
				cpu->holder.value |= *(cpu->holder.ptr);
			} else {
				cpu->holder.value = *cpu->holder.ptr;
			}
			return;
		break;
		case ASSIGN_INDEX:
			if (!cpu->flags.Index) {
				cpu->holder.value = *(cpu->holder.ptr + 1);
				cpu->holder.value <<= 8;
				cpu->holder.value |= *(cpu->holder.ptr);
			} else {
				cpu->holder.value = *cpu->holder.ptr;
			}
			return;
		break;
	}
	return;
}

static void busWriteCPU(snCPU* cpu, char mode, u16 value, u16 offset) {
	/* BUS_WRITE_16 writes sequentially on 2 addresses
	example write A:0xBBAA to Memory:0x0000 
	 0x0000 -> 0xAA; 0x0001 -> 0xBB  */
	if (mode == BUS_WRITE_16) {
		hLoByte = (u8) (value >> 8);
		hHiByte = (u8) value;
		*(cpu->holder.ptr + offset) = hHiByte;
		*(cpu->holder.ptr + offset + 1) = hLoByte;
		printf("writing %04X %02X %02X to [%X][%X] %p %p\n", value, hHiByte, hLoByte, cpu->resolver->realBank, cpu->resolver->realAddr + offset ,cpu->holder.ptr + offset, (cpu->holder.ptr + offset + 1));
	} else if (mode == BUS_WRITE_8) {
		*(cpu->holder.ptr + offset) = (u8) value;	
		printf("warning: casting to uint8_t\nwriting %02X to %X %04X %p\n", value, cpu->resolver->realBank, cpu->resolver->realAddr, cpu->holder.ptr + offset);
	}
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
	printf("cpu_bus: init\n");
	cpu->bus = bus;
	cpu->resolver->realAddr = 0x0000;
	cpu->resolver->realBank = 0x00;
	cpu->resolver->addrPtr = NULL;

	bus->memory = cpu->memory;
	bus->holder = &cpu->holder;
	bus->read = busReadCPU;
	bus->write = busWriteCPU;
	printf("cpu_bus: done\n");
}
