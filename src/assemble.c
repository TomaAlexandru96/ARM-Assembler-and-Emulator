#include "mappings.h"

#define MAX_LINE_LENGTH 512
#define DELIMITERS " ,\n"
#define MEMORY_SIZE 4
#define INSTRUCTION_SIZE 32

uint32_t firstPass(FILE *input, map *labelMapping,
              char errorMessage[], uint32_t *instructionsNumber);
void secondPass(uint32_t linesNumber, uint32_t instructions[],
              char errorMessage[], FILE *input, map labelMapping);
void printStringArray(int n, char arr[][MAX_LINE_LENGTH]);
vector tokenise(char *start, const char *delimiters);
bool isLabel(const char *token);
void printBinary(uint32_t nr);
void setCond(uint32_t *x, char *cond);
/**
* Converts unsigned int to string
**/
char *uintToString(uint32_t num);
uint32_t decode(int type, vector *tokens,
                      map labelMapping, char errorMessage[], uint32_t ln);
uint32_t decodeDataProcessing(vector *tokens, char errorMessage[], uint32_t ln);
uint32_t decodeMultiply(vector *tokens, char errorMessage[], uint32_t ln);
uint32_t decodeSingleDataTransfer(vector *tokens, char errorMessage[],
                                  uint32_t ln);
uint32_t decodeBranch(vector *tokens, map labelMapping,
                      char errorMessage[], uint32_t ln);

int main(int argc, char **argv) {
  // Check for number of arguments
  if (argc != 3) {
    fprintf(stderr, "The function needs 2 arguments!");
    return EXIT_FAILURE;
  }

  FILE *input = fopen(argv[1], "r");
  // check file existance throw error if not found
  if (!input) {
    fprintf(stderr, "The file %s was not found", argv[1]);
    return EXIT_FAILURE;
  }

  char *errorMessage = malloc(0);
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
                errorMessage, &instructionsNumber);
  /**
  * Make second pass now and replace all labels with their mapping
  * also decode all instructions and trow errors if any
  **/
  uint32_t instructions[instructionsNumber];
  rewind(input); // reset file pointer to the beginning of the file
  secondPass(linesNumber, instructions,
                errorMessage, input, labelMapping);

  // if we have compile erros stop and print errors
  if (errorMessage[0] != '\0') {
    fprintf(stderr, "%s\n", errorMessage);
    return EXIT_FAILURE;
  }

  FILE *output = fopen(argv[2], "wb");
  fwrite(instructions, sizeof(uint32_t), instructionsNumber, output);
  fclose(input);
  fclose(output);

  // DEBUG ZONE
  // DEBUG ZONE

  return EXIT_SUCCESS;
}

uint32_t firstPass(FILE *input, map *labelMapping,
                  char errorMessage[], uint32_t *instructionsNumber) {
  uint32_t lineNumber = 1;
  uint32_t currentMemoryLocation = 0;
  vector currentLabels = constructVector();
  char buffer[MAX_LINE_LENGTH];

  *instructionsNumber = 0;

  while(fgets(buffer, MAX_LINE_LENGTH, input)) {
    vector tokens = tokenise(buffer, DELIMITERS);
    // check for all tokens see if there are labels
    // if there are labels add all of them to a vector list and
    // map all labels with the memorry address of the next instruction
    while (!isEmptyVector(tokens)) {
      const char *token = (const char *) getFront(&tokens);
      if (isLabel(token)) {
        if (get(*labelMapping, token) || contains(currentLabels, token)) {
          // if this label already exists in the mapping this means
          // that we have multiple definitions of the same label
          // therefore throw an error message
          char error[] = "[";
          strcat(error, uintToString(lineNumber));
          // strcat(error, "] Multiple definitions of the same label: ");
          strcat(error, token);
          strcat(error, "\n");
          strcat(errorMessage, error);
        }
        putBack(&currentLabels, token);
      }
      if (get(ALL_INSTRUCTIONS, token)) {
        // if we found a valid instruction
        // map all current unmapped labels
        // to this current memmory location
        // and advance memory
        while (!isEmptyVector(currentLabels)) {
          // map all labels to current memorry location
          put(labelMapping, getFront(&currentLabels), currentMemoryLocation);
        }
        currentMemoryLocation += MEMORY_SIZE;
        (*instructionsNumber)++;
      }
    }
    clearVector(&tokens);
    lineNumber++;
  }

  // map all remaining unmached labels to current memory location
  while (!isEmptyVector(currentLabels)) {
    // map all labels to current memorry location
    put(labelMapping, getFront(&currentLabels), currentMemoryLocation);
  }

  clearVector(&currentLabels);
  return lineNumber - 1;
}

