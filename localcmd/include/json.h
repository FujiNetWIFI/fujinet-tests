#ifndef JSON_H
#define JSON_H

#include <fujinet-fuji.h> // for uint8_t

extern uint8_t json_open(const char *path);
extern void json_close();
extern size_t json_query(const char *query, void *buffer);

#endif /* JSON_H */
