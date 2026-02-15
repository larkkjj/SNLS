#include <stdio.h>
#include <unistd.h>
#include <malloc.h>
#include <string.h>

#include "general/references.h"
#include "general/functions.h"
#include "emulator/memory.h"
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


static void generic(snSPC* spc) {
	printf("spc_opcode: last_status: %X %X %X %X %X %X \n", \
			spc->PC, \
			spc->A, \
			spc->DP, \
			spc->X, \
			spc->Y, \
			spc->YA);

	printf("spc_registers: %X %X %X %X\n", spc->internalRAM[0xF4], spc->internalRAM[0xF5],\
	      	 spc->internalRAM[0xF6], spc->internalRAM[0xF7]);

	printf("spc_opcode: %X generic opcode / not implemented\n", *spc->PC);
	return;
};

static void snSPC_BRA(snSPC* spc) {
	spc->bus->readU8const(spc, 0x00);
	spc->PC += (s8) spc->bus->address + 1;
};

static void snSPC_BEQ(snSPC* spc) {
	spc->bus->readU8const(spc, 0x00);
	if (spc->PSW & 0x2) {
		spc->PC += (s8) spc->bus->address + 1;
	} else {
		++spc->PC;
	}
};

static void snSPC_BNE(snSPC* spc) {
	spc->bus->readU8const(spc, 0x00);
	/* i kinda prefer this syntax, howewer it's not that readable */
	spc->PC += (!(spc->PSW & 0x2)) \
				  ? (s8) spc->bus->value + 1 : 1;

	/*if (!(spc->PSW & 0x2)) {
		spc->PC += (s8) spc->bus->value + 1;
		printf("spc_opcode: jumping to %X \n", *spc->PC);
	} else {
		++spc->PC;
		printf("spc_opcode: not jumping \n");
	}*/
};

static void snSPC_CLRP(snSPC* spc) {
	spc->PSW ^= 0x1;
	if (spc->PSW & 0x1) {
		spc->DP = 0x100;
	};
	spc->PC++;
};

static void snSPC_CLRC(snSPC* spc) {
	spc->PSW &= ~(spc->PSW);
};


static void snSPC_SETP(snSPC* spc) {
	spc->PSW |= 0x1;
	if (!(spc->PSW & SPC_P_FLAG)) {
		spc->DP = 0x0;
	};
	spc->PC++;
};


static void snSPC_DEC_A(snSPC* spc) {
	spc->A--;
	spc->PC++;
};

static void snSPC_DEC_X(snSPC* spc) {
	spc->X--;
	updatePSW(&spc->PSW, spc->X, SPC_N_FLAG | SPC_Z_FLAG);
	spc->PC++;
};

static void snSPC_DEC_Y(snSPC* spc) {
	spc->Y--;
	updatePSW(&spc->PSW, spc->Y, SPC_N_FLAG | SPC_Z_FLAG);
	spc->PC++;
};

static void snSPC_INC_A(snSPC* spc) {
	spc->A++;
	spc->PC++;
};

static void snSPC_INC_X(snSPC* spc) {
	spc->X++;
	spc->PC++;
};

static void snSPC_INC_Y(snSPC* spc) {
	spc->Y++;
	spc->PC++;
};

static void snSPC_MOV_A_const(snSPC* spc) {
	spc->A = *++spc->PC;
	updatePSW(&spc->PSW, spc->A, SPC_N_FLAG | SPC_Z_FLAG);
	++spc->PC;
};

static void snSPC_MOV_A_x(snSPC* spc) {};

static void snSPC_MOV_A_xplus(snSPC* spc) {};

static void snSPC_MOV_A_dp(snSPC* spc) {
	spc->bus->readU16absoluteIndirect(spc, 0x02);
	spc->A = spc->bus->value;
};

static void snSPC_MOV_A_DP_X(snSPC* spc) {};

static void snSPC_MOV_A_abs(snSPC* spc) {
	spc->bus->readU16absolute(spc, 0x00);
	spc->PC++;
};

static void snSPC_MOV_A_abs_X(snSPC* spc) {
	spc->bus->readU16absolute(spc, spc->X);
	spc->PC++;
};

static void snSPC_MOV_A_abs_Y(snSPC* spc) {
	spc->bus->readU16absolute(spc, spc->Y);
};

static void snSPC_MOV_Xindr_A(snSPC* spc) {
	spc->bus->writeIndirect(spc, 0x00, spc->A, 0x03);
	spc->PC++;
};

static void snSPC_MOV_A_dp_x(snSPC* spc) {};
static void snSPC_MOV_A_dp_Y(snSPC* spc) {};
static void snSPC_MOV_X_const(snSPC* spc) {
	spc->X = *++spc->PC;
	updatePSW(&spc->PSW, spc->X, SPC_N_FLAG | SPC_Z_FLAG);
	++spc->PC;
}

static void snSPC_MOV_X_dp(snSPC* spc) {};
static void snSPC_MOV_DP_Y(snSPC* spc) {};
static void snSPC_MOV_X_abs(snSPC* spc) {};
static void snSPC_MOV_Y_const(snSPC* spc) {
	spc->Y = *++spc->PC;
	++spc->PC;
};

static void snSPC_MOV_Y_DP(snSPC* spc) {};

static void snSPC_MOV_Y_DP_X(snSPC* spc) {};

