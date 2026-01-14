#include <stdio.h>
#include <string.h>
#include <conio.h>

#include "platform.h"
#include "commands.h"
#include "testing.h"
#include "results.h"

int main(void)
{
    init();
    clrscr();

    if (load_commands("COMMANDS.JSN") != FN_ERR_OK) {
      printf("No commands found\n");
      exit(1);
    }

    execute_tests("TESTS.JSN");
    if (screen_width > 32)
    {
        printf("\nTests complete. Press a key for results.");
    }
    else
    {
        printf("\n<Tests complete.>\n");
        printf("Press a key for results.");
    }
    cgetc();
    print_test_results();

    return 0;
}
