#ifndef STDIO_CPM_H
#define STDIO_CPM_H

#include <stdio.h>

#define MAX_STDIO_OPEN_FILES 4

typedef void cpm_FILE;
extern cpm_FILE *cpm_fopen(const char *pathname, const char *mode);
extern int cpm_fclose(cpm_FILE *stream);
extern size_t cpm_fread(void *ptr, size_t size, size_t nmemb, cpm_FILE *stream);
extern size_t cpm_fwrite(const void *ptr, size_t size, size_t nmemb, cpm_FILE *stream);

#define fopen cpm_fopen
#define fclose cpm_fclose
#define fread cpm_fread
#define fwrite cpm_fwrite

#define FILE cpm_FILE

#endif /* STDIO_CPM_H */
