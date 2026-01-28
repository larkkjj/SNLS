#include <stdio.h>
#include <unistd.h>
#include <malloc.h>
#include <string.h>

#include "general/references.h"
#include "general/functions.h"
#include "emulator/main.h"
#include "core/apu.h"
#include "core/bus.h"

static spcBUS* spcBUS_ptr;

static void updatePSW(u8* PSW, u16 value, u8 flags) {
	printf("PSW: %X \n", *PSW);
	/*for(unsigned int i = 0x00; i < 0x80; i ++) {
		if (flags & i) {
			*PSW |= i;
		}
	}*/

	if (flags & SPC_C_FLAG) {
		*PSW |= SPC_C_FLAG;
	}
	if (flags & SPC_Z_FLAG) /* ZERO FLAG */ {
		if (value == 0) {
			*PSW |= SPC_Z_FLAG;
		} else {
			(*PSW &= ~SPC_Z_FLAG);
		}
	}
	if (flags & SPC_I_FLAG) {
		*PSW |= SPC_I_FLAG;
	}
	if (flags & SPC_H_FLAG) {
		*PSW |= SPC_H_FLAG;
	}
	if (flags & SPC_B_FLAG) {
		*PSW |= SPC_B_FLAG;
	}
	if (flags & SPC_P_FLAG) {
		*PSW |= SPC_P_FLAG;
	}
	if (flags & SPC_V_FLAG) {
		*PSW |= SPC_V_FLAG;
	}
	if (flags & SPC_N_FLAG) /* NEGATIVE FLAG */{
		if (value < 0) {
			*PSW |= SPC_N_FLAG;
		} else {
			(*PSW &= ~SPC_N_FLAG);
		}
	}
};


static void generic(emGeneral* emulator) {
	printf("spc_opcode: last_status: %X %X %X %X %X %X \n", \
			emulator->apu->spc->PC, \
			emulator->apu->spc->A, \
			emulator->apu->spc->DP, \
			emulator->apu->spc->X, \
			emulator->apu->spc->Y, \
			emulator->apu->spc->YA \
	      );
	printf("spc_registers: %X %X %X %X\n", emulator->apu->internalRAM[0xF4], \
		emulator->apu->internalRAM[0xF5], emulator->apu->internalRAM[0xF6], \
		emulator->apu->internalRAM[0xF7]);
	printf("spc_opcode: %X generic opcode / not implemented\n", *emulator->apu->spc->PC);
	return;
};

static void snSPC_BRA(emGeneral* emulator) {
	emulator->apu->spc->bus->readU8const(spcBUS_ptr, 0x00);
	emulator->apu->spc->PC++;
};

static void snSPC_BEQ(emGeneral* emulator) {
	emulator->apu->spc->bus->readU8const(spcBUS_ptr, 0x00);
	if (emulator->apu->spc->PSW & 0x2) {
		emulator->apu->spc->PC += (s8) emulator->apu->spc->bus->value + 1;
	} else {
		++emulator->apu->spc->PC;
	}
};

static void snSPC_BNE(emGeneral* emulator) {
	emulator->apu->spc->bus->readU8const(spcBUS_ptr, 0x00);
	/* i kinda prefer this syntax, howewer it's not that readable */
	emulator->apu->spc->PC += (!(emulator->apu->spc->PSW & 0x2)) \
				  ? (s8) emulator->apu->spc->bus->value + 1 : 1;

	/*if (!(emulator->apu->spc->PSW & 0x2)) {
		emulator->apu->spc->PC += (s8) emulator->apu->spc->bus->value + 1;
		printf("spc_opcode: jumping to %X \n", *emulator->apu->spc->PC);
	} else {
		++emulator->apu->spc->PC;
		printf("spc_opcode: not jumping \n");
	}*/
};

static void snSPC_CLRP(emGeneral* emulator) {
	emulator->apu->spc->PSW ^= 0x1;
	if (emulator->apu->spc->PSW & 0x1) {
		emulator->apu->spc->DP = 0x100;
	};
	emulator->apu->spc->PC++;
};

static void snSPC_CLRC(emGeneral* emulator) {
	emulator->apu->spc->PSW &= ~(emulator->apu->spc->PSW);
};


