#define NDEBUG
#include "mappings.h"

#define MAX_LINE_LENGTH 512
#define DELIMITERS " ,\n"
#define MEMORY_SIZE 4
#define PC_OFFSET 2
#define INSTRUCTION_SIZE 32
#define ALWAYS_CONDITION ""
#define BRANCH_OFFSET_SIZE  26
#define NUMBER_OF_LINES 1000

char **firstPass(FILE *input, map *labelMapping, vector *errorVector,
        uint32_t *instructionsNumber, uint32_t *ldrCount, uint32_t *lineNumber);
void secondPass(uint32_t *instructionsNumber, uint32_t instructions[],
              vector *errorVector, FILE *input, map labelMapping,
              char** linesFromFile, uint32_t lineNumber);
void printStringArray(int n, char arr[][MAX_LINE_LENGTH]);
vector tokenise(char *start, char *delimiters);
void printBinary(uint32_t nr);
void setCond(uint32_t *x, char *cond);
void clearLinesFromFile(char **linesFromFile);
/**
* Converts unsigned int to string
**/
char *uintToString(uint32_t num);
uint32_t decode(vector *tokens, vector *addresses, uint32_t instructionNumber,
  uint32_t instructionsNumber, map labelMapping, vector *errorVector, char *ln);
uint32_t decodeDataProcessing(vector *tokens, vector *errorVector, char *ln);
uint32_t decodeMultiply(vector *tokens, vector *errorVector, char *ln);
bool checkRegMult(vector *tokens, char *multType,
                vector *errorVector, char *ln);
uint32_t decodeSingleDataTransfer(vector *tokens, vector *addresses,
                uint32_t instructionNumber, uint32_t instructionsNumber,
                vector *errorVector, char *ln);
uint32_t decodeBranch(vector *tokens, uint32_t instructionNumber,
                map labelMapping, vector *errorVector, char *ln);
void throwUndefinedError(char *name, vector *errorVector, char *ln);
void throwLabelError(char *name, vector *errorVector, char *ln);
void throwExpressionError(char *expression, vector *errorVector, char *ln);
void throwRegisterError(char *name, vector *errorVector, char *ln);
void throwExpressionMissingError(char *ins, vector *errorVector, char *ln);
typeEnum isExpression(char *token);
bool isInstruction(char *token);
bool isLabel(char *token);
bool isRegister(char *token);
typeEnum getType(char *token);
void getBracketExpr(vector *tokens, int *rn, int32_t *offset, int *i, int *u);
int32_t getExpression(char *exp, vector *errorVector, char *ln);
int32_t getHex(char *exp);
int32_t getDec(char *exp);

int main(int argc, char **argv) {
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
  * fill the mappings
  * And after:
  * make first pass thorugh code and map all labels with their corresponidng
  * memmory addresses (fills labelMapping)
  **/
  fillAll();
  uint32_t ldrCount = 0;
  uint32_t lineNumber = 1;
  char **linesFromFile;
  linesFromFile = firstPass(input, &labelMapping, &errorVector,
                  &instructionsNumber, &ldrCount, &lineNumber);
  // Have checked not NULL condition in firstPass function
  /**
  * Make second pass now and replace all labels with their mapping
  * also decode all instructions and throw errors if any
  **/
  uint32_t instructions[instructionsNumber + ldrCount];
  rewind(input); // reset file pointer to the beginning of the file
  secondPass(&instructionsNumber, instructions,
             &errorVector, input, labelMapping, linesFromFile, lineNumber);

  // clear
  freeAll();
  clearMap(&labelMapping);
  fclose(input);

  // if we have compile erros stop and print errors
  if (!isEmptyVector(errorVector)) {
    while (!isEmptyVector(errorVector)) {
      char *error = getFront(&errorVector);
      fprintf(stderr, "%s\n", error);
      free(error);
    }

    clearVector(&errorVector);
    exit(EXIT_FAILURE);
  }

  FILE *output = fopen(argv[2], "wb");
  fwrite(instructions, sizeof(uint32_t), instructionsNumber, output);
  fclose(output);
  clearVector(&errorVector);
  clearLinesFromFile(linesFromFile);
  exit(EXIT_SUCCESS);
}

