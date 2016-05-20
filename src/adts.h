#include "adts_interface.h"

// -------------------functions definitions-----------------------
int *get(map *m, const char *key) {
  return lookup(m, key, &m) ? &(m->value) : NULL;
}

void put(map **m, const char *key, int value) {
  map *ptr = NULL;
  if(lookup(*m, key, &ptr)) {
    // if found
    ptr->value = value;
  } else {
    // if not found
    map *pNewNode   = malloc(sizeof(map));
    pNewNode->next  = NULL;
    pNewNode->key   = key;
    pNewNode->value = value;
    if (!ptr) {
      // no elemnts in mapping
      *m = pNewNode;
    } else {
      // make last element point to newNode
      ptr->next = pNewNode;
    }
  }
}

bool lookup(map *m, const char *key, map **ptr) {
  *ptr = m;
  while (m) {
    *ptr = m;
    if (!strcmp(m->key, key)) {
      // if keys match
      return true;
    }
    m = m->next;
  }
  return false;
}

void printMap(map *m, int n) {
  int initial = n;
  printf("%s\t%s\t%s\n", "NEXT", "KEY", "VALUE");
  while ((initial && n > 0 && m) || (!initial && m)) {
    printf("%p\t%s\t%d\n", (void *) m->next, m->key, m->value);
    m = m->next;
    n--;
  }
}
