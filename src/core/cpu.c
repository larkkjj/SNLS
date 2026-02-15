#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "core/cpu.h"
#include "emulator/main.h"
#include "emulator/memory.h"
#include "emulator/rom.h"
#include "core/bus.h"

static void sn_OpBIT_dp(snCPU* cpu);
static void sn_OpBIT_addr(snCPU* cpu);
static void sn_OpBIT_dpX(snCPU* cpu);
static void sn_OpBIT_addrX(snCPU* cpu);

static void sn_OpBIT_const(snCPU* cpu) {
	cpu->holder.value = (cpu->A & ++*cpu->PC);
	cpu->flags.Zero = (cpu->holder.value == 0 ? 1 : 0);
};

static void sn_OpBMI(snCPU* cpu) {
	if (cpu->flags.Negative) {
		cpu->bus->read(cpu, ADDRESS_RELATIVE_8, false, 0);
		cpu->PC =  cpu->holder.ptr;
		printf("cpu_minus: branching to %X \n", *(cpu->PC));
	} else {
		cpu->PC += 2;
	}
	return;
};

static void sn_OpBNE(snCPU* cpu) {
	if (!cpu->flags.Zero) {
		cpu->bus->read(cpu, ADDRESS_RELATIVE_8, false, 0);
		cpu->PC = cpu->holder.ptr;
		printf("cpu_branch_notequal: branching to %X \n", *(cpu->PC));
	} else {
		cpu->PC += 2;
	}
	return;
};

static void sn_OpBPL(snCPU* cpu) {
	if (!cpu->flags.Negative) {
		cpu->bus->read(cpu, ADDRESS_RELATIVE_8, false, 0);
		cpu->PC = cpu->holder.ptr;
		printf("cpu_branchplus: %i -> branching to %X \n", cpu->holder.value, *cpu->PC);
	} else {
		cpu->PC += 2;
	}
};

static void sn_OpBRA(snCPU* cpu) {
	cpu->bus->read(cpu, ADDRESS_RELATIVE_8, false, 0);
	cpu->PC = cpu->holder.ptr;
	printf("cpu_branch: %X -> branching to %X\n", (s8) cpu->holder.value, *cpu->PC);
};

static void sn_OpCLC(snCPU* cpu) {
	// cpu->flags ^= 0x01;
	cpu->flags.Carry = 0;
	cpu->PC++;
	return;
};

static void sn_OpCLD(snCPU* cpu) {
	// cpu->flags ^= 0x08;
	cpu->flags.Decimal = 0;
	cpu->PC++;
	return;
};

static void sn_OpCLI(snCPU* cpu) {
	// emulator->cpu->flags ^= 0x04;
	cpu->flags.Interrupt = 0;
	cpu->PC++;
	return;
};

static void sn_OpCLV(snCPU* cpu) {
	cpu->flags.Overflow = 0;
	cpu->PC++;
	return;
};

static void sn_OpCMP_const(snCPU* cpu) {
	if (!cpu->flags.Accumulator) {
		cpu->bus->read(cpu, ADDRESS_CONST_16, false, 0x00);
	} else {
		cpu->bus->read(cpu, ADDRESS_CONST_8, false, 0x00);
	}
	cpu->flags.Negative = (cpu->A - cpu->holder.value < 0 ? 1 : 0);
	cpu->flags.Zero = (cpu->A - cpu->holder.value == 0 ? 1 : 0);
};

static void sn_OpCMP_addr(snCPU* cpu) {
	if (!cpu->flags.Accumulator) {
		cpu->bus->read(cpu, ADDRESS_ABSOLUTE_16, false, 0x00);
	} else {
		cpu->bus->read(cpu, ADDRESS_ABSOLUTE_8, false, 0x00);
	};

	(cpu->flags.Negative = (cpu->A - cpu->holder.value) < 0) ? 1 : 0;
	(cpu->flags.Zero = (cpu->A - cpu->holder.value) == 0) || (cpu->A == cpu->holder.value) ? 1 : 0;
	printf("cmp_address: comparing %X with accumulator: %X \n", cpu->holder.value, cpu->A);
	++cpu->PC;
	// sleep(1);
};

