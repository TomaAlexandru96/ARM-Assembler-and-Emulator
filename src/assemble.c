#include "mappings.h"

#define MAX_LINE_LENGTH 512
#define DELIMITERS " ,\n"
#define MEMORY_SIZE 4
#define PC_OFFSET 2
#define INSTRUCTION_SIZE 32
#define ALWAYS_CONDITION ""
#define BRANCH_OFFSET_SIZE  26

uint32_t firstPass(FILE *input, map *labelMapping,
              vector *errorVector, uint32_t *instructionsNumber);
void secondPass(uint32_t linesNumber, uint32_t instructions[],
              vector *errorVector, FILE *input, map labelMapping);
void printStringArray(int n, char arr[][MAX_LINE_LENGTH]);
vector tokenise(char *start, char *delimiters);
void printBinary(uint32_t nr);
void setCond(uint32_t *x, char *cond);
/**
* Converts unsigned int to string
**/
char *uintToString(uint32_t num);
uint32_t decode(vector *tokens, uint32_t instructionNumber,
                map labelMapping, vector *errorVector, char *ln);
uint32_t decodeDataProcessing(vector *tokens, vector *errorVector, char *ln);
uint32_t decodeMultiply(vector *tokens, vector *errorVector, char *ln);
int checkRegMult(vector *tokens, char *multType,
                vector *errorVector, char *ln);
uint32_t decodeSingleDataTransfer(vector *tokens, vector *errorVector,
                                  char *ln);
uint32_t decodeBranch(vector *tokens, uint32_t instructionNumber,
                map labelMapping, vector *errorVector, char *ln);
void throwUndeifinedError(char *name, vector *errorVector, char *ln);
void throwLabelError(char *name, vector *errorVector, char *ln);
void throwExpressionError(char *expression, vector *errorVector, char *ln);
void throwExpressionMissingError(char *ins, vector *errorVector, char *ln);
bool isExpression(char *token);
bool isInstruction(char *token);
bool isLabel(char *token);
bool isRegister(char *token);
typeEnum getType(char *token);
uint32_t getExpression(char *exp, vector *errorVector, char *ln);
uint32_t getHex(char *exp);
uint32_t getDec(char *exp);

int main(int argc, char **argv) {
  // DEBUG ZONE
  // DEBUG ZONE

  // Check for number of arguments
  if (argc != 3) {
    fprintf(stderr, "The function needs 2 arguments!");
    exit(EXIT_FAILURE);
  }

  FILE *input = fopen(argv[1], "r");
  // check file existance throw error if not found
  if (!input) {
    fprintf(stderr, "The file %s was not found", argv[1]);
    exit(EXIT_FAILURE);
  }

  vector errorVector = constructVector();
  uint32_t instructionsNumber;
  map labelMapping = constructMap();
  /**
  * fill the mappings:
  * map DATA_OPCODE;
  * map ALL_INSTRUCTIONS;
  * map CONDITIONS;
  * And after:
  * make first pass thorugh code and map all labels with their corresponidng
  * memmory addresses (fills labelMapping)
  **/
  fillAll();
  uint32_t linesNumber = firstPass(input, &labelMapping,
                &errorVector, &instructionsNumber);
  /**
  * Make second pass now and replace all labels with their mapping
  * also decode all instructions and trow errors if any
  **/

  uint32_t instructions[instructionsNumber];
  rewind(input); // reset file pointer to the beginning of the file
  secondPass(linesNumber, instructions,
                &errorVector, input, labelMapping);

  // clear
  freeAll();
  clearFullMap(&labelMapping);
  fclose(input);

  // if we have compile erros stop and print errors
  if (!isEmptyVector(errorVector)) {
    while (!isEmptyVector(errorVector)) {
      char *error = getFront(&errorVector);
      fprintf(stderr, "%s\n", error);
      free(error);
    }

    clearFullVector(&errorVector);
    exit(EXIT_FAILURE);
  }

  FILE *output = fopen(argv[2], "wb");
  fwrite(instructions, sizeof(uint32_t), instructionsNumber, output);
  fclose(output);
  clearFullVector(&errorVector);

  exit(EXIT_SUCCESS);
}

