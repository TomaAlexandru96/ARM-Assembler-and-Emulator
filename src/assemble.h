#include "mappings.h"

// ---------------------------MACROS-----------------------------
#define MAX_LINE_LENGTH 512
#define DELIMITERS " ,\n"
#define MEMORY_SIZE 4
#define PC_OFFSET 2
#define INSTRUCTION_SIZE 32
#define ALWAYS_CONDITION ""
#define BRANCH_OFFSET_SIZE  26
#define NUMBER_OF_LINES 1000

// --------------------GLOBAL VARIABLES--------------------------
int countDynamicExpansions = 1;

// -------------------FUNCTION DECLARATIONS-----------------------
// -----------------------FILE PASSES-----------------------------
/**
* Returns a char array that contains all the program lines
* Maps all labels with their respective memory location
* Finds the number of the instructions and returns it through instructionsNumber
* Finds the number of ldr instructions and returns it through ldrCount
* Finds the number of lines and returns it through lineNumber
* Throws any errors occour during the first pass such as multiple definitions
* of the same label
**/
char **firstPass(FILE *input, map *labelMapping, vector *errorVector,
        uint32_t *instructionsNumber, uint32_t *ldrCount, uint32_t *lineNumber);

/**
* Fills the instrcutions array with all the decode instrcutions
* Throws any errors encountered during the pass
**/
void secondPass(uint32_t *instructionsNumber, uint32_t instructions[],
              vector *errorVector, map labelMapping,
              char** linesFromFile, uint32_t lineNumber);
// --------------------DECODING FUNCTIONS-------------------------
/* Main decode function which returns the decoded instruction */
uint32_t decode(vector *tokens, vector *addresses, uint32_t instructionNumber,
  uint32_t instructionsNumber, map labelMapping, vector *errorVector, char *ln);

/* Decodes any Data Processing Instruction */
uint32_t decodeDataProcessing(vector *tokens, vector *errorVector, char *ln);

/* Decodes any Multiply Instruction */
uint32_t decodeMultiply(vector *tokens, vector *errorVector, char *ln);

/* Decodes any Single Data Transfer Instruction */
uint32_t decodeSingleDataTransfer(vector *tokens, vector *addresses,
                uint32_t instructionNumber, uint32_t instructionsNumber,
                vector *errorVector, char *ln);

/* Decodes any Branch Instruction */
uint32_t decodeBranch(vector *tokens, uint32_t instructionNumber,
                map labelMapping, vector *errorVector, char *ln);

/* Decodes any Shift Instruction */
uint32_t decodeShift(vector *tokens, vector *errorVector, char *ln);

/**
* Creates a vector with all of the tokens of the original string
* without modifing it with respect to the delimiters (eg. " ,.")
**/
vector tokenise(char *start, char *delimiters);

// ---------------------TYPE FUNCTIONS-------------------------
/* Returns the type of the passed token */
typeEnum getType(char *token);

/* Helpers for type */
bool isInstruction(char *token);
bool isLabel(char *token);
bool isRegister(char *token);
bool isShift(char *token);
typeEnum isExpression(char *token);

// ----------------------GET FUNCTIONS-------------------------
/* Gets the shift type and applays the shift rules to the operand parameter */
void getShift(vector *tokens, uint32_t *operand, vector *errorVector, char *ln);

/**
* Gets expressions of the type [register], [register, register],
* [register, register, shift] (everything that is related to memory access)
**/
void getBracketExpr(vector *tokens, int *rn, int32_t *offset, int *i, int *u,
                    vector *errorVector, char *ln);
/**
* Gets expression of type <#expression> or <=expression> and throws errors
* if the value in the exp can't be represented
**/
int32_t getExpression(char *exp, vector *errorVector, char *ln);

/* Gets hexadecimal value from string */
int32_t getHex(char *exp);

/* Gets decimal value from string */
int32_t getDec(char *exp);

/* Clears the vector tokens if the token '@' is found (i.e. a comment) */
void getComment(vector *tokens);

// ------------------------HELPERS-------------------------------
/* Converts unsigned int to string */
char *uintToString(uint32_t num);

/* Checks format of registers and throws error if the format is invalid */
bool checkReg(vector *tokens, char *instr,
                vector *errorVector, char *ln);

/* Sets the cond field of the instruction */
void setCond(uint32_t *x, char *cond);

/* Frees the matrix of lines */
void clearLinesFromFile(char **linesFromFile);

// ----------------------ERRORS--------------------------------
/**
* All of the error functions append an error message to the errorVector
* which will be printed at the end of the program if there are any errors
**/
void throwUndefinedError(char *name, vector *errorVector, char *ln);
void throwLabelError(char *name, vector *errorVector, char *ln);
void throwExpressionError(char *expression, vector *errorVector, char *ln);
void throwRegisterError(char *name, vector *errorVector, char *ln);
void throwExpressionMissingError(char *ins, vector *errorVector, char *ln);

// -----------------------DEBUGGING---------------------------
void printStringArray(int n, char arr[][MAX_LINE_LENGTH]);
void printBinary(uint32_t nr);
