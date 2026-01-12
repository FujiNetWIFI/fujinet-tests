#include "json.h"
#include "commands.h"
#include "testing.h"
#include "results.h"

#define malloc(len) sbrk(len)
#define strcasecmp(x, y) stricmp(x, y)

#define FLAG_EXCEEDS_U8  0x04
#define FLAG_EXCEEDS_U16 0x02

void add_aux_val(TestCommand *test, uint16_t val, uint16_t size, int *auxpos)
{
  uint8_t *ptr;
  int offset, remain;


  ptr = &test->aux1;
  for (remain = size, offset = *auxpos; remain; remain -= 8, offset++) {
    // FXIME - deal with endianness
    *(ptr + offset) = (uint8_t) (val & 0xFF);
    val >>= 8;
  }
  *auxpos = offset;

  test->flags++;
  if (test->flags < 4 && size > 8) {
    test->flags += FLAG_EXCEEDS_U8;
    if (size > 16)
      test->flags += FLAG_EXCEEDS_U16;
  }

  return;
}

void add_test_argument(TestCommand *test, FujiArg *arg, const char *input,
                       int *auxpos, const void **dataptr)
{
  int val, size, offset;


  switch (arg->type) {
  case 'b':
    val = 0;
    if (!strcasecmp(input, "TRUE"))
      val = 1;
    else if (atoi(input))
      val = 1;
    add_aux_val(test, val, 8, auxpos);
    break;

  case 'f':
    test->data_len = arg->size;
    *dataptr = input;
    break;

  case 's':
    test->data_len = strlen(input);
    *dataptr = input;
    add_aux_val(test, test->data_len, arg->size, auxpos);
    break;

  default: // integer types
    // FIXME - handle unsigned greater than 32767
    add_aux_val(test, atoi(input), arg->size, auxpos);
    break;
  }

  return;
}

int get_json_data(void)
{
    char command[50];
    int16_t bytesread;
    int count = 0;
    char query[256];
    FujiCommand *cmd;
    uint16_t idx;
    TestCommand test;
    int auxpos;
    void *data, *expected;
    bool success;
    TestResult *result_ptr;

    result_list_init(&result_list);

    while (true)
    {
        sprintf(query, "/%d/command", count);
        bytesread = json_query(query, command);
        if (!bytesread) // If we didn't get any more objects with "command" in them, we're done.
        {
            break;
        }

        printf("command: %s\n", command);
        cmd = find_command(command);
        if (!cmd) {
          printf("Unknown command\n");
          return -1;
        }
        printf("found: %s\n", cmd->name);

        sprintf(query, "/%d/device", count);
        bytesread = json_query(query, command);
        if (!bytesread)
          test.device = FUJI_DEVICEID_FUJINET;
        else {
          printf("FIXME - deal with device %s\n", command);
          exit(1);
        }
        test.command = cmd->command;
        test.flags = 0;
        test.aux1 = test.aux2 = test.aux3 = test.aux4 = 0;
        test.data_len = test.reply_len = 0;
        auxpos = 0;
        data = expected = NULL;

        for (idx = 0; idx < cmd->argCount; idx++) {
          sprintf(query, "/%d/%s", count, cmd->args[idx].name);
          bytesread = json_query(query, command);
          if (!bytesread) {
            printf("Argument %s missing\n", cmd->args[idx].name);
            return count;
          }
          printf("Arg %s = %s\n", cmd->args[idx].name, command);
          add_test_argument(&test, &cmd->args[idx], command, &auxpos, (const void **) &data);
        }

        sprintf(query, "/%d/replyLength", count);
        bytesread = json_query(query, command);
        if (bytesread)
          test.reply_len = atoi(command);
        else if (cmd->reply.type == 'f')
          test.reply_len = cmd->reply.size;

        // FIXME - get expected

        sprintf(query, "/%d/warnOnly", count);
        bytesread = json_query(query, command);
        if (bytesread)
          test.flags |= FLAG_WARN;

        sprintf(query, "/%d/errorExpected", count);
        bytesread = json_query(query, command);
        if (bytesread)
          test.flags |= FLAG_EXPERR;

        success = run_test(&test, data, expected);

        result_ptr = (TestResult *) malloc(sizeof(TestResult));
        result_ptr->command_name = cmd->name;
        result_ptr->command = test.command;
        result_ptr->device = test.device;
        result_ptr->success = success;
        result_ptr->flags = test.flags;

        result_list_insert(&result_list, result_ptr);

        count++;

        if (!(test.flags & FLAG_WARN) && !success)
        {
            printf("TEST FAILED\n");
            return count;
        }

    }

    printf("\n%d tests read.\n", count);
    return count;
}

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

    if (json_open("TESTS.JSN") != FN_ERR_OK) {
      printf("Failed to open JSON file\n");
      return 1;
    }

    printf("Getting json data\n");
    test_count = get_json_data();
    json_close();
    printf("Tests complete.  Press any key to see results...\n");
#ifdef _CMOC_VERSION_    
    waitkey(0);
#else    
#endif
    print_test_results();

    return 0;
}
