#include "gc.h"
#include "gc_backptr.h"
#include "gc_typed.h"
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

  int* p = (int*)GC_MALLOC(sizeof(int));
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

  int* p = (int*)GC_MALLOC_UNCOLLECTABLE(sizeof(int));
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

  int** p = (int**)GC_MALLOC(sizeof(int*));
  assert(*p == 0);
  int* q = (int*)GC_MALLOC(sizeof(int));
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

  int** p = (int**)GC_MALLOC_UNCOLLECTABLE(sizeof(int*));
  assert(*p == 0);
  int* q = (int*)GC_MALLOC(sizeof(int));
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

  int** p = (int**)GC_MALLOC(sizeof(int*));
  assert(*p == 0);
  int* q = (int*)GC_MALLOC_UNCOLLECTABLE(sizeof(int));
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

struct obj {
  int* pointer1;
  int* value1;
  int* pointer2;
};

bool finalizerObjCalled;
bool finalizerPointer1Called;
bool finalizerValue1Called;
bool finalizerPointer2Called;

void GC_CALLBACK finalizerObj(void* obj, void* client_data) {
  finalizerObjCalled = true;
}

void GC_CALLBACK finalizerPointer1(void* obj, void* client_data) {
  finalizerPointer1Called = true;
}

void GC_CALLBACK finalizerValue1(void* obj, void* client_data) {
  finalizerValue1Called = true;
}

void GC_CALLBACK finalizerPointer2(void* obj, void* client_data) {
  finalizerPointer2Called = true;
}

void testFinalizer6(bool value1pointer) {
  finalizerObjCalled = false;
  finalizerPointer1Called = false;
  finalizerValue1Called = false;
  finalizerPointer2Called = false;

  GC_descr obj_descr;
  GC_word obj_bitmap[GC_BITMAP_SIZE(obj)] = { 0 };
  GC_set_bit(obj_bitmap, GC_WORD_OFFSET(obj, pointer1));
  if (value1pointer) {
    GC_set_bit(obj_bitmap, GC_WORD_OFFSET(obj, value1));
  }
  GC_set_bit(obj_bitmap, GC_WORD_OFFSET(obj, pointer2));

  obj_descr = GC_make_descriptor(obj_bitmap, GC_WORD_LEN(obj));

  obj** ptr_to_an_obj = (obj**)GC_MALLOC_UNCOLLECTABLE(sizeof(obj**));

  obj* an_obj = (obj*)GC_MALLOC_EXPLICITLY_TYPED(sizeof(obj), obj_descr);

  *ptr_to_an_obj = an_obj;

  assert(an_obj->pointer1 == 0);
  assert(an_obj->value1 == 0);
  assert(an_obj->pointer2 == 0);

  int* pointer1 = (int*)GC_MALLOC(sizeof(int));
  int* value1 = (int*)GC_MALLOC(sizeof(int));
  int* pointer2 = (int*)GC_MALLOC(sizeof(int));

  GC_REGISTER_FINALIZER((void*)an_obj, finalizerObj, NULL, (GC_finalization_proc*)0, (void**)0);
  GC_REGISTER_FINALIZER((void*)pointer1, finalizerPointer1, NULL, (GC_finalization_proc*)0, (void**)0);
  GC_REGISTER_FINALIZER((void*)value1, finalizerValue1, NULL, (GC_finalization_proc*)0, (void**)0);
  GC_REGISTER_FINALIZER((void*)pointer2, finalizerPointer2, NULL, (GC_finalization_proc*)0, (void**)0);

  an_obj->pointer1 = pointer1;
  an_obj->value1 = value1;
  an_obj->pointer2 = pointer2;

  GC_gcollect();

  assert(!finalizerObjCalled);
  assert(!finalizerPointer1Called);
  if (value1pointer) {
    assert(!finalizerValue1Called);
  } else {
    assert(finalizerValue1Called);
  }
  assert(!finalizerPointer2Called);

  GC_FREE(ptr_to_an_obj);

  GC_gcollect();

  assert(finalizerObjCalled);

  GC_gcollect();

  assert(finalizerPointer1Called);
  assert(finalizerValue1Called);
  assert(finalizerPointer2Called);
}

int main() {
  GC_INIT();

  testFinalizer1();
  testFinalizer2();
  testFinalizer3();
  testFinalizer4();
  testFinalizer5();
  testFinalizer6(true);
  testFinalizer6(false);

  return 0;
}
