#ifndef FILECMD_H
#define FILECMD_H

#include "testing.h"

#define OPEN_READ    0x40
#define OPEN_WRITE   0x80

extern bool file_command(TestCommand *cmd, void *data, void *reply, size_t reply_max);

#endif /* FILECMD_H */
