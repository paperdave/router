// HTTP router for JS written in C using FFI.
// Based on memoirist by SaltyAOM, which is a fork of @medley/router by nwoltman
// Uses a Radix Tree to store routes and their handlers.
#include "native-router.h"
#include "jserror.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define parse_param_t struct parse_param
struct parse_param {
  char* name;
  bool wildcard;
  int start;
  int end;
  bool error;
};

#define RETURN_BUFFER_SIZE 2048
char* parambuf;

void* parambuf_init() {
  parambuf = malloc(RETURN_BUFFER_SIZE);
  return parambuf;
}

void parambuf_free() {
  free(parambuf);
}

radix_t char_to_radix[256] = {
  // first 32 chars are not used
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 0x00-0x0f
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 0x10-0x1f
  // # is not url safe, * is internal
  0,  1,  2,  -1, 3,  4,  5,  6,  7,  8,  -1,  9, 10, 11, 12, 13,       // 0x20-0x2f
  // ? is not url safe, and : is internal
  14, 15, 16, 17, 18, 19, 20, 21, 22, 23, -1, 24, 25, 26, 27, -1, // 0x30-0x3f
  28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, // 0x40-0x4f
  44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59,  // 0x50-0x5f
  // alphabet comes around again, but lowercase
  60, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, // 0x60-0x6f
  44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, -1, -1, -1, -1, -1, // 0x70-0x7f
};

router_t* router_new() {
  router_t* router = malloc(sizeof(router_t));
  memset(router->children, 0, sizeof(router->children));
  return router;
}

node_t* node_new(char* part, route_id_t id, size_t part_len) {
  node_t* node = calloc(1, sizeof(node_t));
  node->part = malloc(sizeof(char) * part_len);
  memcpy(node->part, part, part_len);
  node->part_len = part_len;
  node->id = id;
  return node;
}

#define maybe_node_t struct maybe_node
struct maybe_node {
  node_t* node;
  char* error;
};

node_t* node_new_unnamed(route_id_t route_id) {
  node_t* node = calloc(1, sizeof(node_t));
  node->id = route_id;
  return node;
}

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

int node_add(node_t** ref, route_id_t id, char* route, size_t route_len);

int node_insert_and_parse_params(node_t** ref, char* part, route_id_t id, size_t part_len) {
  parse_param_t param = parse_param(part, part_len);
  if (param.name == NULL) {
    *ref = node_new(part, id, part_len);
    return true;
  } else if(param.error) {
    return jserror(param.name);
  }

  if (param.start == 0) {
    *ref = node_new_unnamed(id);
    free(param.name);
    return true;
  }
  node_t* node = node_new(part, 0, param.start - 1);
  *ref = node;
  if (param.wildcard) {
    node->wildcard = node_new_unnamed(id);
  } else {
    node_t* param_node = calloc(1, sizeof(node_t));
    node->param = param_node;
    if(param.end == part_len) {
      param_node->id = id;
    } else {
      node_add(&node->param, id, part + param.end + 1, part_len - param.end - 1);
    }
  }
  free(param.name);
  return true;
}

int node_insert(node_t** ref, char* part, route_id_t id, size_t part_len) {
  *ref = node_new(part, id, part_len);
  return true;
}

void node_free(node_t* node) {
  if (node->part != NULL) {
    free(node->part);
  }
  for (int i = 0; i < 64; i++) {
    if (node->children[i] != NULL) {
      node_free(node->children[i]);
    }
  }
  free(node);
}

void router_free(router_t* router) {
  for (int i = 0; i < 64; i++) {
    if (router->children[i] != NULL) {
      node_free(router->children[i]);
    }
  }
  free(router);
}

char stupid_radix_to_char(radix_t radix) {
  for (int i = 0; i < 256; i++) {
    if (char_to_radix[i] == radix) {
      return i;
    }
  }
  return '#';
}

void node_print(node_t* node, int depth, radix_t last);

void node_print_wild(node_t* node, int depth, char* last) {
  for (int i = 0; i < depth; i++) {
    printf("  ");
  }
  printf("type=%s, id=%d\n", last, node->id);
  for (int i = 0; i < 61; i++) {
    if (node->children[i] != NULL) {
      node_print(node->children[i], depth + 1, i);
    }
  }
  if(node->wildcard != NULL) {
    node_print_wild(node->wildcard, depth + 1, "wildcard");
  }
  if(node->param != NULL) {
    node_print_wild(node->param, depth + 1, "param");
  }
}

