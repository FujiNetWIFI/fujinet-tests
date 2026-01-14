#define MAX_STDIO_OPEN_FILES 4

#include <cmoc.h>
#include <coco.h>

typedef uint8_t FILE;

extern FILE *fopen(const char *pathname, const char *mode);
extern int fclose(FILE *stream);
extern size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream);
extern size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream);