uint32_t firstPass(FILE *input, map *labelMapping,
                  vector *errorVector, uint32_t *instructionsNumber) {
  uint32_t lineNumber = 1;
  uint32_t currentMemoryLocation = 0;
  vector currentLabels = constructVector();
  char buffer[MAX_LINE_LENGTH];

  *instructionsNumber = 0;

  while(fgets(buffer, MAX_LINE_LENGTH, input)) {
    vector tokens = tokenise(buffer, DELIMITERS);
    char *lineNo = uintToString(lineNumber);
    // check for all tokens see if there are labels
    // if there are labels add all of them to a vector list and
    // map all labels with the memorry address of the next instruction
    while (!isEmptyVector(tokens)) {
      char *token = getFront(&tokens);
      if (getType(token) == LABEL) {
        token[strlen(token) - 1] = '\0';
        if (get(*labelMapping, token) || contains(currentLabels, token)) {
          // if this label already exists in the mapping this means
          // that we have multiple definitions of the same label
          // therefore throw an error message
          throwLabelError(token, errorVector, lineNo);
          free(token);
        } else {
          putBack(&currentLabels, token);
        }
      } else if (getType(token) == INSTRUCTION) {
        // if we found a valid instruction
        // map all current unmapped labels
        // to this current memmory location
        // and advance memory
        free(token);
        while (!isEmptyVector(currentLabels)) {
          // map all labels to current memorry location
          put(labelMapping, getFront(&currentLabels), currentMemoryLocation);
        }
        currentMemoryLocation++;
        (*instructionsNumber)++;
      } else {
        // token is neither label nor instruction so we have to free it
        free(token);
      }
    }
    free(lineNo);
    lineNumber++;
  }

  // map all remaining unmached labels to current memory location
  while (!isEmptyVector(currentLabels)) {
    // map all labels to current memorry location
    put(labelMapping, getFront(&currentLabels), currentMemoryLocation);
  }

  return lineNumber - 1;
}

void secondPass(uint32_t linesNumber, uint32_t instructions[],
              vector *errorVector, FILE *input, map labelMapping) {
  char buffer[MAX_LINE_LENGTH];
  uint32_t PC = 0;
  uint32_t ln = 1;

  while(fgets(buffer, MAX_LINE_LENGTH, input)) {
    vector tokens = tokenise(buffer, DELIMITERS);
    char *lineNo = uintToString(ln);
    while (!isEmptyVector(tokens)) {
      char *token = peekFront(tokens);
      if (getType(token) == INSTRUCTION) {
        // if there is a valid isntruction decode it and increase
        // instruction counter
        instructions[PC] = decode(&tokens, PC,
                          labelMapping, errorVector, lineNo);
        PC++;
      } else if (getType(token) == LABEL) {
        // we have a label so we just remove it
        free(getFront(&tokens));
      } else {
        // throw error because instruction is undefined
        throwUndeifinedError(token, errorVector, lineNo);
        free(getFront(&tokens));
      }
    }
    free(lineNo);
    ln++;
  }
}

uint32_t decode(vector *tokens, uint32_t instructionNumber,
            map labelMapping, vector *errorVector, char *ln) {
  uint32_t type = *get(ALL_INSTRUCTIONS, peekFront(*tokens));
  switch (type) {
    case 0: return decodeDataProcessing(tokens, errorVector, ln);
    case 1: return decodeMultiply(tokens, errorVector, ln);
    case 2: return decodeSingleDataTransfer(tokens, errorVector, ln);
    case 3: return decodeBranch(tokens, instructionNumber,
                                      labelMapping, errorVector, ln);
    case 4:
    case 5: free(getFront(tokens));
            return 0;
    default: assert(false);
  }
}