static void sn_OpDEA(snCPU* cpu) {
	cpu->A--;
	cpu->flags.Negative = ((s16) cpu->A < 0 ? 1 : 0);
	cpu->flags.Zero = (cpu->A == 0 ? 1 : 0);
	cpu->PC++;
	return;
};

static void sn_OpDEX(snCPU* cpu) {
	cpu->X--;
	cpu->flags.Negative = ((s16) cpu->X < 0 ? 1 : 0);
	cpu->flags.Zero = (cpu->X == 0 ? 1 : 0);
	cpu->PC++;
	return;
};

static void sn_OpDEY(snCPU* cpu) {
	cpu->Y--;
	cpu->flags.Negative = ((s16) cpu->Y < 0 ? 1 : 0);
	cpu->flags.Zero = (cpu->Y == 0 ? 1 : 0);
	cpu->PC++;
	return;
};

static void sn_OpJSR_addr(snCPU* cpu) {
	cpu->bus->read(cpu, ADDRESS_ABSOLUTE_16, false, 0x00);
	// cpu->subPC = cpu->PC;
	cpu->PC = cpu->holder.ptr;
	
	printf("jump_subroutine: %X %X -> %X\n", cpu->resolver->realBank, cpu->resolver->realAddr, *cpu->PC);
	return;
};

static void sn_OpINA(snCPU* cpu) {
	cpu->A++;
	cpu->flags.Negative = ((s16) cpu->A < 0 ? 1 : 0);
	cpu->flags.Zero = (cpu->A == 0 ? 1 : 0);
	cpu->PC++;
	return;
};

static void sn_OpINC_dp(snCPU* cpu) {
	cpu->DP++;
	cpu->flags.Negative = ((s16) cpu->DP < 0 ? 1 : 0);
	cpu->flags.Zero = (cpu->DP == 0 ? 1 : 0);
	return;
};

static void sn_OpINC_addr(snCPU* cpu) {
	cpu->bus->read(cpu, ADDRESS_ABSOLUTE_16, false, 0x00);
	cpu->bus->write(cpu, +1);
	cpu->flags.Negative = ((s16) cpu->holder.value < 0 ? 1 : 0);
	cpu->flags.Zero = (cpu->holder.value == 0 ? 1 : 0);
	cpu->PC++;
	return;
};

static void sn_OpINC_dpX(snCPU* cpu) {
	cpu->bus->read(cpu, ADDRESS_ABSOLUTE_8, false, cpu->DP + cpu->X);
	cpu->bus->write(cpu, +1);
	cpu->flags.Negative = ((s16) cpu->holder.value < 0 ? 1 : 0);
	cpu->flags.Zero = (cpu->holder.value == 0 ? 1 : 0);
	cpu->PC++;
	return;
};

static void sn_OpINC_addrX(snCPU* cpu) {
	cpu->bus->read(cpu, ADDRESS_ABSOLUTE_16, false, cpu->X);
	cpu->bus->write(cpu, +1);
	cpu->flags.Negative = ((s16) cpu->holder.value < 0 ? 1 : 0);
	cpu->flags.Zero = (cpu->holder.value == 0 ? 1 : 0);
	cpu->PC++;
	return;
};

static void sn_OpINX(snCPU* cpu) {
	cpu->X += 1;
	cpu->flags.Negative = ((s16) cpu->X < 0 ? 1 : 0);
	cpu->flags.Zero = (cpu->X == 0 ? 1 : 0);
	cpu->PC++;
	return;
};

static void sn_OpINY(snCPU* cpu) {
	cpu->Y += 1;
	cpu->flags.Negative = ((s16) cpu->Y < 0 ? 1 : 0);
	cpu->flags.Zero = (cpu->Y == 0 ? 1 : 0);
	cpu->PC++;
	return;
};

static void sn_OpJMP_addr(snCPU* cpu) {
	// cpu->bus->readU16absolute(cpu, 0);
	cpu->bus->read(cpu, ADDRESS_ABSOLUTE_16, false, 0x00);
	cpu->PC = cpu->holder.ptr;
	cpu->PC++;
	return;
};

