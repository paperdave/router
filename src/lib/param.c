#include "param.h"
#include <string.h>

parse_param_t parse_param(const char* part, size_t part_len) {
  parse_param_t parse;
  parse.name = NULL;
  parse.error = false;

  int i = 0;
  while (i < part_len) {
    // colon params must be at the start of a route segment, like /api/:id/etc
    if (part[i] == ':') {
      if (part[i - 1] != '/') {
        parse.error = true;
        parse.name = "Invalid route: colon params cannot be placed in the middle of a route";
        return parse;
      }

      int j = i + 1;
      while (j < part_len && part[j] != '/') {
        j++;
        // if (part[j] < 'a' || part[j] > 'z' || part[j] < 'A' || part[j] > 'Z' || part[j] < '0' || part[j] > '9' || part[j] != '_' || part[j] != '$') {
        //   parse.error = true;
        //   parse.name = "Invalid route: param names can only contain [a-zA-Z0-9_$]";
        //   return parse;
        // }
      }
      parse.name = malloc(sizeof(char) * (j - i));
      memcpy(parse.name, part + i + 1, j - i - 1);
      parse.name[j - i - 1] = 0;
      parse.start = i;
      parse.end = j - 1;
      return parse;
    }

    // bracket params can be one of the following forms:
    // - /api/[id]/etc
    // - /api/[...id] (must be at end of route)
    // - /api/[[...id]] (must be at end of route)
    // unlike colon params, bracket params can be in the middle of a route
    // only error thrown is if the [ dont match up
    if (part[i] == '[') {
      if(part[i + 1] == '[') {
        if (part[i + 2] != '.' || part[i + 3] != '.' || part[i + 4] != '.') {
          parse.error = true;
          parse.name = "Invalid route: could not parse optional bracket wildcard";
          return parse;
        }
        int j = i + 5;
        while (j < part_len && part[j] != ']') {
          j++;
        }
        if (j > part_len - 1 || part[j] != ']' || part[j + 1] != ']') {
          parse.error = true;
          parse.name = "Invalid route: could not parse optional bracket wildcard";
          return parse;
        }
        if (j != part_len - 2) {
          parse.error = true;
          parse.name = "Invalid route: optional bracket wildcards must be at the end of a route";
          return parse;
        }
        parse.name = malloc(sizeof(char) * (j - i - 5));
        memcpy(parse.name, part + i + 5, j - i - 5);
        parse.name[j - i - 5] = 0;
        parse.start = i;
        parse.end = j + 2;
        parse.wildcard = true;
        return parse;
      }
      if (part[i + 1] == '.') {
        if (part[i + 2] != '.' || part[i + 3] != '.') {
          parse.error = true;
          parse.name = "Invalid route: could not parse bracket wildcard";
          return parse;
        }
        int j = i + 4;
        while (j < (part_len) && part[j] != ']') {
          j++;
        }
        if (j == (part_len) || part[j] != ']') {
          parse.error = true;
          parse.name = "Invalid route: could not parse bracket wildcard";
          return parse;
        }
        if (j != part_len - 1) {
          parse.error = true;
          parse.name = "Invalid route: bracket wildcards must be at the end of a route";
          return parse;
        }
        parse.name = malloc(sizeof(char) * (j - i - 4));
        memcpy(parse.name, part + i + 4, j - i - 4);
        parse.name[j - i - 4] = 0;
        parse.start = i;
        parse.end = j + 1;
        parse.wildcard = true;
        return parse;
      }
      int j = i + 1;
      while (j < part_len && part[j] != ']') {
        j++;
      }
      if (j == part_len) {
        parse.error = true;
        parse.name = "Invalid route: could not parse bracket param";
        return parse;
      }
      parse.name = malloc(sizeof(char) * (j - i - 1));
      memcpy(parse.name, part + i + 1, j - i - 1);
      parse.name[j - i - 1] = 0;
      parse.start = i;
      parse.end = j;
      return parse;
    }
    
    // wildcard params can only be at the end of a route segment, like /api/*
    if (part[i] == '*') {
      if(i != part_len - 1) {
        parse.error = true;
        parse.name = "Invalid route: wildcard params can only be at the end of a route segment";
        return parse;
      }
      parse.name = malloc(sizeof(char) * 2);
      parse.name[0] = '*';
      parse.name[1] = 0;
      parse.start = i;
      parse.end = i + 1;
      parse.wildcard = true;
      return parse;
    }
    i++;
  }
  return parse;
}
