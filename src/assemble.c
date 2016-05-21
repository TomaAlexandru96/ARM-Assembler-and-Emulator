#include "mappings.h"

#define MAX_LINE_LENGTH 512
#define DELIMITERS " ,\n"
#define MEMORY_SIZE 4

uint32_t firstPass(FILE *input, map *labelMapping,
              char *errorMessage, uint32_t *instructionsNumber);
void secondPass(uint32_t linesNumber, uint32_t instructions[],
              char *errorMessage, FILE *input, map labelMapping);
void printStringArray(int n, char arr[][MAX_LINE_LENGTH]);
vector tokenise(char *start, const char *delimiters);
bool isLabel(char *token);
char *replaceString(char *original, map m);
/**
* Converts unsigned int to string
**/
char *uintToString(uint32_t num);
uint32_t decode();
uint32_t decodeDataProcessing();
uint32_t decodeMultiply();
uint32_t decodeSingleDataTransfer();
uint32_t decodeBranch();

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

  char errorMessage[] = "";
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
  printMap(labelMapping);
  // DEBUG ZONE

  return EXIT_SUCCESS;
}

uint32_t firstPass(FILE *input, map *labelMapping,
                  char *errorMessage, uint32_t *instructionsNumber) {
  uint32_t lineNumber = 1;
  uint32_t currentMemoryLocation = 0;
  vector currentLabels = constructVector();
  char buffer[MAX_LINE_LENGTH];

  *instructionsNumber = 0;

  while(fgets(buffer, MAX_LINE_LENGTH, input)) {
    vector tokens = tokenise(buffer, DELIMITERS);
    printVector(tokens);
    // check for all tokens see if there are labels
    // if there are labels add all of them to a vector list and
    // map all labels with the memorry address of the next instruction
    while (!isEmptyVector(tokens)) {
      char *token = (char *) getFront(&tokens);
      if (isLabel(token)) {
        if (get(*labelMapping, token) || contains(currentLabels, token)) {
          // if this label already exists in the mapping this means
          // that we have multiple definitions of the same label
          // therefore throw an error message
          char error[] = "Multiple definitions of the same label: ";
          strcat(error, token);
          strcat(error, "\n");
          // strcat(errorMessage, error);
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
  clearVector(&currentLabels);
  return lineNumber - 1;
}

void secondPass(uint32_t linesNumber, uint32_t instructions[],
              char *errorMessage, FILE *input, map labelMapping) {
  char buffer[MAX_LINE_LENGTH];
  int i = 0;

  while(fgets(buffer, MAX_LINE_LENGTH, input)) {
    // printf("Original: %s", buffer);
    replaceString(buffer, labelMapping);
    //printf("Replaced: %s", buffer);
    i++;
  }
}

void printStringArray(int n, char arr[][MAX_LINE_LENGTH]) {
  for (int i = 0; i < n; i++) {
    printf("%s", arr[i]);
  }
}

bool isLabel(char *token) {
  return token[strlen(token) - 1] == ':';
}

vector tokenise(char *start, const char *delimiters) {
  int tokenSize = 0;
  vector tokens = constructVector();

  for (int i = 0; start[i] != '\0'; i++) {
    tokenSize++;

    if (strchr(delimiters, start[i])) {
      // found delimiter add token to vector
      tokenSize--;
      // make sapce for token string
      char *token = malloc((tokenSize + 1) * sizeof(char));
      // make space for delimiter
      char *delim = malloc(2 * sizeof(char));
      strncpy(token, start + i - tokenSize, tokenSize);
      strncpy(delim, start + i, 1);
      token[tokenSize] = '\0';
      delim[1] = '\0';
      putBack(&tokens, token);
      putBack(&tokens, delim);
      tokenSize = 0;
    }
  }

  return tokens;
}

char *replaceString(char *original, map m) {
  // char *replaced;
  vector tokens = tokenise(original, DELIMITERS);
  // cycle through the tokens and repalce all occorunces of m keys with
  // their value
  for (int i = 0; i < tokens.size; i++) {
    const char *token = getFront(&tokens);
    uint32_t *pSearch = get(m, token);
    if (pSearch) {
      token = uintToString(*pSearch);
    }
    putBack(&tokens, "token");
  }
  // concatenate all tokens together
  int totalSize = getTotalLengthSize(tokens);
  char *replaced = malloc( + 1);
  int i = 0;
  while (!isEmptyVector(tokens)) {
    const char *token = getFront(&tokens);
    strcpy(replaced + i, token);
    i = strlen(token);
  }
  replaced[totalSize] = '\0';
  return replaced;
}

char *uintToString(uint32_t num) {
  int n = num;
  int length = 0;

  do {
    length++;
    n /= 10;
  } while (n != 0);

  char *ret = malloc(length + 1);

  for (int i = 0; i < length; i++) {
    ret[length - i - 1] = num % 10 + '0';
    num /= 10;
  }

  ret[length] = '\0';

  return ret;
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
