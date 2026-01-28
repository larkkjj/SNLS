#ifndef SNLS_CPU_HEADER
#define SNLS_CPU_HEADER

#include <stdbool.h>
#include "general/types.h"
#include "general/references.h"

/*ですただ */
typedef struct snCPU {
	u8*		PC;
	u8*		subPC;	/* Subroutine */

	u16		SP;
	u16		A;
	u16		X;
	u16		Y;
	u16		DP;
	u8		DBR;
	u8		PB;
	u16		P;

	u8		flags;
	void		(*fetch)(emGeneral* emulator);

	/* in the future, i want
	 * to implement an idea
	 * that i had in the project
	 * s starting phase,
	 * a single u8 value
	 * acting all the sn_Flags
	 * like the OG hardware */
	bool 		sn_NFlag;
	bool 		sn_VFlag;
	bool 		sn_MFlag;
	bool 		sn_XFlag;
	bool 		sn_DFlag;
	bool 		sn_IFlag;
	bool 		sn_ZFlag;
	bool 		sn_CFlag;
	bool 		sn_EFlag;
	bool 		sn_BFlag;

} snCPU;

typedef enum {
	_adc_dp_x_indr	= 0x61,
	_adc_sr_s 	= 0x63,
	_adc_dp		= 0x65,
	_adc_dp_indr_l	= 0x67,
	_adc_const	= 0x69,
	_adc_addr 	= 0x6D,
	_adc_l	 	= 0x6F,
	_adc_dp_indr_y 	= 0x71,
	_adc_dp_indr 	= 0x72,
	_adc_sr_s_indr_y= 0x73,
	_adc_dp_x	= 0x75,
	_adc_dp_y 	= 0x77,
	_adc_addr_y 	= 0x79,
	_adc_addr_x 	= 0x7D,
	_adc_long_x 	= 0x7F,

	// AND INS
	_and_dp_x_indr	= 0x21,
	_and_sr_s 	= 0x23,
	_and_dp		= 0x25,
	_and_dp_indr_l	= 0x27,
	_and_const	= 0x29,
	_and_addr 	= 0x2D,
	_and_l	 	= 0x2F,

	_and_dp_indr_y 	= 0x31,
	_and_dp_indr 	= 0x32,
	_and_sr_s_indr_y= 0x33,
	_and_dp_x	= 0x35,
	_and_dp_y_ind 	= 0x37,
	_and_addr_y 	= 0x39,
	_and_addr_x 	= 0x3D,
	_and_long_x 	= 0x3F,

	// ASL INS
	_asl_dp		= 0x06,
	_asl_a		= 0x0A,
	_asl_addr	= 0x0E,
	_asl_dp_x	= 0x16,
	_asl_addr_x	= 0x1E,

	// BCC BLT BSS BGE INS
	_bcc		= 0x90,
	_bcs		= 0xB0,
	_blt		= _bcc, //alias to BCC
	_bss		= 0xB0,
	_bge		= _bcs, //alias to BCS

	// BEQ INS
	_beq		= 0xF0,

	// BIT INS
	_bit_dp		= 0x24,
	_bit_addr	= 0x2C,
	_bit_dp_x	= 0x34,
	_bit_addr_x	= 0x3C,
	_bit_const	= 0x89,

	// BMI BNE BMP BRA BRK INS
	_bmi		= 0x30,
	_bne		= 0xD0,
	_bpl		= 0x10,
	_bra		= 0x80,
	_brl		= 0x82,
	_brk		= 0x00, //break

	// BVC BVS CLC CLD CLI CLV INS
	_bvc		= 0x50,
	_bvs		= 0x70,
	_clc		= 0x18,
	_cld		= 0xD8,
	_cli		= 0x58,
	_clv		= 0xB8,

	// CMP INS
	_cmp_dp_x_indr	= 0xC1,
	_cmp_sr_s_indr_y= 0xC3,
	_cmp_dp		= 0xC5,
	_cmp_dp_indr_l	= 0xC7,
	_cmp_const	= 0xC9,
	_cmp_addr 	= 0xCD,
	_cmp_l	 	= 0xCF,
	_cmp_dp_indr_y	= 0xD1,
	_cmp_dp_indr 	= 0xD2,
	_cmp_sr_s_y	= 0xD3,
	_cmp_dp_x	= 0xD5,
	_cmp_dp_indr_l_y= 0xD7,
	_cmp_addr_y	= 0xD9,
	_cmp_addr_x	= 0xDD,
	_cmp_l_x	= 0xDF,

	_cpx_const	= 0xE0,
	_cpx_dp		= 0xE4,
	_cpx_addr	= 0xEC,

	_cpy_const	= 0xC0,
	_cpy_dp		= 0xC4,
	_cpy_addr	= 0xCC,

	// COP INS
	_cop		= 0x02,

	_dea		= 0x3A,
	_dec_dp		= 0xC6,
	_dec_addr	= 0xCE,
	_dec_dp_x	= 0xD6,
	_dec_addr_x	= 0xDE,

	_dex		= 0xCA, //basically a dec(x) equivalent
	_dey		= 0x88, //basically a dec(y) equivalent

	// EOR INS
	_eor_dp_x_indr	= 0x41,
	_eor_sr_s	= 0x43,
	_eor_dp		= 0x45,
	_eor_dp_x	= 0x55,
	_eor_dp_indr_y	= 0x51,
	_eor_dp_indr	= 0x52,
	_eor_sr_s_indr_y= 0x53,
	_eor_addr	= 0x4D,
	_eor_const	= 0x49,
	_eor_addr_y	= 0x59,
	_eor_addr_x	= 0x5D,

	// INC INS
	_inc_a		= 0x1A, // Increment accumulator by 1
	_inc_dp		= 0xE6, //basically a add equivalent
	_inc_addr	= 0xEE, //basically a add equivalent
	_inc_dp_x	= 0xF6, //basically a add equivalent
	_inc_addr_x	= 0xFE, //basically a add equivalent
	_inx		= 0xE8, // Increment X by 1
	_iny		= 0xC8, // Increment Y by 1

	// JMP INS
	_jmp_addr	= 0x4C,
	_jmp_l		= 0x5C,
	_jmp_addr_indr	= 0x6C,
	_jmp_addr_x_indr= 0x7C,
	_jmp_addr_indr_l= 0xDC,

	_jsr_addr	= 0x20,
	_jsr_l		= 0x22,
	_jsr_addr_x_indr= 0xFC,

	_lda_addr	= 0xAD,
	_lda_const	= 0xA9,
	_lda_addr_x	= 0xB9,
	_lda_addr_y	= 0xBD,
	_lda_dp_indr_l	= 0xA5,
	_lda_dp_indr_l_y= 0xB7,
	_lda_dp		= 0xA7,
	_lda_l		= 0xAF,
	_lda_l_x	= 0xBF,
	_ldx_const	= 0xA2,
	_lda_dp_x	= 0xA1,
	_lda_dp_indr_y	= 0xB1,
	_ldx_addr	= 0xAE,
	_ldx_addr_y	= 0xBE,
	_ldx_dp		= 0xA6,
	_ldx_dp_y	= 0xB6,
	_ldy_addr	= 0xAC,
	_ldy_addr_x	= 0xBC,
	_ldy_dp		= 0xA4,
	_ldy_dp_x	= 0xB4,
	_ldy_const	= 0xA0,

	_lsr_dp		= 0x46,
	_lsr_a		= 0x4A,
	_lsr_addr	= 0x4E,
	_lsr_dp_x	= 0x56,
	_lsr_addr_x	= 0x5E,

	_mvn		= 0x54,
	_mvp		= 0x44,

	// a sleep...?
	_nop		= 0xEA,

	_ora_dp		= 0x05,
	_ora_dp_indr_l	= 0x07,
	_ora_const	= 0x09,
	_ora_addr	= 0x0D,
	_ora_l		= 0x0F,
	_ora_dp_x	= 0x15,
	_ora_dp_x_indr	= 0x01,
	_ora_dp_indr_y	= 0x11,
	_ora_dp_indr	= 0x12,
	_ora_sr_s	= 0x03,
	_ora_addr_y	= 0x19,
	_ora_addr_x	= 0x1D,
	_ora_l_x	= 0x1F,

	_rti		= 0x40,
	_rtl		= 0x6B,
	_rts		= 0x60,
	// REP
	_rep		= 0xC2, //reset flag

	// PEA PEI PER PHA PHD PHK PHX PHY PLA PLD PLP PLX PLY INS
	_pea		= 0xF4,
	_pei		= 0xD4,
	_per		= 0x62,
	_pha		= 0x48,
	_phb		= 0x8B,
	_phd		= 0x0B,
	_phk		= 0x4B,
	_phx		= 0xDA,
	_php		= 0x08,
	_phy		= 0x5A,
	_pla		= 0x68,
	_plb		= 0xAB,
	_pld		= 0x2B,
	_plp		= 0x28,
	_plx		= 0xFA,
	_ply		= 0x7A,

	_rol_dp		= 0x26,
	_rol_a		= 0x2A,
	_rol_addr	= 0x2E,
	_rol_dp_x	= 0x36,
	_rol_addr_x 	= 0x3E,

	_ror_dp		= 0x66,
	_ror_a		= 0x6A,
	_ror_addr	= 0x6E,
	_ror_dp_x	= 0x76,
	_ror_addr_x 	= 0x7E,

	_sec		= 0x38,
	_sed		= 0xF8,
	_sei		= 0x78,
	_sep		= 0xE2,

	_sta_dp_x_indr	= 0x81,
	_sta_sr_s	= 0x83,
	_sta_dp		= 0x85,
	_sta_dp_indr_l 	= 0x87,
	_sta_addr 	= 0x8D,
	_sta_l		= 0x8F,
	_sta_dp_indr_y	= 0x91,
	_sta_dp_indr	= 0x92,
	_sta_sr_s_y	= 0x93,
	_sta_dp_x	= 0x95,
	_sta_dp_indr_l_y= 0x97,
	_sta_addr_y	= 0x99,
	_sta_addr_x	= 0x9D,
	_sta_l_x	= 0x9F,

	_sbc_dp_x_indr	= 0xE1,
	_sbc_sr_s	= 0xE3,
	_sbc_dp		= 0xE5,
	_sbc_dp_indr_l	= 0xE7,
	_sbc_const	= 0xE9,
	_sbc_addr	= 0xED,
	_sbc_l		= 0xEF,
	_sbc_dp_indr_y	= 0xF1,
	_sbc_dp_indr	= 0xF2,
	_sbc_sr_s_indr_y= 0xF3,
	_sbc_dp_x	= 0xF5,
	_sbc_dp_indr_l_y= 0xF7,
	_sbc_addr_y	= 0xF9,
	_sbc_addr_x	= 0xFD,
	_sbc_l_x	= 0xFF,

	_stx_addr	= 0x8E,
	_stx_dp		= 0x86,
	_stx_dp_y	= 0x96,

	_sty_dp		= 0x84,
	_sty_addr	= 0x8C,
	_sty_dp_x	= 0x94,

	_stz_dp		= 0x64,
	_stz_dp_x	= 0x74,
	_stz_addr	= 0x9C,
	_stz_addr_x	= 0x9E,

	_stp		= 0xDB,

	_tax		= 0xAA,
	_tay		= 0xA8,
	_tcd		= 0x5B,
	_tcs		= 0x1B,
	_tdc		= 0x7B,
	_tsc		= 0x3B,
	_tsx		= 0xBA,
	_txa		= 0x8A,
	_txs		= 0x9A,
	_txy		= 0x9B,
	_tya		= 0x98,
	_tyx		= 0xBB,

	_trb_dp		= 0x14,
	_trb_addr	= 0x1C,
	_tsb_dp		= 0x04,
	_tsb_addr	= 0x0C,

	/* An entire reserved space for you */
	_wdm		= 0x42,
	/* Hate you little N-brother :) */
	_wai		= 0xCB,
	_xba		= 0xEB,
	_xce		= 0xFB,
} cpu_instructions;


#endif
