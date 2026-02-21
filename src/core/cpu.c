#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "core/cpu.h"
#include "emulator/main.h"
#include "emulator/memory.h"
#include "emulator/rom.h"
#include "core/bus.h"

static void flagsUpdate(snCPU* cpu, u8 flag, u16 value) {
	if (flag & CPU_N_FLAG) {
		cpu->flags.Negative = ((s16) value < 0);
	}
	if (flag & CPU_V_FLAG) {
		if (!cpu->flags.Accumulator) {
			/* forcing a cast to accumulator */
			s32 sigCalc = (s16) cpu->flags.Accumulator - (s16) value;
			cpu->flags.Overflow = ((sigCalc <= -32768 || sigCalc >= 32768) ? 1 : 0);
		} else {
			s16 sigCalc = (s8) cpu->flags.Accumulator - (s8) value;
			cpu->flags.Overflow = ((sigCalc <= -128 || sigCalc >= 127) ? 1 : 0);
		}
	}
	if (flag & CPU_Z_FLAG) {
		cpu->flags.Zero = ((value == 0) ? 1 : 0);
	}
	if (flag & CPU_C_FLAG) {
		switch (*cpu->PC) {
			/* carry acts differently
			depending of the opcode */

		
			/* Rotate Left */
			case (0x2A):
				if (!cpu->flags.Accumulator) {
					cpu->flags.Carry = (cpu->A & 0x8000 ? 0 : 1);
				} else {
					cpu->flags.Carry = (cpu->A & 0x80 ? 0 : 1);
				}
			break;
			case (0x26):
				if (cpu->flags.Emulation) {
					cpu->flags.Carry = (cpu->DP & 0x80 ? 0 : 1);
				}	else {
					cpu->flags.Carry = (cpu->DP & 0x8000 ? 0 : 1);
				}
			break;
			case (0x2E || 0x36 || 0x3E):
				cpu->flags.Carry = (value & 0x80 ? 0 : 1);
			break;
		}
	}
};

static void sn_OpADC_const(snCPU* cpu) {
	cpu->bus->read(cpu, (ADDRESS_CONST), (ASSIGN_NONE), 0x00);
	cpu->A += cpu->holder.value + (cpu->flags.Carry ? 1 : 0);
	cpu->flags.Carry = 0;
	cpu->flagsUpdate(cpu, CPU_N_FLAG | CPU_Z_FLAG | CPU_V_FLAG, 0);
	// cpu->flags.Negative = ((s16) cpu->A < 0 ? 1 : 0);
	// cpu->flags.Zero = (cpu->A == 0 ? 1 : 0);
	// cpu->flags.Overflow = ((cpu->A) > 0xFFFF ? 1 : 0);
	// cpu->flags.Carry = ((s16) cpu->A == 0 ? 1 : 0);
	cpu->PC++;
};

static void sn_OpBIT_dp(snCPU* cpu);
static void sn_OpBIT_addr(snCPU* cpu) {
	cpu->bus->read(cpu, ADDRESS_ABSOLUTE_16, ASSIGN_NONE, 0);
	cpu->flagsUpdate(cpu, (CPU_N_FLAG | CPU_V_FLAG | CPU_Z_FLAG), (cpu->holder.value & cpu->A));
};

static void sn_OpBIT_dpX(snCPU* cpu);
static void sn_OpBIT_addrX(snCPU* cpu);

static void sn_OpBIT_const(snCPU* cpu) {
	cpu->holder.value = (cpu->A & ++*cpu->PC);
	cpu->flagsUpdate(cpu, (CPU_Z_FLAG), (cpu->holder.value & cpu->A));
};


static void sn_OpBCC(snCPU* cpu) {
	cpu->bus->read(cpu, (ADDRESS_RELATIVE_8), (ASSIGN_NONE), 0);
	if (!cpu->flags.Carry) {
		cpu->PC =  cpu->holder.ptr;
		printf("cpu_minus: branching to %X \n", *(cpu->PC));
	} else {
		cpu->PC += 1;
	}
	return;
};

