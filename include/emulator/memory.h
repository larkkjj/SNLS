#ifndef SNLS_MEMORY_HEADER 
#define SNLS_MEMORY_HEADER

#include "general/types.h"
#include "general/references.h"

typedef struct memoryHolder {
	u8*	ptr;
	u16	value;
} memoryHolder;

typedef struct {
	char	ppu;
	char	apu;
	char	dma;
	char	wramH;
	char	wramL;
} indexer;

typedef struct {
	u16			value;
	u8			realBank;
	u16			realAddr;
	u8*			addrPtr;
	indexer*	indexer;
	void		(*get)(u8 realBank, u16 realAddress);
} addrResolver;

typedef struct emMemory {
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

extern u8 bankCount;
extern u8 hLoByte;
extern u8 hHiByte;
extern u8 hBank;
extern u16 hAddr;
extern u16 hLoAddr;
extern u16 hHiAddr;

extern void printMemory(u8* pointer, u16 count);

#endif
