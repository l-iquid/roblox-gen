#include "internal.h"

int string_size(const char* x) {
  int sz = -1;
  while (x[++sz] != '\0');
  return sz;
}
int string_compare(const char* x, const char* y) {
  int i = -1;
  while(x[++i] != '\0' && x[i] == y[i]);
  return string_size(y) == i;
}