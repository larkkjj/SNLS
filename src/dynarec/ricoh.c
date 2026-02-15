#include "general/types.h"

/* Warning: Nothing here is implemented, this is
placeholder until SNLS have proper PPU and SPC
components */

typedef struct byteChunk {
  u8* pointer;
  u8  length;
} byteChunk;

typedef struct eqCode {
  char*  section;  
} eqCode;

typedef struct eeDynarec {
  char*   (*returnEquivalent)(byteChunk chunk);
  void*   (*analyzeChunk)(byteChunk chunk);
} eeDynarec;

static void __UNNAMED__OPTIMIZECODE__();

static char* returnEquivalent(byteChunk chunk) {
  eqCode code;  
  return code.section;
};

static void analyzeChunk(byteChunk chunk);
