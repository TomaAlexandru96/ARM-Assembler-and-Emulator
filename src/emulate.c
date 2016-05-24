#include "headers.h"
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>

#define MEM_SIZE_BYTES 16384 // 2 ^ 14 instruction addresses
#define NUMBER_REGS 17
#define INDEX_PC 15
#define INDEX_CPSR 16
#define INDEX_SP 13
#define INDEX_LR 14
#define INT_BITS 32
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
void memoryLoader(FILE *file, proc_state_t *pState);

void printMemory(int memory[]);

int convertToLittleEndian(int instruction);

void printProcessorState(proc_state_t *pState);

void decodeFetched(int instruction, proc_state_t *pState, pipeline_t *pipeline);

void procCycle(proc_state_t *pState);

int extractIDbits(int instruction);

bool isMult(int instruction);

void executeSDataTransfer(int instruction, proc_state_t *pState);

void executeDataProcessing(int instruction, proc_state_t *pState);

void executeMultiply(int instruction, proc_state_t *pState);

void executeBranch(int instruction, proc_state_t *pState, pipeline_t *pipeline);

uint8_t getCond(int instruction);

bool shouldExecute(int instruction, proc_state_t *pState);

int getSBit(int instruction);

int getIBit(int instruction);

int getRn(int instruction);

int getRdest(int instruction);

int getRotate(int instruction);

int getImm(int instruction);

int getShift(int instruction);

int getRm(int instruction);

int rightRotate(int value, int numberOfTimes);

int getAdditionCarry(int operand1, int operand2);

int getMSbit(int value);

int getLSbit(int value);

int getOpcode(int instruction);

bool isZero(int value);

int getShiftType(int value);

int arShift(int value, int numberOfTimes, int signBit);

int getBitAtPosition(int value, int shiftValueInteger);

void executeOperation(proc_state_t *pState, int Rdest,
                     int Rn, int operand2, int auxResultArithmeticOps,
                     int carry, int S, bool resultAllZeros, int opcode);

int getOffset(int instruction);

int getABit(int instruction);

int getSBitMul(int instruction);

int getRdMul(int instruction);

int getRnMul(int instruction);

int getRsMul(int instruction);

int getRmMul(int instruction);

int getRdSingle(int instruction);

int getBaseRegister(int Rn, int offset, int U);

int getISingle(int instruction);

int getPBit(int instruction);

int getUBit(int instruction);

int getOffsetDataTransfer(int instruction);

int getLBit(int instruction);

/*----------------------------------------------*/
int main(int argc, char **argv) {
  if(!argv[1]) {
   fprintf(stderr, "%s\n", "Wrong number of arguments");
   return EXIT_FAILURE;
  }
  FILE *file = fopen(argv[1], "rb");
  proc_state_t pState;
  memoryLoader(file, &pState);
  printf("%s\n", "State of the memory before procCycle");
  printMemory(pState.memory);
  printf("Entering procCycle with a pointer to pState %p\n", (void *) &pState);
  procCycle(&pState);
  printf("%s\n", "I finished procCycle");
  printf("0x%.8x\n", 255 << 31);
  return EXIT_SUCCESS;
}

void printProcessorState(proc_state_t *pState) {
  printf("%s\n", "~~~~~Processor state:~~~~~");
  printf("NEG = %d\n", pState->NEG);
  printf("ZER = %d\n", pState->ZER);
  printf("CRY = %d\n", pState->CRY);
  printf("OVF = %d\n", pState->OVF);
  printf("PC  = %d\n", pState->PC);
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
  printMemory(pState->memory);
}


void printPipeline(pipeline_t *pipeline) {
  printf("%s\n", "----------Pipeline----------");
  printf("Decoded: 0x%.8x \nFetched: 0x%.8x\n", pipeline->decoded, pipeline->fetched);
  printf("%s\n", "----------End Pipeline----------");
}


