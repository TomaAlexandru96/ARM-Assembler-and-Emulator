#include "instructionManipulation.h"

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
  return (instruction & mask) >> 26;
}
//-----------------------------------------------------------------------------

//--------------Execute DataProcessingI----------------------------------------
int getShiftRegister(int instruction) {
  int maskShiftRegister = 0xF00;
  return (instruction & maskShiftRegister) >> 8;
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
   return (instruction & maskS20) >> 20;
}

int getIBit(int instruction) {
  int maskI25 = 0x2000000;
  return (instruction & maskI25) >> 25;
}

int getRn(int instruction) {
   int maskRn = 0xF0000;
   return (instruction & maskRn) >> 16;
}

int getRdest(int instruction) {
   int maskRdest = 0xF000;
   return (instruction & maskRdest) >> 12;
}
//------------------------------------------------------------------------------

//--------------Execute SDataTransferI------------------------------------------
int setByte(int word, int index, int newByte) {
  // word indexed as in big endian {0, 1, 2, 3}
  switch (index) {
    case 0: return (0x00FFFFFF & word) | (newByte << BYTE0_LSB_INDEX);
    case 1: return (0xFF00FFFF & word) | (newByte << BYTE1_LSB_INDEX);
    case 2: return (0xFFFF00FF & word) | (newByte << BYTE2_LSB_INDEX);
    case 3: return (0XFFFFFF00 & word) | newByte;
    default: return -1;
  }
}

int getByteBigEndian(int content, int index) {
  switch (index) {
    case 0: return (content & MASK_BYTE0_BE) >> BYTE0_LSB_INDEX;
    case 1: return (content & MASK_BYTE1_BE) >> BYTE1_LSB_INDEX;
    case 2: return (content & MASK_BYTE2_BE) >> BYTE2_LSB_INDEX;
    case 3: return (content & MASK_BYTE3_BE);
    default: return -1;
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
int getOffset(int instruction) {
  int maskOffset = 0xFFFFFF;
  return instruction & maskOffset;
}

//------------------------------------------------------------------------------

//--------------General Helper Functions Execution------------------------------
uint8_t getCond(int instruction) {
  int mask = 0xF0000000;
  uint8_t cond;
  uint8_t maskLS4 = 0xF;
  instruction = (instruction & mask) >> 28;
  cond = (uint8_t) (instruction & maskLS4);
  return cond;
}

//-----------------------------------------------------------------------------
