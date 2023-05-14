#include "find.h"
#include "param-buffer.h"
#include <stdlib.h>
#include <string.h>

route_id_t router_find(router_t* router, char* path) {
  parambuf[0] = 0;

  size_t route_len = strlen(path);
  if (route_len == 0) return 0;

  radix_t radix = char_to_radix[path[0]];
  node_t* node = router->children[radix];
  if (node == NULL) {
    return 0;
  }

  return 1;
}
