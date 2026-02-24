#include "json.h"
#include <fujinet-network.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#ifdef __COLECOADAM__
#include <stdio_cpm.h>
#else /* ! __COLECOADAM__ */
#include <stdio.h>
#endif /* __COLECOADAM__ */

static char json_buffer[256];

#define PORT "7501"
#define WRITE_SOCKET "N1:TCP://:" PORT
#define READ_SOCKET "N2:TCP://localhost:" PORT
#define READ_URL_PREFIX "N3:"

#define MAX_CONN_WAIT 10

// Open Watcom can't do far pointers in a function declaration
static FN_ERR err;
static uint8_t status;
static uint16_t avail;

FN_ERR json_open(const char *path)
{
  uint8_t status_count;
  size_t length, total;
  FILE *fd;
  uint16_t bytesWaiting;
  uint8_t connected;
  uint8_t error;
  bool is_url;

  is_url = false;

  if (strstr(path, "//") != NULL)
  {
    is_url = true;

    if (strncmp(path, READ_URL_PREFIX, strlen(READ_URL_PREFIX)) != 0)
    {
      printf("URL MUST START WITH %s\n", READ_URL_PREFIX);
      return FN_ERR_IO_ERROR;
    }

    // Open the path as a network url
    printf("Opening URL (%s)...\n", path);
    err = network_open(path, OPEN_MODE_READ, 0);
    if (err != FN_ERR_OK)
    {
      printf("URL OPEN FAIL %d\n", err);
      return FN_ERR_IO_ERROR;   
    }
  }
  else
  {
    fd = fopen(path, "r");
    if (!fd)
    {
      printf("Failed to open %s %d\n", path, errno);
      return FN_ERR_IO_ERROR;
    }
  }

  printf("Opening write socket (%s)...\n", WRITE_SOCKET);
  err = network_open(WRITE_SOCKET, OPEN_MODE_RW, 0);
  if (err != FN_ERR_OK)
    return err;

  printf("Opening read socket (%s)...\n", READ_SOCKET);
  err = network_open(READ_SOCKET, OPEN_MODE_READ, 0);
  if (err != FN_ERR_OK)
    return err;

  for (status_count = 0; status_count < MAX_CONN_WAIT; status_count++) {
    err = network_status(WRITE_SOCKET, &avail, &status, &err);
    //printf("AVAIL: %u  STATUS: %u  ERR: %u\n", avail, status, err);
    if (err != FN_ERR_OK)
      return err;
    if (status == 1)
      break;
  }

  if (status_count >= MAX_CONN_WAIT)
    return FN_ERR_IO_ERROR;

  printf("DOING ACCEPT\n");
  err = network_accept(WRITE_SOCKET);
  if (err != FN_ERR_OK) {
    printf("ACCEPT FAIL %d\n", err);
    return err;
  }
  printf("ACCEPTED\n");

  for (total = 0; ; total += length) {
    if (is_url)
    {
      // Read from the network url instead of a file
      network_status(path, &bytesWaiting, (uint8_t *) &connected, &error);

      if (bytesWaiting == 0 || !error) 
      {
        printf("URL EOF\n");
        break;
      }
      length = network_read(path, json_buffer, bytesWaiting > 256 ? 256 : bytesWaiting);
    }
    else
    {
      length = fread(json_buffer, 1, 256, fd);
      if (!length)
      {
        printf("EOF\n");
        break;
      }
    }

    err = network_write(WRITE_SOCKET, json_buffer, length);
    printf("%d ", total + length);
    fflush(stdout);
    if (err != FN_ERR_OK)
      break;
  }

  if (is_url)
  {
    network_close(path);
  }
  else
  {
    fclose(fd);
  }

  network_close(WRITE_SOCKET);
  return network_json_parse(READ_SOCKET);
}

FN_ERR json_close()
{
  return network_close(READ_SOCKET);
}

int json_query(const char *query, void *buffer)
{
  int length = network_json_query(READ_SOCKET, query, (char *) buffer);


  if (length < 0) {
    printf("ERROR %d\n", length);
    return length;
  }
  return length;
}
