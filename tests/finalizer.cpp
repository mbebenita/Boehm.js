#include "gc.h"
#include "gc_backptr.h"
#include <assert.h>
#include <stdio.h>

bool finalizerCalled;
bool finalizerLinkedCalled;

void GC_CALLBACK finalizer(void* obj, void* client_data) {
  finalizerCalled = true;
}

void GC_CALLBACK finalizerLinked(void* obj, void* client_data) {
  finalizerLinkedCalled = true;
}

// Test GC with a collectable object
void testFinalizer1() {
  finalizerCalled = false;

  int *p = (int*)GC_MALLOC(sizeof(int));
  assert(*p == 0);
  *p = 5;
  assert(*p == 5);

  GC_REGISTER_FINALIZER((void*)p, finalizer, NULL, (GC_finalization_proc*)0, (void**)0);

  GC_gcollect();

  assert(finalizerCalled);
}

// Test GC with an uncollectable object
void testFinalizer2() {
  finalizerCalled = false;

  int *p = (int*)GC_MALLOC_UNCOLLECTABLE(sizeof(int));
  assert(*p == 0);
  *p = 5;
  assert(*p == 5);

  GC_REGISTER_FINALIZER((void*)p, finalizer, NULL, (GC_finalization_proc*)0, (void**)0);

  GC_gcollect();

  assert(!finalizerCalled);

  // Unregister finalizer, otherwise p can't be freed.
  GC_REGISTER_FINALIZER((void*)p, 0, NULL, (GC_finalization_proc*)0, (void**)0);

  GC_FREE(p);
}

// Test GC with a collectable object pointing to a collectable object
void testFinalizer3() {
  finalizerCalled = false;
  finalizerLinkedCalled = false;

  int **p = (int**)GC_MALLOC(sizeof(int*));
  assert(*p == 0);
  int *q = (int*)GC_MALLOC(sizeof(int));
  assert(*q == 0);

  *p = q;
  assert(*p == q);

  GC_REGISTER_FINALIZER((void*)p, finalizer, NULL, (GC_finalization_proc*)0, (void**)0);
  GC_REGISTER_FINALIZER((void*)q, finalizerLinked, NULL, (GC_finalization_proc*)0, (void**)0);

  GC_gcollect();

  assert(finalizerCalled);
  assert(!finalizerLinkedCalled);

  GC_gcollect();

  assert(finalizerLinkedCalled);
}

// Test GC with an uncollectable object pointing to a collectable object
void testFinalizer4() {
  finalizerCalled = false;
  finalizerLinkedCalled = false;

  int **p = (int**)GC_MALLOC_UNCOLLECTABLE(sizeof(int*));
  assert(*p == 0);
  int *q = (int*)GC_MALLOC(sizeof(int));
  assert(*q == 0);

  *p = q;
  assert(*p == q);

  GC_REGISTER_FINALIZER((void*)p, finalizer, NULL, (GC_finalization_proc*)0, (void**)0);
  GC_REGISTER_FINALIZER((void*)q, finalizerLinked, NULL, (GC_finalization_proc*)0, (void**)0);

  GC_gcollect();

  assert(!finalizerCalled);
  assert(!finalizerLinkedCalled);

  GC_gcollect();

  assert(!finalizerCalled);
  assert(!finalizerLinkedCalled);

  // Unregister finalizer, otherwise p can't be freed.
  GC_REGISTER_FINALIZER((void*)p, 0, NULL, (GC_finalization_proc*)0, (void**)0);

  GC_FREE(p);

  GC_gcollect();

  assert(finalizerLinkedCalled);
}

// Test GC with a collectable object pointing to an uncollectable object
void testFinalizer5() {
  finalizerCalled = false;
  finalizerLinkedCalled = false;

  int **p = (int**)GC_MALLOC(sizeof(int*));
  assert(*p == 0);
  int *q = (int*)GC_MALLOC_UNCOLLECTABLE(sizeof(int));
  assert(*q == 0);

  *p = q;
  assert(*p == q);

  GC_REGISTER_FINALIZER((void*)p, finalizer, NULL, (GC_finalization_proc*)0, (void**)0);
  GC_REGISTER_FINALIZER((void*)q, finalizerLinked, NULL, (GC_finalization_proc*)0, (void**)0);

  GC_gcollect();

  assert(finalizerCalled);
  assert(!finalizerLinkedCalled);

  GC_gcollect();

  assert(!finalizerLinkedCalled);

  // Unregister finalizer, otherwise q can't be freed.
  GC_REGISTER_FINALIZER((void*)q, 0, NULL, (GC_finalization_proc*)0, (void**)0);

  GC_FREE(q);
}

int main() {
  GC_INIT();

  testFinalizer1();
  testFinalizer2();
  testFinalizer3();
  testFinalizer4();
  testFinalizer5();

  return 0;
}
