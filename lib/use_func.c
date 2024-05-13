#include "use_func.h"
#include "../uart/uart0.h"

void *memcpy(void *dest, const void *src, unsigned long n)
{
    char *d = dest;
    const char *s = src;
    while (n--)
    {
        *d++ = *s++;
    }
    return dest;
}

void strncpy(char *dest, const char *src, int n)
{
    int i = 0;
    while (i < n && src[i] != '\0')
    {
        dest[i] = src[i];
        i++;
    }
    while (i < n)
    {
        dest[i] = '\0';
        i++;
    }
}

int strlen(const char *str)
{
    int len = 0;
    while (str[len] != '\0')
    {
        len++;
    }
    return len;
}

int strncmp(const char *str1, const char *str2, unsigned int n)
{
    for (unsigned int i = 0; i < n; i++)
    {
        if (str1[i] == '\0' || str2[i] == '\0' || str1[i] != str2[i])
        {
            return str1[i] - str2[i];
        }
    }
    return 0;
}

// from https://github.com/openbsd/src/blob/fe4b30a0642f0cd4c7dedc0f03aef70250ce15b6/lib/libc/string/strtok.c#L42
char *strtok_r(char *s, const char *delim, char **last)
{
    const char *spanp;
    int c, sc;
    char *tok;

    if (s == 0 && (s = *last) == 0)
        return (0);

    /*
     * Skip (span) leading delimiters (s += strspn(s, delim), sort of).
     */
cont:
    c = *s++;
    for (spanp = delim; (sc = *spanp++) != 0;)
    {
        if (c == sc)
            goto cont;
    }

    if (c == 0)
    { /* no non-delimiter characters */
        *last = 0;
        return (0);
    }
    tok = s - 1;

    /*
     * Scan token (scan for delimiters: s += strcspn(s, delim), sort of).
     * Note that delim must have one NUL; we stop if we see that, too.
     */
    for (;;)
    {
        c = *s++;
        spanp = delim;
        do
        {
            if ((sc = *spanp++) == c)
            {
                if (c == 0)
                    s = 0;
                else
                    s[-1] = '\0';
                *last = s;
                return (tok);
            }
        } while (sc != 0);
    }
    /* NOTREACHED */
}

int isspace(unsigned char c)
{
    return c == ' ' || c == '\t' || c == '\n' || c == '\v' || c == '\f' || c == '\r';
}

// from https://stackoverflow.com/questions/122616/how-do-i-trim-leading-trailing-whitespace-in-a-standard-way
void trim(char *str)
{
    int i, j;

    // Find the beginning of the non-whitespace characters
    i = 0;
    while (str[i] != '\0' && isspace((unsigned char)str[i]))
    {
        i++;
    }

    // If the entire string was whitespace, set the string to empty
    if (str[i] == '\0')
    {
        str[0] = '\0';
        return;
    }

    // Find the end of the non-whitespace characters
    j = strlen(str) - 1;
    while (j >= 0 && isspace((unsigned char)str[j]))
    {
        j--;
    }

    // Move the 0 terminator to the end of the non-whitespace characters
    str[j + 1] = '\0';

    // Shift the non-whitespace characters to the beginning of the string
    if (i > 0)
    {
        int k = 0;
        while (str[i] != '\0')
        {
            str[k++] = str[i++];
        }
        str[k] = '\0';
    }
}

void to_uppercase(char *str)
{
    for (int i = 0; str[i]; i++)
    {
        if (str[i] >= 'a' && str[i] <= 'z')
        {
            str[i] = str[i] - 32;
        }
    }
}

void wait_msec(unsigned int msVal){
    register unsigned long f, t, r, expiredTime; //64 bits

    // Get the current counter frequency (Hz), 1Hz = 1 pulses/second
    asm volatile ("mrs %0, cntfrq_el0" : "=r"(f));
    
    // Read the current counter value
    asm volatile ("mrs %0, cntpct_el0" : "=r"(t));
    
    // Calculate expire value for counter
    /* Note: both expiredTime and counter value t are 64 bits,
    thus, it will still be correct when the counter is overflow */  
    expiredTime = t + f * msVal / 1000;

    do {
    	asm volatile ("mrs %0, cntpct_el0" : "=r"(r));
    } while(r < expiredTime);
}
