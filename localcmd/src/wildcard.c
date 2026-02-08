#include <ctype.h>
#include <string.h>
#include <wildcard.h>

#define MAX_STR_LEN 128
#define MAX_PATTERN_LEN 64

// Function that matches input string against given wildcard pattern
bool wildcard_match(const char *str, const char *pattern)
{
    int m, n, i, j;
    static bool lookup[MAX_STR_LEN + 1][MAX_PATTERN_LEN + 1];
    
    if (str == NULL || pattern == NULL)
        return false;
    
    m = strlen(pattern);
    n = strlen(str);
    
    // Check bounds
    if (n > MAX_STR_LEN || m > MAX_PATTERN_LEN)
        return false;
    
    // Empty pattern can only match with empty string
    if (m == 0)
        return (n == 0);
    
    // Initialize lookup table to false
    memset(lookup, 0, sizeof(lookup));
    
    // Empty pattern can match with empty string
    lookup[0][0] = true;
    
    // Only '*' can match with empty string
    for (j = 1; j <= m; j++)
        if (pattern[j - 1] == '*')
            lookup[0][j] = lookup[0][j - 1];
    
    // Fill the table in bottom-up fashion
    for (i = 1; i <= n; i++)
    {
        for (j = 1; j <= m; j++)
        {
            if (pattern[j - 1] == '*')
            {
                lookup[i][j] = lookup[i][j - 1] || lookup[i - 1][j];
            }
            else if (pattern[j - 1] == '?' ||
                     (pattern[j - 1] == '#' && isdigit((unsigned char)str[i - 1])) ||
                     (pattern[j - 1] == '@' && isalpha((unsigned char)str[i - 1])) ||
                     (pattern[j - 1] == '%' && isalnum((unsigned char)str[i - 1])) ||
                     str[i - 1] == pattern[j - 1] ||
                     tolower((unsigned char)str[i - 1]) == tolower((unsigned char)pattern[j - 1]))
            {
                lookup[i][j] = lookup[i - 1][j - 1];
            }
            else
                lookup[i][j] = false;
        }
    }
    
    return lookup[n][m];
}