#ifndef RESULTS_H
#define RESULTS_H

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


extern ResultList result_list;

#define RESULTLIST_FOR_EACH(node_ptr, list_ptr) \
  for (ResultNode *(node_ptr) = (list_ptr)->head; \
       (node_ptr) != NULL; \
       (node_ptr) = (node_ptr)->next)

void result_list_init(ResultList *list);
bool result_list_insert(ResultList *list, TestResult *tr);
void print_test_results();

#endif /* RESULTS_H */