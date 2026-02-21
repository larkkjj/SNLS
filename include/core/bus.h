#ifndef SNLS_BUS_HEADER
#define SNLS_BUS_HEADER

#include <stdbool.h>
#include "general/types.h" 
#include "general/references.h"
#include "emulator/memory.h"

#define ADDRESS_CONST									0x0
#define ADDRESS_RELATIVE_8						0x3
#define ADDRESS_RELATIVE_16						0x4
#define ADDRESS_ABSOLUTE_8						0x5
#define ADDRESS_ABSOLUTE_16						0x6
#define ADDRESS_ABSOLUTE_16_INDIRECT	0x7
#define ADDRESS_ABSOLUTE_24						0x8
#define ADDRESS_ABSOLUTE_24_INDIRECT	0x9
#define ADDRESS_DP										0xA
#define ADDRESS_DP_24									0xB
#define ADDRESS_DP_INDIRECT						0xC
#define ADDRESS_DP_24_INDIRECT				0xD
#define ASSIGN_NONE										0x0
#define ASSIGN_16											0x1
#define ASSIGN_ACCUMULATOR						0x2
#define ASSIGN_INDEX									0x3

#define BUS_WRITE_16 0x0
#define BUS_WRITE_8  0x1


typedef struct cpuBUS {
	__attribute__	((access(read_only, 1)))
	void		(*read)(snCPU* cpu, char mode, char assign_mode, u16 offset);

	__attribute__	((access(read_only, 1)))
	void		(*write)(snCPU* cpu, char mode, u16 value, u16 offset);

	emMemory*			memory;
	memoryHolder*	holder;
	addrResolver*	resolver;
} cpuBUS;

typedef struct spcBUS {
	u8*		pointer;
	u16		value;
	u16		address;

	__attribute__	((access(read_only, 1)))
	void		(*readU8const)(snSPC* spc, u16 offset);

	__attribute__	((access(read_only, 1)))
	void		(*readU16const)(snSPC* spc);

	__attribute__	((access(read_only, 1)))
	void		(*readU16absolute)(snSPC* spc, u16 offset);

	__attribute__	((access(read_only, 1)))
	void		(*readU16absoluteIndirect)(snSPC* spc, u8 mode);

	__attribute__	((access(read_only, 1)))
	void		(*write)(snSPC* spc, u16 value);

	__attribute__	((access(read_only, 1)))
	void		(*writeIndirect)(snSPC* spc, u16 offset, u16 value, u8 mode);
	emGeneral*	emulator;
	addrResolver	resolver;
} spcBUS;

#endif

