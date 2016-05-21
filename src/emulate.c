#include "headers.h"
#include <limits.h>

#define MEM_SIZE_BYTES 65536
#define NUMBER_REGS 17

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
void memoryLoader(FILE *file, proc_state_t pState);

void printMemory(int memory[]);

void decodeFetched(int instruction);

void procCycle(proc_state_t *pState);

int extractCond(int instruction);

int extractIDbits(int instruction);
/*----------------------------------------------*/
int main(int argc, char **argv) {
  if(!argv[1]) {
   fprintf(stderr, "%s\n", "Wrong number of arguments");
   return EXIT_FAILURE;
  }
  FILE *file = fopen(argv[1], "rb");
  proc_state_t pState;
  memoryLoader(file, pState);
  procCycle(&pState);

  return EXIT_SUCCESS;
}

void procCycle(proc_state_t *pState) {
  pipeline_t pipeline;
  pipeline.fetched = pState->memory[pState->PC];
  pipeline.decoded = -1;
  while(pipeline.fetched) {
     decodeFetched(pipeline.decoded);
     pipeline.decoded = pipeline.fetched;
     pState->PC += 4;
     pipeline.fetched = pState->memory[pState->PC / 4];
  }

}


void decodeFetched(int instruction) {
   int idBits = extractIDbits(instruction);
   if(idBits == 1) {
      executeSDataTransfer(instruction); //TODO
   } else if(idBits == 2) {
      executeBranch(instruction); //TODO
   } else {
      //Choose between MultiplyI or DataProcessingI
      //TODO
   }

  /*when executing: if cond succeeds or is al(always), instruction
    is executed. Otherwise not */
}

int extractCond(int instruction) {
  int mask = 0x80000000;
  instruction = instruction & mask;
  //call by value, so the decoded instruction will not change
  return instruction >> 29;
}

int extractIDbits(int instruction) {
  int mask = 0xC000000;
  instruction =  instruction & mask;
  //call by value, so the decoded instruction will not change
  return instruction >> 26;
}


void memoryLoader(FILE *file, proc_state_t pState) {
  //Pointers are passed to functions by value(they are addresses)
  //So, passing *file makes a copy of the original pointer
  if(!file) {
    fprintf(stderr, "%s\n", "File not found");
  }
  fread(pState.memory, sizeof(uint32_t), MEM_SIZE_BYTES, file);
  fclose(file);
  //load every instruction in binary file into memory
  //Instructions stored in Big Endian, ready to be decoded
  printMemory(pState.memory);

}


void printMemory(int memory[]) {
   for(int i = 0; i < MEM_SIZE_BYTES; i++) {
     if(memory[i]) {
       printf("%d\n", memory[i]);
     } else {
       break;
     }
   }
}
