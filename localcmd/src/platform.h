#ifndef PLATFORM_H
#define PLATFORM_H

#ifdef _CMOC_VERSION_
#define platform_name() "coco"
#include <cmoc.h>
#include <coco.h>
#endif /* _CMOC_VERSION_ */

#endif /* PLATFORM_H */
