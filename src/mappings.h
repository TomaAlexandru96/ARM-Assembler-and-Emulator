#define NDEBUG
#include "adts.h"

// -------------------FUNCTION DECLARATIONS-----------------------
/**
* fillInstruction will fill a map with all the Data Processing instructions
* in the asembler and map them to their respective opcode (bits 24, 23, 22, 21)
**/
map fillDataToOpcode(void);
/**
* Will return a map with all instructions mapped to their respective type:
* 0 Data Processing
* 1 Multiply
* 2 Single Data Transfer
* 3 Branch
* 4 Special lsl
* 5 Special andeq
**/
map fillAllInstructions(void);
/**
* Returns a mapping of conditions names with thier specific code
**/
map fillConditions(void);
map fillDataToType(void);
/**
* Fills all mappings
**/
void fillAll(void);
void freeAll(void);
// --------------------------ENUMS-------------------------------
typedef enum {INSTRUCTION, LABEL, EXPRESSION_TAG,
              EXPRESSION_EQUAL, REGISTER, UNDEFINED} typeEnum;

// --------------------GLOBAL VARIABLES--------------------------
map DATA_OPCODE;
map ALL_INSTRUCTIONS;
map CONDITIONS;
map DATA_TYPE;
int countDynamicExpansions = 1;

// -------------------FUNCTION DEFINITIONS-----------------------
map fillDataToOpcode(void) {
  map m = constructMap();

  put(&m, "add", 4);
  put(&m, "sub", 2);
  put(&m, "rsb", 3);
  put(&m, "and", 0);
  put(&m, "eor", 1);
  put(&m, "orr", 12);
  put(&m, "mov", 13);
  put(&m, "tst", 8);
  put(&m, "teq", 9);
  put(&m, "cmp", 10);

  return m;
}

map fillAllInstructions(void) {
  map m = constructMap();

  // 0 Data Processing
  put(&m, "add", 0);
  put(&m, "sub", 0);
  put(&m, "rsb", 0);
  put(&m, "and", 0);
  put(&m, "eor", 0);
  put(&m, "orr", 0);
  put(&m, "mov", 0);
  put(&m, "tst", 0);
  put(&m, "teq", 0);
  put(&m, "cmp", 0);

  // 1 Multiply
  put(&m, "mul", 1);
  put(&m, "mla", 1);

  // 2 Single Data Transfer
  put(&m, "ldr", 2);
  put(&m, "str", 2);

  // 3 Branch
  put(&m, "beq", 3);
  put(&m, "bne", 3);
  put(&m, "bge", 3);
  put(&m, "blt", 3);
  put(&m, "bgt", 3);
  put(&m, "ble", 3);
  put(&m, "b", 3);

  // 4 Special
  put(&m, "lsl", 4);

  // 5 Special
  put(&m, "andeq", 5);

  return m;
}

map fillConditions(void) {
  map m = constructMap();

  put(&m, "eq", 0);
  put(&m, "ne", 1);
  put(&m, "ge", 10);
  put(&m, "lt", 11);
  put(&m, "gt", 12);
  put(&m, "le", 13);
  put(&m, "", 14);

  return m;
}

map fillDataToType(void) {
  map m = constructMap();

  // instructions that compute results and, eor, sub, rsb, add, orr
  put(&m, "and", 0);
  put(&m, "eor", 0);
  put(&m, "sub", 0);
  put(&m, "rsb", 0);
  put(&m, "add", 0);
  put(&m, "orr", 0);

  // single operand instruction mov
  put(&m, "mov", 1);

  // Instructions that do not compute results,
  // but do set the CPSR flags: tst, teq, cmp
  put(&m, "tst", 2);
  put(&m, "teq", 2);
  put(&m, "cmp", 2);

  return m;
}

void fillAll(void) {
  DATA_OPCODE      = fillDataToOpcode();
  ALL_INSTRUCTIONS = fillAllInstructions();
  CONDITIONS       = fillConditions();
  DATA_TYPE        = fillDataToType();
}

void freeAll(void) {
  clearMap(&DATA_OPCODE);
  clearMap(&ALL_INSTRUCTIONS);
  clearMap(&CONDITIONS);
  clearMap(&DATA_TYPE);
}
