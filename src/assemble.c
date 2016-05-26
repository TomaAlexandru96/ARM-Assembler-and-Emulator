#include "mappings.h"

#define MAX_LINE_LENGTH 512
#define DELIMITERS " ,\n"
#define MEMORY_SIZE 4
#define INSTRUCTION_SIZE 32
#define ALWAYS_CONDITION ""

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
uint32_t decode(vector *tokens,
                map labelMapping, vector *errorVector, char *ln);
uint32_t decodeDataProcessing(vector *tokens, vector *errorVector, char *ln);
uint32_t decodeMultiply(vector *tokens, vector *errorVector, char *ln);
uint32_t decodeSingleDataTransfer(vector *tokens, vector *errorVector,
                                  char *ln);
uint32_t decodeBranch(vector *tokens, map labelMapping,
                      vector *errorVector, char *ln);
void throwUndeifinedError(char *name, vector *errorVector, char *ln);
void throwLabelError(char *name, vector *errorVector, char *ln);
void throwExpressionError(vector *errorVector, char *ln);
void throwExpressionMissingError(char *ins, vector *errorVector, char *ln);
bool isExpression(char *token);
bool isInstruction(char *token);
bool isLabel(char *token);
typeEnum getType(char *token);
uint32_t getExpression(char *exp);
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
        currentMemoryLocation += MEMORY_SIZE;
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
        instructions[PC] = decode(&tokens, labelMapping, errorVector, lineNo);
        printBinary(instructions[PC]);
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

uint32_t decode(vector *tokens,
            map labelMapping, vector *errorVector, char *ln) {
  uint32_t type = *get(ALL_INSTRUCTIONS, peekFront(*tokens));
  switch (type) {
    case 0: return decodeDataProcessing(tokens, errorVector, ln);
    case 1: return decodeMultiply(tokens, errorVector, ln);
    case 2: return decodeSingleDataTransfer(tokens, errorVector, ln);
    case 3: return decodeBranch(tokens, labelMapping, errorVector, ln);
    case 4:
    case 5: free(getFront(tokens));
            return 0;
    default: assert(false);
  }
}

uint32_t decodeDataProcessing(vector *tokens,
                        vector *errorVector, char *ln) {
  free(getFront(tokens));
  return 0;
}

uint32_t decodeMultiply(vector *tokens,
                        vector *errorVector, char *ln) {
  free(getFront(tokens));
  return 0;
}

uint32_t decodeSingleDataTransfer(vector *tokens,
                        vector *errorVector, char *ln) {
  free(getFront(tokens));
  return 0;
}

uint32_t decodeBranch(vector *tokens, map labelMapping,
                        vector *errorVector, char *ln) {
  char *branch = (char *) getFront(tokens);
  uint32_t ins = 0xA << 0x18;
  setCond(&ins, (char *) (branch + 1));
  uint32_t *mem;
  uint32_t target;

  char *expression = (char *) peekFront(*tokens);

  if (!expression) {
    throwExpressionMissingError(branch, errorVector, ln);
    free(branch);
    return -1;
  }

  if ((mem = get(labelMapping, expression))) {
    // we have a mapping
    target = *mem;
  } else {
    // we have an offset
    if (getType(expression) != EXPRESSION) {
      throwExpressionError(errorVector, ln);
      free(branch);
      return -1;
    }
    target = 0;
  }
  ins |= target;

  getFront(tokens);
  free(branch);
  free(expression);
  return ins;
}

void setCond(uint32_t *x, char *cond) {
  uint32_t condition = *get(CONDITIONS, cond) << 0x1B;
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

  return UNDEFINED;
}

uint32_t getExpression(char *exp) {
  if (strlen(exp) >= 3 && exp[0] == '0' && exp[1] == 'x') {
    return getHex(exp + 2);
  } else {
    return getDec(exp);
  }
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

void throwExpressionError(vector *errorVector, char *ln) {
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
