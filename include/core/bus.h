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

	__attribute__	((access(read_only, 1)))
	void		(*readU8const)(snesBUS* busParent, u16 offset);

	__attribute__	((access(read_only, 1)))
	void		(*readU16const)(snesBUS* busParent);

	__attribute__	((access(read_only, 1)))
	void		(*readU16absolute)(snesBUS* busParent, u16 offset);

	__attribute__	((access(read_only, 1)))
	void		(*readU16absoluteIndirect)(snesBUS* busParent, u16 offset, u8 mode);

	__attribute__	((access(read_only, 1)))
	void		(*readU24absolute)(snesBUS* busParent, u16 offset);
	
	__attribute__	((access(read_only, 1)))
	void		(*write)(snesBUS* busParent, u16 value);

	__attribute__	((access(read_only, 1)))
	void		(*writeIndirect)(snesBUS* busParent, u16 value, u8 mode);

	emGeneral*	emulator;
	u8*		pointer;
} snesBUS;

typedef struct spcBUS {
	u8*		pointer;
	u16		value;
	u16		address;

	__attribute__	((access(read_only, 1)))
	void		(*readU8const)(spcBUS* busParent, u16 offset);

	__attribute__	((access(read_only, 1)))
	void		(*readU16const)(spcBUS* busParent);

	__attribute__	((access(read_only, 1)))
	void		(*readU16absolute)(spcBUS* busParent, u16 offset);

	__attribute__	((access(read_only, 1)))
	void		(*readU16absoluteIndirect)(spcBUS* busParent, u8 mode);

	__attribute__	((access(read_only, 1)))
	void		(*write)(spcBUS* busParent, u16 value);

	__attribute__	((access(read_only, 1)))
	void		(*writeIndirect)(spcBUS* busParent, u16 offset, u16 value, u8 mode);
	emGeneral*	emulator;
} spcBUS;

#endif