static void sn_OpBCS(snCPU* cpu) {
	cpu->bus->read(cpu, (ADDRESS_RELATIVE_8), (ASSIGN_NONE), 0);
	if (cpu->flags.Carry) {
		cpu->PC =  cpu->holder.ptr;
		printf("cpu_minus: branching to %X \n", *(cpu->PC));
	} else {
		cpu->PC += 1;
	}
	return;
};

static void sn_OpBEQ(snCPU* cpu) {
	cpu->bus->read(cpu, (ADDRESS_RELATIVE_8), (ASSIGN_NONE), 0);
	if (cpu->flags.Zero) {
		cpu->PC =  cpu->holder.ptr;
		printf("cpu_minus: branching to %X \n", *(cpu->PC));
	} else {
		cpu->PC += 1;
	}
	return;
};



static void sn_OpBMI(snCPU* cpu) {
	cpu->bus->read(cpu, (ADDRESS_RELATIVE_8), (ASSIGN_NONE), 0);
	if (cpu->flags.Negative) {
		cpu->PC =  cpu->holder.ptr;
		printf("cpu_minus: branching to %X \n", *(cpu->PC));
	} else {
		cpu->PC += 1;
	}
	return;
};

static void sn_OpBNE(snCPU* cpu) {
	cpu->bus->read(cpu, (ADDRESS_RELATIVE_8), (ASSIGN_NONE), 0);
	if (!cpu->flags.Zero) {
		cpu->PC = cpu->holder.ptr;
		printf("cpu_branch_notequal: branching to %X \n", *(cpu->PC));
	} else {
		cpu->PC += 1;
	}
	return;
};

static void sn_OpBPL(snCPU* cpu) {
	cpu->bus->read(cpu, (ADDRESS_RELATIVE_8), (ASSIGN_NONE), 0);
	if (!cpu->flags.Negative) {
		cpu->PC = cpu->holder.ptr;
		printf("cpu_branchplus: %d -> branching to %X \n", (s8) cpu->holder.value, *cpu->PC);
	} else {
		cpu->PC += 1;
	}
};

static void sn_OpBRA(snCPU* cpu) {
	cpu->bus->read(cpu, (ADDRESS_RELATIVE_8), (ASSIGN_NONE), 0);
	cpu->PC = cpu->holder.ptr;
	printf("cpu_branch: %X -> branching to %X\n", (s8) cpu->holder.value, *cpu->PC);
};

static void sn_OpBVC(snCPU* cpu) {
	cpu->bus->read(cpu, (ADDRESS_RELATIVE_8), (ASSIGN_NONE), 0);
	if (!cpu->flags.Overflow) {
		cpu->PC = cpu->holder.ptr;
		printf("cpu_branch_overflow_unset: branching to %X \n", *(cpu->PC));
	} else {
		cpu->PC += 1;
	}
	return;
};

static void sn_OpBVS(snCPU* cpu) {
	cpu->bus->read(cpu, (ADDRESS_RELATIVE_8), (ASSIGN_NONE), 0);
	if (cpu->flags.Overflow) {
		cpu->PC = cpu->holder.ptr;
		printf("cpu_branch_overflow_set: branching to %X \n", *(cpu->PC));
	} else {
		cpu->PC += 1;
	}
	return;
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
	cpu->bus->read(cpu, (ADDRESS_CONST), (ASSIGN_ACCUMULATOR), 0x00);
	cpu->flagsUpdate(cpu, (CPU_N_FLAG | CPU_Z_FLAG | CPU_C_FLAG), cpu->holder.value - cpu->A);
	cpu->PC++;
};

static void sn_OpCMP_addr(snCPU* cpu) {
	cpu->bus->read(cpu, ADDRESS_ABSOLUTE_16, ASSIGN_ACCUMULATOR, 0x00);
	cpu->flagsUpdate(cpu, (CPU_N_FLAG | CPU_Z_FLAG | CPU_C_FLAG), (cpu->A - cpu->holder.value));
	printf("cmp_address: comparing %X with accumulator: %X \n", cpu->holder.value, cpu->A);
	cpu->PC++;
};

