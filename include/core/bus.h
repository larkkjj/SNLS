#ifndef SNLS_BUS_HEADER
#define SNLS_BUS_HEADER

#include <stdbool.h>
#include "general/types.h" 
#include "general/references.h"
#include "emulator/memory.h"

#define ADDRESS_CONST_8			0x1
#define ADDRESS_CONST_16		0x2
#define ADDRESS_RELATIVE_8	0x3
#define ADDRESS_RELATIVE_16	0x4
#define ADDRESS_ABSOLUTE_8	0x5
#define ADDRESS_ABSOLUTE_16	0x6
#define ADDRESS_ABSOLUTE_24	0x7
// #define ADDRESS_DP_8				0x8  not used
// #define ADDRESS_DP_16				0x9 not used
#define ADDRESS_DP					0x8
#define ADDRESS_DP_24				0x9

typedef struct cpuBUS {
	__attribute__	((access(read_only, 1)))
	void		(*read)(snCPU* cpu, char mode, bool indirect, u16 offset);

	__attribute__	((access(read_only, 1)))
	void		(*write)(snCPU* cpu, u16 value);

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