uint32_t decodeDataProcessing(vector *tokens,
                        vector *errorVector, char *ln) {
  char *instruction = getFront(tokens);
  uint32_t ins = 0;
  uint32_t opcode = *get(DATA_OPCODE, instruction) << 0x15;
  setCond(&ins, ALWAYS_CONDITION);
  // set opcode
  ins |= opcode;
  uint32_t dataType = *get(DATA_TYPE, instruction);
  uint32_t rd = 0;
  uint32_t rn = 0;
  uint32_t operand2 = 0;
  uint32_t i = 0;
  char *token;
  if (dataType != 2) {
    // we have either 0, 1 type instruction
    token = peekFront(*tokens);
    if (getType(token) != REGISTER) {
      // throw register error
      return -1;
    }
    getFront(tokens);

    rd = getDec(token + 1) << 0xC;
  }

  if (dataType != 1) {
    // we have either 0, 2 type instruction
    token = peekFront(*tokens);
    if (getType(token) != REGISTER) {
      // throw register error
      return -1;
    }
    getFront(tokens);

    rn = getDec(token + 1) << 0x10;
  }

  token = peekFront(*tokens);
  if (getType(token) != EXPRESSION && getType(token) != REGISTER) {
    // throw expression error
    return -1;
  }
  getFront(tokens);

  if (getType(token) == EXPRESSION) {
    // decode expression and set bit i to 1
    operand2 = getExpression(token, errorVector, ln);
    i = 1;
  } else {
    // we have a register
    operand2 = getDec(token + 1);
  }

  if (dataType == 2) {
    // we have third type of instruction
    // with syntax <opcode> Rn, <Operand2>
    // set S bit to 1
    ins |= 0x1 << 0x14;
  }

  // set bit I
  ins |= i << 0x19;
  // set rn
  ins |= rn;
  // set rd register
  ins |= rd;
  // set operand2
  ins |= operand2;

  return ins;
}

uint32_t decodeMultiply(vector *tokens,
                        vector *errorVector, char *ln) {
  char *multType = (char *) getFront(tokens);
  // set bits 4-7, same for mul and mla
  uint32_t instr = 0x9 << 0x4;
  // set cond
  setCond(&instr, ALWAYS_CONDITION);

  uint32_t rd = 0;
  uint32_t rn = 0;
  uint32_t rs = 0;
  uint32_t rm = 0;

  // "mul" instr case
  if(!strcmp(multType, "mul")) {
    if(!checkRegMult(tokens, multType, errorVector, ln)) {
      char *token = (char *) getFront(tokens);
      rd = getDec(token + 1) << 0x10;
    }
    // set rd
    instr |= rd;

    if(!checkRegMult(tokens, multType, errorVector, ln)) {
      char *token = (char *) getFront(tokens);
      rm = getDec(token + 1);
    }
    // set rm
    instr |= rm;

    if(!checkRegMult(tokens, multType, errorVector, ln)) {
      char *token = (char *) getFront(tokens);
      rs = getDec(token + 1) << 0x8;
    }
    // set rs
    instr |= rs;
  }

  // "mla" instr case
  if(!strcmp(multType, "mla")) {
    //set bit 21 (Accumulator)
    instr = 1 << 0x15;

    if(!checkRegMult(tokens, multType, errorVector, ln)) {
      char *token = (char *) getFront(tokens);
      rd = getDec(token + 1) << 0x10;
    }
    // set rd
    instr |= rd;

    if(!checkRegMult(tokens, multType, errorVector, ln)) {
      char *token = (char *) getFront(tokens);
      rm = getDec(token + 1);
    }
    // set rm
    instr |= rm;

    if(!checkRegMult(tokens, multType, errorVector, ln)) {
      char *token = (char *) getFront(tokens);
      rs = getDec(token + 1) << 0x8;
    }
    // set rs
    instr |= rs;

    if(!checkRegMult(tokens, multType, errorVector, ln)) {
      char *token = (char *) getFront(tokens);
      rn = getDec(token + 1) << 0xC;
    }
    // set rn
    instr |= rn;
  }

  return instr;
}

