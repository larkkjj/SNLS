#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "emulator/memory.h"
#include "emulator/main.h"

static void fetchPPU(emGeneral* emulator) {
	*emulator->active |= 0x02;
	/* this doesn't "fetch" the ppu, it just prints a value
	 * of a register to check if setupping is working */
	printf("ppu_fetch: ppu-> %X %X %X %X\n", emulator->ppu->IniDisp, emulator->ppu->BgMode, emulator->ppu->cgAdd, emulator->ppu->cgData);
	*emulator->active ^= 0x02;

	p_Tile sprite;

	sprite.tileSprite = emulator->ppu->VmAddL << 8 | emulator->ppu->VmAddH;
	printf("ppu -> %x\n", sprite.tileSprite);

	return;
}

extern void setupPPU(emGeneral* emulator, u8** buffer, bool absolute) {
	/* don't use absolute if you don't know what you're doing. */
	/* it was here 'cause of a direct setupping method using Map
	 * instead of PPU_setup, it's here for future reasons...
	 * also if you use, keep in mind that you will need a bigger
	 * malloc */
	emulator->ppu->fetch = fetchPPU;

	if (absolute) {
		buffer[INIDISP] = &emulator->ppu->IniDisp;
		buffer[OBJSEL] = &emulator->ppu->ObjSel;
		buffer[OAMADDL] = &emulator->ppu->OamADDL;
		buffer[OAMADDH] = &emulator->ppu->OamADDH;
		buffer[BGMODE] = &emulator->ppu->BgMode;
	} else {
		buffer[0x00] = &emulator->ppu->IniDisp;
		buffer[0x01] = &emulator->ppu->ObjSel;
		buffer[0x02] = &emulator->ppu->OamADDL;
		buffer[0x03] = &emulator->ppu->OamADDH;
		buffer[0x04] = &emulator->ppu->OamData;
		buffer[0x05] = &emulator->ppu->BgMode;
		buffer[0x06] = &emulator->ppu->Mosaic;
		buffer[0x07] = &emulator->ppu->Bg1SC;
		buffer[0x08] = &emulator->ppu->Bg2SC;
		buffer[0x09] = &emulator->ppu->Bg3SC;
		buffer[0x0A] = &emulator->ppu->Bg4SC;
		buffer[0x0B] = &emulator->ppu->Bg12NBA;
		buffer[0x0C] = &emulator->ppu->Bg34NBA;
		buffer[0x0D] = &emulator->ppu->Bg1HOFS;
		//buffer[0x0D] = &emulator->ppu->M7HOFS;
		buffer[0x0E] = &emulator->ppu->Bg1VOFS;
		//buffer[0x0E] = &emulator->ppu->M7VOFS;
		buffer[0x0F] = &emulator->ppu->Bg2HOFS;
		buffer[0x10] = &emulator->ppu->Bg2VOFS;
		buffer[0x11] = &emulator->ppu->Bg3HOFS;
		buffer[0x11] = &emulator->ppu->Bg3VOFS;
		
		buffer[0x16] = &emulator->ppu->VmAddL;
		buffer[0x17] = &emulator->ppu->VmAddH;

		buffer[0x21] = &emulator->ppu->cgAdd;
		buffer[0x22] = &emulator->ppu->cgData;
	}
	return;
}