void node_print(node_t* node, int depth, radix_t last) {
  for (int i = 0; i < depth; i++) {
    printf("  ");
  }
  char* part = malloc(node->part_len + 1);
  memcpy(part, node->part, node->part_len);
  part[node->part_len] = 0;
  printf("radix=%c part=\"%s\", part_len=%d, id=%d\n", stupid_radix_to_char(last), part, node->part_len, node->id);
  for (int i = 0; i < 61; i++) {
    if (node->children[i] != NULL) {
      node_print(node->children[i], depth + 1, i);
    }
  }
  if(node->wildcard != NULL) {
    node_print_wild(node->wildcard, depth + 1, "wildcard");
  }
  if(node->param != NULL) {
    node_print_wild(node->param, depth + 1, "param");
  }
}

void router_print(router_t* router) {
  for (int i = 0; i < 61; i++) {
    if (router->children[i] != NULL) {
      node_print(router->children[i], 0, i);
    }
  }
}


node_t* node_create_split(node_t* node, route_id_t id, int index, char* second, size_t second_len) {
  node_t* shared_parent = node_new(node->part, 0, index);

  // put the current node as a child of the new parent.
  shared_parent->children[char_to_radix[node->part[index]]] = node;
  char* newPart = malloc(sizeof(char) * (node->part_len - index));
  memcpy(newPart, node->part + index + 1, node->part_len - index - 1);
  free(node->part);
  node->part = newPart;
  node->part_len -= index + 1;

  // put the new node as a child of the new parent
  if (second != NULL) {
    node_insert(&shared_parent->children[char_to_radix[second[0]]], second, id, second_len - 1);
  } else {
    shared_parent->id = id;
  }

  return shared_parent;
}

int node_add(node_t** ref, route_id_t id, char* route, size_t route_len) {
  node_t* node = *ref;
  int offset = 0;
  int i = 0;

  parse_param_t param = parse_param(route, route_len);
  if (param.error) {
    return jserror(param.name);
  }

  int end = route_len;
  int setId = id;
  int ret;

  if(param.name != NULL) {
    end = param.start;
    setId = 0;
    // printf("param %s, from %d to %d, %c\n", param.name, param.start, param.end, route[param.start]);
  }

  while (i < end) {
    radix_t inner_radix = char_to_radix[route[i]];
    if (inner_radix == -1) return jserror("Invalid route: disallowed characters in route");

    // if we are at the end of the node, we need to go deeper
    if (offset == node->part_len) {
      if(node->children[inner_radix] == NULL) {
        ret = node_insert(&node->children[inner_radix], route + i + 1, setId, route_len - i - 1);
        goto after;
      } else {
        ret = node_add(&node->children[inner_radix], setId, route + i + 1, route_len - i - 1);
        goto after;
      }
    }

    // if we differ from the node, we split
    if (route[i] != node->part[offset]) {
      *ref = node_create_split(node, setId, offset, route + i, route_len - i);
      ret = true;
      goto after;
    }

    i++;
    offset++;
  }

  ret = true;

  // we are in the middle of a node and our path is shorter
  if (offset < node->part_len) {
    *ref = node_create_split(node, id, offset, NULL, 0);
    goto after;
  }

  if (param.name == NULL) {
    // we are at the end of a node
    if (node->id != 0) return jserror("Duplicate route");
    node->id = id;
    goto after;
  }

  after:
  if (ret != true) return ret;

  if (param.name == NULL) {
    return true;
  }

  node = *ref;
  if (node->param == NULL) {
    node_t* param_node = node_new_unnamed(0);
    node->param = param_node;
    if(route_len == param.end + 1) {
      param_node->id = id;
      return true;
    } else {
      radix_t inner_radix = char_to_radix[route[param.end + 1]];
      return node_insert_and_parse_params(&param_node->children[inner_radix], route + param.end + 2, id, strlen(route) - param.end - 1);
    }
    return true;
  }

  return true;
}

int router_add(router_t* router, char* route, route_id_t id) {
  parambuf[0] = 0;
  radix_t radix = char_to_radix[route[0]];
  if (radix == -1) return jserror("Invalid route: disallowed characters in route");

  if (router->children[radix] == NULL) {
    return node_insert_and_parse_params(&router->children[radix], route + 1, id, strlen(route) - 1);
  }

  return node_add(&router->children[radix], id, route + 1, strlen(route) - 1);
}

route_id_t router_find(router_t* router, char* path) {
  return jserror("Not Implemented");
}

int main() {
  parambuf_init();

  router_t* router = router_new();
  router_add(router, "GET/users/", 1);
  router_add(router, "GET/users/:id/name/:again", 2);
  router_print(router);
  // router_free(router);

  // printf("---\n");

  // router = router_new();
  // router_add(router, "GET/users/:id", 2);
  // router_print(router);
  // router_add(router, "GET/users/", 1);
  // router_print(router);
  // // router_free(router);

  return 0;
}