static void sn_OpJMP_long(snCPU* cpu) {
	/* call emulator->memory->returnPtrBank*/
	//emulator->cpu->PC = sn_Mread_u24(emulator, 0) - 1;
	// cpu->bus->readU24absolute(cpu, 0);

	cpu->bus->read(cpu, ADDRESS_ABSOLUTE_24, false, 0x00);
	cpu->PC = cpu->resolver->addrPtr;
	//cpu->PC++;
	return;
};

static void sn_OpLDA_const(snCPU* cpu) {
	if (!cpu->flags.Accumulator) {
		cpu->bus->read(cpu, ADDRESS_CONST_16, false, 0x00);
	} else {
		cpu->bus->read(cpu, ADDRESS_CONST_8, false, 0x00);
	}
	cpu->A = cpu->holder.value;
	cpu->flags.Negative = (s16) cpu->A < 0 ? 1 : 0;
	cpu->flags.Zero = cpu->A == 0 ? 1 : 0;
	++cpu->PC;
	// usleep(10000);
	return;
};


static void sn_OpLDA_addr(snCPU* cpu) {
	// cpu->bus->readU16absolute(cpu, 0x00);

	cpu->bus->read(cpu, ADDRESS_ABSOLUTE_16, false, 0x00);
	cpu->A = cpu->holder.value;
	cpu->flags.Negative = (s16) cpu->A < 0 ? 1 : 0;
	cpu->flags.Zero = cpu->A == 0 ? 1 : 0;
	cpu->PC++;
	return;
};

static void sn_OpLDA_DP_indirect_long_Y(snCPU* cpu) {
	cpu->bus->read(cpu, ADDRESS_DP_24, true, 0);
	if (!cpu->flags.Accumulator) {
		hLoAddr = *(cpu->holder.ptr + cpu->Y);
		hHiAddr = *(cpu->holder.ptr + cpu->Y + 1);				
		cpu->A = (hHiAddr << 8 | hLoAddr);
	} else {
		cpu->A = cpu->holder.value + cpu->Y;
	}
	cpu->PC++;
	return;
};


static void sn_OpLDX_const(snCPU* cpu) {
	if (!cpu->flags.Index) {
		cpu->bus->read(cpu, ADDRESS_CONST_16, false, 0x00);
	} else {

	cpu->bus->read(cpu, ADDRESS_CONST_8, false, 0x00);
	}
	cpu->X = cpu->holder.value;
	cpu->flags.Negative = (s16) cpu->X < 0 ? 1 : 0;
	cpu->flags.Zero = cpu->X == 0 ? 1 : 0;
	cpu->PC++;
	return;
};

static void sn_OpLDY_addr(snCPU* cpu) {
	/*TODO: make new functions for replacing
	 * pointer dereference of u16 addresses */
	/* TODO: DID I ALREADY DO IT????? IDK BRO */
	//sn_Mread_u16_absolute(emulator, 0);
	// cpu->bus->readU16absolute(cpu, 0);

	cpu->bus->read(cpu, ADDRESS_ABSOLUTE_16, false, 0x00);
	cpu->Y = cpu->holder.value;
	cpu->flags.Negative = (s16) cpu->Y < 0 ? 1 : 0;
	cpu->flags.Zero = cpu->Y == 0 ? 1 : 0;
	return;
};
static void sn_OpLDY_const(snCPU* cpu) {
	if (!cpu->flags.Index) {
		cpu->bus->read(cpu, ADDRESS_CONST_16, false, 0x00);
	} else {
		cpu->bus->read(cpu, ADDRESS_CONST_8, false, 0x00);
	}
	cpu->Y = cpu->holder.value;
	cpu->flags.Negative = (s16) cpu->Y < 0 ? 1 : 0;
	cpu->flags.Zero = cpu->Y == 0 ? 1 : 0;
	cpu->PC++;
	return;
};

