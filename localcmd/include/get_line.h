#ifndef BWC_GETLINE_H
#define BWC_GETLINE_H

#ifndef _CMOC_VERSION_
#include <stdint.h>
#endif /* _CMOC_VERSION_ */

#if defined(BUILD_APPLE2)
#define CH_DEL          127
#endif

#if defined(BUILD_MSDOS)
#define get_line(buf, size) fgets((buf), (size), stdin)
#else
void get_line(char* buf, uint8_t max_len);
#endif /* BUILD_MSDOS */

#endif // BWC_GETLINE_H
