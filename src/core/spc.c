#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "general/references.h"
#include "emulator/main.h"
#include "core/apu.h"

static void (*snSPC_opcodes[])(emGeneral* emulator);

static void generic(emGeneral* emulator) {
	printf("spc_opcode: generic opcode / not implemented\n");
	return;
};

static void snSPC_MOV_A_const(emGeneral* emulator) {
	emulator->apu->spc->A = *++emulator->apu->spc->PC;
	++emulator->apu->spc->PC;
};

static void snSPC_MOV_X_const(emGeneral* emulator) {
	emulator->apu->spc->X = *++emulator->apu->spc->PC;
	++emulator->apu->spc->PC;
}

static void snSPC_MOV_SP_X(emGeneral* emulator) {
	emulator->apu->spc->SP = emulator->apu->spc->X;
	++emulator->apu->spc->PC;
}
static void setupIPL(emGeneral* emulator) {
	memcpy(&emulator->apu->internalRAM[0xFFC0], IPL_NTSC, 0x3F);
	for(unsigned int i = 0; i < 0x3F; i ++ ) {
		printf("%X \n", emulator->apu->internalRAM[0xFFC0 + i]);
	}
	return;
};

static void fetchSPC(emGeneral* emulator) {
	*emulator->active |= 0x04;
	(snSPC_opcodes[(*emulator->apu->spc->PC)])(emulator);
	printf("spc_fetch: %X %X %X %X %X \n", *emulator->apu->spc->PC, emulator->apu->spc->X, emulator->apu->spc->Y, emulator->apu->spc->SP, emulator->apu->spc->X);
	emulator->endfetch(emulator);
	return;
}

extern void setupSPC(emGeneral* emulator, snSPC* spc_ptr, u8** buffer, bool absolute) {
	/* APU != SPC 
	 * SPC-700 is a completely separated Processor 
	 * APU is the communicate interface
	 * DSL produces AUDIO */
	emulator->apu->spc = spc_ptr;
	emulator->apu->spc->fetch = fetchSPC;	
	emulator->apu->spc->PC = &emulator->apu->internalRAM[0xFFC0];

	buffer[0x00] = &emulator->apu->internalRAM[0xF4];
	buffer[0x01] = &emulator->apu->internalRAM[0xF5];
	buffer[0x02] = &emulator->apu->internalRAM[0xF6];
	buffer[0x03] = &emulator->apu->internalRAM[0xF7];

	setupIPL(emulator);

	snSPC_opcodes[0xCD] = snSPC_MOV_X_const;
	snSPC_opcodes[0xBD] = snSPC_MOV_SP_X;
	snSPC_opcodes[0xE8] = generic;

	return;
};

