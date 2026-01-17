#ifndef SNLS_BUS_HEADER
#define SNLS_BUS_HEADER

#include "general/types.h" 
#include "general/references.h"
#include "core/cpu.h"
#include "emulator/memory.h"

typedef struct snBUS {
	u8		bank;
	u16		address;
	u8		(*readU8const)(emGeneral* ptrParent, u16 offset);
	u16		(*readU16const)(emGeneral* ptrParent, u16 offset);
	void		(*readU16absolute)(emGeneral* ptrParent, u16 offset);
	u8*		(*write)(emGeneral* ptrParent, u16 offset, u32 value);
	
	emGeneral*	ptrParent;
	u8*		pointer;
} snBUS;

extern u8 sn_Mread_u8_const(emGeneral* emulator, u16 offset);
extern u16 sn_Mread_u16_const(emGeneral* emulator, u16 offset);

extern u8* sn_Mread_u24_absolute(emGeneral* emulator, u8 pagemode);

#endif

