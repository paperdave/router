#pragma once

#define RETURN_BUFFER_SIZE 2048

extern char* parambuf;

void* parambuf_init();
void parambuf_free();