static void sn_OpPHA(snCPU* cpu) {
	if (cpu->flags.Accumulator) {
		cpu->stack.push(cpu, CPU_STACK_PUSH16, cpu->A);
	} else {
		cpu->stack.push(cpu, CPU_STACK_PUSH8, cpu->A);
	}
	cpu->PC++;
}

static void sn_OpPHP(snCPU* cpu) {
	cpu->stack.push(cpu, CPU_STACK_PUSH8, cpu->flags.value);
	cpu->PC++;
};

static void sn_OpSTA_addr(snCPU* cpu) {
	cpu->bus->read(cpu, ADDRESS_ABSOLUTE_16, false, 0x00);
	if (!cpu->flags.Accumulator) {
		cpu->bus->write_test(cpu, BUS_WRITE_16, cpu->A, 0);
	} else {	
		cpu->bus->write_test(cpu, BUS_WRITE_8, cpu->A, 0);
	}
	cpu->PC++;
	return;
};

static void sn_OpSTA_long(snCPU* cpu) {
	// cpu->bus->readU24absolute(cpu, 0);
	cpu->bus->read(cpu, ADDRESS_ABSOLUTE_24, false, 0x00);

	if (!cpu->flags.Accumulator) {
		cpu->bus->write_test(cpu, BUS_WRITE_16, cpu->A, 0);
	} else {
		cpu->bus->write_test(cpu, BUS_WRITE_8, cpu->A, 0);
	}
	sleep(1);
	cpu->PC++;
	return;
};

static void sn_OpSTA_longX(snCPU* cpu) {
	cpu->bus->read(cpu, ADDRESS_ABSOLUTE_24, false, 0x00);

	if (!cpu->flags.Accumulator) {
		cpu->bus->write_test(cpu, BUS_WRITE_16, cpu->A, cpu->X);
	} else {
		cpu->bus->write_test(cpu, BUS_WRITE_8, cpu->A, cpu->X);
	}
	cpu->PC++;
	return;
};

static void sn_OpSTZ_dp(snCPU* cpu) {
	////sn_Mwrite(emulator, emulator->cpu->DP, 0, sn_Mread_u8_const(emulator, 0), 0);
	return;
};
static void sn_OpSTZ_dpX(snCPU* cpu) {
	////sn_Mwrite(emulator, sn_Mread_u8_const(emulator, emulator->cpu->DP), emulator->cpu->X, 0);
	return;
};
static void sn_OpSTZ_addr(snCPU* cpu) {
	cpu->bus->read(cpu, ADDRESS_ABSOLUTE_16, false, 0x00);
	cpu->bus->write(cpu, 0);
	cpu->PC++;
	return;
};
static void sn_OpSTZ_addrX(snCPU* cpu) {
	cpu->bus->read(cpu, ADDRESS_ABSOLUTE_16, false, cpu->X);
	cpu->bus->write(cpu, 0);
	cpu->PC++;
	return;
};

static void sn_OpSEC(snCPU* cpu) {
	cpu->flags.Carry = 1;
	cpu->PC++;
	return;
};

static void sn_OpSED(snCPU* cpu) {
	cpu->flags.Decimal = 1;
	cpu->PC++;
	return;
};

static void sn_OpSEI(snCPU* cpu) {
	cpu->flags.Interrupt = 1;
	cpu->PC++;
	return;
};

static void sn_OpREP(snCPU* cpu) {
	cpu->bus->read(cpu, ADDRESS_CONST_8, false, 0x00);
	cpu->flags.Carry = (cpu->holder.value & 0x80 ? 0 : cpu->flags.Carry);
	cpu->flags.Overflow = (cpu->holder.value & 0x40 ? 0 : cpu->flags.Overflow);
	cpu->flags.Accumulator = (cpu->holder.value & 0x20 ? 0 : cpu->flags.Accumulator);
	cpu->flags.Index = (cpu->holder.value & 0x10 ? 0 : cpu->flags.Index);
	cpu->flags.Decimal = (cpu->holder.value & 0x08 ? 0 : cpu->flags.Decimal);
	cpu->flags.Interrupt = (cpu->holder.value & 0x04 ? 0 : cpu->flags.Interrupt);
	cpu->flags.Zero = (cpu->holder.value & 0x02 ? 0 : cpu->flags.Zero);
	cpu->flags.Emulation = (cpu->holder.value & 0x01 ? 0 : cpu->flags.Emulation);
	cpu->PC++;
	return;
};

