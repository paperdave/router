// This file contains the tree data structure used by the router,
// It does not handle manipulation, just alloc/free of them.
#pragma once
#include <stdbool.h>
#include <stdlib.h>

typedef short radix_t;
extern radix_t char_to_radix[256];

typedef unsigned int route_id_t;

#define node_t struct node
struct node {
  char* part;            // the part of this route
  unsigned int part_len; // the length of the part
  route_id_t id;         // the id of a route handler, if exists. 0 if none.
  node_t* children[61];  // 61 is the number of possible radixes
  node_t* param;         // if a param exists, it is stored here
  node_t* wildcard;      // if a param exists, it is stored here
};

#define router_t struct router
struct router {
  node_t* children[61];
};

router_t* router_new();
void router_free(router_t* router);

node_t* node_new(char* part, route_id_t id, size_t part_len);
node_t* node_new_unnamed(route_id_t route_id);
void node_free(node_t* node);
