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

map constructMap() {
  map m = {NULL, 0};
  return m;
}

int *get(map m, const char *key) {
  assert(!isEmptyMap(m));

  mapNode *ptr = NULL;

  return lookup(m, key, &ptr) ? &(ptr->value) : NULL;
}

bool isEmptyMap(map m) {
  return !m.size;
}

void put(map *m, const char *key, int value) {
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

void printMap(map m, int n) {
  int initial = n;
  printf("Map: HEAD: %p, SIZE: %d\n", (void *) m.head, m.size);
  printf("%s\t%s\t%s\n", "NEXT", "KEY", "VALUE");
  while ((initial && n > 0 && m.head) || (!initial && m.head)) {
    printf("%p\t%s\t%d\n", (void *) m.head->next,
                                m.head->key, m.head->value);
    m.head = m.head->next;
    n--;
  }
  puts("");
}

// --------------------------VECTOR--------------------------------
void clearVector(vector *v) {
  while (!isEmptyVector(*v)) {
    getFront(v);
  }
}

vector constructVector() {
  vector v = {NULL, NULL};
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
}

const char *peekFront(vector v) {
  assert(!isEmptyVector(v));

  return v.first->value;
}

const char *peekBack(vector v) {
  assert(!isEmptyVector(v));

  return v.last->value;
}

const char *getFront(vector *v) {
  assert(!isEmptyVector(*v));

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

  return ret;
}

const char *getBack(vector *v) {
  assert(!isEmptyVector(*v));

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

  return ret;
}

bool isEmptyVector(vector v) {
  return !v.first || !v.last;
}

void printVector(vector v) {
  printf("Vector: FIRST: %p, LAST: %p\n", (void *) v.first, (void *) v.last);
  printf("%s\t%s\t%s\n", "PREVIOUS", "VALUE", "NEXT");
  while (v.first) {
    printf("%p\t%s\t%p\n", (void *) v.first->previous,
                        v.first->value, (void *) v.first->next);
    v.first = v.first->next;
  }
  puts("");
}
