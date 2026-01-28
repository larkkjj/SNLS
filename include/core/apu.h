#ifndef SNLS_APU_HEADER
#define SNLS_APU_HEADER

#include "general/references.h"
#include "general/types.h"

#define SPC_N_FLAG 	(0x1 << 7)
#define SPC_V_FLAG 	(0x1 << 6)
#define SPC_P_FLAG 	(0x1 << 5)
#define SPC_B_FLAG 	(0x1 << 4)
#define SPC_H_FLAG 	(0x1 << 3)
#define SPC_I_FLAG 	(0x1 << 2)
#define SPC_Z_FLAG 	(0x1 << 1)
#define SPC_C_FLAG 	(0x1)

static u8 IPL_NTSC[0x40] = {
	0xCD, 0xEF, /* load 0xEF */
	0xBD,	    /* copy X to SP */

	0xE8, 0x00,
	0xC6,
	0x1D,

	0xD0, 0xFC,
	0x8F, 0xAA, 0xF4,
	0x8F, 0xBB, 0xF5,

	0x78, 0xCC, 0xF4,
	0xD0, 0xFB,
	0x2F, 0x19,

	0xEB, 0xF4,
	0xD0, 0xFC,

	0x7E, 0xF4,
	0xD0, 0x0B,
	0xE4, 0xF5,
	0xCB, 0xF4,
	0xD7, 0x00,
	0xFC,
	0xD0, 0xF3,
	0xAB, 0x01,

	0x10, 0xEF,
	0x7E, 0xF4,
	0x10, 0xEB,

	0xBA, 0xF6,
	0xDA, 0x00,
	0xBA, 0xF4,
	0xC4, 0xF4,
	0xDD,
	0x5D,
	0xD0, 0xDB,
	0x1F, 0x00, 0x00,

	//0xE8, 0xAA, /* load 0xAA */
	//0xC5, 0xF4, /* store */
	//0xE8, 0xBB, /* load 0xBB */
	//0xC5, 0xF5, /* store */
};


typedef struct snSPC {
	u8*		PC;
	u16		DP; /* This is not a register, this is just for
	tell the SPC-700 the base offset, which can be 0x100 or simply 0 */

	u8		A;
	u8		X;
	u8		Y;
	u16*		YA; /* A << 8 | Y*/
	u16		SP; /* Stack Pointer */
	u8		PSW; /* Same as CPU flags or PSW if you prefer */

	spcBUS*		bus; /* yes, an independent bus

				i love you SPC-700   */

	u8		test; /* he's just here */
	u8		control;
	u8		DSPAddr;
	u8		DSPData;

	u8		T0target;
	u8		T1target;
	u8		T2target;

	u8		T0out;
	u8		T1out;
	u8		T2out;


	void		(*fetch)(emGeneral* emulator);
	void		(*opcode[0xFF])(emGeneral* emulator);
} snSPC;

typedef struct snDSL {
	/* this is really responsible for SNES AUDIO */
	u32		channel1;
	u32		channel2;
	u32		channel3;
	u32		channel4;
	u32		channel5;
	u32		channel6;
	u32		channel7;
	u32		channel8;
	
} snDSL;

typedef struct snAPU {
	u8	    located;
	u8	    internalRAM[0xFFFF];
	snSPC*		spc;
} snAPU;

#endif
