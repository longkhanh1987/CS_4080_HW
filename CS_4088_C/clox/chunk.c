#include <stdlib.h>

#include "chunk.h"
#include "memory.h"

void initChunk(Chunk* chunk) {
  chunk->count = 0;
  chunk->capacity = 0;
  chunk->code = NULL;

  initValueArray(&chunk->constants);

  chunk->linesCount = 0;
  chunk->linesCapacity = 0;
  chunk->lines = NULL;
}

void freeChunk(Chunk* chunk) {
  FREE_ARRAY(uint8_t, chunk->code, chunk->capacity);
  freeValueArray(&chunk->constants);
  FREE_ARRAY(LineStart, chunk->lines, chunk->linesCapacity);
  initChunk(chunk);
}

static void writeLineInfo(Chunk* chunk, int line) {
  if (chunk->linesCount > 0 &&
      chunk->lines[chunk->linesCount - 1].line == line) {
    chunk->lines[chunk->linesCount - 1].count++;
    return;
  }

  if (chunk->linesCapacity < chunk->linesCount + 1) {
    int oldCapacity = chunk->linesCapacity;
    chunk->linesCapacity = GROW_CAPACITY(oldCapacity);
    chunk->lines = GROW_ARRAY(LineStart, chunk->lines,
                              oldCapacity, chunk->linesCapacity);
  }

  chunk->lines[chunk->linesCount].line = line;
  chunk->lines[chunk->linesCount].count = 1;
  chunk->linesCount++;
}

void writeChunk(Chunk* chunk, uint8_t byte, int line) {
  if (chunk->capacity < chunk->count + 1) {
    int oldCapacity = chunk->capacity;
    chunk->capacity = GROW_CAPACITY(oldCapacity);
    chunk->code = GROW_ARRAY(uint8_t, chunk->code,
                             oldCapacity, chunk->capacity);
  }

  chunk->code[chunk->count] = byte;
  chunk->count++;

  writeLineInfo(chunk, line);
}

int addConstant(Chunk* chunk, Value value) {
  writeValueArray(&chunk->constants, value);
  return chunk->constants.count - 1;
}

int getLine(Chunk* chunk, int instruction) {
  int accumulated = 0;

  for (int i = 0; i < chunk->linesCount; i++) {
    accumulated += chunk->lines[i].count;
    if (instruction < accumulated) {
      return chunk->lines[i].line;
    }
  }

  return -1;
}

void writeConstant(Chunk* chunk, Value value, int line) {
  int constant = addConstant(chunk, value);

  if (constant <= 0xFF) {
    writeChunk(chunk, OP_CONSTANT, line);
    writeChunk(chunk, (uint8_t)constant, line);
  } else if (constant <= 0xFFFFFF) {
    writeChunk(chunk, OP_CONSTANT_LONG, line);
    writeChunk(chunk, (uint8_t)(constant & 0xFF), line);
    writeChunk(chunk, (uint8_t)((constant >> 8) & 0xFF), line);
    writeChunk(chunk, (uint8_t)((constant >> 16) & 0xFF), line);
  } else {
    exit(1);
  }
}