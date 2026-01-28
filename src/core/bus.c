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

static void writeSNES(snesBUS* busParent, u16 value) {
	printf("bus_write: writing %X to [%X][%X] %p \n", value, busParent->bank, busParent->address, busParent->pointer);
	*busParent->pointer = value;
};

static void writeSNESindirect(snesBUS* busParent, u16 value, u8 mode) {
	printf("bus_write: not implemented... \n");
	switch (mode) {
	
		/* 0x00 -> PC */
		/* 0x01 -> AC */
		/* 0x02 -> DP */
		/* 0x03 -> X  */
		/* 0x04 -> Y  */
		case 0x00:

		break;
		case 0x01:

		break;
		case 0x02:

		break;

		case 0x03:

		break;
	}
};

static void writeSNESindirect_SPC(spcBUS* busParent, u16 offset, u16 value, u8 mode) {
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
			busParent->address = (busParent->emulator->apu->spc->X) + offset;
		break;
		case 0x05:
			busParent->address = ((*++busParent->emulator->apu->spc->PC) | busParent->emulator->apu->spc->DP) + offset;
		break;
		case 0x07:
			busParent->address = (busParent->emulator->apu->spc->DP | busParent->emulator->apu->spc->X) + offset;
		break;
		default:
			printf("spc_bus: unknown mode %X \n");
			exit(1);
		break;
	};
	busParent->value = value;
	printf("spc_bus_write_indirect: writing %X to %X \n", busParent->value, busParent->address);
	busParent->pointer = &busParent->emulator->apu->internalRAM[busParent->address];
	*busParent->pointer = busParent->value;
};


static void writeSNES_SPC(spcBUS* busParent, u16 value) {
	printf("spc_bus_write: writing %X to [%X] \n", value, busParent->address);
	*busParent->pointer = value;
};

static void readU8const(snesBUS* busParent, u16 offset) {
	busParent->value = *(++busParent->emulator->cpu->PC + offset);
};

static void readU8const_SPC(spcBUS* busParent, u16 offset) {
	busParent->value = *(++busParent->emulator->apu->spc->PC + offset);
};

static void readU16absolute(snesBUS* busParent, u16 offset) {
	hLoAddr = *(++busParent->emulator->cpu->PC);
	hHiAddr = *(++busParent->emulator->cpu->PC);
	hAddr = (hHiAddr << 8 | hLoAddr) + offset;
	getMappedBank(busParent->emulator->cpu->DBR, hAddr, busParent->emulator);
};

static void readU16absoluteIndirect(snesBUS* busParent, u16 offset, u8 mode) {
	/* kinda specific-use */
	switch (mode) {
		case 0x00:
			hLoAddr = *(++busParent->emulator->cpu->PC);
			hHiAddr = *(++busParent->emulator->cpu->PC);
			hAddr = (hHiAddr << 8 | hLoAddr) + offset;
			printf("%X %X \n", hAddr, hAddr + 1);


			getMappedBank(busParent->emulator->cpu->DBR, hAddr, busParent->emulator);
			hLoByte = busParent->value;
			getMappedBank(busParent->emulator->cpu->DBR, hAddr + 1, busParent->emulator);
			hHiByte = busParent->value;
			busParent->value = (hHiByte << 8 | hLoByte);

			break;
		default:
	
		break;
	}
};

static void readU16absolute_SPC(spcBUS* busParent, u16 offset) {

	hLoAddr = *(++busParent->emulator->cpu->PC);
	hHiAddr = *(++busParent->emulator->cpu->PC);
	hAddr = (hHiAddr << 8 | hLoAddr) + offset;
	printf("bus_spc: WARNING, code notimplemented due to SPC RAM implementation \n");
	printf("bus_spc: add it in the get Mapped Bank function \n");

};

static void readU16absoluteIndirect_SPC(spcBUS* busParent, u8 mode) {

	/* 0x00 -> PC */
	/* 0x01 -> AC */
	/* 0x02 -> DP */
	/* 0x03 -> X  */
	/* 0x04 -> Y  */
	/* 0x05 -> DP + X */
	/* 0x06 -> DP + Y */

	switch (mode) {
		case 0x02:
			busParent->value = busParent->emulator->apu->internalRAM[busParent->emulator->apu->spc->DP];
		break;
		default:

		break;
	}

};

static void readU16const(snesBUS* busParent) {
	hLoAddr = *(++busParent->emulator->cpu->PC);
	hHiAddr = *(++busParent->emulator->cpu->PC);
	hAddr = (hHiAddr << 8 | hLoAddr); 
	busParent->value = hAddr;
	printf("bus_read: returning const address %X \n", busParent->value);
};

static void readU16const_SPC(spcBUS* busParent) {
	hLoAddr = *(++busParent->emulator->apu->spc->PC);
	hHiAddr = *(++busParent->emulator->apu->spc->PC);
	hAddr = hHiAddr << 8 | hLoAddr;
	busParent->value = hAddr;
}

static void readU24absolute(snesBUS* busParent, u16 offset) {
	hLoAddr = *(++busParent->emulator->cpu->PC);
	hHiAddr = *(++busParent->emulator->cpu->PC);
	hAddr = (hHiAddr << 8 | hLoAddr) + offset;
	hBank = *(++busParent->emulator->cpu->PC);
	printf("bus_read: got [0x%02X][%X]\n", hBank, hAddr);
	getMappedBank(hBank, hAddr, busParent->emulator);
	printf("bus_read: returning [0x%02X][%X]\n", busParent->bank, busParent->address);
};

extern void setupSPCBUS(emGeneral* emulator, spcBUS* busParent) {
	//emulator->apu->spc->bus = busParent;
	/* We just make a callback to emulatorPtr
	 * So we can still acess something from out */
	busParent->emulator = emulator;
	busParent->address = 0x0000;
	busParent->value = 0x0000;
	busParent->pointer = NULL;

	busParent->readU8const = readU8const_SPC;
	busParent->readU16const = readU16const_SPC;
	busParent->readU16absolute = readU16absolute_SPC;
	busParent->readU16absoluteIndirect = readU16absoluteIndirect_SPC;
	busParent->write = writeSNES_SPC;
	busParent->writeIndirect = writeSNESindirect_SPC;
}

extern void setupOpenBUS(emGeneral* emulator, snesBUS* busParent) {
	emulator->bus = busParent;
	busParent->emulator = emulator;
	busParent->address = 0x0000;
	busParent->bank = 0x00;
	busParent->pointer = NULL;

	busParent->readU8const = readU8const;
	busParent->readU16absolute = readU16absolute;
	busParent->readU16absoluteIndirect = readU16absoluteIndirect;
	busParent->readU16const = readU16const;
	busParent->readU24absolute = readU24absolute;
	busParent->write = writeSNES;
	busParent->writeIndirect = writeSNESindirect;
}

