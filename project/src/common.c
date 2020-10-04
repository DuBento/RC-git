#include "common.h"

void fatal(const char *message) {
    fprintf(stderr, "\033[1;31m[FATAL]: \33[0m%s\n", message);
    exit(EXIT_FAILURE);
}

/* 
int checkString(char *str, void *predicate, int forceLen) {

} */


int isNumber(char c) {
    return c >= '0' && c <= '9';
}

int isChar(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'); 
}

/**
*   Function that checks all chars of a string for alfanumeric chars
*   and the string length is the same as the argument `forceLen`
*   \return String length if evalued to True
*           else False
  */
int checkAlfaNum(char *str, int forceLen) {
    int i=0;
    if (str == NULL) return FALSE;
    
    while (*str!='\0') {
        if (!isNumber(*str) || !isChar(*str))
            return FALSE;
        ++str;
        ++i;
    }
    if (forceLen != 0 && forceLen != i)
        return FALSE;
    return i;
}

/**
*   Function that checks all chars of a string for only numbers
*   and the string length is the same as the argument `forceLen`
*   \return String length if evalued to True
*           else False
  */
int checkOnlyNum(char *str, int forceLen) {
    int i=0;
    if (str == NULL) return FALSE;

    while (*str!='\0') {
        if (!isNumber(*str))
            return FALSE;
        ++str;
        ++i;
    }
    if (forceLen != 0 && forceLen != i)
        return FALSE;
    return i;
}

/**
*   Function that checks all chars of a string for only char
*   and the string length is the same as the argument `forceLen`
*   \return String length if evalued to True
*           else False
  */
int checkOnlyChar(char *str, int forceLen) {
    int i=0;
    if (str == NULL) return FALSE;

    while (*str!='\0') {
        if (!isChar(*str))
            return FALSE;
        ++str;
        ++i;
    }
    if (forceLen != 0 && forceLen != i)
        return FALSE;
    return i;
}


 
/**
*   Function that checks the string for the IPv4 notation
*   "xxx.xxx.xxx.xxx" and xxx <= 255
*   \return True if valid IPv4 notation
*           else False
  */
int checkValidIp(char *ip_str) {
    int delim_counter=0, size, num;
    char *ptr;
    
    if (ip_str == NULL) return FALSE;
    
    ptr = strtok(ip_str, IP_DELIM);
    if (ptr == NULL) return FALSE;
    while(ptr) {
        /* between delim must contain only numbers */
        size = checkOnlyNum(ptr, 0);
        if (size < 0 && size > 3) return FALSE;

        /* check for valid IP parcel number */
        num=atoi(ptr);
        if (num >= 0 && num <= 255) {
            /* continue spliting string */
            ptr = strtok(NULL, IP_DELIM);
            if (ptr != NULL) ++delim_counter;
        } else
            return FALSE;
    }
    
    if (delim_counter != 3) return FALSE;
    
    return TRUE;
}
