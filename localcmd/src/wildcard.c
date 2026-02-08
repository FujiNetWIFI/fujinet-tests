#include <ctype.h>
#include <string.h>
#include <wildcard.h>

#define MAX_STR_LEN 128
#define MAX_PATTERN_LEN 64

// Helper function for recursive matching
static bool match_char(char str_ch, char pat_ch)
{
    if (pat_ch == '?')
        return true;
    if (pat_ch == '#')
        return (bool)isdigit((unsigned char)str_ch);
    if (pat_ch == '@')
        return (bool)isalpha((unsigned char)str_ch);
    if (pat_ch == '%')
        return (bool)isalnum((unsigned char)str_ch);

    return (str_ch == pat_ch ||
            tolower((unsigned char)str_ch) == tolower((unsigned char)pat_ch));
}

// Recursive wildcard matching function
bool wildcard_match(const char *str, const char *pattern)
{
    if (str == NULL || pattern == NULL)
        return false;
    
    // If we reach end of both strings, it's a match
    if (*pattern == '\0')
        return (*str == '\0');
    
    // Handle '*' wildcard
    if (*pattern == '*')
    {
        // Skip consecutive '*' characters
        while (*pattern == '*')
            pattern++;
        
        // If '*' is at the end, it matches everything remaining
        if (*pattern == '\0')
            return true;
        
        // Try matching '*' with empty string, or with one or more characters
        while (*str != '\0')
        {
            if (wildcard_match(str, pattern))
                return true;
            str++;
        }
        
        // Try matching '*' with empty string at end
        return wildcard_match(str, pattern);
    }
    
    // If string is empty but pattern is not (and not '*'), no match
    if (*str == '\0')
        return false;
    
    // Check if current characters match
    if (match_char(*str, *pattern))
    {
        // Move to next characters
        return wildcard_match(str + 1, pattern + 1);
    }
    
    // Characters don't match
    return false;
}