static void sn_OpRTS(snCPU* cpu) {
	printf("cpu: rts executed, which is good, implement function return\n");
	sleep(5);
};

static void sn_OpSBC_addrX(snCPU* cpu) {
	cpu->bus->read(cpu, ADDRESS_ABSOLUTE_16, false, cpu->X);
	cpu->A = cpu->A - cpu->holder.value + (!cpu->flags.Carry ? 1 : 0);
	cpu->flags.Negative = ((s16) cpu->A < 0 ? 1 : 0);
	cpu->flags.Zero = (cpu->A == 0 ? 1 : 0);
	cpu->flags.Overflow = ((cpu->A) > 0xFFFF ? 1 : 0);
	cpu->flags.Carry = ((s16) cpu->A == 0 ? 1 : 0);
	cpu->PC++;
};
static void sn_OpSBC_const(snCPU* cpu) {
	if (!cpu->flags.Accumulator) {
		cpu->bus->read(cpu, ADDRESS_CONST_16, false, 0x00);
	} else {
		cpu->bus->read(cpu, ADDRESS_CONST_8, false, 0x00);
	}
	cpu->A = cpu->A - cpu->holder.value + (!cpu->flags.Carry ? 1 : 0);
	cpu->flags.Negative = ((s16) cpu->A < 0 ? 1 : 0);
	cpu->flags.Zero = (cpu->A == 0 ? 1 : 0);
	cpu->flags.Overflow = ((cpu->A) > 0xFFFF ? 1 : 0);
	cpu->flags.Carry = ((s16) cpu->A == 0 ? 1 : 0);
	cpu->PC++;
};

static void sn_OpSEP(snCPU* cpu) {
	cpu->bus->read(cpu, ADDRESS_CONST_8, false, 0x00);
	cpu->flags.Carry = (cpu->holder.value & 0x80 ? 1 : cpu->flags.Carry);
	cpu->flags.Overflow = (cpu->holder.value & 0x40 ? 1 : cpu->flags.Overflow);
	cpu->flags.Accumulator = (cpu->holder.value & 0x20 ? 1 : cpu->flags.Accumulator);
	cpu->flags.Index = (cpu->holder.value & 0x10 ? 1 : cpu->flags.Index);
	cpu->flags.Decimal = (cpu->holder.value & 0x08 ? 1 : cpu->flags.Decimal);
	cpu->flags.Interrupt = (cpu->holder.value & 0x04 ? 1 : cpu->flags.Interrupt);
	cpu->flags.Zero = (cpu->holder.value & 0x02 ? 1 : cpu->flags.Zero);
	cpu->flags.Emulation = (cpu->holder.value & 0x01 ? 1 : cpu->flags.Emulation);
	cpu->PC++;
	return;
};

static void sn_OpTAX(snCPU* cpu) {
	cpu->X = cpu->A;
	cpu->flags.Negative = ((s16) cpu->X < 0 ? 1 : 0);
	cpu->flags.Zero = (cpu->X == 0 ? 1 : 0);
	cpu->PC++;
	return;
};

static void sn_OpTCS(snCPU* cpu) {
	cpu->SP = cpu->A;
	cpu->flags.Negative = ((s16) cpu->SP < 0 ? 1 : 0);
	cpu->flags.Zero = (cpu->SP == 0 ? 1 : 0);
	cpu->PC++;
	return;
};

static void sn_OpTAY(snCPU* cpu) {
	cpu->Y = cpu->A;
	cpu->flags.Negative = ((s16) cpu->Y < 0 ? 1 : 0);
	cpu->flags.Zero = (cpu->Y == 0 ? 1 : 0);
	cpu->PC++;
	return;
};

