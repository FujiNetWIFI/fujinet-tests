#include "platform.h"

int screen_width = 40;
int page_size = 20;

void init(void)
{
    initCoCoSupport();
    if (isCoCo3)
    {
        width(40);
        screen_width = 40;
        page_size = 18;
    }
    else
    {
        width(32);
        screen_width = 32;
        page_size = 10;
    }

    return;
}

void platform_number_string(char *buffer)
{    
    if (isCoCo3)
    {
        strcpy(buffer, "3");
    }
    else
    {
        strcpy(buffer, "1/2");
    }   
    return;
}