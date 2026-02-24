#ifndef CONSOLE_H
#define CONSOLE_H

#include <stdio.h>
#include <stdint.h>

#ifdef _CMOC_VERSION_

#define clrscr() cls(1)

byte cgetc(void);

#else /* ! _CMOC_VERSION */

#include <conio.h>

  #ifdef __WATCOMC__
  #include <graph.h>
  #define clrscr() _clearscreen(_GCLEARSCREEN)
  #define cgetc() getch()
  #endif /* __WATCOMC__ */

#endif /* _CMOC_VERSION_ */

extern uint8_t console_width, console_height;

extern void console_init(void);

#endif /* CONSOLE_H */