void procCycle(proc_state_t *pState) {
  printf("%s\n", "I am in procCycle");
  pipeline_t pipeline = {-1, -1};
  bool finished = false;
  printf("%s\n", "Before loop");
  printProcessorState(pState);
  // Initialisation
  pState->PC = 4;
  pState->regs[INDEX_PC] = pState->PC;
  // PC is stored twice in pState(regs array and separate field)
  pipeline.fetched = pState->memory[0];

  while (!finished) {
    pState->PC += 4;
    pState->regs[INDEX_PC] = pState->PC;
    pipeline.decoded = pipeline.fetched;
    pipeline.fetched = pState->memory[pState->PC / 4 - 1];
    if (pipeline.decoded != -1) {
      decodeFetched(pipeline.decoded, pState, &pipeline);
    }
    finished = !pipeline.decoded;
  }
  printf("%s\n", "After loop");
  printProcessorState(pState);
}
//--------------Execute DataProcessingI----------------------------------------
void executeDataProcessing(int instruction, proc_state_t *pState) {
   int S = getSBit(instruction);
   int I = getIBit(instruction);
   int Rn = getRn(instruction);
   int Rdest = getRdest(instruction);
   int opcode = getOpcode(instruction);
   bool resultAllZeros = false;
   int carry = -1;
   int auxResultArithmeticOps = -1;
   if(I) {
     int rotate = getRotate(instruction);
     int immediate = getImm(instruction);
     rotate =  2 * rotate;
     int rotatedImmediate = rightRotate(immediate, rotate);
     executeOperation(pState, Rdest, Rn, rotatedImmediate,
                      auxResultArithmeticOps, carry, S, resultAllZeros, opcode);
   } else {
     int shift = getShift(instruction);
     printf("shift = %d\n", shift);
     int Rm = getRm(instruction);
     printf("Rm = %d\n", Rm);
     int shiftType = getShiftType(shift); //2 bits (UX to 32)
     printf("shift type = %d\n", shiftType);
     int highBitRm = getMSbit(pState->regs[Rm]);
     if(getLSbit(shift)) {
       //register
       fprintf(stderr, "%s\n", "Optional case shift register");
     } else {
       //integer
       int maskInteger = 0xF8;
       int shiftValueInteger = (shift & maskInteger) >> 3;
       printf("shift by value = %d\n", shiftValueInteger);
       int contentsRm = pState->regs[Rm];
       int operand2ThroughShifter = -1;
       switch(shiftType) {
         case 0x0: operand2ThroughShifter = contentsRm << shiftValueInteger;
                   //carry = getMSbit(pState->regs[Rm] <<
                   //               (shiftValueInteger - 1));
                   //resultAllZeros = isZero(operand2ThroughShifter);
                   break;
         case 0x1: operand2ThroughShifter = contentsRm >> shiftValueInteger;
                   pState->CRY = getLSbit(contentsRm >>
                                    (shiftValueInteger - 1));
                   //resultAllZeros = isZero(operand2ThroughShifter);
                   break;
         case 0x2: operand2ThroughShifter = arShift(contentsRm,
                                                 shiftValueInteger, highBitRm);
                   pState->CRY = getLSbit(arShift(pState->regs[Rm],
                                     shiftValueInteger - 1, highBitRm));
                   //resultAllZeros = isZero(operand2ThroughShifter);
                   break;
         case 0x3: operand2ThroughShifter = rightRotate(pState->regs[Rm],
                                                     shiftValueInteger);
                   pState->CRY = getBitAtPosition(pState->regs[Rm],
                                             shiftValueInteger);
                   //resultAllZeros = isZero(operand2ThroughShifter);
                   break;
       }
       assert(operand2ThroughShifter != -1);
       executeOperation(pState, Rdest, Rn, operand2ThroughShifter,
                        auxResultArithmeticOps, carry, S,
                        resultAllZeros, opcode);
     }

   }




}

