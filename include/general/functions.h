#include "emulator/rom.h"
#include "emulator/memory.h"
#include "general/references.h"
#include "general/types.h"

extern void setupDMA(emGeneral* emulator, u8** buffer);
extern void setupSPC(emGeneral* emulator, snSPC* spc_ptr, u8** buffer);
extern void setupPPU(emGeneral* emulator, u8** buffer);
extern void setupCPU(emGeneral* emulator, rom* rom_Ptr);

extern void setupCPUBUS(snCPU* cpu, cpuBUS* busParent);
extern void setupSPCBUS(snSPC* spc, spcBUS* busParent);

extern void initWindow();
extern void pollWindow(emGeneral* emulator);

extern void openRom(char* rom_name, rom* rom_Ptr);
extern void attachROM(u8* buffer);
extern void assignToMap(u8** dest, u8** src, unsigned int offset, unsigned int count, unsigned int type);

extern void syncFetch(emGeneral* emulator);
extern void initEmu(rom* rom_Ptr);

extern void setupResolver(indexer* index, snCPU* cpu);
extern void setupCPUstack(cpuStack* stack);
extern void setupCPUstack(cpuStack* stack);
