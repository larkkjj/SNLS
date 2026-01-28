#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "core/cpu.h"
#include "emulator/main.h"
#include "emulator/memory.h"
#include "emulator/rom.h"
#include "core/bus.h"

static void sn_OpBMI(emGeneral* emulator) {
	if (emulator->cpu->sn_NFlag) {
		emulator->bus->readU8const(emulator->bus, 0);
		emulator->cpu->PC += (s8) emulator->bus->value + 1;
		printf("cpu_opcode: branching to %X \n", *(emulator->cpu->PC));
	} else {
		emulator->cpu->PC += 2;
	}
	emulator->cpu->cycles += 2;
	return;
};

static void sn_OpBNE(emGeneral* emulator) {
	if (!emulator->cpu->sn_ZFlag) {
		emulator->bus->readU8const(emulator->bus, 0);
		emulator->cpu->PC += (s8) emulator->bus->value + 1;
		printf("cpu_opcode: branching to %X \n", *(emulator->cpu->PC));
	} else {
		emulator->cpu->PC += 2;
	}
	emulator->cpu->cycles += 2;
	return;
};

static void sn_OpBPL(emGeneral* emulator) {
	if (!emulator->cpu->sn_NFlag) {
		emulator->bus->readU8const(emulator->bus, 0);
		emulator->cpu->PC += (s8) emulator->bus->value + 1;
		printf("cpu_opcode: %X -> branching to %X \n", ((s8) emulator->bus->value), *emulator->cpu->PC);
	} else {
		emulator->cpu->PC += 2;
	}
	emulator->cpu->cycles += 2;
};

static void sn_OpBRA(emGeneral* emulator) {
	//emulator->cpu->PC += (s8) sn_Mread_u8_const(emulator, 0) + 1;
	emulator->bus->readU8const(emulator->bus, 0x00);
	emulator->cpu->PC += (s8) emulator->bus->value + 1;
	printf("cpu_opcode: %X -> branching to %X\n", (s8) emulator->bus->value, *emulator->cpu->PC);
	emulator->cpu->cycles += 2;
};

static void sn_OpCLC(emGeneral* emulator) {
	emulator->cpu->flags ^= 0x01;
	emulator->cpu->PC++;
	emulator->cpu->cycles += 2;
	return;
};

static void sn_OpCLD(emGeneral* emulator) {
	emulator->cpu->flags ^= 0x08;
	emulator->cpu->PC++;
	emulator->cpu->cycles += 2;
	return;
};

static void sn_OpCLI(emGeneral* emulator) {
	emulator->cpu->flags ^= 0x04;
	emulator->cpu->PC++;
	emulator->cpu->cycles += 2;
	return;
};

static void sn_OpCLV(emGeneral* emulator) {
	emulator->cpu->flags ^= 0x40;
	emulator->cpu->PC++;
	emulator->cpu->cycles += 2;
	return;
};

static void sn_OpCMP_const(emGeneral* emulator) {
	if (!emulator->cpu->flags & 0x20 == 0) {
		emulator->bus->readU16const(emulator->bus);
	} else {
		emulator->bus->readU8const(emulator->bus, 0x00);
	}
	emulator->cpu->sn_NFlag = (emulator->cpu->A - emulator->bus->value < 0 ? 1 : 0);
	emulator->cpu->sn_ZFlag = (emulator->cpu->A - emulator->bus->value == 0 ? 1 : 0);
};

static void sn_OpCMP_addr(emGeneral* emulator) {
	if (!emulator->cpu->sn_MFlag) {
		emulator->bus->readU16absoluteIndirect(emulator->bus, 0x00, 0);
		printf("cmp: 16-bit mode\n");
	} else {
		emulator->bus->readU16absolute(emulator->bus, 0x00);
		printf("cmp: 8-bit mode\n");
	};

	(emulator->cpu->sn_NFlag = (emulator->cpu->A - emulator->bus->value) < 0) ? 1 : 0;
	(emulator->cpu->sn_ZFlag = (emulator->cpu->A - emulator->bus->value) == 0) || (emulator->cpu->A == emulator->bus->value) ? 1 : 0;
	printf("cmp_address: address [%X:%X] comparing %X with accumulator: %X \n", emulator->bus->bank, emulator->bus->address, emulator->bus->value, emulator->cpu->A);
	++emulator->cpu->PC;
	emulator->cpu->cycles += 4;
	usleep(10000);
};

