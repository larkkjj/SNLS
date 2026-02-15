#ifndef SNLS_MAIN_HEADER
#define SNLS_MAIN_HEADER

#include "core/cpu.h"
#include "core/apu.h"
#include "core/dma.h"
#include "core/ppu.h"

/* definition of the above*/
typedef struct emGeneral {
	snCPU*		cpu;
	snRAM*		ram;
	snPPU*		ppu;
	snAPU*		apu;
	snDMA*		dma;
	cpuBUS*		bus;
	emMemory*	memory;
	u8*		active;

	bool	allowedFetch;
} emGeneral;

#endif

