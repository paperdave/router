#include "param-buffer.h"
#include <stdlib.h>

char* parambuf;

void* parambuf_init() {
  parambuf = malloc(RETURN_BUFFER_SIZE);
  return parambuf;
}

void parambuf_free() {
  free(parambuf);
}