static void sn_OpDEA(emGeneral* emulator) {
	emulator->cpu->A--;
	emulator->cpu->PC++;
	emulator->cpu->cycles += 2;
	emulator->cpu->sn_NFlag = ((s16) emulator->cpu->A < 0 ? 1 : 0);
	emulator->cpu->sn_ZFlag = (emulator->cpu->A == 0 ? 1 : 0);
	return;
};

static void sn_OpDEX(emGeneral* emulator) {
	emulator->cpu->X--;
	emulator->cpu->PC++;
	emulator->cpu->cycles += 2;
	emulator->cpu->sn_NFlag = ((s16) emulator->cpu->X < 0 ? 1 : 0);
	emulator->cpu->sn_ZFlag = (emulator->cpu->X == 0 ? 1 : 0);
	return;
};

static void sn_OpDEY(emGeneral* emulator) {
	emulator->cpu->Y--;
	emulator->cpu->PC++;
	emulator->cpu->cycles += 2;
	emulator->cpu->sn_NFlag = ((s16) emulator->cpu->Y < 0 ? 1 : 0);
	emulator->cpu->sn_ZFlag = (emulator->cpu->Y == 0 ? 1 : 0);
	return;
};

static void sn_OpJSR_addr(emGeneral* emulator) {
	//sn_Mread_u16_absolute(emulator, 0);
	emulator->bus->readU16absolute(emulator->bus, 0);
	emulator->cpu->subPC = emulator->cpu->PC;
	emulator->cpu->PC = emulator->bus->pointer;
	printf("jump_subroutine: %X %X -> %X\n", emulator->bus->bank, emulator->bus->address, *emulator->cpu->PC);
	emulator->cpu->cycles += 6;
	return;
};

static void sn_OpINA(emGeneral* emulator) {
	emulator->cpu->A++;
	emulator->cpu->sn_NFlag = ((s16) emulator->cpu->A < 0 ? 1 : 0);
	emulator->cpu->sn_ZFlag = (emulator->cpu->A == 0 ? 1 : 0);
	emulator->cpu->PC++;
	emulator->cpu->cycles += 2;
	return;
};

static void sn_OpINC_dp(emGeneral* emulator) {
	emulator->cpu->DP++;
	emulator->cpu->sn_NFlag = ((s16) emulator->cpu->DP < 0 ? 1 : 0);
	emulator->cpu->sn_ZFlag = (emulator->cpu->DP == 0 ? 1 : 0);
	emulator->cpu->cycles += 5;
	return;
};

static void sn_OpINC_addr(emGeneral* emulator) {
	//sn_Mread_u16_absolute(emulator, 0);
	//sn_Mwrite(emulator, sn_Mread_u16_absolute(emulator, 0), 0, +1);
	emulator->bus->readU16absolute(emulator->bus, 0);
	emulator->bus->write(emulator->bus, +1);
	emulator->cpu->sn_NFlag = ((s16) emulator->bus->value < 0 ? 1 : 0);
	emulator->cpu->sn_ZFlag = (emulator->bus->value == 0 ? 1 : 0);
	emulator->cpu->PC++;
	emulator->cpu->cycles += 6;
	return;
};

static void sn_OpINC_dpX(emGeneral* emulator) {
	//holdAddr = *//sn_Mwrite(emulator, sn_Mread_u8_const(emulator, emulator->cpu->DP), emulator->cpu->X, +1);
	emulator->bus->readU8const(emulator->bus, emulator->cpu->DP + emulator->cpu->X);
	emulator->cpu->cycles += 6;
	emulator->cpu->sn_NFlag = ((s16) emulator->bus->value < 0 ? 1 : 0);
	emulator->cpu->sn_ZFlag = (emulator->bus->value == 0 ? 1 : 0);
	emulator->cpu->PC++;
	return;
};

