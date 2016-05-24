#include "adts_interface.h"

// ---------------------------MAP---------------------------------
void clearMap(map *m) {
  mapNode *ptr = m->head;
  while(ptr) {
    mapNode *prev = ptr;
    ptr = ptr->next;
    free(prev);
  }
  m->head = NULL;
  m->size = 0;
}

map constructMap(void) {
  map m = {NULL, 0};
  return m;
}

uint32_t *get(map m, const char *key) {
  mapNode *ptr = NULL;

  return lookup(m, key, &ptr) ? &(ptr->value) : NULL;
}

bool isEmptyMap(map m) {
  return !m.size;
}

void put(map *m, const char *key, uint32_t value) {
  mapNode *ptr = NULL;
  if(lookup(*m, key, &ptr)) {
    // if found
    ptr->value = value;
  } else {
    // if not found
    mapNode *pNewNode   = malloc(sizeof(mapNode));
    pNewNode->next  = NULL;
    pNewNode->key   = key;
    pNewNode->value = value;
    if (!ptr) {
      // no elemnts in mapping
      m->head = pNewNode;
    } else {
      // make last element point to newNode
      ptr->next = pNewNode;
    }
    m->size++;
  }
}

bool lookup(map m, const char *key, mapNode **ptr) {
  *ptr = m.head;
  while (m.head) {
    *ptr = m.head;
    if (!strcmp(m.head->key, key)) {
      // if keys match
      return true;
    }
    m.head = m.head->next;
  }
  return false;
}

void printMap(map m) {
  printf("Map: HEAD: %p, SIZE: %d\n", (void *) m.head, m.size);
  printf("%s\t%s\t%s\n", "NEXT", "KEY", "VALUE");
  while (m.head) {
    printf("%p\t%s\t%d\n", (void *) m.head->next,
                                m.head->key, m.head->value);
    m.head = m.head->next;
  }
  puts("");
}

// --------------------------VECTOR--------------------------------
void clearVector(vector *v) {
  while (!isEmptyVector(*v)) {
    getFront(v);
  }
  v->size = 0;
}

vector constructVector(void) {
  vector v = {NULL, NULL, 0};
  return v;
}

void putFront(vector *v, const char *value) {
  vectorNode *pNv = malloc(sizeof(vectorNode));
  pNv->previous = NULL;
  pNv->value = value;
  pNv->next = NULL;

  if (isEmptyVector(*v)) {
    v->first = pNv;
    v->last  = pNv;
  } else {
    pNv->next = v->first;
    v->first->previous = pNv;
    v->first = pNv;
  }
  (v->size)++;
}

void putBack(vector *v, const char *value) {
  vectorNode *pNv = malloc(sizeof(vectorNode));
  pNv->previous = NULL;
  pNv->value = value;
  pNv->next = NULL;

  if (isEmptyVector(*v)) {
    v->first = pNv;
    v->last  = pNv;
  } else {
    pNv->previous = v->last;
    v->last->next = pNv;
    v->last = pNv;
  }
  (v->size)++;
}

const char *peekFront(vector v) {
  if (isEmptyVector(v)) {
    return NULL;
  }

  return v.first->value;
}

const char *peekBack(vector v) {
  if (isEmptyVector(v)) {
    return NULL;
  }

  return v.last->value;
}

const char *getFront(vector *v) {
  if (isEmptyVector(*v)) {
    return NULL;
  }

  const char *ret = peekFront(*v);

  // remove first node and free memory
  vectorNode *removedNode = v->first;
  v->first = removedNode->next;
  if (!isEmptyVector(*v)) {
    v->first->previous = NULL;
  } else {
    v->last = NULL;
  }
  free(removedNode);
  (v->size)--;

  return ret;
}

const char *getBack(vector *v) {
  if (isEmptyVector(*v)) {
    return NULL;
  }

  const char *ret = peekFront(*v);
  // remove last node and free memory
  vectorNode *removedNode = v->last;
  v->last = removedNode->previous;
  if (!isEmptyVector(*v)) {
    v->last->next = NULL;
  } else {
    v->first = NULL;
  }
  free(removedNode);
  (v->size)--;

  return ret;
}

bool isEmptyVector(vector v) {
  return !v.first || !v.last;
}

void printVector(vector v) {
  printf("Vector: FIRST: %p, LAST: %p, SIZE: %d\n", (void *) v.first,
                  (void *) v.last, v.size);
  printf("%s\t%s\t%s\n", "PREVIOUS", "VALUE", "NEXT");
  while (v.first) {
    printf("%p\t%s\t%p\n", (void *) v.first->previous,
                        v.first->value, (void *) v.first->next);
    v.first = v.first->next;
  }
  puts("");
}

bool contains(vector v, const char *value) {
  vectorNode *current = v.first;
  while (current) {
    if (!strcmp(current->value, value)) {
      // vector contains the item
      return true;
    }
    current = current->next;
  }

  return false;
}

int getTotalLengthSize(vector v) {
  vectorNode *currentNode = v.first;
  int total = 0;

  while (currentNode) {
    total += strlen(currentNode->value);
    currentNode = currentNode->next;
  }

  return total;
}
