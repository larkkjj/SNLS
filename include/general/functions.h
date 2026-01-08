#include "emulator/rom.h"
#include "emulator/memory.h"
#include "general/references.h"
#include "general/types.h"
#include "core/cpu.h"

extern void setupDMA(emGeneral* emulator, u8** buffer);
extern void setupSPC(emGeneral* emulator, snSPC* spc_ptr, u8** buffer);
extern void setupPPU(emGeneral* emulator, u8** buffer);
extern void setupCPU(emGeneral* emulator, rom* rom_Ptr);

extern void initWindow();
extern void pollWindow();

extern void openRom(char* rom_name, rom* rom_Ptr);
extern void attachROM(u8* buffer);
extern void assignToMap(u8** dest, u8** src, unsigned int offset, unsigned int count, unsigned int type);

extern void getMappedBank(u8 index, u16 address, emGeneral* emulator);
extern void syncFetch(emGeneral* emulator);
extern void initEmu(rom* rom_Ptr);
