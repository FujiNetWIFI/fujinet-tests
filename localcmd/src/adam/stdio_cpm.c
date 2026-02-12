#include <stdio_cpm.h>
#include <stdint.h>
#include <stdlib.h>
#include <cpm.h>
#include <string.h>
#include <fujinet-fuji.h> // for bool

struct FCB {
  char drive;
  char filename[8];
  char ext[3];
  char extent;
  char reserved[2];
  char record_count;
  char dma_blocks[16];
  char record_num[4];
};

typedef struct {
  bool is_open;
  struct FCB fcb;
  uint8_t remaining;
  uint8_t buffer[SECSIZE];
} stdio_handle;

int errno = 0;

static bool stdio_initialized = false;
static stdio_handle stdio_fd[MAX_STDIO_OPEN_FILES];

static void stdio_init()
{
  if (stdio_initialized)
    return;
  memset(stdio_fd, 0, sizeof(stdio_fd));
  stdio_initialized = true;
  return;
}

// Returns index into stdio_fd if handle is valid, else returns -1
int stdio_validate_handle(cpm_FILE *stream)
{
  int idx;


  if (!stream)
    return -1;
  idx = ((int) (stream)) - 1;
  if (idx >= MAX_STDIO_OPEN_FILES || !stdio_fd[idx].is_open)
    return -1;
  return idx;
}

void cpm_normalize(struct FCB *fcb, const char *pathname)
{
  const char *dot;
  uint_fast8_t len;


  dot = strchr(pathname, '.');
  if (dot)
    len = dot - pathname;
  else
    len = strlen(pathname);
  if (len > sizeof(fcb->filename))
    len = sizeof(fcb->filename);

  memset(fcb->filename, ' ', sizeof(fcb->filename) + sizeof(fcb->ext));
  memcpy(fcb->filename, pathname, len);

  if (dot) {
    dot++;
    len = strlen(dot);
    if (len > sizeof(fcb->ext))
      len = sizeof(fcb->ext);
    memcpy(fcb->ext, dot, len);
  }

  return;
}

cpm_FILE *cpm_fopen(const char *pathname, const char *mode)
{
  int idx;


  stdio_init();

  for (idx = 0; idx < MAX_STDIO_OPEN_FILES; idx++) {
    if (!stdio_fd[idx].is_open) {
      memset(&stdio_fd[idx].fcb, 0, sizeof(struct FCB));
      cpm_normalize(&stdio_fd[idx].fcb, pathname);
      if (bdos(CPM_OPN, &stdio_fd[idx].fcb) == 0xFF) {
        // FIXME - set errno?
        return NULL;
      }

      // FIXME - do something with mode?
      stdio_fd[idx].is_open = true;
      return (cpm_FILE *) (idx + 1);
    }
  }

  return NULL;
}

int cpm_fclose(cpm_FILE *stream)
{
  int idx;


  idx = stdio_validate_handle(stream);
  if (idx < 0)
    return -1;

  bdos(CPM_CLS, &stdio_fd[idx].fcb);
  stdio_fd[idx].is_open = false;
  return 0;
}

size_t cpm_fread(void *ptr, size_t size, size_t nmemb, cpm_FILE *stream)
{
  int idx;
  size_t length;
  uint8_t err;


  idx = stdio_validate_handle(stream);
  if (idx < 0)
    return -1;

  if (!stdio_fd[idx].remaining) {
    bdos(CPM_SDMA, stdio_fd[idx].buffer);
    err = bdos(20, &stdio_fd[idx].fcb);
    if (err != 0)
      return 0;
    stdio_fd[idx].remaining = SECSIZE;
  }

  length = size * nmemb;
  if (length > stdio_fd[idx].remaining)
    length = stdio_fd[idx].remaining;

  memcpy(ptr, &stdio_fd[idx].buffer[SECSIZE - stdio_fd[idx].remaining], length);
  stdio_fd[idx].remaining -= length;

  return length / size;
}

size_t cpm_fwrite(const void *ptr, size_t size, size_t nmemb, cpm_FILE *stream)
{
  printf("fwrite NOT IMPLEMENTED\n");
  exit(1);
  return 0;
}