static void snSPC_SETP(emGeneral* emulator) {
	emulator->apu->spc->PSW |= 0x1; 
	if (!(emulator->apu->spc->PSW & SPC_P_FLAG)) {
		emulator->apu->spc->DP = 0x0;
	};
	emulator->apu->spc->PC++;
};


static void snSPC_DEC_A(emGeneral* emulator) {
	emulator->apu->spc->A--;
	emulator->apu->spc->PC++;
};

static void snSPC_DEC_X(emGeneral* emulator) {
	emulator->apu->spc->X--;
	updatePSW(&emulator->apu->spc->PSW, emulator->apu->spc->X, SPC_N_FLAG | SPC_Z_FLAG);
	emulator->apu->spc->PC++;
};

static void snSPC_DEC_Y(emGeneral* emulator) {
	emulator->apu->spc->Y--;
	updatePSW(&emulator->apu->spc->PSW, emulator->apu->spc->Y, SPC_N_FLAG | SPC_Z_FLAG);
	emulator->apu->spc->PC++;
};

static void snSPC_INC_A(emGeneral* emulator) {
	emulator->apu->spc->A++;
	emulator->apu->spc->PC++;
};

static void snSPC_INC_X(emGeneral* emulator) {
	emulator->apu->spc->X++;
	emulator->apu->spc->PC++;
};

static void snSPC_INC_Y(emGeneral* emulator) {
	emulator->apu->spc->Y++;
	emulator->apu->spc->PC++;
};

static void snSPC_MOV_A_const(emGeneral* emulator) {
	emulator->apu->spc->A = *++emulator->apu->spc->PC;
	updatePSW(&emulator->apu->spc->PSW, emulator->apu->spc->A, SPC_N_FLAG | SPC_Z_FLAG);
	++emulator->apu->spc->PC;
};

static void snSPC_MOV_A_x(emGeneral* emulator) {};

static void snSPC_MOV_A_xplus(emGeneral* emulator) {};

static void snSPC_MOV_A_dp(emGeneral* emulator) {
	emulator->apu->spc->bus->readU16absoluteIndirect(emulator->apu->spc->bus, 0x02);
	emulator->apu->spc->A = emulator->apu->spc->bus->value;
};

static void snSPC_MOV_A_DP_X(emGeneral* emulator) {};

static void snSPC_MOV_A_abs(emGeneral* emulator) {
	emulator->apu->spc->bus->readU16absolute(emulator->apu->spc->bus, 0x00);
	emulator->apu->spc->PC++;
};

static void snSPC_MOV_A_abs_X(emGeneral* emulator) {
	emulator->apu->spc->bus->readU16absolute(emulator->apu->spc->bus, emulator->apu->spc->X);
	emulator->apu->spc->PC++;
};

static void snSPC_MOV_A_abs_Y(emGeneral* emulator) {
	emulator->apu->spc->bus->readU16absolute(emulator->apu->spc->bus, emulator->apu->spc->Y);
};

static void snSPC_MOV_Xindr_A(emGeneral* emulator) {
	emulator->apu->spc->bus->writeIndirect(emulator->apu->spc->bus, 0x00, emulator->apu->spc->A, 0x03);
	emulator->apu->spc->PC++;
};

