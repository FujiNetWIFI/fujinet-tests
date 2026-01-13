#include "commands.h"
#include "testing.h"
#include "results.h"

#ifndef _CMOC_VERSION_
#include <stdio.h>
#include <string.h>
#include <conio.h>
#endif /* _CMOC_VERSION_ */

int main(void)
{
    int test_count;

#ifdef _CMOC_VERSION_
    initCoCoSupport();
    if (isCoCo3)
    {
        width(80);
    }
    else
    {
        width(32);
    }
    cls(1);
#else
    clrscr();
#endif /* _CMOC_VERSION_ */

    load_commands("COMMANDS.JSN");
    printf("Running tests\n");
    execute_tests("TESTS.JSN");
#ifdef _CMOC_VERSION_
    if (isCoCo3)
    {
        printf("\nTests complete. Press any key for results.");  
    }
    else
    {
        printf("\n<Tests complete.>\n");  
    }
    waitkey(0);
#else
    printf("\nTests complete. Press anykey for results.");  
    cgetc();
#endif
    print_test_results();

    return 0;
}
