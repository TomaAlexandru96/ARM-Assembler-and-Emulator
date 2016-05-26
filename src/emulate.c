#include "headers.h"
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>
#include <stdlib.h>

#define MEM_SIZE_BYTES 16384 // 2 ^ 14 instruction addresses
#define NUMBER_REGS 17
#define INDEX_PC 15
#define INDEX_CPSR 16
#define INDEX_SP 13
#define INDEX_LR 14
#define INT_BITS 32
#define MASK_BYTE0_BE 0xFF000000
#define MASK_BYTE1_BE 0xFF0000
#define MASK_BYTE2_BE 0xFF00
#define MASK_BYTE3_BE 0xFF

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

int executeShift(int contentsRm, int shiftValueInteger, proc_state_t *pState,
                 int shiftType, int highBitRm, int Rm, bool setFlags);

int arShift(int value, int numberOfTimes, int signBit);

int getBitAtPosition(int value, int shiftValueInteger);

int getShiftRegister(int instruction);

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

int getEffectiveAddress(int Rn, int offset, int U, proc_state_t *pState);

int getISingle(int instruction);

int getPBit(int instruction);

int getUBit(int instruction);

int getOffsetDataTransfer(int instruction);

int getLBit(int instruction);

int getMemoryContentsAtAddress(proc_state_t *pState, int address);

int getByteBigEndian(int content, int index);

int getNumberOfDecimalDigits(int number);

int setByte(int word, int index, int newByte);

void writeToMemory(int word, int startByteAddress, proc_state_t *pState);

/*----------------------------------------------*/
int main(int argc, char **argv) {
  if(!argv[1]) {
   fprintf(stderr, "%s\n", "Wrong number of arguments");
   return EXIT_FAILURE;
  }
  FILE *file = fopen(argv[1], "rb");
  proc_state_t pState;
  memoryLoader(file, &pState);
  procCycle(&pState);

  return EXIT_SUCCESS;
}

void printProcessorState(proc_state_t *pState) {
  printf("%s\n", "Registers:");
  int content;
  for(int i = 0; i < NUMBER_REGS; i++) {
    content = (pState->regs)[i];
    if(i != INDEX_LR && i != INDEX_SP) {
     if(i == INDEX_PC) {
       printf("PC  :%11d (0x%.8x)\n", content, content);
     } else if(i == INDEX_CPSR) {
       if(getMSbit(pState->regs[i]) == -1) {
          printf("CPSR: %11d (0x%.8x)\n", content, content);
       } else {
          printf("CPSR:%11d (0x%.8x)\n", content, content);
       }
     } else {
       if(getNumberOfDecimalDigits(pState->regs[i]) >= 10 &&
         getMSbit(pState->regs[i]) == -1) {
          printf("$%-3d: %11d (0x%.8x)\n", i, content, content);
       } else {
          printf("$%-3d:%11d (0x%.8x)\n", i, content, content);
       }
     }
    }
  }
  printMemory(pState->memory);
}

int getNumberOfDecimalDigits(int number) {
  int counter = 0;
  while (number) {
    number /= 10;
    counter++;
  }
  return counter;
}


void printPipeline(pipeline_t *pipeline) {
  printf("%s\n", "----------Pipeline----------");
  printf("Decoded: 0x%.8x \nFetched: 0x%.8x\n", pipeline->decoded, pipeline->fetched);
  printf("%s\n", "----------End Pipeline----------");
}


void procCycle(proc_state_t *pState) {
  pipeline_t pipeline = {-1, -1};
  bool finished = false;
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
     int Rm = getRm(instruction);
     int shiftType = getShiftType(shift); //2 bits (UX to 32)
     int highBitRm = getMSbit(pState->regs[Rm]);
     int contentsRm = pState->regs[Rm];
     int operand2ThroughShifter = -1;
     if(getLSbit(shift)) {
       //register
       //bit7 must be 0
       int maskBit7 = 0x80;
       int bit7 = (instruction & maskBit7) >> 7;
       if(bit7) {
         fprintf(stderr, "%s\n", "Operand2 is invalid");
       }
       int Rs = getShiftRegister(instruction);
       int shiftValue = getByteBigEndian(pState->regs[Rs], 3);
       operand2ThroughShifter = executeShift(contentsRm, shiftValue, pState,
                                             shiftType, highBitRm, Rm, true);
     } else {
       //integer
       int maskInteger = 0xF8;
       int shiftValueInteger = (shift & maskInteger) >> 3;
       operand2ThroughShifter = executeShift(contentsRm, shiftValueInteger,
                                             pState, shiftType, highBitRm,
                                             Rm, true);
     }
     executeOperation(pState, Rdest, Rn, operand2ThroughShifter,
                      auxResultArithmeticOps, carry, S,
                      resultAllZeros, opcode);
   }

}


