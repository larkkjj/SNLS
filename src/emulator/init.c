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

snCPU* eCPU = NULL;
snSPC* eSPC = NULL;
snPPU* ePPU = NULL;
snDMA* eDMA = NULL;
snAPU* eAPU = NULL;
snRAM* eRAM = NULL;

cpuBUS* cBUS = NULL;

static indexer index;
static emGeneral general;
static emMemory memory;

u8 bankCount = 0;

typedef struct snesSync {
	u8	index;
	u8	row[];
} snesSync;

/* name changed from sync to syncOrder due do
sync function conflit which caused a segfault */
snesSync syncOrder;

u8 cpuOperation = 0x01;
u8 ppuOperation = 0x02;
u8 spcOperation = 0x04;
u8 dmaOperation = 0x08;

void mainFetch(emGeneral* emulator) {
	emulator->allowedFetch = 0;
	for(unsigned int i = 0x00; i <= 0xFF; i++) {
		syncOrder.row[i] = cpuOperation;
		if (i % 3 == 0) {
			syncOrder.row[i] |= spcOperation;
		} else if (i % 5 == 0) {
			syncOrder.row[i] |= ppuOperation;
		}
	}
	/* our main infinite loop' which is basically
	 * a sync cycle fetch */
	while (1) {
		#ifdef SNLS_WITH_WINDOW
		pollWindow(emulator);
		#endif
		if (syncOrder.row[syncOrder.index] & 0x01) {
			printf("sync_counter: fetching cpu...\n");
			emulator->cpu->fetch(emulator->cpu);
		}
		if (syncOrder.row[syncOrder.index] & 0x02) {
			printf("sync_counter: fetching ppu...\n");
			emulator->ppu->fetch(emulator->ppu);
		}
		if (syncOrder.row[syncOrder.index] & 0x04) {
			printf("sync_counter: fetching spc...\n");
			emulator->apu->spc->fetch(emulator->apu->spc);
		}
		if (syncOrder.row[syncOrder.index] & 0x08) {
			printf("sync_counter: fetching DMA ...(at least we're supposed to\n");	
		}
		printf("%X %X \n", syncOrder.index, syncOrder.row[syncOrder.index]);

		if (syncOrder.index == 0xFF) {
			printf("sync_counter: we made 256 fetches (CPU + SPC + PPU)\n");
		}
		syncOrder.index++;
	}
}

static void mapPtrBank(emGeneral* emulator, unsigned int count, u8* bank_array[]) {
	/* this is loROM only */
	/* target, make 17 pointers, assuming the rom
	 * have 16 banks, so that would be
	 * 32KB * 16 + 32KB => 544KB*/

	/* this is where our fun beggins */
	for(unsigned int i = 0; i < count; i ++) {
		bank_array[i] = malloc(0x8000);
		fread(bank_array[i], sizeof(u8), 0x8000, rom_File);
	}

	bankCount = count;
	general.memory->pointerTarget = malloc(sizeof(u8*));
	

	general.ppu = ePPU;
	index.ppu = count;
	setupPPU(emulator, &bank_array[index.ppu]);

	general.apu = eAPU;
	index.apu = count + 1;
	setupSPC(emulator, eSPC, &bank_array[index.apu]);

	general.dma = eDMA;
	index.dma = count + 2;
	setupDMA(emulator, &bank_array[index.dma]);

	general.ram = eRAM;
	index.wramL = count + 3;
	index.wramH = count + 4;
	bank_array[index.wramL] = general.ram->wRAM_lo;
	bank_array[index.wramH] = general.ram->wRAM_hi;


	/* doing this for my own crazy sanity
	 * TODO: DUDE IT'S BEEN 1 WEEK WTF IS WRONG */
	printf("%p %p \n", bank_array[index.wramL], general.ram->wRAM_lo);
	printf("%p %p \n", bank_array[index.wramH], general.ram->wRAM_hi);
	/* NOT TODO: YEA I SOLVED IT, I'M CRAZY HAHAHAHA */
}

extern void initEmu(rom* rom_Ptr) {
	cBUS = malloc(sizeof(cpuBUS));
	ePPU = malloc(sizeof(snPPU));
	eDMA = malloc(sizeof(snDMA));
	eAPU = malloc(sizeof(snAPU));
	eCPU = malloc(sizeof(snCPU));
	eSPC = malloc(sizeof(snSPC));
	eRAM = malloc(sizeof(snRAM));

	eRAM->wRAM_lo = malloc(0x10000);
	eRAM->wRAM_hi = malloc(0x10000);
	general.memory = &memory;

	printf("resources allocated \n");
	mapPtrBank(&general, rom_Ptr->banks, memory.bank_array);
	fclose(rom_File);

	#ifdef SNLS_WITH_WINDOW
		initWindow();
	#endif

	/* basic setup */
	general.cpu = eCPU;
	setupCPU(&general, rom_Ptr);
	setupResolver(&index, eCPU);
	setupCPUstack(&general.cpu->stack);
	setupCPUBUS(general.cpu, cBUS); /* our open-bus */
	
	mainFetch(&general);

	free(ePPU);
	free(eSPC);
	free(eRAM);
	free(eCPU);
	free(ePPU);
	free(eDMA);

	for(unsigned int i = 0; i < bankCount; i ++) {
		free(general.memory->bank_array[i]);
	}

	return;
};