char **firstPass(FILE *input, map *labelMapping, vector *errorVector,
      uint32_t *instructionsNumber, uint32_t *ldrCount, uint32_t *lineNumber) {
  uint32_t currentMemoryLocation = 0;
  *ldrCount = 0;
  vector currentLabels = constructVector();
  char buffer[MAX_LINE_LENGTH];
  /*Allocate memory for the array of lines*/
  char **linesFromFile = (char **) malloc(NUMBER_OF_LINES * sizeof(char *));
  for(int i = 0; i < NUMBER_OF_LINES; i++) {
    linesFromFile[i] = (char *) malloc((MAX_LINE_LENGTH + 1) * sizeof(char));
    if(!linesFromFile) {
      perror("malloc");
      exit(EXIT_FAILURE);
    }
  }
  if(!linesFromFile) {
    perror("malloc");
    exit(EXIT_FAILURE);
  }
  /*End memory allocation for array of lines*/
  *instructionsNumber = 0;

  while(fgets(buffer, MAX_LINE_LENGTH, input)) {
    /*Perform dynamic expansion of lines array*/
    if((*lineNumber - 1) >= NUMBER_OF_LINES) {
     countDynamicExpansions++;
     //increment global variable countDynamicExpansions
     char **reallocatedArray =
     realloc(linesFromFile,
             countDynamicExpansions * NUMBER_OF_LINES * sizeof(char *));
             printf("%s\n", "I reallocated");
     if(reallocatedArray) {
        linesFromFile = reallocatedArray;
     } else {
       perror("realloc");
       exit(EXIT_FAILURE);
     }
    }
    /*End dynamic expansion*/
    strcpy(linesFromFile[*lineNumber - 1], buffer);
    vector tokens = tokenise(buffer, DELIMITERS);
    char *lineNo = uintToString(*lineNumber);
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
        } else {
          putBack(&currentLabels, token);
        }
      }

      if (getType(token) == INSTRUCTION) {
        // if the instruction is a ldr instruction and the <=expression>
        // is more than 0xFF we need to store the value at the bottom of the
        // binary file
        // we will assume that the argument is more than 0xFF and if it is
        // not then we will just not use the remaining space
        if (!strcmp(token, "ldr")) {
          (*ldrCount)++;
        }

        // if we found a valid instruction
        // map all current unmapped labels
        // to this current memmory location
        // and advance memory
        while (!isEmptyVector(currentLabels)) {
          // map all labels to current memorry location
          char *label = getFront(&currentLabels);
          put(labelMapping, label, currentMemoryLocation);
          free(label);
        }
        currentMemoryLocation++;
        (*instructionsNumber)++;
      }
      free(token);
    }
    free(lineNo);
    (*lineNumber)++;
  }

  // map all remaining unmached labels to current memory location
  while (!isEmptyVector(currentLabels)) {
    // map all labels to current memorry location
    put(labelMapping, getFront(&currentLabels), currentMemoryLocation);
  }
  return linesFromFile;
}

void secondPass(uint32_t *instructionsNumber, uint32_t instructions[],
              vector *errorVector, FILE *input, map labelMapping,
              char **linesFromFile, uint32_t lineNumber) {
  //char buffer[MAX_LINE_LENGTH];
  uint32_t PC = 0;
  uint32_t ln = 1;
  vector addresses = constructVector();
  //fgets(buffer, MAX_LINE_LENGTH, input) --> was in the loop
  while(ln != lineNumber) {
    vector tokens = tokenise(linesFromFile[ln - 1], DELIMITERS);
    char *lineNo = uintToString(ln);
    while (!isEmptyVector(tokens)) {
      char *token = peekFront(tokens);
      if (getType(token) == INSTRUCTION) {
        // if there is a valid isntruction decode it and increase
        // instruction counter
        instructions[PC] = decode(&tokens, &addresses, PC, *instructionsNumber,
                      labelMapping, errorVector, lineNo);
        PC++;
      } else if (getType(token) == LABEL) {
        // we have a label so we just remove it
        free(getFront(&tokens));
      } else {
        throwUndefinedError(token, errorVector, lineNo);
        // throw error because instruction is undefined
        free(getFront(&tokens));
      }
    }
    ln++;
    free(lineNo);
  }

  // put all ldr addresses > 0xFF at the end of the file
  while (!isEmptyVector(addresses)) {
    char *address = getFront(&addresses);
    instructions[PC] = getExpression(address, NULL, 0);
    free(address);
    PC++;
  }

  *instructionsNumber = PC;
}

