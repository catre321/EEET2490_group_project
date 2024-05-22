#ifndef USE_FUNC_H
#define USE_FUNC_H

void *memcpy(void *dest, const void *src, unsigned long n);
void strncpy(char *dest, const char *src, int n);
int strlen(const char *str);
int strncmp(const char *str1, const char *str2, unsigned int n);
char *strtok_r(char *s, const char *delim, char **last);
int isspace(unsigned char c);
void trim(char *str);
void to_uppercase(char *str);
void wait_msec(unsigned int msVal);
int randomRange(int min, int max);

#endif