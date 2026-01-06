#ifndef SNLS_PPU_HEADER
#define SNLS_PPU_HEADER

#include "general/types.h"
#include "general/references.h"
#include "emulator/memory.h"

#define INIDISP		0x2100
#define OBJSEL		0x2101

#define OAMADDL		0x2102
#define	OAMADDH		0x2103

#define OAMDATA		0x2104

#define BGMODE		0x2105

#define MOSAIC		0x2106

#define BG1SC		0x2107
#define BG2SC		0x2108
#define BG3SC		0x2109
#define BG4SC		0x210A

#define BG12NBA		0x210B
#define BG34NBA		0x210C

#define BG1HOFS		0x210D
#define M7HOFS		//check this again
#define BG1VOFS		0x210E
#define M7VOFS		//check this again

#define BG2HOFS 0x210F
#define BG2VOFS 0x2110
#define BG3HOFS 0x2111
#define BG3VOFS 0x2112
#define BG4HOFS 0x2113
#define BG4VOFS 0x2114

#define VMAIN	0x2115
#define VMADDL	0x2116
#define VMADDH	0x2117

#define VMDATAL 0x2118
#define VMDATAH 0x2119

#define M7SEL 0x211A

#define M7A 0x211B
#define M7B 0x211C
#define M7C 0x211D
#define M7D 0x211E
#define M7X 0x211F
#define M7Y 0x20

#define CGADD 0x21
#define CGDATA 0x22
#define W12SEL 0x23
#define W34SEL 0x24
#define WOBJSEL 0x25

#define WH0 0x26
#define WH1 0x27
#define WH2 0x28
#define WH3 0x29
#define WBGLOG 0x2A
#define WOBJLOG 0x2B
#define TM 0x2C
#define TS 0x2D
#define TMW 0x2E
#define TSW 0x2F

#define CGWSEL 0x30

#define CGADSUB 0x31

#define COLDATA 0x32

#define SETINIT 0x33

#define MPYL 0x34
#define MPYM 0x35
#define MPYH 0x36

#define SLHV 0x37
#define OAMDATAREAD 0x38
#define VMDATALREAD 0x39
#define VMDATAHREAD 0x3A
#define CGDAATAREAD 0x3B

#define OPHCT 0x3C
#define OPVCT 0x3D
#define STAT77 0x3E
#define STAT78 0x3F

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

	void		(*fetch)(emGeneral* emulator);
} snPPU;

typedef struct p_Tile {
    u16     tileSprite;
} p_Tile;

#endif
