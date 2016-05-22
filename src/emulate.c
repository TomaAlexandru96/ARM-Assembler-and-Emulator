#include "headers.h"
#include <limits.h>
#include <stdbool.h>

#define MEM_SIZE_BYTES 65536
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
void memoryLoader(FILE *file, proc_state_t pState);

void printMemory(int memory[]);

void decodeFetched(int instruction);

void procCycle(proc_state_t *pState);

int extractCond(int instruction);

int extractIDbits(int instruction);

bool isMult(int instruction);
/*----------------------------------------------*/
int main(int argc, char **argv) {
  if(!argv[1]) {
   fprintf(stderr, "%s\n", "Wrong number of arguments");
   return EXIT_FAILURE;
  }
  FILE *file = fopen(argv[1], "rb");
  proc_state_t pState;
  memoryLoader(file, pState);
  printf("Entering procCycle with a pointer to pState %p\n", (void *) &pState);
  procCycle(&pState);
printf("%s\n", "I finished procCycle");
  return EXIT_SUCCESS;
}

void printProcessorState(proc_state_t *pState) {
  printf("%s\n", "~~~~~Processor state:~~~~~");
  printf("NEG = %d\n", pState->NEG);
  printf("ZER = %d\n", pState->ZER);
  printf("CRY = %d\n", pState->CRY);
  printf("OVF = %d\n", pState->OVF);
  printf("PC = %d\n", pState->PC);
  printf("%s\n", "Registers:");
  int content;
  for(int i = 0; i < NUMBER_REGS; i++) {
    content = (pState->regs)[i];
    if(i != INDEX_LR && i != INDEX_SP) {
     if(i == INDEX_PC) {
       printf("PC   :%10d (0x%.8x)\n", content, content);
     } else if(i == INDEX_CPSR) {
       printf("CPSR :%10d (0x%.8x)\n", content, content);
     } else {
       printf("$%2d  :%10d (0x%.8x)\n", i, content, content);
     }
    }
  }
  printf("%s\n", "~~~~~End Processor state:~~~~~");
}


void printPipeline(pipeline_t *pipeline) {
  printf("%s\n", "----------Pipeline----------");
  printf("Decoded: 0x%.8x \nFetched: 0x%.8x\n", pipeline->decoded, pipeline->fetched);
  printf("%s\n", "----------End Pipeline----------");
}


void procCycle(proc_state_t *pState) {
  printf("%s\n", "I am in procCycle");
  pipeline_t pipeline;
  //int firstInstruction = (pState->memory)[0];
  //printf("First instruction: %x\n", firstInstruction);
  printMemory(pState->memory);
  pipeline.fetched = (pState->memory)[pState->PC];
  pipeline.decoded = -1;
  printPipeline(&pipeline);
   printf("%s\n", "Before loop");
   printProcessorState(pState);
  while(pipeline.fetched) {
     decodeFetched(pipeline.decoded);
     pipeline.decoded = pipeline.fetched;
     pState->PC += 4;
     pipeline.fetched = pState->memory[pState->PC / 4];
  }

}


void decodeFetched(int instruction) {
  printf("I am decoding %x\n", instruction);
   int idBits = extractIDbits(instruction);
   if(idBits == 1) {
     //check if Cond satisfied before executing TODO
     printf("%s\n", "This is data transfer instruction");
    //  executeSDataTransfer(instruction);  TODO
   } else if(idBits == 2) {
     //check if Cond satisfied before executing TODO
     printf("%s\n", "This is branch instruction");
    //  executeBranch(instruction); TODO
   } else if(!idBits) {
      //Choose between MultiplyI and DataProcessingI
       if(isMult(instruction)) {
         printf("%s\n", "This is multiply instruction");
    //  TODO   executeMultiply(instruction);
       } else {
         printf("%s\n", "This is data processing instruction");
      // TODO  executeDataProcessing(instruction);
       }
      //Then check if Cond satisfied before executing
   } else {
     printf("%s\n", "Should not get here");
     fprintf(stderr, "%s\n", "Invalid instruction executing.");
   }

  /*when executing: if Cond succeeds or is al(always), instruction
    is executed. Otherwise not */
}

bool isMult(int instruction) {
  int mask2522 = 0x3C00000;
  int mask74 = 0xC000F0;
  int instrCopy = instruction;
  instrCopy =  instrCopy & mask74;
  instrCopy = instrCopy >> 4;
  int multPattern = 0x9;
  if(((mask2522 & instruction) == 0) && (multPattern == instrCopy)) {
     return true;
  } else {
    return false;
  }
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
   printf("%s\n", "Memory layout: ");
   for(int i = 0; i < MEM_SIZE_BYTES; i++) {
     if(memory[i]) {
       printf("Mem[%d] = %d\n", i, memory[i]);
     } else {
       break;
     }
   }
   printf("%s\n", "");
}
