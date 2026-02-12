#include "dirent.h"
#include <cpm.h>

#include <stdio.h> // debug

#define MAX_OPEN_DIRS 1

typedef struct {
  uint8_t user;
  char filename[8];
  char extension[3];
  uint8_t ex;
  uint8_t s1;
  uint8_t s2;
  uint8_t rc;
  uint8_t blocks[16];
} cpm_dir_entry;

static DIR dirtable[MAX_OPEN_DIRS];
static struct dirent cur_dirent;
static uint8_t dma[128];
static struct fcb cpm_fcb;

DIR *opendir(const char *name)
{
  uint_fast8_t idx;


  for (idx = 0; idx < MAX_OPEN_DIRS; idx++) {
    if (!dirtable[idx].in_use)
      break;
  }

  if (idx == MAX_OPEN_DIRS)
    return NULL;

  dirtable[idx].in_use = 1;
  dirtable[idx].first = 1;
  setfcb(&cpm_fcb, "????????.???");

  return &dirtable[idx];
}

int closedir(DIR *dirp)
{
  dirp->in_use = 0;
  return 0;
}

// Converts an 11-byte normalized filename back into a readable string.
// Input format: 8 chars name + 3 chars extension (space padded), null-terminated.
// Output: "FILENAME.EXT" or "FILENAME" if no extension.
// Does NOT restore original case.
//
void denormalize(char *dest, const char *src)
{
    int i;
    int end;

    // Copy filename (trim trailing spaces)
    end = 7;
    while (end >= 0 && src[end] == ' ')
        --end;

    for (i = 0; i <= end; ++i)
        *dest++ = src[i];

    // Copy extension if present (trim trailing spaces)
    src += 8;
    end = 2;
    while (end >= 0 && src[end] == ' ')
        --end;

    if (end >= 0)
    {
        *dest++ = '.';
        for (i = 0; i <= end; ++i)
            *dest++ = src[i];
    }

    *dest = '\0';
}

struct dirent *readdir(DIR *dirp)
{
  int idx;
  cpm_dir_entry *entry;


  bdos(CPM_SDMA, dma);

  if (dirp->first)
    idx = bdos(CPM_FFST, &cpm_fcb);
  else
    idx = bdos(CPM_FNXT, &cpm_fcb);

  printf("READDIR R:%d F:%d\n", idx, dirp->first);
  dirp->first = 0;
  if (idx < 0 || idx > 3)
    return NULL;

  entry = (cpm_dir_entry *) dma;
  denormalize(cur_dirent.d_name, (char *) entry[idx].filename);
  return &cur_dirent;
}
