#include "common.h"
#include "chunk.h"
#include "debug.h"
#include "vm.h"

int main(int argc, const char* argv[]) {
  (void)argc;
  (void)argv;

  initVM();

  Chunk chunk;
  initChunk(&chunk);

  int constant;
  // Add 1 * 2 + 3   
  /*
     constant = addConstant(&chunk, 1);
     writeChunk(&chunk, OP_CONSTANT, 123);
     writeChunk(&chunk, constant, 123);

     constant = addConstant(&chunk, 2);
     writeChunk(&chunk, OP_CONSTANT, 123);
     writeChunk(&chunk, constant, 123);

     writeChunk(&chunk, OP_MULTIPLY, 123);

     constant = addConstant(&chunk, 3);
     writeChunk(&chunk, OP_CONSTANT, 123);
     writeChunk(&chunk, constant, 123);

     writeChunk(&chunk, OP_ADD, 123);
     writeChunk(&chunk, OP_RETURN, 123);
     */
     
     // Add 1 + 2 * 3
     /*
     constant = addConstant(&chunk, 1);
     writeChunk(&chunk, OP_CONSTANT, 123);
     writeChunk(&chunk, constant, 123);

     constant = addConstant(&chunk, 2);
     writeChunk(&chunk, OP_CONSTANT, 123);
     writeChunk(&chunk, constant, 123);

     constant = addConstant(&chunk, 3);
     writeChunk(&chunk, OP_CONSTANT, 123);
     writeChunk(&chunk, constant, 123);

     writeChunk(&chunk, OP_MULTIPLY, 123);
     writeChunk(&chunk, OP_ADD, 123);
     writeChunk(&chunk, OP_RETURN, 123);
     */

     // Add 3 - 2 - 1
     /*
     constant = addConstant(&chunk, 3);
     writeChunk(&chunk, OP_CONSTANT, 123);
     writeChunk(&chunk, constant, 123);

     constant = addConstant(&chunk, 2);
     writeChunk(&chunk, OP_CONSTANT, 123);
     writeChunk(&chunk, constant, 123);

     writeChunk(&chunk, OP_SUBTRACT, 123);

     constant = addConstant(&chunk, 1);
     writeChunk(&chunk, OP_CONSTANT, 123);
     writeChunk(&chunk, constant, 123);

     writeChunk(&chunk, OP_SUBTRACT, 123);
     writeChunk(&chunk, OP_RETURN, 123);

     disassembleChunk(&chunk, "test chunk");
     interpret(&chunk);
     */

     //Add 1 + 2 * 3 - 4 / -5
      /*
     constant = addConstant(&chunk, 1);
     writeChunk(&chunk, OP_CONSTANT, 123);
     writeChunk(&chunk, constant, 123);

     constant = addConstant(&chunk, 2);
     writeChunk(&chunk, OP_CONSTANT, 123);
     writeChunk(&chunk, constant, 123);

     constant = addConstant(&chunk, 3);
     writeChunk(&chunk, OP_CONSTANT, 123);
     writeChunk(&chunk, constant, 123);

     writeChunk(&chunk, OP_MULTIPLY, 123);   // 2 * 3
     writeChunk(&chunk, OP_ADD, 123);        // 1 + (2 * 3)

     constant = addConstant(&chunk, 4);
     writeChunk(&chunk, OP_CONSTANT, 123);
     writeChunk(&chunk, constant, 123);

     constant = addConstant(&chunk, 5);
     writeChunk(&chunk, OP_CONSTANT, 123);
     writeChunk(&chunk, constant, 123);

     writeChunk(&chunk, OP_NEGATE, 123);     // -5
     writeChunk(&chunk, OP_DIVIDE, 123);     // 4 / -5

     writeChunk(&chunk, OP_SUBTRACT, 123);   // (1 + 2*3) - (4 / -5)
     writeChunk(&chunk, OP_RETURN, 123);
     */
     /*
     for (int i = 1; i <= 12; i++) {
          constant = addConstant(&chunk, i);
          writeChunk(&chunk, OP_CONSTANT, 123);
          writeChunk(&chunk, constant, 123);
     }

     for (int i = 1; i < 12; i++) {
          writeChunk(&chunk, OP_ADD, 123);
     }

     writeChunk(&chunk, OP_RETURN, 123);
     */
     /* Example: "st" + "ri" + "ng" */
     writeConstant(&chunk, OBJ_VAL(copyString("st", 2)), 123);
     writeConstant(&chunk, OBJ_VAL(copyString("ri", 2)), 123);
     writeChunk(&chunk, OP_ADD, 123);

     writeConstant(&chunk, OBJ_VAL(copyString("ng", 2)), 123);
     writeChunk(&chunk, OP_ADD, 123);

     writeChunk(&chunk, OP_RETURN, 123);

     disassembleChunk(&chunk, "test chunk");
     interpret(&chunk);
     freeVM();
     freeChunk(&chunk);
     return 0;
}