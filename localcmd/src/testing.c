#include "testing.h"

enum {
  FUJI_DEVICEID_FILE            = 0xAA,
};

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
    success = fuji_bus_call(test->device, test->command, test->flags & 0b11101111,
                            test->aux1, test->aux2, test->aux3, test->aux4,
                            data, test->data_len,
                            test->reply_len ? reply : NULL, test->reply_len);
#if 0
  }
#endif // 0

  if (!(test->flags & FLAG_WARN) && !success)
    fail_count++;

  // FIXME - if expected is not NULL check reply

  printf("Success: %d  Flags: 0x%02x\n", success, test->flags);

  return success;
}

