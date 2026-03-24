#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "debug.h"
#include "memory.h"
#include "object.h"
#include "vm.h"

VM vm;

static void resetStack(void) {
  vm.stackTop = vm.stack;
}

void initVM(void) {
  vm.stackCapacity = 8;
  vm.stack = (Value*)malloc(sizeof(Value) * vm.stackCapacity);
  vm.objects = NULL;
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

static void runtimeError(const char* message) {
  fprintf(stderr, "%s\n", message);
}

static void concatenate(void) {
  ObjString* b = AS_STRING(pop());
  ObjString* a = AS_STRING(pop());

  int length = a->length + b->length;
  char* chars = ALLOCATE(char, length + 1);

  memcpy(chars, a->chars, a->length);
  memcpy(chars + a->length, b->chars, b->length);
  chars[length] = '\0';

  ObjString* result = takeString(chars, length);
  push(OBJ_VAL(result));
}

static InterpretResult run(void) {
#define READ_BYTE() (*vm.ip++)
#define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])
#define READ_CONSTANT_LONG() \
  (vm.chunk->constants.values[ \
    READ_BYTE() | (READ_BYTE() << 8) | (READ_BYTE() << 16)])

#define BINARY_OP(op) \
  do { \
    if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))) { \
      runtimeError("Operands must be numbers."); \
      return INTERPRET_RUNTIME_ERROR; \
    } \
    double b = AS_NUMBER(pop()); \
    double a = AS_NUMBER(pop()); \
    push(NUMBER_VAL(a op b)); \
  } while (false)

  for (;;) {
#ifdef DEBUG_TRACE_EXECUTION
    printf("          ");
    for (Value* slot = vm.stack; slot < vm.stackTop; slot++) {
      printf("[ ");
      printValue(*slot);
      printf(" ]");
    }
    printf("\n");
    disassembleInstruction(vm.chunk, (int)(vm.ip - vm.chunk->code));
#endif

    uint8_t instruction;

    switch (instruction = READ_BYTE()) {
      case OP_CONSTANT: {
        Value constant = READ_CONSTANT();
        push(constant);
        break;
      }

      case OP_CONSTANT_LONG: {
        Value constant = READ_CONSTANT_LONG();
        push(constant);
        break;
      }

      case OP_ADD: {
        if (IS_STRING(peek(0)) && IS_STRING(peek(1))) {
          concatenate();
        } else if (IS_NUMBER(peek(0)) && IS_NUMBER(peek(1))) {
          double b = AS_NUMBER(pop());
          double a = AS_NUMBER(pop());
          push(NUMBER_VAL(a + b));
        } else {
          runtimeError("Operands must be two numbers or two strings.");
          return INTERPRET_RUNTIME_ERROR;
        }
        break;
      }

      case OP_SUBTRACT:
        BINARY_OP(-);
        break;

      case OP_MULTIPLY:
        BINARY_OP(*);
        break;

      case OP_DIVIDE:
        BINARY_OP(/);
        break;

      case OP_NEGATE:
        if (!IS_NUMBER(peek(0))) {
          runtimeError("Operand must be a number.");
          return INTERPRET_RUNTIME_ERROR;
        }
        vm.stackTop[-1].as.number = -vm.stackTop[-1].as.number;
        break;

      case OP_RETURN: {
        printValue(pop());
        printf("\n");
        return INTERPRET_OK;
      }
    }
  }

#undef READ_BYTE
#undef READ_CONSTANT
#undef READ_CONSTANT_LONG
#undef BINARY_OP
}

InterpretResult interpret(Chunk* chunk) {
  vm.chunk = chunk;
  vm.ip = vm.chunk->code;
  return run();
}