static void sn_OpTCD(snCPU* cpu) {
	cpu->DP = cpu->A;
	cpu->flags.Negative = ((s16) cpu->DP < 0 ? 1 : 0);
	cpu->flags.Zero = (cpu->DP == 0 ? 1 : 0);
	cpu->PC++;
	return;
};

static void sn_OpTXY(snCPU* cpu) {
	cpu->A = cpu->Y;
	cpu->flags.Negative = ((s16) cpu->A < 0 ? 1 : 0);
	cpu->flags.Zero = (cpu->A == 0 ? 1 : 0);
	cpu->PC++;
	return;
};

static void sn_OpTYA(snCPU* cpu) {
	cpu->A = cpu->Y;
	cpu->flags.Negative = ((s16) cpu->A < 0 ? 1 : 0);
	cpu->flags.Zero = (cpu->A == 0 ? 1 : 0);
	cpu->PC++;
	return;
};

static void sn_OpTYX(snCPU* cpu) {
	cpu->A = cpu->Y;
	cpu->flags.Negative = ((s16) cpu->A < 0 ? 1 : 0);
	cpu->flags.Zero = (cpu->A == 0 ? 1 : 0);
	cpu->PC++;
	return;
};

static void sn_OpXCE(snCPU* cpu) {
	u16 temp_C = cpu->flags.Carry;
	u16 temp_E = cpu->flags.Emulation;

	cpu->flags.Emulation = temp_C;
	cpu->flags.Carry = temp_E;
	cpu->PC++;
	return;
}

