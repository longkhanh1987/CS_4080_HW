#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "common.h"
#include "debug.h"
#include "memory.h"
#include "object.h"
#include "vm.h"

VM vm;

static void resetStack(void) {
  vm.stackTop = vm.stack;
}

static bool valuesEqual(Value a, Value b) {
  if (a.type != b.type) return false;

  switch (a.type) {
    case VAL_BOOL:
      return AS_BOOL(a) == AS_BOOL(b);

    case VAL_NIL:
      return true;

    case VAL_NUMBER:
      return AS_NUMBER(a) == AS_NUMBER(b);

    case VAL_OBJ:
      if (IS_STRING(a) && IS_STRING(b)) {
        ObjString* sa = AS_STRING(a);
        ObjString* sb = AS_STRING(b);

        return sa->length == sb->length &&
               memcmp(sa->chars, sb->chars, sa->length) == 0;
      }

      return AS_OBJ(a) == AS_OBJ(b);
  }

  return false;
}

static bool isFalsey(Value value) {
  return IS_NIL(value) || (IS_BOOL(value) && !AS_BOOL(value));
}

static int findGlobal(ObjString* name) {
  for (int i = 0; i < vm.globalCount; i++) {
    if (vm.globals[i].name->length == name->length &&
        memcmp(vm.globals[i].name->chars,
               name->chars,
               name->length) == 0) {
      return i;
    }
  }

  return -1;
}

void initVM(void) {
  vm.stackCapacity = 8;
  vm.stack = (Value*)malloc(sizeof(Value) * vm.stackCapacity);
  vm.objects = NULL;
  vm.globalCount = 0;
  resetStack();
}

void freeVM(void) {
  free(vm.stack);
  freeObjects();
}

void push(Value value) {
  int count = (int)(vm.stackTop - vm.stack);

  if (count >= vm.stackCapacity) {
    vm.stackCapacity *= 2;
    vm.stack = (Value*)realloc(vm.stack, sizeof(Value) * vm.stackCapacity);
    vm.stackTop = vm.stack + count;
  }

  *vm.stackTop = value;
  vm.stackTop++;
}

Value pop(void) {
  vm.stackTop--;
  return *vm.stackTop;
}

static Value peek(int distance) {
  return vm.stackTop[-1 - distance];
}

static void runtimeError(const char* format, ...) {
  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);
  fprintf(stderr, "\n");
}

static void concatenate(void) {
  ObjString* b = AS_STRING(pop());
  ObjString* a = AS_STRING(pop());

  int length = a->length + b->length;
  char* chars = ALLOCATE(char, length + 1);

  memcpy(chars, a->chars, a->length);
  memcpy(chars + a->length, b->chars, b->length);
  chars[length] = '\0';

  push(OBJ_VAL(takeString(chars, length)));
}

/*
  Chapter 24 Challenge 3:
  Native functions return bool for success/failure and store
  the actual result through a Value* result pointer.
*/
static bool callNative(ObjNative* native, int argCount) {
  if (argCount != native->arity) {
    runtimeError("Expected %d arguments but got %d.",
                 native->arity, argCount);
    return false;
  }

  Value result;
  if (!native->function(argCount, vm.stackTop - argCount, &result)) {
    return false;
  }

  vm.stackTop -= argCount + 1;
  push(result);

  return true;
}

/*
  Example native function for Chapter 24 Challenge 4.
  This returns the type of a value as a string.
*/
static bool typeNative(int argCount, Value* args, Value* result) {
  if (argCount != 1) {
    runtimeError("type() expects 1 argument.");
    return false;
  }

  Value value = args[0];

  if (IS_NUMBER(value)) {
    *result = OBJ_VAL(copyString("number", 6));
  } else if (IS_BOOL(value)) {
    *result = OBJ_VAL(copyString("bool", 4));
  } else if (IS_NIL(value)) {
    *result = OBJ_VAL(copyString("nil", 3));
  } else if (IS_STRING(value)) {
    *result = OBJ_VAL(copyString("string", 6));
  } else {
    *result = OBJ_VAL(copyString("unknown", 7));
  }

  return true;
}

