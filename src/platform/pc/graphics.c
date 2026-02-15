#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define RAYGUI_IMPLEMENTATION
#include "raylib/raylib.h"
#include "raylib/raygui.h"

#include "emulator/main.h"
#include "core/bus.h"

static char SNES_X[0x04];
static char SNES_A[0x04];
static char SNES_Y[0x04];
static char CPU_EFlag[1] = {};
static char CPU_MFlag[1] = {};
static char CPU_PC[0x4F]; 
static char RAM_LOW[1] = {};
static char BUS_ADDRESS[0xF];
static char BUS_BANK[0xF];
static char BUS_VALUE[0xF]; 

static char APU_A[0xF] = {};
static char APU_X[0xF] = {};
static char APU_Y[0xF] = {}; 
static char APU_SP[0xF] = {}; 
static char APU_PC[0xF] = {}; 

bool WindowBox000Active = true;
bool ValueBOx002EditMode = false;
char ValueBOx002Value[0x4F];

static void updateInfo(emGeneral* emulator) {

};

extern void initWindow() {
	InitWindow(640, 480, "SNLS");
	SetTargetFPS(60);

	return;
}
extern void pollWindow(emGeneral* emulator) {
	return;
};


