#include "tree.h"
#include <string.h>

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

node_t* node_new_unnamed(route_id_t route_id) {
  node_t* node = calloc(1, sizeof(node_t));
  node->id = route_id;
  return node;
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
