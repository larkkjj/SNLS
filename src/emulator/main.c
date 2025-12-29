#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "general/functions.h"

#ifdef for_ps2
#include "debug.h"
#include "ps2_filesystem_driver.h"
#endif

char path_buf[255];

int main(int argc, char* argv[]) {
	for(unsigned int i = 0; i < argc; i ++) {
		if (strcmp(argv[i], "--ipl") == 0) {
			if (argv[i] == NULL) {
				printf("WARNING: empty IPL argument \n");
			}
			printf("WARNING: --ipl is not implemented yet.\nparameter [%s] will be ignored\n", argv[++i]);
			sleep(1);
		}
	}
	rom rom_Ptr;
	//initWindow();
	/* Just a check to make sure we're on the correct path */
	getcwd(path_buf, sizeof(path_buf));

	/* Yea, i know this is bad
	 * but this is used until
	 * i make a file explorer */
	//openRom("roms/cputest/cputest-full.sfc", &rom_Ptr);
	openRom("roms/mariow.sfc", &rom_Ptr);
	
	initEmu(&rom_Ptr);

	/* Yea i know this is bad
	 * but i was kidna lazy to implement
		* a better fix like a double pointer
		* or even some more simple
	 */
		//free(m_Buf->mMap);
		//free(m_Buf->mROM);

	return 0;
}