static void sn_OpCPX_const(snCPU* cpu) {
	cpu->bus->read(cpu, (ADDRESS_CONST), (ASSIGN_INDEX), 0x00);
	cpu->flagsUpdate(cpu, (CPU_N_FLAG | CPU_Z_FLAG), cpu->X - cpu->holder.value);
	cpu->PC++;
}
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
	cpu->bus->read(cpu, ADDRESS_ABSOLUTE_16, ASSIGN_NONE, 0x00);

	u16 return_address = ((cpu->PC) - (cpu->memory->bank_array[cpu->PB]));
	cpu->stack.push(cpu, CPU_STACK_PUSH16, return_address);

	cpu->PC = cpu->holder.ptr;
	u16 point_address = ((cpu->PC) - (cpu->memory->bank_array[cpu->PB]));
	printf("jump_subroutine: start: 0x%X return 0x%X-> return %X\n", return_address + 0x8001, point_address + 0x8000, cpu->memory->bank_array[cpu->DBR][return_address + 1]);
	return;
};

static void sn_OpJSR_long(snCPU* cpu) {
	cpu->bus->read(cpu, ADDRESS_ABSOLUTE_24, ASSIGN_NONE, 0x00);
	u16 return_address = ((cpu->PC) - (cpu->memory->bank_array[cpu->PB]));
	cpu->stack.push(cpu, CPU_STACK_PUSH16, return_address);

	cpu->PB = hBank; /* update bank*/
	cpu->PC = cpu->holder.ptr;
	u16 point_address = ((cpu->PC) - (cpu->memory->bank_array[cpu->PB]));
	printf("jump_subroutine: start: 0x%X return 0x%X-> return %X\n", return_address + 0x8001, point_address + 0x8000, cpu->memory->bank_array[cpu->DBR][return_address + 1]);
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
	cpu->bus->read(cpu, ADDRESS_ABSOLUTE_8, ASSIGN_NONE, cpu->DP);
	cpu->bus->write(cpu, BUS_WRITE_8, +1, 0);
	// cpu->DP++;
	cpu->flags.Negative = ((s16) cpu->DP < 0 ? 1 : 0);
	cpu->flags.Zero = (cpu->DP == 0 ? 1 : 0);
	cpu->PC++;
	return;
};

static void sn_OpINC_addr(snCPU* cpu) {
	cpu->bus->read(cpu, ADDRESS_ABSOLUTE_16, ASSIGN_NONE, 0x00);
	cpu->bus->write(cpu, BUS_WRITE_8, +1, 0);
	cpu->flags.Negative = ((s16) cpu->holder.value < 0 ? 1 : 0);
	cpu->flags.Zero = (cpu->holder.value == 0 ? 1 : 0);
	cpu->PC++;
	return;
};

static void sn_OpINC_dpX(snCPU* cpu) {
	cpu->bus->read(cpu, ADDRESS_ABSOLUTE_8, ASSIGN_NONE, cpu->DP + cpu->X);
	cpu->bus->write(cpu, BUS_WRITE_8, +1, 0);
	cpu->flags.Negative = ((s16) cpu->holder.value < 0 ? 1 : 0);
	cpu->flags.Zero = (cpu->holder.value == 0 ? 1 : 0);
	cpu->PC++;
	return;
};

static void sn_OpINC_addrX(snCPU* cpu) {
	cpu->bus->read(cpu, ADDRESS_ABSOLUTE_16, ASSIGN_NONE, cpu->X);
	cpu->bus->write(cpu, BUS_WRITE_8, +1, 0);
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
	cpu->bus->read(cpu, ADDRESS_ABSOLUTE_16, ASSIGN_NONE, 0x00);
	cpu->PC = cpu->holder.ptr;
	cpu->PC++;
	return;
};

static void sn_OpJMP_long(snCPU* cpu) {
	/* call emulator->memory->returnPtrBank*/
	//emulator->cpu->PC = sn_Mread_u24(emulator, 0) - 1;
	// cpu->bus->readU24absolute(cpu, 0);

	cpu->bus->read(cpu, ADDRESS_ABSOLUTE_24, ASSIGN_NONE, 0x00);
	cpu->PC = cpu->resolver->addrPtr;
	//cpu->PC++;
	return;
};

