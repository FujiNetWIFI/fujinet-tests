#ifndef STDIO_H
#define STDIO_H

#include <cmoc.h>
#include <coco.h>

#define MAX_STDIO_OPEN_FILES 4
#define fflush(stream) /* no-op */
#define stdout 0

typedef void FILE;

extern FILE *fopen(const char *pathname, const char *mode);
extern int fclose(FILE *stream);
extern size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream);
extern size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream);

#endif /* STDIO_H */
