#ifndef TESTING_H
#define TESTING_H

#ifdef __MSX__
#include <stdio.h> // MSX changes bool typedef in stdio.h so need to include it first
#endif /* __MSX__ */
#include <fujinet-fuji.h> // for uint8_t

#define FLAG_WARN   0x10
#define FLAG_EXPERR 0x20
#define FLAG_EXCEEDS_U8  0x04
#define FLAG_EXCEEDS_U16 0x02

typedef struct {
  uint8_t device;
  uint8_t command;
  uint8_t flags;
  uint8_t aux1, aux2, aux3, aux4;
  uint16_t data_len, reply_len;
} TestCommand;

extern bool run_test(TestCommand *test, void *data, void *expected);
extern void execute_tests(const char *path);

#endif /* TESTING_H */