static void sn_OpLDA_dp(snCPU* cpu) {
	cpu->bus->read(cpu, (ADDRESS_ABSOLUTE_8), ASSIGN_ACCUMULATOR, cpu->DP);
	cpu->A = cpu->holder.value;
	cpu->flagsUpdate(cpu, (CPU_N_FLAG | CPU_Z_FLAG), cpu->A);
	cpu->PC++;
	return;
}

static void sn_OpLDA_const(snCPU* cpu) {
	cpu->bus->read(cpu, (ADDRESS_CONST), (ASSIGN_ACCUMULATOR), 0x00);
	cpu->A = cpu->holder.value;
	cpu->flagsUpdate(cpu, (CPU_N_FLAG | CPU_Z_FLAG), cpu->A);
	cpu->PC++;
	return;
};


static void sn_OpLDA_addr(snCPU* cpu) {
	// cpu->bus->readU16absolute(cpu, 0x00);

	cpu->bus->read(cpu, ADDRESS_ABSOLUTE_16, ASSIGN_ACCUMULATOR, 0x00);
	cpu->A = cpu->holder.value;
	cpu->flagsUpdate(cpu, (CPU_N_FLAG | CPU_Z_FLAG), cpu->A);
	cpu->PC++;
	return;
};


static void sn_OpLDA_addrX(snCPU* cpu) {
	cpu->bus->read(cpu, ADDRESS_ABSOLUTE_16, ASSIGN_ACCUMULATOR, cpu->X);
	cpu->A = cpu->holder.value;
	cpu->flagsUpdate(cpu, (CPU_N_FLAG | CPU_Z_FLAG), cpu->A);
	cpu->PC++;
	return;
};

static void sn_OpLDA_DP_indirect_long_Y(snCPU* cpu) {
	cpu->bus->read(cpu, ADDRESS_DP_24_INDIRECT, ASSIGN_ACCUMULATOR, 0);
	cpu->A = cpu->holder.value;
	cpu->PC++;
	return;
};

static void sn_OpLDX_const(snCPU* cpu) {
	cpu->bus->read(cpu, (ADDRESS_CONST), (ASSIGN_INDEX), 0x00);
	cpu->X = cpu->holder.value;
	cpu->flagsUpdate(cpu, (CPU_N_FLAG | CPU_Z_FLAG), cpu->X);
	cpu->PC++;
	return;
};

static void sn_OpLDY_addr(snCPU* cpu) {
	cpu->bus->read(cpu, ADDRESS_ABSOLUTE_16, ASSIGN_INDEX, 0x00);
	cpu->Y = cpu->holder.value;
	cpu->flags.Negative = (s16) cpu->Y < 0 ? 1 : 0;
	cpu->flags.Zero = cpu->Y == 0 ? 1 : 0;
	cpu->PC++;
	return;
};
static void sn_OpLDY_const(snCPU* cpu) {
	cpu->bus->read(cpu, (ADDRESS_CONST), (ASSIGN_INDEX), 0x00);
	cpu->Y = cpu->holder.value;
	cpu->flags.Negative = (s16) cpu->Y < 0 ? 1 : 0;
	cpu->flags.Zero = cpu->Y == 0 ? 1 : 0;
	cpu->PC++;
	return;
};

static void sn_OpPHA(snCPU* cpu) {
	if (!cpu->flags.Accumulator) {
		cpu->stack.push(cpu, CPU_STACK_PUSH16, cpu->A);
	} else {
		cpu->stack.push(cpu, CPU_STACK_PUSH8, cpu->A);
	}
	cpu->PC++;
}

static void sn_OpPHP(snCPU* cpu) {
	cpu->stack.push(cpu, CPU_STACK_PUSH8, cpu->flags.value);
	u8 hold = cpu->PC - &cpu->memory->bank_array[cpu->DBR][0x0];
	printf("PHP: offset %X %X\n", hold, hold + 0x8000);
	cpu->PC++;
};

