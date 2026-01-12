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
    cls(1);
#else
#endif /* _CMOC_VERSION_ */

cls(1);

    load_commands("COMMANDS.JSN");
    printf("Running tests\n");
    execute_tests("TESTS.JSN");
#ifdef _CMOC_VERSION_
    printf("Tests complete.  Press any key to see results...\n");  
    waitkey(0);
#else    
#endif
    print_test_results();

    return 0;
}