static void sn_OpINC_addrX(emGeneral* emulator) {
	//sn_Mread_u16_absolute(emulator, 0);
	emulator->bus->readU16absolute(emulator->bus, emulator->cpu->X);
	emulator->bus->write(emulator->bus, +1);
	emulator->cpu->sn_NFlag = ((s16) emulator->bus->value < 0 ? 1 : 0);
	emulator->cpu->sn_ZFlag = (emulator->bus->value == 0 ? 1 : 0);
	emulator->cpu->cycles += 7;
	emulator->cpu->PC++;
	return;
};

static void sn_OpINX(emGeneral* emulator) {
	emulator->cpu->X += 1;
	emulator->cpu->sn_NFlag = ((s16) emulator->cpu->X < 0 ? 1 : 0);
	emulator->cpu->sn_ZFlag = (emulator->cpu->X == 0 ? 1 : 0);
	emulator->cpu->cycles += 2;
	emulator->cpu->PC++;
	return;
};

static void sn_OpINY(emGeneral* emulator) {
	emulator->cpu->Y += 1;
	emulator->cpu->sn_NFlag = ((s16) emulator->cpu->Y < 0 ? 1 : 0);
	emulator->cpu->sn_ZFlag = (emulator->cpu->Y == 0 ? 1 : 0);
	emulator->cpu->cycles += 2;
	emulator->cpu->PC++;
	return;
};

static void sn_OpJMP_addr(emGeneral* emulator) {
	/* call emulator->memory->returnPtrBank*/
	//emulator->cpu->PC = sn_Mread_u16(emulator, 0) - 1;
	emulator->bus->readU16absolute(emulator->bus, 0);
	emulator->cpu->PC = emulator->bus->pointer;
	emulator->cpu->cycles += 3;
	emulator->cpu->PC++;
	return;
};

static void sn_OpJMP_long(emGeneral* emulator) {
	/* call emulator->memory->returnPtrBank*/
	//emulator->cpu->PC = sn_Mread_u24(emulator, 0) - 1;
	emulator->cpu->PC++;
	emulator->cpu->cycles += 4;
	return;
};

static void sn_OpLDA_addr(emGeneral* emulator) {
	//sn_Mread_u16_absolute(emulator, 0);
	emulator->bus->readU16absolute(emulator->bus, 0x00);
	emulator->cpu->A = emulator->bus->value;
	emulator->cpu->sn_NFlag = (s16) emulator->cpu->A < 0 ? 1 : 0;
	emulator->cpu->sn_ZFlag = emulator->cpu->A == 0 ? 1 : 0;
	emulator->cpu->PC++;
	emulator->cpu->cycles += 4;
	return;
};

static void sn_OpLDA_const(emGeneral* emulator) {
	if (!emulator->cpu->sn_MFlag) {
		emulator->bus->readU16const(emulator->bus);
	} else {
		emulator->bus->readU8const(emulator->bus, 0x00);
	}
	emulator->cpu->A = emulator->bus->value;
	emulator->cpu->sn_NFlag = (s16) emulator->cpu->A < 0 ? 1 : 0;
	emulator->cpu->sn_ZFlag = emulator->cpu->A == 0 ? 1 : 0;
	++emulator->cpu->PC;
	emulator->cpu->cycles += 2;
	return;
};

static void sn_OpLDA_DP_indirect_long_Y(emGeneral* emulator) {
	emulator->bus->readU24absolute(emulator->bus, 0);
	emulator->cpu->cycles += 6;
	return;
};


static void sn_OpLDX_const(emGeneral* emulator) {
	if (!emulator->cpu->sn_XFlag) {
		emulator->bus->readU16const(emulator->bus);
	} else {
		emulator->bus->readU8const(emulator->bus, 0x00);
	}
	emulator->cpu->X = emulator->bus->value;
	emulator->cpu->sn_NFlag = (s16) emulator->cpu->X < 0 ? 1 : 0;
	emulator->cpu->sn_ZFlag = emulator->cpu->X == 0 ? 1 : 0;
	emulator->cpu->PC++;
	emulator->cpu->cycles += 2;
	return;
};