static void sn_OpPLA(snCPU* cpu) {
	if (!cpu->flags.Accumulator) {
		cpu->stack.pull(cpu, CPU_STACK_PULL16);
	} else {
		cpu->stack.pull(cpu, CPU_STACK_PULL8);
	}
	cpu->A = cpu->holder.value;
	cpu->PC++;
}

static void sn_OpPLP(snCPU* cpu) {
	cpu->stack.pull(cpu, CPU_STACK_PULL8);
	cpu->flags.value = cpu->holder.value;
	cpu->PC++;
}

static void sn_OpSTA_addr(snCPU* cpu) {
	cpu->bus->read(cpu, ADDRESS_ABSOLUTE_16, ASSIGN_NONE, 0x00);
	if (!cpu->flags.Accumulator) {
		cpu->bus->write(cpu, BUS_WRITE_16, cpu->A, 0);
	} else {	
		cpu->bus->write(cpu, BUS_WRITE_8, cpu->A, 0);
	}
	cpu->PC++;
	return;
};

static void sn_OpSTA_addrX(snCPU* cpu) {
	cpu->bus->read(cpu, ADDRESS_ABSOLUTE_16, ASSIGN_NONE, cpu->X);
	if (!cpu->flags.Accumulator) {
		cpu->bus->write(cpu, BUS_WRITE_16, cpu->A, 0);
	} else {	
		cpu->bus->write(cpu, BUS_WRITE_8, cpu->A, 0);
	}
	cpu->PC++;
	return;
};

static void sn_OpSTA_long(snCPU* cpu) {
	// cpu->bus->readU24absolute(cpu, 0);
	cpu->bus->read(cpu, ADDRESS_ABSOLUTE_24, ASSIGN_NONE, 0x00);

	if (!cpu->flags.Accumulator) {
		cpu->bus->write(cpu, BUS_WRITE_16, cpu->A, 0);
	} else {
		cpu->bus->write(cpu, BUS_WRITE_8, cpu->A, 0);
	}
	cpu->PC++;
	return;
};

static void sn_OpSTA_longX(snCPU* cpu) {
	cpu->bus->read(cpu, ADDRESS_ABSOLUTE_24, ASSIGN_NONE, 0x00);

	if (!cpu->flags.Accumulator) {
		cpu->bus->write(cpu, BUS_WRITE_16, cpu->A, cpu->X);
	} else {
		cpu->bus->write(cpu, BUS_WRITE_8, cpu->A, cpu->X);
	}
	cpu->PC++;
	return;
};

static void sn_OpSTZ_dp(snCPU* cpu) {
	////sn_Mwrite(emulator, emulator->cpu->DP, 0, sn_Mread_u8_const(emulator, 0), 0);
	return;
};
static void sn_OpSTZ_dpX(snCPU* cpu) {
	cpu->bus->read(cpu, ADDRESS_ABSOLUTE_8, ASSIGN_NONE, cpu->DP + cpu->X);
	if (!cpu->flags.Emulation) {
		cpu->bus->write(cpu, BUS_WRITE_16, 0, 0);
	} else {
		cpu->bus->write(cpu, BUS_WRITE_8, 0, 0);
	}
	cpu->PC++;
	return;
};
static void sn_OpSTZ_addr(snCPU* cpu) {
	cpu->bus->read(cpu, ADDRESS_ABSOLUTE_16, ASSIGN_NONE, 0x00);
	cpu->bus->write(cpu, BUS_WRITE_8, 0, 0);
	cpu->PC++;
	return;
};
static void sn_OpSTZ_addrX(snCPU* cpu) {
	cpu->bus->read(cpu, ADDRESS_ABSOLUTE_16, ASSIGN_NONE, cpu->X);
	cpu->bus->write(cpu, BUS_WRITE_8, 0, 0);
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
	printf("cpu_sei \n");
	return;
};

