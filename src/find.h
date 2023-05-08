#pragma once
#include "tree.h"

/** lookup a route in a router
 * 
 *  returns the id of the route handler, or 0 if not found. -1 is returned if an error happens.
 * 
 *  on success, the param buffer contains an array of strings of the param values
 *  <array length><strlen><string><strlen><string>...
 */
route_id_t router_find(router_t* router, char* path);
