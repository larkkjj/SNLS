#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <unistd.h>

#include "core/apu.h"
#include "core/bus.h"
#include "core/cpu.h"
#include "core/dma.h"
#include "core/ppu.h"

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
snBUS* eBUS = NULL;

emGeneral general;
emMemory memory;

typedef struct snesSync {
	u8	order[0xFF];
	u8	counter;
} snesSync;

u8 cpuOperation = 0x01;
u8 ppuOperation = 0x02;
u8 spcOperation = 0x04;
u8 dmaOperation = 0x08;

void mainFetch(emGeneral* emulator) {
	snesSync sync;
	for(int i = 0x00; i < 0xFF; i++) {
		sync.order[i] = cpuOperation;
		/* only cpu for testing */
	}
	/* our main infinite loop' which is basically
	 * a sync cycle fetch */
	while (1) {
		pollWindow();
		if (sync.order[sync.counter] & 0x01) {
			printf("sync_counter: fetching cpu...\n");
			emulator->cpu->fetch(emulator);
		}
		if (sync.order[sync.counter] & 0x02) {
			printf("sync_counter: fetching ppu...\n");
			emulator->ppu->fetch(emulator);
		}
		if (sync.order[sync.counter] & 0x04) {
			printf("sync_counter: fetching spc...\n");
			emulator->apu->spc->fetch(emulator);
		}
		if (sync.order[sync.counter] & 0x08) {
			printf("sync_counter: fetching DMA ...(at least we're supposed to\n");	
		}
		printf("%X %X \n", sync.counter, sync.order[sync.counter]);

		sync.counter++;
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
	general.memory->pointerTarget = malloc(sizeof(u8*));
	
	setupBUS(emulator, eBUS);

	general.ppu = ePPU;
	general.ppu->located = index;
	setupPPU(emulator, &bank_array[general.ppu->located]);

	general.apu = eAPU;
	general.apu->located = index + 1;
	setupSPC(emulator, eSPC, &bank_array[general.apu->located]);

	general.dma = eDMA;
	general.dma->located = index + 2;
	setupDMA(emulator, &bank_array[general.dma->located]);

	general.ram = eRAM;
	bank_array[index + 3] = general.ram->wRAM_lo;
	bank_array[index + 4] = general.ram->wRAM_hi;


	/* doing this for my own crazy sanity
	 * TODO: DUDE IT'S BEEN 1 WEEK WTF IS WRONG */
	printf("%p %p \n", bank_array[index + 3], general.ram->wRAM_lo);
	printf("%p %p \n", bank_array[index + 4], general.ram->wRAM_hi);
}

extern void initEmu(rom* rom_Ptr) {
	/*TODO: move all this to memory.c, where they belong*/
/*	for(unsigned int i = 0; i < 0x8000; i ++) {
		eRAM->wRAM_exp1[i] = calloc(1, sizeof(u8*));
		eRAM->wRAM_exp2[i] = calloc(1, sizeof(u8*));
	}
*/


	eBUS = malloc(sizeof(snBUS));
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
	setupCPU(&general, rom_Ptr);
	
	mainFetch(&general);

	free(ePPU);
	free(eSPC);
	free(eRAM);
	free(eCPU);
	free(ePPU);
	free(eDMA);

	return;
};
