#include "stdio.h"
#include <disk.h>
#include <fujinet-fuji.h> // for bool

typedef struct {
  bool is_open;
  struct FileDesc fd;
} stdio_handle;

int errno = 0;

static bool stdio_initialized = false;
static byte fat_buffer[MAX_NUM_GRANULES];
static stdio_handle stdio_fd[MAX_STDIO_OPEN_FILES];

static void stdio_init()
{
  if (stdio_initialized)
    return;
  memset(stdio_fd, 0, sizeof(stdio_fd));
  initdisk(fat_buffer);
  stdio_initialized = true;
  return;
}

// Returns index into stdio_fd if handle is valid, else returns -1
int stdio_validate_handle(FILE *stream)
{
  int idx;


  if (!stream)
    return -1;
  idx = ((int) (stream)) - 1;
  if (idx >= MAX_STDIO_OPEN_FILES || !stdio_fd[idx].is_open)
    return -1;
  return idx;
}

FILE *fopen(const char *pathname, const char *mode)
{
  int idx;


  stdio_init();

  for (idx = 0; idx < MAX_STDIO_OPEN_FILES; idx++) {
    if (!stdio_fd[idx].is_open) {
      if (!openfile(&stdio_fd[idx].fd, pathname)) {
        // FIXME - set errno?
        return NULL;
      }

      // FIXME - do something with mode?
      stdio_fd[idx].is_open = true;
      return (FILE *) (idx + 1);
    }
  }

  return NULL;
}

int fclose(FILE *stream)
{
  int idx;


  idx = stdio_validate_handle(stream);
  if (idx < 0)
    return -1;

  close(&stdio_fd[idx].fd);
  stdio_fd[idx].is_open = false;
  return 0;
}

size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
  int idx;
  word length;


  idx = stdio_validate_handle(stream);
  if (idx < 0)
    return -1;

  // FIXME - what if size * nmemb > 32767
  length = read(&stdio_fd[idx].fd, (char *) ptr, size * nmemb);
  return length / size;
}

size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream)
{
  printf("fwrite NOT IMPLEMENTED\n");
  exit(1);
  return 0;
}

// Converts an 11-byte normalized filename back into a readable string.
// Input format: 8 chars name + 3 chars extension (space padded), null-terminated.
// Output: "FILENAME.EXT" or "FILENAME" if no extension.
// Does NOT restore original case.
//
void denormalizeFilename(char *dest, const char *src)
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

byte find_file_by_extension(char *outfname, const char *ext)
{   
    byte dirSector[256];
    char extbuf[4];
    char entry_fname[12];
    
    printf("SEARCHING FOR: *.%s\n", ext);
    for (byte sector = 3; sector <= 18; ++sector)
    { 

        printf("READING DIRECTORY on DRIVE: %d\n", curDriveNo);
        if (!readDiskSector(dirSector, curDriveNo, 17, sector))
            return 0;  // TODO: report I/O error instead of "not found"

        byte *entry;
        for (word index = 0; index < 256; index += 32)
        {
            entry = dirSector + index;
            if (!*entry)  // if erased entry
                continue;
            if (*entry == 0xFF)  // if end of dir
                break;

            // Get the extension of the directory entry
            memcpy(extbuf, &entry[8], 3);
            extbuf[3] = '\0';

            if (stricmp(extbuf,ext) == 0)  // if extension matches
            {

                memcpy(entry_fname, entry, 11);
                denormalizeFilename(outfname, entry_fname);
                printf ("FOUND: %s\n", outfname);

                return 1;  // found
            }
        }
        if (*entry == 0xFF)  // if end of dir
            break;
    }

    return 0;  // not found
}
