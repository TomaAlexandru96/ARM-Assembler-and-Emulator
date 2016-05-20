#include "adts.h"

#define MAX_LINE_LENGTH 512

int main(int argc, char **argv) {
  vector x = constructVector();

  putFront(&x, "Hello1");
  putFront(&x, "Hello2");
  putFront(&x, "Hello3");
  printVector(x);

  map m = constructMap();

  put(&m, "string1", 20);
  put(&m, "string3", 15);
  put(&m, "string1", 15);
  put(&m, "string2", 28);

  printMap(m, 0);

  /*
  // Check for number of arguments
  if (argc != 3) {
    fprintf(stderr, "The function needs 2 arguments!");
    return EXIT_FAILURE;
  }

  FILE *input = fopen(argv[1], "r");
  FILE *output;

  // check file existance throw error if not found
  if (!input) {
    fprintf(stderr, "The file %s was not found", argv[1]);
    return EXIT_FAILURE;
  }

  output = fopen(argv[2], "wb");

  //fwrite(&x, sizeof(int), 1, output);

  fclose(input);
  fclose(output);

  return EXIT_SUCCESS;
  */
}
