#ifndef SNLS_BUS_HEADER
#define SNLS_BUS_HEADER

#include "general/types.h" 
#include "general/references.h"
#include "core/cpu.h"
#include "emulator/memory.h"

typedef struct snesBUS {
	u8		bank;
	u16		value;
	u16		address;

	void		(*readU8const)(snesBUS* busParent, u16 offset);
	void		(*readU16const)(snesBUS* busParent);
	void		(*readU16absolute)(snesBUS* busParent, u16 offset);
	void		(*readU16absoluteAddr)(snesBUS* busParent, u16 offset);
	void		(*write)(snesBUS* busParent, u32 value);
	void		(*readU24absolute)(snesBUS* busParent, u16 offset);
	
	emGeneral*	emulator;
	u8*		pointer;
} snesBUS;

typedef struct spcBUS {
	u8		bank;
	u16		value;
	u16		address;

	void		(*readU8const)(spcBUS* busParent, u16 offset);
	void		(*readU8absolute)(spcBUS* busParent, u16 offset);
	void		(*readU16const)(spcBUS* busParent);
	void		(*readU16absolute)(spcBUS* busParent, u16 offset);
	void		(*write)(spcBUS* busParent, u32 value);

	emGeneral*	emulator;
	u8*		pointer;
} spcBUS;

#endif