static void snSPC_MOV_A_dp_x(emGeneral* emulator) {};
static void snSPC_MOV_A_dp_Y(emGeneral* emulator) {};
static void snSPC_MOV_X_const(emGeneral* emulator) {
	emulator->apu->spc->X = *++emulator->apu->spc->PC;
	updatePSW(&emulator->apu->spc->PSW, emulator->apu->spc->X, SPC_N_FLAG | SPC_Z_FLAG);
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
static void snSPC_MOV_dp_dp(emGeneral* emulator) {
	/* what does this shit do ???? */
	//emulator->apu->spc->DP;
	++emulator->apu->spc->PC;
};

static void snSPC_MOV_DP_const(emGeneral* emulator) {
	emulator->apu->spc->bus->readU8const(emulator->apu->spc->bus, 0x00);
	emulator->apu->spc->bus->writeIndirect(emulator->apu->spc->bus, 0x00, emulator->apu->spc->bus->value, 0x05);
	emulator->apu->spc->PC++;
	sleep(1);
};

static void snSPC_POP_Y(emGeneral* emulator) {
	printf("spc_opcode: pop_y \n");
};

static void snSPC_SETC(emGeneral* emulator) {
	emulator->apu->spc->PSW |= 0x1;
};

static void setupIPL(emGeneral* emulator, u8 type) {
	/* NTSC SNES */
	if (type == 0x00) {
		memcpy(&emulator->apu->internalRAM[0xFFC0], IPL_NTSC, 0x3F);
	}
	return;
};
static void fetchSPC(emGeneral* emulator) {
	/* TODO: implement this on CPU */

	printf("spc_fetch: %X %X %X %X %X %08b %X\n", *emulator->apu->spc->PC, emulator->apu->spc->X, emulator->apu->spc->Y, emulator->apu->spc->A, emulator->apu->spc->SP, emulator->apu->spc->PSW, emulator->apu->spc->PSW);
	printf("spc_registers: %X %X %X %X\n", emulator->apu->internalRAM[0xF4], \
		emulator->apu->internalRAM[0xF5], emulator->apu->internalRAM[0xF6], \
		emulator->apu->internalRAM[0xF7]);
	(emulator->apu->spc->opcode[(*emulator->apu->spc->PC)])(emulator);
	return;
}

extern void setupSPC(emGeneral* emulator, snSPC* spc_ptr, u8** buffer) {
	spcBUS_ptr = malloc(sizeof(spcBUS));

	/* APU != SPC
	 * SPC-700 is a completely separated Processor
	 * APU is the communicate interface
	 * DSL produces AUDIO */
	setupIPL(emulator, 0x00);
	setupSPCBUS(emulator, spcBUS_ptr);

	emulator->apu->spc = spc_ptr;
	emulator->apu->spc->bus = spcBUS_ptr;
	emulator->apu->spc->fetch = fetchSPC;

	emulator->apu->spc->PC = &emulator->apu->internalRAM[0xFFC0];
	emulator->apu->spc->SP = 0x00;
	emulator->apu->spc->A = 0x00;
	emulator->apu->spc->X = 0x00;
	emulator->apu->spc->Y = 0x00;
	emulator->apu->spc->DP = 0x0000;
	emulator->apu->spc->PSW = 0x0;

	buffer[0x00] = &emulator->apu->internalRAM[0xF4];
	buffer[0x01] = &emulator->apu->internalRAM[0xF5];
	buffer[0x02] = &emulator->apu->internalRAM[0xF6];
	buffer[0x03] = &emulator->apu->internalRAM[0xF7];

	for(unsigned int i = 0; i < 0xFF; i ++) {
		emulator->apu->spc->opcode[i] = generic;
	};

	emulator->apu->spc->opcode[0x2F] = snSPC_BRA;
	emulator->apu->spc->opcode[0xF0] = snSPC_BEQ;
	emulator->apu->spc->opcode[0xD0] = snSPC_BNE;
	emulator->apu->spc->opcode[0x20] = snSPC_CLRP;
	emulator->apu->spc->opcode[0x60] = snSPC_CLRC;
	emulator->apu->spc->opcode[0xBC] = snSPC_INC_A;
	emulator->apu->spc->opcode[0x3D] = snSPC_INC_X;
	emulator->apu->spc->opcode[0xFC] = snSPC_INC_Y;
	emulator->apu->spc->opcode[0x1D] = snSPC_DEC_X;
	emulator->apu->spc->opcode[0xDC] = snSPC_DEC_Y;
	emulator->apu->spc->opcode[0xF4] = snSPC_MOV_A_dp;
	emulator->apu->spc->opcode[0xCD] = snSPC_MOV_X_const;
	emulator->apu->spc->opcode[0xC6] = snSPC_MOV_Xindr_A;
	emulator->apu->spc->opcode[0xE9] = snSPC_MOV_X_abs;
	emulator->apu->spc->opcode[0xE8] = snSPC_MOV_A_const;
	emulator->apu->spc->opcode[0x8D] = snSPC_MOV_Y_const;
	emulator->apu->spc->opcode[0xEC] = snSPC_MOV_Y_abs;
	emulator->apu->spc->opcode[0x8F] = snSPC_MOV_DP_const;
	emulator->apu->spc->opcode[0xBD] = snSPC_MOV_SP_X;
	emulator->apu->spc->opcode[0xEE] = snSPC_POP_Y;
	emulator->apu->spc->opcode[0x40] = snSPC_SETP;
	emulator->apu->spc->opcode[0x80] = snSPC_SETC;

	return;
};
