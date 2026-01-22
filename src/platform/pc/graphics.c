#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define RAYGUI_IMPLEMENTATION
#include "raylib/raylib.h"
#include "raylib/raygui.h"

#include "emulator/main.h"
#include "core/bus.h"

char SNES_X[0x04];
char SNES_A[0x04];
char SNES_Y[0x04];
char CPU_EFlag[1] = {};
char CPU_MFlag[1] = {};
char CPU_PC[0x4F]; 
char RAM_LOW[1] = {};
char BUS_ADDRESS[0xF];
char BUS_BANK[0xF];
char BUS_VALUE[0xF]; 

static char APU_A[0xF] = {};
static char APU_X[0xF] = {};
static char APU_Y[0xF] = {}; 
static char APU_SP[0xF] = {}; 
static char APU_PC[0xF] = {}; 

bool WindowBox000Active = true;
bool ValueBOx002EditMode = false;
char ValueBOx002Value[0x4F];

static void updateInfo(emGeneral* emulator) {
	sprintf(BUS_VALUE, "%X\0", emulator->bus->value);
	sprintf(BUS_BANK, "%X\0", emulator->bus->bank);
	sprintf(CPU_EFlag, "%X\0", emulator->apu->internalRAM[0xF4]);
	sprintf(CPU_MFlag, "%X\0", emulator->apu->internalRAM[0xF5]);
	
	sprintf(SNES_X, "%X\0", emulator->cpu->X);
	sprintf(SNES_A, "%X\0", emulator->cpu->A);
	sprintf(SNES_Y, "%X\0", emulator->cpu->Y);

	sprintf(APU_A, "%X", emulator->apu->spc->A);
	sprintf(APU_X, "%X", emulator->apu->spc->X);
	sprintf(APU_Y, "%X", emulator->apu->spc->Y);
	sprintf(APU_SP, "%X", emulator->apu->spc->SP);
	sprintf(APU_PC, "%X %X", emulator->apu->spc->PC, *emulator->apu->spc->PC);

	sprintf(ValueBOx002Value, "%X %X\0", emulator->cpu->PC, *emulator->cpu->PC);
	sprintf(BUS_ADDRESS, "%X\0", emulator->bus->address);
};

extern void initWindow() {
	InitWindow(640, 480, "SNLS");
	SetTargetFPS(60);

	return;
}
extern void pollWindow(emGeneral* emulator) {

	while (!WindowShouldClose()) {
		BeginDrawing();
		            ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));
		if (WindowBox000Active) {
			WindowBox000Active = !GuiWindowBox((Rectangle){ 88, 56, 160, 80 }, "SAMPLE TEXT");
		}
		if (GuiTextBox((Rectangle){ 104, 136, 120, 24 }, ValueBOx002Value, 0x4F, ValueBOx002EditMode)) ValueBOx002EditMode = !ValueBOx002EditMode;
		(GuiTextBox((Rectangle){500, 100, 30, 20}, BUS_ADDRESS, 2, 0x00));
		(GuiTextBox((Rectangle){500, 120, 30, 20}, BUS_BANK, 4, 0x00));
		(GuiTextBox((Rectangle){500, 140, 30, 20}, BUS_VALUE, 2, 0x00));
		(GuiTextBox((Rectangle){500, 160, 30, 20}, CPU_EFlag, 2, 0x00));
		(GuiTextBox((Rectangle){500, 180, 30, 20}, CPU_MFlag, 2, 0x00));

		(GuiTextBox((Rectangle){500, 200, 30, 20}, SNES_A, 2, 0x00));
		(GuiTextBox((Rectangle){500, 220, 30, 20}, SNES_X, 4, 0x00));
		(GuiTextBox((Rectangle){500, 240, 30, 20}, SNES_Y, 2, 0x00));
		

		(GuiTextBox((Rectangle){500, 200, 30, 20}, SNES_A, 2, 0x00));
		(GuiTextBox((Rectangle){500, 220, 30, 20}, SNES_X, 4, 0x00));
		(GuiTextBox((Rectangle){500, 240, 30, 20}, SNES_Y, 2, 0x00));
		
		(GuiTextBox((Rectangle){550, 100, 30, 20}, APU_A, 2, 0x00));
		(GuiTextBox((Rectangle){550, 120, 30, 20}, APU_X, 2, 0x00));
		(GuiTextBox((Rectangle){550, 140, 30, 20}, APU_Y, 2, 0x00));
		(GuiTextBox((Rectangle){550, 160, 30, 20}, APU_SP, 2, 0x00));
		(GuiTextBox((Rectangle){550, 180, 120, 20}, APU_PC, 4, 0x00));
		
		if (GuiButton((Rectangle){500, 50, 130, 20}, "Fetch System")) {
			updateInfo(emulator);
				emulator->allowedFetch = 1;
		}

			if (IsKeyDown(KEY_ENTER)) {
				updateInfo(emulator);
				emulator->allowedFetch = 1;
			}

		EndDrawing();
		break;
	}
	return;
};


