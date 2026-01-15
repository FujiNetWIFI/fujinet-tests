#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "commands.h"
#include "json.h"
#include "platform.h"

FujiCommand *fuji_commands = NULL;

static char query[32];
static char buffer[64];

void parse_command_arg(FujiArg *arg, const char *buffer)
{
  const char *p;
  uint16_t len;


  p = strchr(buffer, ':');
  if (!p) {
    printf("Invalid format string %s\n", buffer);
    return;
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

int load_commands(const char *path)
{
  uint8_t err, cnum;
  uint16_t idx, jdx;
  int length;
  FujiCommand *cmd, *next;


  err = json_open(path);
  if (err != FN_ERR_OK)
    return err;

  printf("Loading Fuji commands...\n");

  cmd = NULL;
  for (idx = 0; ; idx++) {
    sprintf(query, "/%d/command", idx);
    length = json_query(query, buffer);
    if (length < 0)
      return -length;
    if (!length)
      break;

    cnum = (uint8_t) atoi(buffer);
    sprintf(query, "/%d/name", idx);
    length = json_query(query, buffer);
    if (length < 0)
      return -length;
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
    if (length < 0)
      return -length;
    if (length)
      parse_command_arg(&cmd->reply, buffer);
    else {
      sprintf(query, "/%d/reply/0", idx);
      length = json_query(query, buffer);
      if (length < 0)
        return -length;
      if (length)
        parse_command_arg(&cmd->reply, buffer);
    }

    for (jdx = 0; ; jdx++) {
      sprintf(query, "/%d/args/%d", idx, jdx);
      length = json_query(query, buffer);
      if (length < 0)
        return -length;
      if (!length)
        break;
    }

    cmd->argCount = (uint8_t) jdx;
    if (cmd->argCount) {
      cmd->args = (FujiArg *) malloc(sizeof(FujiArg) * cmd->argCount);
      for (jdx = 0; jdx < cmd->argCount; jdx++) {
        sprintf(query, "/%d/args/%d", idx, jdx);
        length = json_query(query, buffer);
        if (length < 0)
          return -length;
        parse_command_arg(&cmd->args[jdx], buffer);
      }
    }
  }

  json_close();

  if (!idx)
    return -1;

  return FN_ERR_OK;
}

FujiCommand *find_command(const char *name)
{
  FujiCommand *cmd;


  for (cmd = fuji_commands; cmd; cmd = cmd->next) {
    if (!stricmp(name, cmd->name))
      return cmd;
  }

  return NULL;
}
