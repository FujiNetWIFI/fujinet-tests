#include "commands.h"
#include "testing.h"
#include "results.h"
#include "platform.h"
#include "console.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef BUILD_ATARI
#define exit(x) while(1)
#define PREFIX "D:"
#else /* ! BUILD_ATARI */
#define PREFIX ""
#endif /* BUILD_ATARI */

int main(void)
{
    uint8_t fail_count = 0;
    FN_ERR err;
    char testfname[13];

    console_init();
    clrscr();

    if (!fuji_get_adapter_config_extended(&fn_config)) {
      strcpy(fn_config.fn_version, "FAIL");
      fail_count++;
    }
    printf("FujiNet: %-14s  Make: ???\n", fn_config.fn_version);
    if (fail_count)
      exit(1);

    err = load_commands(PREFIX "COMMANDS.JSN");
    if (err != FN_ERR_OK) {
      printf("No commands found - ERROR %02x %02x\n", err, fn_device_error);
      exit(1);
    }

#ifdef _CMOC_VERSION_
    if (!find_file_by_extension(testfname, "TST"))
    {
        printf("NO TEST FILE FOUND!\n");
        exit(1);
    }
#else
    strcpy(testfname, "TESTS.TST");
#endif

    printf("RUNNING TESTS: %s\n", testfname);
    execute_tests(testfname);

    printf("DONE\n");
    if (console_width > 32)
    {
        printf("\nTests complete. Press a key for results.");
        fflush(stdout);
    }
    else
    {
        printf("\n<Tests complete.>\n");
        printf("Press a key for results.");
        fflush(stdout);
    }
    cgetc();
    print_test_results();

    exit(0);
#ifdef _CMOC_VERSION_
    // All other compilers will complain this is unreachable code
    return 0;
#endif /* _CMOC_VERSION_ */
}
