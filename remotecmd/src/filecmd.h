#ifndef FILECMD_H
#define FILECMD_H

#include "command.h"
#include <stdbool.h>
#include <stdlib.h>

#define OPEN_READ    0x40
#define OPEN_WRITE   0x80

typedef struct {
  uint16_t length;
  uint16_t unused;
  uint8_t data[];
} ReadReply;

extern bool file_command(TestCommand *cmd, void *data, void *reply, size_t reply_max);

#endif /* FILECMD_H */