static void sn_OpLDY_addr(emGeneral* emulator) {
	/*TODO: make new functions for replacing
	 * pointer dereference of u16 addresses */
	/* TODO: DID I ALREADY DO IT????? IDK BRO */
	//sn_Mread_u16_absolute(emulator, 0);
	emulator->bus->readU16absolute(emulator->bus, 0);
	emulator->cpu->Y = emulator->bus->value;
	emulator->cpu->sn_NFlag = (s16) emulator->cpu->Y < 0 ? 1 : 0;
	emulator->cpu->sn_ZFlag = emulator->cpu->Y == 0 ? 1 : 0;
	emulator->cpu->cycles += 4;
	return;
};
static void sn_OpLDY_const(emGeneral* emulator) {
	if (!emulator->cpu->sn_XFlag) {
		emulator->bus->readU16const(emulator->bus);
	} else {
		emulator->bus->readU8const(emulator->bus, 0x00);
	}
	emulator->cpu->Y = emulator->bus->value;
	emulator->cpu->sn_NFlag = (s16) emulator->cpu->Y < 0 ? 1 : 0;
	emulator->cpu->sn_ZFlag = emulator->cpu->Y == 0 ? 1 : 0;
	emulator->cpu->PC++;
	emulator->cpu->cycles += 2;
	return;
};

static void sn_OpSTA_addr(emGeneral* emulator) {
	//sn_Mread_u16_absolute(emulator, 0);
	emulator->bus->readU16absolute(emulator->bus, 0);
	emulator->bus->write(emulator->bus, emulator->cpu->A);
	emulator->cpu->PC++;
	emulator->cpu->cycles += 4;
	return;
};

static void sn_OpSTA_long(emGeneral* emulator) {
	emulator->bus->readU24absolute(emulator->bus, 0);
	emulator->bus->write(emulator->bus, emulator->cpu->A);
	emulator->cpu->PC++;
	emulator->cpu->cycles += 5;
	return;
};

static void sn_OpSTA_longX(emGeneral* emulator) {
	emulator->bus->readU24absolute(emulator->bus, emulator->cpu->X);
	emulator->bus->write(emulator->bus, emulator->cpu->A);
	emulator->cpu->PC++;
	emulator->cpu->cycles += 5;
	return;
};

static void sn_OpSTZ_dp(emGeneral* emulator) {
	////sn_Mwrite(emulator, emulator->cpu->DP, 0, sn_Mread_u8_const(emulator, 0), 0);
	emulator->cpu->PC++;
	emulator->cpu->cycles += 3;
	return;
};
static void sn_OpSTZ_dpX(emGeneral* emulator) {
	////sn_Mwrite(emulator, sn_Mread_u8_const(emulator, emulator->cpu->DP), emulator->cpu->X, 0);
	emulator->cpu->PC++;
	emulator->cpu->cycles += 4;
	return;
};
static void sn_OpSTZ_addr(emGeneral* emulator) {
	//sn_Mread_u16_absolute(emulator, 0);
	emulator->bus->readU16absolute(emulator->bus, 0);
	emulator->bus->write(emulator->bus, 0);
	emulator->cpu->PC++;
	emulator->cpu->cycles += 4;
	return;
};
static void sn_OpSTZ_addrX(emGeneral* emulator) {
	emulator->bus->readU16absolute(emulator->bus, emulator->cpu->X);
	emulator->bus->write(emulator->bus, 0);
	emulator->cpu->PC++;
	emulator->cpu->cycles += 5;
	return;
};

static void sn_OpSEC(emGeneral* emulator) {
	emulator->cpu->sn_CFlag = 1;
	emulator->cpu->PC++;
	emulator->cpu->cycles += 2;
	return;
};

static void sn_OpSED(emGeneral* emulator) {
	emulator->cpu->sn_DFlag = 1;
	emulator->cpu->cycles += 2;
	emulator->cpu->PC++;
	return;
};

static void sn_OpSEI(emGeneral* emulator) {
	emulator->cpu->sn_IFlag = 1;
	emulator->cpu->cycles += 2;
	emulator->cpu->PC++;
	return;
};