static void fetchCPU(snCPU* cpu) {
	/* emulator->cpu->PC = emulator->memory->bank_array[emulator->cpu->PB][emulator->cpu->PC] */
	/* or/and, make a variable called emulator->cpu->BankChanged, that opcodes like JMP/JSR can 
	 * enable, making the bank change. */

	/* remember to use cpu_Opcodes[*++emulator->cpu->PC](emulator->cpu)
	 * fast and btw, we will need that 'cause of PBR register 
				
			that's a strange coincidence */

	printf("cpu_fetch: A: %X X: %X Y: %X SP: %X Flags: %08b DP: %X\n", cpu->A, cpu->X,cpu->Y,cpu->SP, cpu->flags, cpu->DP);
	printf("cpu_opcode: %X \n", *cpu->PC);
	switch (*(cpu->PC)) {
		case _bit_const:
			sn_OpBIT_const(cpu);
			break;
		case _bmi:
			sn_OpBMI(cpu);
			break;
		case _bne:
			sn_OpBNE(cpu);
			break;
		case _bpl:
			sn_OpBPL(cpu);
			break;
		case _bra:
			sn_OpBRA(cpu);
			break;
		case _clc:
			sn_OpCLC(cpu);
			break;
		case _cld:
			sn_OpCLD(cpu);
			break;
		case _cli:
			sn_OpCLI(cpu);
			break;
		case _clv:
			sn_OpCLV(cpu);
			break;
		case _cmp_addr:
			sn_OpCMP_addr(cpu);
			break;
		case _dea:
			sn_OpDEA(cpu);
			break;
		case _dex:
			sn_OpDEX(cpu);
			break;
		case _dey:
			sn_OpDEY(cpu);
			break;
		case _inc_a:
			sn_OpINA(cpu);
			break;
		case _inc_dp:
			sn_OpINC_dp(cpu);
			break;
		case _inc_addr:
			sn_OpINC_addr(cpu);
			break;
		case _inc_dp_x:
			sn_OpINC_dpX(cpu);
			break;
		case _inc_addr_x:
			sn_OpINC_addrX(cpu);
			break;
		case _inx:
			sn_OpINX(cpu);
			break;
		case _iny:
			sn_OpINY(cpu);
			break;
		case _jmp_addr:
			sn_OpJMP_addr(cpu);
			break;
		case _jsr_addr:
			sn_OpJSR_addr(cpu);
			break;
		case _lda_addr:
			sn_OpLDA_addr(cpu);
			break;
		case _lda_const:
			sn_OpLDA_const(cpu);
			break;
		case _lda_dp_indr_l_y:
			sn_OpLDA_DP_indirect_long_Y(cpu);
			break;
		case _ldx_const:
			sn_OpLDX_const(cpu);
			break;
		case _ldy_addr:
			sn_OpLDY_addr(cpu);
			break;
		case _ldy_const:
			sn_OpLDY_const(cpu);
			break;
		case _pha:
			sn_OpPHA(cpu);
			break;
		case _php:
			sn_OpPHP(cpu);
		break;
		case _sec:
			sn_OpSEC(cpu);
		break;
		case _sed:
			sn_OpSED(cpu);
		break;
		case _sbc_addr_x:
			sn_OpSBC_addrX(cpu);
		break;
		case _sbc_const:
			sn_OpSBC_const(cpu);
			break;
		case _sei:
			sn_OpSEI(cpu);
			break;
		case _rep:
			sn_OpREP(cpu);
			break;
		case _sep:
			sn_OpSEP(cpu);
			break;
		case _sta_addr:
			sn_OpSTA_addr(cpu);
			break;
		case _sta_l:
			sn_OpSTA_long(cpu);
			break;
		case _sta_l_x:
			sn_OpSTA_longX(cpu);
			break;
		case _stz_dp:
			sn_OpSTZ_dp(cpu);
			break;
		case _stz_dp_x:
			sn_OpSTZ_dpX(cpu);
			break;
		case _stz_addr:
			sn_OpSTZ_addr(cpu);
			break;
		case _stz_addr_x:
			sn_OpSTZ_addrX(cpu);
			break;
		case _tax:
			sn_OpTAX(cpu);
			break;
		case _tay:
			sn_OpTAY(cpu);
			break;
		case _tcd:
			sn_OpTCD(cpu);
			break;
		case _tcs:
			sn_OpTCS(cpu);
			break;
		case _txy:
			sn_OpTXY(cpu);
			break;
		case _tya:
			sn_OpTYA(cpu);
			break;
		case _tyx:
			sn_OpTYX(cpu);
			break;
		case _xce:
			sn_OpXCE(cpu);
			break;
		case 0x00:
			printf("cpu_fetch: cpu crossed at zero, something wrong? \n \
			       last byte: %02X current byte: %02X next byte: %02X at \n \
			       address: %X\n", *(cpu->PC - 1), *cpu->PC, *(cpu->PC + 1), cpu->memory->address_target);
			
			exit(1);
			break;
		default:
			printf("%X %X %X\n", cpu->memory->bank_array[cpu->resolver->indexer->wramL][0x00],
				cpu->memory->bank_array[cpu->resolver->indexer->wramL][0x01],
				cpu->memory->bank_array[cpu->resolver->indexer->wramL][0x02]);
			
			printf("cpu_fetch: unknown opcode %X \n",*(cpu->PC));
			exit(1);
			break;
	};
	printf("%p \n", cpu->PC);
	return;
}

extern void setupCPU(emGeneral* emulator, rom* rom_Ptr, addrResolver* resolver) {
	printf("cpu_setup: init\n");
	/* Emulation Mode */
	emulator->cpu->flags.Emulation = 1;
	emulator->cpu->flags.Accumulator = 1;
	emulator->cpu->flags.Interrupt = 0;
	emulator->cpu->flags.Index = 1;
	emulator->cpu->SP = 0x100;
	emulator->cpu->holder.value = 0;

	/* this sets registers to zero
	 * this is only made for debugging
	 * the real SNES can point these
	 * values to every thing */

	emulator->cpu->P = 0;
	emulator->cpu->PB = 0;
	emulator->cpu->DBR = 0;
	emulator->cpu->fetch = fetchCPU;
	emulator->cpu->resolver = resolver;
	emulator->cpu->memory = emulator->memory;

	printf("cpu_setup: done\n");
	emulator->cpu->PC = &emulator->memory->bank_array[emulator->cpu->PB][rom_Ptr->resetV - 0x8000];
	printf("cpu_setup: starting at %X %X \n", emulator->memory->bank_array[emulator->cpu->PB][0], emulator->cpu->PC);
}