void executeOperation(proc_state_t *pState, int Rdest,
                     int Rn, int operand2, int auxResultArithmeticOps,
                     int carry, int S, bool resultAllZeros, int opcode) {
  switch(opcode) {
    case 0x0: pState->regs[Rdest] = pState->regs[Rn] & operand2;
    /*AND*/   auxResultArithmeticOps = pState->regs[Rdest];
              carry = 0;
              resultAllZeros = isZero(auxResultArithmeticOps);
              //Set CPSR bits if(S)
             break;
    case 0x1: pState->regs[Rdest] = pState->regs[Rn] ^ operand2;
     /*EOR*/  auxResultArithmeticOps = pState->regs[Rdest];
              carry = 0;
              resultAllZeros = isZero(pState->regs[Rdest]);
              //Set CPSR bits if(S)
             break;
    case 0x2: pState->regs[Rdest] = pState->regs[Rn] - operand2;
    /*SUB*/   auxResultArithmeticOps = pState->regs[Rdest];
              //C is 0 if borrow produced, 1 Otherwise
               carry =  getMSbit(auxResultArithmeticOps) ? 0 : 1;
               //getAdditionCarry(pState->regs[Rn],
               //                       (!(operand2) + 1)) == 1 ?
               //
              resultAllZeros = isZero(pState->regs[Rdest]);
              //Set CPSR bits if(S)
             break;
    case 0x3: pState->regs[Rdest] = operand2 - pState->regs[Rn];
     /*RSB*/  auxResultArithmeticOps = pState->regs[Rdest];
              carry = getAdditionCarry(operand2,
                                       !(pState->regs[Rn]) + 1);
              resultAllZeros = isZero(pState->regs[Rdest]);
              //Set CPSR bits if(S)
             break;
    case 0x4: pState->regs[Rdest] = pState->regs[Rn] + operand2;
    /*ADD*/   auxResultArithmeticOps = pState->regs[Rdest];
              carry = getAdditionCarry(pState->regs[Rn], operand2);
              resultAllZeros = isZero(pState->regs[Rdest]);
             //Set CPSR bits if(S)
             break;
   /***Onwards results are not written to Rd***/
    case 0x8: auxResultArithmeticOps = pState->regs[Rn] & operand2;
   /*TST*/    carry = 0;
              resultAllZeros = isZero(auxResultArithmeticOps);
              //Set CPSR bits if(S)
             break;
    case 0x9: auxResultArithmeticOps = pState->regs[Rn] ^ operand2;
   /*TEQ*/    carry = 0;
              resultAllZeros = isZero(auxResultArithmeticOps);
              //Set CPSR bits if(S)
             break;
    case 0xA: auxResultArithmeticOps = pState->regs[Rn] - operand2;
   /*CMP*/    carry = getMSbit(auxResultArithmeticOps) ? 0 : 1;
              resultAllZeros = isZero(auxResultArithmeticOps);
             //Set CPSR bits if(S)
             break;
    case 0xC: pState->regs[Rdest] = pState->regs[Rn] | operand2;
   /*ORR*/    auxResultArithmeticOps = pState->regs[Rdest];
              carry = 0;
              resultAllZeros = isZero(auxResultArithmeticOps);
              //Set CPSR bits if(S)
             break;
    case 0xD: pState->regs[Rdest] = operand2;
    /*MOV*/   carry = 0;
              resultAllZeros = false;
              auxResultArithmeticOps = operand2;
              //This result will help se the CPSR N bit
              //Set CPSR bits if(S)
             break;
  }

  if(S) {
    //Set C bit
    //barrel shifter ~ set C to carry out from any shift operation;
    //ALU ~ C = Cout of bit 31;
    pState->CRY = carry;
    //Set Z bit
    pState->ZER = resultAllZeros ? 1 : 0;
    // Set N bit
    pState->NEG = getMSbit(auxResultArithmeticOps);
    pState->regs[INDEX_CPSR] = (pState->NEG << 31) | (pState->ZER << 30) |
                               (pState->CRY << 29) | (pState->OVF << 28);
  }

}



int getBitAtPosition(int value, int shiftValueInteger) {
  if(shiftValueInteger) {
    int mask = 1 << (shiftValueInteger - 1);
    value = value & mask;
    return value >> (shiftValueInteger - 1);
  } else {
    return 0;
  }
}

int arShift(int value, int numberOfTimes, int signBit) {
  signBit = signBit << 31;
  for(int i = 0; i < numberOfTimes; i++) {
    value = value >> 1;
    value =  value | signBit;
  }
  return value;
}