int executeShift(int contentsRm, int shiftValue, proc_state_t *pState,
                 int shiftType, int highBitRm, int Rm, bool setFlags) {

  uint32_t operand2ThroughShifter = -1;
  switch(shiftType) {
    case 0x0: operand2ThroughShifter = contentsRm << shiftValue;
              break;
    case 0x1: operand2ThroughShifter = ((uint32_t) contentsRm) >> shiftValue;
              if(setFlags) {
                pState->CRY = getLSbit(contentsRm >> (shiftValue - 1));
              }
              break;
    case 0x2: operand2ThroughShifter = arShift(contentsRm,
                                            shiftValue, highBitRm);
              if(setFlags) {
                 pState->CRY = getLSbit(arShift(pState->regs[Rm],
                                shiftValue - 1, highBitRm));
              }
              break;
    case 0x3: operand2ThroughShifter = rightRotate(pState->regs[Rm],
                                                shiftValue);
              if(setFlags) {
                pState->CRY = getBitAtPosition(pState->regs[Rm],
                                               shiftValue);
              }
              break;
  }
  assert(operand2ThroughShifter != -1);
  return operand2ThroughShifter;
}

int getShiftRegister(int instruction) {
  int maskShiftRegister = 0xF00;
  return (instruction & maskShiftRegister) >> 8;
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
              carry =  getMSbit(auxResultArithmeticOps) ? 0 : 1;
              resultAllZeros = isZero(pState->regs[Rdest]);
              //Set CPSR bits if(S)
             break;
    case 0x3: pState->regs[Rdest] = operand2 - pState->regs[Rn];
    /*RSB*/   auxResultArithmeticOps = pState->regs[Rdest];
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
    int shift = getShift(instruction);
    int Rm = getRm(instruction);
    int shiftType = getShiftType(shift); //2 bits (UX to 32)
    int highBitRm = getMSbit(pState->regs[Rm]);
    int contentsRm = pState->regs[Rm];
    int operand2ThroughShifter = -1;
    if(getLSbit(shift)) {
      //register
      fprintf(stderr, "%s\n", "Operand2 is invalid");
    } else {
      int maskInteger = 0xF8;
      int shiftValueInteger = (shift & maskInteger) >> 3;

      operand2ThroughShifter = executeShift(contentsRm, shiftValueInteger,
                               pState, shiftType, highBitRm, Rm, false);
    }
    //operand2ThroughShifter is the value to be added/subtracted to/from Rn
    offset = operand2ThroughShifter;
  } else {
    //Offset interpreted as 12-bit immediate value
    offset = (uint32_t) getOffsetDataTransfer(instruction);
  }
  int address = -1;
  if(L) {
    //Word loaded from memory into register
    //regs[Rd] = Mem[address]
     if(P) {
       //Pre-indexing
       address = getEffectiveAddress(Rn, offset, U, pState);
       //Now transfer data
       if(address > (MEM_SIZE_BYTES - 4)) {
         printf("Error: Out of bounds memory access at address 0x%.8x\n",
                address);
       } else {
          pState->regs[Rd] = getMemoryContentsAtAddress(pState, address);
       }
     } else {
       //Post-indexing
       address = pState->regs[Rn];
       //Transfer data
       if(address > (MEM_SIZE_BYTES - 4)) {
         printf("Error: Out of bounds memory access at address 0x%.8x\n",
                address);
       } else {
          pState->regs[Rd] = getMemoryContentsAtAddress(pState, address);
       }
       //Then set base register
       pState->regs[Rn] = getEffectiveAddress(Rn,offset, U, pState);
     }
  } else {
    //Word stored in memory
    //Mem[address] = regs[Rd]
     if(P) {
       //Pre-indexing
       address = getEffectiveAddress(Rn, offset, U, pState);
       //Now transfer data
       //pState->memory[address / 4] = pState->regs[Rd];
       writeToMemory(pState->regs[Rd], address, pState);
     } else {
       //Post-indexing
       //Transfer data
       address = pState->regs[Rn];
       writeToMemory(pState->regs[Rd], address, pState);
       //pState->memory[address / 4] = pState->regs[Rd];
       //Then set base register
       pState->regs[Rn] = getEffectiveAddress(Rn, offset, U, pState);
     }
  }
}

