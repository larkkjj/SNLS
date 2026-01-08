#include <stdio.h>
#include <unistd.h>
#include <malloc.h>
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

static void snSPC_MOV_A_x(emGeneral* emulator) {};
static void snSPC_MOV_A_xplus(emGeneral* emulator) {};
static void snSPC_MOV_A_DP(emGeneral* emulator) {};
static void snSPC_MOV_A_DP_X(emGeneral* emulator) {};
static void snSPC_MOV_A_abs(emGeneral* emulator) {};
static void snSPC_MOV_A_abs_X(emGeneral* emulator) {};
static void snSPC_MOV_A_abs_Y(emGeneral* emulator) {};
static void snSPC_MOV_A_dp_x(emGeneral* emulator) {};
static void snSPC_MOV_A_dp_Y(emGeneral* emulator) {};
static void snSPC_MOV_X_const(emGeneral* emulator) {
	emulator->apu->spc->X = *++emulator->apu->spc->PC;
	++emulator->apu->spc->PC;
}

static void snSPC_MOV_X_dp(emGeneral* emulator) {};
static void snSPC_MOV_DP_Y(emGeneral* emulator) {};
static void snSPC_MOV_X_abs(emGeneral* emulator) {};
static void snSPC_MOV_Y_const(emGeneral* emulator) {
    emulator->apu->spc->Y = *++emulator->apu->spc->PC;
    ++emulator->apu->spc->PC;
};

static void snSPC_MOV_Y_DP(emGeneral* emulator) {};
static void snSPC_MOV_Y_DP_X(emGeneral* emulator) {};
static void snSPC_MOV_Y_abs(emGeneral* emulator) {};

/* copy register to register */
static void snSPC_MOV_A_X(emGeneral* emulator) {
    emulator->apu->spc->A = emulator->apu->spc->X;
    ++emulator->apu->spc->PC;
};

static void snSPC_MOV_A_Y(emGeneral* emulator) {
    emulator->apu->spc->A = emulator->apu->spc->Y;
    ++emulator->apu->spc->PC;
};

static void snSPC_MOV_X_A(emGeneral* emulator) {
    emulator->apu->spc->X = emulator->apu->spc->A;
    ++emulator->apu->spc->PC;
};

static void snSPC_MOV_Y_A(emGeneral* emulator) {
    emulator->apu->spc->Y = emulator->apu->spc->A;
    ++emulator->apu->spc->PC;
};
static void snSPC_MOV_X_SP(emGeneral* emulator) {
    emulator->apu->spc->X = emulator->apu->spc->SP;
    ++emulator->apu->spc->PC;
};
static void snSPC_MOV_SP_X(emGeneral* emulator) {
	emulator->apu->spc->SP = emulator->apu->spc->X;
	++emulator->apu->spc->PC;
}
static void snSPC_MOV_DP_DP(emGeneral* emulator) {
    /* what does this shit do ???? */
    //emulator->apu->spc->DP;
    ++emulator->apu->spc->PC;
};

static void snSPC_MOV_DP_const(emGeneral* emulator) {
    //emulator->apu->spc->DP = ++emulator->cpu->spc->PC;
    ++emulator->apu->spc->PC;
};


static void setupIPL(emGeneral* emulator, u8 type) {
    /* NTSC SNES */
    if (type == 0x00) {
        memcpy(&emulator->apu->internalRAM[0xFFC0], IPL_NTSC, 0x3F);
    }
	for(unsigned int i = 0; i < 0x3F; i ++ ) {
		printf("%X \n", emulator->apu->internalRAM[0xFFC0 + i]);
	}
	return;
};

static void fetchSPC(emGeneral* emulator) {
	*emulator->active |= 0x04;
    (snSPC_opcodes[(*emulator->apu->spc->PC)])(emulator);

	printf("spc_fetch: %X %X %X %X %X \n", *emulator->apu->spc->PC, emulator->apu->spc->X, emulator->apu->spc->Y, emulator->apu->spc->SP, emulator->apu->spc->X);
	return;
}

extern void setupSPC(emGeneral* emulator, snSPC* spc_ptr, u8** buffer) {
	/* APU != SPC
	 * SPC-700 is a completely separated Processor
	 * APU is the communicate interface
	 * DSL produces AUDIO */
	setupIPL(emulator, 0x00);
	emulator->apu->spc = spc_ptr;
	emulator->apu->spc->fetch = fetchSPC;
	emulator->apu->spc->PC = &emulator->apu->internalRAM[0xFFC0];

	buffer[0x00] = &emulator->apu->internalRAM[0xF4];
	buffer[0x01] = &emulator->apu->internalRAM[0xF5];
	buffer[0x02] = &emulator->apu->internalRAM[0xF6];
	buffer[0x03] = &emulator->apu->internalRAM[0xF7];


	snSPC_opcodes[0xCD] = snSPC_MOV_X_const;
	snSPC_opcodes[0xC6] = generic;
	snSPC_opcodes[0xE8] = snSPC_MOV_A_const;
	snSPC_opcodes[0x8D] = snSPC_MOV_Y_const;
	snSPC_opcodes[0xBD] = snSPC_MOV_SP_X;

	return;
};
