#include "instructionManipulation.h"
#include "emulate.h"

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
  printf("Decoded: 0x%.8x \nFetched: 0x%.8x\n", pipeline->decoded,
                                                pipeline->fetched);
  printf("%s\n", "----------End Pipeline----------");
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
       if(address > (MEM_SIZE_WORDS - 4)) {
         printf("Error: Out of bounds memory access at address 0x%.8x\n",
                address);
       } else {
          pState->regs[Rd] = getMemoryContentsAtAddress(pState, address);
       }
     } else {
       //Post-indexing
       address = pState->regs[Rn];
       //Transfer data
       if(address > (MEM_SIZE_WORDS - 4)) {
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
       writeToMemory(pState->regs[Rd], address, pState);
     } else {
       //Post-indexing
       //Transfer data
       address = pState->regs[Rn];
       writeToMemory(pState->regs[Rd], address, pState);
       //Then set base register
       pState->regs[Rn] = getEffectiveAddress(Rn, offset, U, pState);
     }
  }
}


int getEffectiveAddress(int Rn, int offset, int U, proc_state_t *pState) {
  if(U) {
    return (pState->regs[Rn] + offset);
  } else {
    return (pState->regs[Rn] - offset);
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

//------------------------------------------------------------------------------


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



void memoryLoader(FILE *file, proc_state_t *pState) {
  //Pointers are passed to functions by value(they are addresses)
  //So, passing *file makes a copy of the original pointer
  if(!file) {
    fprintf(stderr, "%s\n", "File not found");
  }
  fread(pState->memory, sizeof(uint32_t), MEM_SIZE_WORDS, file);
  fclose(file);
  //load every instruction in binary file into memory
  //Instructions stored in Big Endian, ready to be decoded
}

void printMemory(int memory[]) {
   printf("%s", "Non-zero memory:\n");
   for(int i = 0; i < MEM_SIZE_WORDS; i += 4) {
     if(memory[i / 4]) {
       printf("0x%.8x: 0x%.8x\n", i, convertToLittleEndian(memory[i / 4]));
     }
   }
}
