#include <stdio.h>
#include <stdlib.h>
#include <fujinet-network.h>

#include "json.h"

static char json_buffer[256];

#define PORT "7501"
#define WRITE_SOCKET "N1:TCP://:" PORT
#define READ_SOCKET "N2:TCP://localhost:" PORT

uint8_t json_open(const char *path)
{
  size_t length;
  uint8_t err, status;
  uint16_t avail;
  FILE *fd;


  fd = fopen(path, "r");
  if (!fd) {
    printf("Failed to open %s\n", path);
    exit(1);
    return FN_ERR_IO_ERROR;
  }

  printf("Opening write socket (%s)...\n", WRITE_SOCKET);
  err = network_open(WRITE_SOCKET, OPEN_MODE_RW, 0);
  if (err != FN_ERR_OK)
    return err;

  printf("Opening read socket (%s)...\n", READ_SOCKET);
  err = network_open(READ_SOCKET, OPEN_MODE_READ, 0);
  if (err != FN_ERR_OK)
    return err;

  for (;;) {
    err = network_status(WRITE_SOCKET, &avail, &status, &err);
    // printf("AVAIL: %u  STATUS: %u  ERR: %u\n", avail, status, err);
    if (err != FN_ERR_OK)
      return err;
    if (status == 1)
      break;
  }

  err = network_accept(WRITE_SOCKET);
  if (err != FN_ERR_OK)
    return err;

  for (;;) {
    length = fread(json_buffer, 1, 256, fd);
    if (!length)
      break;
    err = network_write(WRITE_SOCKET, json_buffer, length);
    if (err != FN_ERR_OK)
      break;
  }

  fclose(fd);
  network_close(WRITE_SOCKET);
  network_json_parse(READ_SOCKET);

  return FN_ERR_OK;
}

void json_close()
{
  network_close(READ_SOCKET);
  return;
}

size_t json_query(const char *query, void *buffer)
{
  int length = network_json_query(READ_SOCKET, query, (char *) buffer);


  if (length < 0) {
    printf("ERROR %d\n", length);
    exit(1);
  }
  return length;
}
