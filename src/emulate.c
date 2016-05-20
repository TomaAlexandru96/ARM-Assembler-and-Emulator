#include<stdlib.h>
#include<stdio.h>
#include<limits.h>

#define MEM_SIZE_BYTES 65536
#define NUMBER_REGS 17
#define LINE_LENGTH 50

typedef struct proc_state {
  int NEG;
  int ZER;
  int CRY;
  int OVF;
  int regs[NUMBER_REGS];
  int memory[MEM_SIZE_BYTES][CHAR_BIT];
} proc_state_t;

int main(int argc, char **argv) {

  int m = 0;

  FILE *file = fopen(argv[0], "rb");
  proc_state_t state;
  char buffer[LINE_LENGTH];
  
  while (!feof(file)) {
    int ret = fread(buffer, sizeof(char), LINE_LENGTH, file);
    if(ret != LINE_LENGTH) {
      perror("Invalid read");
      exit(EXIT_FAILURE);
    }

    for(int i = 9; i < 44; i++) {
      for(int j = m + 3; j >= m; j--) {
        memory[m][j]
      }
      m = m + 1;
    }


  }

  return EXIT_SUCCESS;
}