// helper function to check if incoming token is a valid register
int checkRegMult(vector *tokens, char *multType,
                vector *errorVector, char *ln) {
  char *reg = (char *) peekFront(*tokens);
  if(!reg) {
    throwExpressionMissingError(multType, errorVector, ln);
    free(multType);
    return -1;
  }

  if (!isRegister(reg)) {
    //throw Register error
    return -1;
  }
  return 0;
}

uint32_t decodeSingleDataTransfer(vector *tokens,
                        vector *errorVector, char *ln) {
  free(getFront(tokens));
  return 0;
}

uint32_t decodeBranch(vector *tokens, uint32_t instructionNumber,
                        map labelMapping, vector *errorVector, char *ln) {
  char *branch = getFront(tokens);
  uint32_t ins = 0xA << 0x18;
  setCond(&ins, (branch + 1));
  uint32_t *mem;
  uint32_t target;

  char *expression = (char *) peekFront(*tokens);

  if (!expression || getType(expression) == INSTRUCTION) {
    throwExpressionMissingError(branch, errorVector, ln);
    free(branch);
    return -1;
  }

  if ((mem = get(labelMapping, expression))) {
    // we have a mapping
    target = *mem - instructionNumber - PC_OFFSET;
    target <<= INSTRUCTION_SIZE - (BRANCH_OFFSET_SIZE - 2);
    target >>= INSTRUCTION_SIZE - (BRANCH_OFFSET_SIZE - 2);
  }

  ins |= target;
  getFront(tokens);
  free(branch);
  free(expression);
  return ins;
}

void setCond(uint32_t *x, char *cond) {
  uint32_t condition = *get(CONDITIONS, cond) << 0x1C;
  // make space
  *x <<= 4;
  *x >>= 4;
  *x |= condition;
}

void printStringArray(int n, char arr[][MAX_LINE_LENGTH]) {
  for (int i = 0; i < n; i++) {
    printf("%s", arr[i]);
  }
}

vector tokenise(char *start, char *delimiters) {
  int tokenSize = 0;
  int i;
  vector tokens = constructVector();

  for (i = 0; start[i] != '\0'; i++) {
    tokenSize++;

    if (strchr(delimiters, start[i])) {
      // found delimiter add token to vector
      tokenSize--;
      if (tokenSize) {
        // make sapce for token string
        char *token = malloc((tokenSize + 1) * sizeof(char));
        strncpy(token, start + i - tokenSize, tokenSize);
        token[tokenSize] = '\0';
        putBack(&tokens, token);
        tokenSize = 0;
      }
    }
  }

  if (tokenSize) {
    // we still have one token left
    char *token = malloc((tokenSize + 1) * sizeof(char));
    strncpy(token, start + i - tokenSize, tokenSize);
    token[tokenSize] = '\0';
    putBack(&tokens, token);
  }

  return tokens;
}

bool isRegister(char *token) {
  int length  = strlen(token);

  if (length < 2 || length > 3 || token[0] != 'r') {
    return false;
  }

  for (int i = 1; i < length; i++) {
    if (token[i] < '0' || token[i] > '9') {
      return false;
    }
  }

  int number  = atoi(token + 1);
  return number >= 0 && number <= 16;
}

bool isLabel(char *token) {
  return token[strlen(token) - 1] == ':';
}

bool isExpression(char *token) {
  if (!strlen(token)) {
    return false;
  }

  if (token[0] != '#') {
    return false;
  }

  if (strlen(token) >= 4 && token[1] == '0' &&token[2] == 'x') {
    // we might have a hex value
    for (int i = 3; token[i] != '\0'; i++) {
      if ((token[i] < '0' || token[i] > '9') &&
                    (token[i] < 'A' || token[i] > 'F')) {
        return false;
      }
    }
  } else {
    // we might have a decimal value
    for (int i = 1; token[i] != '\0'; i++) {
      if (token[i] < '0' || token[i] > '9') {
        return false;
      }
    }
  }

  return true;
}

bool isInstruction(char *token) {
  return get(ALL_INSTRUCTIONS, token);
}

typeEnum getType(char *token) {
  if (isLabel(token)) {
    return LABEL;
  }

  if (isExpression(token)) {
    return EXPRESSION;
  }

  if (isInstruction(token)) {
    return INSTRUCTION;
  }

  if (isRegister(token)) {
    return REGISTER;
  }

  return UNDEFINED;
}

