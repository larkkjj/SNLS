#ifndef SNLS_MEMORY_HEADER 
#define SNLS_MEMORY_HEADER

#include "general/types.h"
#include "core/ppu.h"
#include "core/dma.h"
#include "emulator/rom.h"

typedef struct emMemory {
	u8	bank_count;
	u16	bank_target;
	u16	address_target;
	u8*	pointerTarget; /* this is really useful */
	u8*	bank_array[];
} emMemory;

typedef struct snRAM {
	u8*	wRAM_lo;
	u8*	wRAM_hi;
	u8*	vRAM;
} snRAM;

typedef struct emROM {
	u8*	buffer;
} emROM;

extern u8 hLoByte;
extern u8 hHiByte;
extern u8 hBank;
extern u16 hAddr;
extern u16 hLoAddr;
extern u16 hHiAddr;

#endif
