#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include "emulator/main.h"
#include "general/types.h"
#include "emulator/memory.h"
#include "general/references.h"

static void fetchDMA(emGeneral* emulator) {
	*emulator->active |= 0x08;
	printf("dma_fetch: mdmaen: %X hdmaen: %X \n", emulator->dma->MdMaen, emulator->dma->HdMaen);
	/* setup 7 channels*/
	if (emulator->dma->MdMaen & 0x01) {
		//sleep(1);
		printf("channel 0 enabled \n");
	}  if (emulator->dma->MdMaen & 0x02) {
		//sleep(1);
		printf("channel 1 enabled \n");
	}  if (emulator->dma->MdMaen & 0x04) {
		//sleep(1);
		printf("channel 2 enabled \n");
	}  if (emulator->dma->MdMaen & 0x08) {
		//sleep(1);
		printf("channel 3 enabled \n");
	}  if (emulator->dma->MdMaen & 0x10) {
		//sleep(1);
		printf("channel 4 enabled \n");
	}  if (emulator->dma->MdMaen & 0x20) {
		//sleep(1);
		printf("channel 5 enabled \n");
	}  if (emulator->dma->MdMaen & 0x40) {
		//sleep(1);
		printf("channel 6 enabled \n");
	}  if (emulator->dma->MdMaen & 0x80)
		printf("channel 7 enabled \n");;
};

extern void setupDMA(emGeneral* emulator, u8** buffer) {
	emulator->dma->fetch = fetchDMA;

	buffer[0x0B] = &emulator->dma->MdMaen;
	buffer[0x0C] = &emulator->dma->HdMaen;

	emulator->dma->MdMaen = 0;
	emulator->dma->HdMaen = 0;

	printf("dma_setup: done\n");
	usleep(1000);
	return;
};

