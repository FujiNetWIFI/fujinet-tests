#ifndef BWC_GETLINE_H
#define BWC_GETLINE_H

#ifndef _CMOC_VERSION_
#include <stdint.h>

#if defined(BUILD_APPLE2)
#include <apple2.h>
#endif

#endif

void get_line(char* buf, uint8_t max_len);

#endif // BWC_GETLINE_H
