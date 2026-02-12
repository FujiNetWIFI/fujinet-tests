#include "results.h"
#include "testing.h"
#include "platform.h"
#include "console.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

ResultList result_list;
AdapterConfig fn_config;
char outbuf[80];
char resultbuf[5];
int total, pass_count, warn_count;

void result_list_init(ResultList *list)
{
    list->head = 0;
    list->tail = 0;
    list->last_failure = 0;
    list->last_warn = 0;
    total = pass_count = warn_count = 0;
}

bool result_list_insert(ResultList *list, TestResult *tr)
{
    bool is_warn, is_fail;
    ResultNode *node;
    ResultNode *insert_after = NULL;


    node = (ResultNode *)malloc(sizeof(*node));
    if (!node)
        return false;

    total++;
    node->tr = tr;
    node->next = 0;

    /* Classify */
    is_warn = (!tr->success) && (tr->flags & FLAG_WARN);
    is_fail = (!tr->success) && !is_warn; /* i.e., failure without warn */
    /* pass is tr->success == true */
    if (is_warn)
      warn_count++;
    else if (!is_fail)
      pass_count++;

    if (!list->head)
    {
        list->head = list->tail = node;
        if (is_fail)
            list->last_failure = node;
        else if (is_warn)
            list->last_warn = node;
        return true;
    }

    /* Bucket 1: FAIL (success==false, not WARN) */
    if (is_fail)
    {
        if (!list->last_failure)
        {
            node->next = list->head;
            list->head = node;
        }
        else
        {
            node->next = list->last_failure->next;
            list->last_failure->next = node;
        }

        if (!node->next)
            list->tail = node;

        list->last_failure = node;

        /* If WARN block existed, and we inserted before it, it remains valid.
           (We inserted after last_failure, which is before WARN block by construction.) */
        return true;
    }

    /* Bucket 2: WARN (success==false AND FLAG_WARN) */
    if (is_warn)
    {
        if (list->last_warn)
            insert_after = list->last_warn; /* after last WARN */
        else if (list->last_failure)
            insert_after = list->last_failure; /* after FAIL block */

        if (!insert_after)
        {
            node->next = list->head;
            list->head = node;
        }
        else
        {
            node->next = insert_after->next;
            insert_after->next = node;
        }

        if (!node->next)
            list->tail = node;

        list->last_warn = node;
        return true;
    }

    /* Bucket 3: PASS */
    list->tail->next = node;
    list->tail = node;
    return true;
}

void print_test_result_header(char *fn_version)
{
    int fail_count = total - pass_count - warn_count;

    printf("LCLTEST: %s FujiNet FW: %s\n\n", GIT_VERSION, fn_version);
    printf("Computer: %s\n", computer_model());
    printf("Total: %d PASS: %d WARN: %d FAIL: %d\n",
           total, pass_count, warn_count, fail_count);
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
    for (count = 0; count < total; count++)
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
