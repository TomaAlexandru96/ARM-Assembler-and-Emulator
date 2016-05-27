#include "headers.h"
#include <limits.h>
#include <stdbool.h>
#include <assert.h>

#define MEM_SIZE_BYTES 16384 // 2 ^ 14 instruction addresses
#define NUMBER_REGS 17
#define INDEX_PC 15
#define INDEX_CPSR 16
#define INDEX_SP 13
#define INDEX_LR 14

/*-------------TypeDefinitions------------------*/
typedef struct proc_state proc_state_t;

typedef struct pipeline pipeline_t;

/*-------------Defining processor state---------*/
struct proc_state {
  int NEG;
  int ZER;
  int CRY;
  int OVF;
  int PC;
  int regs[NUMBER_REGS];
  int memory[MEM_SIZE_BYTES];
};

struct pipeline {
  int fetched;
  int decoded;
};

/*------------------Prototypes-------------------*/
bool shouldExecute(int instruction, proc_state_t *pState);

int executeShift(int contentsRm, int shiftValueInteger, proc_state_t *pState,
                 int shiftType, int highBitRm, int Rm, bool setFlags);

void executeSDataTransfer(int instruction, proc_state_t *pState);

void executeDataProcessing(int instruction, proc_state_t *pState);

void executeMultiply(int instruction, proc_state_t *pState);

void executeBranch(int instruction, proc_state_t *pState, pipeline_t *pipeline);

void memoryLoader(FILE *file, proc_state_t *pState);

void printProcessorState(proc_state_t *pState);

void decodeFetched(int instruction, proc_state_t *pState, pipeline_t *pipeline);

void procCycle(proc_state_t *pState);

void executeOperation(proc_state_t *pState, int Rdest,
                     int Rn, int operand2, int auxResultArithmeticOps,
                     int carry, int S, bool resultAllZeros, int opcode);

int getEffectiveAddress(int Rn, int offset, int U, proc_state_t *pState);

int getMemoryContentsAtAddress(proc_state_t *pState, int address);

void writeToMemory(int word, int startByteAddress, proc_state_t *pState);

void printMemory(int memory[]);

int convertToLittleEndian(int instruction);