uint32_t getExpression(char *exp, vector *errorVector, char *ln) {
  assert(getType(exp) == EXPRESSION);

  uint32_t res = 0;
  uint32_t rotations = 0;

  if (exp[1] == '0' && exp[2] == 'x') {
    res = getHex(exp + 3);
  } else {
    res = getDec(exp + 1);
  }

  printBinary(res);

  // check if exp can be roatated to a 8 bit imediate value
  while (res >= 0x100 && rotations <= 30) {
    char bits31_30 = (res & 0xC0000000) >> (INSTRUCTION_SIZE - 2);
    res <<= 2;
    res |= bits31_30;
    rotations += 2;
  }

  if (rotations > 30 || rotations % 2 != 0) {
    // throw an error
    // number can't be represented
    throwExpressionError(exp, errorVector, ln);
    return -1;
  }

  res |= ((rotations / 2) << 0x8);

  return res;
}

uint32_t getHex(char *exp) {
  int res = 0;
  for (int i = 0; exp[i] != '\0'; i++) {
    int x;
    if (exp[i] >= 'A' && exp[i] <= 'F') {
      x = exp[i] - 'A' + 10;
    } else {
      x = exp[i] - '0';
    }
    res <<= 4;
    res |= x;
  }

  return res;
}

uint32_t getDec(char *exp) {
  return atoi(exp);
}

char *uintToString(uint32_t num) {
  int n = num;
  int length = 0;

  do {
    length++;
    n /= 10;
  } while (n != 0);

  char *ret = malloc((length + 1) * sizeof(char));

  for (int i = 0; i < length; i++) {
    ret[length - i - 1] = num % 10 + '0';
    num /= 10;
  }

  ret[length] = '\0';

  return ret;
}

void printBinary(uint32_t nr) {
  uint32_t mask = 1 << (INSTRUCTION_SIZE - 1);

  printf("%d: ", nr);

  for (int i = 0; i < INSTRUCTION_SIZE; i++) {
    if (!(i % 8) && i) {
      putchar(' ');
    }
    putchar((nr & mask) ? '1' : '0');
    mask >>= 1;
  }

  putchar('\n');
}

// ----------------------ERRORS--------------------------------
void throwUndeifinedError(char *name, vector *errorVector, char *ln) {
  char *error = malloc(200);
  strcpy(error, "[");
  strcat(error, ln);
  strcat(error, "] Undefined instruction ");
  strcat(error, name);
  strcat(error, ".");
  putBack(errorVector, error);
}

void throwLabelError(char *name, vector *errorVector, char *ln) {
  char *error = malloc(200);
  strcpy(error, "[");
  strcat(error, ln);
  strcat(error, "] Multiple definitions of the same label: ");
  strcat(error, name);
  strcat(error, ".");
  putBack(errorVector, error);
}

void throwExpressionError(char *name, vector *errorVector, char *ln) {
  char *error = malloc(200);
  strcpy(error, "[");
  strcat(error, ln);
  strcat(error, "] The expression ");
  strcat(error, name);
  strcat(error, " is invalid.");
  putBack(errorVector, error);
}

void throwExpressionMissingError(char *ins, vector *errorVector, char *ln) {
  char *error = malloc(200);
  strcpy(error, "[");
  strcat(error, ln);
  strcat(error, "] The expression is missing from the ");
  strcat(error, ins);
  strcat(error, " instruction.");
  putBack(errorVector, error);
}

// ---------------------ADT TESTS------------------------------
/*vector x = constructVector();

putFront(&x, "Hello1");
putFront(&x, "Hello2");
putFront(&x, "Hello3");
printVector(x);
clearVector(&x);
printVector(x);

map m = constructMap();

put(&m, "string1", 20);
put(&m, "string3", 15);
put(&m, "string1", 15);
put(&m, "string2", 28);

printMap(m, 0);
clearMap(&m);
printMap(m, 0);*/
