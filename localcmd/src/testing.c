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
#include "hexdump.h"

enum {
  FUJI_DEVICEID_FILE            = 0xAA,
};

typedef struct {
  uint8_t num;
  const char *name;
} FujiDeviceID;

typedef struct {
  size_t len;
  bool is_binary;
  void *data;
} BinaryBlob;

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
static char query[MAX_COMMAND_LENGTH];
static char command[MAX_COMMAND_LENGTH];

bool run_test(TestCommand *test, void *data, const BinaryBlob *expected)
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
    printf("EXPECTED:\n");
    hexdump(expected->data, expected->len);
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
    if (!expected->is_binary) {
      success = wildcard_match((const char *)reply_buf, (const char *) expected->data);
      if (!success)
        printf("MISMATCH: pattern: \"%s\" received: \"%s\"\n", expected->data, reply_buf);
    }
    else {
      success = !memcmp(expected->data, reply_buf, expected->len);
      if (!success) {
        printf("MISMATCH: received:\n");
        hexdump(reply_buf, test->reply_len);
      }
    }
  }

  if (!(test->flags & FLAG_WARN) && !success)
    fail_count++;

  printf("Success: %d  Flags: 0x%02x\n", success, test->flags);

  return success;
}

void add_little_endian(uint8_t *ptr, uint16_t val, uint16_t size)
{
  uint16_t idx;


  for (idx = 0; idx < size; idx++) {
    *(ptr + idx) = (uint8_t) (val & 0xFF);
    val >>= 8;
  }

  return;
}

void add_big_endian(uint8_t *ptr, uint16_t val, uint16_t size)
{
  uint16_t idx;


  for (idx = 0; idx < size; idx++) {
    *(ptr + (size - 1 - idx)) = (uint8_t) (val & 0xFF);
    val >>= 8;
  }

  return;
}

void add_val(uint8_t *ptr, uint16_t val, uint16_t size, ENDIAN endian, uint16_t *pos)
{
  size /= 8;
  ptr += *pos;
  switch (endian) {
  case ENDIAN_LITTLE:
    add_little_endian(ptr, val, size);
    break;

  case ENDIAN_BIG:
    add_big_endian(ptr, val, size);
    break;
  }

  *pos += size;

  return;
}

void add_aux_val(TestCommand *test, uint16_t val, uint16_t size, ENDIAN endian, uint16_t *auxpos)
{
  add_val(&test->aux1, val, size, endian, auxpos);
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
    add_aux_val(test, val, 8, ENDIAN_NATIVE, auxpos);
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
    add_aux_val(test, test->data_len, arg->size, arg->endian, auxpos);
    break;

  case TYPE_STRUCT:
    // This is pretty much the same as TYPE_UNSIGNED but packs into
    // data instead of aux
    *dataptr = data_buf;
    add_val(data_buf, atoi(input), arg->size, arg->endian, &test->data_len);
    break;

  default: // integer types
    // FIXME - handle unsigned greater than 32767
    add_aux_val(test, atoi(input), arg->size, arg->endian, auxpos);
    break;
  }

  return;
}

int hex_val(char c) {
    if (c >= '0' && c <= '9')
      return c - '0';
    if (c >= 'a' && c <= 'f')
      return c - 'a' + 10;
    if (c >= 'A' && c <= 'F')
      return c - 'A' + 10;
    return -1;
}

size_t decode_hex(uint8_t *dest, const char *source)
{
  size_t count = 0;
  int high_nibble = -1;


  while (*source) {
    int val = hex_val(*source);
    if (val == -1) {
      source++;
      continue;
    }

    if (high_nibble == -1) {
      // This is the first character of a byte pair
      high_nibble = val;
    }
    else {
      // This is the second character; combine them
      dest[count++] = (high_nibble << 4) | val;
      high_nibble = -1;
    }
    source++;
  }

  return count;
}

void *load_expected(uint16_t index)
{
  int16_t len;
  static BinaryBlob expected;


  sprintf(query, "/%d/expected/hex", index);
  len = json_query(query, command);
  if (len) {
    expected.len = decode_hex(expected_buf, command);
    expected.is_binary = true;
    expected.data = expected_buf;
    return &expected;
  }

  sprintf(query, "/%d/expected", index);
  len = json_query(query, command);
  if (!len)
    return NULL;

  strncpy((char *) expected_buf, command, len);
  expected.len = strlen(expected_buf);
  expected.is_binary = false;
  expected.data = expected_buf;
  return &expected;
}

void execute_tests(const char *path)
{
  int16_t bytesread;
  int count = 0;
  FujiCommand *cmd;
  uint16_t idx, dev_idx;
  void *data;
  BinaryBlob *expected;
  bool success;

  printf("Running tests...\n");

  if (json_open(path) != FN_ERR_OK) {
    printf("Failed to open JSON file\n");
    return;
  }

  results_reset();

  while (true) {
    sprintf(query, "/%d/command", count);
    bytesread = json_query(query, command);
    if (!bytesread)
      break;

    printf("command: %s\n", command);
    result_create(count, command);

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

    expected = load_expected(count);

    sprintf(query, "/%d/warnOnly", count);
    bytesread = json_query(query, command);
    if (bytesread)
      exec_test.flags |= FLAG_WARN;

    sprintf(query, "/%d/errorExpected", count);
    bytesread = json_query(query, command);
    if (bytesread)
      exec_test.flags |= FLAG_EXPERR;

    success = run_test(&exec_test, data, expected);
    result_record(count, &exec_test, cmd, success);
    count++;

    if (!(exec_test.flags & FLAG_WARN) && !success)
    {
      printf("TEST FAILED\n");
      return;
    }

  }

  json_close();
}
