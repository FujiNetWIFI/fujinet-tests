#ifndef PLATFORM_H
#define PLATFORM_H

extern int screen_width; 
extern int page_size;  

extern void init(void);
extern void platform_number_string(char *);

#if defined(BUILD_COCO)
#define platform_name() "coco"
#include <cmoc.h>
#include <coco.h>

extern void platform_number_string(char *);

#elif defined(BUILD_APPLE2)
#define platform_name() "apple2"
#elif defined(BUILD_ATARI)
#define platform_name() "atari"
#elif defined(BUILD_C64)
#define platform_name() "c64"
#elif defined(BUILD_MSX)
#define platform_name() "msx"
#endif /* BUILD_COCO */

#endif /* PLATFORM_H */
