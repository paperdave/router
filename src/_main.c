#include "tree.h"
#include "param-buffer.h"
#include "add.h"
#include "print.h"

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
