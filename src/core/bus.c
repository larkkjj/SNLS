#include <stdio.h>
#include <unistd.h>

#include "core/bus.h"

#include "emulator/main.h"
#include "general/functions.h"
#include "general/types.h"
#include "emulator/memory.h"
 
extern u8* sn_Mwrite(emGeneral* emulator, u16 offset, u32 value) {
	printf("bus_write: writing %X to [%X][%X] %p \n", value, emulator->memory->bank_target, emulator->memory->address_target + offset, emulator->memory->pointerTarget);
	*emulator->memory->pointerTarget = value;
	return emulator->memory->pointerTarget;
};


extern u8 sn_Mread_u8_const(emGeneral* emulator, u16 offset) {
	holdAddr = (u8) *++emulator->cpu->PC + offset;
	return holdAddr;
};

extern u16 sn_Mread_u16_const(emGeneral* emulator, u16 offset) {
	holdLoAddr = *(++emulator->cpu->PC);
	holdHiAddr = *(++emulator->cpu->PC);
	holdAddr = (holdHiAddr << 8 | holdLoAddr) + offset;
	printf("bus_read: returning 16-bit-const [%X] \n", holdAddr);
	return holdAddr;
};



extern u8* sn_Mread_u24_absolute(emGeneral* emulator, u8 pagemode) {
	holdLoAddr = *(++emulator->cpu->PC);
	holdHiAddr = *(++emulator->cpu->PC);
	holdAddr = (holdHiAddr << 8 | holdLoAddr);

	printf("bus_read: got [0x%02X][%X]\n", *emulator->cpu->PC, holdAddr);
	printf("bus_read: returning [0x%02X][%X]\n", emulator->memory->bank_target, emulator->memory->address_target);

	getMappedBank(*++emulator->cpu->PC, holdAddr, emulator);
	return &emulator->memory->bank_array[emulator->memory->bank_target][emulator->memory->address_target];
}

static u8* writeSNES(emGeneral* ptrParent, u16 offset, u32 value) {
	printf("new bus_write: writing %X to [%X][%X] %p \n", value, ptrParent->bus->bank, ptrParent->bus->address + offset, ptrParent->bus->pointer);
	*ptrParent->bus->pointer = value;
	return ptrParent->bus->pointer;
};

static void readU16absolute(emGeneral* ptrParent, u16 offset) {
	holdLoAddr = *(++ptrParent->cpu->PC);
	holdHiAddr = *(++ptrParent->cpu->PC);
	holdAddr = (holdHiAddr << 8 | holdLoAddr) + offset;
	getMappedBank(ptrParent->cpu->DBR, holdAddr, ptrParent);
};

extern void setupBUS(emGeneral* emulator, snBUS* bus) {
	emulator->bus = bus;
	emulator->bus->ptrParent = emulator;
	emulator->bus->address = 0x0000;
	emulator->bus->bank = 0x00;
	emulator->bus->pointer = NULL;

	emulator->bus->readU16absolute = readU16absolute;
	emulator->bus->write = writeSNES;
};