/*
  Chapter 25 Challenge 1:
  Proposed call support for both raw ObjFunction and ObjClosure.

  In a full Chapter 24/25 VM, these would create CallFrames.
  Your current VM does not have CallFrame yet, so these are placeholders.
*/
static bool callFunction(ObjFunction* function, int argCount) {
  if (argCount != function->arity) {
    runtimeError("Expected %d arguments but got %d.",
                 function->arity, argCount);
    return false;
  }

  runtimeError("Function call not fully implemented yet.");
  return false;
}

static bool callClosure(ObjClosure* closure, int argCount) {
  if (argCount != closure->function->arity) {
    runtimeError("Expected %d arguments but got %d.",
                 closure->function->arity, argCount);
    return false;
  }

  runtimeError("Closure call not fully implemented yet.");
  return false;
}

static InterpretResult run(void) {
  register uint8_t* ip = vm.ip;

#define READ_BYTE() (*ip++)

#define READ_SHORT() \
  (ip += 2, (uint16_t)((ip[-2] << 8) | ip[-1]))

#define READ_CONSTANT() \
  (vm.chunk->constants.values[READ_BYTE()])

#define READ_CONSTANT_LONG() \
  ({ \
    uint32_t b1 = READ_BYTE(); \
    uint32_t b2 = READ_BYTE(); \
    uint32_t b3 = READ_BYTE(); \
    vm.chunk->constants.values[b1 | (b2 << 8) | (b3 << 16)]; \
  })

#define BINARY_OP(valueType, op) \
  do { \
    if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))) { \
      vm.ip = ip; \
      runtimeError("Operands must be numbers."); \
      return INTERPRET_RUNTIME_ERROR; \
    } \
    double b = AS_NUMBER(pop()); \
    double a = AS_NUMBER(pop()); \
    push(valueType(a op b)); \
  } while (false)

  for (;;) {
    uint8_t instruction = READ_BYTE();

    switch (instruction) {
      case OP_CONSTANT:
        push(READ_CONSTANT());
        break;

      case OP_CONSTANT_LONG:
        push(READ_CONSTANT_LONG());
        break;

      case OP_NIL:
        push(NIL_VAL);
        break;

      case OP_TRUE:
        push(BOOL_VAL(true));
        break;

      case OP_FALSE:
        push(BOOL_VAL(false));
        break;

      case OP_POP:
        pop();
        break;

      case OP_GET_LOCAL: {
        uint8_t slot = READ_BYTE();
        push(vm.stack[slot]);
        break;
      }

      case OP_SET_LOCAL: {
        uint8_t slot = READ_BYTE();
        vm.stack[slot] = peek(0);
        break;
      }

      case OP_GET_GLOBAL: {
        ObjString* name = AS_STRING(READ_CONSTANT());
        int slot = findGlobal(name);

        if (slot == -1) {
          vm.ip = ip;
          runtimeError("Undefined variable.");
          return INTERPRET_RUNTIME_ERROR;
        }

        push(vm.globals[slot].value);
        break;
      }

      case OP_DEFINE_GLOBAL: {
        ObjString* name = AS_STRING(READ_CONSTANT());
        int slot = findGlobal(name);

        if (slot == -1) {
          if (vm.globalCount >= 256) {
            vm.ip = ip;
            runtimeError("Too many global variables.");
            return INTERPRET_RUNTIME_ERROR;
          }

          slot = vm.globalCount++;
          vm.globals[slot].name = name;
        }

        vm.globals[slot].value = peek(0);
        pop();
        break;
      }

      case OP_SET_GLOBAL: {
        ObjString* name = AS_STRING(READ_CONSTANT());
        int slot = findGlobal(name);

        if (slot == -1) {
          vm.ip = ip;
          runtimeError("Undefined variable.");
          return INTERPRET_RUNTIME_ERROR;
        }

        vm.globals[slot].value = peek(0);
        break;
      }

      case OP_CALL: {
        int argCount = READ_BYTE();
        Value callee = peek(argCount);

        /*
          Chapter 25 Challenge 1:
          Support both raw functions and closures.
          Native functions still work from Chapter 24.
        */
        if (IS_FUNCTION(callee)) {
          if (!callFunction(AS_FUNCTION(callee), argCount)) {
            vm.ip = ip;
            return INTERPRET_RUNTIME_ERROR;
          }

        } else if (IS_CLOSURE(callee)) {
          if (!callClosure(AS_CLOSURE(callee), argCount)) {
            vm.ip = ip;
            return INTERPRET_RUNTIME_ERROR;
          }

        } else if (IS_NATIVE(callee)) {
          if (!callNative(AS_NATIVE_OBJ(callee), argCount)) {
            vm.ip = ip;
            return INTERPRET_RUNTIME_ERROR;
          }

        } else {
          vm.ip = ip;
          runtimeError("Can only call functions and closures.");
          return INTERPRET_RUNTIME_ERROR;
        }

        break;
      }

      case OP_EQUAL: {
        Value b = pop();
        Value a = pop();
        push(BOOL_VAL(valuesEqual(a, b)));
        break;
      }

      case OP_GREATER:
        BINARY_OP(BOOL_VAL, >);
        break;

      case OP_LESS:
        BINARY_OP(BOOL_VAL, <);
        break;

      case OP_ADD:
        if (IS_STRING(peek(0)) && IS_STRING(peek(1))) {
          concatenate();
        } else if (IS_NUMBER(peek(0)) && IS_NUMBER(peek(1))) {
          double b = AS_NUMBER(pop());
          double a = AS_NUMBER(pop());
          push(NUMBER_VAL(a + b));
        } else {
          vm.ip = ip;
          runtimeError("Operands must be two numbers or two strings.");
          return INTERPRET_RUNTIME_ERROR;
        }
        break;

      case OP_SUBTRACT:
        BINARY_OP(NUMBER_VAL, -);
        break;

      case OP_MULTIPLY:
        BINARY_OP(NUMBER_VAL, *);
        break;

      case OP_DIVIDE:
        BINARY_OP(NUMBER_VAL, /);
        break;

      case OP_NOT:
        push(BOOL_VAL(isFalsey(pop())));
        break;

      case OP_NEGATE:
        if (!IS_NUMBER(peek(0))) {
          vm.ip = ip;
          runtimeError("Operand must be a number.");
          return INTERPRET_RUNTIME_ERROR;
        }

        vm.stackTop[-1].as.number = -vm.stackTop[-1].as.number;
        break;

      case OP_PRINT:
        printValue(pop());
        printf("\n");
        break;

      case OP_JUMP: {
        uint16_t offset = READ_SHORT();
        ip += offset;
        break;
      }

      case OP_JUMP_IF_FALSE: {
        uint16_t offset = READ_SHORT();

        if (isFalsey(peek(0))) {
          ip += offset;
        }

        break;
      }

      case OP_LOOP: {
        uint16_t offset = READ_SHORT();
        ip -= offset;
        break;
      }

      case OP_RETURN:
        vm.ip = ip;

        if (vm.stackTop > vm.stack) {
          printValue(pop());
          printf("\n");
        }

        return INTERPRET_OK;
    }
  }

#undef READ_BYTE
#undef READ_SHORT
#undef READ_CONSTANT
#undef READ_CONSTANT_LONG
#undef BINARY_OP
}

InterpretResult interpret(Chunk* chunk) {
  vm.chunk = chunk;
  vm.ip = vm.chunk->code;
  return run();
}