#include "commands.h"
#include "testing.h"
#include "results.h"
#include "platform.h"
#include "console.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#ifdef __WATCOMC__
#include <strings.h>
#endif /* _WATCOMC__ */

#ifdef BUILD_ATARI
#define exit(x) while(1)
#define PREFIX "D:"
#else /* ! BUILD_ATARI */
#define PREFIX ""
#endif /* BUILD_ATARI */

// Open Watcom can't do far pointers in a function declaration
static char testfname[32];

bool find_file_by_extension(char *outfname, const char *ext)
{
  DIR *dirp;
  struct dirent *entry;
  char *p;


  printf("SEARCHING FOR: *.%s\n", ext);
  dirp = opendir(".");
  if (!dirp) {
    printf("FAILED TO OPEN DIRECTORY\n");
    return 0;
  }

  while (1) {
    entry = readdir(dirp);
    if (!entry)
      break;
    p = strchr(entry->d_name, '.');
    if (p && !strcasecmp(p + 1, ext))
      break;
  }

  closedir(dirp);
  if (!entry)
    return 0;

  strcpy(outfname, entry->d_name);
  printf("FOUND \"%s\"\n", entry->d_name);
  return 1;
}

int main(void)
{
    uint8_t fail_count = 0;
    FN_ERR err;

    console_init();
    clrscr();

    if (!fuji_get_adapter_config(&fn_config)) {
      strcpy(fn_config.fn_version, "FAIL");
      fail_count++;
    }
    printf("FujiNet: %-14s  Make: ???\n", fn_config.fn_version);
    if (fail_count)
      exit(1);

    // Make sure there is a test file before loading COMMANDS.JSN
    if (!find_file_by_extension(testfname, "TST"))
    {
        printf("NO TEST FILE FOUND!\n");
        exit(1);
    }

    err = load_commands(PREFIX "COMMANDS.JSN");
    if (err != FN_ERR_OK) {
      printf("No commands found - ERROR %02x %02x\n", err, fn_device_error);
      exit(1);
    }

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

    return 0;
}
