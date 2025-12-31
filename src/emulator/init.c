#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "emulator/main.h"
#include "emulator/memory.h"
#include "emulator/gs.h"
#include "general/tools.h"
#include "general/functions.h"

#include "SDL.h"

snCPU* eCPU = NULL;
snSPC* eSPC = NULL;
snPPU* ePPU = NULL;
snDMA* eDMA = NULL;
snAPU* eAPU = NULL;
snRAM* eRAM = NULL;

emGeneral general;
emMemory memory;

static void endfetch(emGeneral* emulator) {
	printf("emulator: 0x%02X\n", *emulator->active);

	return;
};

static void mainFetch(emGeneral* emulator) {
	pollWindow();
	emulator->cpu->fetch(emulator);
	emulator->apu->spc->fetch(emulator);

	emulator->ppu->fetch(emulator);

	while (1) {
		
		printf("emulator: 0x%02X\n", *emulator->active);
		break;
	}	
}

static void mapPtrBank(emGeneral* emulator, uint index, u8* bank_array[]) {
	/* this is loROM only */
	/* target, make 17 pointers, assuming the rom
	 * have 16 banks, so that would be 
	 * 32KB * 16 + 32KB => 544KB*/
	emROM rom[index];
	/* this is where our fun beggins */
	for(uint i = 0; i < index; i ++) {
		bank_array[i] = malloc(0x8000);
		rom[i].buffer = malloc(0x8000);
		fread(rom[i].buffer, sizeof(u8), 0x8000, rom_File);
		bank_array[i] = rom[i].buffer;
		free(rom[i].buffer);
	}

	general.memory->bank_count = index;
	general.ppu = ePPU;
	//bank_array[index] = mMap.ppu_addr;
	general.ppu->located = index;
	setupPPU(emulator, &bank_array[general.ppu->located], false);

	general.apu = eAPU;
	//bank_array[index + 1] = mMap.apu_addr;
	general.apu->located = index + 1;
	setupSPC(emulator, eSPC, &bank_array[general.apu->located], false);

	general.dma = eDMA;
	//bank_array[index + 2] = mMap.dma_addr;
	general.dma->located = index + 2;
	setupDMA(emulator, &bank_array[general.dma->located], false);

	bank_array[index + 3] = eRAM->wRAM_lo; /* if our rom have 16 banks, our RAM 
	will be located at 16 + 3 = 19 and above */
	bank_array[index + 4] = eRAM->wRAM_hi;
	
}

extern void initEmu(rom* rom_Ptr) {
	/*TODO: move all this to memory.c, where they belong*/
/*	for(unsigned int i = 0; i < 0x8000; i ++) {
		eRAM->wRAM_exp1[i] = calloc(1, sizeof(u8*));
		eRAM->wRAM_exp2[i] = calloc(1, sizeof(u8*));
	}
*/
	
	ePPU = malloc(sizeof(snPPU));
	eDMA = malloc(sizeof(snDMA));
	eAPU = malloc(sizeof(snAPU));
	eCPU = malloc(sizeof(snCPU));
	eSPC = malloc(sizeof(snSPC));
	eRAM = malloc(sizeof(snRAM));

	eRAM->wRAM_lo = malloc(0x10000);
	eRAM->wRAM_hi = malloc(0x10000);
	general.memory = &memory;

	mapPtrBank(&general, rom_Ptr->banks, memory.bank_array);
	fclose(rom_File);

	initWindow();

	/* basic setup */
	general.cpu = eCPU;
	general.active = malloc(1);
	*general.active = 0;
	general.mainFetch = mainFetch;
	general.endfetch = endfetch;
	setupCPU(&general, rom_Ptr);

	/* call our main fetch */
	while (1) {
		mainFetch(&general);
	}
	free(eCPU);
	free(ePPU);
	free(eDMA);

	return;
};
