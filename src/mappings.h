#ifndef MAPPINGS_H
#define MAPPINGS_H

#include "adts.h"

// -------------------FUNCTION DECLARATIONS-----------------------
/**
* Returns a map with all the Data Processing instructions from the assembler
* and maps them to their respective opcode (bits 24, 23, 22, 21)
**/
map fillDataToOpcode(void);

/**
* Returns a map with all instructions mapped to their respective type:
* 0 Data Processing
* 1 Multiply
* 2 Single Data Transfer
* 3 Branch
* 4 Shifts
* 5 Special andeq r0, r0, r0
**/
map fillAllInstructions(void);

/**
* Returns a map containing all conditions from the cond field of the
* instruction with their respective code
**/
map fillConditions(void);

/**
* Returns a map that classifies all Data Processing instructions:
* 0 Instructions that compute results
* 1 Mov instruction
* 2 Instructions that set flags
**/
map fillDataToType(void);

/* Returns a map with all 4 shifts mapped to their respective code */
map fillShifts(void);

/* Fills all mappings */
void fillAll(void);

/* Frees all mappings */
void freeAll(void);
// --------------------------ENUMS-------------------------------
typedef enum {INSTRUCTION, LABEL, EXPRESSION_TAG,
              EXPRESSION_EQUAL, REGISTER, UNDEFINED} typeEnum;

// --------------------GLOBAL VARIABLES--------------------------
map DATA_OPCODE;
map ALL_INSTRUCTIONS;
map CONDITIONS;
map DATA_TYPE;
map SHIFTS;

#endif