static void sn_OpREP(emGeneral* emulator) {
	emulator->bus->readU8const(emulator->bus, 0);
	emulator->cpu->sn_CFlag = (emulator->bus->value & 0x80 ? 0 : emulator->cpu->sn_CFlag);
	emulator->cpu->sn_VFlag = (emulator->bus->value & 0x40 ? 0 : emulator->cpu->sn_VFlag);
	emulator->cpu->sn_MFlag = (emulator->bus->value & 0x20 ? 0 : emulator->cpu->sn_MFlag);
	emulator->cpu->sn_XFlag = (emulator->bus->value & 0x10 ? 0 : emulator->cpu->sn_XFlag);
	emulator->cpu->sn_DFlag = (emulator->bus->value & 0x08 ? 0 : emulator->cpu->sn_DFlag);
	emulator->cpu->sn_IFlag = (emulator->bus->value & 0x04 ? 0 : emulator->cpu->sn_IFlag);
	emulator->cpu->sn_ZFlag = (emulator->bus->value & 0x02 ? 0 : emulator->cpu->sn_ZFlag);
	emulator->cpu->sn_EFlag = (emulator->bus->value & 0x01 ? 0 : emulator->cpu->sn_EFlag);
	emulator->cpu->cycles += 3;
	emulator->cpu->PC++;
	return;
};

static void sn_OpRTS(emGeneral* emulator) {
	sleep(1);
};

static void sn_OpSBC_const(emGeneral* emulator) {
	if (!emulator->cpu->sn_MFlag) {
		emulator->bus->readU16const(emulator->bus);
		emulator->cpu->A = emulator->cpu->A - emulator->bus->value + (!emulator->cpu->sn_CFlag ? 1 : 0);
	} else {
		emulator->bus->readU8const(emulator->bus, 0x00);
		emulator->cpu->A = emulator->cpu->A - emulator->bus->value + (!emulator->cpu->sn_CFlag ? 1 : 0);
	}
	emulator->cpu->sn_NFlag = ((s16) emulator->cpu->A < 0 ? 1 : 0);
	emulator->cpu->sn_ZFlag = (emulator->cpu->A == 0 ? 1 : 0);
	emulator->cpu->sn_VFlag = ((emulator->cpu->A) > 0xFFFF ? 1 : 0);
	emulator->cpu->sn_CFlag = ((s16) emulator->cpu->A == 0 ? 1 : 0);
	emulator->cpu->PC++;
};

static void sn_OpSEP(emGeneral* emulator) {
	emulator->bus->readU8const(emulator->bus, 0);
	emulator->cpu->sn_CFlag = (emulator->bus->value & 0x80 ? 1 : emulator->cpu->sn_CFlag);
	emulator->cpu->sn_VFlag = (emulator->bus->value & 0x40 ? 1 : emulator->cpu->sn_VFlag);
	emulator->cpu->sn_MFlag = (emulator->bus->value & 0x20 ? 1 : emulator->cpu->sn_MFlag);
	emulator->cpu->sn_XFlag = (emulator->bus->value & 0x10 ? 1 : emulator->cpu->sn_XFlag);
	emulator->cpu->sn_DFlag = (emulator->bus->value & 0x08 ? 1 : emulator->cpu->sn_DFlag);
	emulator->cpu->sn_IFlag = (emulator->bus->value & 0x04 ? 1 : emulator->cpu->sn_IFlag);
	emulator->cpu->sn_ZFlag = (emulator->bus->value & 0x02 ? 1 : emulator->cpu->sn_ZFlag);
	emulator->cpu->sn_EFlag = (emulator->bus->value & 0x01 ? 1 : emulator->cpu->sn_EFlag);
	emulator->cpu->cycles += 3;
	emulator->cpu->PC++;
	return;
};

static void sn_OpTAX(emGeneral* emulator) {
	emulator->cpu->X = emulator->cpu->A;
	emulator->cpu->sn_NFlag = ((s16) emulator->cpu->X < 0 ? 1 : 0);
	emulator->cpu->sn_ZFlag = (emulator->cpu->X == 0 ? 1 : 0);
	emulator->cpu->cycles += 2;
	emulator->cpu->PC++;
	return;
};

static void sn_OpTCS(emGeneral* emulator) {
	emulator->cpu->SP = emulator->cpu->A;
	emulator->cpu->sn_NFlag = ((s16) emulator->cpu->SP < 0 ? 1 : 0);
	emulator->cpu->sn_ZFlag = (emulator->cpu->SP == 0 ? 1 : 0);
	emulator->cpu->cycles += 2;
	emulator->cpu->PC++;
	return;
};

