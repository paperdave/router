#include "print.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void node_print(node_t* node, int depth, radix_t last);
void node_print_wild(node_t* node, int depth, char* last);

char stupid_radix_to_char(radix_t radix) {
  for (int i = 256; i > 0; i--) {
    if (char_to_radix[i] == radix) {
      return i;
    }
  }
  return '#';
}

void node_print_wild(node_t* node, int depth, char* last) {
  for (int i = 0; i < depth; i++) {
    printf("  ");
  }
  printf("type=%s, id=%d\n", last, node->id);
  for (int i = 0; i < 61; i++) {
    if (node->children[i] != NULL) {
      node_print(node->children[i], depth + 1, i);
    }
  }
  if(node->wildcard != NULL) {
    node_print_wild(node->wildcard, depth + 1, "wildcard");
  }
  if(node->param != NULL) {
    node_print_wild(node->param, depth + 1, "param");
  }
}

void node_print(node_t* node, int depth, radix_t last) {
  for (int i = 0; i < depth; i++) {
    printf("  ");
  }
  char* part = malloc(node->part_len + 1);
  memcpy(part, node->part, node->part_len);
  part[node->part_len] = 0;
  printf("radix=%c part=\"%s\", part_len=%d, id=%d\n", stupid_radix_to_char(last), part, node->part_len, node->id);
  for (int i = 0; i < 61; i++) {
    if (node->children[i] != NULL) {
      node_print(node->children[i], depth + 1, i);
    }
  }
  if(node->wildcard != NULL) {
    node_print_wild(node->wildcard, depth + 1, "wildcard");
  }
  if(node->param != NULL) {
    node_print_wild(node->param, depth + 1, "param");
  }
}

void router_print(router_t* router) {
  for (int i = 0; i < 61; i++) {
    if (router->children[i] != NULL) {
      node_print(router->children[i], 0, i);
    }
  }
}

char* node_print_json(node_t* node) {
  char *buf = malloc(4096);
  int len = 0;
  len += sprintf(buf + len, "{");
  char* part = malloc(node->part_len + 1);
  memcpy(part, node->part, node->part_len);
  part[node->part_len] = 0;
  len += sprintf(buf + len, "\"id\":%d,", node->id);
  len += sprintf(buf + len, "\"part\":\"%s\",", part);
  len += sprintf(buf + len, "\"children\":{");
  bool first = true;
  for (int i = 0; i < 61; i++) {
    if (node->children[i] != NULL) {
      if (first) {
        first = false;
      } else {
        len += sprintf(buf + len, ",");
      }
      len += sprintf(buf + len, "\"%c\":", stupid_radix_to_char(i));
      char* json = node_print_json(node->children[i]);
      len += sprintf(buf + len, "%s", json);
      free(json);
    }
  }
  len += sprintf(buf + len, "}");
  if(node->wildcard != NULL) {
    len += sprintf(buf + len, ",\"wildcard\":");
    char* json = node_print_json(node->wildcard);
    len += sprintf(buf + len, "%s", json);
    free(json);
  }
  if(node->param != NULL) {
    len += sprintf(buf + len, ",\"param\":");
    char* json = node_print_json(node->param);
    len += sprintf(buf + len, "%s", json);
    free(json);
  }
  len += sprintf(buf + len, "}");
  return buf;
}

char* router_print_json(router_t* router) {
  char *buf = malloc(4096);
  int len = 0;
  len += sprintf(buf + len, "{");
  bool first = true;
  for (int i = 0; i < 61; i++) {
    if (router->children[i] != NULL) {
      if (first) {
        first = false;
      } else {
        len += sprintf(buf + len, ",");
      }
      len += sprintf(buf + len, "\"%c\":", stupid_radix_to_char(i));
      char* json = node_print_json(router->children[i]);
      len += sprintf(buf + len, "%s", json);
      free(json);
    }
  }
  len += sprintf(buf + len, "}");
  return buf;
}
