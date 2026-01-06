#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>

#include "emulator/main.h"
#include "general/functions.h"
#include "general/types.h"
#include "emulator/memory.h"
#include "emulator/rom.h"
#include "core/cpu.h"

extern u8* sn_Mwrite(emGeneral* emulator, u8* address, u16 offset, u32 value) {
	printf("bus_write: writing %X to [%X][%X]\n", value, emulator->memory->bank_target, emulator->memory->address_target + offset);
	*address = value;
	return address;
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

extern u8* sn_Mread_u16_absolute(emGeneral* emulator, u16 offset) {
	holdLoAddr = *(++emulator->cpu->PC);
	holdHiAddr = *(++emulator->cpu->PC);
	holdAddr = (holdHiAddr << 8 | holdLoAddr) + offset;
	printf("bus_read: got [0x%02X][%X]\n",  emulator->cpu->DBR, holdAddr);
	emulator->memory->bank_target = (emulator->cpu->DBR);
	getMappedBank(emulator->memory->bank_target, holdAddr, emulator);
	printf("bus_read: returning [0x%02X][%X]\n", emulator->memory->bank_target, emulator->memory->address_target);
	return &emulator->memory->bank_array[emulator->memory->bank_target][emulator->memory->address_target];
};

extern u8* sn_Mread_u24_absolute(emGeneral* emulator) {
		holdLoAddr = *(++emulator->cpu->PC);
		holdHiAddr = *(++emulator->cpu->PC);
		holdAddr = (holdHiAddr << 8 | holdLoAddr);

		printf("bus_read: got [0x%02X][%X]\n", *emulator->cpu->PC, holdAddr);
		printf("bus_read: returning [0x%02X][%X]\n", emulator->memory->bank_target, emulator->memory->address_target);

		getMappedBank(*++emulator->cpu->PC, holdAddr, emulator);
		return &emulator->memory->bank_array[emulator->memory->bank_target][emulator->memory->address_target];
}