void secondPass(uint32_t linesNumber, uint32_t instructions[],
              char errorMessage[], FILE *input, map labelMapping) {
  char buffer[MAX_LINE_LENGTH];
  uint32_t PC = 0;
  uint32_t ln = 1;

  while(fgets(buffer, MAX_LINE_LENGTH, input)) {
    printf("%s", buffer);

    vector tokens = tokenise(buffer, DELIMITERS);

    while (!isEmptyVector(tokens)) {
      const char *token = (const char *) peekFront(tokens);
      uint32_t *pType;
      if ((pType = get(ALL_INSTRUCTIONS, token))) {
        // if there is a valid isntruction decode it and increase
        // instruction counter
        instructions[PC] = decode(*pType, &tokens,
                                  labelMapping, errorMessage, ln);
        printBinary(instructions[PC]);
        PC++;
      } else if (!isLabel(token)) {
        // throw error because instruction is undefined
        char error[] = "[";
        strcat(error, uintToString(ln));
        // strcat(error, "] Undefined instruction: ");
        strcat(error, token);
        strcat(error, "\n");
        strcat(errorMessage, error);
        getFront(&tokens);
      } else {
        // we have a label
        getFront(&tokens);
      }
    }

    ln++;
  }
}

uint32_t decode(int type, vector *tokens,
            map labelMapping, char errorMessage[], uint32_t ln) {
  switch (type) {
    case 0: return decodeDataProcessing(tokens, errorMessage, ln);
    case 1: return decodeMultiply(tokens, errorMessage, ln);
    case 2: return decodeSingleDataTransfer(tokens, errorMessage, ln);
    case 3: return decodeBranch(tokens, labelMapping, errorMessage, ln);
    case 4:
    case 5: getFront(tokens);
            return 0;
    default: assert(false);
  }
}

uint32_t decodeDataProcessing(vector *tokens,
                        char errorMessage[], uint32_t ln) {
  getFront(tokens);
  return 0;
}

uint32_t decodeMultiply(vector *tokens,
                        char errorMessage[], uint32_t ln) {
  getFront(tokens);
  return 0;
}

uint32_t decodeSingleDataTransfer(vector *tokens,
                        char errorMessage[], uint32_t ln) {
  getFront(tokens);
  return 0;
}

uint32_t decodeBranch(vector *tokens, map labelMapping,
                        char errorMessage[], uint32_t ln) {
  const char *branch = getFront(tokens);
  uint32_t ins = 0xA << 0x18;
  setCond(&ins, (char *) (branch + 1));
  uint32_t *mem;
  uint32_t target;

  const char *expression = getFront(tokens);

  if (!expression) {
    char error[] = "[";
    strcat(error, uintToString(ln));
    // strcat(error, "] The expression is missing from the branch instruction.\n");
    strcat(errorMessage, error);
    return -1;
  }

  if ((mem = get(labelMapping, branch))) {
    // we have a mapping
    target = *mem;
  } else {
    // we have an offset
    target = 0;
  }

  ins |= target;

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

bool isLabel(const char *token) {
  return token[strlen(token) - 1] == ':';
}

vector tokenise(char *start, const char *delimiters) {
  int tokenSize = 0;
  int i;
  vector tokens = constructVector();

  for (i = 0; start[i] != '\0'; i++) {
    tokenSize++;

    if (strchr(delimiters, start[i])) {
      // found delimiter add token to vector
      tokenSize--;
      // make sapce for token string
      char *token = malloc((tokenSize + 1) * sizeof(char));
      strncpy(token, start + i - tokenSize, tokenSize);
      token[tokenSize] = '\0';
      putBack(&tokens, token);
      tokenSize = 0;
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
