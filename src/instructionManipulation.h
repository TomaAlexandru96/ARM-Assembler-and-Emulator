#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include "headers.h"
#include <limits.h>
#include <stdbool.h>
#include <assert.h>

#define INT_BITS 32
#define MASK_BYTE0_BE 0xFF000000
#define MASK_BYTE1_BE 0xFF0000
#define MASK_BYTE2_BE 0xFF00
#define MASK_BYTE3_BE 0xFF

int extractIDbits(int instruction);

bool isMult(int instruction);

uint8_t getCond(int instruction);

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

int getShiftRegister(int instruction);

int getOffset(int instruction);

int getABit(int instruction);

int getSBitMul(int instruction);

int getRdMul(int instruction);

int getRnMul(int instruction);

int getRsMul(int instruction);

int getRmMul(int instruction);

int getRdSingle(int instruction);

int getISingle(int instruction);

int getPBit(int instruction);

int getUBit(int instruction);

int getOffsetDataTransfer(int instruction);

int getLBit(int instruction);

int getByteBigEndian(int content, int index);

int getNumberOfDecimalDigits(int number);

int setByte(int word, int index, int newByte);

#endif
