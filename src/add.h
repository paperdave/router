#pragma once
#include <stdbool.h>
#include "tree.h"

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