uint32_t decode(vector *tokens, vector *addresses, uint32_t instructionNumber,
                uint32_t instructionsNumber, map labelMapping,
                vector *errorVector, char *ln) {
  uint32_t type = *get(ALL_INSTRUCTIONS, peekFront(*tokens));
  switch (type) {
    case 0: return decodeDataProcessing(tokens, errorVector, ln);
    case 1: return decodeMultiply(tokens, errorVector, ln);
    case 2: return decodeSingleDataTransfer(tokens, addresses,
                        instructionNumber, instructionsNumber, errorVector, ln);
    case 3: return decodeBranch(tokens, instructionNumber,
                                      labelMapping, errorVector, ln);
    case 4:
    case 5: free(getFront(tokens));
            free(getFront(tokens));
            free(getFront(tokens));
            free(getFront(tokens));
            return 0;
    default: //assert(false);
             free(getFront(tokens));
             return -1;
  }
}

uint32_t decodeDataProcessing(vector *tokens, vector *errorVector, char *ln) {
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
      free(instruction);
      return -1;
    }

    rd = getDec(token + 1) << 0xC;
    free(getFront(tokens));
  }

  if (dataType != 1) {
    // we have either 0, 2 type instruction
    token = peekFront(*tokens);
    if (getType(token) != REGISTER) {
      // throw register error
      free(instruction);
      return -1;
    }

    rn = getDec(token + 1) << 0x10;
    free(getFront(tokens));
  }

  token = peekFront(*tokens);
  if (getType(token) != EXPRESSION_TAG &&
      getType(token) != EXPRESSION_EQUAL &&
            getType(token) != REGISTER) {
    // throw expression error
    free(instruction);
    return -1;
  }

  if (getType(token) == EXPRESSION_TAG || getType(token) == EXPRESSION_EQUAL) {
    // decode expression and set bit i to 1
    operand2 = getExpression(token, errorVector, ln);
    i = 1;
  } else {
    // we have a register
    operand2 = getDec(token + 1);
  }
  free(getFront(tokens));

  if (dataType == 2) {
    // we have third type of instruction
    // with syntax <opcode> Rn, <Operand2>
    // set S bit to 1
    ins |= 0x1 << 0x14;
  }
  free(instruction);
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

uint32_t decodeMultiply(vector *tokens, vector *errorVector, char *ln) {
  char *multType = (char *) getFront(tokens);
  // set bits 4-7, same for mul and mla
  uint32_t instr = 0x9 << 0x4;
  // set cond
  setCond(&instr, ALWAYS_CONDITION);

  uint32_t acc = 0;
  uint32_t rd = 0;
  uint32_t rm = 0;
  uint32_t rs = 0;
  uint32_t rn = 0;

  // "mul"/"mla" set common registers (rd, rm, rs)
  if(checkRegMult(tokens, multType, errorVector, ln)) {
    char *token = (char *) getFront(tokens);
    rd = getDec(token + 1) << 0x10;
    free(token);
  }

  if(checkRegMult(tokens, multType, errorVector, ln)) {
    char *token = (char *) getFront(tokens);
    rm = getDec(token + 1);
    free(token);
  }

  if(checkRegMult(tokens, multType, errorVector, ln)) {
    char *token = (char *) getFront(tokens);
    rs = getDec(token + 1) << 0x8;
    free(token);
  }

 // "mla" instr case
  if(!strcmp(multType, "mla")) {
    //set bit 21 (Accumulator)
    acc = 0x1 << 0x15;

    if(checkRegMult(tokens, multType, errorVector, ln)) {
      char *token = (char *) getFront(tokens);
      rn = getDec(token + 1) << 0xC;
      free(token);
    }
  }

  // set acc
  instr |= acc;
  // set rd
  instr |= rd;
  // set rm
  instr |= rm;
  // set rs
  instr |= rs;
  // set rn
  instr |= rn;

  return instr;
}

// helper function to check if incoming token is a valid register
bool checkRegMult(vector *tokens, char *multType,
                vector *errorVector, char *ln) {
  char *reg = (char *) peekFront(*tokens);
  if(!reg) {
    throwExpressionMissingError(multType, errorVector, ln);
    free(multType);
    return false;
  }

  if (getType(reg) != REGISTER) {
    throwRegisterError(reg, errorVector, ln);
    return false;
  }
  return true;
}

