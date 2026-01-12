#include <stdio.h>
#include <malloc.h>
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

static u8 syncCycle[0xFF];
static u8 syncCounter = 0x00;

static void mainFetch(emGeneral* emulator) {

		u8 a = 0x01; //cpu 0000 0001
		u8 b = 0x02; //ppu 0000 0010
		u8 c = 0x04; //spc 0000 0100
		u8 d = 0x08; //dma 0000 1000

	for(u8 i = 0; i < 0xFF; i ++) {
		syncCycle[i] |= a;
		if (syncCycle[i] % 3 == 0) {
			syncCycle[i] |= c;
		} else {
			continue;
		}
	}
	
	/* our main infinite loop' which is basically
	 * a sync cycle fetch */
	while (1) {

		pollWindow();
		if (syncCycle[syncCounter] & 0x01) {
				printf("sync_counter: fetching cpu...\n");
				emulator->cpu->fetch(emulator);
		} else if (syncCycle[syncCounter] & 0x02) {
				printf("sync_counter: fetching ppu...\n");
				emulator->ppu->fetch(emulator);
		} else if (syncCycle[syncCounter] & 0x04) {
				printf("sync_counter: fetching spc...\n");
				emulator->apu->spc->fetch(emulator);
		} else if (syncCycle[syncCounter] & 0x08) {
				printf("sync_counter: fetching DMA ...(at least we're supposed to\n");	
		}
	}
}

extern void mapPtrBank(emGeneral* emulator, unsigned int index, u8* bank_array[]) {
	/* this is loROM only */
	/* target, make 17 pointers, assuming the rom
	 * have 16 banks, so that would be
	 * 32KB * 16 + 32KB => 544KB*/
	emROM rom[index];
	/* this is where our fun beggins */
	for(unsigned int i = 0; i < index; i ++) {
		bank_array[i] = malloc(0x8000);
		rom[i].buffer = malloc(0x8000);
		fread(rom[i].buffer, sizeof(u8), 0x8000, rom_File);
		bank_array[i] = rom[i].buffer;
		free(rom[i].buffer);
	}

	general.memory->bank_count = index;

	general.ppu = ePPU;
	general.ppu->located = index;
	setupPPU(emulator, &bank_array[general.ppu->located]);

	general.apu = eAPU;
	general.apu->located = index + 1;
	setupSPC(emulator, eSPC, &bank_array[general.apu->located]);

	general.dma = eDMA;
	general.dma->located = index + 2;
	setupDMA(emulator, &bank_array[general.dma->located]);

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

	general.memory = &memory;

	eRAM->wRAM_lo = malloc(0x10000);
	eRAM->wRAM_hi = malloc(0x10000);
	mapPtrBank(&general, rom_Ptr->banks, memory.bank_array);
	fclose(rom_File);

	initWindow();

	/* basic setup */
	general.cpu = eCPU;
	general.active = malloc(1);
	*general.active = 0;
	general.mainFetch = mainFetch;
	setupCPU(&general, rom_Ptr);

	/* call our main fetch */
	mainFetch(&general);

	free(eCPU);
	free(ePPU);
	free(eDMA);

	return;
};
