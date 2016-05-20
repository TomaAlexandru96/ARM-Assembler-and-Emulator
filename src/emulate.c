#include <stdlib.h>
#include<stdio.h>

#define MEM_SIZE_BYTES 

typedef struct proc_state {
  int NEG;
  int ZER;
  int CRY;
  int OVF;
  int regs[17];
  int memory[][32];
} proc_state_t;

int main(int argc, char **argv) {
  FILE *file = fopen(argv[0], "rb");


  return EXIT_SUCCESS;
}
