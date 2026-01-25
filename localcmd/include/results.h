#ifndef RESULTS_H
#define RESULTS_H

#ifdef __MSX__
#include <stdio.h> // MSX changes bool typedef in stdio.h so need to include it first
#endif /* __MSX__ */
#include <fujinet-fuji.h>

typedef struct {
  uint8_t command;
  char *command_name;
  uint8_t device;
  bool success;
  uint8_t flags;
} TestResult;

typedef struct ResultNode {
  TestResult *tr;           
  struct ResultNode *next;
} ResultNode;

typedef struct {
  ResultNode *head;
  ResultNode *tail;
  ResultNode *last_failure;  // end of failure block
  ResultNode *last_warn;     // end of warn block (warns are after failures)
} ResultList;


extern AdapterConfigExtended fn_config;
extern ResultList result_list;

void result_list_init(ResultList *list);
bool result_list_insert(ResultList *list, TestResult *tr);
void print_test_results();

#endif /* RESULTS_H */
