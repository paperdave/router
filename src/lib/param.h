// The file handles parsing route parameters
#pragma once
#include <stdlib.h>
#include <stdbool.h>

#define parse_param_t struct parse_param
struct parse_param {
  char* name;
  bool wildcard;
  int start;
  int end;
  bool error;
};

parse_param_t parse_param(const char* part, size_t part_len);
