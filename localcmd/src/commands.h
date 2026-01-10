#ifndef COMMANDS_H
#define COMMANDS_H

#include <fujinet-fuji.h> // for uint8_t

typedef struct {
  char *name;
  uint8_t type;
  uint16_t size;
} FujiArg;

typedef struct FujiCommand {
  uint8_t command;
  char *name;
  uint8_t argCount;
  FujiArg *args;
  FujiArg reply;
  struct FujiCommand *next;
} FujiCommand;

extern uint8_t load_commands(const char *path);
extern FujiCommand *find_command(const char *name);

#endif /* COMMANDS_H */
