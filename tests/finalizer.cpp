#include "gc.h"
#include "gc_backptr.h"
#include <assert.h>
#include <stdio.h>

bool finalizerCalled = false;

void GC_CALLBACK finalizer(void* obj, void* client_data) {
  printf("\n\nfinalizer called\n\n");
  finalizerCalled = true;
}

int main() {
  GC_INIT();

  int *p = (int*)GC_MALLOC(sizeof(int));
  assert(*p == 0);
  *p = 5;
  assert(*p == 5);

  printf("p at 0x%x\n", p);

  GC_REGISTER_FINALIZER((void*)p, finalizer, NULL, (GC_finalization_proc*)0, (void**)0);

  GC_dump();

  GC_gcollect();
  GC_gcollect();
  GC_gcollect();

  GC_dump();

  assert(finalizerCalled);

  return 0;
}