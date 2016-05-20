#include "adts_interface.h"

// ---------------------------MAP---------------------------------
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

// --------------------------VECTOR--------------------------------
void putFront(vector *v, const char *value) {
  nodeVector *pNv = malloc(sizeof(nodeVector));
  pNv->previous = NULL;
  pNv->value = value;
  pNv->next = NULL;

  if (isEmpty(*v)) {
    v->first = pNv;
    v->last  = pNv;
  } else {
    pNv->next = v->first;
    v->first->previous = pNv;
    v->first = pNv;
  }
}

void putBack(vector *v, const char *value) {
  nodeVector *pNv = malloc(sizeof(nodeVector));
  pNv->previous = NULL;
  pNv->value = value;
  pNv->next = NULL;

  if (isEmpty(*v)) {
    v->first = pNv;
    v->last  = pNv;
  } else {
    pNv->previous = v->last;
    v->last->next = pNv;
    v->last = pNv;
  }
}

const char *peekFront(vector v) {
  assert(!isEmpty(v));

  return v.first->value;
}

const char *peekBack(vector v) {
  assert(!isEmpty(v));

  return v.last->value;
}

const char *getFront(vector *v) {
  assert(!isEmpty(*v));

  const char *ret = peekFront(*v);

  // remove first node and free memory
  nodeVector *removedNode = v->first;
  v->first = removedNode->next;
  if (!isEmpty(*v)) {
    v->first->previous = NULL;
  } else {
    v->last = NULL;
  }
  free(removedNode);

  return ret;
}

const char *getBack(vector *v) {
  assert(!isEmpty(*v));

  const char *ret = peekFront(*v);

  // remove last node and free memory
  nodeVector *removedNode = v->last;
  v->last = removedNode->previous;
  if (!isEmpty(*v)) {
    v->last->next = NULL;
  } else {
    v->first = NULL;
  }
  free(removedNode);

  return ret;
}

bool isEmpty(vector v) {
  return !v.first || !v.last;
}

void printVector(vector v) {
  printf("%s\t%s\t%s\n", "PREVIOUS", "VALUE", "NEXT");
  while (v.first) {
    printf("%p\t%s\t%p\n", (void *) v.first->previous,
                        v.first->value, (void *) v.first->next);
    v.first = v.first->next;
  }
}