void getBracketExpr(vector *tokens, int *rn, int32_t *offset, int *i, int *u) {
  char *token;
  int tokenSize = 0;
  vector bracketExpr = constructVector();

  // remove the square brackets from the existing tokens
  do {
    token = getFront(tokens);
    tokenSize = strlen(token);
    char *newToken = token;

    if (token[0] == '[') {
      newToken++;
    }

    if (token[tokenSize - 1] == ']') {
      token[tokenSize - 1] = '\0';
      if (strlen(token)) {
        // token not empty
        putBack(&bracketExpr, newToken);
      }
      free(token);
      break;
    }

    putBack(&bracketExpr, newToken);

    free(token);
  } while (!isEmptyVector(*tokens));

  token = getFront(&bracketExpr);
  *rn = getDec(token + 1);
  free(token);

  token = getFront(&bracketExpr);

  if (token) {
    if (!strcmp(token, "-")) {
      u = 0;
      free(token);
      token = getFront(&bracketExpr);
    } else if (token[0] == '-') {
      token++;
      u = 0;
    }

    if (getType(token) == EXPRESSION_TAG) {
      *offset = getExpression(token, NULL, 0);
    } else if (getType(token) == REGISTER) {
      *offset = getDec(token + 1);
      *i = 1;
    }
  }

  free(token);
}

