#include "headers.h"
#include <limits.h>
#include <stdbool.h>
#include <assert.h>

#define MEM_SIZE_WORDS 16384 // 2 ^ 14 word addresses
#define NUMBER_REGS 17
#define INDEX_PC 15
#define INDEX_CPSR 16
#define INDEX_SP 13
#define INDEX_LR 14
#define GPIO20_29_ADDRESS 0x20200008
#define GPIO10_19_ADDRESS 0x20200004
#define GPIOo_9_ADDRESS 0x20200000
#define GPIO_OUTPUT_OFF 0x20200028
#define GPIO_OUTPUT_ON 0x2020001C

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
  int memory[MEM_SIZE_WORDS];
};

struct pipeline {
  int fetched;
  int decoded;
};

/*------------------Prototypes-------------------*/
bool shouldExecute(int instruction, proc_state_t *pState);
/*returns true iff the instruction should be executed*/

int executeShift(int contentsRm, int shiftValueInteger, proc_state_t *pState,
                 int shiftType, int highBitRm, int Rm, bool setFlags);
/*returns shifted value of the contents of Rm*/

void executeSDataTransfer(int instruction, proc_state_t *pState);
/*extracts instruction bits from a data transfer instruction and performs the
  load/store according to flag bits*/

void executeDataProcessing(int instruction, proc_state_t *pState);
/*extracts flag bits and performs operation indicated by opcode field. Uses
  helper functions to compute the value of opperand2*/

void executeMultiply(int instruction, proc_state_t *pState);
/*performs multiplication with/without accumulate, as indicated by A bit*/

void executeBranch(int instruction, proc_state_t *pState, pipeline_t *pipeline);
/*Changes the value of PC and ignores previously fetched instruction*/

void memoryLoader(FILE *file, proc_state_t *pState);
/*writes MEM_SIZE_WORDS starting at address on the stack indicated by memory
 field of proc_state_t */

void printProcessorState(proc_state_t *pState);
/*Prints register contents and memory*/

void decodeFetched(int instruction, proc_state_t *pState, pipeline_t *pipeline);
/*Classifies instructions and then executes them*/

void procCycle(proc_state_t *pState);
/*Function that uses pipeline to keep track of fetched/decoded instructions.
  Changes value of the PC with each execution*/

void executeOperation(proc_state_t *pState, int Rdest,
                     int Rn, int operand2, int auxResultArithmeticOps,
                     int carry, int S, bool resultAllZeros, int opcode);
/*Performs data proceesing operation involving Rn and operand2 loading or not
  the destination register, depending on the opcode*/

int getEffectiveAddress(int Rn, int offset, int U, proc_state_t *pState);
/*Returns byte address in memory where value is stored into/ loaded from*/

void executeLoadFromMemoryGPIO(proc_state_t *pState, int Rd, int address);

void loadMemoryContentIntoRegister(proc_state_t *pState, int Rd, int address);
  /*Load Rd with memory[address]. Helper function
    for executeLoadFromMemoryGPIO*/


int getMemoryContentsAtAddress(proc_state_t *pState, int address);
/*Perfoms a memory read, selecting bytes starting from the passed byte address*/

void writeToMemory(int word, int startByteAddress, proc_state_t *pState);
/*Writes to memory starting at specified byte address*/

void fillByteAddress(int byteAddress, proc_state_t *pState, int *byteArray);
/*Write each byte in array to memory starting at byteAddress*/

void printMemory(int memory[]);
/*Prints all nonzero memory contents*/

int convertToLittleEndian(int instruction);
/*returns little endian representation of an instruction/value*/
