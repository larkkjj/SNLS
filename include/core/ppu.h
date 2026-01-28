#ifndef SNLS_PPU_HEADER
#define SNLS_PPU_HEADER

#include "general/types.h"
#include "general/references.h"
#include "emulator/memory.h"

typedef struct snPPU {
	u8	located;

	u8	IniDisp;
	u8	ObjSel;
	u8	OamADDL;
	u8	OamADDH;
	u8	OamData;
	u8	BgMode;
	u8	Mosaic;
	u8	Bg1SC;
	u8	Bg2SC;
	u8	Bg3SC;
	u8	Bg4SC;
	u8	Bg12NBA;
	u8	Bg34NBA;
	u8	Bg1HOFS;
	u8 	M7HOFS;
	u8	Bg1VOFS;
	u8  	M7VOFS;
	u8	Bg2HOFS;
	u8	Bg2VOFS;
	u8	Bg3HOFS;
	u8	Bg3VOFS;
	u8	Bg4HOFS;
	u8	Bg4VOFS;
	u8	VMain;
	u8	VmAddL;
	u8	VmAddH;
	u8	VmDataL;
	u8	VmDataH;

	u8	cgAdd;
	u8	cgData;
	u8	cgDataRead;

	void		(*fetch)(snPPU* ppu);
} snPPU;

typedef struct p_Tile {
    u16     tileSprite;
} p_Tile;


static u8 NTSC_SCREEN[256][224];
static u8 PAL_SCREEN[256][264];
#endif
