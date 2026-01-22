#include <stdio.h>
#include <unistd.h>
#include <malloc.h>

#include "general/types.h"

#include "core/dma.h"
#include "core/apu.h"
#include "core/bus.h"
#include "emulator/main.h"
#include "emulator/memory.h"

/*u8* memory_vMap = NULL;
u8* memory_wRAM = NULL;
u8* memory_sPPU = NULL;
u8* memory_iCPU = NULL;
u8* memory_rDMA = NULL;*/

u8 holdBankAddr = 0x00;
u16 holdAddr = 0x0000;
u16 holdLoAddr = 0x0000;
u16 holdHiAddr = 0x0000;
u16 temp_address;
u8 temp_bank;

/* TODO: Make a new function unifying PPU, APU
 * DMA, and buffer setup, something like this
 * setupSystem(sn_PPU, mainDMA, ...) so we can
 * stop using #include all the time for 2 functions
 * only */

extern void attachROM(u8* buffer) {
	fread(buffer, sizeof(u8), 0x8000, rom_File);
	return;
};

extern void getMappedBank(u8 index, u16 address, emGeneral* emulator) {
	if (index >= 0x00 && index <= 0x3F) {
		if (address >= 0x8000) {
			/* goes to rom*/
			temp_address = address - 0x8000;
			temp_bank = index;
		} else if (address >= 0x2100 && address <= 0x213F){
			/* falls back to ppu! */
			temp_address = address - 0x2100;
			temp_bank = emulator->ppu->located;
		} else if (address >= 0x2140 && address <= 0x2143) {
			/* falls back to apu! */
			temp_address = address - 0x2140;
			temp_bank = emulator->apu->located;
		} else if (address == 0x4200 || address == 0x4201) {
			temp_address = address - 0x4200;
			temp_bank = emulator->memory->bank_count + 3;
			printf("we're supposed to got joypad registers, but not implemented yet\n");
		} else if (address == 0x420B || address == 0x420C) {
			temp_address = address - 0x420B;
			temp_bank = emulator->dma->located;
		}
	} else if (index == 0x7E) {
		temp_address = address;
		temp_bank = emulator->memory->bank_count + 3;
	} else if (index == 0x7F) {
		temp_address = address;
		temp_bank = emulator->memory->bank_count + 4;
	}
	printf("get_mapped_bank: bank: %X address: %X \n", index, address);
	emulator->bus->address = temp_address;
	emulator->bus->bank = temp_bank;
	emulator->bus->pointer = &emulator->memory->bank_array[temp_bank][temp_address];
	emulator->bus->value = *emulator->bus->pointer; 
	return;
};

extern void assignToMap(u8** dest, u8** src, unsigned int offset, unsigned int count, unsigned int type) {
	/* don't misunderstand with a memcpy alternative
	 * this references a map to another one */
	/* double is used here 'cause of rom buffer that uses
	 * a single pointer */

	/* source[0x8000]  ->  destination[0x133]
	 * source[0x8001]  ->  destination[0x134]*/
	/* 2 = u8 */
	for(unsigned int i = 0; i < count; i ++) {
		dest[offset + i] = src[i];
	};
}

extern void setupSystem(u8* buffer, snPPU* ppu, snDMA* dma) {
	/*for(unsigned int i = 0; i < 0xFF; i ++) {
		mBank[i] = malloc(sizeof(u8));
	};

	for(unsigned int i = 0; i < rom_Ptr->banks; i ++) {
		mMemory_ptr[i] = malloc(sizeof(u8));
		mMemory_ptr[i]->buffer = malloc(0x10000);
		//attachROM(mMemory_ptr[i]->buffer, rom_Ptr);
		mapPPU(ePPU, mMemory_ptr[i]->buffer);

		mBank[i] = *mMemory_ptr[i]->buffer;
	}*/
};