void writeToMemory(int word, int startByteAddress, proc_state_t *pState) {
  //word written to memory starting from startByteAddress
  int byte[4] = {getByteBigEndian(word, 3),
                 getByteBigEndian(word, 2),
                 getByteBigEndian(word, 1),
                 getByteBigEndian(word, 0)};
  //Got every byte of the number to be written to memory
  for(int i = 0; i < 4; i++) {
    pState->memory[startByteAddress / 4] =
    setByte(pState->memory[startByteAddress / 4],
            3 - startByteAddress % 4,
            byte[i]);
            startByteAddress++;
  }
}

int setByte(int word, int index, int newByte) {
  // word indexed as in big endian {0, 1, 2, 3}
  switch (index) {
    case 0: return (0x00FFFFFF & word) | (newByte << 24);
    case 1: return (0xFF00FFFF & word) | (newByte << 16);
    case 2: return (0xFFFF00FF & word) | (newByte << 8);
    case 3: return (0XFFFFFF00 & word) | newByte;
    default: return -1;
  }
}


int getMemoryContentsAtAddress(proc_state_t *pState, int address) {
  int byteAddress = address;
  int byte0 = getByteBigEndian(pState->memory[byteAddress / 4],
                               3 - (byteAddress % 4));
  byteAddress++;
  int byte1 = getByteBigEndian(pState->memory[byteAddress / 4],
                               3 - (byteAddress % 4));
  byteAddress++;
  int byte2 = getByteBigEndian(pState->memory[byteAddress / 4],
                               3 - (byteAddress % 4));
  byteAddress++;
  int byte3 = getByteBigEndian(pState->memory[byteAddress / 4],
                               3 - (byteAddress % 4));
  byteAddress++;
  return byte0 | (byte1 << 8) | (byte2 << 16) | (byte3 << 24);
}

int getByteBigEndian(int content, int index) {
  switch (index) {
    case 0: return (content & MASK_BYTE0_BE) >> 24;
    case 1: return (content & MASK_BYTE1_BE) >> 16;
    case 2: return (content & MASK_BYTE2_BE) >> 8;
    case 3: return (content & MASK_BYTE3_BE);
    default: return -1;
  }
}


int getEffectiveAddress(int Rn, int offset, int U, proc_state_t *pState) {
  if(U) {
    return (pState->regs[Rn] + offset);
  } else {
    return (pState->regs[Rn] - offset);
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
    pState->regs[Rd] = (int) (pState->regs[Rm] * pState->regs[Rs]) +
                                                 pState->regs[Rn];
  } else {
    pState->regs[Rd] = (int) pState->regs[Rm] * pState->regs[Rs];
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
   int idBits = extractIDbits(instruction);
   if(idBits == 1) {
     //check if Cond satisfied before executing
       if(shouldExecute(instruction, pState)) {
        executeSDataTransfer(instruction, pState);
       }
   } else if(idBits == 2) {
     //check if Cond satisfied before executing
      if(shouldExecute(instruction, pState)) {
        executeBranch(instruction, pState, pipeline);
      }
   } else if(!idBits) {
      //Choose between MultiplyI and DataProcessingI
       if(isMult(instruction)) {
         if(shouldExecute(instruction, pState)) {
           executeMultiply(instruction, pState);
         }
       } else {
         //Then check if Cond satisfied before executing
         if(shouldExecute(instruction, pState)) {
          executeDataProcessing(instruction, pState);
         }
       }
   } else {
     printf("%s\n", "Should not get here");
     fprintf(stderr, "%s\n", "Invalid instruction executing.");
     exit(EXIT_FAILURE);
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
}

void printMemory(int memory[]) {
   printf("%s", "Non-zero memory:\n");
   for(int i = 0; i < MEM_SIZE_BYTES; i += 4) {
     if(memory[i / 4]) {
       printf("0x%.8x: 0x%.8x\n", i, convertToLittleEndian(memory[i / 4]));
     }
   }
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
