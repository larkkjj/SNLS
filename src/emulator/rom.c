#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "general/types.h"
#include "emulator/gs.h"

#include "emulator/rom.h"
#include "core/cpu/ricoh.h"
#include "emulator/memory.h"
#include "general/tools.h"

#ifdef _EE
#include <debug.h>
#endif

FILE* rom_File = NULL;

/* Remember to replace variable calls with struct members */
int splitROM(rom* rom_Ptr) {
	/* Warning, this code is not implemented at all
	 * since it only get up to 16-bit addresses */

	fseek(rom_File, rom_Ptr->offset, SEEK_SET);
	/* This will probally not be used in the
	 * final product */

	/* This was used to distinguishe 
	 * loROM and hiROM, making loROM 
	 * buffer half the size 
	if (rom_Ptr->type & 0x01) {
		rom_Ptr->banksize = 0x8000;
	} else if (rom_Ptr->type & 0x00) {
		rom_Ptr->banksize = 0x10000;
	} */

	switch (rom_Ptr->type) {
		case 0x20:
			rom_Ptr->header = 0x7FFC + rom_Ptr->offset;
			rom_Ptr->banksize = 0x8000;
			printf("LoROM \n");
		break;
		case 0x21:
			rom_Ptr->header = 0xFFFC + rom_Ptr->offset;
			printf("HiROM \n");
		break;
		case 0x23:
			rom_Ptr->header = 0x7FFC + rom_Ptr->offset;
			printf("SA-1\n");
		break;
		case 0x30:
			rom_Ptr->header = 0x7FFC + rom_Ptr->offset;
			printf("LoROM + FastROM)\n");
		break;
		case 0x31:
			rom_Ptr->header = 0xFFFC + rom_Ptr->offset;
			//rom_Ptr->resetV += 0x8000;
			printf("HiROM \n");
		break;
		default:
			printf("Unknown ROM type %X", rom_Ptr->type);
			exit(0);
	}
	fseek(rom_File, rom_Ptr->header - 0x3C, SEEK_SET);
	fread(rom_Ptr->headername, 21, sizeof(char), rom_File);


	fseek(rom_File, rom_Ptr->header, SEEK_SET);
	fread(&rom_Ptr->resetV, sizeof(u16), 1, rom_File);
	fseek(rom_File, rom_Ptr->offset, SEEK_SET);
//	fseek(rom_File, 0, SEEK_SET);
	/* remanescent code */
	//rom_Ptr->MapArea = 0x8000;
	return 1;
}

int findROMMap(rom* rom_Ptr) {
	for(int i = 0; i < sizeof(address_romTypes) / sizeof(address_romTypes[0]); i ++) {
		fseek(rom_File, rom_Ptr->offset, SEEK_SET);
		fseek(rom_File, address_romTypes[i], SEEK_CUR);
		fread(&rom_Ptr->type, sizeof(u8), 1, rom_File);
		for(int j = 0; j < sizeof(valid_romTypes) / sizeof(valid_romTypes[0]); j ++) {
			if (memcmp(&rom_Ptr->type, &valid_romTypes[j], sizeof(u8)) == 0) {
				printf("Found: %02X\n", rom_Ptr->type);
				return 1;
			}
		}
		continue;
	};
	return 0;
}

void openRom(char* rom_name, rom* rom_Ptr) {
	rom_File = fopen(rom_name, "r");
	if (rom_File == NULL) {
		#ifdef _EE
		//Using a generic function here to overdraw
		scr_printf("open_rom: error, invalid rom, did you set it correctly \n");
		#endif
		printf("open_rom: error, invalid rom, did you set it correctly? \n");
		exit(1);
	} else {
		/* Tell rom size*/
		printf("Sucess opening the rom \n");
		fseek(rom_File, 0, SEEK_END);
		/*if (ftell(rom_File) % 1024 == 512) {
			rom_Ptr->offset = 0x200;
		}*/
		rom_Ptr->offset = (ftell(rom_File) % 1024);
		rom_Ptr->size = ftell(rom_File) - rom_Ptr->offset;
		rom_Ptr->banks = returnBank(rom_Ptr->size);

		printf("ROM size: %zu Offset: %zu \n", rom_Ptr->size, rom_Ptr->offset);

		if (rom_Ptr->size - rom_Ptr->offset >= 4194304) {
			/* Wrong address btw */
			//address_romTypes[3] = 0x40FFC0;
		}

		printf("Checking rom type..\n");
		if (findROMMap(rom_Ptr)) {
			//memory_vMap = malloc(0x7FFF + BytePerBank(rom_Ptr->size));
			//memory_vMap = malloc(0x7FFF + rom_Ptr->size);
			splitROM(rom_Ptr);
		}
		printf("%s \n", rom_Ptr->headername);
	};
	return;
};
