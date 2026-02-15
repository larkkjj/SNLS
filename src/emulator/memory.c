#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <malloc.h>

#include "general/types.h"

#include "core/dma.h"
#include "core/apu.h"
#include "core/bus.h"
#include "emulator/rom.h"
#include "emulator/main.h"
#include "emulator/memory.h"

u8 hBank = 0x00;
u16 hAddr = 0x0000;
u16 hLoAddr = 0x0000;
u16 hHiAddr = 0x0000;
u8 hLoByte;
u8 hHiByte;

static addrResolver resolver;

/* TODO: Make a new function unifying PPU, APU
 * DMA, and buffer setup, something like this
 * setupSystem(sn_PPU, mainDMA, ...) so we can
 * stop using #include all the time for 2 functions
 * only */

extern void attachROM(u8* buffer) {
	fread(buffer, sizeof(u8), 0x8000, rom_File);
	return;
};

static inline void resolveAddr(u8 index, u16 address) {
	printf("got address [%X]:[%X]\n", index, address);
	if (index >= 0x00 && index <= 0x3F) {
		if (address <= 0x2000) {
			 index = resolver.indexer->wramL;
		} else if (address >= 0x8000) {
			/* goes to rom*/
			address -= 0x8000;
		} else if (address >= 0x2100 && address <= 0x213F) {
			/* falls back to ppu! */
			address -= 0x2100;
			index = resolver.indexer->ppu;
		} else if (address >= 0x2140 && address <= 0x2143) {
			/* falls back to apu! */
			address -= 0x2140;
			index = resolver.indexer->apu;
		} else if (address == 0x4200 || address == 0x4201) {
			address -= 0x4200;
			index = bankCount + 3;
			printf("we're supposed to got joypad registers, but not implemented yet\n");
		} else if (address == 0x420B || address == 0x420C) {
			address -= 0x420B;
			index = resolver.indexer->dma;
		}
	} else if (index == 0x7E) {
		index = resolver.indexer->wramL;
	} else if (index == 0x7F) {
		index = resolver.indexer->wramH;
	}
	resolver.realAddr = address;
	resolver.realBank = index;
	return;
};

extern void setupResolver(indexer* index, snCPU* cpu) {
	resolver.get = resolveAddr;
	resolver.indexer = index;
	cpu->resolver = &resolver;
}

extern void assignToMap(u8** dest, u8** src, unsigned int offset, unsigned int count, unsigned int type) {
	for(unsigned int i = 0; i < count; i ++) {
		dest[offset + i] = src[i];
	};
}