int getShiftType(int value) {
  int mask1To2 = 0x6;
  return (value & mask1To2) >> 1;
}

bool isZero(int value) {
  return value == 0;
}

int getAdditionCarry(int operand1, int operand2) {
  int msbOp1 =  getMSbit(operand1);
  int msbOp2 = getMSbit(operand2);
  int result = operand1 + operand2;
  int msbResult = getMSbit(result);
  if((msbOp1 + msbOp2) != msbResult) {
    return 1;
  } else {
    return 0;
  }
}

int getMSbit(int value) {
  int maskMSb = 0x80000000;
  return (value & maskMSb) >> 31;
}

int getLSbit(int value) {
  int mask = 0x1;
  return value & mask;
}

int rightRotate(int value, int numberOfTimes) {
  return (value >> numberOfTimes) | (value << (INT_BITS - numberOfTimes));
}

int getOpcode(int instruction) {
  int mask21to24 = 0x1E00000;
  return (instruction & mask21to24) >> 21;
}

int getRotate(int instruction) {
   int mask8To11 = 0xF00;
   return (instruction & mask8To11) >> 8;
}

int getImm(int instruction) {
  int mask0To7 = 0xFF;
  return instruction & mask0To7;
}

int getShift(int instruction) {
  int mask4To11 = 0xFF0;
  return (instruction & mask4To11) >> 4;
}

int getRm(int instruction) {
  int mask0to3 = 0xF;
  return instruction & mask0to3;
}


int getSBit(int instruction) {
   int maskS20 = 0x100000;
   instruction = instruction & maskS20;
   instruction = instruction >> 20;
   return instruction;
}

int getIBit(int instruction) {
  int maskI25 = 0x2000000;
  instruction = instruction & maskI25;
  instruction = instruction >> 25;
  return instruction;
}

int getRn(int instruction) {
   int maskRn = 0xF0000;
   instruction = instruction & maskRn;
   instruction = instruction >> 16;
   return instruction;
}

int getRdest(int instruction) {
   int maskRdest = 0xF000;
   instruction = instruction & maskRdest;
   instruction = instruction >> 12;
   return instruction;
}
//------------------------------------------------------------------------------

//--------------Execute SDataTransferI------------------------------------------
void executeSDataTransfer(int instruction, proc_state_t *pState) {
  int I = getISingle(instruction);
  int L = getLBit(instruction);
  int P = getPBit(instruction);
  int U = getUBit(instruction);
  int Rn = getRn(instruction);
  int Rd = getRdSingle(instruction);
  int offset = -1;
  if(I) {
    //Offset interpreted as a shifted register

    //printf("%s\n", "Doing data transfer where offset is a register");
    int shift = getShift(instruction);
    //printf("shift = %d\n", shift);
    int Rm = getRm(instruction);
    //printf("Rm = %d\n", Rm);
    int shiftType = getShiftType(shift); //2 bits (UX to 32)
    //printf("shift type = %d\n", shiftType);
    int highBitRm = getMSbit(pState->regs[Rm]);
    if(getLSbit(shift)) {
      //register
      fprintf(stderr, "%s\n", "Optional case shift register");
    } else {
      int maskInteger = 0xF8;
      int shiftValueInteger = (shift & maskInteger) >> 3;
      int contentsRm = pState->regs[Rm];
      int operand2ThroughShifter = -1;
      switch(shiftType) {
        case 0x0: operand2ThroughShifter = contentsRm << shiftValueInteger;
                  break;
        case 0x1: operand2ThroughShifter = contentsRm >> shiftValueInteger;
                  break;
        case 0x2: operand2ThroughShifter = arShift(contentsRm,
                                          shiftValueInteger, highBitRm);
                  break;
        case 0x3: operand2ThroughShifter = rightRotate(pState->regs[Rm],
                                                    shiftValueInteger);
                  break;
      }
      assert(operand2ThroughShifter != -1);
      /*
       *operand2ThroughShifter is the value to be added/subtracted to/from Rn
       */
       offset = operand2ThroughShifter;
    }
  } else {
    //Offset interpreted as 12-bit immediate value
    offset = getOffsetDataTransfer(instruction);
  }

  int address = -1;
  if(L) {
    //Word loaded from memory into register
    //regs[Rd] = Mem[address]
     if(P) {
       //Pre-indexing
       address = getBaseRegister(Rn, offset, U);
       //Now transfer data
       pState->regs[Rd] = pState->memory[address];
     } else {
       //Post-indexing
       //Transfer data
       pState->regs[Rd] = pState->memory[address];
       //Then set base register
       pState->regs[Rn] = address;
     }

  } else {
    //Word stored in memory
    //Mem[address] = regs[Rd]
     if(P) {
       //Pre-indexing
       address = getBaseRegister(Rn, offset, U);
       //Now transfer data
       pState->memory[address] = pState->regs[Rd];
     } else {
       //Post-indexing
       //Transfer data
       pState->memory[address] = pState->regs[Rd];
       //Then set base register
       pState->regs[Rn] = address;
     }
  }
}

