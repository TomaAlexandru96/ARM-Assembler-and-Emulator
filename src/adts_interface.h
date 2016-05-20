#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include "headers.h"

// -------------------------types---------------------------------
typedef struct mapping map;
typedef struct nodeVector nodeVector;
typedef struct vector vector;

// -------------------------structs-------------------------------
struct mapping {
  map        *next;
  const char *key;
  int        value;
};

struct nodeVector {
  nodeVector *previous;
  const char *value;
  nodeVector *next;
};

struct vector {
  nodeVector *first;
  nodeVector *last;
};

// -------------------functions declarations----------------------
// ---------------------------MAP---------------------------------
/**
* Retrives the pointer to the value of the key
* Returns NULL if nothing is found
**/
int *get(map *m, const char *key);

/**
* Takes a pointer to a table, a key of type string
* and a value of type int as parameeters
* If the key is found the value is modified to the value parameter
* If the key is not found the pair (key, value) is added to the table
**/
void put(map **m, const char *key, int value);

/**
* Takes 3 parameters: pointer to table, a key, a pointer which will be
* modified to the element which maches the key or the last element if nothing is
* found or NULL if there are no elements
* Returns false if element is found
* Retruns true if element isn't found
**/
bool lookup(map *m, const char *key, map **ptr);

/**
* Print n struct items if 0 print all elements
* (For debugging purposes)
**/
void printMap(map *m, int n);

// --------------------------VECTOR--------------------------------
void putFront(vector *v, const char *value);
void putBack(vector *v, const char *value);
const char *getFront(vector *v);
const char *getBack(vector *v);
bool isEmpty(vector v);
const char *peekFront(vector v);
const char *peekBack(vector v);
void printVector(vector v);
