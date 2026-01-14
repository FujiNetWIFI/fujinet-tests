#include "platform.h"

int screen_width;
int page_size;

void init(void)
{
    screen_width = 40;
    page_size = 18;

    return;
}

void platform_number_string(char *buffer)
{    
    // Most platforms have no number variants
    buffer[0] = '\0';  
    return;
}