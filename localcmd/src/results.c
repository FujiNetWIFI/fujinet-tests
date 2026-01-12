
#include <fujinet-fuji.h>
#include "results.h"
#include "testing.h"

#define malloc(len) sbrk(len)

ResultList result_list;

void result_list_init(ResultList *list)
{
    list->head = 0;
    list->tail = 0;
    list->last_failure = 0;
    list->last_warn = 0;
}

bool result_list_insert(ResultList *list, TestResult *tr)
{
    ResultNode *node = (ResultNode *)malloc(sizeof(*node));
    if (!node)
        return false;

    node->tr = tr;
    node->next = 0;

    if (!list->head)
    {
        list->head = list->tail = node;
        if (!tr->success)
        {
            list->last_failure = node;
        }
        else if (tr->flags & FLAG_WARN)
        {
            list->last_warn = node;
        }
        return true;
    }

    /* Bucket 1: failures */
    if (!tr->success)
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
        return true;
    }

    /* Bucket 2: warnings (success==true implied here) */
    if (tr->flags & FLAG_WARN)
    {
        ResultNode *insert_after = 0;

        if (list->last_warn)
        {
            insert_after = list->last_warn;
        }
        else if (list->last_failure)
        {
            insert_after = list->last_failure;
        }

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

    /* Bucket 3: normal successes */
    list->tail->next = node;
    list->tail = node;
    return true;
}

void print_test_results()
{
    int count = 0;
    int pass_count = 0;
    AdapterConfigExtended config;
    ResultNode *n;
    TestResult *result;
    int page_size = 5;

#ifdef _CMOC_VERSION_
    if (isCoCo3)
    {
        page_size = 6;
    }
    else
    {
        page_size = 3;
    }
    cls(1);
#endif /* _CMOC_VERSION_ */
    fuji_get_adapter_config_extended(&config);

    printf("Test Results:\n");
    printf("FujiNet Version: %s\n\n", config.fn_version);

    n = result_list.head;
    while (n != 0)
    {
        result = n->tr;

        printf("Command 0x%02x:%02x (%s) : %s\n", result->device, result->command, result->command_name,
               result->success ? "PASS" : "FAIL");
        printf("FLAGS: ");
        if (result->flags & FLAG_WARN)
            printf(" WARN");
        if (result->flags & FLAG_EXPERR)
            printf(" EXPERR");
        if (result->flags & FLAG_EXCEEDS_U8)
            printf(" EXCEEDS_U8");
        if (result->flags & FLAG_EXCEEDS_U16)
            printf(" EXCEEDS_U16");
        printf("\n\n");
        if (result->success)
            pass_count++;
        count++;

        if (count % page_size == 0)
        {
#ifdef _CMOC_VERSION_
            if (count != 0)
            {
                printf("Press any key to continue...");
                waitkey(0);
                cls(1);
                printf("Test Results:\n");
                printf("FujiNet Version: %s\n\n", config.fn_version);
            }
#else
#endif /* _CMOC_VERSION_ */
        }

        n = n->next;
    }

    printf("Total: %u  Passed: %u  Failed: %u\n",
           count, pass_count, count - pass_count);
}