#include <stdlib.h>

void *malloc(uint16_t len)
{
  if (len > sbrkmax()) {
    printf("OUT OF MEM: %d > %d\n", len, sbrkmax());
    exit(1);
  }
  return sbrk(len);
}