static void snSPC_MOV_Y_abs(snSPC* spc) {};

/* copy register to register */
static void snSPC_MOV_A_X(snSPC* spc) {
	spc->A = spc->X;
	++spc->PC;
};

static void snSPC_MOV_A_Y(snSPC* spc) {
	spc->A = spc->Y;
	++spc->PC;
};

static void snSPC_MOV_X_A(snSPC* spc) {
	spc->X = spc->A;
	++spc->PC;
};

static void snSPC_MOV_Y_A(snSPC* spc) {
	spc->Y = spc->A;
	++spc->PC;
};
static void snSPC_MOV_X_SP(snSPC* spc) {
	spc->X = spc->SP;
	++spc->PC;
};
static void snSPC_MOV_SP_X(snSPC* spc) {
	spc->SP = spc->X;
	++spc->PC;
}
static void snSPC_MOV_dp_dp(snSPC* spc) {
	/* what does this shit do ???? */
	//spc->DP;
	++spc->PC;
};

static void snSPC_MOV_DP_const(snSPC* spc) {
	spc->bus->readU8const(spc, 0x00);
	spc->bus->writeIndirect(spc, 0x00, hAddr, 0x05);
	spc->PC++;
	sleep(1);
};

static void snSPC_POP_Y(snSPC* spc) {
	printf("spc_opcode: pop_y \n");
};

static void snSPC_SETC(snSPC* spc) {
	spc->PSW |= 0x1;
};

static void setupIPL(snSPC* spc, u8 type) {
	/* NTSC SNES */
	if (type == 0x00) {
		memcpy(&spc->internalRAM[0xFFC0], IPL_NTSC, 0x3F);
	}
	return;
};

static void fetchSPC(snSPC* spc) {
	/* TODO: implement this on CPU */

	printf("spc_fetch: %X %X %X %X %X %08b %X\n", *spc->PC, spc->X, spc->Y, spc->A, spc->SP, spc->PSW, spc->PSW);
	printf("spc_registers: %X %X %X %X\n", spc->internalRAM[0xF4], \
		spc->internalRAM[0xF5], spc->internalRAM[0xF6], \
		spc->internalRAM[0xF7]);
	(spc->opcode[(*spc->PC)])(spc);
	return;
}

extern void setupSPC(emGeneral* emulator, snSPC* spc_ptr, u8** buffer) {
	printf("spc_setup: init\n");
	spcBUS_ptr = malloc(sizeof(spcBUS));

	/* APU != SPC
	 * SPC-700 is a completely separated Processor
	 * APU is the communicate interface
	 * DSL produces AUDIO */
	setupIPL(spc_ptr, 0x00);

	emulator->apu->spc = spc_ptr;
	spc_ptr->bus = spcBUS_ptr;
	spc_ptr->fetch = fetchSPC;

	spc_ptr->PC = &spc_ptr->internalRAM[0xFFC0];
	spc_ptr->SP = 0x00;
	spc_ptr->A = 0x00;
	spc_ptr->X = 0x00;
	spc_ptr->Y = 0x00;
	spc_ptr->DP = 0x0000;
	spc_ptr->PSW = 0x0;

	setupSPCBUS(spc_ptr, spcBUS_ptr);

	buffer[0x00] = &spc_ptr->internalRAM[0xF4];
	buffer[0x01] = &spc_ptr->internalRAM[0xF5];
	buffer[0x02] = &spc_ptr->internalRAM[0xF6];
	buffer[0x03] = &spc_ptr->internalRAM[0xF7];

	for(unsigned int i = 0; i < 0xFF; i ++) {
		spc_ptr->opcode[i] = generic;
	};

	spc_ptr->opcode[0x2F] = snSPC_BRA;
	spc_ptr->opcode[0xF0] = snSPC_BEQ;
	spc_ptr->opcode[0xD0] = snSPC_BNE;
	spc_ptr->opcode[0x20] = snSPC_CLRP;
	spc_ptr->opcode[0x60] = snSPC_CLRC;
	spc_ptr->opcode[0xBC] = snSPC_INC_A;
	spc_ptr->opcode[0x3D] = snSPC_INC_X;
	spc_ptr->opcode[0xFC] = snSPC_INC_Y;
	spc_ptr->opcode[0x1D] = snSPC_DEC_X;
	spc_ptr->opcode[0xDC] = snSPC_DEC_Y;
	spc_ptr->opcode[0xF4] = snSPC_MOV_A_dp;
	spc_ptr->opcode[0xCD] = snSPC_MOV_X_const;
	spc_ptr->opcode[0xC6] = snSPC_MOV_Xindr_A;
	spc_ptr->opcode[0xE9] = snSPC_MOV_X_abs;
	spc_ptr->opcode[0xE8] = snSPC_MOV_A_const;
	spc_ptr->opcode[0x8D] = snSPC_MOV_Y_const;
	spc_ptr->opcode[0xEC] = snSPC_MOV_Y_abs;
	spc_ptr->opcode[0x8F] = snSPC_MOV_DP_const;
	spc_ptr->opcode[0xBD] = snSPC_MOV_SP_X;
	spc_ptr->opcode[0xEE] = snSPC_POP_Y;
	spc_ptr->opcode[0x40] = snSPC_SETP;
	spc_ptr->opcode[0x80] = snSPC_SETC;

	printf("spc_setup: done\n");
	usleep(1000);
	return;
};
