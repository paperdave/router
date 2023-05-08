#include "error.h"
#include <stdlib.h>
#include <stdio.h>

char* error_str = NULL;

int jserror(char* str) {
  if(error_str != NULL) free(error_str);
  // printf("jserror: %s\n", str);
  error_str = str;
  return -1;
}

char* jserror_get() {
  return error_str;
}
