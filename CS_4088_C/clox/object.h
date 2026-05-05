#ifndef clox_object_h
#define clox_object_h

#include "common.h"
#include "value.h"
#include "chunk.h"
#include "table.h"

typedef enum {
  OBJ_CLOSURE,
  OBJ_FUNCTION,
  OBJ_NATIVE,
  OBJ_STRING,
  OBJ_CLASS,
  OBJ_INSTANCE,
} ObjType;

struct Obj {
  ObjType type;
  bool isMarked;
  int refCount;
  struct Obj* next;
};

typedef bool (*NativeFn)(int argCount, Value* args, Value* result);

typedef struct {
  Obj obj;
  NativeFn function;
  int arity;
} ObjNative;

typedef struct ObjString {
  Obj obj;
  int length;
  char* chars;
  uint32_t hash;
} ObjString;

typedef struct {
  Obj obj;
  int arity;
  int upvalueCount;
  Chunk chunk;
  ObjString* name;
} ObjFunction;

typedef struct {
  Obj obj;
  ObjFunction* function;
} ObjClosure;

typedef struct {
  Obj obj;
  ObjString* name;
  Table methods;
  ObjClosure* initializer;
} ObjClass;

typedef struct {
  Obj obj;
  ObjClass* klass;
  Table fields;
} ObjInstance;

typedef struct {
  ObjClass* cachedClass;
  ObjString* cachedName;
  ObjClosure* cachedMethod;
} InlineCache;

typedef struct {
  ObjString* fieldName;
  ObjClass* owner;
} FieldOwner;

typedef struct {
  ObjClass* klass;
  ObjString* methodName;
  ObjClosure* method;
  int classVersion;
} MethodCache;

typedef struct {
  Obj obj;
  ObjString* name;
  Table methods;
  int version;
} ObjClass;

#define SMALL_STRING_MAX 6

typedef struct {
  uint8_t length;
  char chars[SMALL_STRING_MAX];
} SmallString;

// Example idea inside Value
typedef union {
  double number;
  Obj* obj;
  SmallString smallStr;
} Value;


#define OBJ_TYPE(value) (AS_OBJ(value)->type)
#define IS_CLASS(value)    isObjType(value, OBJ_CLASS)
#define IS_INSTANCE(value) isObjType(value, OBJ_INSTANCE)

#define AS_CLASS(value)    ((ObjClass*)AS_OBJ(value))
#define AS_INSTANCE(value) ((ObjInstance*)AS_OBJ(value))


static inline bool isObjType(Value value, ObjType type) {
  return IS_OBJ(value) && AS_OBJ(value)->type == type;
}

#define IS_FUNCTION(value) isObjType(value, OBJ_FUNCTION)
#define IS_CLOSURE(value)  isObjType(value, OBJ_CLOSURE)
#define IS_NATIVE(value)   isObjType(value, OBJ_NATIVE)
#define IS_STRING(value)   isObjType(value, OBJ_STRING)

#define AS_FUNCTION(value)   ((ObjFunction*)AS_OBJ(value))
#define AS_CLOSURE(value)    ((ObjClosure*)AS_OBJ(value))
#define AS_NATIVE_OBJ(value) ((ObjNative*)AS_OBJ(value))
#define AS_NATIVE(value)     (((ObjNative*)AS_OBJ(value))->function)
#define AS_STRING(value)     ((ObjString*)AS_OBJ(value))
#define AS_CSTRING(value)    (((ObjString*)AS_OBJ(value))->chars)

ObjFunction* newFunction(void);
ObjClosure* newClosure(ObjFunction* function);
ObjNative* newNative(NativeFn function, int arity);

ObjString* copyString(const char* chars, int length);
ObjString* takeString(char* chars, int length);

ObjClass* newClass(ObjString* name);
ObjInstance* newInstance(ObjClass* klass);

void printObject(Value value);

#endif