// HTTP router for JS written in C using FFI.
// Based on memoirist by SaltyAOM, which is a fork of @medley/router by nwoltman
// Uses a Radix Tree to store routes and their handlers.
#include "native-router.h"
#include "jserror.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <string.h>
#include "param.h"
#include "tree.h"
#include "print.h"

#define RETURN_BUFFER_SIZE 2048
char* parambuf;

void* parambuf_init() {
  parambuf = malloc(RETURN_BUFFER_SIZE);
  return parambuf;
}

void parambuf_free() {
  free(parambuf);
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
  size_t route_len = strlen(route);
  for (int i = 0; i < route_len; i++) {
    // if (char_to_radix[route[i]] == -1) return jserror("Invalid route: disallowed characters in route");
    if (route[i] >= 'A' && route[i] <= 'Z') {
      route[i] += 32;
    }
  }
  radix_t radix = char_to_radix[route[0]];

  if (router->children[radix] == NULL) {
    return node_insert_and_parse_params(&router->children[radix], route + 1, id, route_len - 1);
  }

  return node_add(&router->children[radix], id, route + 1, route_len - 1);
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
