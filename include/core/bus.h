#include <stdbool.h>
#include "core/cpu.h"
#include "emulator/memory.h"

extern u8 sn_Mread_u8_const(emGeneral* emulator, u16 offset);
extern u16 sn_Mread_u16_const(emGeneral* emulator, u16 offset);

extern u8 sn_Mread_u8_absolute(emGeneral* emulator, u16 offset);
extern u8* sn_Mread_u16_absolute(emGeneral* emulator, u16 offset);
extern u8* sn_Mread_u24_absolute(emGeneral* emulator, u16 pagemode);

extern u8* sn_Mwrite(emGeneral* emulator, u8* address, u16 offset, u32 value);
