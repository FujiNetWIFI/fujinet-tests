#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "testing.h"
#include "commands.h"
#include "json.h"
#include "results.h"
#include "diskcmd.h"
#include "filecmd.h"
#include "wildcard.h"

enum {
  FUJI_DEVICEID_FILE            = 0xAA,
};

typedef struct {
  uint8_t num;
  const char *name;
} FujiDeviceID;

const FujiDeviceID fujiDeviceTable[] = {
  {FUJI_DEVICEID_FUJINET,   "FUJINET"},
  {FUJI_DEVICEID_CLOCK,     "APETIME"},
  {FUJI_DEVICEID_CPM,       "CPM"},
  {FUJI_DEVICEID_MIDI,      "MIDI"},
  {FUJI_DEVICEID_VOICE,     "VOICE"},
  {FUJI_DEVICEID_NETWORK,   "N1"},
  {FUJI_DEVICEID_NETWORK+1, "N2"},
  {FUJI_DEVICEID_NETWORK+2, "N3"},
  {FUJI_DEVICEID_NETWORK+3, "N4"},
  {0, NULL},
};

#define FLAG_MASK ((uint8_t) ~(FLAG_WARN | FLAG_EXPERR))

unsigned int fail_count = 0;
static uint8_t data_buf[256], reply_buf[256], expected_buf[256];

// Open Watcom can't do far pointers in a function declaration
static TestCommand exec_test;
static uint16_t exec_auxpos;

bool run_test(TestCommand *test, void *data, const void *expected)
{
  bool success;

  printf("Received command: 0x%02x:%02x\n"
         "   FLAGS: 0x%02x\n"
         "     AUX: 0x%02x 0x%02x 0x%02x 0x%02x\n"
         "    DATA: %d \"%s\"\n"
         "   REPLY: %d\n",
         test->device, test->command,
         test->flags,
         test->aux1, test->aux2, test->aux3, test->aux4,
         test->data_len, data ? (const char *) data : "<NULL>",
         test->reply_len);

  if (expected)
  {
    printf("EXPECTED: %s\n", (const char *) expected);
  }

  printf("Executing 0x%02x:%02x\n", test->device, test->command);
  if (test->device >= FUJI_DEVICEID_DISK && test->device <= FUJI_DEVICEID_DISK_LAST) {
    // Disks are handled by the operating system
    success = disk_command(test, data, reply_buf, sizeof(reply_buf));
  }
  else if (test->device == FUJI_DEVICEID_FILE) {
    // pseudo-commands for test controller to open/read/write files
    success = file_command(test, data, reply_buf, sizeof(reply_buf));
  }
  else {
    success = fuji_bus_call(test->device, test->command, test->flags & FLAG_MASK,
                            test->aux1, test->aux2, test->aux3, test->aux4,
                            data, test->data_len,
                            test->reply_len ? reply_buf : (uint8_t *) NULL, test->reply_len);
  }

  if (test->flags & FLAG_EXPERR)
    success = !success;

  if (expected)
  {
      success = wildcard_match((const char *)reply_buf, (const char *) expected);
  }

  if (!(test->flags & FLAG_WARN) && !success)
    fail_count++;

  printf("Success: %d  Flags: 0x%02x\n", success, test->flags);

  return success;
}

void add_val(uint8_t *ptr, uint16_t val, uint16_t size, uint16_t *pos)
{
  uint16_t count, offset;


  size /= 8;
  ptr += *pos;
  for (count = 0; count < size; count++) {
    // FXIME - deal with endianness
#ifdef _CMOC_VERSION_
    offset = size - 1 - count;
#else /* ! _CMOC_VERSION_ */
    offset = count;
#endif /* _CMOC_VERSION_ */
    *(ptr + offset) = (uint8_t) (val & 0xFF);
    val >>= 8;
  }
  *pos += count;

  return;
}

void add_aux_val(TestCommand *test, uint16_t val, uint16_t size, uint16_t *auxpos)
{
  add_val(&test->aux1, val, size, auxpos);
  test->flags++;
  if (test->flags < 4 && size > 8) {
    test->flags += FLAG_EXCEEDS_U8;
    if (size > 16)
      test->flags += FLAG_EXCEEDS_U16;
  }

  return;
}

