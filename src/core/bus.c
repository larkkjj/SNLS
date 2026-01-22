#include <stdio.h>
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

static void writeSNES(snesBUS* busParent, u32 value) {
	printf("bus_write: writing %X to [%X][%X] %p \n", value, busParent->bank, busParent->address, busParent->pointer);
	*busParent->pointer = value;
};

static void writeSNES_SPC(spcBUS* busParent, u32 value) {
	printf("bus_write: writing %X to [%X][%X] %p \n", value, busParent->bank, busParent->address, busParent->pointer);
	*busParent->pointer = value;
};

static void readU8const(snesBUS* busParent, u16 offset) {
	busParent->value = *(++busParent->emulator->cpu->PC + offset);
};

static void readU8const_SPC(spcBUS* busParent, u16 offset) {
	busParent->value = *(++busParent->emulator->apu->spc->PC + offset);
};

static void readU8absolute_SPC(spcBUS* busParent, u16 offset) {
	busParent->address = *(++busParent->emulator->apu->spc->PC + offset);
	busParent->value = busParent->emulator->apu->internalRAM[busParent->address];
};


static void readU16absolute(snesBUS* busParent, u16 offset) {
	holdLoAddr = *(++busParent->emulator->cpu->PC);
	holdHiAddr = *(++busParent->emulator->cpu->PC);
	holdAddr = (holdHiAddr << 8 | holdLoAddr) + offset;
	getMappedBank(busParent->emulator->cpu->DBR, holdAddr, busParent->emulator);
};

static void readU16absoluteAddr(snesBUS* busParent, u16 offset) {
	/* kinda specific-use */
	u8 holdLoByte;
	u8 holdHiByte;

	holdLoAddr = *(++busParent->emulator->cpu->PC);
	holdHiAddr = *(++busParent->emulator->cpu->PC);
	holdAddr = (holdHiAddr << 8 | holdLoAddr) + offset;


	getMappedBank(busParent->emulator->cpu->DBR, holdAddr, busParent->emulator);
	holdLoByte = busParent->value;

	getMappedBank(busParent->emulator->cpu->DBR, ++holdAddr, busParent->emulator);
	holdHiByte = busParent->value;

	busParent->value = (holdHiByte << 8 | holdLoByte);
};


static void readU16absolute_SPC(spcBUS* busParent, u16 offset) {
	holdLoAddr = *(++busParent->emulator->cpu->PC);
	holdHiAddr = *(++busParent->emulator->cpu->PC);
	holdAddr = (holdHiAddr << 8 | holdLoAddr) + offset;
	printf("bus_spc: WARNING, code notimplemented due to SPC RAM implementation \n");
	printf("bus_spc: add it in the get Mapped Bank function \n");
	sleep(1);

};

static void readU16const(snesBUS* busParent) {
	holdLoAddr = *(++busParent->emulator->cpu->PC);
	holdHiAddr = *(++busParent->emulator->cpu->PC);
	holdAddr = (holdHiAddr << 8 | holdLoAddr); 
	busParent->value = holdAddr;
	printf("bus_read: returning const address %X \n", busParent->value);
};

static void readU16const_SPC(spcBUS* busParent) {
	holdLoAddr = *(++busParent->emulator->apu->spc->PC);
	holdHiAddr = *(++busParent->emulator->apu->spc->PC);
	holdAddr = holdHiAddr << 8 | holdLoAddr;
	busParent->value = holdAddr;
}

static void readU24absolute(snesBUS* busParent, u16 offset) {
	holdLoAddr = *(++busParent->emulator->cpu->PC);
	holdHiAddr = *(++busParent->emulator->cpu->PC);
	holdAddr = (holdHiAddr << 8 | holdLoAddr) + offset;
	printf("bus_read: got [0x%02X][%X]\n", *busParent->emulator->cpu->PC, holdAddr);
	printf("bus_read: returning [0x%02X][%X]\n", busParent->bank, busParent->address);
	getMappedBank(*++busParent->emulator->cpu->PC, holdAddr, busParent->emulator);
};

extern void setupSPCBUS(emGeneral* emulator, spcBUS* busParent) {
	//emulator->apu->spc->bus = busParent;
	busParent->emulator = emulator;
	busParent->address = 0x0000;
	busParent->bank = 0x00;
	busParent->pointer = NULL;

	busParent->readU8const = readU8const_SPC;
	busParent->readU16const = readU16const_SPC;
	busParent->readU16absolute = readU16absolute_SPC;
	busParent->write = writeSNES_SPC;
}

extern void setupOpenBUS(emGeneral* emulator, snesBUS* busParent) {
	emulator->bus = busParent;
	busParent->emulator = emulator;
	busParent->address = 0x0000;
	busParent->bank = 0x00;
	busParent->pointer = NULL;

	busParent->readU8const = readU8const;
	busParent->readU16absolute = readU16absolute;
	busParent->readU16absoluteAddr = readU16absoluteAddr;
	busParent->readU16const = readU16const;
	busParent->readU24absolute = readU24absolute;
	busParent->write = writeSNES;
}

