// This file handles printing the tree to the console and serializing it to JSON
#pragma once
#include "tree.h"

void router_print(router_t* router);
char* router_print_json(router_t* router);
