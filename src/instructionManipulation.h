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
/*Returns instruction ID bits(not unique for all instructions). Used to
  classify instructions*/

bool isMult(int instruction);
/*returns true iff instruction is a Multiply Instruction*/

uint8_t getCond(int instruction);
/*Returns bit pattern for instruction condition. Indicates whether the
  instruction should or not be executed*/

int getSBit(int instruction);
/*Extracts S bit from data proc instruction. Use: flgas set iff S bit*/

int getIBit(int instruction);
/*I bit indicates whether operand is immediate or shifted register*/

int getRn(int instruction);
/* Register operand used in arithmetic operations*/

int getRdest(int instruction);
/*Gets index of register where result is stored*/

int getRotate(int instruction);
/*Returns number of rotations to be performed*/

int getImm(int instruction);
/*Gets immediate value in case I is set(Data processing)*/

int getShift(int instruction);
/*Returns first 12 bits of instruction.*/

int getRm(int instruction);
/*Returns index of register to be shifted*/

int rightRotate(int value, int numberOfTimes);
/*Performs a rotation of value numberOfTimes times*/

int getAdditionCarry(int operand1, int operand2);
/*returns resulting carry of operand1 + operand1*/

int getMSbit(int value);
/*returns bit 31 of value*/

int getLSbit(int value);
/*returns bit 0 of value*/

int getOpcode(int instruction);
/*returns associated value of operation to be performed(data proceesing)*/

bool isZero(int value);
/*returns true iff value is 0*/

int getShiftType(int value);
/*returns associated value of shift type-barrel shifter*/

int arShift(int value, int numberOfTimes, int signBit);
/*Performs arithmetic right shift on value, preserving sign bit*/

int getBitAtPosition(int value, int shiftValueInteger);
/*returns bit from value at index shiftValueInteger*/

int getShiftRegister(int instruction);
/*Returns register that holds number of shifts to be performed on Rm*/

int getOffset(int instruction);
/*retuns offset for data transfer instruction*/

int getABit(int instruction);
/*returns accumulate bit*/

int getSBitMul(int instruction);
/*returns set condition codes bit*/

int getRdMul(int instruction);
/*returns index of destination register for Multiply*/

int getRnMul(int instruction);
/*return index of multiplication register operand(accumulate)*/

int getRsMul(int instruction);
/*return index of multiplication register operand*/

int getRmMul(int instruction);
/*return index of multiplication register operand*/

int getRdSingle(int instruction);
/*returns index of register where data should be written to/loaded from*/

int getISingle(int instruction);
/*returns immediate bit: I = 0, then immediate offset; I = 1, then shifted reg*/

int getPBit(int instruction);
/*returns pre/post indexing bit*/

int getUBit(int instruction);
/*indicates whether offset should be subtracted from/ added to Rn*/

int getOffsetDataTransfer(int instruction);
/*Gets offset value. Processed as in data processing if I = 1*/

int getLBit(int instruction);
/*indicates whether a load or store should be performed*/

int getByteBigEndian(int content, int index);
/*Assumption memory layout: ~address~ BYTE0 BYTE1 BYTE2 BYTE3.
  Gets byte at index*/

int getNumberOfDecimalDigits(int number);
/*used for printing processor state*/

int setByte(int word, int index, int newByte);
/*sets byte at index in big endian format(assumption as in getByteBigEndian)*/

#endif
