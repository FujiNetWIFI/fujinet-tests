#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <conio.h>
#include <fujinet-fuji.h> 

#include "results.h"
#include "testing.h"
#include "platform.h"

ResultList result_list;
AdapterConfigExtended config;
char outbuf[80];
char resultbuf[5];

void result_list_init(ResultList *list)
{
    list->head = 0;
    list->tail = 0;
    list->last_failure = 0;
    list->last_warn = 0;
}

bool result_list_insert(ResultList *list, TestResult *tr)
{
    bool is_warn, is_fail;
    ResultNode *node;
    ResultNode *insert_after = NULL;


    node = (ResultNode *)malloc(sizeof(*node));
    if (!node)
        return false;

    node->tr = tr;
    node->next = 0;

    /* Classify */
    is_warn = (!tr->success) && (tr->flags & FLAG_WARN);
    is_fail = (!tr->success) && !is_warn; /* i.e., failure without warn */
    /* pass is tr->success == true */

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
    char platform_num_buf[4];
    printf("Test Results:\n");
    printf("FujiNet Version: %s\n\n", fn_version);
    printf("Platform name: %s ", platform_name());

    platform_number_string(platform_num_buf);
    if (platform_num_buf[0] != '\0')
    {
        printf("(%s)", platform_num_buf);
    }   

    printf("\n\n");
}

void print_test_results()
{
    int count = 0;
    int line_count = 0;
    int pass_count = 0;
    int warn_count = 0;
    ResultNode *n;
    TestResult *result;

    clrscr();

    fuji_get_adapter_config_extended(&config);

    print_test_result_header(config.fn_version);

    n = result_list.head;
    while (n != 0)
    {
        result = n->tr;

        if (result->success)
        {
            strcpy(resultbuf, "PASS");
            pass_count++;
        }
        else
        {
            if (result->flags & FLAG_WARN)
            {
                strcpy(resultbuf, "WARN");
                warn_count++;
            }
            else
            {
                strcpy(resultbuf, "FAIL");
            }   
        }

        sprintf(outbuf, "%s 0x%02x:%02x %s\n", resultbuf, result->device, result->command, result->command_name);
        printf("%s", outbuf);
        if (strlen(outbuf) >= screen_width)
        {
            line_count +=2;
        }
        else
        {
            line_count++;
        }

        count++;

        if (line_count % page_size == 0 || line_count + 1 >= page_size)
        {

            if (count != 0)
            {
                printf("\nPress any key to continue...");
                cgetc();
                clrscr();
                print_test_result_header(config.fn_version);
            }

        }

        n = n->next;
    }

    if (screen_width > 32)
    {
        printf("\nTotal: %u Passed: %u  Warn: %u Failed: %u\n", count, pass_count, warn_count, count - pass_count - warn_count);
    }
    else
    {
        printf("\nTotal: %u\nPassed: %u Warn: %uFailed: %u\n", count, pass_count, warn_count, count - pass_count - warn_count);
    }
}
