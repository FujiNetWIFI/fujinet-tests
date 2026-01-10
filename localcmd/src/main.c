#include "commands.h"
#include "testing.h"
#include "results.h"

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
#else
    cls(1);
#endif /* _CMOC_VERSION_ */


    load_commands("COMMANDS.JSN");
    printf("Running tests\n");
    execute_tests("TESTS.JSN");

    return 0;
}
