#include <stdlib.h>

#include "memory.h"
#include "object.h"
#include "vm.h"

static void markArray(ValueArray* array);
static void markRoots(void);
static void blackenObject(Obj* object);
static void sweep(void);

void markObject(Obj* object) {
  if (object == NULL) return;
  if (object->isMarked) return;

  object->isMarked = true;
  blackenObject(object);
}

void markValue(Value value) {
  if (IS_OBJ(value)) {
    markObject(AS_OBJ(value));
  }
}

static void markArray(ValueArray* array) {
  for (int i = 0; i < array->count; i++) {
    markValue(array->values[i]);
  }
}

static void markRoots(void) {
  // Mark values on the VM stack.
  for (Value* slot = vm.stack; slot < vm.stackTop; slot++) {
    markValue(*slot);
  }

  // Mark global variables.
  for (int i = 0; i < vm.globalCount; i++) {
    markObject((Obj*)vm.globals[i].name);
    markValue(vm.globals[i].value);
  }
}

static void blackenObject(Obj* object) {
  switch (object->type) {
    case OBJ_CLOSURE: {
      ObjClosure* closure = (ObjClosure*)object;
      markObject((Obj*)closure->function);
      break;
    }

    case OBJ_FUNCTION: {
      ObjFunction* function = (ObjFunction*)object;
      markObject((Obj*)function->name);
      markArray(&function->chunk.constants);
      break;
    }

    case OBJ_NATIVE:
      break;

    case OBJ_STRING:
      break;
  }
}

static void freeObject(Obj* object) {
  switch (object->type) {
    case OBJ_CLOSURE: {
      FREE(ObjClosure, object);
      break;
    }

    case OBJ_FUNCTION: {
      ObjFunction* function = (ObjFunction*)object;
      freeChunk(&function->chunk);
      FREE(ObjFunction, object);
      break;
    }

    case OBJ_NATIVE: {
      FREE(ObjNative, object);
      break;
    }

    case OBJ_STRING: {
      ObjString* string = (ObjString*)object;
      FREE_ARRAY(char, string->chars, string->length + 1);
      FREE(ObjString, object);
      break;
    }
  }
}

static void sweep(void) {
  Obj* previous = NULL;
  Obj* object = vm.objects;

  while (object != NULL) {
    if (object->isMarked) {
      object->isMarked = false;
      previous = object;
      object = object->next;
    } else if (object->refCount == 0) {
      freeObject(object);
    }else {
      Obj* unreached = object;
      object = object->next;

      if (previous != NULL) {
        previous->next = object;
      } else {
        vm.objects = object;
      }

      freeObject(unreached);
    }
  }
}

void collectGarbage(void) {
  markRoots();
  sweep();
}

void freeObjects(void) {
  Obj* object = vm.objects;

  while (object != NULL) {
    Obj* next = object->next;
    freeObject(object);
    object = next;
  }
}

void* reallocate(void* pointer, size_t oldSize, size_t newSize) {
  (void)oldSize;

  if (newSize == 0) {
    free(pointer);
    return NULL;
  }

  void* result = realloc(pointer, newSize);
  if (result == NULL) exit(1);

  return result;
}