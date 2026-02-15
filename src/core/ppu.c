#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "emulator/memory.h"
#include "emulator/main.h"

static void fetchPPU(snPPU* ppu) {
	/* this doesn't "fetch" the ppu, it just prints a value
	 * of a register to check if setupping is working */
	printf("ppu_fetch: ini_disp: 0x%X bgmode: 0x%X cgadd: 0x%X cgdata: 0x%X cgdataread: 0x%X\n", ppu->IniDisp, ppu->BgMode, ppu->cgAdd, ppu->cgData, ppu->cgDataRead);

	p_Tile sprite;
	sprite.tileSprite = ppu->VmAddL << 8 | ppu->VmAddH;
	/// sprite.palette = emulator->ppu->cgData;
	return;
}

extern void setupPPU(emGeneral* emulator, u8** buffer) {
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
	buffer[0x0D] = &emulator->ppu->hScroll.pointer;
	buffer[0x0E] = &emulator->ppu->vScroll.pointer;
	// buffer[0x0D] = &emulator->ppu->Bg1HOFS;
	//buffer[0x0D] = &emulator->ppu->M7HOFS;
	// buffer[0x0E] = &emulator->ppu->Bg1VOFS;
	//buffer[0x0E] = &emulator->ppu->M7VOFS;
	buffer[0x0F] = &emulator->ppu->Bg2HOFS;
	buffer[0x10] = &emulator->ppu->Bg2VOFS;
	buffer[0x11] = &emulator->ppu->Bg3HOFS;
	buffer[0x11] = &emulator->ppu->Bg3VOFS;
	
	buffer[0x16] = &emulator->ppu->VmAddL;
	buffer[0x17] = &emulator->ppu->VmAddH;

	buffer[0x21] = &emulator->ppu->cgAdd;
	buffer[0x22] = &emulator->ppu->cgData;
	buffer[0x23] = &emulator->ppu->cgDataRead;

	emulator->ppu->fetch = fetchPPU;
	printf("ppu_setup: done\n");
	usleep(1000);
	return;
}
