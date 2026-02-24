#include "commands.h"
#include "testing.h"
#include "results.h"
#include "platform.h"
#include "console.h"
#include "get_line.h"
#include "json.h"

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

#define NUM_TESTFILES 10
#define CR 0x0D

// Open Watcom can't do far pointers in a function declaration
static char testfname[32];
static char testpath[128];

static struct dirent testfiles[NUM_TESTFILES];

bool find_files_by_extension(char *outfname, const char *ext)
{
  DIR *dirp;
  struct dirent *entry;
  char *p;
  bool found = false;
  int idx = 0;
  int i;
  char c;


  memset(testfiles, 0, sizeof(testfiles));

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
    {
      strcpy(testfiles[idx].d_name, entry->d_name);
      idx++;
      if (idx >= NUM_TESTFILES)
        break;
    }
  }

  closedir(dirp);

  if (idx)
  {
    printf("FOUND %d FILES\n", idx);
    for (i = 0; i < idx; i++)
    {
      printf("%d: %s\n", i+1, testfiles[i].d_name);
    }

    while(true)
    {
      printf("SELECT FILE (1-%d) OR <ENTER>:", idx);
      c = cgetc();
      if (c >= '1' && c <= '0' + idx)
      {
        strcpy(outfname, testfiles[c - '1'].d_name);
        found = true;
        break;
      }
      else if (c == CR)
      {
        found = false;
        break;
      }
    }
  }

  printf("\n");

  return found;
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
      return 1;

    memset(testpath, 0, sizeof(testpath));

    // Make sure there is a test file before loading COMMANDS.JSN
    if (!find_files_by_extension(testfname, "TST"))
    {
      printf("ENTER TEST FILE URL:\n");
      get_line(testpath, sizeof(testpath));

      // Exit if the user didn't enter a path
      if (testpath[0] == '\0') 
      {
        printf("NO TEST FILE SELECTED. EXITING.\n");
        return 1;
      }
    }
    else
    {
      strcpy(testpath, testfname);
    }

    err = load_commands(PREFIX "COMMANDS.JSN");
    if (err != FN_ERR_OK) {
      printf("No commands found - ERROR %02x %02x\n", err, fn_device_error);
      return 1;
    }

    while (1) {
      printf("RUNNING TESTS: %s\n", testpath);
      execute_tests(testpath);

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
    }

    return 0;
}
