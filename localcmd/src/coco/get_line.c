#include <cmoc.h>
#include <coco.h>
#include <ctype.h>
#include "chardef.h"
#include "console.h"
#include "get_line.h"

byte cgetc()
{
  byte shift = false;
  byte k;

  while (true)
  {
    k = inkey();

    if (isKeyPressed(KEY_PROBE_SHIFT, KEY_BIT_SHIFT))
    {
      shift = 0x00;
    }
    else
    {
      if (k > '@' && k < '[')
        shift = 0x20;
    }

    if (k)
      return k + shift;
  }
}

void get_line(char *buf, uint8_t max_len)
{
	uint8_t c;
	uint16_t i = 0;

	do
	{
		c = cgetc();

		if (isprint(c))
		{
			putchar(c);
			buf[i] = c;
			if (i < max_len - 1)
				i++;
		}
		else if (c == KEY_LEFT_ARROW)
		{
			if (i)
			{
				putchar(KEY_LEFT_ARROW);
				putchar(' ');
				putchar(KEY_LEFT_ARROW);
				--i;
			}
		}
	} while (c != KEY_ENTER);
	putchar('\n');
	buf[i] = '\0';
}