int getBaseRegister(int Rn, int offset, int U) {
  if(U) {
    return Rn + offset;
  } else {
    return Rn - offset;
  }
}


int getOffsetDataTransfer(int instruction) {
  int mask0To11 = 0xFFF;
  return instruction & mask0To11;
}



int getISingle(int instruction) {
  // same as I bit for Data Processing
  return getIBit(instruction);
}

int getPBit(int instruction) {
  int mask24 = 0x1000000;
  return (instruction & mask24) >> 24;
}

int getUBit(int instruction) {
  int mask23 = 0x800000;
  return (instruction & mask23) >> 23;
}

int getLBit(int instruction) {
  int mask20 = 0x100000;
  return (instruction & mask20) >> 20;
}

int getRnSingle(int instruction) {
  // same as Rn for Data Processing
  return getRn(instruction);
}

int getRdSingle(int instruction) {
  // same as Rd bit for Data Processing
  return getRdest(instruction);
}

//------------------------------------------------------------------------------

//--------------Execute MultiplyI-----------------------------------------------
void executeMultiply(int instruction, proc_state_t *pState) {
  int A = getABit(instruction);
  int S = getSBitMul(instruction);
  int Rd = getRdMul(instruction);
  int Rn = getRnMul(instruction);
  int Rs = getRsMul(instruction);
  int Rm = getRmMul(instruction);
  int auxResultMult = -1;
  assert(Rd != Rm);
  if(A) {
    pState->regs[Rd] = (pState->regs[Rm] * pState->regs[Rs]) + pState->regs[Rn];
  } else {
    pState->regs[Rd] = pState->regs[Rm] * pState->regs[Rs];
  }
  auxResultMult = pState->regs[Rd];
  if(S) {
    //Set NEG and ZER flags
    pState->NEG = getMSbit(auxResultMult);
    pState->ZER = auxResultMult ? 0 : 1;
    pState->regs[INDEX_CPSR] = (pState->NEG << 31) | (pState->ZER << 30) |
                               (pState->CRY << 29) | (pState->OVF << 28);
  }
}

int getABit(int instruction) {
  int maskA21 = 0x200000;
  return (instruction & maskA21) >> 21;
}

int getSBitMul(int instruction) {
  int maskS20 = 0x100000;
  return (instruction & maskS20) >> 20;
}

int getRdMul(int instruction) {
  //reuse function getRn from DataProcessingI
  return getRn(instruction);
}

int getRnMul(int instruction) {
  return getRdest(instruction);
}

int getRsMul(int instruction) {
  int mask8To11 = 0xF00;
  return (instruction & mask8To11) >> 8;
}

int getRmMul(int instruction) {
  int mask0To3 = 0xF;
  return (instruction & mask0To3);
}

//------------------------------------------------------------------------------

//--------------Execute Branch--------------------------------------------------
void executeBranch(int instruction, proc_state_t *pState, pipeline_t *pipeline){
    int offset = getOffset(instruction);
    offset = offset << 2;
    //Offset is a 32-bit value having bits[25...31] equal 0
    int maskBit25 = 0x2000000;
    int signBitPosition = 25;
    int signBit = (offset & maskBit25) >> signBitPosition;
    int mask26To31 = 0xFC000000;
    if(signBit) {
      offset = offset | mask26To31;
    }
    // otherwise the offset is unchanged
    int PCvalue = pState-> PC;
    pState->PC = PCvalue + offset;
    pState->regs[INDEX_PC] = pState->PC;
    pipeline->decoded = -1;
    pipeline->fetched = -1;
}

