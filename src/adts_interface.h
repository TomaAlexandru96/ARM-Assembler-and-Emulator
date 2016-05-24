#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include "headers.h"

// -------------------------TYPES---------------------------------
typedef struct map map;
typedef struct mapNode mapNode;
typedef struct vector vector;
typedef struct vectorNode vectorNode;

// -------------------------STRUCTS-------------------------------
struct map {
  mapNode *head;
  int     size;
};

struct mapNode {
  mapNode    *next;
  char *key;
  uint32_t    value;
};

struct vectorNode {
  vectorNode *previous;
  char *value;
  vectorNode *next;
};

struct vector {
  vectorNode *first;
  vectorNode *last;
  int size;
};

// -------------------FUNCTION DECLARATIONS-----------------------
// ---------------------------MAP---------------------------------
void clearMap(map *m);
map constructMap(void);

/**
* Retrives the pointer to the value of the key
* Returns NULL if nothing is found
**/
uint32_t *get(map m, char *key);

/**
* Takes a pointer to a table, a key of type string
* and a value of type int as parameeters
* If the key is found the value is modified to the value parameter
* If the key is not found the pair (key, value) is added to the table
**/
void put(map *m, char *key, uint32_t value);

/**
* Takes 3 parameters: pointer to table, a key, a pointer which will be
* modified to the element which maches the key or the last element if nothing is
* found or NULL if there are no elements
* Returns false if element is found
* Retruns true if element isn't found
**/
bool lookup(map m, char *key, mapNode **ptr);

/**
* Print n struct items if 0 print all elements
* (For debugging purposes)
**/
void printMap(map m);

bool isEmptyMap(map m);

// --------------------------VECTOR--------------------------------
void clearVector(vector *v);
vector constructVector(void);
void putFront(vector *v, char *value);
void putBack(vector *v, char *value);
char *getFront(vector *v);
char *getBack(vector *v);
bool isEmptyVector(vector v);
char *peekFront(vector v);
char *peekBack(vector v);
void printVector(vector v);
bool contains(vector v, char *value);
int getTotalLengthSize(vector v);
