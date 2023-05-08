#pragma once
#include <stdbool.h>
#include "tree.h"

/** call once to setup. you are returned a pointer to the "param buffer" */
void* parambuf_init();
void parambuf_free();

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