static void sn_OpTAY(emGeneral* emulator) {
	emulator->cpu->Y = emulator->cpu->A;
	emulator->cpu->sn_NFlag = ((s16) emulator->cpu->Y < 0 ? 1 : 0);
	emulator->cpu->sn_ZFlag = (emulator->cpu->Y == 0 ? 1 : 0);
	emulator->cpu->cycles += 2;
	emulator->cpu->PC++;
	return;
};

static void sn_OpTCD(emGeneral* emulator) {
	emulator->cpu->DP = emulator->cpu->A;
	emulator->cpu->sn_NFlag = ((s16) emulator->cpu->DP < 0 ? 1 : 0);
	emulator->cpu->sn_ZFlag = (emulator->cpu->DP == 0 ? 1 : 0);
	emulator->cpu->PC++;
	emulator->cpu->cycles += 2;
	return;
};

static void sn_OpTXY(emGeneral* emulator) {
	emulator->cpu->A = emulator->cpu->Y;
	emulator->cpu->sn_NFlag = ((s16) emulator->cpu->A < 0 ? 1 : 0);
	emulator->cpu->sn_ZFlag = (emulator->cpu->A == 0 ? 1 : 0);
	emulator->cpu->PC++;
	emulator->cpu->cycles += 2;
	return;
};

static void sn_OpTYA(emGeneral* emulator) {
	emulator->cpu->A = emulator->cpu->Y;
	emulator->cpu->sn_NFlag = ((s16) emulator->cpu->A < 0 ? 1 : 0);
	emulator->cpu->sn_ZFlag = (emulator->cpu->A == 0 ? 1 : 0);
	emulator->cpu->PC++;
	emulator->cpu->cycles += 2;
	return;
};

static void sn_OpTYX(emGeneral* emulator) {
	emulator->cpu->A = emulator->cpu->Y;
	emulator->cpu->sn_NFlag = ((s16) emulator->cpu->A < 0 ? 1 : 0);
	emulator->cpu->sn_ZFlag = (emulator->cpu->A == 0 ? 1 : 0);
	emulator->cpu->PC++;
	emulator->cpu->cycles += 2;
	return;
};

static void sn_OpXCE(emGeneral* emulator) {
	u16 temp_C = emulator->cpu->sn_CFlag;
	u16 temp_E = emulator->cpu->sn_EFlag;

	emulator->cpu->sn_EFlag = temp_C;
	emulator->cpu->sn_CFlag = temp_E;
	emulator->cpu->PC++;
	emulator->cpu->cycles += 2;
	return;
}

