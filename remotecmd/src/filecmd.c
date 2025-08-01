#include "filecmd.h"
#include <stdio.h>

#define MAX_OPEN_FILES 8
static FILE *file_handles[MAX_OPEN_FILES];

#define MIN(a, b) ((a) < (b) ? (a) : (b))

static int8_t avail_handle()
{
  int idx;


  for (idx = 0; idx < MAX_OPEN_FILES; idx++)
    if (!file_handles[idx])
      return idx;
  return -1;
}

static bool valid_handle(int8_t fh)
{
  if (fh < 0 || fh >= MAX_OPEN_FILES)
    return false;

  if (!file_handles[fh])
    return false;

  return true;
}

bool file_command(TestCommand *cmd, void *data, void *reply, size_t reply_max)
{
  int8_t fh;
  const char *mode;
  int err;
  size_t len;
  ReadReply *rr;
  bool success = false;


  switch (cmd->command) {
  case FUJICMD_OPEN:
    if (cmd->aux1 == OPEN_READ)
      mode = "rb";
    else if (cmd->aux1 == OPEN_WRITE)
      mode = "wb";
    else if (cmd->aux1 == OPEN_READ | OPEN_WRITE)
      mode = "rb+";
    else
      return false;

    fh = avail_handle();
    if (fh < 0)
      return false;

    file_handles[fh] = fopen(data, mode);
    if (!file_handles[fh])
      return false;

    ((int8_t *) reply)[0] = fh;
    success = true;
    break;

  case FUJICMD_CLOSE:
    fh = cmd->aux1;
    if (!valid_handle(fh))
      return false;

    err = fclose(file_handles[fh]);
    file_handles[fh] = NULL;
    if (err)
      return false;

    success = true;
    break;

  case FUJICMD_READ:
    fh = cmd->aux1;
    if (!valid_handle(fh))
      return false;

    rr = reply;
    rr->length = fread(&rr->data, 1,
		       MIN(cmd->reply_len, reply_max - (&rr->data[0] - ((uint8_t *) rr))),
		       file_handles[fh]);
    success = true;
    break;

  case FUJICMD_WRITE:
    fh = cmd->aux1;
    if (!valid_handle(fh))
      return false;

    // FIXME - return number of bytes written
    fwrite(data, 1, cmd->data_len, file_handles[fh]);
    success = true;
    break;
  }

  return success;
}