uint32_t decodeSingleDataTransfer(vector *tokens, vector *addresses,
                uint32_t instructionNumber, uint32_t instructionsNumber,
                vector *errorVector, char *ln) {
  char *instruction = getFront(tokens);
  char *rdName;
  uint32_t ins = 1 << 0x1A;
  int i = 0;
  int p = 1;
  int u = 1;
  int l = !strcmp(instruction, "ldr") ? 1 : 0;
  int32_t offset = 0;
  int rn = 0xf; // default rn
  int rd = 0;
  char *token;

  setCond(&ins, ALWAYS_CONDITION);

  token = peekFront(*tokens);
  if (getType(token) != REGISTER) {
    // throw register error
    free(instruction);
    return -1;
  }

  rd = getDec(token + 1);
  rdName = getFront(tokens);

  // check for register or expression
  token = peekFront(*tokens);
  if (token && token[0] == '[') {
    // we have indexed address
    getBracketExpr(tokens, &rn, &offset, &i, &u);

    token = peekFront(*tokens);

    if (token) {
      if (!strcmp(token, "-")) {
        u = 0;
        free(token);
        token = getFront(tokens);
      } else if (token[0] == '-') {
        token++;
        u = 0;
      }

      if (getType(token) == EXPRESSION_TAG) {
        // we have post indexed expression
        token = getFront(tokens);
        offset = getExpression(token, NULL, 0);
        free(token);
        p = 0;
      } else if (getType(token) == REGISTER) {
        // we have post indexed register
        token = getFront(tokens);
        offset = getDec(token + 1);
        free(token);
        p = 0;
        i = 1;
      }
    }

    if (offset < 0) {
      offset = abs(offset);
      u = 0;
    }
  } else {
    p = 1;
    if (getType(token) != EXPRESSION_EQUAL) {
      // throw expression error
      free(rdName);
      free(instruction);
      return -1;
    }

    if (!strcmp(instruction, "ldr")) {
      // we have a load instruction
      if (getType(token) == EXPRESSION_EQUAL) {
        uint32_t address = getExpression(token, errorVector, ln);

        if (address <= 0xFF) {
          // interpret as move instruction
          putFront(tokens, rdName);
          putFront(tokens, "mov");
          free(rdName);
          free(instruction);
          return decodeDataProcessing(tokens, errorVector, ln);
        } else {
          // interpret as normal
          putBack(addresses, token);
          int addressLocation = instructionsNumber + addresses->size - 1;
          offset = (addressLocation - instructionNumber - 2) * MEMORY_SIZE;
        }
      }
      free(getFront(tokens));
    }
  }

  // set bit i 25
  ins |= i << 0x19;
  // set bit p 24
  ins |= p << 0x18;
  // set bit u 23
  ins |= u << 0x17;
  // set bit p 20
  ins |= l << 0x14;
  // set rn
  ins |= rn << 0x10;
  // set rd
  ins |= rd << 0xC;
  // set offset
  ins |= offset;

  free(rdName);
  free(instruction);
  return ins;
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
        free(token);
      }
    }
  }

  if (tokenSize) {
    // we still have one token left
    char *token = malloc((tokenSize + 1) * sizeof(char));
    strncpy(token, start + i - tokenSize, tokenSize);
    token[tokenSize] = '\0';
    putBack(&tokens, token);
    free(token);
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

typeEnum isExpression(char *token) {
  if (!strlen(token)) {
    return UNDEFINED;
  }

  if (token[0] != '#' && token[0] != '=') {
    return UNDEFINED;
  }

  int i = 1;
  if (token[i] == '-') {
    i++;
  }

  if (strlen(token) >= i + 3 && token[i] == '0' &&token[i + 1] == 'x') {
    // we might have a hex value
    i += 2;

    for (; token[i] != '\0'; i++) {
      if ((token[i] < '0' || token[i] > '9') &&
          (token[i] < 'a' || token[i] > 'f') &&
          (token[i] < 'A' || token[i] > 'F')) {
        return UNDEFINED;
      }
    }
  } else {
    // we might have a decimal value

    if (token[i] == '\0') {
      return UNDEFINED;
    }

    for (int i = 1; token[i] != '\0'; i++) {
      if (token[i] < '0' || token[i] > '9') {
        return UNDEFINED;
      }
    }
  }

  return token[0] == '#' ? EXPRESSION_TAG : EXPRESSION_EQUAL;
}

bool isInstruction(char *token) {
  return get(ALL_INSTRUCTIONS, token);
}

typeEnum getType(char *token) {
  if (!token) {
    return UNDEFINED;
  }

  if (isLabel(token)) {
    return LABEL;
  }

  if (isExpression(token) != UNDEFINED) {
    return isExpression(token);
  }

  if (isInstruction(token)) {
    return INSTRUCTION;
  }

  if (isRegister(token)) {
    return REGISTER;
  }

  return UNDEFINED;
}

int32_t getExpression(char *exp, vector *errorVector, char *ln) {
  uint32_t res = 0;
  uint32_t rotations = 0;

  int i = 1;

  if (exp[i] == '-') {
    i++;
  }

  if (exp[i] == '0' && exp[i + 1] == 'x') {
    res = getHex(exp + 1);
  } else {
    res = getDec(exp + 1);
  }

  if (exp[0] == '#') {
    // check if exp can be roatated to a 8 bit imediate value
    while (abs(res) >= 0x100 && rotations <= 30) {
      char bits31_30 = (res & 0xC0000000) >> (INSTRUCTION_SIZE - 2);
      res <<= 2;
      res |= bits31_30;
      rotations += 2;
    }

    if (rotations > 30) {
      // throw an error
      // number can't be represented
      throwExpressionError(exp, errorVector, ln);
      return -1;
    }

    res |= ((rotations / 2) << 0x8);
  }

  return res;
}

int32_t getHex(char *exp) {
  int32_t res = 0;
  bool negative = false;
  if (exp[0] == '-') {
    negative = true;
    exp++;
  }
  exp += 2;
  for (int i = 0; exp[i] != '\0'; i++) {
    int x;
    if (exp[i] >= 'A' && exp[i] <= 'F') {
      x = exp[i] - 'A' + 10;
    } else if (exp[i] >= 'a' && exp[i] <= 'f') {
      x = exp[i] - 'a' + 10;
    } else {
      x = exp[i] - '0';
    }
    res <<= 4;
    res |= x;
  }

  return negative ? -res : res;
}

int32_t getDec(char *exp) {
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


void clearLinesFromFile(char **linesFromFile) {
  for(int i = 0; i < (countDynamicExpansions * NUMBER_OF_LINES); i++) {
    free(linesFromFile[i]);
  }
  free(linesFromFile);
}

// ----------------------ERRORS--------------------------------
void throwUndefinedError(char *name, vector *errorVector, char *ln) {
  char *error = malloc(200);
  strcpy(error, "[");
  strcat(error, ln);
  strcat(error, "] Undefined instruction ");
  strcat(error, name);
  strcat(error, ".");
  putBack(errorVector, error);
  free(error);
}

void throwLabelError(char *name, vector *errorVector, char *ln) {
  char *error = malloc(200);
  strcpy(error, "[");
  strcat(error, ln);
  strcat(error, "] Multiple definitions of the same label: ");
  strcat(error, name);
  strcat(error, ".");
  putBack(errorVector, error);
  free(error);
}

void throwExpressionError(char *name, vector *errorVector, char *ln) {
  char *error = malloc(200);
  strcpy(error, "[");
  strcat(error, ln);
  strcat(error, "] The expression ");
  strcat(error, name);
  strcat(error, " is invalid.");
  putBack(errorVector, error);
  free(error);
}

void throwRegisterError(char *name, vector *errorVector, char *ln) {
  char *error = malloc(200);
  strcpy(error, "[");
  strcat(error, ln);
  strcat(error, "] The register ");
  strcat(error, name);
  strcat(error, " is invalid.");
  putBack(errorVector, error);
  free(error);
}

void throwExpressionMissingError(char *ins, vector *errorVector, char *ln) {
  char *error = malloc(200);
  strcpy(error, "[");
  strcat(error, ln);
  strcat(error, "] The expression is missing from the ");
  strcat(error, ins);
  strcat(error, " instruction.");
  putBack(errorVector, error);
  free(error);
}
