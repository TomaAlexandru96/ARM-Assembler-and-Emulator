#include "adts.h"

#define MAX_LINE_LENGTH 512
#define DELIMITERS " ,\n"

/**
* Makes the first pass in the code and fills array lines with the lines
* from the input file also checkes for errors
* Returns the number of lines
**/
uint32_t firstPass(FILE *input, char lines[][MAX_LINE_LENGTH],
                      char *errorMessage, map *labelMapping);
void printStringArray(int n, char arr[][MAX_LINE_LENGTH]);
vector tokenise(char *start, const char *delimiters);
bool isLabel(char *token);

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

  char *errorMessage = "";
  char lines[5000][MAX_LINE_LENGTH];
  map labelMapping = constructMap();
  uint32_t linesNumber = firstPass(input, lines, errorMessage, &labelMapping);

  printMap(labelMapping, 0);
  printStringArray(linesNumber, lines);

  // if we have compile erros stop and print errors
  if (errorMessage[0] != '\0') {
    fprintf(stderr, "%s\n", errorMessage);
    return EXIT_FAILURE;
  }

  FILE *output = fopen(argv[2], "wb");

  //fwrite(&x, sizeof(int), 1, output);

  fclose(input);
  fclose(output);

  return EXIT_SUCCESS;
}

uint32_t firstPass(FILE *input, char lines[][MAX_LINE_LENGTH],
              char *errorMessage, map *labelMapping) {
  uint32_t lineNumber = 1;
  //uint32_t currentMemory = 0;
  vector currentLabels = constructVector();

  while(fgets(lines[lineNumber - 1], MAX_LINE_LENGTH, input)) {
    vector tokens = tokenise(lines[lineNumber - 1], DELIMITERS);

    // check for all tokens see if there are in labels
    // if there are labels add all of them to a vector list and
    // map all labels with the memorry address of the nex instruction
    while (!isEmptyVector(tokens)) {
      char *token = (char *) getFront(&tokens);
      if (isLabel(token)) {
        putBack(&currentLabels, token);
      }
    }
    lineNumber++;
  }
  printVector(currentLabels);
  return lineNumber;
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
      strncpy(token, start + i - tokenSize, tokenSize);
      token[tokenSize] = '\0';
      putBack(&tokens, token);
      tokenSize = 0;
    }
  }

  return tokens;
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
