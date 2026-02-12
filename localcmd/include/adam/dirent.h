#ifndef DIRENT_H
#define DIRENT_H

#include <stdint.h>

typedef struct {
  uint8_t in_use:1;
  uint8_t first:1;
} DIR;

struct dirent {
  char d_name[8 + 3 + 1 + 1]; // name + extension + dot + null terminator
};

extern DIR *opendir(const char *name);
extern int closedir(DIR *dirp);
extern struct dirent *readdir(DIR *dirp);

#endif /* DIRENT_H */
