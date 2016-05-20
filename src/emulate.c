#include <stdlib.h>
#include<stdio.h>

int main(int argc, char **argv) {
  FILE *file = fopen(argv[0], "wb");
  // char *line;
  if(file == NULL) {
    // err(1, "Enter valid file");
  }
  while (!feof(file)) {
    // fread(line, 1, 50, file);
    /*process line(what instr, )*/
  }
  return EXIT_SUCCESS;
}
