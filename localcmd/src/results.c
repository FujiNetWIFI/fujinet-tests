#include "results.h"
#include "testing.h"
#include "platform.h"
#include "console.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct {
  uint16_t index;
  uint8_t command;
  char *command_name;
  uint8_t device;
  bool success;
  uint8_t flags;
} TestResult;

typedef struct ResultNode {
  TestResult *tr;
  struct ResultNode *prev, *next;
} ResultNode;

typedef struct {
  uint16_t total, pass_count, warn_count;
  ResultNode *head;
  ResultNode *tail;
  ResultNode *last_failure;  // end of failure block
  ResultNode *last_warn;     // end of warn block (warns are after failures)
} ResultList;

ResultList result_list;
AdapterConfig fn_config;
char outbuf[80];
char resultbuf[5];
char temp_cmd_name[MAX_COMMAND_LENGTH];

void results_reset()
{
  result_list.pass_count = result_list.warn_count = 0;
  result_list.last_failure = result_list.last_warn = 0;
  return;
}

ResultNode *node_find(uint16_t index)
{
  ResultNode *node;


  for (node = result_list.head; node; node = node->next) {
    if (node->tr->index == index)
      return node;
  }

  return NULL;
}

void node_detach(ResultNode *node)
{
  if (result_list.last_failure == node)
    result_list.last_failure = node->prev;
  if (result_list.last_warn == node)
    result_list.last_warn = node->prev;

  if (node->prev)
    node->prev->next = node->next;
  else
    result_list.head = node->next;

  if (node->next)
    node->next->prev = node->prev;
  else
    result_list.tail = node->prev;

  node->prev = node->next = NULL;
  return;
}

void node_insert(ResultNode *after, ResultNode *new)
{
  if (!after) { // Insert at beginning of list
    new->next = result_list.head;
    new->prev = NULL;
    if (result_list.head)
      result_list.head->prev = new;
    if (!result_list.tail)
      result_list.tail = new;
    result_list.head = new;
    return;
  }

  new->next = after->next;
  new->prev = after;
  if (after->next)
    after->next->prev = new;
  else
    result_list.tail = new;
  after->next = new;
}

void move_to_failure(ResultNode *node)
{
  ResultNode *after;


  node_detach(node);
  after = result_list.last_failure;
  node_insert(after, node);
  result_list.last_failure = node;
  if (result_list.last_warn == after)
    result_list.last_warn = node;

  return;
}

void move_to_warn(ResultNode *node)
{
  ResultNode *after;


  node_detach(node);
  after = result_list.last_warn;
  if (!after)
    after = result_list.last_failure;

  node_insert(after, node);
  result_list.last_warn = node;

  return;
}

void move_to_success(ResultNode *node)
{
  node_detach(node);
  node_insert(result_list.tail, node);
  return;
}

void result_append(TestResult *result)
{
  ResultNode *node;


  node = (ResultNode *) malloc(sizeof(*node));
  node->tr = result;
  node->prev = node->next = NULL;
  if (!result_list.head) {
    result_list.head = result_list.tail = node;
  }
  else {
    node->prev = result_list.tail;
    result_list.tail->next = node;
    result_list.tail = node;
  }

  result_list.total++;
  return;
}

void result_create(uint16_t index, const char *cmd_name)
{
  TestResult *result;
  ResultNode *node;


  node = node_find(index);
  if (node)
    result = node->tr;
  else {
    result = (TestResult *) malloc(sizeof(TestResult));
    result_append(result);
  }

  memset(result, 0, sizeof(*result));
  strcpy(temp_cmd_name, cmd_name);
  result->index = index;
  result->command_name = temp_cmd_name;
  return;
}

bool node_update(ResultNode *node)
{
    bool is_warn, is_fail;
    ResultNode *insert_after = NULL;
    TestResult *result;


    result = node->tr;

    /* Classify */
    is_warn = (!result->success) && (result->flags & FLAG_WARN);
    is_fail = (!result->success) && !is_warn; /* i.e., failure without warn */
    /* pass is result->success == true */

    if (is_fail) {
      /* Bucket 1: FAIL (success==false, not WARN) */
      move_to_failure(node);
    }
    else if (is_warn) {
      /* Bucket 2: WARN (success==false AND FLAG_WARN) */
      move_to_warn(node);
      result_list.warn_count++;
    }
    else {
      /* Bucket 3: PASS */
      move_to_success(node);
      result_list.pass_count++;
    }

    return true;
}

void result_record(uint16_t index, TestCommand *test, FujiCommand *cmd, bool success)
{
  ResultNode *node;
  TestResult *result;


  node = node_find(index);
  if (!node)
    return;

  result = node->tr;
  result->command_name = cmd->name;
  result->command = test->command;
  result->device = test->device;
  result->success = success;
  result->flags = test->flags;

  node_update(node);
  return;
}

void print_test_result_header(char *fn_version)
{
    int fail_count = result_list.total - result_list.pass_count - result_list.warn_count;

    printf("LCLTEST: %s FujiNet FW: %s\n\n", GIT_VERSION, fn_version);
    printf("Computer: %s\n", computer_model());
    printf("Total: %d PASS: %d WARN: %d FAIL: %d\n",
           result_list.total, result_list.pass_count, result_list.warn_count, fail_count);
    printf("\n");
}

void print_test_results()
{
    ResultNode *n;
    TestResult *result;
    int count = 0;
    int line_count = 0;
    int page_size = console_height - 8;

    clrscr();

    print_test_result_header(fn_config.fn_version);

    n = result_list.head;
    for (count = 0; count < result_list.total; count++)
    {
        result = n->tr;

        if (result->success)
        {
            strcpy(resultbuf, "PASS");
        }
        else
        {
            if (result->flags & FLAG_WARN)
            {
                strcpy(resultbuf, "WARN");
            }
            else
            {
                strcpy(resultbuf, "FAIL");
            }
        }

        sprintf(outbuf, "%s 0x%02x:%02x %s\n", resultbuf, result->device, result->command, result->command_name);
        printf("%s", outbuf);
        if (strlen(outbuf) >= console_width)
        {
            line_count +=2;
        }
        else
        {
            line_count++;
        }

        if ((line_count && line_count % page_size == 0) || !n->next)
        {
            printf("\nPress any key to continue...");
            fflush(stdout);
            cgetc();
            printf("\n");
            if (n->next)
            {
                clrscr();
                print_test_result_header(fn_config.fn_version);
            }
        }

        n = n->next;
    }

}