static void sn_OpREP(snCPU* cpu) {
	cpu->bus->read(cpu, (ADDRESS_CONST), (ASSIGN_NONE), 0x00);
	printf("rep: old %09b %09b\n", cpu->flags.value, cpu->holder.value);
	/* i used my brain */
	cpu->flags.value = (cpu->flags.value ^ (cpu->holder.value & cpu->flags.value));
	// cpu->flags.Carry = (cpu->holder.value & 0x80 ? 0 : cpu->flags.Carry);
	// cpu->flags.Overflow = (cpu->holder.value & 0x40 ? 0 : cpu->flags.Overflow);
	// cpu->flags.Accumulator = (cpu->holder.value & 0x20 ? 0 : cpu->flags.Accumulator);
	// cpu->flags.Index = (cpu->holder.value & 0x10 ? 0 : cpu->flags.Index);
	// cpu->flags.Decimal = (cpu->holder.value & 0x08 ? 0 : cpu->flags.Decimal);
	// cpu->flags.Interrupt = (cpu->holder.value & 0x04 ? 0 : cpu->flags.Interrupt);
	// cpu->flags.Zero = (cpu->holder.value & 0x02 ? 0 : cpu->flags.Zero);
	printf("rep: new %09b\n", cpu->flags.value);
	cpu->PC++;
	return;
};

static void sn_OpROL_A(snCPU* cpu) {
	cpu->A <<= 1;
  cpu->flagsUpdate(cpu, CPU_N_FLAG | CPU_Z_FLAG | CPU_C_FLAG, 0);
  ++cpu->PC;
};

static void sn_OpRTS(snCPU* cpu) {
	u16 where = (cpu->PC - &cpu->memory->bank_array[cpu->PB][0x0]);
	cpu->stack.pull(cpu, CPU_STACK_PULL16);

	cpu->PC = (&cpu->memory->bank_array[cpu->PB][cpu->holder.value] + 1);
	u16 ami = (cpu->PC - &cpu->memory->bank_array[cpu->PB][0x0]);
	printf("return_subroutine: %X\n", ami + 0x8000);
	sleep(1);
};

static void sn_OpSBC_addrX(snCPU* cpu) {
	cpu->bus->read(cpu, ADDRESS_ABSOLUTE_16, ASSIGN_NONE, cpu->X);
	cpu->A -= cpu->holder.value + (!cpu->flags.Carry ? 1 : 0);
	cpu->flags.Negative = ((s16) cpu->A < 0 ? 1 : 0);
	cpu->flags.Zero = (cpu->A == 0 ? 1 : 0);
	cpu->flags.Overflow = ((cpu->A) > 0xFFFF ? 1 : 0);
	cpu->flags.Carry = ((s16) cpu->A == 0 ? 1 : 0);
	cpu->PC++;
};

static void sn_OpSBC_const(snCPU* cpu) {
	cpu->bus->read(cpu, ADDRESS_CONST, ASSIGN_ACCUMULATOR, 0x00);
	cpu->A -= cpu->holder.value + (!cpu->flags.Carry ? 1 : 0);
	cpu->flags.Carry = 0;
	cpu->flagsUpdate(cpu, CPU_N_FLAG | CPU_Z_FLAG | CPU_V_FLAG, 0);
	cpu->PC++;
};

static void sn_OpSEP(snCPU* cpu) {
	cpu->bus->read(cpu, (ADDRESS_CONST), (ASSIGN_NONE), 0x00);

	cpu->flags.value = (cpu->flags.value | (cpu->holder.value));
	// cpu->flags.Carry = (cpu->holder.value & 0x80 ? 1 : cpu->flags.Carry);
	// cpu->flags.Overflow = (cpu->holder.value & 0x40 ? 1 : cpu->flags.Overflow);
	// cpu->flags.Accumulator = (cpu->holder.value & 0x20 ? 1 : cpu->flags.Accumulator);
	// cpu->flags.Index = (cpu->holder.value & 0x10 ? 1 : cpu->flags.Index);
	// cpu->flags.Decimal = (cpu->holder.value & 0x08 ? 1 : cpu->flags.Decimal);
	// cpu->flags.Interrupt = (cpu->holder.value & 0x04 ? 1 : cpu->flags.Interrupt);
	// cpu->flags.Zero = (cpu->holder.value & 0x02 ? 1 : cpu->flags.Zero);
	// cpu->flags.Emulation = (cpu->holder.value & 0x01 ? 1 : cpu->flags.Emulation);
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
	u8 temp_C = cpu->flags.Carry;
	u8 temp_E = cpu->flags.Emulation;

	cpu->flags.Emulation = temp_C;
	cpu->flags.Carry = temp_E;
	cpu->PC++;
	return;
}

