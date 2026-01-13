#include "commands.h"
#include "json.h"
#include "platform.h"

#ifndef _CMOC_VERSION_
#include <stdio.h>
#include <string.h>
#include <conio.h>
#endif /* _CMOC_VERSION_ */

#define malloc(len) sbrk(len)
#define strcasecmp(x, y) stricmp(x, y)

//uint16_t num_commands = 0;
FujiCommand *fuji_commands = 0xAAAA;

void parse_command_arg(FujiArg *arg, const char *buffer)
{
  const char *p;
  uint16_t len;


  p = strchr(buffer, ':');
  if (!p) {
    printf("Invalid format string %s\n", buffer);
    exit(1);
  }

  len = p - buffer;
  arg->name = (char *) malloc(len + 1);
  strncpy(arg->name, buffer, len);
  arg->name[len] = 0;
  p++;
  arg->type = *p;
  p++;
  arg->size = atoi(p);

  return;
}

uint8_t load_commands(const char *path)
{
  uint8_t err, cnum;
  uint16_t idx, jdx;
  size_t length;
  FujiCommand *cmd, *next;
  char query[32];
  char buffer[64];


  err = json_open(path);
  if (err != FN_ERR_OK)
    return err;

  printf("Loading Fuji commands...\n");
#if 0
  for (count = 0; ; count++) {
    sprintf(query, "/%d/command", count);
    length = json_query(query, buffer);
    if (!length)
      break;
  }

  printf("Total commands: %d\n", count);
  num_commands = count;

  idx = sizeof(FujiCommand) * count;
  printf("Allocating %d bytes\n", idx);
  fuji_commands = (FujiCommand *) malloc(idx);
  printf("FUJI COMMANDS = %04X\n", fuji_commands);
#endif

  cmd = NULL;
  for (idx = 0; ; idx++) {
    sprintf(query, "/%d/command", idx);
    length = json_query(query, buffer);
    if (!length)
      break;

    cnum = (uint8_t) atoi(buffer);
    sprintf(query, "/%d/name", idx);
    length = json_query(query, buffer);
    if (!length)
      break;

    next = (FujiCommand *) malloc(sizeof(FujiCommand));
    next->next = NULL;
    if (!cmd)
      cmd = fuji_commands = next;
    else {
      cmd->next = next;
      cmd = cmd->next;
    }

    cmd->command = cnum;
    cmd->name = (char *) malloc(strlen(buffer) + 1);
    strcpy(cmd->name, buffer);

    printf("IDX %d == 0x%02x %s\n", idx, cmd->command, cmd->name);

    cmd->reply.name = NULL;

    sprintf(query, "/%d/reply-%s/0", idx, platform_name());
    length = json_query(query, buffer);
    if (length)
      parse_command_arg(&cmd->reply, buffer);
    else {
      sprintf(query, "/%d/reply/0", idx);
      length = json_query(query, buffer);
      if (length)
        parse_command_arg(&cmd->reply, buffer);
    }

    for (jdx = 0; ; jdx++) {
      sprintf(query, "/%d/args/%d", idx, jdx);
      length = json_query(query, buffer);
      if (!length)
        break;
    }

    cmd->argCount = (uint8_t) jdx;
    if (cmd->argCount) {
      cmd->args = (FujiArg *) sbrk(sizeof(FujiArg)
                                                 * cmd->argCount);
      for (jdx = 0; jdx < cmd->argCount; jdx++) {
        sprintf(query, "/%d/args/%d", idx, jdx);
        length = json_query(query, buffer);
        parse_command_arg(&cmd->args[jdx], buffer);
      }
    }
  }

  json_close();

  return FN_ERR_OK;
}

FujiCommand *find_command(const char *name)
{
  FujiCommand *cmd;


  for (cmd = fuji_commands; cmd; cmd = cmd->next) {
    if (!strcasecmp(name, cmd->name))
      return cmd;
  }

  return NULL;
}
