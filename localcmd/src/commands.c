#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "commands.h"
#include "json.h"
#include "platform.h"

FujiCommand *fuji_commands = NULL;

static char query[32];
static char buffer[64];

bool parse_command_arg(FujiArg *arg, const char *buffer)
{
  const char *p;
  uint16_t len;


  p = strchr(buffer, ':');
  if (!p) {
    printf("Invalid format string %s\n", buffer);
    return false;
  }

  len = p - buffer;
  arg->name = (char *) malloc(len + 1);
  strncpy(arg->name, buffer, len);
  arg->name[len] = 0;
  p++;
  arg->type = *p;
  p++;
  arg->size = atoi(p);

  return true;
}

uint8_t parse_struct(FujiArg *arg, const char *buffer)
{
  const char *p;
  uint8_t idx;


  p = buffer + 1;
  for (idx = 0; p && *p; idx++) {
    if (!parse_command_arg(&arg[idx], p))
      return 0;
    arg[idx].type = TYPE_STRUCT;
    p = strchr(p, ',');
    if (p)
      p++;
  }

  return idx;
}

uint8_t count_commas(const char *buffer)
{
  uint8_t idx, commas;


  for (commas = idx = 0; buffer[idx]; idx++) {
    if (buffer[idx] == ',')
      commas++;
  }

  return commas;
}

FN_ERR add_arguments(FujiCommand *cmd, uint16_t json_idx)
{
  uint8_t count, struct_count, total;
  uint8_t arg_idx;
  int length;


  for (count = struct_count = 0; ; count++) {
    sprintf(query, "/%d/args/%d", json_idx, count);
    length = json_query(query, buffer);
    if (length < 0)
      return (FN_ERR) -length; // Turn negative FN_ERR back to positive
    if (!length)
      break;

    if (buffer[0] == TYPE_STRUCT)
      struct_count += count_commas(buffer);
  }

  total = count + struct_count;
  cmd->argCount = 0;
  if (total) {
    cmd->args = (FujiArg *) malloc(sizeof(FujiArg) * (total));
    for (arg_idx = 0; arg_idx < count; arg_idx++, cmd->argCount++) {
      sprintf(query, "/%d/args/%d", json_idx, arg_idx);
      length = json_query(query, buffer);
      if (length < 0)
        return (FN_ERR) -length; // Turn negative FN_ERR back to positive
      if (buffer[0] == TYPE_STRUCT) {
        struct_count = parse_struct(&cmd->args[cmd->argCount], buffer);
        if (!struct_count)
          return FN_ERR_IO_ERROR;
        cmd->argCount += struct_count - 1;
      }
      else if (!parse_command_arg(&cmd->args[cmd->argCount], buffer))
        return FN_ERR_IO_ERROR;
    }
  }

  return FN_ERR_OK;
}

FN_ERR load_commands(const char *path)
{
  uint8_t err, cnum;
  uint16_t idx;
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
      return (FN_ERR) -length; // Turn negative FN_ERR back to positive
    if (!length)
      break;

    cnum = (uint8_t) atoi(buffer);
    sprintf(query, "/%d/name", idx);
    length = json_query(query, buffer);
    if (length < 0)
      return (FN_ERR) -length; // Turn negative FN_ERR back to positive
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
      return (FN_ERR) -length; // Turn negative FN_ERR back to positive
    if (length) {
      if (!parse_command_arg(&cmd->reply, buffer))
        return FN_ERR_IO_ERROR;
    }
    else {
      sprintf(query, "/%d/reply/0", idx);
      length = json_query(query, buffer);
      if (length < 0)
        return (FN_ERR) -length; // Turn negative FN_ERR back to positive
      if (length) {
        if (!parse_command_arg(&cmd->reply, buffer))
          return FN_ERR_IO_ERROR;
      }
    }

    err = add_arguments(cmd, idx);
    if (err != FN_ERR_OK)
      return err;
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