static void sn_OpWDM(snCPU* cpu) {
	cpu->PC += 2;
}

static void fetchCPU(snCPU* cpu) {
	const u16 where = (cpu->PC - &cpu->memory->bank_array[cpu->PB][0x0]);
	/* emulator->cpu->PC = emulator->memory->bank_array[emulator->cpu->PB][emulator->cpu->PC] */
	/* or/and, make a variable called emulator->cpu->BankChanged, that opcodes like JMP/JSR can 
	 * enable, making the bank change. */

	/* remember to use cpu_Opcodes[*++emulator->cpu->PC](emulator->cpu)
	 * fast and btw, we will need that 'cause of PBR register 
				
			that's a strange coincidence */

	printf("cpu_fetch: A: %X DP: %X X: %X Y: %X SP: %X Flags: %09b %X DP: %X\n", cpu->A, cpu->DP, cpu->X,cpu->Y,cpu->SP, cpu->flags, cpu->flags, cpu->DP);
	printf("cpu_opcode: %X %p\n", *cpu->PC, where + 0x8000);
	switch (*(cpu->PC)) {
		case _adc_const:
			sn_OpADC_const(cpu);
		break;
		case _bit_addr:
			sn_OpBIT_addr(cpu);
		break;
		case _bit_const:
			sn_OpBIT_const(cpu);
			break;
		case _bcc:
			sn_OpBCC(cpu);
		break;
		case _bcs:
			sn_OpBCS(cpu);
		break;
		case _beq:
			sn_OpBEQ(cpu);
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
		case _bvc:
			sn_OpBVC(cpu);
		break;
		case _bvs:
			sn_OpBVS(cpu);
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
		case _cpx_const:
			sn_OpCPX_const(cpu);
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
		case _lda_addr_x:
			sn_OpLDA_addrX(cpu);
		break;
		case _lda_dp:
			sn_OpLDA_dp(cpu);
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
		case _pla:
			sn_OpPLA(cpu);
		break;
		case _plp:
			sn_OpPLP(cpu);
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
		case _rol_a:
			sn_OpROL_A(cpu);
		break;
		case _rts:
			sn_OpRTS(cpu);
		break;
		case _sep:
			sn_OpSEP(cpu);
			break;
		case _sta_addr:
			sn_OpSTA_addr(cpu);
		break;
		case _sta_addr_x:
			sn_OpSTA_addrX(cpu);
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

	u8* test = &cpu->memory->bank_array[0x0][0x1FF];
	printf("stack %X %X %X %X %X\n", *test, *(test - 1), *(test - 2), *(test - 3), *(test - 4));
	// printMemory(cpu->PC, 30);
	return;
}

extern void setupCPU(emGeneral* emulator, rom* rom_Ptr, addrResolver* resolver) {
	printf("cpu_setup: init\n");
	/* Emulation Mode */
	emulator->cpu->flags.value = 0x0;
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
	emulator->cpu->flagsUpdate = flagsUpdate;
	emulator->cpu->resolver = resolver;
	emulator->cpu->memory = emulator->memory;

	printf("cpu_setup: done\n");
	printf("cpu_setup: status flag %09b \n", emulator->cpu->flags.value);
	emulator->cpu->PC = &emulator->memory->bank_array[emulator->cpu->PB][rom_Ptr->resetV];
	printf("cpu_setup: starting at %X %p \n", emulator->memory->bank_array[emulator->cpu->PB][0], emulator->cpu->PC);
}