static void fetchCPU(emGeneral* emulator) {
	/* emulator->cpu->PC = emulator->memory->bank_array[emulator->cpu->PB][emulator->cpu->PC] */
	/* or/and, make a variable called emulator->cpu->BankChanged, that opcodes like JMP/JSR can 
	 * enable, making the bank change. */

	/* remember to use cpu_Opcodes[*++emulator->cpu->PC](emulator)
	 * fast and btw, we will need that 'cause of PBR register 
				
			that's a strange coincidence */

	printf("cpu_fetch: A: %X X: %X Y: %X SP: %X E: %X M: %X I: %X\n", emulator->cpu->A, emulator->cpu->X, emulator->cpu->Y, emulator->cpu->SP, emulator->cpu->sn_EFlag, emulator->cpu->sn_MFlag, emulator->cpu->sn_IFlag);
	printf("cpu_opcode: %X \n", *emulator->cpu->PC);
	printf("cpu_cycles: %d\n", emulator->cpu->cycles);
	switch (*(emulator->cpu->PC)) {
		case _bmi:
			sn_OpBMI(emulator);
			break;
		case _bne:
			sn_OpBNE(emulator);
			break;
		case _bpl:
			sn_OpBPL(emulator);
			break;
		case _bra:
			sn_OpBRA(emulator);
			break;
		case _clc:
			sn_OpCLC(emulator);
			break;
		case _cld:
			sn_OpCLD(emulator);
			break;
		case _cli:
			sn_OpCLI(emulator);
			break;
		case _clv:
			sn_OpCLV(emulator);
			break;
		case _cmp_addr:
			sn_OpCMP_addr(emulator);
			break;
		case _dea:
			sn_OpDEA(emulator);
			break;
		case _dex:
			sn_OpDEX(emulator);
			break;
		case _dey:
			sn_OpDEY(emulator);
			break;
		case _inc_a:
			sn_OpINA(emulator);
			break;
		case _inc_dp:
			sn_OpINC_dp(emulator);
			break;
		case _inc_addr:
			sn_OpINC_addr(emulator);
			break;
		case _inc_dp_x:
			sn_OpINC_dpX(emulator);
			break;
		case _inc_addr_x:
			sn_OpINC_addrX(emulator);
			break;
		case _jmp_addr:
			sn_OpJMP_addr(emulator);
			break;
		case _jsr_addr:
			sn_OpJSR_addr(emulator);
			break;
		case _lda_addr:
			sn_OpLDA_addr(emulator);
			break;
		case _lda_const:
			sn_OpLDA_const(emulator);
			break;
		case _ldx_const:
			sn_OpLDX_const(emulator);
			break;
		case _ldy_addr:
			sn_OpLDY_addr(emulator);
			break;
		case _ldy_const:
			sn_OpLDY_const(emulator);
			break;
		case _sec:
			sn_OpSEC(emulator);
			break;
		case _sed:
			sn_OpSED(emulator);
			break;
		case _sbc_const:
			sn_OpSBC_const(emulator);
			break;
		case _sei:
			sn_OpSEI(emulator);
			break;
		case _rep:
			sn_OpREP(emulator);
			break;
		case _sep:
			sn_OpSEP(emulator);
			break;
		case _sta_addr:
			sn_OpSTA_addr(emulator);
			break;
		case _sta_l:
			sn_OpSTA_long(emulator);
			break;
		case _sta_l_x:
			sn_OpSTA_longX(emulator);
			break;
		case _stz_dp:
			sn_OpSTZ_dp(emulator);
			break;
		case _stz_dp_x:
			sn_OpSTZ_dpX(emulator);
			break;
		case _stz_addr:
			sn_OpSTZ_addr(emulator);
			break;
		case _stz_addr_x:
			sn_OpSTZ_addrX(emulator);
			break;
		case _tax:
			sn_OpTAX(emulator);
			break;
		case _tay:
			sn_OpTAY(emulator);
			break;
		case _tcd:
			sn_OpTCD(emulator);
			break;
		case _tcs:
			sn_OpTCS(emulator);
			break;
		case _txy:
			sn_OpTXY(emulator);
			break;
		case _tya:
			sn_OpTYA(emulator);
			break;
		case _tyx:
			sn_OpTYX(emulator);
			break;
		case _xce:
			sn_OpXCE(emulator);
			break;
		case 0x00:
			printf("cpu_fetch: cpu crossed at zero, something wrong? \nlast byte: %02X current byte: %02X next byte: %02X at address: %X\n", *(emulator->cpu->PC - 1), *emulator->cpu->PC, *(emulator->cpu->PC + 1), emulator->memory->address_target);
			emulator->cpu->PC++;
			//sleep(2);
			break;
		default:
			printf("cpu_fetch: unknown opcode %X \n",*(emulator->cpu->PC));
			++emulator->cpu->PC;
			break;
	};
	return;
}

extern void setupCPU(emGeneral *emulator, rom* rom_Ptr) {
	printf("cpu_setup: init\n");
	/* Emulation Mode */
	emulator->cpu->sn_EFlag = 1;
	emulator->cpu->sn_MFlag = 1;
	emulator->cpu->sn_IFlag = 0;
	emulator->cpu->sn_XFlag = 1;
	emulator->cpu->SP = 0x100;

	/* this sets registers to zero
	 * this is only made for debugging
	 * the real SNES can point these
	 * values to every thing */
	emulator->cpu->P = 0;
	emulator->cpu->PB = 0;
	emulator->cpu->DBR = 0;
	emulator->cpu->cycles = 0;
	emulator->cpu->fetch = fetchCPU;

	printf("cpu_setup: done\n");
	emulator->cpu->PC = &emulator->memory->bank_array[emulator->cpu->PB][rom_Ptr->resetV - 0x8000];
	printf("cpu_setup: starting at %X %X \n", emulator->memory->bank_array[emulator->cpu->PB][0], emulator->cpu->PC);
}
