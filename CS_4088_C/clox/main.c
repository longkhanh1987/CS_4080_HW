#include "common.h"
#include "chunk.h"
#include "debug.h"
#include "object.h"
#include "value.h"
#include "vm.h"

int main(int argc, const char* argv[]) {
  (void)argc;
  (void)argv;

  initVM();

  Chunk chunk;
  initChunk(&chunk);

  // Example: "st" + "ri" + "ng"
  writeConstant(&chunk, OBJ_VAL(copyString("st", 2)), 123);
  writeConstant(&chunk, OBJ_VAL(copyString("ri", 2)), 123);
  writeChunk(&chunk, OP_ADD, 123);

  writeConstant(&chunk, OBJ_VAL(copyString("ng", 2)), 123);
  writeChunk(&chunk, OP_ADD, 123);

  writeChunk(&chunk, OP_RETURN, 123);

  disassembleChunk(&chunk, "test chunk");
  interpret(&chunk);

  freeChunk(&chunk);
  freeVM();

  return 0;
}