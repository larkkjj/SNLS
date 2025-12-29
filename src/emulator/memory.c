#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>

#include "general/types.h"

#include "core/dma.h"
#include "core/apu.h"
#include "emulator/rom.h"
#include "emulator/main.h"
#include "emulator/memory.h"
#include "core/ppu.h"

/*u8* memory_vMap = NULL;
u8* memory_wRAM = NULL;
u8* memory_sPPU = NULL;
u8* memory_iCPU = NULL;
u8* memory_rDMA = NULL;*/


u8 holdBankAddr;
u16 holdAddr;
u16 holdLoAddr;
u16 holdHiAddr;

/* TODO: Make a new function unifying PPU, APU
 * DMA, and buffer setup, something like this
 * setupSystem(sn_PPU, mainDMA, ...) so we can
 * stop using #include all the time for 2 functions
 * only */

extern void attachROM(u8* buffer) {
	fread(buffer, sizeof(u8), 0x8000, rom_File);
	return;
};

extern u8 getMappedBank(u8 index, u16 address, emGeneral* emulator) {
	if (index >= 0x00 && index <= 0x3F) {
		if (address >= 0x8000) {
			/* goes to rom*/
			emulator->memory->address_target = address - 0x8000;
			emulator->memory->bank_target = index;
		} else if (address >= 0x2100 && address <= 0x213F){
			/* falls back to ppu! */
			emulator->memory->address_target = address - 0x2100;
			emulator->memory->bank_target = emulator->ppu->located;
		} else if (address >= 0x2140 && address <= 0x2143) {
			/* falls back to apu! */
			emulator->memory->address_target = address - 0x2140;
			emulator->memory->bank_target = emulator->apu->located;
		} else if (address == 0x420B || address == 0x420C) {
			emulator->memory->address_target = address - 0x420B;	
			emulator->memory->bank_target = emulator->dma->located;
		}
		printf("%X \n", emulator->memory->address_target);
			sleep(1);
	} else if (index >= 0x7E) {
		emulator->memory->address_target = address;
		emulator->memory->bank_target = emulator->memory->bank_count + 3;
	} else if (index <= 0x7F) {
		emulator->memory->address_target = address;
		emulator->memory->bank_target = emulator->memory->bank_count + 4;
	}
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


