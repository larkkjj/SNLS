#ifndef SNLS_MAIN_HEADER
#define SNLS_MAIN_HEADER

#include "core/cpu.h"
#include "core/apu.h"
#include "core/dma.h"
#include "core/ppu.h"

/* definition of the above*/
typedef struct emGeneral {
	snCPU*		cpu;
	snPPU*		ppu;
	snAPU*		apu;
	snDMA*		dma;
	emMemory*	memory;
	u8*		active;

	void		(*mainFetch)(emGeneral* emulator);
	void		(*endfetch)(emGeneral* emulator);
} emGeneral;

#endif

