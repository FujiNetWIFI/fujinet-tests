#ifndef COMMAND_H
#define COMMAND_H

#include <fujinet-commands.h>
#include <stdint.h>

typedef struct {
  uint8_t device;
  uint8_t command;
  uint8_t flags;
  uint8_t aux1, aux2, aux3, aux4;
  uint16_t data_len, reply_len;
} TestCommand;

#endif /* COMMAND_H */
