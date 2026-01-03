/* This is to prevent usage on host (x86-64) */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "emulator/gs.h"
#include "general/types.h"

extern void initWindow() {
	return;
}
extern void pollWindow() {
	return;
};

