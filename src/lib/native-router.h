#pragma once
#include <stdbool.h>

typedef char radix_t;
typedef unsigned int route_id_t;

#define node_t struct node
struct node {
  char* part; // the part of this route
  unsigned int part_len; // the length of the part
  route_id_t id; // the id of a route handler, if exists. 0 if none.
  node_t* children[61]; // 61 is the number of possible radixes
  node_t* param; // if a param exists, it is stored here
  node_t* wildcard; // if a param exists, it is stored here
};

#define router_t struct router
struct router {
  node_t* children[61];
};

/** call once to setup. you are returned a pointer to the "param buffer" */
void* parambuf_init();
void parambuf_free();

/** create a new router */
router_t* router_new();

/** free a router */
void router_free(router_t* router);

/** add a route to a router
 * 
 *  routeId must start with a `/` and can supports the following:
 *  - params: `/users/:id` or `/users/[id]/name`
 *  - wildcards: `/users/ *` (only at the end of a route)
 * 
 *  returns 0 if the route was added, -1 if an error happens
 * 
 *  on success, the param buffer contains an array of strings of param names
 *  <array length><strlen><string><strlen><string>...
 */
int router_add(router_t* router, char* route, route_id_t id);

/** lookup a route in a router
 * 
 *  returns the id of the route handler, or 0 if not found. -1 is returned if an error happens.
 * 
 *  on success, the param buffer contains an array of strings of the param values
 *  <array length><strlen><string><strlen><string>...
 */
route_id_t router_find(router_t* router, char* path);

/** print a router to stdout */
void router_print(router_t* router);