int getOffset(int instruction) {
  int maskOffset = 0xFFFFFF;
  return instruction & maskOffset;
}

//------------------------------------------------------------------------------

//--------------General Helper Functions Execution------------------------------
bool shouldExecute(int instruction, proc_state_t *pState) {
   uint8_t cond = getCond(instruction);
   int N = pState->NEG;
   int Z = pState->ZER;
   int V = pState->OVF;
   switch(cond) {
     case 0:  return Z == 1;
     case 1:  return Z == 0;
     case 10: return N == V;
     case 11: return N != V;
     case 12: return ((Z == 0) && (N == V));
     case 13: return ((Z == 1) || (N != V));
     case 14: return true;
     default: return false;
   }
}

uint8_t getCond(int instruction) {
  int mask = 0xF0000000;
  uint8_t cond;
  uint8_t maskLS4 = 0xF;
  instruction = instruction & mask;
  instruction = instruction >> 28;
  cond = (uint8_t) (instruction & maskLS4);
  return cond;
}

//-----------------------------------------------------------------------------
void decodeFetched(int instruction, proc_state_t *pState, pipeline_t *pipeline){
  printf("I am decoding %x\n", instruction);
   int idBits = extractIDbits(instruction);
   if(idBits == 1) {
     //check if Cond satisfied before executing
       printf("%s\n", "This is data transfer instruction");
       if(shouldExecute(instruction, pState)) {
        executeSDataTransfer(instruction, pState);
       }
   } else if(idBits == 2) {
     //check if Cond satisfied before executing
      printf("%s\n", "This is branch instruction");
      if(shouldExecute(instruction, pState)) {
        executeBranch(instruction, pState, pipeline);
      }
   } else if(!idBits) {
      //Choose between MultiplyI and DataProcessingI
       if(isMult(instruction)) {
         printf("%s\n", "This is multiply instruction");
         if(shouldExecute(instruction, pState)) {
           executeMultiply(instruction, pState);
         }
       } else {
         //Then check if Cond satisfied before executing
         printf("%s\n", "This is data processing instruction");
         if(shouldExecute(instruction, pState)) {
          executeDataProcessing(instruction, pState);
         }
       }
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


int extractIDbits(int instruction) {
  int mask = 0xC000000;
  instruction =  instruction & mask;
  //call by value, so the decoded instruction will not change
  return instruction >> 26;
}


void memoryLoader(FILE *file, proc_state_t *pState) {
  //Pointers are passed to functions by value(they are addresses)
  //So, passing *file makes a copy of the original pointer
  if(!file) {
    fprintf(stderr, "%s\n", "File not found");
  }
  fread(pState->memory, sizeof(uint32_t), MEM_SIZE_BYTES, file);
  fclose(file);
  //load every instruction in binary file into memory
  //Instructions stored in Big Endian, ready to be decoded
  printMemory(pState->memory);

}

void printMemory(int memory[]) {
   printf("%s\n", "Non-zero memory:\n");
   printf("%s\n", "Memory layout LITTLE ENDIAN: ");
   for(int i = 0; i < MEM_SIZE_BYTES; i += 4) {
     if(memory[i / 4]) {
       printf("0x%.8x: 0x%.8x\n", i, convertToLittleEndian(memory[i / 4]));
     } else {
       break;
     }
   }
   printf("%s\n", "");
}


int convertToLittleEndian(int instruction) {
  int maskByte0 = 0xFF;
  int maskByte1 = 0xFF00;
  int maskByte2 = 0xFF0000;
  int maskByte3 = 0xFF000000;
  return ((instruction >> 24) & maskByte0) |
         ((instruction >> 8) & maskByte1)  |
         ((instruction << 8) & maskByte2)  |
         ((instruction << 24) & maskByte3);
}
