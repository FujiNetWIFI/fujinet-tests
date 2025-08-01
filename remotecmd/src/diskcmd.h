#ifndef DISKCMD_H
#define DISKCMD_H

#include "command.h"
#include <stdbool.h>
#include <stdlib.h>

extern bool disk_command(TestCommand *cmd, void *data, void *reply, size_t reply_max);

#endif /* DISKCMD_H */
