#include "testing.h"
#include "commands.h"
#include "json.h"
#include "results.h"

#ifndef _CMOC_VERSION_
#include <stdio.h>
#include <string.h>
#include <conio.h>
#endif /* _CMOC_VERSION_ */

enum {
  FUJI_DEVICEID_FILE            = 0xAA,
};

typedef struct {
  uint8_t num;
  const char *name;
} FujiDeviceID;

const FujiDeviceID fujiDeviceTable[] = {
  {FUJI_DEVICEID_FUJINET, "FUJINET"},
  {FUJI_DEVICEID_CLOCK,   "APETIME"},
  {FUJI_DEVICEID_CPM,     "CPM"},
  {FUJI_DEVICEID_MIDI,    "MIDI"},
  {FUJI_DEVICEID_VOICE,   "VOICE"},
  {0, NULL},
};

#define FLAG_MASK ((uint8_t) ~(FLAG_WARN | FLAG_EXPERR))

#define malloc(len) sbrk(len)
#define strcasecmp(x, y) stricmp(x, y)

unsigned int fail_count = 0;
static uint8_t reply[256];

bool run_test(TestCommand *test, void *data, void *expected)
{
  bool success;


  printf("Received command: 0x%02x:%02x\n"
         "  FLAGS: 0x%02x\n"
         "  AUX: 0x%02x 0x%02x 0x%02x 0x%02x\n"
         "  DATA: %d REPLY: %d\n",
         test->device, test->command,
         test->flags,
         test->aux1, test->aux2, test->aux3, test->aux4,
         test->data_len, test->reply_len);

  printf("Executing 0x%02x:%02x\n", test->device, test->command);
#if 0
  if (test->device >= FUJI_DEVICEID_DISK && test->device <= FUJI_DEVICEID_DISK_LAST) {
    // Disks are handled by the opearating system
    success = disk_command(test, data, reply, sizeof(reply));
  }
  else if (test->device == FUJI_DEVICEID_FILE) {
#ifdef _CMOC_VERSION_
    // CMOC doesn't provide any file I/O routines for DECB
    success = 0;
#else /* ! _CMOC_VERSION_ */
      // pseudo-commands for test controller to open/read/write files
    success = file_command(test, data, reply, sizeof(reply));
#endif /* _CMOC_VERSION_ */
  }
  else {
#endif // 0
    success = fuji_bus_call(test->device, test->command, test->flags & FLAG_MASK,
                            test->aux1, test->aux2, test->aux3, test->aux4,
                            data, test->data_len,
                            test->reply_len ? reply : (uint8_t *) NULL, test->reply_len);
#if 0
  }
#endif // 0

  if (test->flags & FLAG_EXPERR)
    success = !success;

  if (!(test->flags & FLAG_WARN) && !success)
    fail_count++;

  // FIXME - if expected is not NULL check reply

  printf("Success: %d  Flags: 0x%02x\n", success, test->flags);

  return success;
}

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

void execute_tests(const char *path)
{
  char command[50];
  int16_t bytesread;
  int count = 0;
  char query[256];
  FujiCommand *cmd;
  uint16_t idx, dev_idx;
  TestCommand test;
  int auxpos;
  void *data, *expected;
  bool success;
  TestResult *result_ptr;


  if (json_open("TESTS.JSN") != FN_ERR_OK) {
    printf("Failed to open JSON file\n");
    return;
  }

  while (true) {
    sprintf(query, "/%d/command", count);
    bytesread = json_query(query, command);
    if (!bytesread)
      break;

    printf("command: %s\n", command);
    cmd = find_command(command);
    if (!cmd) {
      printf("Unknown command\n");
      return;
    }
    printf("found: %s\n", cmd->name);

    sprintf(query, "/%d/device", count);
    bytesread = json_query(query, command);
    if (!bytesread)
      test.device = FUJI_DEVICEID_FUJINET;
    else {
      for (dev_idx = 0; fujiDeviceTable[dev_idx].num; dev_idx++) {
        if (!strcasecmp(fujiDeviceTable[dev_idx].name, command)) {
          test.device = fujiDeviceTable[dev_idx].num;
          break;
        }
      }

      if (!fujiDeviceTable[dev_idx].num) {
        printf("Unknown device %s\n", command);;
        exit(1);
      }
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
        return;
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

    // Record result
    result_ptr = (TestResult *)malloc(sizeof(TestResult));
    result_ptr->command_name = cmd->name;
    result_ptr->command = test.command;
    result_ptr->device = test.device;
    result_ptr->success = success;
    result_ptr->flags = test.flags;

    result_list_insert(&result_list, result_ptr);

    if (!(test.flags & FLAG_WARN) && !success)
    {
      printf("TEST FAILED\n");
      return;
    }

    count++;
  }

  json_close();
}