void add_test_argument(TestCommand *test, FujiArg *arg, const char *input,
                       uint16_t *auxpos, const void **dataptr)
{
  int val;

  switch (arg->type) {
  case TYPE_BOOL:
    val = 0;
    if (!stricmp(input, "TRUE"))
      val = 1;
    else if (atoi(input))
      val = 1;
    add_aux_val(test, val, 8, auxpos);
    break;

  case TYPE_FIXED_LEN:
    test->data_len += arg->size;
    strcpy((char *) data_buf, input);
    *dataptr = data_buf;
    break;

  case TYPE_VAR_LEN:
    test->data_len += strlen(input);
    strcpy((char *) data_buf, input);
    *dataptr = data_buf;
    add_aux_val(test, test->data_len, arg->size, auxpos);
    break;

  case TYPE_STRUCT:
    // This is pretty much the same as TYPE_UNSIGNED but packs into
    // data instead of aux
    *dataptr = data_buf;
    add_val(data_buf, atoi(input), arg->size, &test->data_len);
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
  int16_t bytesread;
  int count = 0;
  FujiCommand *cmd;
  uint16_t idx, dev_idx;
  void *data, *expected;
  bool success;
  TestResult *result_ptr;
  char command[50];
  char query[50];

  printf("Running tests...\n");

  if (json_open(path) != FN_ERR_OK) {
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
      exec_test.device = FUJI_DEVICEID_FUJINET;
    else {
      for (dev_idx = 0; fujiDeviceTable[dev_idx].num; dev_idx++) {
        if (!stricmp(fujiDeviceTable[dev_idx].name, command)) {
          exec_test.device = fujiDeviceTable[dev_idx].num;
          break;
        }
      }

      if (!fujiDeviceTable[dev_idx].num) {
        printf("Unknown device %s\n", command);
        // FIXME - Record this test as a FAIL
        return;
      }
    }

    exec_test.command = cmd->command;
    exec_test.flags = 0;
    exec_test.aux1 = exec_test.aux2 = exec_test.aux3 = exec_test.aux4 = 0;
    exec_test.data_len = exec_test.reply_len = 0;
    exec_auxpos = 0;
    data = expected = NULL;

    for (idx = 0; idx < cmd->argCount; idx++) {
      sprintf(query, "/%d/%s", count, cmd->args[idx].name);
      bytesread = json_query(query, command);
      if (!bytesread) {
        printf("Argument %s missing\n", cmd->args[idx].name);
        return;
      }
      printf("Arg %s = %s\n", cmd->args[idx].name, command);
      add_test_argument(&exec_test, &cmd->args[idx], command, &exec_auxpos,
                        (const void **) &data);
    }

    sprintf(query, "/%d/replyLength", count);
    bytesread = json_query(query, command);
    if (bytesread)
      exec_test.reply_len = atoi(command);
    else if (cmd->reply.type == 'f')
      exec_test.reply_len = cmd->reply.size;

    sprintf(query, "/%d/expected", count);
    bytesread = json_query(query, command);
    if (bytesread)
    {
      strncpy(expected_buf, command, bytesread);
      expected = (char *) expected_buf;
    }
    else
    {
      expected = NULL;
    }

    sprintf(query, "/%d/warnOnly", count);
    bytesread = json_query(query, command);
    if (bytesread)
      exec_test.flags |= FLAG_WARN;

    sprintf(query, "/%d/errorExpected", count);
    bytesread = json_query(query, command);
    if (bytesread)
      exec_test.flags |= FLAG_EXPERR;

    success = run_test(&exec_test, data, expected);


    // Record result
    result_ptr = (TestResult *)malloc(sizeof(TestResult));
    result_ptr->command_name = cmd->name;
    result_ptr->command = exec_test.command;
    result_ptr->device = exec_test.device;
    result_ptr->success = success;
    result_ptr->flags = exec_test.flags;

    result_list_insert(&result_list, result_ptr);
    count++;

    if (!(exec_test.flags & FLAG_WARN) && !success)
    {
      printf("TEST FAILED\n");
      return;
    }

  }

  json_close();
}
